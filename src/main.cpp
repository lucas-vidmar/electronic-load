#include "main.h"

Encoder encoder = Encoder();
BuiltInLed led = BuiltInLed();
I2C i2c = I2C();
DAC dac = DAC();
ADC adc = ADC();
AnalogSws analog_sws = AnalogSws();
LVGL_LCD lcd = LVGL_LCD();
float input = 0.0;

// Electronic Load FSM
FSM fsm = FSM();

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  // Initialize encoder
  encoder.init();
  // Initialize builtin led
  led.init();
  led.blink(500);
  // Initialize relays
  analog_sws.init();
  analog_sws.relayDUTDisable();
  analog_sws.mosfetInputCCMode();
  analog_sws.vDACDisable();
  // Initialize I2C devices
  i2c.init();
  dac.init(&i2c);
  adc.init(&i2c);
  // Initialize LVGL Display
  lcd.init();
  // Initialize FSM
  fsm.init();
}

void loop() {
  lcd.update();
  delay(10);
  fsm.run();
}

void main_menu(){

  static int pos = 0;

  if (fsm.hasChanged()) { // First time entering main menu
    Serial.println("Main menu");
    encoder.setMinPosition(0);
    encoder.setMaxPosition(FSM_MAIN_STATES::SETTINGS - FSM_MAIN_STATES::CC); //  quantity of options in main menu
    encoder.setPosition(0);
    lcd.print_main_menu(0);
  }

  if (encoder.hasChanged()) { // Update menu with selected option
    Serial.println("Has changed");
    pos = encoder.getPosition();
    lcd.print_main_menu(pos);
  }

  // Check if encoder button is pressed
  if (encoder.isButtonPressed()) {
    Serial.println("Button pressed");
    fsm.changeState(FSM_MAIN_STATES::CC + pos); // Change to selected option
    // Reset vars and close main menu
    Serial.println("Exiting main menu to option " + String(FSM_MAIN_STATES::CC + pos));
    lcd.close_main_menu();
    return;
  }
}

float digits_to_number(int digits_values[], int digits_before_decimal, int digits_after_decimal) {
  float number = 0.0;

  // Validación de cada elemento para que esté en el rango [0, 9]
  for (int i = 0; i < CC_TOTAL_DIGITS; i++) {
    if (digits_values[i] < 0) {
      digits_values[i] = 0;
    } else if (digits_values[i] > 9) {
      digits_values[i] = 9;
    }
  }

  // Calcula la parte entera
  for (int i = 0; i < CC_DIGITS_BEFORE_DECIMAL; i++) {
    number += digits_values[i] * pow(10, CC_DIGITS_BEFORE_DECIMAL - i - 1); // Suma dígitos antes del decimal
  }
  // Calcula la parte decimal
  for (int i = 0; i < CC_DIGITS_AFTER_DECIMAL; i++) {
    number += digits_values[CC_DIGITS_BEFORE_DECIMAL + i] * pow(10, -i - 1); // Suma dígitos después del decimal
  }
  return number;
}

void constant_x(String unit){

  enum CC_STATES {
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
  static int digits_values[CC_TOTAL_DIGITS] = {0};  // Valores de los dígitos
  static int selected_digit = 0; // Dígito seleccionado
  static CC_STATES state = CC_STATES::SELECTING; // Estado de la máquina de estados
  static float value = 0.0;

  // First time entering this mode, reset static vars
  if (fsm.hasChanged()) {
    memset(digits_values, 0, sizeof(digits_values));
    selected_digit = 0;
    state = CC_STATES::SELECTING;
    value = 0.0;
  }

  // Check if encoder button is pressed
  if (encoder.isButtonPressed()) {
    Serial.println("Button pressed");
    Serial.println("State: " + String(state));
    switch (state) {
      case CC_STATES::SELECTING: // Start modifying digit
        Serial.println("Selecting digit: " + String(selected_digit));
        if (selected_digit < CC_TOTAL_DIGITS) {
          state = CC_STATES::MODIFYING_DIGIT;
          encoder.setPosition(digits_values[selected_digit]); // Set encoder position to the value of the selected digit
        } else if (selected_digit == CC_TOTAL_DIGITS) {
          state = CC_STATES::TRIGGER_OUTPUT;
        }
        else {
          state = CC_STATES::EXIT;
        }
        break;
      case CC_STATES::MODIFYING_DIGIT: // Modify digit
        Serial.println("Modifying digit");
        encoder.setPosition(selected_digit);
        selected_digit = 0; // Reset digit selected
        state = CC_STATES::SELECTING;
        Serial.println("Value: " + String(value, CC_DIGITS_AFTER_DECIMAL));
        break;
      default:
        break;
    }
    switch (state) { // Check if trigger output or exit is pressed
      case CC_STATES::TRIGGER_OUTPUT: // Trigger output and return to selecting digit
        input = value;
        Serial.println("Output activated: " + String(input, CC_DIGITS_AFTER_DECIMAL));
        state = CC_STATES::SELECTING;
        break;
      case CC_STATES::EXIT: // Exit mode
        fsm.changeState(FSM_MAIN_STATES::MAIN_MENU);
        input = 0.0; // Reset input
        lcd.close_cx_screen();
        return;
      default:
        break;
    }
  }

  switch (state) {
    case CC_STATES::SELECTING:
      encoder.setMinPosition(0);
      encoder.setMaxPosition(CC_TOTAL_DIGITS - 1 + 2); // -1 because it starts from 0, +2 for trigger output and exit
      selected_digit = encoder.getPosition();
      break;
    case CC_STATES::MODIFYING_DIGIT:
      encoder.setMaxPosition(9); // 0-9
      digits_values[selected_digit] = encoder.getPosition();
      break;
    case CC_STATES::TRIGGER_OUTPUT: // if trigger output is hovered, do nothing
    case CC_STATES::EXIT: // if exit is hovered, do nothing
    default:
      break;
  }

  value = digits_to_number(digits_values, CC_DIGITS_BEFORE_DECIMAL, CC_DIGITS_AFTER_DECIMAL);

  // Print constant x screen
  lcd.print_cx_screen(value, selected_digit, CC_TOTAL_DIGITS, unit, vDUT, iDUT, CC_DIGITS_BEFORE_DECIMAL, CC_TOTAL_DIGITS, String(input, CC_DIGITS_AFTER_DECIMAL));

  delay(15);
}