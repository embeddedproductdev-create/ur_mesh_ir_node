#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"

#define BLE_NVS_NAMESPACE "BLE"
#define IR_NVS_NAMESPACE "IR"
#define GENERAL_NVS_NAMESPACE "GENERAL"

void nvs_init()
{
    
}