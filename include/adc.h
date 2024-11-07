/**
 * @file adc.h
 * @brief Header file for the ADC class.
 * 
 * This file contains the declaration of the ADC class, which provides methods
 * to interface with an Analog-to-Digital Converter (ADC) module. The class
 * allows for initialization of the ADC, and reading current, temperature, and
 * voltage values from the Device Under Test (DUT) using I2C communication.
 * 
 * @note This class requires an I2C instance for communication.
 * 
 * @date 2024-11-01
 */
#pragma once

#include <Wire.h>
#include <Arduino.h>
#include <i2c.h>

#define ADS1115_ADDR 0x48
#define ADC_CONVERSION_TIME 8
#define ADC_CHANNEL_IDUT 0
#define ADC_CHANNEL_TEMP 1
#define ADC_CHANNEL_VDUT 2
#define ADC_MAX_VALUE (1 << 16)
#define ADC_PGA 6.144

/**
 * @class ADC
 * @brief A class to interface with an Analog-to-Digital Converter (ADC) module.
 * 
 * The ADC class provides methods to initialize the ADC module, read current,
 * temperature, and voltage values from the Device Under Test (DUT). It uses
 * I2C communication to interact with the ADC hardware.
 * 
 * @note This class requires an I2C instance for communication.
 */
class ADC {
public:
    /**
     * @brief Constructor for the ADC class.
     * 
     * Initializes the ADC (Analog-to-Digital Converter) module.
     */
    ADC();

    /**
     * @brief Initializes the ADC with the given I2C pointer.
     * 
     * This function sets up the ADC (Analog-to-Digital Converter) using the provided
     * I2C pointer. It configures the necessary settings to enable communication
     * with the ADC over the I2C bus.
     * 
     * @param i2c_pointer A pointer to an I2C instance used for communication with the ADC.
     */
    void init(I2C* i2c_pointer);

    /**
     * @brief Reads the current from the Device Under Test (DUT).
     * 
     * This function measures and returns the current flowing through the DUT.
     * 
     * @return float The current value in amperes.
     */
    float read_iDUT();

    /**
     * @brief Reads the temperature from the sensor.
     * 
     * This function interfaces with the temperature sensor to obtain the current
     * temperature reading. The temperature value is returned as a floating-point
     * number representing the temperature in degrees Celsius.
     * 
     * @return float The current temperature in degrees Celsius.
     */
    float read_temperature();

    /**
     * @brief Reads the voltage from the Device Under Test (DUT).
     * 
     * This function measures and returns the voltage value from the DUT.
     * 
     * @return float The voltage value read from the DUT.
     */
    float read_vDUT();

private:
    /**
     * @brief Pointer to an I2C instance used for communication.
     */
    I2C* i2c;

    /**
     * @brief Reads the ADC value from the specified channel.
     * 
     * This function reads the analog-to-digital converter (ADC) value from the 
     * specified channel and stores the result in the provided value pointer.
     * 
     * @param channel The ADC channel to read from.
     * @param value Pointer to an int16_t variable where the read value will be stored.
     */
    void read(uint8_t channel, int16_t* value);

    /**
     * @brief Reads the voltage from the specified ADC channel.
     * 
     * This function reads the analog voltage from the given ADC channel and 
     * returns the corresponding voltage value as a float.
     * 
     * @param channel The ADC channel to read from (0-255).
     * @return float The voltage read from the specified ADC channel.
     */
    float read_voltage(uint8_t channel);
};