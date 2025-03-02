#pragma once

#include "main.h"

enum FSM_STATES {
    STATE_INITAL,
    STATE_MAIN_MENU,
    STATE_CONSTANT_CURRENT,
    STATE_CONSTANT_VOLTAGE,
    STATE_CONSTANT_RESISTANCE,
    STATE_CONSTANT_POWER,
    STATE_SETTINGS,
    STATE_EXIT
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