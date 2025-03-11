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


void LVGL_LCD::create_main_menu() {
    // Inicializar estilos si no se han inicializado

    // Verificar si el menú ya existe
    if (main_menu != nullptr) return;

    menu_items.clear(); // Limpiar el vector si el menú se recrea

    // Crear un contenedor para el menú
    main_menu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_menu, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL)); // Full screen
    lv_obj_align(main_menu, LV_ALIGN_TOP_LEFT, 0, 0); // Posicionar en la esquina superior izquierda

    // Establecer el layout del contenedor como columna
    lv_obj_set_flex_flow(main_menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // Establecer estilo para el espacio entre elementos
    lv_obj_set_style_pad_gap(main_menu, PADDING, 0);

    // Agregar título con fuente más grande
    lv_obj_t* title_label = create_section_header("Main Menu", main_menu, COLOR1_DARK);

    // Items para el menú principal
    items = {"Corriente Constante", "Voltaje Constante", "Resistencia Constante", "Potencia Constante", "Ajustes"};
    for (int i = 0; i < items.size(); ++i) {
        // Crear una etiqueta para cada item
        lv_obj_t* label = create_button(items[i], main_menu, false, COLOR1_LIGHT);
        lv_obj_set_width(label, lv_pct(100)); // Hace que obj2 ocupe todo el ancho del padre
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0); // Centrar texto a la izquierda
        menu_items.push_back(label); // Agregar al vector de items
    }
}

void LVGL_LCD::update_main_menu(int hovered_option) {

    // Actualizar la opción resaltada sin recrear el menú
    for (int i = 0; i < menu_items.size(); ++i) {
        if (i == hovered_option) {
            update_button(menu_items[i], true); // Resaltado
        } else {
            update_button(menu_items[i], false); // Normal
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
    lv_style_set_text_color(&style_value_hovered, lv_color_hex(COLOR4_LIGHT)); // Resaltado en rojo

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
    input_title = create_section_header("Input",input_screen, COLOR4_DARK);

    // Value container
    digits = lv_obj_create(input_screen);
    lv_obj_set_width(digits, lv_pct(100)); // Ancho del padre
    lv_obj_set_height(digits, LV_SIZE_CONTENT); // Altura basada en el contenido

    lv_obj_set_flex_flow(digits, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(digits, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_border_color(digits, lv_color_hex(COLOR_GRAY), 0); // set border color gray
    
    // Button Container
    buttons = lv_obj_create(input_screen);
    lv_obj_set_width(buttons, lv_pct(100)); // Ancho del padre
    lv_obj_set_height(buttons, LV_SIZE_CONTENT); // Altura basada en el contenido
    lv_obj_set_style_border_width(buttons, 0, 0); // Sin borde
    lv_obj_set_style_pad_hor(buttons, 0, 0); // Padding horizontal
    lv_obj_set_style_pad_ver(buttons, 0, 0); // Padding vertical

    lv_obj_set_flex_flow(buttons, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(buttons, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(buttons, PADDING, 0);

    // Output button as label
    output_button = create_button("Set", buttons, false, COLOR4_LIGHT);
    lv_obj_set_flex_grow(output_button, 1);

    // Back button as label
    back_button = create_button("Back", buttons, false, COLOR4_LIGHT);
    lv_obj_set_flex_grow(back_button, 1);

    // Output title
    current_selection_title = create_section_header("Output", input_screen, COLOR1_DARK);

    // Output container
    cur_selection = lv_obj_create(input_screen);
    lv_obj_set_width(cur_selection, lv_pct(100)); // Ancho del padre
    lv_obj_set_height(cur_selection, LV_SIZE_CONTENT); // Altura basada en el contenido

    lv_obj_set_flex_flow(cur_selection, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cur_selection, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_border_color(cur_selection, lv_color_hex(COLOR_GRAY), 0); // set border color gray

    // Output label
    cur_selection_label = lv_label_create(cur_selection);
    lv_obj_add_style(cur_selection_label, &style_value, LV_PART_MAIN);
    lv_obj_set_style_text_font(cur_selection_label, FONT_S, 0);

    // DUT Reading title
    output_title = create_section_header("DUT Reading", input_screen, COLOR2_DARK);

    // DUT Container
    dut_container = lv_obj_create(input_screen);
    lv_obj_set_width(dut_container, lv_pct(100)); // Ancho del padre
    lv_obj_set_height(dut_container, LV_SIZE_CONTENT); // Altura basada en el contenido
    lv_obj_set_style_pad_gap(dut_container, 0, 0);
    lv_obj_set_flex_flow(dut_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(dut_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(dut_container, 0, 0); // Remove padding
    lv_obj_set_style_pad_gap(dut_container, PADDING, 0); // separación entre objetos
    lv_obj_set_style_border_width(dut_container, 0, 0); // Sin borde

    dut_container_row1 = lv_obj_create(dut_container);
    lv_obj_set_width(dut_container_row1, lv_pct(100)); // Ancho del padre
    lv_obj_set_height(dut_container_row1, LV_SIZE_CONTENT); // Altura basada en el contenido
    lv_obj_set_flex_flow(dut_container_row1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dut_container_row1, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(dut_container_row1, 0, 0); // Remove padding
    lv_obj_set_style_pad_gap(dut_container_row1, PADDING, 0); // separación entre objetos
    lv_obj_set_style_border_width(dut_container_row1, 0, 0); // Sin borde

    dut_container_row2 = lv_obj_create(dut_container);
    lv_obj_set_width(dut_container_row2, lv_pct(100)); // Ancho del padre
    lv_obj_set_height(dut_container_row2, LV_SIZE_CONTENT); // Altura basada en el contenido
    lv_obj_set_flex_flow(dut_container_row2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dut_container_row2, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(dut_container_row2, 0, 0); // Remove padding
    lv_obj_set_style_pad_gap(dut_container_row2, PADDING, 0); // separación entre objetos
    lv_obj_set_style_border_width(dut_container_row2, 0, 0); // Sin borde

    // DUT Voltage
    dut_voltage = create_button("V", dut_container_row1, false, COLOR_GRAY);
    lv_obj_set_flex_grow(dut_voltage, 1);
    // DUT Current
    dut_current = create_button("A", dut_container_row1, false, COLOR_GRAY);
    lv_obj_set_flex_grow(dut_current, 1);
    // DUT Power
    dut_power = create_button("W", dut_container_row2, false, COLOR_GRAY);
    lv_obj_set_flex_grow(dut_power, 1);
    // DUT Resistance
    dut_resistance = create_button("kR", dut_container_row2, false, COLOR_GRAY);
    lv_obj_set_flex_grow(dut_resistance, 1);
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
        lv_obj_set_style_text_font(label, FONT_L, 0);

        // Aplicar el estilo resaltado si el carácter es un dígito y es el hovered
        if (isdigit(value_str[i]) && i == hovered_digit_to_process) {
            lv_obj_add_style(label, &style_value_hovered, LV_PART_MAIN);
        } else {
            lv_obj_add_style(label, &style_value, LV_PART_MAIN);
        }
    }

    if (selection == total_digits) update_button(output_button, true); // Resaltado output
    else update_button(output_button, false); // Normal output
    if (selection == total_digits + 1) update_button(back_button, true); // Resaltado back
    else update_button(back_button, false); // Normal back

    // Actualizad valor de selected
    lv_label_set_text(cur_selection_label, selected_str.c_str());

    // Actualizar valores dut
    String values = String(vDUT,CV_DIGITS_AFTER_DECIMAL) + " V";
    lv_label_set_text_fmt(dut_voltage, values.c_str());
    values = String(iDUT,CC_DIGITS_AFTER_DECIMAL) + " A";
    lv_label_set_text_fmt(dut_current, values.c_str());
    values = String(vDUT*iDUT,CW_DIGITS_AFTER_DECIMAL) + " W";
    lv_label_set_text_fmt(dut_power, values.c_str());
    values = (iDUT > 0 ? String((vDUT/iDUT)/1000,CR_DIGITS_AFTER_DECIMAL) : "---") + + " kR";
    lv_label_set_text_fmt(dut_resistance, values.c_str());
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

lv_obj_t* LVGL_LCD::create_section_header(String label, lv_obj_t* parent, int color) {
    lv_obj_t* obj = lv_label_create(parent);

    lv_label_set_text(obj, label.c_str());
    lv_obj_set_width(obj, lv_pct(100)); // Establecer el ancho al 100% del contenedor padre
    lv_obj_set_style_pad_ver(obj, PADDING, 0);
    lv_obj_set_style_pad_hor(obj, PADDING, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0); // Opaco
    lv_obj_set_style_radius(obj, ROUNDED_CORNER_CURVE, 0);
    lv_obj_set_style_text_color(obj, lv_color_white(), 0);
    lv_obj_set_style_text_font(obj, FONT_S, 0);

    return obj;
}

lv_obj_t* LVGL_LCD::create_button(String label, lv_obj_t* parent, bool selected, int color) {
    lv_obj_t* obj = lv_label_create(parent);

    lv_label_set_text(obj, label.c_str());
    lv_obj_set_style_pad_ver(obj, PADDING, 0);
    lv_obj_set_style_pad_hor(obj, PADDING, 0);
    lv_obj_set_style_radius(obj, ROUNDED_CORNER_CURVE, 0); // Rounded corners
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0); // Center text
    lv_obj_set_style_border_color(obj, lv_color_hex(color), 0); // set border color
    lv_obj_set_style_border_width(obj, BORDER_WIDTH, 0); // set border width
    lv_obj_set_style_border_opa(obj, LV_OPA_COVER, 0); // borde Opaco
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(obj, FONT_S, 0);

    update_button(obj, selected);

    return obj;
}

void LVGL_LCD::update_button(lv_obj_t* button, bool selected) {
    if (selected) {
        lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0); // Opaco
    }
    else {
        lv_obj_set_style_bg_opa(button, LV_OPA_TRANSP, 0); // Transparente
    }
}