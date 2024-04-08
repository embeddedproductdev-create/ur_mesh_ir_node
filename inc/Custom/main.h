/**
 * @file main.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains the main functions and definitions
 * @version 0.1
 * @date 2024-02-29
 * @copyright Copyright (c) 2024
 */

#ifndef MAIN_H
#define MAIN_H

/* General */
#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

/* Arduino General */
#include <Arduino.h>

/* MQTT-LTE */
#include "../LTE/CJSON.h"
#include "../LTE/LTE.h"
#include "../LTE/mqtt.h"

/* BLE-MESH */
#include "../mesh/mesh_main.h"

/* ESP General */
#include "driver/adc.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "rom/ets_sys.h"
#include "sdkconfig.h"

/* Custom */
#include "LED.h"
#include "temperature_sensor.h"
#include "queue.h"

#define TAG "UART"
#define DEBUG_TAG "[DEBUG] : "
#define ERROR_TAG "[ERROR] : "
#define SETUP_TAG "[SETUP] : "
#define MAJ_VERSION 0
#define MIN_VERSION 2
#define BAUD_RATE 115200

#define IR_RECV_PART_ENABLED true
#define IR_RECV_LOG_ENABLED true
#define TEMPERATURE_SENSOR_PART_ENABLED true
#define LTE_PART_ENABLED true
#define BUTTON_PART_ENABLED true
#define LED_PART_ENABLED true
#define MESH_PART_ENABLED true
#define AP_PART_ENABLED false
#define TEACHING_PART_ENABLED true

/* GLOBAL VARIABLES */
extern bool configured;
extern bool mqtt_connected;
extern bool registered;
extern bool needtosend;
extern bool sending;
extern bool teaching_mode;
extern int16_t protocol_selected_num;
extern uint16_t GWY_SER_NO;


#ifdef __cplusplus
extern "C" {
#endif
/* FUNCTION DECLARATIONS */
void app_main();
void create_AP_task();

#ifdef __cplusplus
}
#endif

#endif



