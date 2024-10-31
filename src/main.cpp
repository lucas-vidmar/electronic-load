#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "encoder.h"

Encoder encoder = Encoder();
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

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // print encoder position
  Serial.println(encoder.getPosition());
  // write in display
  tft.setCursor(0, 20);
  tft.print(encoder.getPosition());
  // blink led
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}