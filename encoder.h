/**
 * @file encoder.h
 * @brief Header file for encoder functionalities.
 *
 * This file contains the declarations for initializing the encoder, handling
 * interrupts, and managing encoder state and position.
 */
#pragma once

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/pulse_cnt.h>
#include <esp_log.h>

#define ENCODER_SW GPIO_NUM_25
#define ENCODER_CLK GPIO_NUM_32
#define ENCODER_DT GPIO_NUM_33
#define PULSE_FILTER 10000
#define DEBOUNCE_TIME 100

/**
 * @brief Initializes the encoder.
 *
 * This function sets up the necessary configurations for the encoder.
 *
 * @return esp_err_t Returns ESP_OK on success or an error code on failure.
 */
esp_err_t encoder_init();

/**
 * @brief Interrupt Service Routine (ISR) handler for the pulse counter.
 *
 * This function handles the pulse count interrupts.
 *
 * @param arg Pointer to the argument passed to the ISR.
 */
static void IRAM_ATTR encoder_pcnt_isr_handler(void *arg);

/**
 * @brief Interrupt handler for the encoder switch.
 *
 * This function handles the interrupts generated by the encoder switch.
 *
 * @param arg Pointer to the argument passed to the ISR.
 */
static void encoder_itr_sw(void *arg);

/**
 * @brief Gets the current position of the encoder.
 *
 * This function returns the current position value of the encoder.
 *
 * @return int The current position of the encoder.
 */
int encoder_getPosition();

/**
 * @brief Increments the encoder position.
 *
 * This function increments the current position of the encoder by one step.
 */
void encoder_increment();

/**
 * @brief Decrements the encoder position.
 *
 * This function decrements the current position of the encoder by one step.
 */
void encoder_decrement();

/**
 * @brief Gets the state of the encoder switch.
 *
 * This function returns the current state of the encoder switch.
 *
 * @return bool True if the switch is pressed, false otherwise.
 */
bool encoder_getSwitchState();