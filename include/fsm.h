/**
 * @file fsm.h
 * @brief Finite State Machine implementation for Electronic Load control
 * @note Handles different operation modes: CC, CV, CR, CW
 * @date 2023
 */

#pragma once

#include "main.h"

/**
 * @brief Main states for the Finite State Machine
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

/* -- CC Digits -- */
#define CC_DIGITS_BEFORE_DECIMAL 2
#define CC_DIGITS_AFTER_DECIMAL 2
#define CC_DIGITS_TOTAL (CC_DIGITS_BEFORE_DECIMAL + CC_DIGITS_AFTER_DECIMAL)
/* -- CV Digits -- */
#define CV_DIGITS_BEFORE_DECIMAL 3
#define CV_DIGITS_AFTER_DECIMAL 2
#define CV_DIGITS_TOTAL (CV_DIGITS_BEFORE_DECIMAL + CV_DIGITS_AFTER_DECIMAL)
/* -- CR Digits -- */
#define CR_DIGITS_BEFORE_DECIMAL 2
#define CR_DIGITS_AFTER_DECIMAL 3
#define CR_DIGITS_TOTAL (CR_DIGITS_BEFORE_DECIMAL + CR_DIGITS_AFTER_DECIMAL)
/* -- CW Digits -- */
#define CW_DIGITS_BEFORE_DECIMAL 2
#define CW_DIGITS_AFTER_DECIMAL 2
#define CW_DIGITS_TOTAL (CW_DIGITS_BEFORE_DECIMAL + CW_DIGITS_AFTER_DECIMAL)

/**
 * @brief Finite State Machine class for electronic load control
 */
class FSM {
public:
    /**
     * @brief Constructor for FSM
     */
    FSM();

    /**
     * @brief Initialize the FSM to default state
     */
    void init();

    /**
     * @brief Execute the FSM logic based on current state
     * @param input The input value (current, voltage, etc.)
     * @param dac DAC controller for setting output values
     * @param sws Analog switches controller for configuring hardware
     * @param output_active Pointer to the output active state
     */
    void run(float input, DAC dac, AnalogSws sws, bool* output_active);

    /**
     * @brief Change the current state of the FSM
     * @param newState The new state to transition to
     */
    void change_state(FSM_MAIN_STATES newState);

    /**
     * @brief Check if the state has changed since last check
     * @return true if state has changed, false otherwise
     */
    bool has_changed();

    /**
     * @brief Get the current state of the FSM
     * @return The current state
     */
    FSM_MAIN_STATES get_current_state();

private:
    FSM_MAIN_STATES currentState; ///< Current state of the FSM
    FSM_MAIN_STATES lastState;    ///< Previous state of the FSM
};