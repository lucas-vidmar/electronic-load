#include "main.h"
#include "webserver.h"
#include "I2CScanner.h"
#include <SPIFFS.h>

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
// PID controllers with tuning parameters
PIDFanController pidController(fan, PID_KP, PID_KI, PID_KD);
float input = 0.0;

// RTC instance
RTC rtc = RTC();

// Start time in milliseconds
uint64_t startTimeMs = 0;

I2CScanner scanner;

// Electronic Load FSM
FSM fsm = FSM();

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

  // Iniciar el servidor web
  Serial.println("Iniciando servidor web...");
  webServer.set_default_file("index.html");
  webServer.begin();
}

void loop() {
  lcd.update();
  delay(10);
  fsm.run(input, dac, analogSws);
  float currentTemp = adc.read_temperature(); // Read temperature from ADC
    
  // Compute and adjust fan speed based on temperature
  pidController.compute(currentTemp);
}

void main_menu() {
  static int pos = 0;

  if (fsm.has_changed()) { // First time entering main menu
    Serial.println("Main menu");
    encoder.set_min_position(0);
    encoder.set_max_position(FSM_MAIN_STATES::SETTINGS - FSM_MAIN_STATES::CC); //  quantity of options in main menu
    encoder.set_position(0);
    lcd.create_main_menu(); // Create main menu
    lcd.update_main_menu(0);
  }

  if (encoder.has_changed()) { // Update menu with selected option
    pos = encoder.get_position();
    lcd.update_main_menu(pos);
    //#define DEBUG_ENCODER
    #ifdef DEBUG_ENCODER
    Serial.println("Encoder position: " + String(encoder.get_position()));
    Serial.println("Max:" + String(encoder.get_encoder_max_position()) + " - Min: " + String(encoder.get_encoder_min_position()));
    #endif
  }

  // Check if encoder button is pressed
  if (encoder.is_button_pressed()) {
    Serial.println("Button pressed");
    fsm.change_state(FSM_MAIN_STATES::CC + pos); // Change to selected option
    // Reset vars and close main menu
    Serial.println("Exiting main menu to option " + String(FSM_MAIN_STATES::CC + pos));
    lcd.close_main_menu();
    return;
  }
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

void constant_x(String unit, int digitsBeforeDecimal, int digitsAfterDecimal, int totalDigits) {
  // State definitions for constant mode
  enum CX_STATES {
    SELECTING,
    MODIFYING_DIGIT,
    TRIGGER_OUTPUT,
    EXIT
  };

  // Read DUT voltage and current
  float vDUT = adc.read_vDUT();
  float iDUT = adc.read_iDUT();
  //Serial.println("vDUT: " + String(vDUT, 3) + " V");
  //Serial.println("iDUT: " + String(iDUT, 3) + " A");

  // Digits for constant value
  static std::vector<int> digitsValues(totalDigits, 0); // Digit values initialized to 0
  static int selected = 0; // Currently selected option
  static CX_STATES state = CX_STATES::SELECTING; // State machine state
  static float value = 0.0; // Current value

  // First time entering this mode, reset static vars
  if (fsm.has_changed()) {
    encoder.set_position(0);
    digitsValues.assign(totalDigits, 0);  // Reset digits values
    selected = 0;
    state = CX_STATES::SELECTING;
    value = 0.0;
    lcd.create_cx_screen(value, selected, unit);
  }

  // Check if encoder button is pressed
  if (encoder.is_button_pressed()) {
    Serial.println("Button pressed");
    Serial.println("State: " + String(state));
    switch (state) {
      case CX_STATES::SELECTING: // Start modifying digit
        Serial.println("selected: " + String(selected));
        if (selected < totalDigits) { // Selection is a digit
          state = CX_STATES::MODIFYING_DIGIT;
          encoder.set_position(digitsValues[selected]); // Set encoder position to the value of the selected digit
        } else if (selected == totalDigits) { // Trigger output
          state = CX_STATES::TRIGGER_OUTPUT;
        }
        else { // Exit
          state = CX_STATES::EXIT;
        }
        break;
      case CX_STATES::MODIFYING_DIGIT: // Modify digit
        Serial.println("Modifying digit");
        encoder.set_position(selected);
        selected = 0; // Reset digit selected
        state = CX_STATES::SELECTING;
        Serial.println("Value: " + String(value, digitsAfterDecimal));
        break;
      default:
        break;
    }
    switch (state) { // Check if trigger output or exit is pressed
      case CX_STATES::TRIGGER_OUTPUT: // Trigger output and return to selecting digit
        input = value;
        Serial.println("Output activated: " + String(input, digitsAfterDecimal));
        state = CX_STATES::SELECTING;
        break;
      case CX_STATES::EXIT: // Exit mode
        fsm.change_state(FSM_MAIN_STATES::MAIN_MENU);
        input = 0.0; // Reset input
        lcd.close_cx_screen();
        return;
      default:
        break;
    }
  }

  switch (state) {
    case CX_STATES::SELECTING:
      encoder.set_min_position(0);
      encoder.set_max_position(totalDigits - 1 + 2); // -1 because it starts from 0, +2 for trigger output and exit
      selected = encoder.get_position();
      break;
    case CX_STATES::MODIFYING_DIGIT:
      encoder.set_min_position(0);
      encoder.set_max_position(9); // 0-9
      digitsValues[selected] = encoder.get_position(); // Set digit value to encoder position
      break;
    case CX_STATES::TRIGGER_OUTPUT: // should not reach this state
    case CX_STATES::EXIT: // should not reach this state
    default:
      break;
  }

  value = digits_to_number(digitsValues, digitsBeforeDecimal, digitsAfterDecimal, totalDigits);

  // Print constant x screen
  lcd.update_cx_screen(value, selected, unit, vDUT, iDUT, digitsBeforeDecimal, totalDigits, String(input, digitsAfterDecimal));

  delay(15);
}