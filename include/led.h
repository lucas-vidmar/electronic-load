#pragma once

#include <Arduino.h>

#define BUILT_IN_LED_PIN GPIO_NUM_2

/**
 * @class BuiltInLed
 * @brief A class to control the built-in LED on a microcontroller.
 * 
 * The BuiltInLed class provides methods to initialize and control the built-in LED
 * on a microcontroller. It includes functionality to initialize the LED hardware
 * and to make the LED blink at a specified interval.
 */
class BuiltInLed {
    
public:
    /**
     * @brief Constructor for the BuiltInLed class.
     * 
     * This constructor initializes the built-in LED on the microcontroller.
     */
    BuiltInLed();

    /**
     * @brief Initializes the LED hardware.
     * 
     * This function sets up the necessary configurations to initialize the LED hardware.
     * It should be called before any other LED-related functions are used.
     */
    void init();

    /**
     * @brief Blink an LED at a specified interval.
     * 
     * This function will cause the LED to blink on and off at the interval specified
     * by the parameter. The interval is given in milliseconds.
     * 
     * @param interval The time in milliseconds for the LED to stay on or off.
     */
    void blink(int interval);

private:
    hw_timer_t *timer;
    static void onTimer();
};