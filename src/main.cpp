#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "encoder.h"
#include "led.h"
#include "i2c.h"
#include "dac.h"
#include "analog_sws.h"
#include "adc.h"

Encoder encoder = Encoder();
BuiltInLed led = BuiltInLed();
I2C i2c = I2C();
DAC dac = DAC();
ADC adc = ADC();
AnalogSws analog_sws = AnalogSws();

TFT_eSPI tft = TFT_eSPI();

#define LED_BUILTIN 2
#define TFT_BL 27

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.println("Carga electronica");

  // turn on backlight
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  encoder.init();

  led.init();
  led.blink(500);

  analog_sws.init();
  //analog_sws.relayDutEnable();
  analog_sws.vdacDisable();

  i2c.init();
  
  //dac.init(&i2c);
  //dac.set_voltage(200); // Set DAC output to 200mV

  adc.init(&i2c);

}

void loop() {
  static unsigned long lastTime = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastTime >= 16) { // approximately 60Hz
    lastTime = currentTime;

    // write in display
    tft.setCursor(0, 20);
    // clean line
    tft.fillRect(0, 20, 240, 20, TFT_BLACK);
    tft.print("Encoder: " + String(encoder.getPosition())); 

    tft.setCursor(0, 40);
    tft.fillRect(0, 40, 240, 20, TFT_BLACK);
    tft.print("Current: " + String(adc.read_iDUT()) + " A");

    tft.setCursor(0, 60);
    tft.fillRect(0, 60, 240, 20, TFT_BLACK);
    tft.print("Voltage: " + String(adc.read_vDUT()) + " V");

    tft.setCursor(0, 80);
    tft.fillRect(0, 80, 240, 20, TFT_BLACK);
    tft.print("Temperature: " + String(adc.read_temperature()) + " C");

  }
}