/**
 * @file lvgl_lcd.cpp
 * @brief Implementation of LVGL LCD interface.
 * @date 2023
 */

#include "lvgl_lcd.h"

TFT_eSPI* LVGL_LCD::tftPointer = nullptr;

LVGL_LCD::LVGL_LCD() : tft(TFT_HOR_RES, TFT_VER_RES) {}

void LVGL_LCD::init() {
    // Wait 1 second before initialization
    delay(1000);

    String lvglArduino = "LVGL Version: ";
    lvglArduino += String('v') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    Serial.println(lvglArduino);

    lv_init();

    /* Set a tick source so LVGL knows how much time has elapsed */
    lv_tick_set_cb(tick);

    /* TFT_eSPI can be enabled in lv_conf.h to initialize the screen easily */
    disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, drawBuf, sizeof(drawBuf));
    lv_display_set_rotation(disp, TFT_ROTATION);
    lv_display_set_flush_cb(disp, flush_lv);

    tftPointer = &tft;

    Serial.println("LVGL configuration completed");

    // Turn on the screen backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
}

void LVGL_LCD::update() {
    lv_timer_handler(); /* let the GUI do its work */
}

void LVGL_LCD::flush_lv(lv_display_t *display, const lv_area_t *area, uint8_t *pxMap) {
    uint16_t xStart = area->x1;
    uint16_t yStart = area->y1;
    uint16_t xEnd = area->x2;
    uint16_t yEnd = area->y2;

    uint16_t width = xEnd - xStart + 1;
    uint16_t height = yEnd - yStart + 1;

    tftPointer->startWrite();
    tftPointer->setAddrWindow(xStart, yStart, width, height);
    tftPointer->pushColors((uint16_t *)pxMap, width * height, true);
    tftPointer->endWrite();

    lv_display_flush_ready(display); // Notify LVGL that flushing is done
}

/*use ESP as tick source*/
uint32_t LVGL_LCD::tick() {
    return esp_timer_get_time() / 1000;
}

void LVGL_LCD::create_header(lv_obj_t* parent) {
    // Create header container
    headerContainer = lv_obj_create(parent);
    lv_obj_remove_style_all(headerContainer); // Remove default styles like padding
    lv_obj_set_size(headerContainer, lv_pct(100), LV_SIZE_CONTENT); // Full width, content height
    lv_obj_set_flex_flow(headerContainer, LV_FLEX_FLOW_ROW); // Arrange items horizontally
    lv_obj_set_flex_align(headerContainer, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER); // Space out items
    lv_obj_set_style_pad_bottom(headerContainer, PADDING / 2, 0); // Add some padding below

    // Temperature Label
    tempLabel = lv_label_create(headerContainer);
    lv_obj_set_style_text_font(tempLabel, FONT_S, 0);
    lv_label_set_text(tempLabel, "Temp: -- C");

    // Fan Speed Label
    fanLabel = lv_label_create(headerContainer);
    lv_obj_set_style_text_font(fanLabel, FONT_S, 0);
    lv_label_set_text(fanLabel, "Fan: -- %");
}

void LVGL_LCD::update_header(float temperature, int fan_speed) {
    if (tempLabel) lv_label_set_text(tempLabel, (String(temperature, 1) + " C").c_str()); // Format temperature to 1 decimal place
    if (fanLabel) lv_label_set_text_fmt(fanLabel, "Fan: %d %%", fan_speed); // Use %% for literal %
}

void LVGL_LCD::create_main_menu() {
    // Initialize styles if not initialized

    // Check if menu already exists
    if (mainMenu != nullptr) return;

    menuItems.clear(); // Clear vector if menu is recreated

    // Create container for the menu
    mainMenu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mainMenu, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL)); // Full screen
    lv_obj_align(mainMenu, LV_ALIGN_TOP_LEFT, 0, 0); // Position at top-left corner
    lv_obj_set_style_pad_all(mainMenu, PADDING, 0); // Add padding to the main container

    // Set container layout as column
    lv_obj_set_flex_flow(mainMenu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(mainMenu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // Set style for space between elements
    lv_obj_set_style_pad_gap(mainMenu, PADDING, 0);

    // Create the header at the top of the mainMenu container
    create_header(mainMenu); 

    // Add title with larger font (below the header)
    lv_obj_t* titleLabel = create_section_header("Main Menu", mainMenu, COLOR1_DARK);

    // Items for main menu
    items = {"Corriente Constante", "Voltaje Constante", "Resistencia Constante", "Potencia Constante", "Ajustes"};
    for (int i = 0; i < items.size(); ++i) {
        // Create a label for each item
        lv_obj_t* label = create_button(items[i], mainMenu, false, COLOR1_LIGHT);
        lv_obj_set_width(label, lv_pct(100)); // Make obj2 take the full width of parent
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0); // Center text to the left
        menuItems.push_back(label); // Add to the items vector
    }
}

void LVGL_LCD::update_main_menu(int hoveredOption) {
    // Update the highlighted option without recreating the menu
    for (int i = 0; i < menuItems.size(); ++i) {
        if (i == hoveredOption) {
            update_button(menuItems[i], true); // Highlighted
        } else {
            update_button(menuItems[i], false); // Normal
        }
    }
}

void LVGL_LCD::close_main_menu() {
    if (mainMenu != nullptr) {
        lv_obj_del(mainMenu); // Delete the main menu object (and its children, including header)
        mainMenu = nullptr; // Set pointer to nullptr to indicate it doesn't exist
        // Clear header pointers as they were children of mainMenu
        headerContainer = nullptr; 
        tempLabel = nullptr;
        fanLabel = nullptr;
    }
}

void LVGL_LCD::create_cx_screen(float current, int selection, String unit) {
    // Normal style
    lv_style_init(&styleValue);
    lv_style_set_text_color(&styleValue, lv_color_black()); // Common black

    // Highlighted style for hovered digits
    lv_style_init(&styleValueHovered);
    lv_style_set_text_color(&styleValueHovered, lv_color_hex(COLOR4_LIGHT)); // Highlighted in red

    // Check if current container already exists
    if (inputScreen != nullptr) return;
        
    // INPUT SCREEN
    inputScreen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(inputScreen, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL)); // Full screen
    lv_obj_align(inputScreen, LV_ALIGN_TOP_MID, 0, 0); // Position at top-middle
    lv_obj_set_flex_flow(inputScreen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(inputScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(inputScreen, PADDING, PADDING); // Padding

    // Create the header at the top of the inputScreen container
    create_header(inputScreen);

    // Input title (below header)
    inputTitle = create_section_header("Input", inputScreen, COLOR4_DARK);

    // Value container
    digits = lv_obj_create(inputScreen);
    lv_obj_set_width(digits, lv_pct(100)); // Parent width
    lv_obj_set_height(digits, LV_SIZE_CONTENT); // Height based on content

    lv_obj_set_flex_flow(digits, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(digits, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_border_color(digits, lv_color_hex(COLOR_GRAY), 0); // set border color gray
    
    // Button Container
    buttons = lv_obj_create(inputScreen);
    lv_obj_set_width(buttons, lv_pct(100)); // Parent width
    lv_obj_set_height(buttons, LV_SIZE_CONTENT); // Height based on content
    lv_obj_set_style_border_width(buttons, 0, 0); // No border
    lv_obj_set_style_pad_hor(buttons, 0, 0); // Horizontal padding
    lv_obj_set_style_pad_ver(buttons, 0, 0); // Vertical padding

    lv_obj_set_flex_flow(buttons, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(buttons, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(buttons, PADDING, 0);

    // Output button as label
    outputButton = create_button("Set", buttons, false, COLOR4_LIGHT);
    lv_obj_set_flex_grow(outputButton, 1);

    // Back button as label
    backButton = create_button("Back", buttons, false, COLOR4_LIGHT);
    lv_obj_set_flex_grow(backButton, 1);

    // Output title
    currentSelectionTitle = create_section_header("Output", inputScreen, COLOR1_DARK);

    // Output container
    curSelection = lv_obj_create(inputScreen);
    lv_obj_set_width(curSelection, lv_pct(100)); // Parent width
    lv_obj_set_height(curSelection, LV_SIZE_CONTENT); // Height based on content

    lv_obj_set_flex_flow(curSelection, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(curSelection, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_border_color(curSelection, lv_color_hex(COLOR_GRAY), 0); // set border color gray

    // Output label
    curSelectionLabel = lv_label_create(curSelection);
    lv_obj_add_style(curSelectionLabel, &styleValue, LV_PART_MAIN);
    lv_obj_set_style_text_font(curSelectionLabel, FONT_S, 0);

    // DUT Reading title
    outputTitle = create_section_header("DUT Reading", inputScreen, COLOR2_DARK);

    // DUT Container
    dutContainer = lv_obj_create(inputScreen);
    lv_obj_set_width(dutContainer, lv_pct(100)); // Parent width
    lv_obj_set_height(dutContainer, LV_SIZE_CONTENT); // Height based on content
    lv_obj_set_style_pad_gap(dutContainer, 0, 0);
    lv_obj_set_flex_flow(dutContainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(dutContainer, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(dutContainer, 0, 0); // Remove padding
    lv_obj_set_style_pad_gap(dutContainer, PADDING, 0); // spacing between objects
    lv_obj_set_style_border_width(dutContainer, 0, 0); // No border

    dutContainerRow1 = lv_obj_create(dutContainer);
    lv_obj_set_width(dutContainerRow1, lv_pct(100)); // Parent width
    lv_obj_set_height(dutContainerRow1, LV_SIZE_CONTENT); // Height based on content
    lv_obj_set_flex_flow(dutContainerRow1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dutContainerRow1, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(dutContainerRow1, 0, 0); // Remove padding
    lv_obj_set_style_pad_gap(dutContainerRow1, PADDING, 0); // spacing between objects
    lv_obj_set_style_border_width(dutContainerRow1, 0, 0); // No border

    dutContainerRow2 = lv_obj_create(dutContainer);
    lv_obj_set_width(dutContainerRow2, lv_pct(100)); // Parent width
    lv_obj_set_height(dutContainerRow2, LV_SIZE_CONTENT); // Height based on content
    lv_obj_set_flex_flow(dutContainerRow2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dutContainerRow2, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(dutContainerRow2, 0, 0); // Remove padding
    lv_obj_set_style_pad_gap(dutContainerRow2, PADDING, 0); // spacing between objects
    lv_obj_set_style_border_width(dutContainerRow2, 0, 0); // No border

    // DUT Voltage
    dutVoltage = create_button("V", dutContainerRow1, false, COLOR_GRAY);
    lv_obj_set_flex_grow(dutVoltage, 1);
    // DUT Current
    dutCurrent = create_button("A", dutContainerRow1, false, COLOR_GRAY);
    lv_obj_set_flex_grow(dutCurrent, 1);
    // DUT Power
    dutPower = create_button("W", dutContainerRow2, false, COLOR_GRAY);
    lv_obj_set_flex_grow(dutPower, 1);
    // DUT Resistance
    dutResistance = create_button("kR", dutContainerRow2, false, COLOR_GRAY);
    lv_obj_set_flex_grow(dutResistance, 1);
}

void LVGL_LCD::update_cx_screen(float current, int selection, String unit, float vDUT, float iDUT, int digitsBeforeDecimal, int totalDigits, String selected) {
    // Clean container before adding new digits
    String selectedStr = selected + " " + unit;
    lv_obj_clean(digits);

    // Convert the value to string with format "XX.XXX [X]"
    char valueStr[10];
    String format = "%0" + String(totalDigits+1) + "."+String(totalDigits-digitsBeforeDecimal)+"f " + String(unit);
    snprintf(valueStr, sizeof(valueStr), format.c_str(), current);
    //Serial.println("Value_str: " + String(valueStr));

    // Add each character as a separate label
    int hoveredDigitToProcess = selection;
    if (selection >= digitsBeforeDecimal) hoveredDigitToProcess++; // Skip decimal point

    for (int i = 0; valueStr[i] != '\0'; ++i) {

        lv_obj_t* label = lv_label_create(digits);
        lv_label_set_text_fmt(label, "%c", valueStr[i]);
        lv_obj_set_style_text_font(label, FONT_L, 0);

        // Apply highlighted style if character is a digit and is hovered
        if (isdigit(valueStr[i]) && i == hoveredDigitToProcess) {
            lv_obj_add_style(label, &styleValueHovered, LV_PART_MAIN);
        } else {
            lv_obj_add_style(label, &styleValue, LV_PART_MAIN);
        }
    }

    if (selection == totalDigits) update_button(outputButton, true); // Highlight output
    else update_button(outputButton, false); // Normal output
    if (selection == totalDigits + 1) update_button(backButton, true); // Highlight back
    else update_button(backButton, false); // Normal back

    // Update selected value
    lv_label_set_text(curSelectionLabel, selectedStr.c_str());

    // Update DUT values
    String values = String(vDUT,CV_DIGITS_AFTER_DECIMAL) + " V";
    lv_label_set_text_fmt(dutVoltage, values.c_str());
    values = String(iDUT,CC_DIGITS_AFTER_DECIMAL) + " A";
    lv_label_set_text_fmt(dutCurrent, values.c_str());
    values = String(vDUT*iDUT,CW_DIGITS_AFTER_DECIMAL) + " W";
    lv_label_set_text_fmt(dutPower, values.c_str());
    values = (iDUT > 0 ? String((vDUT/iDUT)/1000,CR_DIGITS_AFTER_DECIMAL) : "---") + + " kR";
    lv_label_set_text_fmt(dutResistance, values.c_str());
}

void LVGL_LCD::close_cx_screen(){
    if (inputScreen == nullptr) return; // Already deleted

    lv_obj_del(inputScreen); // Deletes inputScreen and all children (including header)
    inputScreen = nullptr;
    inputTitle = nullptr;
    digits = nullptr;
    buttons = nullptr; outputButton = nullptr; backButton = nullptr;
    curSelection = nullptr; curSelectionLabel = nullptr;
    dutContainer = nullptr; dutVoltage = nullptr; dutCurrent = nullptr; dutPower = nullptr; dutResistance = nullptr;
    // Clear header pointers as they were children of inputScreen
    headerContainer = nullptr; 
    tempLabel = nullptr;
    fanLabel = nullptr;

    lv_style_reset(&styleValue);
    lv_style_reset(&styleValueHovered);
}

lv_obj_t* LVGL_LCD::create_section_header(String label, lv_obj_t* parent, int color) {
    lv_obj_t* obj = lv_label_create(parent);

    lv_label_set_text(obj, label.c_str());
    lv_obj_set_width(obj, lv_pct(100)); // Set width to 100% of parent container
    lv_obj_set_style_pad_ver(obj, PADDING, 0);
    lv_obj_set_style_pad_hor(obj, PADDING, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0); // Opaque
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
    lv_obj_set_style_border_opa(obj, LV_OPA_COVER, 0); // Opaque border
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(obj, FONT_S, 0);

    update_button(obj, selected);

    return obj;
}

void LVGL_LCD::update_button(lv_obj_t* button, bool selected) {
    if (selected) {
        lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0); // Opaque
    }
    else {
        lv_obj_set_style_bg_opa(button, LV_OPA_TRANSP, 0); // Transparent
    }
}