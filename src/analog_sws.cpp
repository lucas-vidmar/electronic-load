#include "analog_sws.h"

AnalogSws::AnalogSws() {}

bool relayEnabled = false;

void AnalogSws::init() {
    // Set the pin modes for the analog switches
    pinMode(ANALOG_SW1_ENABLE, OUTPUT);
    pinMode(ANALOG_SW4_ENABLE, OUTPUT);
    pinMode(DUT_ENABLE, OUTPUT);
    Serial.printf("[ANALOG_SWS] Initialized - SW1: %d, SW4: %d, DUT: %d\n", 
                 ANALOG_SW1_ENABLE, ANALOG_SW4_ENABLE, DUT_ENABLE);
}

void AnalogSws::v_dac_enable() { 
    digitalWrite(ANALOG_SW1_ENABLE, HIGH); 
    Serial.println("[ANALOG_SWS] V_DAC enabled");
}

void AnalogSws::v_dac_disable() { 
    digitalWrite(ANALOG_SW1_ENABLE, LOW); 
    Serial.println("[ANALOG_SWS] V_DAC disabled");
}

void AnalogSws::mosfet_input_cc_mode() {
    digitalWrite(ANALOG_SW4_ENABLE, HIGH); 
    Serial.println("[ANALOG_SWS] MOSFET set to CC mode");
}

void AnalogSws::mosfet_input_cv_mode() { 
    digitalWrite(ANALOG_SW4_ENABLE, LOW); 
    Serial.println("[ANALOG_SWS] MOSFET set to CV mode");
}

int AnalogSws::get_mosfet_input_mode() { return digitalRead(ANALOG_SW4_ENABLE); }

void AnalogSws::relay_dut_enable() {
    if (relayEnabled) return;
    relayEnabled = true;
    digitalWrite(DUT_ENABLE, HIGH); 
    Serial.println("[ANALOG_SWS] DUT relay enabled");
}

void AnalogSws::relay_dut_disable() { 
    if (!relayEnabled) return;
    relayEnabled = false;
    digitalWrite(DUT_ENABLE, LOW); 
    Serial.println("[ANALOG_SWS] DUT relay disabled");
}