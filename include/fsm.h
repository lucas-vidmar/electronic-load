/**
 * @file fsm.h
 * @brief Header file for the FSM class and state definitions.
 *
 * This file contains the declaration of the FSM class and its states,
 * which implement a finite state machine for controlling the electronic load
 * operation modes: Constant Current (CC), Constant Voltage (CV),
 * Constant Resistance (CR), and Constant Power (CW). It handles state
 * transitions and execution logic using DAC and analog switches, and reads
 * measurements via ADC.
 *
 * @note Ensure to call init() before run().
 *
 * @date 2025-05-02
 */
#pragma once

#include "main.h"

/**
 * @enum FSM_MAIN_STATES
 * @brief Main states for the finite state machine.
 */
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

// Digit counts for display in different modes
#define CC_DIGITS_BEFORE_DECIMAL 2
#define CC_DIGITS_AFTER_DECIMAL 2
#define CC_DIGITS_TOTAL (CC_DIGITS_BEFORE_DECIMAL + CC_DIGITS_AFTER_DECIMAL)
#define CV_DIGITS_BEFORE_DECIMAL 3
#define CV_DIGITS_AFTER_DECIMAL 2
#define CV_DIGITS_TOTAL (CV_DIGITS_BEFORE_DECIMAL + CV_DIGITS_AFTER_DECIMAL)
#define CR_DIGITS_BEFORE_DECIMAL 2
#define CR_DIGITS_AFTER_DECIMAL 3
#define CR_DIGITS_TOTAL (CR_DIGITS_BEFORE_DECIMAL + CR_DIGITS_AFTER_DECIMAL)
#define CW_DIGITS_BEFORE_DECIMAL 3
#define CW_DIGITS_AFTER_DECIMAL 2
#define CW_DIGITS_TOTAL (CW_DIGITS_BEFORE_DECIMAL + CW_DIGITS_AFTER_DECIMAL)

/**
 * @class FSM
 * @brief A finite state machine for electronic load control.
 *
 * The FSM class manages operation modes (CC, CV, CR, CW) of the electronic load.
 * It transitions between states, applies output settings via DAC and switches,
 * and reads inputs via ADC.
 */
class FSM {
public:
    /** @brief Constructor for FSM. */
    FSM();

    /** @brief Initialize the FSM to the default state. */
    void init();

    /**
     * @brief Execute the FSM logic for the current state.
     * @param input The target input value (current, voltage, etc.).
     * @param dac Reference to the DAC controller.
     * @param sws Reference to the AnalogSws controller.
     * @param output_active Pointer to the output active flag.
     * @param adc Reference to the ADC controller.
     */
    void run(float input, DAC dac, AnalogSws sws, bool* output_active, ADC adc);

    /**
     * @brief Change the current state of the FSM.
     * @param newState The state to transition to.
     */
    void change_state(FSM_MAIN_STATES newState);

    /** @brief Check if the state has changed since last check. */
    bool has_changed();

    /** @brief Get the current FSM state. */
    FSM_MAIN_STATES get_current_state();

private:
    FSM_MAIN_STATES currentState;  ///< Current state
    FSM_MAIN_STATES lastState;     ///< Previous state
};