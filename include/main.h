#pragma once

#include <Arduino.h>

#include "encoder.h"
#include "led.h"
#include "i2c.h"
#include "dac.h"
#include "analog_sws.h"
#include "adc.h"
#include "lvgl_lcd.h"
#include "fsm.h"

/* -- Functions -- */
void main_menu();
void constant_x(String unit, int digits_before_decimal, int digits_after_decimal, int total_digits);