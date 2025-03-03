#include "fsm.h"

FSM::FSM() {}

void FSM::init() {
    currentState = FSM_MAIN_STATES::MAIN_MENU;
    lastState = FSM_MAIN_STATES::INITAL;
}

void FSM::run() {
    switch (currentState) {
        case FSM_MAIN_STATES::MAIN_MENU:
            main_menu();
            break;
        case FSM_MAIN_STATES::CC:
            constant_x(String("A"));
            break;
        case FSM_MAIN_STATES::CV:
            constant_x(String("V"));
            break;
        case FSM_MAIN_STATES::CR:
            constant_x(String("R"));
            break;
        case FSM_MAIN_STATES::CW:
            constant_x(String("W"));
            break;
        case FSM_MAIN_STATES::SETTINGS:
            break;
        default:
            break;
    }

}

void FSM::changeState(int newState) {
    if (newState < FSM_MAIN_STATES::INITAL || newState > FSM_MAIN_STATES::FINAL) return; // Invalid state
    lastState = currentState;
    currentState = newState;
}

bool FSM::hasChanged() {
    bool changed = lastState != currentState;
    lastState = currentState;
    return changed;
}