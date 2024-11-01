#include "analog_sws.h"

AnalogSws::AnalogSws() {}

void AnalogSws::init() {
    // Set the pin modes for the analog switches
    pinMode(ANALOG_SW1_ENABLE, OUTPUT);
    pinMode(ANALOG_SW4_ENABLE, OUTPUT);
    pinMode(DUT_ENABLE, OUTPUT);
}

void AnalogSws::vdacEnable() { digitalWrite(ANALOG_SW1_ENABLE, HIGH); }

void AnalogSws::vdacDisable() { digitalWrite(ANALOG_SW1_ENABLE, LOW); }

void AnalogSws::mosfetInputCCMode() { digitalWrite(ANALOG_SW4_ENABLE, HIGH); }

void AnalogSws::mosfetInputCVMode() { digitalWrite(ANALOG_SW4_ENABLE, LOW); }

int AnalogSws::getMosfetInputMode() { return digitalRead(ANALOG_SW4_ENABLE); }

void AnalogSws::relayDutEnable() { digitalWrite(DUT_ENABLE, HIGH); }

void AnalogSws::relayDutDisable() { digitalWrite(DUT_ENABLE, LOW); }