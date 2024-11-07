#pragma once

#include <stdint.h>
#include <lvgl.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#include "examples/lv_examples.h"

void lv_example_get_started_1(void);

void timer_handler(void);

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

static uint32_t my_tick(void);

void my_set_window(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

void my_draw_bitmaps(uint8_t *px_map, uint32_t size);

void tft_init(void);

void tft_fill_screen(uint32_t color);

void tft_set_rotation(uint32_t rotation);
