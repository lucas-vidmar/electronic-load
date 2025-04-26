#pragma once

#include <Arduino.h>

// Fan control pin definitions
#define EN_FAN_PIN     GPIO_NUM_2
#define PWM_FAN_PIN    GPIO_NUM_0
#define LOCK_FAN_PIN   GPIO_NUM_16
// PID controller tuning parameters
#define PID_KP          7.0
#define PID_KI          0.01
#define PID_KD          0.5
#define PID_SETPOINT   40.0 // Target temperature in degrees C

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