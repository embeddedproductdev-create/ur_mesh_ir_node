#ifndef MAIN_H
#define MAIN_H

#include <cJSON.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "EC200U.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt.h"
#include "rom/ets_sys.h"
#include "sdkconfig.h"

/*Recviver part*/
#define kRecvPin 34
#define kBaudRate 115200
#define kCaptureBufferSize 1024
#define kTimeout 50
#define kMinUnknownSize 12
#define kTolerancePercentage kTolerance

/*Sending part*/
#define kSendPin 22

#define TAG "UART"

/*Function Declarations*/
void *recv_task(void *args);
void *send_task(void *args);
void *LTE_task(void *args);
void configure_sendFunc(void);

/*Global variables*/
extern bool configured;

#endif