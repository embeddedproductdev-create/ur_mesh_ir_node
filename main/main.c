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
#include "../inc/main.h"

TaskHandle_t button_task_handle = NULL;

void app_main(void)
{
    print_chip_info();
    button_intr_init();
    led_init();
    while(1){
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
