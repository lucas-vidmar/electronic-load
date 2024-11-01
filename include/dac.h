#include <Wire.h>
#include <Arduino.h>

#define MCP4725_ADDR 0x60   /*!< MCP4725 I2C address */

#define DAC_BITS 12 /*!< DAC resolution in bits */
#define DAC_RESOLUTION (1 << DAC_BITS) /*!< DAC resolution in bits */
#define DAC_REF_VOLTAGE 4.096   /*!< Reference voltage in volts */
#define DAC_Q (DAC_REF_VOLTAGE / DAC_RESOLUTION)    /*!< Voltage step in V */
#define DAC_QmV (Q * 1000)  /*!< Voltage step in mV */
#define DAC_V_MAX 0.5   /*!< Maximum voltage output in V */
#define DAC_OUTPUT_VOLTAGE_DIVIDER (DAC_V_MAX / DAC_REF_VOLTAGE)    /*!< Output voltage divider */
#define DAC_MAX_DIGITAL_VALUE 83

class DAC {
public:
    void setup() {
        Serial.begin(115200);
        Wire.begin();
        dac_setup();
    }

    void set_voltage(int voltageInMmV) {
        // Calculate DAC value
        float value = ((voltageInMmV / 1000.0) / DAC_V_MAX) * DAC_RESOLUTION;
        Serial.printf("Setting DAC voltage to %d mV\n", voltageInMmV);
        // Round to nearest integer
        dac_write((uint16_t)(value + 0.5));
    }

private:
    void dac_setup() {
        Serial.println("Adding DAC device");
        dac_write(0); // Set DAC to default value (0V)
    }

    void dac_write(uint16_t value) {
        Serial.printf("Writing value to DAC: %d\n", value);
        if (value > DAC_MAX_DIGITAL_VALUE) { // Check if value is out of range
            Serial.println("DAC value out of range");
            return;
        }

        // Write value to DAC
        Wire.beginTransmission(MCP4725_ADDR);
        Wire.write((value >> 8) & 0x0F);
        Wire.write(value & 0xFF);
        Wire.endTransmission();
    }
};

DAC dac;

void setup() {
    dac.setup();
}

void loop() {
    // Example usage
    dac.set_voltage(1000); // Set DAC to 1000 mV
    delay(1000);
}