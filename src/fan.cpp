#include "fan.h"

// Fan class implementation
Fan::Fan(uint8_t pwmPin, uint8_t enablePin, uint8_t lockPin)
    : pwmPin(pwmPin), enablePin(enablePin), lockPin(lockPin), enabled(false), speed(0) {
}

void Fan::init() {
    pinMode(pwmPin, OUTPUT);
    pinMode(enablePin, OUTPUT);
    pinMode(lockPin, INPUT);
    
    // Initialize with fan disabled
    disable();
    analogWrite(pwmPin, 0);
}

void Fan::enable() {
    digitalWrite(enablePin, HIGH);
    enabled = true;
    // Apply current speed setting
    analogWrite(pwmPin, speed);
}

void Fan::disable() {
    digitalWrite(enablePin, LOW);
    // Explicitly set the pin low in addition to setting PWM duty cycle to 0
    digitalWrite(pwmPin, LOW); 
    analogWrite(pwmPin, 0);
    enabled = false;
}

void Fan::set_speed(uint8_t speed) {
    this->speed = speed;
    if (enabled) {
        analogWrite(pwmPin, speed);
    }
}

bool Fan::is_enabled() const {
    return enabled;
}

uint8_t Fan::get_speed() const {
    return speed;
}

bool Fan::is_locked() const {
    unsigned long highTime = pulseIn(lockPin, HIGH, 1000000); // Read the lock pin state
    unsigned long lowTime = pulseIn(lockPin, LOW, 1000000);
    if (highTime == 0 && lowTime == 0) return true; // Fan is locked
    else return false; // Fan is not locked
}

float Fan::get_speed_percentage() const {
    return (static_cast<float>(speed) / 255.0f) * 100.0f; // Convert to percentage
}

// PIDFanController implementation
PIDFanController::PIDFanController(Fan& fan, float kP, float kI, float kD, float minOutput, float maxOutput)
    : fan(fan), kP(kP), kI(kI), kD(kD), targetTemp(0), integral(0), lastError(0), minOutput(minOutput), maxOutput(maxOutput), lastTime(0) { }

void PIDFanController::init(float targetTemperature) {
    targetTemp = targetTemperature;
    reset();
    fan.init();
    fan.enable();
}

void PIDFanController::set_target_temperature(float temp) {
    targetTemp = temp;
}

float PIDFanController::get_target_temperature() const {
    return targetTemp;
}

void PIDFanController::set_tunings(float _kP, float _kI, float _kD) {
    kP = _kP;
    kI = _kI;
    kD = _kD;
}

void PIDFanController::set_output_limits(float min, float max) {
    if (min >= max) return;
    minOutput = min;
    maxOutput = max;
}

void PIDFanController::reset() {
    integral = 0;
    lastError = 0;
    lastTime = millis();
}

void PIDFanController::compute(float currentTemperature) {
    unsigned long now = millis();
    float timeChange = (now - lastTime) / 1000.0;  // Convert to seconds
    
    // Skip computation if no time has passed
    if (timeChange <= 0) return;
    
    // Calculate error terms
    float error = targetTemp - currentTemperature;
    
    // For cooling: if current temp > target temp, we need more cooling (positive error)
    error = -error;  // Invert the error for cooling application
    
    // Calculate integral term with anti-windup
    integral += (kI * error * timeChange);
    
    // Apply limits to integral term to prevent windup
    if (integral > maxOutput) integral = maxOutput;
    else if (integral < minOutput) integral = minOutput;
    
    // Calculate derivative term
    float derivative = 0;
    if (timeChange > 0) {
        derivative = (error - lastError) / timeChange;
    }
    
    // Calculate output
    float output = kP * error + integral + kD * derivative;
    
    // Apply output limits
    if (output > maxOutput) output = maxOutput;
    else if (output < minOutput) output = minOutput;
    
    // Update fan speed
    fan.set_speed(static_cast<uint8_t>(output));
    
    // Save values for next calculation
    lastError = error;
    lastTime = now;
}