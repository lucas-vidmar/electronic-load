/**
 * @file main.h
 * @brief Main header file for the Electronic Load project
 * @note Contains function declarations and includes for the electronic load application
 * @date 2023
 */

#pragma once

#include <Arduino.h>
#include <SPIFFS.h>
#include <vector>

#include "encoder.h"
#include "led.h"
#include "i2c.h"
#include "dac.h"
#include "analog_sws.h"
#include "adc.h"
#include "lvgl_lcd.h"
#include "fsm.h"
#include "webserver.h"
#include "I2CScanner.h"
#include "fan.h"

/* -- Constants -- */
const String SSID = "Electronic Load";
const String PASSWORD = "pfi2025uade";

/* -- Function Declarations -- */
/**
 * @brief Displays and handles the main menu interface
 */
void main_menu();

/**
 * @brief Handles constant mode operation (CC, CV, etc.)
 * @param unit Unit of measurement (e.g., "A", "V")
 * @param digitsBeforeDecimal Number of digits before decimal point
 * @param digitsAfterDecimal Number of digits after decimal point
 * @param totalDigits Total number of digits for the value
 */
void constant_x(String unit, int digitsBeforeDecimal, int digitsAfterDecimal, int totalDigits);

/**
 * @brief Converts an array of digits to a floating-point number
 * @param digitsValues Vector containing individual digits
 * @param digitsBeforeDecimal Number of digits before decimal point
 * @param digitsAfterDecimal Number of digits after decimal point
 * @param totalDigits Total number of digits
 * @return The floating-point value represented by the digits
 */
float digits_to_number(std::vector<int> digitsValues, int digitsBeforeDecimal, int digitsAfterDecimal, int totalDigits);