// Variables
let currentMode = 'CC';
let selectedDigit = null;
let digitValues = [0, 0, 0, 0, 0];
let relayEnabled = false;
let outputActive = false;
let ws = null;

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
const triggerOutput = document.getElementById('trigger-output');
const toggleRelay = document.getElementById('toggle-relay');
const exitMode = document.getElementById('exit-mode');
const currentUnit = document.getElementById('current-unit');

// Measurement elements
const voltageEl = document.getElementById('voltage');
const currentEl = document.getElementById('current');
const powerEl = document.getElementById('power');
const resistanceEl = document.getElementById('resistance');
const temperatureEl = document.getElementById('temperature');
const fanSpeedEl = document.getElementById('fan-speed');
const statusEl = document.getElementById('status');

// Initialize WebSocket connection
function connectWebSocket() {
    // Replace with your ESP32's IP address or hostname
    ws = new WebSocket('ws://' + window.location.hostname + '/ws');
    
    ws.onopen = function() {
        statusEl.textContent = 'Connected to device';
        statusEl.style.display = 'block';
        statusEl.style.backgroundColor = '#e8f5e9';
    };
    
    ws.onclose = function() {
        statusEl.textContent = 'Disconnected from device';
        statusEl.style.display = 'block';
        statusEl.style.backgroundColor = '#ffebee';
        setTimeout(connectWebSocket, 2000); // Try to reconnect
    };
    
    ws.onmessage = function(event) {
        handleMessage(event.data);
    };
}

// Handle incoming WebSocket messages
function handleMessage(message) {
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
            // Update Mode
            if (data.state.mode && data.state.mode !== currentMode && data.state.mode !== "MENU") {
                setMode(data.state.mode); // Update mode buttons and unit
            }
            // Update Relay Button
            if (data.state.relayEnabled !== undefined && data.state.relayEnabled !== relayEnabled) {
                relayEnabled = data.state.relayEnabled;
                updateRelayButton();
            }
            // Update Output Button and State
            if (data.state.outputActive !== undefined && data.state.outputActive !== outputActive) {
                outputActive = data.state.outputActive;
                updateOutputButton();
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
        statusEl.textContent = 'Error processing message';
        statusEl.style.backgroundColor = '#ffcdd2';
    }
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
    // Select first digit by default
    selectDigit(digits[0]);
    
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
        });
    });
    
    valueUp.addEventListener('click', incrementDigit);
    valueDown.addEventListener('click', decrementDigit);
    
    triggerOutput.addEventListener('click', () => {
        // Toggle intended state
        const intendedOutputState = !outputActive;
        // Update UI immediately for responsiveness
        outputActive = intendedOutputState;
        updateOutputButton();
        // Send command with current value
        const value = convertDigitsToNumber();
        sendCommand('setOutput', { active: intendedOutputState, value: value });
    });
    
    toggleRelay.addEventListener('click', () => {
        // Toggle intended state
        const intendedRelayState = !relayEnabled;
        // Update UI immediately
        relayEnabled = intendedRelayState;
        updateRelayButton();
        // Send command
        sendCommand('setRelay', intendedRelayState);
    });
    
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

    // Reset digits display to 0 when mode changes via UI click
    // (WS updates might bring a non-zero value)
    // digitValues = Array(modeConfig[mode].maxDigits).fill(0);
    // updateValueFromNumber(0); // Update display based on reset array

    // Select first digit (optional, might be annoying)
    // selectDigit(digits[0]);
}

// Update relay button state
function updateRelayButton() {
    if (relayEnabled) {
        toggleRelay.textContent = 'Disable DUT';
        toggleRelay.classList.add('danger');
        toggleRelay.classList.remove('success');
    } else {
        toggleRelay.textContent = 'Enable DUT';
        toggleRelay.classList.add('success');
        toggleRelay.classList.remove('danger');
    }
}

// Update output button state
function updateOutputButton() {
    if (outputActive) {
        triggerOutput.textContent = 'Stop Output';
        triggerOutput.classList.add('danger');
        triggerOutput.classList.remove('success');
    } else {
        triggerOutput.textContent = 'Trigger Output';
        triggerOutput.classList.add('success');
        triggerOutput.classList.remove('danger');
    }
}

// Initialize the interface when the DOM is loaded
document.addEventListener('DOMContentLoaded', init);
