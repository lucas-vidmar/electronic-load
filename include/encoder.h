#pragma once

#include <Arduino.h>

#define ENCODER_CLK GPIO_NUM_32
#define ENCODER_DT GPIO_NUM_33
#define ENCODER_SW GPIO_NUM_25
#define ENCODER_ROTATION_DEBOUNCE 15
#define ENCODER_BUTTON_DEBOUNCE 50
#define ENCODER_BUTTON_LONG_PRESS 1000
#define ENCODER_MAX_POSITION 10
#define ENCODER_MIN_POSITION 0

class Encoder {
public:
    Encoder();
    void init();
    bool isButtonPressed();
    int getPosition();
    void setPosition(int pos);
    void resetButton();

    // ISR functions, marked with IRAM_ATTR
    static void IRAM_ATTR handleInterrupt();
    static void IRAM_ATTR handleButtonInterrupt();

private:
    static Encoder* instance;  // Global instance pointer
    volatile int lastState;     // State of the CLK pin
    volatile int position;      // Encoder position (marked volatile)
    volatile bool buttonPressed;  // Button press status (marked volatile)
};
