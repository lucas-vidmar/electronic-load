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
  }

  // Print menu if encoder is moved
  if (encoder.getPosition() == encoderLastPosition) return;
  encoderLastPosition = encoder.getPosition();
  
  // Print main menu with selected option
  //if (pos == 0) pos == -1; // To avoid highlighting the first option
  lcd.print_main_menu(pos);

}

void constant_current(){
  Serial.println("Constant Current Mode");
}