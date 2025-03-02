#include "fsm.h"

FSM::FSM() {}

void FSM::init() {
    currentState = FSM_STATES::STATE_MAIN_MENU;
    lastState = FSM_STATES::STATE_INITAL;
}

void FSM::run() {
    switch (currentState) {
        case FSM_STATES::STATE_MAIN_MENU:
            main_menu();
            break;
        case FSM_STATES::STATE_CONSTANT_CURRENT:
            //constant_current();
            break;
        case FSM_STATES::STATE_CONSTANT_VOLTAGE:
            break;
        case FSM_STATES::STATE_CONSTANT_RESISTANCE:
            break;
        case FSM_STATES::STATE_CONSTANT_POWER:
            break;
        case FSM_STATES::STATE_SETTINGS:
            break;
        default:
            break;
    }

}

void FSM::changeState(int newState) {
    if (newState < FSM_STATES::STATE_INITAL || newState > FSM_STATES::STATE_EXIT) return; // Invalid state
    lastState = currentState;
    currentState = newState;
}

bool FSM::hasChanged() {
    bool changed = lastState != currentState;
    lastState = currentState;
    return changed;
}