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
  tft.println("Carga electrónica");

  // turn on backlight
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  encoder.init();

  led.init();
  led.blink(500);

  analog_sws.init();
  analog_sws.relayDutEnable();
  analog_sws.mosfetInputCCMode();
  analog_sws.vdacEnable();

  i2c.init();
  
  dac.init(&i2c);

  adc.init(&i2c);

}

void loop() {
  static unsigned long lastTime = 0;
  static int encoderSet = 0;
  static int digitalSet = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastTime >= 16) { // approximately 60Hz
    lastTime = currentTime;

    // write in display
    tft.setCursor(0, 20);
    tft.fillRect(0, 20, 240, 20, TFT_BLACK);
    tft.print("Encoder: " + String(encoder.getPosition())); 

    tft.setCursor(0, 40);
    tft.print("-------DUT-------");

    tft.setCursor(0, 60);
    tft.fillRect(0, 60, 240, 20, TFT_BLACK);
    tft.print("iDUT: " + String(adc.read_iDUT(), 3) + " A");

    tft.setCursor(0, 80);
    tft.fillRect(0, 80, 240, 20, TFT_BLACK);
    tft.print("vDUT: " + String(adc.read_vDUT(), 3) + " V");

    tft.setCursor(0, 100);
    tft.fillRect(0, 100, 240, 20, TFT_BLACK);
    tft.print("Temp: " + String(adc.read_temperature()) + " C");

    tft.setCursor(0, 120);
    tft.print("-------DAC-------");

    tft.setCursor(0, 140);
    tft.fillRect(0, 140, 240, 20, TFT_BLACK);
    tft.print("vDAC: " + String(encoderSet) + " mV");

    tft.setCursor(0, 160);
    tft.fillRect(0, 160, 240, 20, TFT_BLACK);
    digitalSet = (uint16_t)(((encoderSet / 1000.0) / DAC_V_MAX) * DAC_RESOLUTION + 0.5);
    tft.print("dDAC: " + String(digitalSet));

  }

  //if button was pressed, put encoder as voltage in DAC
  if (encoder.isButtonPressed()) {
    encoderSet = encoder.getPosition();
    dac.set_voltage(encoderSet);
    Serial.println("DAC set to " + String(encoderSet) + " mV");
  }

  //Delay 10ms
  delay(10);
}