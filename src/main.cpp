#include "main.h"

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
I2CScanner scanner;
FSM fsm = FSM();


// --- Global Variables for State Management ---
float input = 0.0;
float dutVoltage = 0.0; // Track DUT voltage
float dutCurrent = 0.0; // Track DUT current
float dutPower = 0.0;   // Track DUT power
float dutResistance = 0.0; // Track DUT resistance
float dutEnergy = 0.0;  // Track DUT energy

bool outputActive = false; // Track output state
float temperature = 0.0;   // Track temperature
int fanSpeed = 0;          // Get current fan speed percentage

uint64_t timeMs = 0;       // Track RTC time
String uptimeString = "00:00:00"; // Track uptime string

// --- Global Variables for WS/UI Sync ---
bool wsDeleteMainMenu = false; // Track if in main menu

// Helper function to format uptime
String format_uptime(uint64_t ms) {
  uint32_t totalSeconds = ms / 1000;
  uint8_t seconds = totalSeconds % 60;
  uint32_t totalMinutes = totalSeconds / 60;
  uint8_t minutes = totalMinutes % 60;
  uint32_t hours = totalMinutes / 60;
  char buf[10]; // hh:mm:ss\0
  // Format as HHH:MM:SS if hours > 99, otherwise HH:MM:SS
  if (hours > 99) {
    sprintf(buf, "%03lu:%02u:%02u", hours, minutes, seconds);
  } else {
    sprintf(buf, "%02lu:%02u:%02u", hours, minutes, seconds);
  }
  return String(buf);
}

void setup() {
  Serial.begin(115200);

  Serial.println("[MAIN] Starting Electronic Load System...");

  // I2C Scanner
  Serial.println("[MAIN] Initializing I2C Scanner...");
  scanner.Init();
  scanner.Scan();

  // Initialize SPIFFS
  Serial.println("[MAIN] Initializing SPIFFS...");
  if (!SPIFFS.begin(true)) {
    Serial.println("[MAIN] ERROR: Failed to mount SPIFFS");
    return;
  }
  Serial.println("[MAIN] SPIFFS mounted successfully");

  // Initialize encoder
  Serial.println("[MAIN] Initializing encoder...");
  encoder.init();
  // Initialize builtin led
  Serial.println("[MAIN] Initializing built-in LED...");
  led.init();
  led.blink(500);
  // Initialize relays
  Serial.println("[MAIN] Initializing analog switches and relays...");
  analogSws.init();
  analogSws.relay_dut_disable();
  analogSws.mosfet_input_cc_mode();
  analogSws.v_dac_enable();
  // Initialize I2C devices
  Serial.println("[MAIN] Initializing I2C devices...");
  i2c.init();
  dac.init(&i2c);
  adc.init(&i2c);
  // Initialize RTC
  Serial.println("[MAIN] Initializing RTC...");
  rtc.init(&i2c);
  
  // Set current date and time (example: January 1, 2025 at 12:00:00)
  DateTime currentTime = {0, 0, 12, 1, 1, 1, 25};  // seconds, minutes, hours, dayOfWeek, date, month, year
  rtc.set_time(currentTime);
  
  // Store start time
  Serial.println("[MAIN] RTC initialized. Start time recorded for uptime tracking.");
  
  // Initialize LVGL Display
  Serial.println("[MAIN] Initializing LVGL LCD...");
  lcd.init();
  // Initialize FANs and PID controllers
  Serial.println("[MAIN] Initializing fan control and PID controller...");
  pidController.init(PID_SETPOINT); // Set target temperature for fan
  fan.set_speed(0); // Set initial speed to 0
  // Initialize FSM
  Serial.println("[MAIN] Initializing Finite State Machine...");
  fsm.init();

  // Start web server and WebSocket
  Serial.println("[MAIN] Starting web server and WebSocket...");
  webServer.set_default_file("index.html");
  webServer.attachWsHandler(on_ws_event); // Attach the WebSocket handler
  webServer.begin();

  // Initial relay state
  analogSws.relay_dut_disable();
  outputActive = false; // Ensure output is off
  Serial.println("[MAIN] System initialization completed successfully");
}

void loop() {
  // Handle WebSocket clients
  webServer.cleanupClients(); // Important for AsyncWebServer

  // Update all global variables
  fanSpeed = fan.get_speed_percentage(); // Get current fan speed percentage
  temperature = adc.read_temperature(); // Read temperature from ADC
  dutVoltage = adc.read_v_dut(); // Read DUT voltage
  dutCurrent = adc.read_i_dut(); // Read DUT current
  dutPower = dutVoltage * dutCurrent; // Calculate DUT power
  dutResistance = (dutCurrent != 0) ? (dutVoltage / dutCurrent) : 0; // Calculate DUT resistance

  // Safety monitoring - check if DUT readings exceed safe levels
  check_safety_limits();

  lcd.update();
  lcd.update_header(temperature, fanSpeed, uptimeString.c_str()); // Update header with temperature, fan speed, and uptime

  // Store previous state to detect changes
  FSM_MAIN_STATES prevState = fsm.get_current_state();
  float prevInput = input;
  bool prevOutputActive = outputActive;

  // Run FSM which might change state or apply 'input'
  fsm.run(input, dac, analogSws, &outputActive, adc);

  // Compute and adjust fan speed based on temperature
  pidController.compute(temperature);

  unsigned long lastBroadcastTime = 0;

  // Broadcast state periodically or if changed
  unsigned long currentTime = millis();
  bool stateChanged = (fsm.get_current_state() != prevState) ||
                      (input != prevInput) ||
                      (outputActive != prevOutputActive);

  // Log state changes
  if (fsm.get_current_state() != prevState) {
    Serial.printf("[MAIN] FSM state changed from %d to %d\n", prevState, fsm.get_current_state());
  }
  
  if (outputActive != prevOutputActive) {
    Serial.printf("[MAIN] Output state changed: %s\n", outputActive ? "ENABLED" : "DISABLED");
  }

  if (stateChanged || (currentTime - lastBroadcastTime >= BROADCAST_INTERVAL)) {
    broadcast_state();
    lastBroadcastTime = currentTime;
  }

  if (wsDeleteMainMenu) {
    Serial.println("[MAIN] Closing CX screen due to WebSocket request");
    lcd.close_cx_screen(); // Close CX screen if in main menu, moved here because it was taking too long to close
    wsDeleteMainMenu = false; // Reset the flag
  }

  static uint64_t lastMillis = 0;
  static uint64_t lastStatusLog = 0;
  uint64_t currentMillis = rtc.get_timestamp_ms();
  
  // Periodic system status logging (every 30 seconds)
  if (currentMillis - lastStatusLog >= 30000) {
    Serial.printf("[STATUS] Temp: %.1f°C, Fan: %d%%, V: %.3fV, I: %.3fA, P: %.3fW\n", 
                  temperature, fanSpeed, dutVoltage, dutCurrent, dutPower);
    lastStatusLog = currentMillis;
  }
  
  if (currentMillis - lastMillis >= 1000) { // Update every second
    lastMillis = currentMillis;
    if (outputActive) {
      dutEnergy += dutPower / 1000; // Update DUT energy
      timeMs += 1000;
      uptimeString = format_uptime(timeMs); // Format uptime string
    }
  }
  // Short delay to prevent busy-waiting
  delay(10);
}

void main_menu() {
  static int pos = 0;
  timeMs = 0.0; // Reset time when entering main menu
  uptimeString = format_uptime(timeMs); // Format uptime string
  dutEnergy = 0.0; // Reset DUT energy when entering main menu

  if (fsm.has_changed()) { // First time entering main menu
    #ifdef DEBUG_ENCODER
    Serial.println("[MAIN_MENU] Encoder position: " + String(encoder.get_position()));
    Serial.println("[MAIN_MENU] Max:" + String(encoder.get_encoder_max_position()) + " - Min: " + String(encoder.get_encoder_min_position()));
    #endif

    Serial.println("[MAIN_MENU] Entering main menu");
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
  }

  // Check if encoder button is pressed
  if (encoder.is_button_pressed()) {
    Serial.println("[MAIN_MENU] Button pressed - Main Menu Selection");
    fsm.change_state(static_cast<FSM_MAIN_STATES>(FSM_MAIN_STATES::CC + pos)); // Change to selected option
    // Reset vars and close main menu
    Serial.println("[MAIN_MENU] Exiting main menu to option " + String(FSM_MAIN_STATES::CC + pos));
    lcd.close_main_menu();
    input = 0.0; // Reset input when changing mode
    outputActive = false; // Ensure output is off
    // No return here, let loop handle broadcast
  }

  // If state changed relevant to WS, broadcast
  // Note: FSM change is handled in loop(), but menu selection itself isn't broadcasted yet.
  // We rely on the FSM state change broadcast in loop().
}

void setting() {
  // Settings menu logic
  if (fsm.has_changed()) { // First time entering settings
    Serial.println("[SETTINGS] Entering settings menu");
    encoder.set_min_position(0);
    encoder.set_max_position(0); //  quantity of options in settings menu
    encoder.set_position(0);
    lcd.create_settings_menu(); // Create settings menu
  }

  // Check if encoder button is pressed
  if (encoder.is_button_pressed()) {
    Serial.println("[SETTINGS] Button pressed - Back to Main Menu");
    fsm.change_state(FSM_MAIN_STATES::MAIN_MENU); // Change to selected main menu
    lcd.close_settings_menu();
  }

}

int get_digit_value(int selected_item, int digitsBeforeDecimal, int digitsAfterDecimal, float input) {
  int digit_value = 0;
  if (selected_item < digitsBeforeDecimal) { // Before decimal point
    int pow10 = pow(10, digitsBeforeDecimal - selected_item - 1);
    digit_value = static_cast<int>(input / pow10) % 10;
  } else { // After decimal point
    int dec_pos = selected_item - digitsBeforeDecimal + 1;
    digit_value = static_cast<int>(input * pow(10, dec_pos)) % 10;
  }
  return digit_value;
}

void constant_x(String unit, int digitsBeforeDecimal, int digitsAfterDecimal, int totalDigits, int maxInputValue) {
  enum CX_EDIT_STATES {
    SELECTING_ITEM,
    MODIFYING_DIGIT
  };

  static int selected_item = 0;
  static CX_EDIT_STATES edit_state = CX_EDIT_STATES::SELECTING_ITEM;

  if (fsm.has_changed()) {
    timeMs = 0.0;
    uptimeString = format_uptime(timeMs);
    dutEnergy = 0.0;
    encoder.set_position(0);
    selected_item = 0;
    edit_state = CX_EDIT_STATES::SELECTING_ITEM;
    input = 0.0;
    outputActive = false;
    encoder.set_min_position(0);
    encoder.set_max_position(totalDigits + 1);
    lcd.create_cx_screen(selected_item, unit);
  }

  if (encoder.is_button_pressed()) {
    Serial.println("[CX_MODE] Button pressed - CX Mode");
    Serial.println("[CX_MODE] Edit State: " + String(edit_state) + ", Selected Item: " + String(selected_item));
    switch (edit_state) {
      case CX_EDIT_STATES::SELECTING_ITEM:
        if (selected_item < totalDigits) {
          edit_state = CX_EDIT_STATES::MODIFYING_DIGIT; // Enter digit modification mode
          // Set encoder to current digit value (calculate from input)
          encoder.set_position(0);
          encoder.set_min_position(-1); // negative decreses
          encoder.set_max_position(1); // positive increases
          Serial.println("[CX_MODE] Starting modify digit " + String(selected_item));
        } else if (selected_item == totalDigits) {
          outputActive = !outputActive;
          if (outputActive) {
            Serial.println("[CX_MODE] Output activated: " + String(input, digitsAfterDecimal));
          } else {
            Serial.println("[CX_MODE] Output deactivated");
          }
        } else {
          Serial.println("[CX_MODE] Exiting CX mode");
          fsm.change_state(FSM_MAIN_STATES::MAIN_MENU);
          input = 0.0;
          outputActive = false;
          lcd.close_cx_screen();
          return;
        }
        break;
      case CX_EDIT_STATES::MODIFYING_DIGIT: // Exit digit modification mode
        Serial.println("[CX_MODE] Finished modifying digit " + String(selected_item));
        edit_state = CX_EDIT_STATES::SELECTING_ITEM;
        encoder.set_position(selected_item);
        encoder.set_min_position(0);
        encoder.set_max_position(totalDigits - 1 + 2);
        break;
    }
  }

  if (encoder.has_changed()) {
    #ifdef DEBUG_ENCODER
    Serial.println("[CX_MODE] Encoder position: " + String(encoder.get_position()));
    Serial.println("[CX_MODE] Max:" + String(encoder.get_encoder_max_position()) + " - Min: " + String(encoder.get_encoder_min_position()));
    #endif

    switch (edit_state) {
      case CX_EDIT_STATES::SELECTING_ITEM:
        selected_item = encoder.get_position();
        break;
      case CX_EDIT_STATES::MODIFYING_DIGIT: {
        int encValue = encoder.get_position(); // can be -1, 0, or 1
        input += encValue * pow(10, digitsBeforeDecimal - selected_item - 1); // Adjust input based on selected item
        encoder.set_position(0); // Reset encoder position
        encoder.set_position(0); // Twice to also reset has_changed
        
        if (input < 0) input = 0;

        Serial.println("[CX_MODE] Digit " + String(selected_item) + " changed | New Value: " + String(input));
        break;
      }
    }
  }

  if (input > maxInputValue) {
    input = maxInputValue;
    Serial.println("[CX_MODE] Input clamped to max value: " + String(maxInputValue));
    lcd.show_warning_popup("Limit: " + String(maxInputValue) + unit, 2000);
  }

  lcd.update_cx_screen(input, selected_item, unit, dutVoltage, dutCurrent, digitsBeforeDecimal, totalDigits, String(input, digitsAfterDecimal), outputActive, (edit_state == CX_EDIT_STATES::MODIFYING_DIGIT), temperature, dutEnergy);
}

// --- WebSocket Handler ---
void on_ws_event(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT: {
      Serial.printf("[WEBSOCKET] Client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      // Send current state to the newly connected client
      client->text(get_current_state_json());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("[WEBSOCKET] Client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      AwsFrameInfo *info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0; // Null-terminate
        Serial.printf("[WEBSOCKET] Received message from client #%u: %s\n", client->id(), (char*)data);

        // Parse JSON command
        StaticJsonDocument<256> doc; // Adjust size as needed
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
          Serial.print("[WEBSOCKET] ERROR: deserializeJson() failed: ");
          Serial.println(error.f_str());
          client->text("{\"error\":\"Invalid JSON\"}");
          return;
        }

        // Handle the command
        handle_ws_command(client, doc);
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
void handle_ws_command(AsyncWebSocketClient *client, JsonDocument& doc) {
  const char* command = doc["command"];
  if (!command) {
    client->text("{\"error\":\"Missing command\"}");
    return;
  }

  if (strcmp(command, "getMeasurements") == 0) handle_get_measurements(client);
  else if (strcmp(command, "setMode") == 0) handle_set_mode(doc);
  else if (strcmp(command, "setValue") == 0) handle_set_value(doc);
  else if (strcmp(command, "setRelay") == 0) handle_set_relay(doc);
  else if (strcmp(command, "exit") == 0) handle_exit();
  else client->text("{\"error\":\"Unknown command\"}");

  // After handling command, broadcast the updated state
  broadcast_state();
}

void handle_get_measurements(AsyncWebSocketClient *client) {
  // Measurements are sent periodically via broadcastState,
  // but we can send an immediate update if requested.
  client->text(get_current_state_json());
}

void handle_set_mode(JsonDocument& doc) {
  const char* modeStr = doc["value"];
  if (!modeStr) return;

  Serial.printf("[WEBSOCKET] Setting mode to %s\n", modeStr);
  if (strcmp(modeStr, "CC") == 0) fsm.change_state(FSM_MAIN_STATES::CC);
  else if (strcmp(modeStr, "CV") == 0) fsm.change_state(FSM_MAIN_STATES::CV);
  else if (strcmp(modeStr, "CR") == 0) fsm.change_state(FSM_MAIN_STATES::CR);
  else if (strcmp(modeStr, "CW") == 0) fsm.change_state(FSM_MAIN_STATES::CW);
  // Reset input value when changing mode via WS
  input = 0.0;
  outputActive = false; // Turn off output when changing mode
  // Update physical interface if necessary (e.g., LCD screen)
  // Note: The FSM state change should trigger screen updates in the main loop
}

void handle_set_value(JsonDocument& doc) {
  if (!doc["value"].is<float>() && !doc["value"].is<int>()) return;
  float newValue = doc["value"];
  Serial.printf("[WEBSOCKET] Setting value to %.3f\n", newValue);
  input = newValue;
}

void handle_set_relay(JsonDocument& doc) {
  if (!doc["value"].is<bool>()) return;
  
  outputActive = doc["value"];
  Serial.printf("[WEBSOCKET] Setting relay %s\n", outputActive ? "ON" : "OFF");
  if (outputActive) analogSws.relay_dut_enable();
  else analogSws.relay_dut_disable();
}

void handle_exit() {
  Serial.println("[WEBSOCKET] Exiting current mode to Main Menu");
  fsm.change_state(FSM_MAIN_STATES::MAIN_MENU);
  input = 0.0;
  outputActive = false;
  analogSws.relay_dut_disable(); // Physically disable the relay
  wsDeleteMainMenu = true; // Set flag to close main menu
  broadcast_state(); // Broadcast the state after exiting
}


// --- State Management ---
String get_current_state_json() {
  StaticJsonDocument<512> doc; // Adjust size as needed

  // Measurements
  JsonObject measurements = doc.createNestedObject("measurements");
  measurements["voltage"] = dutVoltage;
  measurements["current"] = dutCurrent;
  measurements["power"] = dutPower;
  measurements["resistance"] = dutResistance; // Assuming kOhm, adjust if needed
  measurements["temperature"] = temperature;
  measurements["fanSpeed"] = fanSpeed;
  measurements["uptime"] = uptimeString; // Uptime string
  measurements["energy"] = dutEnergy; // Energy in kJ

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
  state["outputActive"] = outputActive;
  state["value"] = input;

  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

void broadcast_state() {
  webServer.notifyClients(get_current_state_json());
}

// --- Safety Monitoring ---
bool check_safety_limits() {
  bool limitExceeded = false;
  String alertMessage = "";

  // Check voltage limit
  if (dutVoltage > SAFETY_MAX_VOLTAGE) {
    limitExceeded = true;
    alertMessage = "VOLTAGE LIMIT EXCEEDED: " + String(dutVoltage, 3) + "V > " + String(SAFETY_MAX_VOLTAGE, 1) + "V";
  }
  
  // Check current limit
  if (dutCurrent > SAFETY_MAX_CURRENT) {
    limitExceeded = true;
    alertMessage = "CURRENT LIMIT EXCEEDED: " + String(dutCurrent, 3) + "A > " + String(SAFETY_MAX_CURRENT, 1) + "A";
  }
  
  // Check power limit
  if (dutPower > SAFETY_MAX_POWER) {
    limitExceeded = true;
    alertMessage = "POWER LIMIT EXCEEDED: " + String(dutPower, 3) + "W > " + String(SAFETY_MAX_POWER, 1) + "W";
  }
  
  // Check temperature limit
  if (temperature > SAFETY_MAX_TEMPERATURE) {
    limitExceeded = true;
    alertMessage = "TEMPERATURE LIMIT EXCEEDED: " + String(temperature, 1) + "°C > " + String(SAFETY_MAX_TEMPERATURE, 1) + "°C";
  }

  if (limitExceeded && outputActive) {
    Serial.println("[SAFETY] " + alertMessage);
    Serial.println("[SAFETY] Emergency disconnect - DUT disabled for safety");
    
    // Immediately disable output and relay
    outputActive = false;
    analogSws.relay_dut_disable();
    
    // Set input to zero for safety
    input = 0.0;
    dac.cc_mode_set_current(0.0);
    
    // Show warning on LCD if in CX mode
    if (fsm.get_current_state() >= FSM_MAIN_STATES::CC && fsm.get_current_state() <= FSM_MAIN_STATES::CW) {
      lcd.show_warning_popup("SAFETY: " + alertMessage.substring(0, 20), 5000);
    }
    
    // Broadcast updated state
    broadcast_state();
    
    return true;
  }
  
  return false;
}