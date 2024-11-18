/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include <main.h>
#include <general.h>
#include <button.h>
#include <led.h>
#include <lte.h>
#include <heartbeat.h>
#include <flash.h>
#include <ir.h>
#include <ble.h>

#define TAG "MAIN"

#define LTE_THREAD_STACK_SIZE 4096
#define IR_THREAD_STACK_SIZE 4096

TaskHandle_t button_task_handle = NULL;
TaskHandle_t lte_task_handle = NULL;
TaskHandle_t ir_recv_task_handle = NULL;

/*Global Flags/Variables Initialization*/
uint8_t newDevice;
char serialNoStr[10];
bool mqtt_connected;
uint8_t registered;
uint8_t provisioned;
uint8_t configured;
bool sending_ir_command;
bool teaching_in_progress;
int16_t ir_protocol_num;
char ir_protocol[20];
uint16_t publishPeriod;
CommandStruct last_command;
char device_location_str[LOCATION_STR_LEN];
uint16_t teaching_mode_raw_len;

uint16_t teachingModeIrCmds[MAX_CMDS_IN_TEACHING_MODE][TEACHING_MODE_CMD_SIZE];

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
    ESP_LOGI(TAG, "%s : %ds", "Heartbeat Interval", publishPeriod);
    ESP_LOGI(TAG, "%s : %s", NVS_IR_PROTOCOL_KEY, ir_protocol);
    if(strcmp(ir_protocol, RAW_IR_PROTOCOL)==0) ESP_LOGI(TAG, "%s : %d", NVS_RAWLEN_KEY, teaching_mode_raw_len);
    ESP_LOGI(TAG, "%s : %s", "MQTT Publish Topic", publish_topic);
    ESP_LOGI(TAG, "%s : %s", "MQTT Subscribe Topic", subscribe_topic);
    ESP_LOGW(TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGW(TAG, "LAST AC-SETTINGS:");
    ESP_LOGI(TAG, "%s : %d", NVS_POWER_KEY, last_command.power);
    ESP_LOGI(TAG, "%s : %d", NVS_TEMPERATURE_KEY, last_command.temperature);
    ESP_LOGI(TAG, "%s : %d", NVS_FANSPEED_KEY, last_command.fanspeed);
    ESP_LOGI(TAG, "%s : %d", NVS_SWINGH_KEY, last_command.swingh);
    ESP_LOGI(TAG, "%s : %d", NVS_SWINGV_KEY, last_command.swingv);
    ESP_LOGI(TAG, "%s : %s", NVS_MODE_KEY, last_command.mode_str);
    ESP_LOGI(TAG, "%s : %d", NVS_ONTIMER_KEY, last_command.ontimer);
    ESP_LOGI(TAG, "%s : %d", NVS_OFFTIMER_KEY, last_command.offtimer);
    ESP_LOGI(TAG, "%s : %d", NVS_LOCKING_KEY, last_command.locking);
    ESP_LOGI(TAG, "%s : %d", NVS_UPPER_TEMPERATURE_LIMIT_KEY, last_command.upperTemperatureLimit);
    ESP_LOGI(TAG, "%s : %d", NVS_LOWER_TEMPERATURE_LIMIT_KEY, last_command.lowerTemperatureLimit);
    ESP_LOGW(TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGW(TAG, "%s : %d", "CommandStruct Size", sizeof(CommandStruct));
}

/**
 * @brief Function that initializes global variables to defaults before overwriting them with 
 * contents from flash in case of old device.
 * 
 */
void init_global_variables()
{
    newDevice = 1;
    mqtt_connected = false;
    registered = 0;
    provisioned = 0;
    configured = 0;
    sending_ir_command = false;
    teaching_in_progress = false;
    ir_protocol_num = -1;
    strcpy(ir_protocol, get_protocol_string(ir_protocol_num));
    publishPeriod = MIN_PUBLISH_PERIOD_SEC;
    strcpy(device_location_str, DEFAULT_DEVICE_LOCATION_STR);
    teaching_mode_raw_len = 0;
}

void app_main(void)
{
    init_global_variables();
    nvs_init();

    strcpy(serialNoStr, "GWY00002");
    sprintf(subscribe_topic, "%s/command", serialNoStr);
	sprintf(publish_topic, "%s/message", serialNoStr);

    print_basic_info();

    ble_init();
    led_init();
    hb_init();

    gpio_install_isr_service(0);
    button_intr_init();
    ir_tran_setup();

    xTaskCreate(lte_task, "LTE Task", LTE_THREAD_STACK_SIZE, NULL, 2, &lte_task_handle);
    xTaskCreate(ir_recv_task, "IR Recv Task", IR_THREAD_STACK_SIZE, NULL, 2, &ir_recv_task_handle);
}
