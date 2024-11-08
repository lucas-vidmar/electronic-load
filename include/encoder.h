/**
 * @file encoder.h
 * @brief Header file for the Encoder class.
 * 
 * This file contains the definition and documentation for the Encoder class, 
 * which provides functionality for interfacing with a rotary encoder. The 
 * Encoder class includes methods for initializing the encoder, checking button 
 * presses, retrieving and setting the encoder position, and handling interrupts.
 * 
 * @note This class assumes that the underlying hardware supports rotary encoders.
 * @note This class assumes only one encoder is used.
 * 
 * @date 2024-11-01
 */
#pragma once

#include <Arduino.h>

#define ENCODER_CLK GPIO_NUM_32
#define ENCODER_DT GPIO_NUM_33
#define ENCODER_SW GPIO_NUM_25
#define ENCODER_ROTATION_DEBOUNCE 15
#define ENCODER_BUTTON_DEBOUNCE 200
#define ENCODER_BUTTON_LONG_PRESS 1000

class Encoder {
public:

    /**
     * @brief Construct a new Encoder object.
     * 
     * This constructor initializes a new instance of the Encoder class.
     */
    Encoder();

    /**
     * @brief Initializes the encoder.
     * 
     * This function sets up the necessary configurations and initializes the encoder
     * for use. It should be called before any other encoder-related functions.
     */
    void init();

    /**
     * @brief Checks if the button is pressed.
     * 
     * This function returns a boolean value indicating whether the button
     * associated with the encoder is currently pressed.
     * 
     * @return true if the button is pressed, false otherwise.
     */
    bool isButtonPressed();

    /**
     * @brief Retrieves the current position of the encoder.
     * 
     * @return int The current position value of the encoder.
     */
    int getPosition();

    /**
     * @brief Sets the position of the encoder.
     * 
     * This function updates the current position of the encoder to the specified value.
     * 
     * @param pos The new position to set for the encoder.
     */
    void setPosition(int pos);

    /**
     * @brief Resets the state of the button.
     * 
     * This function is used to reset any state or counters associated with the button.
     * It should be called when the button needs to be reinitialized or cleared.
     */
    void resetButton();

    void setMaxPosition(int maxPos);

    void setMinPosition(int minPos);

private:

    /**
     * @brief Interrupt handler for the encoder.
     * 
     * This function is marked with IRAM_ATTR to ensure it is placed in the 
     * Internal RAM (IRAM) for faster execution. This is particularly important 
     * for interrupt service routines (ISRs) to minimize latency.
     */
    static void IRAM_ATTR handleInterrupt();

    /**
     * @brief Interrupt Service Routine (ISR) for handling button interrupts.
     * 
     * This function is marked with IRAM_ATTR to ensure it is placed in the 
     * IRAM (Instruction RAM) for faster execution. It is intended to be 
     * called when a button interrupt occurs, allowing for quick and 
     * efficient handling of the button press event.
     */
    static void IRAM_ATTR handleButtonInterrupt();

    static Encoder* instance;       // Global instance pointer
    volatile int lastState;         // State of the CLK pin
    volatile int position;          // Encoder position (marked volatile)
    volatile bool buttonPressed;    // Button press status (marked volatile)
    int encoderMaxPosition;         // Maximum position value for the encoder
    int encoderMinPosition;         // Minimum position value for the encoder
};
