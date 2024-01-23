#ifndef MAIN_H
#define MAIN_H

#define TAG "UART"

#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_timer.h>
#include <esp_check.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <inttypes.h>
#include <pthread.h>
#include <rom/ets_sys.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "EC200U.h"
#include "custom_irsend.h"
#include "custom_daikin.h"
#include "custom_hitachi.h"
#include "custom_NEC.h"
#include "custom_irrecv.h"
#include "custom_Samsung.h"
#include "CJSON.h"
#include "freertos/queue.h"
#include "mqtt.h"

#endif
