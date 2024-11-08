#include "encoder.h"

Encoder* Encoder::instance = nullptr;

Encoder::Encoder() : lastState(LOW), position(0), buttonPressed(false), encoderMaxPosition(10), encoderMinPosition(1) { }

void Encoder::init() {
    pinMode(ENCODER_CLK, INPUT);
    pinMode(ENCODER_DT, INPUT);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    lastState = digitalRead(ENCODER_CLK);

    instance = this;

    // Attach interrupts without calling gpio_install_isr_service
    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_SW), handleButtonInterrupt, FALLING);
}

void IRAM_ATTR Encoder::handleInterrupt() {
    static unsigned long lastInterruptTime = 0;
    unsigned long interruptTime = millis();
    
    // Debounce: ignore interrupts that occur within 15ms of the last interrupt
    if (interruptTime - lastInterruptTime > ENCODER_ROTATION_DEBOUNCE) {
        if (instance) {
            int state = digitalRead(ENCODER_CLK);
            if (state != instance->lastState) { // If state has changed, pulse occurred
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

void IRAM_ATTR Encoder::handleButtonInterrupt() {
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

bool Encoder::isButtonPressed() {
    bool wasPressed = buttonPressed;
    buttonPressed = false;  // Reset after reading
    return wasPressed;
}

int Encoder::getPosition() {
    if (position > encoderMaxPosition) {
        setPosition(encoderMaxPosition);
    } else if (position < encoderMinPosition) {
        setPosition(encoderMinPosition);
    }
    return position;
}

void Encoder::setPosition(int pos) {
    position = pos;
}

void Encoder::setMaxPosition(int maxPos) {
    encoderMaxPosition = maxPos;
}

void Encoder::setMinPosition(int minPos) {
    encoderMinPosition = minPos;
}