#include "main.h"

Encoder encoder = Encoder();
BuiltInLed led = BuiltInLed();
I2C i2c = I2C();
DAC dac = DAC();
ADC adc = ADC();
AnalogSws analog_sws = AnalogSws();
LVGL_LCD lcd = LVGL_LCD();

// Electronic Load FSM
enum FSMState {
  INIT,
  MAIN_MENU,
  CONSTANT_CURRENT,
  CONSTANT_VOLTAGE,
  CONSTANT_POWER,
  CONSTANT_RESISTANCE,
  ADJUSTMENTS
};
FSMState state = MAIN_MENU;
FSMState lastState = INIT;
int encoderLastPosition = -1;

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

}

void loop() {
  lcd.update();
  fsm();
  delay(10);
}

void fsm(){
  
  switch (state) {
    case MAIN_MENU:
      main_menu();
      break;
    case CONSTANT_CURRENT:
      constant_current();
      break;
    case CONSTANT_VOLTAGE:
      break;
    case CONSTANT_POWER:
      break;
    case CONSTANT_RESISTANCE:
      break;
    case ADJUSTMENTS:
      break;
    default:
      break;
  }

  lastState = state;

}

void main_menu(){

  if (lastState != state){
    encoder.setMinPosition(0);
    encoder.setMaxPosition(ADJUSTMENTS-2); // quantity of options - init
  }

  int pos = encoder.getPosition();

  // Check if encoder button is pressed
  if (encoder.isButtonPressed()) {
    Serial.println("Button pressed");
    switch (pos) {
      case 0:
        state = CONSTANT_CURRENT;
        break;
      case 1:
        state = CONSTANT_VOLTAGE;
        break;
      case 2:
        state = CONSTANT_POWER;
        break;
      case 3:
        state = CONSTANT_RESISTANCE;
        break;
      case 4:
        state = ADJUSTMENTS;
        break;
      default:
        break;
    }
    // Reset vars and close main menu
    lcd.close_main_menu();
    encoder.setPosition(0); // Reset encoder position
    encoderLastPosition = -1; // Reset encoder position
    return;
  }

  // Print menu if encoder is moved
  if (pos == encoderLastPosition) return;
  encoderLastPosition = pos;
  
  // Print main menu with selected option
  lcd.print_main_menu(pos);
}

void constant_current(){

  enum CCState {
    SELECTING_DIGIT,
    MODIFYING_DIGIT,
    TRIGGER_OUTPUT,
    EXIT
  }; 

  // Digits for constant current
  static int digits[TOTAL_DIGITS] = {0};  // Valores de los dígitos
  static int digit_selected = 0; // Dígito seleccionado
  static CCState state = SELECTING_DIGIT; // Estado de la máquina de estados

  int pos = encoder.getPosition(); 

  // Check if encoder button is pressed
  if (encoder.isButtonPressed()) {
    switch (state) {
      case SELECTING_DIGIT:
        state = MODIFYING_DIGIT;
        encoder.setPosition(digits[digit_selected]);
        break;
      case MODIFYING_DIGIT:
        encoder.setPosition(digit_selected);
        digit_selected = 0; // Reset digit selected
        state = SELECTING_DIGIT;
        break;
      case TRIGGER_OUTPUT:
        state = SELECTING_DIGIT; // @todo: Implement trigger output
        break;
      case EXIT:
        state = SELECTING_DIGIT; // @todo: Implement exit
        break;
      default:
        break;
    }
  }

  // Print constant current menu if encoder is moved
  if (pos == encoderLastPosition) return;
  encoderLastPosition = pos;

  switch (state) {
    case SELECTING_DIGIT:
      encoder.setMaxPosition(TOTAL_DIGITS - 1);
      digit_selected = pos;
      break;
    case MODIFYING_DIGIT:
      encoder.setMaxPosition(9); // 0-9
      digits[digit_selected] = pos;
      break;
    case TRIGGER_OUTPUT:
      break;
    case EXIT:
      break;
    default:
      break;
  }


 
  // Validación de cada elemento para que esté en el rango [0, 9]
  for (int i = 0; i < TOTAL_DIGITS; i++) {
    if (digits[i] < 0) {
      digits[i] = 0;
    } else if (digits[i] > 9) {
      digits[i] = 9;
    }
  }

  // Convert digits to float
  float current = 0.0;
  // Calcula la parte entera
  for (int i = 0; i < DIGITS_BEFORE_DECIMAL; i++) {
    current += digits[i] * pow(10, DIGITS_BEFORE_DECIMAL - i - 1); // Suma dígitos antes del decimal
  }
  // Calcula la parte decimal
  for (int i = 0; i < DIGITS_AFTER_DECIMAL; i++) {
    current += digits[DIGITS_BEFORE_DECIMAL + i] * pow(10, -i - 1); // Suma dígitos después del decimal
  }

  Serial.println("Current: " + String(current, DIGITS_AFTER_DECIMAL));

  lcd.print_cx_screen(current, digit_selected, "A");
}