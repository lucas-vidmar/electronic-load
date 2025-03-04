#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <vector>
#include "fsm.h"

#define TOTAL_PIXELS (TFT_HOR_RES * TFT_VER_RES) // 240*320 = 76800 pixels
#define BUFFER_SIZE_PIXELS (TOTAL_PIXELS / 10) // 76800 / 10 = 7680 pixels
#define COLOR_DEPH_BYTES (LV_COLOR_DEPTH / 8) // 16/8 = 2 bytes
#define DRAW_BUF_SIZE BUFFER_SIZE_PIXELS * COLOR_DEPH_BYTES // 7680 pixels * 2 bytes = 15360 bytes
#define PADDING 10
#define ROUNDED_CORNER_CURVE 5
#define BORDER_WIDTH 2

// https://coolors.co/002642-99d5ff-3b6349-a1bbaa-6a5905-ecdd92-7e0909-f8a0a0
#define COLOR1_DARK 0x002642 // blue
#define COLOR1_LIGHT 0x99d5ff
#define COLOR2_DARK 0x3b6349 // green
#define COLOR2_LIGHT 0xa1bbaa
#define COLOR3_DARK 0x6a5905 // yellow
#define COLOR3_LIGHT 0xecdd92
#define COLOR4_DARK 0x7e0909 // red
#define COLOR4_LIGHT 0xf8a0a0

class LVGL_LCD {
public:
    LVGL_LCD();

    void init();

    void update();

    void create_main_menu();
    void update_main_menu(int hovered_option);
    void close_main_menu();

    void create_cx_screen(float current, int selection, String unit);
    void update_cx_screen(float current, int selection, String unit, float vDUT, float iDUT, int digits_before_decimal, int total_digits, String selected);
    void close_cx_screen();

private:
    TFT_eSPI tft;
    static TFT_eSPI* tft_pointer;
    lv_display_t *disp;
    uint32_t draw_buf[DRAW_BUF_SIZE / 4]; // uint32_t has 32 bits = 4 bytes || 15360 bytes / 4 bytes per element = 3840 elements

    static void flush_lv(lv_display_t *display, const lv_area_t *area, uint8_t *px_map);

    /*use ESP as tick source*/
    static uint32_t tick();

    /* UI Elements for main_menu */
    lv_obj_t *main_menu, *button_container = nullptr;      // Stores main menu object
    std::vector<lv_obj_t*> menu_items;  // Stores menu items
    std::vector<String> items;

    /* UI Elements for cx_screen */
    lv_obj_t* input_screen = nullptr;   // Stores input screen object
    lv_style_t style_value, style_value_hovered; // Styles for values
    lv_obj_t *input_title, *current_selection_title, *output_title,
    *digits,
    *buttons, *output_button, *back_button, // Buttons
    *cur_selection, *cur_selection_label, // current selection
    *dut_container, *dut_container_row1, *dut_container_row2, // DUT container
    *dut_voltage, *dut_current, *dut_power, *dut_resistance = nullptr; // DUT values

    /* Helpers */
    lv_obj_t* create_section_header(String label, lv_obj_t* parent, int color); // Creates section headers
    lv_obj_t* create_button(String label, lv_obj_t* parent, bool selected, int color); // Creates button
    void update_button(lv_obj_t* button, bool selected); // Toggles button style
};