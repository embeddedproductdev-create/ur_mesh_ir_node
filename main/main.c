/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "inc/main.h"
#include "inc/general.h"
#include "inc/button.h"
#include "inc/led.h"
#include "inc/lte.h"
#include "inc/heartbeat.h"
#include "inc/flash.h"
#include "inc/ir.h"

#define TAG "MAIN"

#define LTE_THREAD_STACK_SIZE 4096

TaskHandle_t button_task_handle = NULL;
TaskHandle_t lte_task_handle = NULL;
TaskHandle_t ir_recv_task_handle = NULL;

/*Global Flags Initialization*/
uint8_t newDevice = 1;
char serialNoStr[10] = "";
bool mqtt_connected = false;
uint8_t registered = 0;
uint8_t provisioned = 0;
uint8_t configured = 0;
bool sending_ir_command = false;
bool teaching_in_progress;
int16_t ir_protocol_num = 0;
char ir_protocol[20] = "";
uint16_t publishPeriod = MIN_PUBLISH_PERIOD_SEC;
CommandStruct last_command;
char device_location_str[LOCATION_STR_LEN] = "";
uint16_t teaching_mode_raw_len = 0;

/**
 * @brief Function that prints basic information about the device after fetching info from nvs flash
 */
void print_basic_info()
{
    ESP_LOGW(TAG, "=-=-=-=-=-=-=-=-=-=- BOOT SUCCESSFUL - %d.%d.%d -=-=-=-=-=-=-=-=-=-=", MAJ_VERSION, MIN_VERSION, PATCH_VERSION);
    ESP_LOGI(TAG, "%s : %d", NVS_NEW_DEVICE_KEY, newDevice);
    ESP_LOGI(TAG, "%s : %s", NVS_DEVICE_LOCATION_KEY, device_location_str);
    ESP_LOGI(TAG, "%s : %s", NVS_SERIAL_NO_KEY, serialNoStr);
    ESP_LOGI(TAG, "%s : %d", NVS_REGISTERED_KEY, registered);
    ESP_LOGI(TAG, "%s : %d", NVS_CONFIGURED_KEY, configured);
    ESP_LOGI(TAG, "%s : %d", NVS_PUBPERIOD_KEY, publishPeriod);
    ESP_LOGI(TAG, "%s : %d", NVS_IR_PROTOCOL_KEY, ir_protocol_num);
    ESP_LOGW(TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGW(TAG, "LAST AC-SETTINGS:");
    ESP_LOGI(TAG, "%s : %d",NVS_POWER_KEY, last_command.power);
    ESP_LOGI(TAG, "%s : %d",NVS_TEMPERATURE_KEY, last_command.temperature);
    ESP_LOGI(TAG, "%s : %d",NVS_FANSPEED_KEY, last_command.fanspeed);
    ESP_LOGI(TAG, "%s : %d",NVS_SWINGH_KEY, last_command.swingh);
    ESP_LOGI(TAG, "%s : %d",NVS_SWINGV_KEY, last_command.swingv);
    ESP_LOGI(TAG, "%s : %s",NVS_MODE_KEY, last_command.mode_str);
    ESP_LOGI(TAG, "%s : %d",NVS_ONTIMER_KEY, last_command.ontimer);
    ESP_LOGI(TAG, "%s : %d",NVS_OFFTIMER_KEY, last_command.offtimer);
    ESP_LOGI(TAG, "%s : %d",NVS_LOCKING_KEY, last_command.locking);
    ESP_LOGI(TAG, "%s : %d",NVS_UPPER_TEMPERATURE_LIMIT_KEY, last_command.upperTemperatureLimit);
    ESP_LOGI(TAG, "%s : %d",NVS_LOWER_TEMPERATURE_LIMIT_KEY, last_command.lowerTemperatureLimit);
    ESP_LOGW(TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
}

void app_main(void)
{
    nvs_init();
    strcpy(serialNoStr, "GWY00002");
    // print_chip_info();
    print_basic_info();
    button_intr_init();
    led_init();
    hb_init();
    ir_init();
    xTaskCreate(lte_task, "LTE Task", LTE_THREAD_STACK_SIZE, NULL, 2, &lte_task_handle);
}
