/**
 * @file lvgl_lcd.cpp
 * @brief Implementation of LVGL LCD interface.
 * @date 2023
 */

#include "lvgl_lcd.h"
#include "main.h" // For SSID, PASSWORD
#include <WiFi.h>  // For WiFi.softAPIP()
#include "lvgl.h"

TFT_eSPI* LVGL_LCD::tftPointer = nullptr;

LVGL_LCD::LVGL_LCD() : tft(TFT_HOR_RES, TFT_VER_RES) {}

void LVGL_LCD::init() {
    // Wait 1 second before initialization
    delay(1000);

    String lvglArduino = "LVGL Version: ";
    lvglArduino += String('v') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    Serial.println("[LVGL_LCD] " + lvglArduino);

    Serial.println("[LVGL_LCD] Initializing LVGL library...");
    lv_init();

    /* Set a tick source so LVGL knows how much time has elapsed */
    lv_tick_set_cb(tick);

    /* TFT_eSPI can be enabled in lv_conf.h to initialize the screen easily */
    Serial.println("[LVGL_LCD] Creating display driver...");
    disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, drawBuf, sizeof(drawBuf));
    lv_display_set_rotation(disp, TFT_ROTATION);
    lv_display_set_flush_cb(disp, flush_lv);

    tftPointer = &tft;

    Serial.println("[LVGL_LCD] Configuration completed successfully");

    // Turn on the screen backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    Serial.println("[LVGL_LCD] Screen backlight enabled");
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

    // Fan Speed Label - Aligned to the left
    fanLabel = lv_label_create(headerContainer);
    lv_obj_set_style_text_font(fanLabel, FONT_S, 0);
    lv_label_set_text(fanLabel, "Fan: -- %");
    lv_obj_align(fanLabel, LV_ALIGN_LEFT_MID, 0, 0);


    // Uptime Label - Aligned to the right
    uptimeLabel = lv_label_create(headerContainer);
    lv_obj_set_style_text_font(uptimeLabel, FONT_S, 0);
    lv_label_set_text(uptimeLabel, "Up: 00:00:00"); // Initial text
    lv_obj_align(uptimeLabel, LV_ALIGN_RIGHT_MID, 0, 0);
}

void LVGL_LCD::update_header(float temperature, int fan_speed, const char* uptime) {
    if (uptimeLabel != nullptr) lv_label_set_text(uptimeLabel, uptime); // Update uptime text
    if (fanLabel != nullptr) lv_label_set_text_fmt(fanLabel, "Fan: %d %%", fan_speed); // Use %% for literal %
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
    items = {"Constant Current", "Constant Voltage", "Constant Resistance", "Constant Power", "Settings"};
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
        // tempLabel = nullptr; // Removed
        fanLabel = nullptr;
        uptimeLabel = nullptr; // Clear uptime label pointer
    }
}

void LVGL_LCD::create_cx_screen(int selection, String unit) {
    // Normal style
    lv_style_init(&styleDigitNormal);
    lv_style_set_text_color(&styleDigitNormal, lv_color_black()); // Common black
    lv_style_set_pad_hor(&styleDigitNormal, PADDING); // Horizontal padding
    lv_style_set_pad_ver(&styleDigitNormal, PADDING / 2); // Vertical padding
    lv_style_set_radius(&styleDigitNormal, ROUNDED_CORNER_CURVE); // Border radius
    lv_style_set_border_color(&styleDigitNormal, lv_color_hex(COLOR_GRAY)); // Set border color gray
    lv_style_set_border_width(&styleDigitNormal, 2); // Set border width

    // Highlighted style for hovered digits (selecting)
    lv_style_init(&styleDigitHover);
    lv_style_set_text_color(&styleDigitHover, lv_color_white()); // White text
    lv_style_set_bg_color(&styleDigitHover, lv_color_hex(COLOR4_LIGHT)); // Light Red background
    lv_style_set_bg_opa(&styleDigitHover, LV_OPA_COVER);
    lv_style_set_pad_hor(&styleDigitHover, PADDING); // Horizontal padding
    lv_style_set_pad_ver(&styleDigitHover, PADDING / 2); // Vertical padding
    lv_style_set_radius(&styleDigitHover, ROUNDED_CORNER_CURVE); // Border radius
    lv_style_set_border_color(&styleDigitHover, lv_color_hex(COLOR4_LIGHT)); // Set border color
    lv_style_set_border_width(&styleDigitHover, 2); // Set border width

    // Highlighted style for editing digits (modifying)
    lv_style_init(&styleDigitEditing);
    lv_style_set_bg_color(&styleDigitEditing, lv_color_hex(COLOR4_DARK)); // Dark Red background
    lv_style_set_text_color(&styleDigitEditing, lv_color_white()); // White text
    lv_style_set_bg_opa(&styleDigitEditing, LV_OPA_COVER);
    lv_style_set_pad_hor(&styleDigitEditing, PADDING); // Horizontal padding
    lv_style_set_pad_ver(&styleDigitEditing, PADDING / 2); // Vertical padding
    lv_style_set_radius(&styleDigitEditing, ROUNDED_CORNER_CURVE); // Border radius
    lv_style_set_border_color(&styleDigitEditing, lv_color_hex(COLOR4_DARK)); // Set border color
    lv_style_set_border_width(&styleDigitEditing, 2); // Set border width

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

    lv_obj_set_style_pad_all(digits, 0, 0); // Remove padding
    lv_obj_set_style_pad_gap(digits, PADDING/4, 0); // spacing between objects
    lv_obj_set_style_border_width(digits, 0, 0); // No border
    lv_obj_set_style_margin_ver(digits, PADDING, 0); // Vertical margin
    
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

    // DUT Reading title
    outputTitle = create_section_header("Readings", inputScreen, COLOR2_DARK);
    lv_obj_set_style_margin_top(outputTitle, PADDING, 0); // Add margin above the title

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

    dutContainerRow3 = lv_obj_create(dutContainer);
    lv_obj_set_width(dutContainerRow3, lv_pct(100)); // Parent width
    lv_obj_set_height(dutContainerRow3, LV_SIZE_CONTENT); // Height based on content
    lv_obj_set_flex_flow(dutContainerRow3, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dutContainerRow3, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(dutContainerRow3, 0, 0); // Remove padding
    lv_obj_set_style_pad_gap(dutContainerRow3, PADDING, 0); // spacing between objects
    lv_obj_set_style_border_width(dutContainerRow3, 0, 0); // No border

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
    // DUT Temperature
    dutTemperature = create_button("°C", dutContainerRow3, false, COLOR_GRAY);
    lv_obj_set_flex_grow(dutTemperature, 1);
    // DUT Energy
    dutEnergy = create_button("kJ", dutContainerRow3, false, COLOR_GRAY);
    lv_obj_set_flex_grow(dutEnergy, 1);
}

void LVGL_LCD::update_cx_screen(float current, int selection, String unit, float vDUT, float iDUT, int digitsBeforeDecimal, int totalDigits, String targetValueStr, bool output_active, bool is_modifying, float temperatureDUT, float energyDUT) {
    // Clean container before adding new digits
    lv_obj_clean(digits);

    // Convert the value being edited to string with format "XX.XXX [X]"
    char valueStr[15]; // Increased size slightly for safety
    // Use enough precision for display, potentially more than target decimals
    snprintf(valueStr, sizeof(valueStr), "%.*f", totalDigits - digitsBeforeDecimal, current);
    // Manually add padding if needed (e.g., leading zeros) - LVGL might handle this with label width/alignment

    // Add each character as a separate label
    int char_index = 0; // Tracks position in the formatted string
    int digit_index = 0; // Tracks the logical digit position (0 to totalDigits-1)
    bool decimal_passed = false;

    // Create labels for integer part
    String intPartStr = String((int)floor(current));
    int intDigits = intPartStr.length();
    // Add leading zeros if needed
    for(int i = 0; i < digitsBeforeDecimal - intDigits; ++i) {
        lv_obj_t* label = lv_label_create(digits);
        lv_label_set_text(label, "0");
        lv_obj_set_style_text_font(label, FONT_L, 0);
        if (digit_index == selection && !is_modifying) { // Highlight selected digit when SELECTING
            lv_obj_add_style(label, &styleDigitHover, LV_PART_MAIN);
        } else if (digit_index == selection && is_modifying) { // Editing the digit
            lv_obj_add_style(label, &styleDigitEditing, LV_PART_MAIN);
        }
        else {
            lv_obj_add_style(label, &styleDigitNormal, LV_PART_MAIN);
        }
        digit_index++;
    }
    // Add actual integer digits
    for(int i = 0; i < intDigits; ++i) {
        lv_obj_t* label = lv_label_create(digits);
        lv_label_set_text_fmt(label, "%c", intPartStr[i]);
        lv_obj_set_style_text_font(label, FONT_L, 0);
         if (digit_index == selection && !is_modifying) { // Highlight selected digit when SELECTING
             lv_obj_add_style(label, &styleDigitHover, LV_PART_MAIN);
        } else if (digit_index == selection && is_modifying) { // Editing the digit
             lv_obj_add_style(label, &styleDigitEditing, LV_PART_MAIN);
        } else {
            lv_obj_add_style(label, &styleDigitNormal, LV_PART_MAIN);
        }
        digit_index++;
    }

    // Add decimal point
    lv_obj_t* dotLabel = lv_label_create(digits);
    lv_label_set_text(dotLabel, ".");
    lv_obj_set_style_text_font(dotLabel, FONT_L, 0);
    lv_obj_add_style(dotLabel, &styleDigitNormal, LV_PART_MAIN); // Never highlighted
    lv_obj_set_style_pad_all(dotLabel, 0, 0); // Remove padding
    lv_obj_set_style_margin_hor(dotLabel, 2, 0); // Reduce horizontal margin
    lv_obj_set_style_margin_ver(dotLabel, 0, 0); // Remove vertical margin
    lv_obj_set_style_border_width(dotLabel, 0, 0); // No border

    // Add decimal part
    String decPartStr = String(round((current - floor(current)) * pow(10, totalDigits - digitsBeforeDecimal)), 0);
    int decDigits = decPartStr.length();
     // Add leading zeros for decimal part if needed (e.g., 0.01)
    // This part needs careful handling based on precision
    String formattedDecimal;
    snprintf(valueStr, sizeof(valueStr), "%.*f", totalDigits - digitsBeforeDecimal, current - floor(current));
    formattedDecimal = String(valueStr).substring(2); // Get digits after "0."

    for(int i = 0; i < totalDigits - digitsBeforeDecimal; ++i) {
         lv_obj_t* label = lv_label_create(digits);
         if (i < formattedDecimal.length()) {
            lv_label_set_text_fmt(label, "%c", formattedDecimal[i]);
         } else {
             lv_label_set_text(label, "0"); // Pad with trailing zeros if needed
         }
         lv_obj_set_style_text_font(label, FONT_L, 0);
         if (digit_index == selection && !is_modifying) { // Highlight selected digit when SELECTING
             lv_obj_add_style(label, &styleDigitHover, LV_PART_MAIN);
        } else if (digit_index == selection && is_modifying) { // Editing the digit
             lv_obj_add_style(label, &styleDigitEditing, LV_PART_MAIN);
        } else {
            lv_obj_add_style(label, &styleDigitNormal, LV_PART_MAIN);
        }
        digit_index++;
    }


    // Add Unit
    lv_obj_t* unitLabel = lv_label_create(digits);
    lv_label_set_text(unitLabel, unit.c_str());
    lv_obj_set_style_text_font(unitLabel, FONT_L, 0);
    lv_obj_add_style(unitLabel, &styleDigitNormal, LV_PART_MAIN); // Never highlighted
    lv_obj_set_style_pad_all(unitLabel, 0, 0); // Remove padding
    lv_obj_set_style_margin_hor(unitLabel, PADDING / 2, 0); // Reduce horizontal margin
    lv_obj_set_style_margin_ver(unitLabel, 0, 0); // Remove vertical margin
    lv_obj_set_style_border_width(unitLabel, 0, 0); // No border


    // Status indicator for output enable/disable
    enable_status_indicator = lv_obj_create(digits);
    lv_obj_set_size(enable_status_indicator, 15, 15); // Adjust size as needed
    lv_obj_set_style_radius(enable_status_indicator, LV_RADIUS_CIRCLE, 0); // Make it a circle.
    lv_obj_set_style_bg_opa(enable_status_indicator, LV_OPA_COVER, 0);     // Ensure background is fully opaque.
    lv_obj_set_style_border_width(enable_status_indicator, 0, 0);         // Explicitly set border width to 0 for all parts and states.
    // Note: The background color is set later based on output_active state.

    // Default/initial color will be set below based on output_active

    // Update button states and text
    // Selection: 0..totalDigits-1 = digits, totalDigits = Trigger/Stop, totalDigits+1 = Exit
    if (selection == totalDigits) { // Trigger/Stop button selected
        update_button(outputButton, true);
    } else {
        update_button(outputButton, false);
    }
    // Change text based on output state
    lv_label_set_text(outputButton, "Toggle");

    // Update enable status indicator color
    if (output_active) {
        lv_obj_set_style_bg_color(enable_status_indicator, lv_color_hex(COLOR2_DARK), 0); // Green for enabled
    } else {
        lv_obj_set_style_bg_color(enable_status_indicator, lv_color_hex(COLOR4_DARK), 0); // Red for disabled
    }

    if (selection == totalDigits + 1) { // Back button selected
        update_button(backButton, true);
    } else {
        update_button(backButton, false);
    }

    // Update DUT values
    String values = String(vDUT, CV_DIGITS_AFTER_DECIMAL) + " V";
    lv_label_set_text(dutVoltage, values.c_str()); // Use lv_label_set_text directly on the label child
    values = String(iDUT, CC_DIGITS_AFTER_DECIMAL) + " A";
    lv_label_set_text(dutCurrent, values.c_str());
    values = String(vDUT * iDUT, CW_DIGITS_AFTER_DECIMAL) + " W";
    lv_label_set_text(dutPower, values.c_str());
    values = (iDUT > 1e-6 ? String((vDUT / iDUT) / 1000.0, CR_DIGITS_AFTER_DECIMAL) : "---") + " kR"; // Check for near-zero current
    lv_label_set_text(dutResistance, values.c_str());
    values = String(temperatureDUT, 1) + " °C";
    lv_label_set_text(dutTemperature, values.c_str());
    values = String(energyDUT, 3) + " kJ";
    lv_label_set_text(dutEnergy, values.c_str());
}

void LVGL_LCD::close_cx_screen(){
    if (inputScreen == nullptr) return; // Already deleted

    lv_obj_del(inputScreen); // Deletes inputScreen and all children (including header)
    inputScreen = nullptr;
    inputTitle = nullptr;
    digits = nullptr;
    buttons = nullptr; outputButton = nullptr; backButton = nullptr; enable_status_indicator = nullptr;
    dutContainer = nullptr; dutVoltage = nullptr; dutCurrent = nullptr; dutPower = nullptr; dutResistance = nullptr; dutTemperature = nullptr; dutEnergy = nullptr;
    dutContainerRow1 = nullptr; dutContainerRow2 = nullptr; dutContainerRow3 = nullptr;
    // Clear header pointers as they were children of inputScreen
    headerContainer = nullptr; 
    // tempLabel = nullptr; // Removed
    fanLabel = nullptr;
    uptimeLabel = nullptr; // Clear uptime label pointer

    lv_style_reset(&styleDigitNormal);
    lv_style_reset(&styleDigitHover);
    lv_style_reset(&styleDigitEditing);
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

void LVGL_LCD::create_settings_menu() {
    if (settingsMenu != nullptr) return; // Already open

    settingsMenu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(settingsMenu, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_align(settingsMenu, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_pad_all(settingsMenu, PADDING, 0);
    lv_obj_set_flex_flow(settingsMenu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(settingsMenu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_gap(settingsMenu, PADDING * 2, 0); // Increased gap for more spacing

    // Header
    create_header(settingsMenu);
    lv_obj_t* titleLabel = create_section_header("Settings", settingsMenu, COLOR3_DARK);

    // SSID
    String ssidStr = "SSID: " + SSID;
    settingsSSIDLabel = lv_label_create(settingsMenu);
    lv_label_set_text(settingsSSIDLabel, ssidStr.c_str());
    lv_obj_set_style_text_font(settingsSSIDLabel, FONT_S, 0);
    lv_obj_set_width(settingsSSIDLabel, lv_pct(100));
    lv_obj_set_style_pad_ver(settingsSSIDLabel, PADDING/2, 0);

    // Password
    String passStr = "Password: " + PASSWORD;
    settingsPasswordLabel = lv_label_create(settingsMenu);
    lv_label_set_text(settingsPasswordLabel, passStr.c_str());
    lv_obj_set_style_text_font(settingsPasswordLabel, FONT_S, 0);
    lv_obj_set_width(settingsPasswordLabel, lv_pct(100));
    lv_obj_set_style_pad_ver(settingsPasswordLabel, PADDING/2, 0);

    // IP Address
    String ipStr = "IP: " + WiFi.softAPIP().toString();
    settingsIPLabel = lv_label_create(settingsMenu);
    lv_label_set_text(settingsIPLabel, ipStr.c_str());
    lv_obj_set_style_text_font(settingsIPLabel, FONT_S, 0);
    lv_obj_set_width(settingsIPLabel, lv_pct(100));
    lv_obj_set_style_pad_ver(settingsIPLabel, PADDING/2, 0);

    // Spacer
    lv_obj_t* spacer = lv_obj_create(settingsMenu);
    lv_obj_set_height(spacer, 20);
    lv_obj_set_style_border_width(spacer, 0, 0);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);

    // Back button
    settingsBackButton = create_button("Back", settingsMenu, true, COLOR3_DARK); // Use dark color for selected
    lv_obj_set_style_text_color(settingsBackButton, lv_color_white(), 0);
    lv_obj_set_width(settingsBackButton, lv_pct(100));
    lv_obj_set_style_pad_ver(settingsBackButton, PADDING, 0);
}

void LVGL_LCD::close_settings_menu() {
    if (settingsMenu == nullptr) return;
    lv_obj_del(settingsMenu);
    settingsMenu = nullptr;
    settingsBackButton = nullptr;
    settingsSSIDLabel = nullptr;
    settingsPasswordLabel = nullptr;
    settingsIPLabel = nullptr;
    uptimeLabel = nullptr; //
    headerContainer = nullptr; //
}

void LVGL_LCD::show_warning_popup(const String& message, uint32_t timeout_ms) {
    // Create a modal container (centered, with adaptive height)
    lv_obj_t* popup = lv_obj_create(lv_scr_act());
    
    // Set fixed width but let height adapt
    int popup_width = 240;
    lv_obj_set_width(popup, popup_width);
    lv_obj_set_style_radius(popup, ROUNDED_CORNER_CURVE, 0);
    lv_obj_set_style_bg_color(popup, lv_color_hex(COLOR4_DARK), 0);
    lv_obj_set_style_bg_opa(popup, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(popup, 2, 0);
    lv_obj_set_style_border_color(popup, lv_color_hex(COLOR4_LIGHT), 0);
    lv_obj_set_style_pad_all(popup, PADDING, 0);
    
    // Disable scrolling to prevent scrollbars
    lv_obj_clear_flag(popup, LV_OBJ_FLAG_SCROLLABLE);

    // Add warning label with text wrapping
    lv_obj_t* label = lv_label_create(popup);
    lv_label_set_text(label, message.c_str());
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, FONT_S, 0);
    lv_obj_set_width(label, popup_width - (PADDING * 2)); // Account for padding
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP); // Enable word wrapping at spaces
    
    // Let LVGL calculate the label height, then adjust popup height
    lv_obj_update_layout(label);
    int label_height = lv_obj_get_height(label);
    int popup_height = label_height + (PADDING * 2) + 4; // Add padding and small margin
    
    // Set the final popup size and center it
    lv_obj_set_height(popup, popup_height);
    lv_obj_center(popup);
    lv_obj_center(label);

    // Timer callback to delete popup
    struct TimerContext {
        lv_obj_t* obj;
    };
    TimerContext* ctx = new TimerContext{popup};
    auto timer_cb = [](lv_timer_t* timer) {
        TimerContext* ctx = (TimerContext*)lv_timer_get_user_data(timer);
        if(ctx && ctx->obj) lv_obj_del(ctx->obj);
        delete ctx;
        lv_timer_del(timer);
    };
    lv_timer_t* t = lv_timer_create(timer_cb, timeout_ms, ctx);
    lv_timer_set_repeat_count(t, 1);
}