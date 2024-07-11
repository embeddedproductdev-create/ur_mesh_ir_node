/**
 * @file main.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains the main functions and definitions
 * @version 0.1
 * @date 2024-06-19
 * @copyright Copyright (c) 2024
 */

#ifndef MAIN_H
#define MAIN_H

/* General */
#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
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
#define IS_GWY false
#define CLIENT_RELEASE false
/*====================================================================*/

#if(IS_GWY)
#define MAJ_VERSION 0
#define MIN_VERSION 8
#define INTERNAL_MIN_VERSION 6
#endif

#if(!IS_GWY)
#define MAJ_VERSION 0
#define MIN_VERSION 8
#define INTERNAL_MIN_VERSION 6
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

/* EEPROM DATA ADDRESSES */

/*General*/
#define SER_NO_IN_FLASH_ADDR_HI 0X0000
#define SER_NO_IN_FLASH_ADDR_MID (SER_NO_IN_FLASH_ADDR_HI+1)
#define SER_NO_IN_FLASH_ADDR_LO (SER_NO_IN_FLASH_ADDR_MID+1)
#define REGISTERED_FLAG_FLASH_ADDR (SER_NO_IN_FLASH_ADDR_LO+1)
#define CONFIGURED_FLAG_FLASH_ADDR (REGISTERED_FLAG_FLASH_ADDR+1)
#define PROTOCOL_SEL_FLASH_ADDR_HI (CONFIGURED_FLAG_FLASH_ADDR+1)
#define PROTOCOL_SEL_FLASH_ADDR_LO (PROTOCOL_SEL_FLASH_ADDR_HI+1)
#define HB_PUB_CONF_PERIOD_ADDR (PROTOCOL_SEL_FLASH_ADDR_LO+1)
/*AC Control Settings*/
#define POWER_FLASH_ADDR (HB_PUB_CONF_PERIOD_ADDR+1)
#define MODE_FLASH_ADDR (POWER_FLASH_ADDR+1)
#define FAN_FLASH_ADDR (MODE_FLASH_ADDR+1)
#define TEMPERATURE_FLASH_ADDR (FAN_FLASH_ADDR+1)
#define SWINGH_FLASH_ADDR (TEMPERATURE_FLASH_ADDR+1)
#define SWINGV_FLASH_ADDR (SWINGH_FLASH_ADDR+1)
#define ONTIMER_FLASH_ADDR (SWINGV_FLASH_ADDR+1)
#define OFFTIMER_FLASH_ADDR (ONTIMER_FLASH_ADDR+1)
#define LOCKING_FLASH_ADDR (OFFTIMER_FLASH_ADDR+1)
#define TEMPLOCKUPLIMIT_FLASH_ADDR (LOCKING_FLASH_ADDR+1)
#define TEMPLOCKLOWLIMIT_FLASH_ADDR (TEMPLOCKUPLIMIT_FLASH_ADDR+1)
#define RAWLEN_ADDR_HI (TEMPLOCKLOWLIMIT_FLASH_ADDR+1)
#define RAWLEN_ADDR_LO (RAWLEN_ADDR_HI+1)

/*Misc*/
#define FACTORY_DEVICE_CHECK_FLASH_ADDR  (TEMPLOCKLOWLIMIT_FLASH_ADDR+1)

/* GLOBAL VARIABLES */
extern bool show_boot_indication;

extern bool mqtt_connected;

extern bool registered;
extern bool provisioned;
extern uint8_t op_bind_counter;
extern bool configured;

extern bool needToSendIRComamnd;
extern bool sending;
extern bool teaching_mode;

extern bool esp_restart_flag;

extern int16_t protocol_selected_num;

extern uint32_t GWY_SER_NO;
extern char GWY_SER_NO_IN_STRING[15];
extern uint32_t NODE_SER_NO;
extern char NODE_SER_NO_IN_STRING[15];

#ifdef __cplusplus
extern "C" {
#endif

/* GLOBAL VARIABLES */

/*TaskHandles*/
// extern TaskHandle_t LTE_task_handle;
// extern TaskHandle_t LED_task_handle;
// extern TaskHandle_t queue_task_handle;
// extern TaskHandle_t button_task_handle;

/* FUNCTION DECLARATIONS */
void app_main();
void create_AP_task();
void fetch_from_flash();
void get_new_serial_no();

#ifdef __cplusplus
}
#endif

#endif



