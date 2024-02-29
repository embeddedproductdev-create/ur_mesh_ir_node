/**
 * @file main.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains the main functions and definitions
 * @version 0.1
 * @date 2024-02-29
 * @copyright Copyright (c) 2024
 */

#ifdef __cplusplus
extern "C" {
#endif

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
#include <Wire.h>

/* IR part */
#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

/* MQTT-LTE */
#include <cJSON.h>
#include "LTE.h"

/* ESP General */
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"
#include "sdkconfig.h"

/* Custom */
#include ""

#define TAG "UART"
#define MAJ_VERSION 0
#define MIN_VERSION 1

/* FUNCTION DECLARATIONS */
void app_main(void);

#ifdef __cplusplus
}
#endif

#endif