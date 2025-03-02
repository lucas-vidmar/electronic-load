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
    static lv_style_t style_normal, style_hovered, style_title;
    static std::vector<lv_obj_t*> menu_items;
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

void LVGL_LCD::close_main_menu() {
    if (main_menu != nullptr) {
        lv_obj_del(main_menu); // Eliminar el objeto del menú principal
        main_menu = nullptr; // Establecer el puntero a nullptr para indicar que no existe
    }
}

void LVGL_LCD::print_cx_screen(float current, int hovered_digit, char* unit, float vDUT, float iDUT) {
    // Inicializar estilos si no se han inicializado
    static bool styles_initialized = false;
    static lv_style_t style_value, style_value_hovered;
    static lv_obj_t *input_title, *digits, *buttons, *output_button, *back_button, *dut_container, *dut_voltage, *dut_current, *dut_power;

    if (!styles_initialized) {
        // Estilo normal para los dígitos
        lv_style_init(&style_value);
        lv_style_set_text_color(&style_value, lv_color_black());
        lv_style_set_text_font(&style_value, &lv_font_montserrat_28);

        // Estilo resaltado para los dígitos hovered
        lv_style_init(&style_value_hovered);
        lv_style_set_text_color(&style_value_hovered, lv_color_hex(0xFF0000)); // Resaltado en rojo
        lv_style_set_text_font(&style_value_hovered, &lv_font_montserrat_28);

        styles_initialized = true;
    }

    // Verificar si el contenedor de corriente ya existe
    if (input_screen == nullptr) {
        // INPUT SCREEN
        input_screen = lv_obj_create(lv_scr_act());
        lv_obj_set_size(input_screen, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL)); // Full screen
        lv_obj_align(input_screen, LV_ALIGN_TOP_LEFT, 0, 0); // Posicionar en la esquina superior izquierda
        lv_obj_set_flex_flow(input_screen, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_all(input_screen, 0, 0); // Eliminar padding

        // Input title
        input_title = lv_label_create(input_screen);
        lv_label_set_text(input_title, "Input:");
        lv_obj_align(input_title, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_set_style_pad_hor(input_title, PADDING, 0);

        // Value container
        digits = lv_obj_create(input_screen);
        lv_obj_set_size(digits, lv_disp_get_hor_res(NULL), 75); // Altura de 50 píxeles
        lv_obj_align(digits, LV_ALIGN_TOP_MID, 0, 0); // Posicionar en la parte superior

        lv_obj_set_flex_flow(digits, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(digits, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        // Button Container
        buttons = lv_obj_create(input_screen);
        lv_obj_set_size(buttons, lv_disp_get_hor_res(NULL), 50); // Altura de 50 píxeles
        lv_obj_align(buttons, LV_ALIGN_TOP_MID, 0, 0); // Posicionar en la parte superior
        lv_obj_set_style_pad_gap(buttons, 0, 0);

        lv_obj_set_flex_flow(buttons, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(buttons, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        // Output button as label
        output_button = lv_label_create(buttons);
        lv_label_set_text(output_button, "Output");

        // Back button as label
        back_button = lv_label_create(buttons);
        lv_label_set_text(back_button, "Back");

        // Output title
        input_title = lv_label_create(input_screen);
        lv_label_set_text(input_title, "Output:");
        lv_obj_align(input_title, LV_ALIGN_TOP_LEFT, 0, 0);

        // DUT Container
        dut_container = lv_obj_create(input_screen);
        lv_obj_set_size(dut_container, lv_disp_get_hor_res(NULL), 100); // Altura de 50 píxeles
        lv_obj_align(dut_container, LV_ALIGN_TOP_MID, 0, 0); // Posicionar en la parte superior
        lv_obj_set_style_pad_gap(dut_container, 0, 0);
        lv_obj_set_flex_flow(dut_container, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(dut_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

        // DUT Voltage
        String values = String(vDUT,3) + " V";
        dut_voltage = lv_label_create(dut_container);
        lv_label_set_text_fmt(dut_voltage, values.c_str());
        lv_obj_set_style_text_font(dut_voltage, &lv_font_montserrat_18, 0);
        // DUT Current
        values = String(iDUT,3) + " A";
        dut_current = lv_label_create(dut_container);
        lv_label_set_text_fmt(dut_current, values.c_str());
        lv_obj_set_style_text_font(dut_current, &lv_font_montserrat_18, 0);
        // DUT Power
        values = String(vDUT*iDUT,3) + " W";
        dut_power = lv_label_create(dut_container);
        lv_label_set_text_fmt(dut_power, values.c_str());
        lv_obj_set_style_text_font(dut_power, &lv_font_montserrat_14, 0);

    }

    // Limpiar el contenedor antes de agregar nuevos dígitos
    lv_obj_clean(digits);

    // Convertir el valor de corriente a cadena con formato "XX.XXX A"
    char value_str[10];
    String format = "%0" + String(TOTAL_DIGITS+1) + ".3f " + String(unit);
    snprintf(value_str, sizeof(value_str), format.c_str(), current);

    // Agregar cada carácter como una etiqueta separada
    int hovered_digit_to_process = hovered_digit;
    if (hovered_digit > TOTAL_DIGITS-1) hovered_digit_to_process = TOTAL_DIGITS-1; // Limitar el número de dígitos
    if (hovered_digit >= DIGITS_BEFORE_DECIMAL) hovered_digit_to_process++; // Saltar el punto decimal

    for (int i = 0; value_str[i] != '\0'; ++i) {

        lv_obj_t* label = lv_label_create(digits);
        lv_label_set_text_fmt(label, "%c", value_str[i]);

        // Aplicar el estilo resaltado si el carácter es un dígito y es el hovered
        if (isdigit(value_str[i]) && i == hovered_digit_to_process) {
            lv_obj_add_style(label, &style_value_hovered, LV_PART_MAIN);
        } else {
            lv_obj_add_style(label, &style_value, LV_PART_MAIN);
        }
    }


    // Actualizar valores dut
    String values = String(vDUT,3) + " V";
    lv_label_set_text_fmt(dut_voltage, values.c_str());
    values = String(iDUT,3) + " A";
    lv_label_set_text_fmt(dut_current, values.c_str());
    values = String(vDUT*iDUT,3) + " W";
    lv_label_set_text_fmt(dut_power, values.c_str());
    
}
