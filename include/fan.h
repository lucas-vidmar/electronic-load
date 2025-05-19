/**
 * @file FanController.h
 * @brief Header file for fan control and PID fan controller classes.
 * @date 2025-04-29
 */
#pragma once

#include <Arduino.h>

// Fan control pin definitions
#define EN_FAN_PIN     GPIO_NUM_2   /*!< GPIO pin to enable/disable the fan */
#define PWM_FAN_PIN    GPIO_NUM_0   /*!< GPIO pin for PWM control of fan speed */
#define LOCK_FAN_PIN   GPIO_NUM_16  /*!< GPIO pin to detect fan lock/stall */

// PID controller tuning parameters
#define PID_KP                      240.0f        /*!< Proportional gain */
#define PID_KI                      16.0f         /*!< Integral gain */
#define PID_KD                      900.0f         /*!< Derivative gain */
#define PID_SAFETY_MARGIN           10.0f    /*!< Safety margin for temperature control */
#define PID_SETPOINT_BEFORE_MARGIN  56.0f /*!< Setpoint before applying safety margin */
#define PID_SETPOINT                (PID_SETPOINT_BEFORE_MARGIN - PID_SAFETY_MARGIN) /*!< Target temperature in degrees Celsius */

/**
 * @class Fan
 * @brief A class to control a cooling fan via PWM and status detection.
 *
 * The Fan class provides methods to initialize hardware pins,
 * enable/disable the fan, set speed via PWM, and detect fan lock/stall status.
 *
 * @note PWM speed range is 0 (off) to 255 (full speed).
 */
class Fan {
private:
    uint8_t pwmPin;
    uint8_t enablePin;
    uint8_t lockPin;
    bool enabled;
    uint8_t speed;

public:
    Fan(uint8_t pwmPin, uint8_t enablePin, uint8_t lockPin);
    void init();
    void enable();
    void disable();
    void set_speed(uint8_t speed); // 0-255 for PWM
    bool is_enabled() const;
    uint8_t get_speed() const;
    bool is_locked() const; // Check if fan is stalled/locked
    int get_speed_percentage() const;
};

/**
 * @class PIDFanController
 * @brief A PID controller to regulate fan speed based on temperature.
 *
 * The PIDFanController class calculates fan speed adjustments
 * based on proportional, integral, and derivative gains to maintain
 * a target temperature.
 *
 * @note Configure output limits via set_output_limits().
 */
class PIDFanController {
private:
    Fan& fan;
    float kP;           // Proportional gain
    float kI;           // Integral gain
    float kD;           // Derivative gain
    float targetTemp;   // Target temperature in degrees C
    float integral;     // Integral term accumulator
    float lastError;    // Previous error for derivative calculation
    float minOutput;    // Minimum output value (fan speed)
    float maxOutput;    // Maximum output value (fan speed)
    unsigned long lastTime; // Last time the PID was computed

public:
    PIDFanController(Fan& fan, float kP, float kI, float kD, float minOutput = 0, float maxOutput = 255);
    void init(float targetTemperature);
    void set_target_temperature(float temp);
    float get_target_temperature() const;
    void set_tunings(float kP, float kI, float kD);
    void set_output_limits(float min, float max);
    void reset();
    
    // Compute new fan speed based on current temperature
    void compute(float currentTemperature);
};