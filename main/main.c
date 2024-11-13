/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../inc/general.h"
#include "../inc/button.h"
#include "../inc/led.h"
#include "../inc/lte.h"
#include "../inc/main.h"

#define LTE_THREAD_STACK_SIZE 4096

TaskHandle_t button_task_handle = NULL;
TaskHandle_t lte_task_handle = NULL;

bool mqtt_connected = false;
bool registered = false;
bool provisioned = false;
bool configured = false;
bool sending_ir_command = false;
bool teaching_in_progress;
char ir_protocol[20] = "";
uint16_t publishPeriod = MIN_PUBLISH_PERIOD_SEC;

void app_main(void)
{
    print_chip_info();
    button_intr_init();
    led_init();
    hb_init();
    xTaskCreate(lte_task, "LTE Task", LTE_THREAD_STACK_SIZE, NULL, 2, &lte_task_handle);
}
