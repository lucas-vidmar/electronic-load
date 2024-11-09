#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <vector>

#define TOTAL_PIXELS (TFT_HOR_RES * TFT_VER_RES) // 240*320 = 76800 pixels
#define BUFFER_SIZE_PIXELS (TOTAL_PIXELS / 10) // 76800 / 10 = 7680 pixels
#define COLOR_DEPH_BYTES (LV_COLOR_DEPTH / 8) // 16/8 = 2 bytes
#define DRAW_BUF_SIZE BUFFER_SIZE_PIXELS * COLOR_DEPH_BYTES // 7680 pixels * 2 bytes = 15360 bytes
#define PADDING 10
#define TOTAL_DIGITS (DIGITS_BEFORE_DECIMAL + DIGITS_AFTER_DECIMAL)
#define DIGITS_BEFORE_DECIMAL 2
#define DIGITS_AFTER_DECIMAL 3

class LVGL_LCD {
public:
    LVGL_LCD();

    void init();

    void update();

    void print_main_menu(int hovered_option);
    void close_main_menu();

    void print_cx_screen(float current, int hovered_digit, char* unit, float vDUT, float iDUT);

private:
    TFT_eSPI tft;
    static TFT_eSPI* tft_pointer;
    lv_display_t *disp;
    uint32_t draw_buf[DRAW_BUF_SIZE / 4]; // uint32_t has 32 bits = 4 bytes || 15360 bytes / 4 bytes per element = 3840 elements

    static void flush_lv(lv_display_t *display, const lv_area_t *area, uint8_t *px_map);

    /*use ESP as tick source*/
    static uint32_t tick();

    lv_obj_t* main_menu = nullptr;      // Stores main menu object
    lv_obj_t* input_screen = nullptr;   // Stores input screen object
};