/**
 * @file main.h
 * @brief Main header for the Electronic Load application.
 *
 * This file contains global definitions, includes, Wi-Fi credentials,
 * WebSocket UI synchronization variables, and function declarations
 * for menu handling, measurement conversion, and WebSocket commands.
 *
 * @date 2025-04-25
 */

#pragma once

#include <Arduino.h>
#include <SPIFFS.h>
#include <vector>

#include "encoder.h"
#include "led.h"
#include "i2c.h"
#include "dac.h"
#include "analog_sws.h"
#include "adc.h"
#include "lvgl_lcd.h"
#include "fsm.h"
#include "webserver.h"
#include "I2CScanner.h"
#include "fan.h"
#include "rtc.h"
#include <ArduinoJson.h> // Include ArduinoJson

/* -- Wi-Fi Credentials -- */
const String SSID = "Electronic Load";        /*!< Wi-Fi network SSID */
const String PASSWORD = "pfi2025uade";        /*!< Wi-Fi network password */

/* -- Global Variables for WS/UI Sync -- */
extern float ws_requested_value;    /*!< Last requested value from WebSocket */
extern bool ws_value_updated;      /*!< Flag indicating a WebSocket value update */

#define BROADCAST_INTERVAL 1000 // Interval for broadcasting state updates (in milliseconds)

/* -- Safety Limits -- */
#define SAFETY_MAX_VOLTAGE 110.0    /*!< Maximum safe DUT voltage in volts */
#define SAFETY_MAX_CURRENT 22.0     /*!< Maximum safe DUT current in amperes */
#define SAFETY_MAX_POWER 220.0      /*!< Maximum safe DUT power in watts */
#define SAFETY_MAX_TEMPERATURE 66.0 /*!< Maximum safe temperature in degrees Celsius */

/* -- Function Declarations -- */
/**
 * @brief Displays and handles the main menu interface
 */
void main_menu();

/**
 * @brief Displays and handles the settings menu interface
 */
void setting();

/**
 * @brief Handles constant mode operation (CC, CV, etc.)
 * @param unit Unit of measurement (e.g., "A", "V")
 * @param digitsBeforeDecimal Number of digits before decimal point
 * @param digitsAfterDecimal Number of digits after decimal point
 * @param totalDigits Total number of digits for the value
 * @param maxInputValue Maximum input value for the mode
 */
void constant_x(String unit, int digitsBeforeDecimal, int digitsAfterDecimal, int totalDigits, int maxInputValue);

/**
 * @brief WebSocket event handler function.
 */
void on_ws_event(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

/**
 * @brief Handles commands received via WebSocket.
 * @param client The client that sent the command.
 * @param data The JSON payload containing the command and value.
 */
void handle_ws_command(AsyncWebSocketClient *client, JsonDocument& doc);

/**
 * @brief Handles the 'getMeasurements' command from WebSocket.
 * @param client The client requesting measurements.
 */
void handle_get_measurements(AsyncWebSocketClient *client);

/**
 * @brief Handles the 'setMode' command from WebSocket.
 * @param doc JSON document containing the new mode.
 */
void handle_set_mode(JsonDocument& doc);

/**
 * @brief Handles the 'setValue' command from WebSocket.
 * @param doc JSON document containing the new value.
 */
void handle_set_value(JsonDocument& doc);

/**
 * @brief Handles the 'setOutput' command from WebSocket.
 * @param doc JSON document containing the output state and value.
 */
void handle_set_output(JsonDocument& doc);

/**
 * @brief Handles the 'setRelay' command from WebSocket.
 * @param doc JSON document containing the relay state.
 */
void handle_set_relay(JsonDocument& doc);

/**
 * @brief Handles the 'exit' command from WebSocket.
 */
void handle_exit();

/**
 * @brief Gets the current state of the electronic load as a JSON string.
 * @return String containing the JSON representation of the current state.
 */
String get_current_state_json();

/**
 * @brief Sends the current state to all WebSocket clients.
 */
void broadcast_state();

/**
 * @brief Monitors DUT safety levels and automatically disconnects if limits are exceeded.
 * @return true if safety limits are exceeded and DUT was disconnected, false otherwise
 */
bool check_safety_limits();