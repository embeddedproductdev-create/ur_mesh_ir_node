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
#include "printf_custom.h"

/* Flash */
#include "../flash/flash.h"

/**
 * ===================================================================
 * @brief VERY IMPORTANT MACROS !!!
 * DO NOT CHANGE THE THINGS BELOW WITHOUT KNOWING WHAT YOU ARE DOING
 * IT AFFECTS ACROSS THE WHOLE CODEBASE. YOU HAVE BEEN WARNED !!!
 * ===================================================================
 */
#define IS_GWY true
#define CLIENT_RELEASE false
/*====================================================================*/

#if(IS_GWY)
#define MAJ_VERSION 0
#define MIN_VERSION 6
#endif

#if(!IS_GWY)
#define MAJ_VERSION 0
#define MIN_VERSION 3
#endif

#define TAG "UART"
#define BAUD_RATE 115200

#define CORE0 0
#define CORE1 1

#define IR_RECV_PART_ENABLED true
#define IR_RECV_LOG_ENABLED true
#define TEMPERATURE_SENSOR_PART_ENABLED true
#define BUTTON_PART_ENABLED true
#define LED_PART_ENABLED true
#define MESH_PART_ENABLED true
#define TEACHING_PART_ENABLED true
#if(IS_GWY)
    #define LTE_PART_ENABLED true
    #define AP_PART_ENABLED false
    #define QUEUE_PART_ENABLED true
#endif
#if(!IS_GWY)
    #define AP_PART_ENABLED false
    #define QUEUE_PART_ENABLED false
    #define LTE_PART_ENABLED false
#endif

/* GLOBAL VARIABLES */
extern bool mqtt_connected;

extern bool registered;
extern bool provisioned;

extern bool configured;

extern bool needToSendIRComamnd;
extern bool sending;
extern bool teaching_mode;

extern bool esp_restart_flag;

extern int16_t protocol_selected_num;

extern uint16_t GWY_SER_NO;
extern char GWY_SER_NO_IN_STRING[15];
extern uint16_t NODE_SER_NO;
extern char NODE_SER_NO_IN_STRING[15];

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



