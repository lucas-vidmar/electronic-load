#include "fsm.h"

FSM::FSM() {}

void FSM::init() {
    currentState = FSM_MAIN_STATES::MAIN_MENU;
    lastState = FSM_MAIN_STATES::INITAL;
}

void FSM::run(float input, DAC dac, AnalogSws sws, bool* output_active, ADC adc) {

    if (*output_active) {
        sws.relay_dut_enable();
    } else {
        sws.relay_dut_disable();
    }

    static float lastInput = 0.0;

    switch (currentState) {
        case FSM_MAIN_STATES::MAIN_MENU:
            main_menu();
            if (lastInput != input) {
                sws.mosfet_input_cc_mode();
                sws.relay_dut_disable();
                dac.cc_mode_set_current(0.0);

                *output_active = false;
            }
            break;
        case FSM_MAIN_STATES::CC:
            constant_x(String("A"), CC_DIGITS_BEFORE_DECIMAL, CC_DIGITS_AFTER_DECIMAL, CC_DIGITS_TOTAL, DAC_CC_MAX_CURRENT);
            if (lastInput != input) {
                sws.mosfet_input_cc_mode();
                sws.v_dac_enable();
                dac.cc_mode_set_current(input);
            }
            break;
        case FSM_MAIN_STATES::CV:
            constant_x(String("V"), CV_DIGITS_BEFORE_DECIMAL, CV_DIGITS_AFTER_DECIMAL, CV_DIGITS_TOTAL, DAC_CV_MAX_VOLTAGE);
            if (lastInput != input) {
                sws.mosfet_input_cv_mode();
                sws.v_dac_enable();
                dac.cv_mode_set_voltage(input);
            }
            break;
        case FSM_MAIN_STATES::CR:
            constant_x(String("kR"), CR_DIGITS_BEFORE_DECIMAL, CR_DIGITS_AFTER_DECIMAL, CR_DIGITS_TOTAL, DAC_CR_MAX_RESISTANCE);
            if (lastInput != input) {
                sws.mosfet_input_cc_mode();
                sws.v_dac_enable();
            }
            dac.cr_mode_set_resistance(input, adc.read_v_dut());
            break;
        case FSM_MAIN_STATES::CW:
            constant_x(String("W"), CW_DIGITS_BEFORE_DECIMAL, CW_DIGITS_AFTER_DECIMAL, CW_DIGITS_TOTAL, DAC_CW_MAX_POWER);
            if (lastInput != input) {
                sws.mosfet_input_cc_mode();
                sws.v_dac_enable();
            }
            dac.cw_mode_set_power(input, adc.read_v_dut());
            break;
        case FSM_MAIN_STATES::SETTINGS:
            setting();
            break;
        default:
            break;
    }

    if (lastInput != input) lastInput = input;

}

void FSM::change_state(FSM_MAIN_STATES newState) {
    if (newState < FSM_MAIN_STATES::INITAL || newState > FSM_MAIN_STATES::FINAL) return;
    lastState = currentState;
    currentState = newState;
}

bool FSM::has_changed() {
    bool changed = lastState != currentState;
    lastState = currentState;
    return changed;
}

FSM_MAIN_STATES FSM::get_current_state() {
    return currentState;
}