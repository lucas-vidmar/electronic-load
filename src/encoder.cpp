#include "encoder.h"

Encoder* Encoder::instance = nullptr;

Encoder::Encoder() : lastState(LOW), position(0), lastPosition(0), buttonPressed(false), encoderMaxPosition(10), encoderMinPosition(1) { }

void Encoder::init() {
    pinMode(ENCODER_CLK, INPUT);
    pinMode(ENCODER_DT, INPUT);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    lastState = digitalRead(ENCODER_CLK);

    instance = this;

    // Attach interrupts without calling gpio_install_isr_service
    // Change CHANGE to FALLING to trigger only once per detent
    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handle_interrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(ENCODER_SW), handle_button_interrupt, FALLING);
    
    Serial.printf("[ENCODER] Initialized - CLK: %d, DT: %d, SW: %d\n", ENCODER_CLK, ENCODER_DT, ENCODER_SW);
}

void IRAM_ATTR Encoder::handle_interrupt() {
    static unsigned long lastInterruptTime = 0;
    unsigned long interruptTime = millis();

    // Debounce: ignore interrupts that occur within 5ms of the last interrupt
    if (interruptTime - lastInterruptTime > ENCODER_ROTATION_DEBOUNCE) {
        if (instance) {
            // Read the DT pin to determine direction
            // Note: CLK state check is removed as interrupt triggers only on FALLING edge
            if (digitalRead(ENCODER_DT) != digitalRead(ENCODER_CLK)) { // Check DT against CLK (which is now LOW due to FALLING trigger)
                // Clockwise rotation
                if (instance->position < instance->encoderMaxPosition) {
                    instance->position++;
                }
            } else {
                // Counter-clockwise rotation
                if (instance->position > instance->encoderMinPosition) {
                    instance->position--;
                }
            }
            // No need to update lastState as we only care about the falling edge
        }
    }
    lastInterruptTime = interruptTime;
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
    return position;
}

void Encoder::set_position(int pos) {
    lastPosition = position;
    position = pos;
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
    bool changed = lastPosition != position;
    lastPosition = position;
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