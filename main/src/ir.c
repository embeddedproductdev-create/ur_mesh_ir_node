#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"

#include "../inc/ir.h"

#define IR_TAG "IR"
#define IR_RECV_GPIO 38
#define IR_TRAN_GPIO 7

/**
 * @brief Interrupt handler function that gets triggered when we
 * recv an IR signal
 * 
 */
void IRAM_ATTR irRecvHandler(void *args)
{
    gpio_isr_handler_remove(IR_RECV_GPIO);
    if (xTaskCreate(ir_recv_task, "ir_recv_task", 4096, NULL, 10, &ir_recv_task_handle) != pdPASS) {
        ESP_LOGE(IR_TAG, "Failed to create ir_recv_task");
        gpio_isr_handler_add(IR_RECV_GPIO, irRecvHandler, NULL);
    }
}

/**
 * @brief Thread that takes care of IR Decoding
 *  
 */
void ir_recv_task(void *args)
{
    ESP_LOGI(IR_TAG, "IR SIGNAL RECEIVED");
    vTaskDelay(pdMS_TO_TICKS(3000));
    gpio_isr_handler_add(IR_RECV_GPIO, irRecvHandler, NULL);
    vTaskDelete(NULL);
}

/**
 * @brief Function that initializes the IR Receiver interrupt
 * 
 */
void ir_recv_intr_init()
{
    gpio_reset_pin(IR_RECV_GPIO);
    gpio_set_direction(IR_RECV_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(IR_RECV_GPIO, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(IR_RECV_GPIO, GPIO_INTR_POSEDGE);
    gpio_isr_handler_add(IR_RECV_GPIO, irRecvHandler, NULL);
}