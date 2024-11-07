/**
 * @file i2c.h
 * @brief Header file for the I2C class.
 * 
 * This file contains the declaration of the I2C class, which provides methods
 * for initializing the I2C interface and performing read and write operations
 * to devices on the I2C bus.
 * 
 * @note This class assumes that the underlying hardware and software support
 *       I2C communication.
 * 
 * @date 2024-11-01
 */
#pragma once

#include <Arduino.h>
#include <Wire.h>

#define I2C_MASTER_SCL_IO GPIO_NUM_22   /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO GPIO_NUM_21   /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_FREQ_HZ 100000       /*!< I2C master clock frequency */

/**
 * @class I2C
 * @brief A class to handle I2C communication.
 * 
 * The I2C class provides methods to initialize the I2C interface and perform
 * read and write operations to devices on the I2C bus.
 * 
 * @note This class assumes that the underlying hardware and software support
 *       I2C communication.
 */
class I2C {
public:
    /**
     * @brief Default constructor for the I2C class.
     */
    I2C();

    /**
     * @brief Initializes the I2C interface.
     * 
     * This function sets up the necessary configurations and initializes the I2C
     * hardware for communication. It should be called before any I2C transactions
     * are performed.
     */
    void init();

    /**
     * @brief Writes data to a specified I2C address.
     * 
     * This function sends a sequence of bytes to a device on the I2C bus.
     * 
     * @param addr The 7-bit I2C address of the target device.
     * @param data Pointer to the data buffer containing the bytes to be written.
     * @param size The number of bytes to write from the data buffer.
     */
    void write(uint8_t addr, uint8_t *data, size_t size);

    /**
     * @brief Reads data from the specified I2C address.
     * 
     * This function reads a specified number of bytes from a given I2C address
     * and stores the data in the provided buffer.
     * 
     * @param addr The I2C address to read from.
     * @param data Pointer to the buffer where the read data will be stored.
     * @param size The number of bytes to read.
     */
    void read(uint8_t addr, uint8_t *data, size_t size);
};