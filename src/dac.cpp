#include "dac.h"

DAC::DAC() : i2c(nullptr) {}

void DAC::init(I2C* i2c_pointer){
    i2c = i2c_pointer;
    digitalWrite(0); // Set DAC to default value (0V)
}

void DAC::set_voltage(int voltageInMmV, float dac_v_max) {

    Serial.println("DAC voltage in mV: " + String(voltageInMmV));
    // Check in range
    if (voltageInMmV < 0 || (voltageInMmV / 1000) > dac_v_max) {
        Serial.println("Voltage out of range");
        return;
    }

    // Calculate DAC value
    float value = ((voltageInMmV / 1000.0) / dac_v_max) * (DAC_RESOLUTION - 1); // Convert mV to V and scale to DAC resolution
    // Round to nearest integer
    digitalWrite((uint16_t)(value + 0.5));
}

void DAC::digitalWrite(uint16_t value) {
    Serial.println("DAC Digital value: " + String(value));
    if (value > DAC_MAX_DIGITAL_VALUE) { // Check if value is out of range
        Serial.println("DAC value out of range");
        return;
    }

    // Write value to DAC using I2C library
    uint8_t data[2];
    data[0] = (value >> 8) & 0x0F;
    data[1] = value & 0xFF;
    i2c->write(MCP4725_ADDR, data, 2);
}

void DAC::cc_mode_set_current(float current) {
    // I_RS = V_DAC / 100mOhm
    // V_DAC = I_RS * 100mOhm
    // V_DAC[mV] = I_RS[A] * 100mOhm
    if (current < 0 || current > DAC_CC_MAX_CURRENT) {
        Serial.println("Current out of range");
        return;
    }
    
    static float prev_current = 0.0;

    if (current != prev_current) {
        prev_current = current;
        set_voltage(current * 100 / CANT_MOSFET, DAC_V_MAX_CC); // Set voltage to current * 100mOhm / CANT_MOSFET
    }
}

void DAC::cv_mode_set_voltage(float voltage) {
    // V_DAC = V_DUT / 200
    // V_DAC[mV] = V_DUT[V] * 1000mV / 200
    if (voltage < 0 || voltage > DAC_CV_MAX_VOLTAGE) {
        Serial.println("Voltage out of range");
        return;
    }

    static float prev_voltage = 0.0;

    if (voltage != prev_voltage) {
        prev_voltage = voltage;
        set_voltage(voltage * 1000 / 200, DAC_V_MAX_CV); // Set voltage to V_DUT * 1000mV / 200
    }
}