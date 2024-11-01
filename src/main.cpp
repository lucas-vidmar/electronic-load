#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "encoder.h"
#include "led.h"

Encoder encoder = Encoder();
BuiltInLed led = BuiltInLed();
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
}

void loop() {
  // write in display
  tft.setCursor(0, 20);
  //clean line
  tft.fillRect(0, 20, 240, 20, TFT_BLACK);
  tft.print(encoder.getPosition());
  
}