#include "lvgl_lcd.h"

TFT_eSPI* LVGL_LCD::tft_pointer = nullptr;

LVGL_LCD::LVGL_LCD() : tft(TFT_HOR_RES, TFT_VER_RES) {}

void LVGL_LCD::init() {
    // Wait 1 second before starting
    delay(1000);

    String LVGL_Arduino = "LGVL Version: ";
    LVGL_Arduino += String('v') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    Serial.println(LVGL_Arduino);

    lv_init();

    /*Set a tick source so that LVGL will know how much time elapsed. */
    lv_tick_set_cb(tick);

    /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
    disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
    lv_display_set_rotation(disp, TFT_ROTATION);
    lv_display_set_flush_cb(disp, flush_lv);

    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Probando rotacion");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    tft_pointer = &tft;

    Serial.println("LVGL Setup done");

    // Turn on the display backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
}

void LVGL_LCD::update() {
    lv_timer_handler(); /* let the GUI do its work */
}


void LVGL_LCD::flush_lv(lv_display_t *display, const lv_area_t *area, uint8_t *px_map) {
    uint16_t x_start = area->x1;
    uint16_t y_start = area->y1;
    uint16_t x_end = area->x2;
    uint16_t y_end = area->y2;

    uint16_t width = x_end - x_start + 1;
    uint16_t height = y_end - y_start + 1;

    tft_pointer->startWrite();
    tft_pointer->setAddrWindow(x_start, y_start, width, height);
    tft_pointer->pushColors((uint16_t *)px_map, width * height, true);
    tft_pointer->endWrite();

    lv_display_flush_ready(display); // Notify LVGL that flushing is done
}

/*use ESP as tick source*/
uint32_t LVGL_LCD::tick() {
    return esp_timer_get_time() / 1000;
}
