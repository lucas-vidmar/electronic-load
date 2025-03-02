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

class FSM {
public:
    // Constructor
    FSM();

    // Method to initialize the FSM
    void init();

    // Method to update the FSM
    void run();

    // Method to change the state
    void changeState(int newState);

    // Method to check if the state has changed
    bool hasChanged();

private:
    int currentState;
    int lastState;
};