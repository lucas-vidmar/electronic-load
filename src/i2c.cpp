#include "i2c.h"

I2C::I2C() {}

void I2C::init() {
    Wire.begin(I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
    Wire.setClock(I2C_MASTER_FREQ_HZ);
}

void I2C::write(uint8_t addr, uint8_t *data, size_t size) {
    Wire.beginTransmission(addr);
    for (size_t i = 0; i < size; i++) { // Write the data to the I2C device
        Wire.write(data[i]);
    }
    Wire.endTransmission();
}

void I2C::read(uint8_t addr, uint8_t *data, size_t size) {
    // Request data from the I2C device
    Wire.requestFrom(addr, size);
    for (size_t i = 0; i < size; i++) { // Read the data into the buffer
        if (Wire.available()) {
            data[i] = Wire.read();
        }
    }
}