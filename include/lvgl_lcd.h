/**
 * @file lvgl_lcd.h
 * @brief Header file for LVGL LCD interface.
 * @note This file contains the class definition for interfacing with the LCD display using LVGL.
 * @date 2023
 */

#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <vector>
#include "fsm.h"

// Display buffer definitions
#define TOTAL_PIXELS (TFT_HOR_RES * TFT_VER_RES) // 240*320 = 76800 pixels
#define BUFFER_SIZE_PIXELS (TOTAL_PIXELS / 10) // 76800 / 10 = 7680 pixels
#define COLOR_DEPH_BYTES (LV_COLOR_DEPTH / 8) // 16/8 = 2 bytes
#define DRAW_BUF_SIZE BUFFER_SIZE_PIXELS * COLOR_DEPH_BYTES // 7680 pixels * 2 bytes = 15360 bytes
#define PADDING 10
#define ROUNDED_CORNER_CURVE 5
#define BORDER_WIDTH 2

// Color definitions - https://coolors.co/002642-99d5ff-3b6349-a1bbaa-6a5905-ecdd92-7e0909-f8a0a0
#define COLOR1_DARK 0x002642 // blue
#define COLOR1_LIGHT 0x99d5ff
#define COLOR2_DARK 0x3b6349 // green
#define COLOR2_LIGHT 0xa1bbaa
#define COLOR3_DARK 0x6a5905 // yellow
#define COLOR3_LIGHT 0xecdd92
#define COLOR4_DARK 0x7e0909 // red
#define COLOR4_LIGHT 0xf8a0a0
#define COLOR_GRAY 0xE0E0E0 // gray

// Font definitions
#define FONT_S &lv_font_montserrat_18
#define FONT_L &lv_font_montserrat_24

/**
 * @brief LVGL LCD interface class.
 * 
 * This class provides methods to initialize, update, and manage the LCD interface using LVGL.
 */
class LVGL_LCD {
public:
    /**
     * @brief Constructor for LVGL_LCD.
     */
    LVGL_LCD();

    /**
     * @brief Initialize the LCD interface.
     */
    void init();

    /**
     * @brief Update the LCD interface.
     * 
     * Handles LVGL tasks and updates the display.
     */
    void update();

    /**
     * @brief Create the main menu on the display.
     */
    void create_main_menu();
    
    /**
     * @brief Update the main menu with the currently selected option.
     * 
     * @param hoveredOption Index of the option currently selected.
     */
    void update_main_menu(int hoveredOption);
    
    /**
     * @brief Close and clean up the main menu.
     */
    void close_main_menu();

    /**
     * @brief Create a constant current/voltage/resistance/power screen.
     * 
     * @param current Initial value to display.
     * @param selection Currently selected digit position.
     * @param unit Unit of measurement (A, V, W, R).
     */
    void create_cx_screen(float current, int selection, String unit);
    
    /**
     * @brief Update the CX screen with new values.
     * 
     * @param current Current value to display.
     * @param selection Currently selected digit position.
     * @param unit Unit of measurement.
     * @param vDUT Voltage reading from device under test.
     * @param iDUT Current reading from device under test.
     * @param digitsBeforeDecimal Number of digits before decimal point.
     * @param totalDigits Total number of digits to display.
     * @param selected Label for the selected mode.
     */
    void update_cx_screen(float current, int selection, String unit, float vDUT, float iDUT, int digitsBeforeDecimal, int totalDigits, String selected);
    
    /**
     * @brief Close and clean up the CX screen.
     */
    void close_cx_screen();

    /**
     * @brief Update the header with temperature and fan speed.
     * 
     * @param temperature Current temperature value.
     * @param fan_speed Current fan speed percentage.
     */
    void update_header(float temperature, int fan_speed);

private:
    TFT_eSPI tft;
    static TFT_eSPI* tftPointer;
    lv_display_t *disp;
    uint32_t drawBuf[DRAW_BUF_SIZE / 4]; // uint32_t has 32 bits = 4 bytes || 15360 bytes / 4 bytes per element = 3840 elements

    /**
     * @brief LVGL flush callback for the TFT display.
     * 
     * @param display Pointer to the display.
     * @param area Area to flush.
     * @param pxMap Pixel map to flush.
     */
    static void flush_lv(lv_display_t *display, const lv_area_t *area, uint8_t *pxMap);

    /**
     * @brief Get tick value for LVGL.
     * 
     * @return uint32_t Current tick value in milliseconds.
     */
    static uint32_t tick();

    /* UI Elements for main_menu */
    lv_obj_t *mainMenu = nullptr, *buttonContainer = nullptr;      // Stores main menu object
    std::vector<lv_obj_t*> menuItems;  // Stores menu items
    std::vector<String> items;

    /* UI Elements for cx_screen */
    lv_obj_t* inputScreen = nullptr;   // Stores input screen object
    lv_style_t styleValue, styleValueHovered; // Styles for values
    lv_obj_t *inputTitle = nullptr, *currentSelectionTitle = nullptr, *outputTitle = nullptr,
    *digits = nullptr,
    *buttons = nullptr, *outputButton = nullptr, *backButton = nullptr, // Buttons
    *curSelection = nullptr, *curSelectionLabel = nullptr, // current selection
    *dutContainer = nullptr, *dutContainerRow1 = nullptr, *dutContainerRow2 = nullptr, // DUT container
    *dutVoltage = nullptr, *dutCurrent = nullptr, *dutPower = nullptr, *dutResistance = nullptr; // DUT values

    /* Common UI Elements */
    lv_obj_t *headerContainer = nullptr;
    lv_obj_t *tempLabel = nullptr;
    lv_obj_t *fanLabel = nullptr;

    /**
     * @brief Create a common header for screens.
     * 
     * @param parent The parent object for the header.
     */
    void create_header(lv_obj_t* parent);

    /**
     * @brief Create a section header for the UI.
     * 
     * @param label Text for the header.
     * @param parent Parent object.
     * @param color Background color.
     * @return lv_obj_t* Pointer to created header object.
     */
    lv_obj_t* create_section_header(String label, lv_obj_t* parent, int color);
    
    /**
     * @brief Create a button for the UI.
     * 
     * @param label Text for the button.
     * @param parent Parent object.
     * @param selected Whether the button is initially selected.
     * @param color Button color.
     * @return lv_obj_t* Pointer to created button object.
     */
    lv_obj_t* create_button(String label, lv_obj_t* parent, bool selected, int color);
    
    /**
     * @brief Update a button's selected state.
     * 
     * @param button Button to update.
     * @param selected Whether the button should be selected.
     */
    void update_button(lv_obj_t* button, bool selected);
};