// Variables
let currentMode = 'CC';
let selectedDigit = null;
let digitValues = [0, 0, 0, 0, 0];
let relayEnabled = false;
let ws = null;
let heartbeatTimeout = null; // Added timeout variable

// Mode configuration
const modeConfig = {
    'CC': { unit: 'A', decimals: 3, maxDigits: 5, beforeDecimal: 2 },
    'CV': { unit: 'V', decimals: 3, maxDigits: 5, beforeDecimal: 2 },
    'CR': { unit: 'kΩ', decimals: 3, maxDigits: 5, beforeDecimal: 2 },
    'CP': { unit: 'W', decimals: 3, maxDigits: 5, beforeDecimal: 2 }
};

// DOM Elements
const digits = document.querySelectorAll('.digit');
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
const resistanceEl = document.getElementById('resistance');
const temperatureEl = document.getElementById('temperature');
const fanSpeedEl = document.getElementById('fan-speed');

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
            // Ensure resistance is handled correctly (e.g., avoid Infinity)
            const resistance = data.measurements.resistance;
            resistanceEl.textContent = (isFinite(resistance) ? resistance.toFixed(3) : '---') + ' kΩ';
            temperatureEl.textContent = data.measurements.temperature.toFixed(1) + ' °C';
            // Update Fan Speed display
            fanSpeedEl.textContent = data.measurements.fanSpeed + ' %';
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
            if (data.state.relayEnabled !== undefined && data.state.relayEnabled !== relayEnabled) {
                relayEnabled = data.state.relayEnabled;
                updateOperationButton(); // Update the combined button
            }
            // Update Output State (affects combined button)
            if (data.state.outputActive !== undefined && data.state.outputActive !== outputActive) {
                outputActive = data.state.outputActive;
                updateOperationButton(); // Update the combined button
            }
            // Update Value Display (Digits)
            // Only update digits if the mode matches and output is NOT active (to avoid overwriting user input)
            // Or if the value received is significantly different from the current display
            const currentValueDisplayed = convertDigitsToNumber();
            if (data.state.value !== undefined && data.state.mode === currentMode) {
                 // Update digits if value changed significantly or if output is off
                 // (prevents minor fluctuations during active output from resetting digits)
                 if (!outputActive || Math.abs(data.state.value - currentValueDisplayed) > 1e-4) {
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

// Initialize the interface
function init() {
    // Select first digit by default - only if display is visible initially
    // selectDigit(digits[0]); // Remove or comment out initial digit selection
    
    // Add event listeners
    digits.forEach(digit => {
        digit.addEventListener('click', () => selectDigit(digit));
    });
    
    modeButtons.forEach(button => {
        button.addEventListener('click', () => {
            const newMode = button.getAttribute('data-mode');
            // Only send command if mode actually changes
            if (newMode !== currentMode) {
                setMode(newMode); // Update UI immediately
                sendCommand('setMode', newMode);
            }
            // Show the value display container when any mode button is clicked
            valueDisplay.classList.remove('hidden'); 
        });
    });
    
    valueUp.addEventListener('click', incrementDigit);
    valueDown.addEventListener('click', decrementDigit);

    toggleOperation.addEventListener('click', () => {
        // Update UI immediately
        relayEnabled = relayEnabled ? false : true; // Toggle relay state
        updateOperationButton();
        // Send command
        sendCommand('setRelay', relayEnabled);
    });

    // Removed event listener for toggleRelay

    exitMode.addEventListener('click', () => {
        sendCommand('exit', null);
        // Optionally reset UI elements immediately or wait for WS confirmation
    });
    
    // Connect to WebSocket
    connectWebSocket();
    
    // Request initial state/measurements after connection (optional, handled by connect event on server)
    // setInterval(() => { sendCommand('getMeasurements', null); }, 2000); // Reduced frequency, rely on broadcasts
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

// Increment selected digit
function incrementDigit() {
    if (!selectedDigit) return;
    
    const position = parseInt(selectedDigit.getAttribute('data-position'));
    digitValues[position] = (digitValues[position] + 1) % 10;
    selectedDigit.textContent = digitValues[position];
    // Send updated value only if output is active OR if user explicitly changes it
    // Avoid sending intermediate values during editing if output is off?
    // Let's send it always for now, backend can decide.
    sendValue();
}

// Decrement selected digit
function decrementDigit() {
    if (!selectedDigit) return;
    
    const position = parseInt(selectedDigit.getAttribute('data-position'));
    digitValues[position] = (digitValues[position] - 1 + 10) % 10;
    selectedDigit.textContent = digitValues[position];
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
    if (!config) return;

    // Format number to fixed decimal places matching the mode
    const formattedNumber = number.toFixed(config.decimals);
    const [integerPart, decimalPart] = formattedNumber.split('.');

    // Pad parts with leading/trailing zeros if necessary
    const paddedInteger = integerPart.padStart(config.beforeDecimal, '0');
    const paddedDecimal = (decimalPart || "").padEnd(config.decimals, '0');

    const valueStr = paddedInteger + paddedDecimal;

    // Update digitValues array and display
    digits.forEach((digit, index) => {
        if (index < valueStr.length) {
            const digitValue = parseInt(valueStr[index]);
            digitValues[index] = digitValue;
            digit.textContent = digitValue;
        } else {
            // Should not happen if padding is correct, but handle defensively
            digitValues[index] = 0;
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
    // Check if mode is valid
    if (!modeConfig[mode]) {
        console.warn(`Invalid mode received or set: ${mode}`);
        return; // Don't change if mode is unknown
    }

    // Update mode buttons
    modeButtons.forEach(button => {
        if (button.getAttribute('data-mode') === mode) {
            button.classList.add('active');
        } else {
            button.classList.remove('active');
        }
    });

    // Update current mode variable
    currentMode = mode;

    // Update unit display
    currentUnit.textContent = modeConfig[mode].unit;

    // Show the value display container when mode is set (e.g., from WS)
    valueDisplay.classList.remove('hidden');

    // Reset digits display to 0 when mode changes via UI click
    // (WS updates might bring a non-zero value)
    // digitValues = Array(modeConfig[mode].maxDigits).fill(0);
    // updateValueFromNumber(0); // Update display based on reset array

    // Select first digit (optional, might be annoying)
    // selectDigit(digits[0]);
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

// Add initial call to set button state correctly on load
document.addEventListener('DOMContentLoaded', () => {
    // ... other init logic ...
    updateOperationButton(); // Set initial state of the combined button and dot
    // Ensure value display is hidden initially (redundant if class is set in HTML)
    // valueDisplay.classList.add('hidden'); 
});
