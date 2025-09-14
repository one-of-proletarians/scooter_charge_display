#pragma once

#include <stdio.h>
#include <math.h>
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

esp_err_t ina_init();
float ina_get_voltage();
float ina_get_mV();
