#include "analog_sws.h"

AnalogSws::AnalogSws() {}

void AnalogSws::init() {
    // Set the pin modes for the analog switches
    // pinMode(ANALOG_SW1_ENABLE, OUTPUT); // Commented out to avoid UART conflict @todo
    pinMode(ANALOG_SW4_ENABLE, OUTPUT);
    pinMode(DUT_ENABLE, OUTPUT);
}

void AnalogSws::v_dac_enable() { digitalWrite(ANALOG_SW1_ENABLE, HIGH); }

void AnalogSws::v_dac_disable() { digitalWrite(ANALOG_SW1_ENABLE, LOW); }

void AnalogSws::mosfet_input_cc_mode() { digitalWrite(ANALOG_SW4_ENABLE, HIGH); }

void AnalogSws::mosfet_input_cv_mode() { digitalWrite(ANALOG_SW4_ENABLE, LOW); }

int AnalogSws::get_mosfet_input_mode() { return digitalRead(ANALOG_SW4_ENABLE); }

void AnalogSws::relay_dut_enable() { digitalWrite(DUT_ENABLE, HIGH); }

void AnalogSws::relay_dut_disable() { digitalWrite(DUT_ENABLE, LOW); }