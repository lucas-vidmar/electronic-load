#include "main.h"
#include "webserver.h"
#include "I2CScanner.h"
#include <SPIFFS.h>
#include <ArduinoJson.h> // Include ArduinoJson

#define BROADCAST_INTERVAL 1000 // Interval for broadcasting state updates (in milliseconds)

/* ------- Global Variables ------- */
WebServerESP32 webServer(SSID.c_str(), PASSWORD.c_str());

Encoder encoder = Encoder();
BuiltInLed led = BuiltInLed();
I2C i2c = I2C();
DAC dac = DAC();
ADC adc = ADC();
AnalogSws analogSws = AnalogSws();
LVGL_LCD lcd = LVGL_LCD();
Fan fan(PWM_FAN_PIN, EN_FAN_PIN, LOCK_FAN_PIN);
PIDFanController pidController(fan, PID_KP, PID_KI, PID_KD);
RTC rtc = RTC();
uint64_t startTimeMs = 0;
I2CScanner scanner;
FSM fsm = FSM();


// --- Global Variables for State Management ---
float input = 0.0;
float dut_voltage = 0.0; // Track DUT voltage
float dut_current = 0.0; // Track DUT current
float dut_power = 0.0; // Track DUT power
float dut_resistance = 0.0; // Track DUT resistance

bool output_active = false; // Track output state
float temperature = 0.0; // Track temperature
int fanSpeed = 0; // Get current fan speed percentage

// --- Global Variables for WS/UI Sync ---
float ws_requested_value = 0.0;
bool ws_value_updated = false;
bool ws_delete_main_menu = false; // Track if in main menu


void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  // I2C Scanner
  scanner.Init();
  scanner.Scan();

  // Inicializar SPIFFS
  Serial.println("Inicializando SPIFFS...");
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
    return;
  }

  // Initialize encoder
  encoder.init();
  // Initialize builtin led
  led.init();
  led.blink(500);
  // Initialize relays
  analogSws.init();
  analogSws.relay_dut_disable();
  analogSws.mosfet_input_cc_mode();
  analogSws.v_dac_enable();
  // Initialize I2C devices
  i2c.init();
  dac.init(&i2c);
  adc.init(&i2c);
  // Initialize RTC
  rtc.init(&i2c);
  
  // Set current date and time (example: January 1, 2024 at 12:00:00)
  DateTime currentTime = {0, 0, 12, 1, 1, 1, 24};  // seconds, minutes, hours, dayOfWeek, date, month, year
  rtc.set_time(currentTime);
  
  // Store start time
  startTimeMs = rtc.get_timestamp_ms();
  Serial.println("RTC initialized. Start time recorded.");
  
  // Initialize LVGL Display
  lcd.init();
  // Initialize FANs and PID controllers
  pidController.init(PID_SETPOINT); // Set target temperature for fan
  fan.set_speed(0); // Set initial speed to 0
  // Initialize FSM
  fsm.init();

  // Iniciar el servidor web y WebSocket
  Serial.println("Iniciando servidor web y WebSocket...");
  webServer.set_default_file("index.html");
  webServer.attachWsHandler(onWsEvent); // Attach the WebSocket handler
  webServer.begin();

  // Initial relay state
  analogSws.relay_dut_disable();
  output_active = false; // Ensure output is off
}

void loop() {
  // Handle WebSocket clients
  webServer.cleanupClients(); // Important for AsyncWebServer

  // Update all global variables
  fanSpeed = fan.get_speed_percentage(); // Get current fan speed percentage
  temperature = adc.read_temperature(); // Read temperature from ADC
  dut_voltage = adc.read_vDUT(); // Read DUT voltage
  dut_current = adc.read_iDUT(); // Read DUT current
  dut_power = dut_voltage * dut_current; // Calculate DUT power
  dut_resistance = (dut_current != 0) ? (dut_voltage / dut_current) : 0; // Calculate DUT resistance

  lcd.update();
  lcd.update_header(temperature, fanSpeed); // Update header with temperature and fan speed

  // Store previous state to detect changes
  FSM_MAIN_STATES prevState = fsm.get_current_state();
  float prevInput = input;
  bool prevOutputActive = output_active;

  // Run FSM which might change state or apply 'input'
  fsm.run(input, dac, analogSws, &output_active);

  // Compute and adjust fan speed based on temperature
  pidController.compute(temperature);

  unsigned long lastBroadcastTime = 0;

  // Broadcast state periodically or if changed
  unsigned long currentTime = millis();
  bool stateChanged = (fsm.get_current_state() != prevState) ||
                      (input != prevInput) ||
                      (output_active != prevOutputActive);

  if (stateChanged || (currentTime - lastBroadcastTime >= BROADCAST_INTERVAL)) {
    broadcastState();
    lastBroadcastTime = currentTime;
  }

  if (ws_delete_main_menu) {
    lcd.close_cx_screen(); // Close CX screen if in main menu, moved here because it was taking too long to close
    ws_delete_main_menu = false; // Reset the flag
  }

  // Short delay to prevent busy-waiting
  delay(10);
}

void main_menu() {
  static int pos = 0;

  if (fsm.has_changed()) { // First time entering main menu
    Serial.println("Main menu");
    encoder.set_min_position(0);
    encoder.set_max_position(FSM_MAIN_STATES::SETTINGS - FSM_MAIN_STATES::CC); //  quantity of options in main menu
    encoder.set_position(0);
    pos = 0; // Reset position
    lcd.create_main_menu(); // Create main menu
    lcd.update_main_menu(0);
  }

  if (encoder.has_changed()) { // Update menu with selected option
    pos = encoder.get_position();
    lcd.update_main_menu(pos);
    #ifdef DEBUG_ENCODER
    Serial.println("Encoder position: " + String(encoder.get_position()));
    Serial.println("Max:" + String(encoder.get_encoder_max_position()) + " - Min: " + String(encoder.get_encoder_min_position()));
    #endif
  }

  // Check if encoder button is pressed
  if (encoder.is_button_pressed()) {
    Serial.println("Button pressed - Main Menu Selection");
    fsm.change_state(static_cast<FSM_MAIN_STATES>(FSM_MAIN_STATES::CC + pos)); // Change to selected option
    // Reset vars and close main menu
    Serial.println("Exiting main menu to option " + String(FSM_MAIN_STATES::CC + pos));
    lcd.close_main_menu();
    input = 0.0; // Reset input when changing mode
    output_active = false; // Ensure output is off
    // No return here, let loop handle broadcast
  }

  // If state changed relevant to WS, broadcast
  // Note: FSM change is handled in loop(), but menu selection itself isn't broadcasted yet.
  // We rely on the FSM state change broadcast in loop().
}

float digits_to_number(std::vector<int> digitsValues, int digitsBeforeDecimal, int digitsAfterDecimal, int totalDigits) {
  float number = 0.0;

  // Validate each element to be in range [0, 9]
  for (int i = 0; i < totalDigits; i++) {
    if (digitsValues[i] < 0) {
      digitsValues[i] = 0;
    } else if (digitsValues[i] > 9) {
      digitsValues[i] = 9;
    }
  }

  // Calculate the integer part
  for (int i = 0; i < digitsBeforeDecimal; i++) {
    number += digitsValues[i] * pow(10, digitsBeforeDecimal - i - 1); // Add digits before decimal
  }
  // Calculate the decimal part
  for (int i = 0; i < digitsAfterDecimal; i++) {
    number += digitsValues[digitsBeforeDecimal + i] * pow(10, -i - 1); // Add digits after decimal
  }
  return number;
}

/**
 * @brief Converts a floating-point number back into digits for the UI.
 */
void number_to_digits(float number, std::vector<int>& digitsValues, int digitsBeforeDecimal, int digitsAfterDecimal, int totalDigits) {
    // Ensure the vector has the correct size
    if (digitsValues.size() != totalDigits) {
        digitsValues.resize(totalDigits);
    }

    // Handle potential negative numbers if necessary (assuming non-negative for load)
    number = abs(number);

    // Scale number based on decimal places for integer conversion
    long long int scaled_number = static_cast<long long int>(round(number * pow(10, digitsAfterDecimal)));

    // Extract digits from right to left
    for (int i = totalDigits - 1; i >= 0; --i) {
        if (i == digitsBeforeDecimal -1) { // Position before the implied decimal point
             // This handles the integer part's last digit
             digitsValues[i] = scaled_number % 10;
        } else if (i < digitsBeforeDecimal) { // Integer part digits (except the last one)
             digitsValues[i] = (scaled_number / (long long int)pow(10, digitsBeforeDecimal - 1 - i + digitsAfterDecimal)) % 10;
        }
         else { // Decimal part digits
             digitsValues[i] = (scaled_number / (long long int)pow(10, totalDigits - 1 - i)) % 10;
        }

        // Basic validation (should ideally not happen with round/abs)
        if (digitsValues[i] < 0 || digitsValues[i] > 9) {
            digitsValues[i] = 0; // Default to 0 if out of range
        }
    }

     // Correct calculation for integer part
    long long int integer_part_val = static_cast<long long int>(number);
    for (int i = digitsBeforeDecimal - 1; i >= 0; --i) {
        digitsValues[i] = integer_part_val % 10;
        integer_part_val /= 10;
    }

    // Correct calculation for decimal part
    long long int decimal_part_val = static_cast<long long int>(round((number - floor(number)) * pow(10, digitsAfterDecimal)));
    for (int i = totalDigits - 1; i >= digitsBeforeDecimal; --i) {
        digitsValues[i] = decimal_part_val % 10;
        decimal_part_val /= 10;
    }

     // Validate digits again after calculation
    for (int i = 0; i < totalDigits; ++i) {
        if (digitsValues[i] < 0 || digitsValues[i] > 9) {
            digitsValues[i] = 0;
        }
    }
}

void constant_x(String unit, int digitsBeforeDecimal, int digitsAfterDecimal, int totalDigits) {
  // State definitions for constant mode editing
  enum CX_EDIT_STATES {
    SELECTING_ITEM, // Selecting digit, trigger, or exit
    MODIFYING_DIGIT // Modifying the selected digit's value
  };

  // Static variables for the state within constant_x
  static std::vector<int> digitsValues(totalDigits, 0); // Digit values initialized to 0
  static int selected_item = 0; // Currently selected item (0..totalDigits-1 = digits, totalDigits = Trigger, totalDigits+1 = Exit)
  static CX_EDIT_STATES edit_state = CX_EDIT_STATES::SELECTING_ITEM; // State machine for editing
  static float current_value = 0.0; // The value being edited (mirrors digitsValues)

  // Check for updates from WebSocket when output is off
  if (ws_value_updated) {
    Serial.printf("constant_x: Detected WS value update to %.3f\n", ws_requested_value);
    current_value = ws_requested_value;
    // Convert the new float value back into the digits array
    number_to_digits(current_value, digitsValues, digitsBeforeDecimal, digitsAfterDecimal, totalDigits);
    ws_value_updated = false; // Consume the update flag
    // If output is active, WS should have updated 'input' directly.
    // If output is inactive, 'input' remains 0, only the display value changes.
  }
  else ws_requested_value = current_value;

  // First time entering this mode, reset static vars
  if (fsm.has_changed()) {
    encoder.set_position(0);
    digitsValues.assign(totalDigits, 0);  // Reset digits values
    selected_item = 0;
    edit_state = CX_EDIT_STATES::SELECTING_ITEM;
    current_value = 0.0;
    input = 0.0; // Ensure target input is 0 initially
    output_active = false; // Ensure output is off initially
    lcd.create_cx_screen(current_value, selected_item, unit);
    // Initial state broadcast happens in loop()
  }

  // Check if encoder button is pressed
  if (encoder.is_button_pressed()) {
    Serial.println("Button pressed - CX Mode");
    Serial.println("CX Edit State: " + String(edit_state) + ", Selected Item: " + String(selected_item));
    switch (edit_state) {
      case CX_EDIT_STATES::SELECTING_ITEM: // Button press selects item
        if (selected_item < totalDigits) { // Selection is a digit -> Start modifying
          edit_state = CX_EDIT_STATES::MODIFYING_DIGIT;
          encoder.set_position(digitsValues[selected_item]); // Set encoder to digit's current value
          encoder.set_min_position(0);
          encoder.set_max_position(9);
          Serial.println("Starting modify digit " + String(selected_item));
        }
        else if (selected_item == totalDigits) { // Trigger/Stop output pressed
          output_active = !output_active; // Toggle output state (global flag)
          if (output_active) {
              input = current_value; // Set target value from edited value
              Serial.println("Output activated: " + String(input, digitsAfterDecimal));
          } else {
              input = 0.0; // Set target to 0
              Serial.println("Output deactivated");
          }
          // State change (output_active) will be broadcast by loop()
        }
        else { // Exit pressed (selected_item == totalDigits + 1)
          Serial.println("Exiting CX mode");
          fsm.change_state(FSM_MAIN_STATES::MAIN_MENU);
          input = 0.0; // Reset input
          output_active = false; // Ensure output is off
          lcd.close_cx_screen();
          // FSM state change will be broadcast by loop()
          return; // Exit function early as state changed
        }
        break;

      case CX_EDIT_STATES::MODIFYING_DIGIT: // Button press confirms digit modification
        Serial.println("Finished modifying digit " + String(selected_item));
        edit_state = CX_EDIT_STATES::SELECTING_ITEM;
        encoder.set_position(selected_item); // Set encoder back to selecting items
        encoder.set_min_position(0);
        encoder.set_max_position(totalDigits - 1 + 2); // Digits + Trigger + Exit
        // Value already updated below, state change (input if active) broadcast by loop()
        break;
    }
    // Button press always causes a state change or action, trigger broadcast via loop check
  }

  // Handle encoder changes based on state
  if (encoder.has_changed()) {
    switch (edit_state) {
      case CX_EDIT_STATES::SELECTING_ITEM:
        selected_item = encoder.get_position();
        break;
      case CX_EDIT_STATES::MODIFYING_DIGIT:
        digitsValues[selected_item] = encoder.get_position(); // Update digit value
        current_value = digits_to_number(digitsValues, digitsBeforeDecimal, digitsAfterDecimal, totalDigits);
        Serial.println("Digit " + String(selected_item) + " changed to " + String(digitsValues[selected_item]) + ", New Value: " + String(current_value));
        // If output is active while modifying, update the target 'input' immediately
        if (output_active) {
            input = current_value; // Update global input
        }
        // Value change will be broadcast by loop()
        break;
    }
  }

  // Update LCD screen - Pass global output_active and current edit state
  lcd.update_cx_screen(current_value, selected_item, unit, dut_voltage, dut_current, digitsBeforeDecimal, totalDigits, String(input, digitsAfterDecimal), output_active, edit_state == CX_EDIT_STATES::MODIFYING_DIGIT);

  // No delay here, rely on loop delay/timing
  // State changes (input, output_active, fsm state) are detected and broadcast in loop()
}

// --- WebSocket Handler ---
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT: {
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      // Send current state to the newly connected client
      client->text(getCurrentStateJson());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      AwsFrameInfo *info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0; // Null-terminate
        Serial.printf("Received WS message from #%u: %s\n", client->id(), (char*)data);

        // Parse JSON command
        StaticJsonDocument<256> doc; // Adjust size as needed
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          client->text("{\"error\":\"Invalid JSON\"}");
          return;
        }

        // Handle the command
        handleWsCommand(client, doc);
      }
      break;
    }
    case WS_EVT_PONG:
      break;
    case WS_EVT_ERROR:
      break;
  }
}

// --- Command Handlers ---
void handleWsCommand(AsyncWebSocketClient *client, JsonDocument& doc) {
  const char* command = doc["command"];
  if (!command) {
    client->text("{\"error\":\"Missing command\"}");
    return;
  }

  if (strcmp(command, "getMeasurements") == 0) handleGetMeasurements(client);
  else if (strcmp(command, "setMode") == 0) handleSetMode(doc);
  else if (strcmp(command, "setValue") == 0) handleSetValue(doc);
  else if (strcmp(command, "setRelay") == 0) handleSetRelay(doc);
  else if (strcmp(command, "exit") == 0) handleExit();
  else client->text("{\"error\":\"Unknown command\"}");

  // After handling command, broadcast the updated state
  broadcastState();
}

void handleGetMeasurements(AsyncWebSocketClient *client) {
  // Measurements are sent periodically via broadcastState,
  // but we can send an immediate update if requested.
  client->text(getCurrentStateJson());
}

void handleSetMode(JsonDocument& doc) {
  const char* modeStr = doc["value"];
  if (!modeStr) return;

  Serial.printf("WS: Setting mode to %s\n", modeStr);
  if (strcmp(modeStr, "CC") == 0) fsm.change_state(FSM_MAIN_STATES::CC);
  else if (strcmp(modeStr, "CV") == 0) fsm.change_state(FSM_MAIN_STATES::CV);
  else if (strcmp(modeStr, "CR") == 0) fsm.change_state(FSM_MAIN_STATES::CR);
  else if (strcmp(modeStr, "CP") == 0) fsm.change_state(FSM_MAIN_STATES::CW);
  // Reset input value when changing mode via WS
  input = 0.0;
  output_active = false; // Turn off output when changing mode
  // Update physical interface if necessary (e.g., LCD screen)
  // Note: The FSM state change should trigger screen updates in the main loop
}

void handleSetValue(JsonDocument& doc) {
  if (!doc["value"].is<float>() && !doc["value"].is<int>()) return;
  float newValue = doc["value"];
  Serial.printf("WS: Setting value to %.3f\n", newValue);
  ws_value_updated = true; // Mark that the value has been updated via WS
  ws_requested_value = newValue; // Store the requested value

  if (output_active) input = newValue; // Update input if output is active
}

void handleSetRelay(JsonDocument& doc) {
  if (!doc["value"].is<bool>()) return;
  
  output_active = doc["value"];
  Serial.printf("WS: Setting relay %s\n", output_active ? "ON" : "OFF");
  if (output_active) {
    input = ws_requested_value; // Set input to the requested value
    analogSws.relay_dut_enable();
  } else {
    input = 0.0; // Set input to 0 when turning off
    analogSws.relay_dut_disable();
  }
}

void handleExit() {
  Serial.println("WS: Exiting current mode to Main Menu");
  fsm.change_state(FSM_MAIN_STATES::MAIN_MENU);
  input = 0.0;
  output_active = false;
  analogSws.relay_dut_disable(); // Physically disable the relay
  ws_delete_main_menu = true; // Set flag to close main menu
  broadcastState(); // Broadcast the state after exiting
}


// --- State Management ---
String getCurrentStateJson() {
  StaticJsonDocument<512> doc; // Adjust size as needed

  // Measurements
  JsonObject measurements = doc.createNestedObject("measurements");
  measurements["voltage"] = dut_voltage;
  measurements["current"] = dut_current;
  measurements["power"] = dut_power;
  measurements["resistance"] = dut_resistance; // Assuming kOhm, adjust if needed
  measurements["temperature"] = temperature;
  measurements["fanSpeed"] = fanSpeed;

  // State
  JsonObject state = doc.createNestedObject("state");
  const char* modeStr = "UNKNOWN";
  switch (fsm.get_current_state()) {
      case FSM_MAIN_STATES::MAIN_MENU: modeStr = "MENU"; break; // Or handle differently
      case FSM_MAIN_STATES::CC: modeStr = "CC"; break;
      case FSM_MAIN_STATES::CV: modeStr = "CV"; break;
      case FSM_MAIN_STATES::CR: modeStr = "CR"; break;
      case FSM_MAIN_STATES::CW: modeStr = "CW"; break;
      case FSM_MAIN_STATES::SETTINGS: modeStr = "SETTINGS"; break; // Add if needed
  }
  state["mode"] = modeStr;
  state["outputActive"] = output_active;
  // Include the current value being set/targeted if applicable
  // This might need refinement based on how 'constant_x' manages its value
  state["value"] = (fsm.get_current_state() >= FSM_MAIN_STATES::CC && fsm.get_current_state() <= FSM_MAIN_STATES::CW) ? ws_requested_value : 0.0; // Send target value if in CX mode

  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

void broadcastState() {
  webServer.notifyClients(getCurrentStateJson());
}