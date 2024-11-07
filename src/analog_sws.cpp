#include "analog_sws.h"

AnalogSws::AnalogSws() {}

void AnalogSws::init() {
    // Set the pin modes for the analog switches
    // pinMode(ANALOG_SW1_ENABLE, OUTPUT); // Commented out to avoid UART conflict @todo
    pinMode(ANALOG_SW4_ENABLE, OUTPUT);
    pinMode(DUT_ENABLE, OUTPUT);
}

void AnalogSws::vDACEnable() { digitalWrite(ANALOG_SW1_ENABLE, HIGH); }

void AnalogSws::vDACDisable() { digitalWrite(ANALOG_SW1_ENABLE, LOW); }

void AnalogSws::mosfetInputCCMode() { digitalWrite(ANALOG_SW4_ENABLE, HIGH); }

void AnalogSws::mosfetInputCVMode() { digitalWrite(ANALOG_SW4_ENABLE, LOW); }

int AnalogSws::getMosfetInputMode() { return digitalRead(ANALOG_SW4_ENABLE); }

void AnalogSws::relayDUTEnable() { digitalWrite(DUT_ENABLE, HIGH); }

void AnalogSws::relayDUTDisable() { digitalWrite(DUT_ENABLE, LOW); }