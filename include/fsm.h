#pragma once

#include "main.h"

enum FSM_MAIN_STATES {
    INITAL,
    MAIN_MENU,
    CC,
    CV,
    CR,
    CW,
    SETTINGS,
    FINAL
};

/* -- CC Digits -- */
#define CC_DIGITS_BEFORE_DECIMAL 2
#define CC_DIGITS_AFTER_DECIMAL 3
#define CC_DIGITS_TOTAL (CC_DIGITS_BEFORE_DECIMAL + CC_DIGITS_AFTER_DECIMAL)
/* -- CV Digits -- */
#define CV_DIGITS_BEFORE_DECIMAL 3
#define CV_DIGITS_AFTER_DECIMAL 1
#define CV_DIGITS_TOTAL (CV_DIGITS_BEFORE_DECIMAL + CV_DIGITS_AFTER_DECIMAL)
/* -- CR Digits -- */
#define CR_DIGITS_BEFORE_DECIMAL 4
#define CR_DIGITS_AFTER_DECIMAL 4
#define CR_DIGITS_TOTAL (CR_DIGITS_BEFORE_DECIMAL + CR_DIGITS_AFTER_DECIMAL)
/* -- CW Digits -- */
#define CW_DIGITS_BEFORE_DECIMAL 3
#define CW_DIGITS_AFTER_DECIMAL 1
#define CW_DIGITS_TOTAL (CW_DIGITS_BEFORE_DECIMAL + CW_DIGITS_AFTER_DECIMAL)

class FSM {
public:
    // Constructor
    FSM();

    // Method to initialize the FSM
    void init();

    // Method to update the FSM
    void run(float input, DAC dac, AnalogSws sws);

    // Method to change the state
    void changeState(int newState);

    // Method to check if the state has changed
    bool hasChanged();

private:
    int currentState;
    int lastState;
};