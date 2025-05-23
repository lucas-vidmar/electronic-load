#include "led.h"

BuiltInLed::BuiltInLed() : timer(nullptr) { }

void BuiltInLed::init() {
    pinMode(BUILT_IN_LED_PIN, OUTPUT);
    digitalWrite(BUILT_IN_LED_PIN, LOW);
    timer = timerBegin(0, 80, true);
    Serial.printf("[LED] Initialized - Pin: %d\n", BUILT_IN_LED_PIN);
}

void BuiltInLed::blink(int interval) {
    // Attach the timer interrupt
    timerAttachInterrupt(timer, &on_timer, true);
    timerAlarmWrite(timer, interval * 1000, true); // Convert milliseconds to microseconds
    timerAlarmEnable(timer);
    Serial.printf("[LED] Blink started with interval: %d ms\n", interval);
}

void BuiltInLed::on_timer() {
    digitalWrite(BUILT_IN_LED_PIN, !digitalRead(BUILT_IN_LED_PIN));
}