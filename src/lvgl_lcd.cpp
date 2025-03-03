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

void LVGL_LCD::create_cx_screen(float current, int selection, String unit) {

    // Estilo normal
    lv_style_init(&style_value);
    lv_style_set_text_color(&style_value, lv_color_black()); // Negro comun

    // Estilo resaltado para los dígitos hovered
    lv_style_init(&style_value_hovered);
    lv_style_set_text_color(&style_value_hovered, lv_color_hex(0xFF0000)); // Resaltado en rojo

    // Verificar si el contenedor de corriente ya existe
    if (input_screen != nullptr) return;
        
    // INPUT SCREEN
    input_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(input_screen, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL)); // Full screen
    lv_obj_align(input_screen, LV_ALIGN_TOP_MID, 0, 0); // Posicionar en la esquina superior izquierda
    lv_obj_set_flex_flow(input_screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(input_screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(input_screen, PADDING, PADDING); // Padding

    // Input title
    input_title = lv_label_create(input_screen);
    lv_label_set_text(input_title, "Input:");

    // Value container
    digits = lv_obj_create(input_screen);
    lv_obj_set_size(digits, lv_pct(100), 75); // Altura de 75 píxeles, ancho del padre

    lv_obj_set_flex_flow(digits, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(digits, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Button Container
    buttons = lv_obj_create(input_screen);
    lv_obj_set_size(buttons, lv_pct(100), 55); // Altura de 55 píxeles, ancho del padre
    lv_obj_set_style_pad_gap(buttons, 0, 0);

    lv_obj_set_flex_flow(buttons, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(buttons, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Output button as label
    output_button = lv_label_create(buttons);
    lv_label_set_text(output_button, "Push");
    lv_obj_set_style_text_font(output_button, &lv_font_montserrat_18, 0);

    // Back button as label
    back_button = lv_label_create(buttons);
    lv_label_set_text(back_button, "Back");
    lv_obj_set_style_text_font(back_button, &lv_font_montserrat_18, 0);

    // Current selection title
    input_title = lv_label_create(input_screen);
    lv_label_set_text(input_title, "Current Selection:");

    // Current selection container
    cur_selection = lv_obj_create(input_screen);
    lv_obj_set_size(cur_selection, lv_pct(100), 75); // Altura de 75 píxeles, ancho del padre

    lv_obj_set_flex_flow(cur_selection, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cur_selection, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Current selection label
    cur_selection_label = lv_label_create(cur_selection);
    lv_obj_add_style(cur_selection_label, &style_value, LV_PART_MAIN);
    lv_obj_set_style_text_font(cur_selection_label, &lv_font_montserrat_28, 0);

    // Output title
    input_title = lv_label_create(input_screen);
    lv_label_set_text(input_title, "Output:");

    // DUT Container
    dut_container = lv_obj_create(input_screen);
    lv_obj_set_size(dut_container, 150, 100); // Altura de 100 píxeles
    lv_obj_set_style_pad_gap(dut_container, 0, 0);
    lv_obj_set_flex_flow(dut_container, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(dut_container, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);

    // DUT Voltage
    dut_voltage = lv_label_create(dut_container);
    lv_obj_set_style_text_font(dut_voltage, &lv_font_montserrat_18, 0);
    // DUT Current
    dut_current = lv_label_create(dut_container);
    lv_obj_set_style_text_font(dut_current, &lv_font_montserrat_18, 0);
    // DUT Power
    dut_power = lv_label_create(dut_container);
    lv_obj_set_style_text_font(dut_power, &lv_font_montserrat_14, 0);
    // DUT Resistance
    dut_resistance = lv_label_create(dut_container);
    lv_obj_set_style_text_font(dut_resistance, &lv_font_montserrat_14, 0);
}

void LVGL_LCD::update_cx_screen(float current, int selection, String unit, float vDUT, float iDUT, int digits_before_decimal, int total_digits, String selected) {
    // Limpiar el contenedor antes de agregar nuevos dígitos
    String selected_str = selected + " " + unit;
    lv_obj_clean(digits);

    // Convertir el valor a string con formato "XX.XXX [X]"
    char value_str[10];
    String format = "%0" + String(total_digits+1) + "."+String(total_digits-digits_before_decimal)+"f " + String(unit);
    snprintf(value_str, sizeof(value_str), format.c_str(), current);
    //Serial.println("Value_str: " + String(value_str));

    // Agregar cada carácter como una etiqueta separada
    int hovered_digit_to_process = selection;
    if (selection >= digits_before_decimal) hovered_digit_to_process++; // Saltar el punto decimal

    for (int i = 0; value_str[i] != '\0'; ++i) {

        lv_obj_t* label = lv_label_create(digits);
        lv_label_set_text_fmt(label, "%c", value_str[i]);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);

        // Aplicar el estilo resaltado si el carácter es un dígito y es el hovered
        if (isdigit(value_str[i]) && i == hovered_digit_to_process) {
            lv_obj_add_style(label, &style_value_hovered, LV_PART_MAIN);
        } else {
            lv_obj_add_style(label, &style_value, LV_PART_MAIN);
        }
    }

    if (selection == total_digits) lv_obj_add_style(output_button, &style_value_hovered, LV_PART_MAIN); // Resaltado output
    else lv_obj_add_style(output_button, &style_value, LV_PART_MAIN); // Normal output
    if (selection == total_digits + 1) lv_obj_add_style(back_button, &style_value_hovered, LV_PART_MAIN); // Resaltado back
    else lv_obj_add_style(back_button, &style_value, LV_PART_MAIN); // Normal back

    // Actualizad valor de selected
    lv_label_set_text(cur_selection_label, selected_str.c_str());

    // Actualizar valores dut
    String values = String(vDUT,CV_DIGITS_AFTER_DECIMAL) + " V";
    lv_label_set_text_fmt(dut_voltage, values.c_str());
    values = String(iDUT,CC_DIGITS_AFTER_DECIMAL) + " A";
    lv_label_set_text_fmt(dut_current, values.c_str());
    values = String(vDUT*iDUT,CW_DIGITS_AFTER_DECIMAL) + " W";
    lv_label_set_text_fmt(dut_power, values.c_str());
    values = (iDUT > 0 ? String(vDUT/iDUT,CR_DIGITS_AFTER_DECIMAL) : "---") + + " R";
    lv_label_set_text_fmt(dut_resistance, values.c_str());
    lv_obj_set_style_text_font(dut_resistance, &lv_font_montserrat_14, 0);
}

void LVGL_LCD::close_cx_screen(){
    if (input_screen == nullptr) return; // Ya se ha eliminado

    lv_obj_del(input_screen);
    input_screen = nullptr;
    input_title = nullptr;
    digits = nullptr;
    buttons = nullptr; output_button = nullptr; back_button = nullptr;
    cur_selection = nullptr; cur_selection_label = nullptr;
    dut_container = nullptr; dut_voltage = nullptr; dut_current = nullptr; dut_power = nullptr; dut_resistance = nullptr;

    lv_style_reset(&style_value);
    lv_style_reset(&style_value_hovered);
}