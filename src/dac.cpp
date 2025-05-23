#include "dac.h"

DAC::DAC() : i2c(nullptr) {}

void DAC::init(I2C* i2cPointer){
    i2c = i2cPointer;
    digital_write(0); // Set DAC to default value (0V)
    Serial.println("[DAC] Initialized with default value (0V)");
}

void DAC::set_voltage(float voltageInMmV, float dacVMax) {

    // Serial.println("DAC voltage in mV: " + String(voltageInMmV,2));
    // Check in range
    if (voltageInMmV < 0 || (voltageInMmV / 1000) > dacVMax) {
        Serial.println("[DAC] ERROR: Voltage out of range: " + String(voltageInMmV, 2) + " mV - Max: " + String(dacVMax, 2) + " V");
        return;
    }

    // Calculate DAC value
    float value = ((voltageInMmV / 1000.0) / dacVMax) * (DAC_RESOLUTION - 1); // Convert mV to V and scale to DAC resolution
    // Round to nearest integer
    digital_write((uint16_t)(value + 0.5));
}

void DAC::digital_write(uint16_t value) {
    // Serial.println("DAC Digital value: " + String(value));
    if (value > DAC_MAX_DIGITAL_VALUE) { // Check if value is out of range
        Serial.println("[DAC] ERROR: Digital value out of range: " + String(value));
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
        Serial.println("[DAC] ERROR: CC MODE - Current out of range: " + String(current, 3) + "A (Max: " + String(DAC_CC_MAX_CURRENT, 1) + "A)");
        return;
    }
    
    static float prevCurrent = 0.0;

    if (current != prevCurrent) {
        prevCurrent = current;
        Serial.printf("[DAC] CC MODE: Setting current to %.3fA\n", current);
        float correctedCurrent = current - (current * CC_CORRECTION_PARAMETER_SLOPE + CC_CORRECTION_PARAMETER_INTERCEPT); // Apply correction PARAMETER
        if (correctedCurrent < 0) correctedCurrent = 0; // Ensure current is not negative
        set_voltage(correctedCurrent * 100 / CANT_MOSFET, DAC_V_MAX_CC); // Set voltage to current * 100mOhm / CANT_MOSFET
    }
}

void DAC::cv_mode_set_voltage(float voltage) {
    // V_DAC = V_DUT / 200
    // V_DAC[mV] = V_DUT[V] * 1000mV / 200
    if (voltage < 0 || voltage > DAC_CV_MAX_VOLTAGE) {
        Serial.println("[DAC] ERROR: CV MODE - Voltage out of range: " + String(voltage, 3) + "V (Max: " + String(DAC_CV_MAX_VOLTAGE, 1) + "V)");
        return;
    }

    static float prevVoltage = 0.0;

    if (voltage != prevVoltage) {
        prevVoltage = voltage;
        Serial.printf("[DAC] CV MODE: Setting voltage to %.3fV\n", voltage);
        float correctedVoltage = voltage - (voltage * CV_CORRECTION_PARAMETER_SLOPE + CV_CORRECTION_PARAMETER_INTERCEPT); // Apply correction PARAMETER
        if (correctedVoltage < 0) correctedVoltage = 0; // Ensure voltage is not negative
        set_voltage(correctedVoltage * 1000 / 200, DAC_V_MAX_CV); // Set voltage to V_DUT * 1000mV / 200
    }
}

void DAC::cr_mode_set_resistance(float resistance, float dutVoltage) {
    // V = I * R
    // I = V / R
    if (resistance < 0 || resistance > DAC_CR_MAX_RESISTANCE) {
        Serial.println("[DAC] ERROR: CR MODE - Resistance out of range: " + String(resistance, 3) + "kΩ (Max: " + String(DAC_CR_MAX_RESISTANCE, 1) + "kΩ)");
        return;
    }
    
    Serial.printf("[DAC] CR MODE: Setting resistance to %.3fkΩ (V_DUT: %.3fV)\n", resistance, dutVoltage);
    float current = dutVoltage / (resistance * 1000); // I_DUT = V_DUT / R
    cc_mode_set_current(current); // Set current to I_DUT
}

void DAC::cw_mode_set_power(float power, float dutVoltage) {
    // P = V * I
    // I = P / V
    if (power < 0 || power > DAC_CW_MAX_POWER) {
        Serial.println("[DAC] ERROR: CW MODE - Power out of range: " + String(power, 3) + "W (Max: " + String(DAC_CW_MAX_POWER, 1) + "W)");
        return;
    }

    Serial.printf("[DAC] CW MODE: Setting power to %.3fW (V_DUT: %.3fV)\n", power, dutVoltage);
    float current = power / dutVoltage; // I_DUT = P / V_DUT
    cc_mode_set_current(current); // Set current to I_DUT
}