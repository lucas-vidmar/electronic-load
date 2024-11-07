#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI(TFT_HOR_RES , TFT_VER_RES);

#define TOTAL_PIXELS (TFT_HOR_RES * TFT_VER_RES) // 240*320 = 76800 pixels
#define BUFFER_SIZE_PIXELS (TOTAL_PIXELS / 10) // 76800 / 10 = 7680 pixels
#define COLOR_DEPH_BYTES (LV_COLOR_DEPTH / 8) // 16/8 = 2 bytes
#define DRAW_BUF_SIZE BUFFER_SIZE_PIXELS * COLOR_DEPH_BYTES // 7680 pixels * 2 bytes = 15360 bytes

// uint32_t has 32 bits = 4 bytes
uint32_t draw_buf[DRAW_BUF_SIZE / 4]; // 15360 bytes / 4 bytes per element = 3840 elements

void flush_lv(lv_display_t * display, const lv_area_t * area, uint8_t * px_map)
{
    uint16_t x_start = area->x1;
    uint16_t y_start = area->y1;
    uint16_t x_end = area->x2;
    uint16_t y_end = area->y2;

    uint16_t width = x_end - x_start + 1;
    uint16_t height = y_end - y_start + 1;

    Serial.println("x1: " + String(x_start) + " y1: " + String(y_start) +
                   " x2: " + String(x_end) + " y2: " + String(y_end));

    tft.startWrite();
    tft.setAddrWindow(x_start, y_start, width, height);
    tft.pushColors((uint16_t *)px_map, width * height, true);
    tft.endWrite();

    // Notifica a LVGL que el flushing ha terminado
    lv_display_flush_ready(display);
}


/*use ESP as tick source*/
static uint32_t tick(void)
{
    return esp_timer_get_time() / 1000;
}

void setup()
{
    // Wait 1 second before starting
    delay(1000);

    String LVGL_Arduino = "LGVL Version:";
    LVGL_Arduino += String('v') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.begin( 115200 );
    Serial.println( LVGL_Arduino );

    lv_init();

    /*Set a tick source so that LVGL will know how much time elapsed. */
    lv_tick_set_cb(tick);


    lv_display_t * disp;
    /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
    disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
    lv_display_set_rotation(disp, TFT_ROTATION);
    lv_display_set_flush_cb(disp, flush_lv);



    lv_obj_t *label = lv_label_create( lv_screen_active() );
    lv_label_set_text( label, "Probando rotacion" );
    lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

    Serial.println( "Setup done" );

    // Turn on the display backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
}

void loop()
{
    lv_timer_handler(); /* let the GUI do its work */
    delay(5); /* let this time pass */
}