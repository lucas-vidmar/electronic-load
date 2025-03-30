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
    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handle_interrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_SW), handle_button_interrupt, FALLING);
}

void IRAM_ATTR Encoder::handle_interrupt() {
    static unsigned long lastInterruptTime = 0;
    unsigned long interruptTime = millis();
    
    // Debounce: ignore interrupts that occur within 15ms of the last interrupt
    if (interruptTime - lastInterruptTime > ENCODER_ROTATION_DEBOUNCE) {
        if (instance) {
            int state = digitalRead(ENCODER_CLK);
            if (state != instance->lastState) { // If state has changed, pulse occurred
                instance->lastPosition = instance->position; // Save last position
                if (digitalRead(ENCODER_DT) != state) { // If DT state is different from CLK state
                    instance->position++;
                } else {
                    instance->position--;
                }
                instance->lastState = state;
            }
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
    return wasPressed;
}

int Encoder::get_position() {
    if (position > encoderMaxPosition) {
        set_position(encoderMaxPosition);
    } else if (position < encoderMinPosition) {
        set_position(encoderMinPosition);
    }
    return position;
}

void Encoder::set_position(int pos) {
    lastPosition = position;
    position = pos;
}

void Encoder::set_max_position(int maxPos) {
    encoderMaxPosition = maxPos;
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