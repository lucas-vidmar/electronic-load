/**
 * @file dac.h
 * @brief Header file for the DAC class.
 * 
 * This file contains the definition of the DAC class, which provides methods
 * to initialize and control a Digital-to-Analog Converter (DAC) over an I2C bus.
 * It includes functions to set the output voltage and write digital values to the DAC.
 * 
 * The DAC class requires an I2C instance for communication.
 * 
 * @note The DAC resolution is 12 bits, with a reference voltage of 4.096V.
 * The maximum output voltage is 0.5V, and the maximum digital value is 4095.
 * 
 * @date 2024-11-01
 */
#pragma once

#include <Wire.h>
#include <Arduino.h>

#include "i2c.h"

#define MCP4725_ADDR 0x60   /*!< MCP4725 I2C address */
#define CANT_MOSFET 4

#define DAC_BITS 12 /*!< DAC resolution in bits */
#define DAC_RESOLUTION (1 << DAC_BITS) /*!< DAC resolution in bits */
#define DAC_REF_VOLTAGE 4.096   /*!< Reference voltage in volts */
#define DAC_Q (DAC_REF_VOLTAGE / DAC_RESOLUTION)    /*!< Voltage step in V */
#define DAC_QMV (DAC_Q * 1000)  /*!< Voltage step in mV */
#define DAC_V_MAX_CC 0.5   /*!< Maximum voltage output in V for CC mode*/
#define DAC_V_MAX_CV 0.45   /*!< Maximum voltage output in V for CV mode*/
#define DAC_OUTPUT_VOLTAGE_DIVIDER (DAC_V_MAX / DAC_REF_VOLTAGE)    /*!< Output voltage divider */
#define DAC_MAX_DIGITAL_VALUE 4095 // To limit either current or voltage
#define DAC_CC_MAX_CURRENT 10
#define DAC_CV_MAX_VOLTAGE 30

/**
 * @class DAC
 * @brief A class to represent a Digital-to-Analog Converter (DAC).
 * 
 * The DAC class provides methods to initialize and control a DAC device
 * over an I2C bus. It allows setting the output voltage and writing digital
 * values to the DAC.
 * 
 * @note This class requires an I2C instance for communication.
 */
class DAC {
public:
    /**
     * @brief Construct a new DAC object.
     * 
     * This constructor initializes a new instance of the DAC class.
     */
    DAC();

    /**
     * @brief Initializes the DAC with the given I2C pointer.
     * 
     * This function sets up the DAC (Digital-to-Analog Converter) using the provided
     * I2C pointer. It configures the necessary settings to enable communication
     * with the DAC over the I2C bus.
     * 
     * @param i2cPointer A pointer to an I2C instance used for communication with the DAC.
     */
    void init(I2C* i2cPointer);

    /**
     * @brief Sets the output voltage of the DAC.
     * 
     * This function sets the output voltage of the DAC (Digital-to-Analog Converter)
     * to the specified value in millivolts (mV).
     * 
     * @param voltageInMmV The desired output voltage in millivolts (mV).
     * @param dacVMax The maximum output voltage of the DAC in volts (V).
     */
    void set_voltage(float voltageInMmV, float dacVMax);

    /**
     * @brief Writes a digital value to the DAC.
     * 
     * This function sets the DAC output to the specified digital value.
     * 
     * @param value The digital value to write to the DAC. This should be a 
     *              16-bit unsigned integer representing the desired output level.
     */
    void digital_write(uint16_t value);

    /**
     * @brief Sets the current in constant current (CC) mode.
     *
     * @param current The current value in amperes.
     */
    void cc_mode_set_current(float current);

    /**
     * @brief Sets the voltage in constant voltage (CV) mode.
     *
     * @param voltage The voltage value in volts.
     */
    void cv_mode_set_voltage(float voltage);

    /**
     * @brief Sets the resistance in constant resistance (CR) mode.
     *
     * @param resistance The resistance value in ohms.
     */
    void cr_mode_set_resistance(float resistance);

    /**
     * @brief Sets the power in constant power (CW) mode.
     *
     * @param power The power value in watts.
     */
    void cw_mode_set_power(float power);

private:
    I2C* i2c; /*!< Pointer to I2C communication interface */
};