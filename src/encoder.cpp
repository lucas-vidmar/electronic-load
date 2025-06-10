#include "encoder.h"

Encoder* Encoder::instance = nullptr;

Encoder::Encoder() : lastState(LOW), position(0), lastPosition(0), buttonPressed(false), encoderMaxPosition(10), encoderMinPosition(1), lastStateDT(LOW) { }

void Encoder::init() {
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    
    // Small delay to stabilize readings
    delay(10);
    lastState = digitalRead(ENCODER_CLK);
    lastStateDT = digitalRead(ENCODER_DT);

    instance = this;

    // Use CHANGE instead of FALLING for more reliable detection
    // This triggers on both rising and falling edges
    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handle_interrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_SW), handle_button_interrupt, FALLING);
    
    Serial.printf("[ENCODER] Initialized - CLK: %d, DT: %d, SW: %d\n", ENCODER_CLK, ENCODER_DT, ENCODER_SW);
}

void IRAM_ATTR Encoder::handle_interrupt() {
    static unsigned long lastCWInterruptTime = 0;
    static unsigned long lastCCWInterruptTime = 0;
    unsigned long interruptTime = millis();

    if (instance) {
        int currentStateCLK = digitalRead(ENCODER_CLK);
        int currentStateDT = digitalRead(ENCODER_DT);
        
        // Check if CLK state actually changed (additional bounce protection)
        if (currentStateCLK != instance->lastState) {
            // Only process on CLK falling edge for one pulse per detent
            if (instance->lastState == HIGH && currentStateCLK == LOW) {
                // Add small delay to let signals stabilize
                delayMicroseconds(50);
                
                // Re-read DT to ensure stable reading
                currentStateDT = digitalRead(ENCODER_DT);
                
                // Check direction and apply appropriate debounce timing
                if (currentStateDT == HIGH) {
                    // Clockwise rotation - check CW debounce
                    if (interruptTime - lastCWInterruptTime > ENCODER_ROTATION_DEBOUNCE_CW) {
                        if (instance->position < instance->encoderMaxPosition) {
                            instance->position++;
                            Serial.printf("[ENCODER] CW - Position: %d\n", instance->position);
                        }
                        lastCWInterruptTime = interruptTime;
                    }
                } else {
                    // Counter-clockwise rotation - check CCW debounce
                    if (interruptTime - lastCCWInterruptTime > ENCODER_ROTATION_DEBOUNCE_CCW) {
                        if (instance->position > instance->encoderMinPosition) {
                            instance->position--;
                            Serial.printf("[ENCODER] CCW - Position: %d\n", instance->position);
                        }
                        lastCCWInterruptTime = interruptTime;
                    }
                }
            }
            
            instance->lastState = currentStateCLK;
            instance->lastStateDT = currentStateDT;
        }
    }
}

void IRAM_ATTR Encoder::handle_button_interrupt() {
    static unsigned long lastButtonInterruptTime = 0;
    unsigned long interruptTime = millis();
    
    // Debounce: ignore interrupts that occur within 50ms of the last interrupt
    if (interruptTime - lastButtonInterruptTime > ENCODER_BUTTON_DEBOUNCE) {
        if (instance) {
            instance->buttonPressed = true;
        }
    }
    lastButtonInterruptTime = interruptTime;
}

bool Encoder::is_button_pressed() {
    bool wasPressed = buttonPressed;
    buttonPressed = false;  // Reset after reading
    if (wasPressed) {
        Serial.println("[ENCODER] Button pressed");
    }
    return wasPressed;
}

int Encoder::get_position() {
    // Disable interrupts temporarily to ensure atomic read
    noInterrupts();
    int currentPos = position;
    interrupts();
    return currentPos;
}

void Encoder::set_position(int pos) {
    // Disable interrupts temporarily to ensure atomic write
    noInterrupts();
    lastPosition = position;
    position = pos;
    interrupts();
    Serial.printf("[ENCODER] Position set to: %d\n", pos);
}

void Encoder::set_max_position(int maxPos) {
    encoderMaxPosition = maxPos;
    Serial.printf("[ENCODER] Max position set to: %d\n", maxPos);
}

void Encoder::set_min_position(int minPos) {
    encoderMinPosition = minPos;
}

bool Encoder::has_changed() {
    // Disable interrupts temporarily to ensure atomic read/write
    noInterrupts();
    bool changed = lastPosition != position;
    lastPosition = position;
    interrupts();
    return changed;
}

void Encoder::reset_button() {
    buttonPressed = false;
}

int Encoder::get_encoder_max_position(){
    return encoderMaxPosition;
}

int Encoder::get_encoder_min_position(){
    return encoderMinPosition;
}