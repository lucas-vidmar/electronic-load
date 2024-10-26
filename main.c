/* STD Libs */
#include <stdio.h>
#include <inttypes.h>
#include <stdio.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/gpio.h"

/* Parts */
#include "encoder.h"
#include "dac.h"
#include "i2c.h"
#include "analog_sws.h"
#include "led.h"
#include "rtc.h"
#include "adc.h"

void setup(void);
void print_esp_info(void);
void display_variables(void);

uint8_t value_in_mV = 0;
uint16_t adc_value = 0;

void app_main(void)
{
    printf("***************************\n* Electronic Load Project *\n***************************\n");
    print_esp_info();
    printf("***************************\n");
    esp_log_level_set("*", ESP_LOG_DEBUG); // Set all components to log level VERBOSE

    setup();
    ESP_LOGI("MAIN", "----- Finished Configurations -----");

    // Write 10mV to DAC
    ESP_LOGI("MAIN", "Setting DAC voltage to 0mV");
    ESP_ERROR_CHECK(dac_set_voltage(0));

    uint8_t last_value_in_mV = 0;
    uint16_t last_adc_value = 0;
    bool last_switch_state = false;

    while (1)
    {
        // Calculate and show DAC and ADC value in mV to set
        value_in_mV = encoder_getPosition();
        //ESP_ERROR_CHECK(adc_read(&adc_value));
             

        // Display when some variable changes
        if (last_value_in_mV != value_in_mV)
        {
            display_variables();
            last_value_in_mV = value_in_mV;
            last_adc_value = adc_value;
            last_switch_state = encoder_getSwitchState();
        }

        // When switch is pressed, DAC voltage is set
        if (encoder_getSwitchState()) 
        {
            ESP_LOGI("MAIN", "Setting DAC voltage to %d mV", value_in_mV);
            ESP_ERROR_CHECK(dac_set_voltage(value_in_mV));
            encoder_resetSwitchState();
        }

        vTaskDelay(10 / portTICK_PERIOD_MS); // feed the watchdog
        
    }
}

void setup(){

    // Initialize I2C
    ESP_LOGI("MAIN", "----- Initializing I2C master -----");
    ESP_ERROR_CHECK(i2c_master_init());

    // Initialize RTC
    ESP_LOGI("MAIN", "----- Initializing RTC -----");
    rtc_setup();

    // Initialize LED
    ESP_LOGI("MAIN", "----- Initializing LED -----");
    led_setup();

    // Initialize DAC
    ESP_LOGI("MAIN", "----- Initializing DAC -----");
    ESP_ERROR_CHECK(dac_setup());

    // Initialize ADC
    ESP_LOGI("MAIN", "----- Initializing ADC -----");
    adc_setup();

    // Initialize encoder
    ESP_LOGI("MAIN", "----- Initializing encoder -----");
    ESP_ERROR_CHECK(encoder_init());

    // Initialize analog switches
    ESP_LOGI("MAIN", "----- Initializing analog switches -----");
    ESP_ERROR_CHECK(analog_sws_setup());
    ESP_ERROR_CHECK(vdac_enable()); // Enable the DAC output voltage
    ESP_ERROR_CHECK(mosfet_input_cc_mode()); // Set the input mode to constant current
    ESP_ERROR_CHECK(relay_dut_enable()); // Enable the DUT
}

void print_esp_info(){

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

}

void display_variables(void){

    // Clear the terminal
    printf("\e[1;1H\e[2J");

    // Print the current voltage of the DAC
    printf("DAC voltage: %d mV\n", value_in_mV);

    // Print the current voltage of the ADC
    //adc_read(&adc_value);
    printf("ADC value: %d\n", adc_value);
}