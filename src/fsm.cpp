#include "fsm.h"

FSM::FSM() {}

void FSM::init() {
    currentState = FSM_MAIN_STATES::MAIN_MENU;
    lastState = FSM_MAIN_STATES::INITAL;
}

void FSM::run(float input, DAC dac, AnalogSws sws) {

    static float last_input = 0.0;

    switch (currentState) {
        case FSM_MAIN_STATES::MAIN_MENU:
            main_menu();
            if (last_input != input) {
                sws.mosfetInputCCMode();
                sws.vDACDisable();
                sws.relayDUTDisable();

                dac.cc_mode_set_current(0.0);
            }
            break;
        case FSM_MAIN_STATES::CC:
            constant_x(String("A"), CC_DIGITS_BEFORE_DECIMAL, CC_DIGITS_AFTER_DECIMAL, CC_DIGITS_TOTAL);
            if (last_input != input) {
                sws.mosfetInputCCMode();
                sws.vDACEnable();
                dac.cc_mode_set_current(input);

                sws.relayDUTEnable();
            }
            break;
        case FSM_MAIN_STATES::CV:
            constant_x(String("V"), CV_DIGITS_BEFORE_DECIMAL, CV_DIGITS_AFTER_DECIMAL, CV_DIGITS_TOTAL);
            if (last_input != input) {
                sws.mosfetInputCVMode();
                sws.vDACEnable();
                dac.cv_mode_set_voltage(input);

                sws.relayDUTEnable();
            }
            break;
        case FSM_MAIN_STATES::CR:
            constant_x(String("kR"), CR_DIGITS_BEFORE_DECIMAL, CR_DIGITS_AFTER_DECIMAL, CR_DIGITS_TOTAL);
            break;
        case FSM_MAIN_STATES::CW:
            constant_x(String("W"), CW_DIGITS_BEFORE_DECIMAL, CW_DIGITS_AFTER_DECIMAL, CW_DIGITS_TOTAL);
            break;
        case FSM_MAIN_STATES::SETTINGS:
            break;
        default:
            break;
    }

    if (last_input != input) last_input = input; // Update latest input

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