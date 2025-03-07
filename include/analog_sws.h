
/**
 * @file analog_sws.h
 * @brief Header file for the AnalogSws class.
 * 
 * This file contains the definition of the AnalogSws class, which provides methods
 * to manage analog switches, including enabling/disabling VDAC, setting MOSFET input
 * modes, and controlling DUT relays.
 * 
 * @note This file is part of the Electronic Load project.
 * 
 * @date 2024-11-01
 */
#pragma once

#include <Arduino.h>

#define ANALOG_SW1_ENABLE GPIO_NUM_1    // ADG1219BRJZ-REEL Analog Switch | Analog SW1 Enable - DAC Enable
#define ANALOG_SW4_ENABLE GPIO_NUM_19   // ADG1334BRSZ-REEL Analog Switch | Analog SW4 Enable - Power
#define DUT_ENABLE GPIO_NUM_18          // AHES4191 Relay | DUT Enable

/**
 * @class AnalogSws
 * @brief A class to manage analog switches.
 * 
 * The AnalogSws class provides methods to initialize and control analog switches,
 * including enabling/disabling VDAC, setting MOSFET input modes, and controlling
 * DUT relays.
 */
class AnalogSws {
public:
    /**
     * @brief Constructor for the AnalogSws class.
     * 
     * Initializes an instance of the AnalogSws class.
     */
    AnalogSws();

    /**
     * @brief Initializes the analog switches.
     * 
     * This function sets up the necessary configurations for the analog switches
     * to operate correctly. It should be called during the system initialization
     * phase.
     */
    void init();

    /**
     * @brief Enables the VDAC (Voltage Digital-to-Analog Converter).
     *
     * This function initializes and enables the VDAC, allowing it to start
     * converting digital values to analog voltages.
     */
    void vDACEnable();

    /**
     * @brief Disables the VDAC (Voltage Digital-to-Analog Converter).
     * 
     * This function turns off the VDAC to save power or to stop generating an analog output.
     * It should be called when the VDAC is no longer needed.
     */
    void vDACDisable();

    /**
     * @brief Controls the MOSFET input in Constant Current (CC) mode.
     *
     * This function configures and manages the MOSFET to operate in Constant Current mode,
     * ensuring that the current remains stable and within the desired parameters.
     */
    void mosfetInputCCMode();

    /**
     * @brief Controls the MOSFET input in Constant Voltage (CV) mode.
     *
     * This function configures and manages the MOSFET to operate in a mode where
     * it maintains a constant voltage across its terminals. It is typically used
     * in electronic load applications to simulate a constant voltage load.
     */
    void mosfetInputCVMode();

    /**
     * @brief Retrieves the current input mode of the MOSFET.
     * 
     * This function returns the current input mode setting of the MOSFET, 
     * which determines how the MOSFET is being controlled or operated.
     * 
     * @return int The current input mode of the MOSFET.
     */
    int getMosfetInputMode();

    /**
     * @brief Enables the DUT (Device Under Test) relay.
     * 
     * This function activates the relay that connects the DUT to the circuit,
     * allowing it to be tested or used in the circuit.
     */
    void relayDUTEnable();

    /**
     * @brief Disables the DUT (Device Under Test) relay.
     *
     * This function is used to disable the relay connected to the DUT, 
     * effectively disconnecting the DUT from the circuit.
     */
    void relayDUTDisable();
};