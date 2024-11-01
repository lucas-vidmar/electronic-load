#include "adc.h"

ADC::ADC() : i2c(nullptr) {}

void ADC::init(I2C* i2c_pointer) {
    i2c = i2c_pointer;
}

float ADC::read_iDUT() {
    float voltage = read_voltage(ADC_CHANNEL_IDUT);
    return (voltage / 5.0) * 20.0; // 5V ≡ 20A;
}

float ADC::read_temperature() {
    float voltage = read_voltage(ADC_CHANNEL_TEMP);
    return voltage * 100.0; // 10 mV/°C
}

float ADC::read_vDUT() {
    float voltage = read_voltage(ADC_CHANNEL_VDUT);
    return (voltage / 4.0) * 100.0; // 4V ≡ 100V
}

void ADC::read(uint8_t channel, int16_t* value) {

    // Validate the channel (0 to 3)
    if (channel > 3) return;

    // Configure MUX[14:12] for the selected channel in single-ended mode
    uint8_t mux = 0x04 + channel; // MUX[14:12] = 100 + channel

    // Configure the configuration register
    uint16_t config = 0;
    config |= (1 << 15);      // OS = 1 (Start a single conversion)
    config |= (mux << 12);    // MUX[14:12]: Selected channel with respect to GND
    config |= (0 << 9);       // PGA[11:9] = 000 (±6.144V)
    config |= (1 << 8);       // MODE = 1 (Single conversion mode)
    config |= (4 << 5);       // DR[7:5] = 100 (128 SPS)
    config |= (0x03);         // COMP_QUE[1:0] = 11 (Disable the comparator)

    // Write to the configuration register
    uint8_t data[3];
    data[0] = 0x01;                   // Configuration register address
    data[1] = (config >> 8) & 0xFF;   // MSB
    data[2] = config & 0xFF;          // LSB
    i2c->write(ADS1115_ADDR, data, 3);

    // Wait for the conversion to complete (according to the selected data rate)
    delay(ADC_CONVERSION_TIME * 3); // 8ms for 128 SPS

    // Read the conversion register
    data[0] = 0x00; // Conversion register address
    i2c->write(ADS1115_ADDR, data, 1);
    i2c->read(ADS1115_ADDR, data, 2);

    // Combine the MSB and LSB to get the 16-bit value
    int16_t raw_adc = (data[0] << 8) | data[1];

    // Ensure the value is positive
    if (raw_adc < 0) {
        raw_adc = 0;
    }

    // Return the ADC value
    *value = raw_adc;
}

float ADC::read_voltage(uint8_t channel) {
    int16_t v = 0;
    read(channel, &v);

    // Convert the ADC value to a voltage
    return 2 * ADC_PGA * ((float)v) / ((float)ADC_MAX_VALUE); // V = 2 * PGA * ADC / 2^16
}