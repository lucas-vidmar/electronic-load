#include "i2c.h"

I2C::I2C() {}

void I2C::init() {
    Wire.begin(I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
    Wire.setClock(I2C_MASTER_FREQ_HZ);
    Serial.printf("[I2C] Initialized - SDA: %d, SCL: %d, Freq: %d Hz\n", I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_MASTER_FREQ_HZ);
}

void I2C::write(uint8_t addr, uint8_t *data, size_t size) {
    Wire.beginTransmission(addr);
    for (size_t i = 0; i < size; i++) { // Write the data to the I2C device
        Wire.write(data[i]);
    }
    uint8_t result = Wire.endTransmission();
    if (result != 0) {
        Serial.printf("[I2C] Write error to 0x%02X: result=%d, size=%d\n", addr, result, size);
    }
}

void I2C::read(uint8_t addr, uint8_t *data, size_t size) {
    // Request data from the I2C device
    uint8_t received = Wire.requestFrom(addr, size);
    if (received != size) {
        Serial.printf("[I2C] Read error from 0x%02X: requested=%d, received=%d\n", addr, size, received);
    }
    for (size_t i = 0; i < size; i++) { // Read the data into the buffer
        if (Wire.available()) {
            data[i] = Wire.read();
        } else {
            Serial.printf("[I2C] No data available for byte %d from 0x%02X\n", i, addr);
            data[i] = 0; // Fill with 0 if no data available
        }
    }
}