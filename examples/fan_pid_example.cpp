#include <Arduino.h>
#include "fan.h"

// Simulated temperature sensor reading
float getTemperature() {
    // This would typically be replaced with an actual temperature sensor reading
    // For demo purposes, let's simulate temperature changing over time
    static float simulatedTemp = 25.0;
    static unsigned long lastUpdate = 0;
    
    unsigned long now = millis();
    if (now - lastUpdate > 500) {
        // Simulate some temperature changes
        simulatedTemp += random(-100, 100) / 100.0;
        // Keep it in a realistic range
        if (simulatedTemp < 20.0) simulatedTemp = 20.0;
        if (simulatedTemp > 60.0) simulatedTemp = 60.0;
        lastUpdate = now;
    }
    
    return simulatedTemp;
}

// Create fan instances
Fan fan1(PWM_FAN_1_PIN, EN_FAN_1_PIN, LOCK_FAN_1_PIN);

// Create PID controllers with tuning parameters
// P=2.0, I=0.5, D=1.0, min=0, max=255
PIDFanController pidController(fan1, 2.0, 0.5, 1.0);

void setup() {
    Serial.begin(115200);
    
    // Initialize PID controller with target temperature of 30°C
    pidController.init(30.0);
    
    Serial.println("PID Fan Controller Example");
    Serial.println("Target temperature: 30°C");
}

void loop() {
    // Get current temperature
    float currentTemp = getTemperature();
    
    // Compute and adjust fan speed based on temperature
    pidController.compute(currentTemp);
    
    // Print status every second
    static unsigned long lastPrint = 0;
    unsigned long now = millis();
    if (now - lastPrint > 1000) {
        Serial.print("Temperature: ");
        Serial.print(currentTemp);
        Serial.print("°C, Fan Speed: ");
        Serial.print(fan1.get_speed());
        Serial.print("/255, Fan Locked: ");
        Serial.println(fan1.is_locked() ? "YES" : "NO");
        lastPrint = now;
    }
    
    delay(100); // Small delay for stability
}
