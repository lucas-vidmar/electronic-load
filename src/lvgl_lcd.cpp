#include "lvgl_lcd.h"

TFT_eSPI* LVGL_LCD::tft_pointer = nullptr;

LVGL_LCD::LVGL_LCD() : tft(TFT_HOR_RES, TFT_VER_RES) {}

void LVGL_LCD::init() {
    // Esperar 1 segundo antes de iniciar
    delay(1000);

    String LVGL_Arduino = "LVGL Version: ";
    LVGL_Arduino += String('v') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    Serial.println(LVGL_Arduino);

    lv_init();

    /* Establecer una fuente global */
    //lv_obj_set_style_text_font(lv_scr_act(), &lv_font_montserrat_18, 0);

    /* Establecer una fuente de tick para que LVGL sepa cuánto tiempo ha transcurrido */
    lv_tick_set_cb(tick);

    /* TFT_eSPI se puede habilitar en lv_conf.h para inicializar la pantalla de manera sencilla */
    disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
    lv_display_set_rotation(disp, TFT_ROTATION);
    lv_display_set_flush_cb(disp, flush_lv);

    tft_pointer = &tft;

    Serial.println("Configuración de LVGL completada");

    // Encender la retroiluminación de la pantalla
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


void LVGL_LCD::print_main_menu(int hovered_option)
{
    // Inicializar estilos si no se han inicializado
    static bool styles_initialized = false;
    if (!styles_initialized) {
        // Estilo para los ítems normales del menú
        lv_style_init(&style_normal);
        lv_style_set_text_color(&style_normal, lv_color_black());
        lv_style_set_text_font(&style_normal, &lv_font_montserrat_18);

        // Estilo para el ítem resaltado
        lv_style_init(&style_hovered);
        lv_style_set_text_color(&style_hovered, lv_color_hex(0x0000FF));
        lv_style_set_text_font(&style_normal, &lv_font_montserrat_18);

        // Estilo para el título
        lv_style_init(&style_title);
        lv_style_set_text_color(&style_title, lv_color_black());
        lv_style_set_text_font(&style_title, &lv_font_montserrat_22); // Usar una fuente más grande para el título

        styles_initialized = true;
    }

    // Verificar si el menú ya existe
    if (main_menu == NULL) {
        menu_items.clear(); // Limpiar el vector si el menú se recrea

        // Crear un contenedor para el menú
        main_menu = lv_obj_create(lv_scr_act());
        lv_obj_set_size(main_menu, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
        // Posicionar en la esquina superior izquierda
        lv_obj_align(main_menu, LV_ALIGN_TOP_LEFT, 0, 0);

        // Establecer el layout del contenedor como columna
        lv_obj_set_flex_flow(main_menu, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(main_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

        // Establecer estilo para el espacio entre elementos
        lv_obj_set_style_pad_gap(main_menu, PADDING, 0);

        // Agregar título con fuente más grande
        lv_obj_t* title_label = lv_label_create(main_menu);
        lv_label_set_text(title_label, "Carga electronica");
        lv_obj_add_style(title_label, &style_title, LV_PART_MAIN);

        // Agregar un separador
        lv_obj_t* separator = lv_obj_create(main_menu);
        lv_obj_set_size(separator, lv_pct(100), 2); // Altura de 2 píxeles
        lv_obj_set_style_bg_color(separator, lv_color_black(), 0);
        lv_obj_set_style_border_width(separator, 0, 0);

        // Items para el menú principal
        std::vector<std::string> items = {"Corriente Constante", "Voltaje Constante", "Resistencia Constante", "Potencia Constante", "Ajustes"};
        for (int i = 0; i < items.size(); ++i) {
            // Crear una etiqueta para cada item
            lv_obj_t* label = lv_label_create(main_menu);
            lv_label_set_text(label, items[i].c_str());
            // Aplicar estilo normal
            lv_obj_add_style(label, &style_normal, LV_PART_MAIN);
            menu_items.push_back(label); // Almacenar cada etiqueta para actualizaciones
        }
    }

    // Actualizar la opción resaltada sin recrear el menú
    if (hovered_option > menu_items.size() - 1) hovered_option = menu_items.size() - 1; // Maintain the last option if the encoder is turned too much
    for (int i = 0; i < menu_items.size(); ++i) {
        if (i == hovered_option) {
            lv_obj_add_style(menu_items[i], &style_hovered, LV_PART_MAIN);
            lv_obj_remove_style(menu_items[i], &style_normal, LV_PART_MAIN);
            Serial.println("Resaltado: " + String(i));
        } else {
            lv_obj_add_style(menu_items[i], &style_normal, LV_PART_MAIN);
            lv_obj_remove_style(menu_items[i], &style_hovered, LV_PART_MAIN);
        }
    }
}