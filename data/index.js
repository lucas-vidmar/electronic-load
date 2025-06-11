// Variables
let currentMode = 'CC';
let selectedDigit = null;
// let digitValues = [0, 0, 0, 0, 0]; // Initialize dynamically
let digitValues = []; // Initialize as empty, will be sized in updateDigitDisplay
let relayEnabled = false;
let ws = null;
let heartbeatTimeout = null; // Added timeout variable

// Mode configuration
const modeConfig = {
    'CC': { unit: 'A', decimals: 2, maxDigits: 4, beforeDecimal: 2 },
    'CV': { unit: 'V', decimals: 2, maxDigits: 5, beforeDecimal: 3 },
    'CR': { unit: 'kΩ', decimals: 3, maxDigits: 5, beforeDecimal: 2 },
    'CW': { unit: 'W', decimals: 2, maxDigits: 5, beforeDecimal: 3 }
};

// DOM Elements
// const digits = document.querySelectorAll('.digit'); // Remove static selection
let digits = []; // Initialize as an empty array, will be populated dynamically
const digitContainer = document.getElementById('digit-container'); // Get the container for digits
const modeButtons = document.querySelectorAll('.mode-btn');
const valueUp = document.getElementById('value-up');
const valueDown = document.getElementById('value-down');
const toggleOperation = document.getElementById('toggle-operation'); // Renamed from trigger-output
const dutStatusIndicator = document.getElementById('dut-status-indicator'); // Added reference for the dot
const exitMode = document.getElementById('exit-mode');
const currentUnit = document.getElementById('current-unit');
const valueDisplay = document.getElementById('value-display'); // Get reference to the container
const connectionStatusIndicator = document.getElementById('connection-status-indicator'); // Select the connection dot
const statusTextEl = document.getElementById('status-text'); // Select the status text span

// Measurement elements
const voltageEl = document.getElementById('voltage');
const currentEl = document.getElementById('current');
const powerEl = document.getElementById('power');
const energyEl = document.getElementById('energy');
const resistanceEl = document.getElementById('resistance');
const temperatureEl = document.getElementById('temperature');
const fanSpeedEl = document.getElementById('fan-speed');
const fanIconEl = document.getElementById('fan-icon'); // Added fan icon element
const uptimeEl = document.getElementById('uptime-timer'); // Added uptime element


// Warning banner elements
const warningBanner = document.getElementById('warning-banner');
const warningMessage = document.getElementById('warning-message');

// Maximum values for each mode (matching C++ DAC limits)
const maxValues = {
    'CC': 20.0,    // 20A
    'CV': 100.0,   // 100V 
    'CR': 20.475,  // 20475kΩ / 1000 = 20.475kΩ (UI shows kΩ)
    'CW': 200.0,    // 200W
    'T': 60        // 60 °C
};

// Warning message templates (matching C++ error messages)
const warningMessages = {
    'CC': 'Max: {max}A',
    'CV': 'Max: {max}V', 
    'CR': 'Max: {max}kΩ',
    'CW': 'Max: {max}W'
};

// Initialize WebSocket connection
function connectWebSocket() {
    // Clear any existing timeout before attempting connection
    if (heartbeatTimeout) {
        clearTimeout(heartbeatTimeout);
        heartbeatTimeout = null;
    }
    // Replace with your ESP32's IP address or hostname
    ws = new WebSocket('ws://' + window.location.hostname + '/ws');

    ws.onopen = function() {
        statusTextEl.textContent = 'Connected to device'; // Update text content
        connectionStatusIndicator.classList.add('connected'); // Add 'connected' class to dot
        resetHeartbeatTimeout(); // Start the timeout check
    };

    ws.onclose = function() {
        statusTextEl.textContent = 'Disconnected from device'; // Update text content
        connectionStatusIndicator.classList.remove('connected'); // Remove 'connected' class from dot
        // Clear the timeout when connection explicitly closes
        if (heartbeatTimeout) {
            clearTimeout(heartbeatTimeout);
            heartbeatTimeout = null;
        }
        setTimeout(connectWebSocket, 2000); // Try to reconnect
    };

    ws.onmessage = function(event) {
        handleMessage(event.data);
    };

    // Handle potential errors
    ws.onerror = function(error) {
        console.error('WebSocket Error:', error);
        statusTextEl.textContent = 'Connection error';
        connectionStatusIndicator.classList.remove('connected');
        // Clear the timeout on error as well
        if (heartbeatTimeout) {
            clearTimeout(heartbeatTimeout);
            heartbeatTimeout = null;
        }
        // Optionally attempt reconnect on error too, or rely on onclose
    };
}

// Handle incoming WebSocket messages
function handleMessage(message) {
    // Reset the timeout since we received a message
    resetHeartbeatTimeout();

    // Ensure status is shown as connected if it was previously marked as lost
    if (!connectionStatusIndicator.classList.contains('connected')) {
        statusTextEl.textContent = 'Connected to device';
        connectionStatusIndicator.classList.add('connected');
    }

    try {
        const data = JSON.parse(message);
        console.log("Received data:", data); // Log received data

        // Update measurements
        if (data.measurements) {
            voltageEl.textContent = data.measurements.voltage.toFixed(3) + ' V';
            currentEl.textContent = data.measurements.current.toFixed(3) + ' A';
            powerEl.textContent = data.measurements.power.toFixed(3) + ' W';
            energyEl.textContent = data.measurements.energy.toFixed(3) + ' kJ';
            // Ensure resistance is handled correctly (e.g., avoid Infinity)
            const resistance = data.measurements.resistance;
            resistanceEl.textContent = (isFinite(resistance) ? resistance.toFixed(3) : '---') + ' Ω';
            temperatureEl.textContent = data.measurements.temperature.toFixed(1) + ' °C';
            // Update Fan Speed display
            const fanSpeed = data.measurements.fanSpeed;
            fanSpeedEl.textContent = fanSpeed + ' %';
            if (fanSpeed > 0) {
                fanIconEl.classList.add("spinning");
                // Calculate duration based on speed, ensuring a minimum duration for visibility
                // Lower fan speed = slower animation (longer duration)
                // Higher fan speed = faster animation (shorter duration)
                // Example: 100% speed = 1s duration, 1% speed = 100s duration (adjust multiplier as needed)
                const duration = Math.max(0.5, 100 / fanSpeed); // Ensure minimum 0.5s duration
                fanIconEl.style.animationDuration = duration + 's';
            }
            else {
                fanIconEl.classList.remove("spinning");
            }
            // Update Uptime display
            uptimeEl.textContent = data.measurements.uptime;

            // Check if maximum values have been exceeded
            if (data.measurements.voltage > maxValues.CV * 1.1) {
                showWarning("Voltage exceeds max limit of " + maxValues.CV.toFixed(0) + " V");
                setTimeout(hideWarning, 2000);
            } else if (data.measurements.current > maxValues.CC * 1.1) {
                showWarning("Current exceeds max limit of " + maxValues.CC.toFixed(0) + " A");
                setTimeout(hideWarning, 2000);
            } else if (data.measurements.power > maxValues.CW * 1.1) {
                showWarning("Power exceeds max limit of " + maxValues.CW.toFixed(0) + " W");
                setTimeout(hideWarning, 2000);
            } else if (data.measurements.temperature > maxValues.T * 1.1) {
                showWarning("Temperature exceeds max limit of " + maxValues.T.toFixed(0) + " °C");
                setTimeout(hideWarning, 2000);
            }
        }

        // Update state
        if (data.state) {
            // Check if returning to MENU
            if (data.state.mode === "MENU") {
                valueDisplay.classList.add('hidden'); // Hide controls
                // Deactivate all mode buttons
                modeButtons.forEach(button => {
                    button.classList.remove('active');
                });
                currentMode = "MENU"; // Update local mode state
            } else if (data.state.mode && data.state.mode !== currentMode) {
                // Update Mode (if not MENU and different from current)
                setMode(data.state.mode); // Update mode buttons and unit
            }
            // Update Relay State (affects combined button)
            if (data.state.outputActive !== undefined && data.state.outputActive !== relayEnabled) {
                relayEnabled = data.state.outputActive;
                updateOperationButton(); // Update the combined button
            }
            // Update Value Display (Digits)
            // Only update digits if the mode matches and output is NOT active (to avoid overwriting user input)
            // Or if the value received is significantly different from the current display
            const currentValueDisplayed = convertDigitsToNumber();
            if (data.state.value !== undefined && data.state.mode === currentMode) {
                // Update digits if value changed significantly or if output is off
                // (prevents minor fluctuations during active output from resetting digits)
                // Also check if digits array is populated for the current mode
                if (digits.length > 0 && (!relayEnabled || Math.abs(data.state.value - currentValueDisplayed) > 1e-4)) {
                    updateValueFromNumber(data.state.value);
                }
            }
        }
    } catch (e) {
        console.error('Error parsing message:', e);
        statusTextEl.textContent = 'Error processing message'; // Update text content
        // Don't change connection dot color here, let timeout/onclose handle it
    }
}

// Function to reset the heartbeat timeout
function resetHeartbeatTimeout() {
    if (heartbeatTimeout) {
        clearTimeout(heartbeatTimeout);
    }
    heartbeatTimeout = setTimeout(() => {
        console.warn('WebSocket timeout: No message received for 10 seconds.');
        statusTextEl.textContent = 'Connection lost? No updates...';
        connectionStatusIndicator.classList.remove('connected');
        // Do not automatically try to reconnect here, wait for onclose or manual action
        heartbeatTimeout = null; // Clear the handle after firing
    }, 10000); // 10 seconds
}

// Send command to ESP32
function sendCommand(command, value) {
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({
            command: command,
            value: value
        }));
    }
}

// Function to dynamically create digits based on mode
function updateDigitDisplay(mode) {
    const config = modeConfig[mode];
    if (!config) return; // Exit if mode is invalid

    digitContainer.innerHTML = ''; // Clear existing digits
    digits = []; // Reset the digits array
    digitValues = Array(config.maxDigits).fill(0); // Reset digit values based on new maxDigits

    for (let i = 0; i < config.maxDigits; i++) {
        // Insert decimal point
        if (i === config.beforeDecimal) {
            const dot = document.createElement('span');
            dot.classList.add('decimal-point');
            dot.textContent = '.';
            digitContainer.appendChild(dot);
        }

        const digit = document.createElement('span');
        digit.classList.add('digit');
        digit.setAttribute('data-position', i);
        digit.textContent = digitValues[i]; // Initialize with 0
        digit.addEventListener('click', () => selectDigit(digit)); // Add listener
        digitContainer.appendChild(digit);
        digits.push(digit); // Add to the digits array
    }

    // Update unit display as well
    currentUnit.textContent = config.unit;

    // Deselect any previously selected digit
    if (selectedDigit) {
        selectedDigit.classList.remove('selected');
        selectedDigit = null;
    }
    // Optionally select the first digit (can be annoying)
    // if (digits.length > 0) {
    //     selectDigit(digits[0]);
    // }
}


// Initialize the interface
function init() {

    // Add event listeners for static elements
    modeButtons.forEach(button => {
        button.addEventListener('click', () => {
            const newMode = button.getAttribute('data-mode');
            const modeChanged = newMode !== currentMode; // Check if mode actually changed

            // Always update the UI display when a mode button is clicked
            // This ensures digits are rendered even if the mode hasn't changed (e.g., first click)
            setMode(newMode);
            valueDisplay.classList.remove('hidden'); // Ensure display is visible

            // Only send commands and reset value if the mode actually changed
            if (modeChanged) {
                sendCommand('setMode', newMode);
                // Reset value to 0 when mode changes via UI click
                updateValueFromNumber(0); // This will use the new digit structure
                sendValue(); // Send the reset value
            }
        });
    });

    valueUp.addEventListener('click', () => { modifyDigit(1); });
    valueDown.addEventListener('click', () => { modifyDigit(-1); });

    toggleOperation.addEventListener('click', () => {
        relayEnabled = !relayEnabled;
        updateOperationButton();
        sendCommand('setRelay', relayEnabled);
    });

    exitMode.addEventListener('click', () => {
        sendCommand('exit', null);
        relayEnabled = false; // Reset relay state
        valueDisplay.classList.add('hidden'); // Hide controls on exit
        // Deactivate mode buttons visually
        modeButtons.forEach(button => button.classList.remove('active'));
        currentMode = "MENU"; // Reflect exit state locally
        if (selectedDigit) {
            selectedDigit.classList.remove('selected');
            selectedDigit = null;
        }
        digitContainer.innerHTML = ''; // Clear digits on exit
        digits = []; // Clear digits array
    });

    connectWebSocket();
    updateOperationButton(); // Set initial button state
    // Value display is hidden by default via HTML class
}

// Select a digit
function selectDigit(digitElement) {
    // Deselect previously selected digit
    if (selectedDigit) {
        selectedDigit.classList.remove('selected');
    }
    
    // Select new digit
    digitElement.classList.add('selected');
    selectedDigit = digitElement;
}

// Show warning banner with formatted message
function showWarning(message) {
    warningMessage.textContent = message;
    warningBanner.classList.remove('hidden', 'fade-out');
}

// Hide warning banner
function hideWarning() {
    warningBanner.classList.add('fade-out');
    setTimeout(() => {
        warningBanner.classList.add('hidden');
        warningBanner.classList.remove('fade-out');
    }, 300); // Match the CSS transition duration
}

// Modify selected digit
function modifyDigit(sign) {
    if (!selectedDigit) return; // No digit selected

    const position = parseInt(selectedDigit.getAttribute('data-position'));
    const config = modeConfig[currentMode];
    if (!config) return;

    // Calculate the decimal place value of the selected digit
    // Position 0 is the leftmost digit (highest place value)
    const digitPlaceValue = Math.pow(10, config.beforeDecimal - position - 1);
    
    // Get current total value
    const currentValue = convertDigitsToNumber();
    
    // Calculate new value by adding/subtracting the place value
    let newValue = currentValue + (sign * digitPlaceValue);
    
    if (newValue < 0) {
        newValue = 0; // Prevent negative values
    }
    else if (newValue > maxValues[currentMode]) {
        newValue = maxValues[currentMode]; // Prevent exceeding max value
        // Show warning for 2 seconds with fade effect
        const message = warningMessages[currentMode].replace('{max}', maxValues[currentMode].toFixed(modeConfig[currentMode].decimals));
        showWarning(message);
        setTimeout(hideWarning, 2000);
    }
    
    // Update all digits from the new value
    updateValueFromNumber(newValue);
    
    sendValue();
}

// Convert digits array to number
function convertDigitsToNumber() {
    const config = modeConfig[currentMode];
    if (!config) return 0; // Handle case where mode might be invalid temporarily

    let valueStr = "";
    for (let i = 0; i < config.maxDigits; i++) {
        valueStr += digitValues[i];
    }

    const integerPart = valueStr.substring(0, config.beforeDecimal);
    const decimalPart = valueStr.substring(config.beforeDecimal);

    return parseFloat(integerPart + "." + decimalPart);
}

// Update digits from a number
function updateValueFromNumber(number) {
    const config = modeConfig[currentMode];
    // Ensure config exists and digits have been created for the current mode
    if (!config || digits.length === 0) return;

    const formattedNumber = number.toFixed(config.decimals);
    const [integerPart, decimalPart] = formattedNumber.split('.');

    const paddedInteger = integerPart.padStart(config.beforeDecimal, '0');
    const paddedDecimal = (decimalPart || "").padEnd(config.decimals, '0');

    const valueStr = paddedInteger + paddedDecimal;

    // Ensure digitValues array has the correct length for the current mode
    if (digitValues.length !== config.maxDigits) {
        digitValues = Array(config.maxDigits).fill(0);
    }

    // Update digitValues array and display using the dynamic digits array
    digits.forEach((digit) => { // No index needed here, use data-position
        const position = parseInt(digit.getAttribute('data-position')); // Get position from attribute
        if (position < valueStr.length) {
            const digitValue = parseInt(valueStr[position]);
            digitValues[position] = digitValue; // Update the correct index in digitValues
            digit.textContent = digitValue;
        } else {
            // This case might occur if the number is smaller than the display capacity
            digitValues[position] = 0;
            digit.textContent = '0';
        }
    });
    
}

// Send current value to ESP32
function sendValue() {
    const value = convertDigitsToNumber();
    // Only send if output is active? Or always send the edited value?
    // Let's send always, backend handles it.
    // If output is active, this effectively changes the target value.
    // If output is inactive, this updates the value to be used when output is triggered.
    sendCommand('setValue', value);
}

// Set current mode
function setMode(mode) {
    if (!modeConfig[mode]) {
        console.warn(`Invalid mode received or set: ${mode}`);
        return;
    }

    // Update mode buttons
    modeButtons.forEach(button => {
        button.classList.toggle('active', button.getAttribute('data-mode') === mode);
    });

    // Update current mode variable
    currentMode = mode;

    // Update the digit display (creates/recreates digits and sets unit)
    updateDigitDisplay(mode);

    // Show the value display container
    valueDisplay.classList.remove('hidden');
    
    // Hide warning banner when switching modes
    hideWarning();

    // Note: Resetting value to 0 is handled in the mode button click listener
    // for UI clicks. WS updates will call updateValueFromNumber directly.
}

// Update combined operation button state and text
function updateOperationButton() {
    // Button text is now fixed: "Toggle DUT"
    // toggleOperation.textContent = '...'; // No longer needed

    // Remove class manipulation for the button itself
    // toggleOperation.classList.remove('success', 'danger'); // No longer needed

    // Update the status dot based on relayEnabled state
    if (relayEnabled) {
        dutStatusIndicator.classList.add('enabled');
    } else {
        dutStatusIndicator.classList.remove('enabled');
    }

    // The button's click logic still handles toggling relay/output,
    // but this function only updates the visual indicator (the dot).
}

// Initialize the interface when the DOM is loaded
document.addEventListener('DOMContentLoaded', init);

// Remove the second DOMContentLoaded listener as init handles setup
// document.addEventListener('DOMContentLoaded', () => { ... });
