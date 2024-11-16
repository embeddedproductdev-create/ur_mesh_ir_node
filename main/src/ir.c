#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"

#include "../inc/ir.h"

const char *RAW_IR_PROTOCOL = "RAW";
const char *DAIKIN_IR_PROTOCOL = "DAIKIN280";
const char *DAIKIN200_IR_PROTOCOL = "DAIKIN200";
const char *DAIKIN216_IR_PROTOCOL = "DAIKIN216";
const char *DAIKIN2_IR_PROTOCOL = "DAIKIN2";
const char *DAIKIN160_IR_PROTOCOL = "DAIKIN160";
const char *DAIKIN176_IR_PROTOCOL = "DAIKIN176";
const char *DAIKIN64_IR_PROTOCOL = "DAIKIN64";
const char *DAIKIN152_IR_PROTOCOL = "DAIKIN152";
const char *DAIKIN128_IR_PROTOCOL = "DAIKIN128";
const char *HITACHI_AC296_IR_PROTOCOL = "HITACHI_AC296";
const char *HITACHI_AC_IR_PROTOCOL = "HITACHI_AC";
const char *HITACHI_AC1_IR_PROTOCOL = "HITACHI_AC1";
const char *HITACHI_AC424_IR_PROTOCOL = "HITACHI_AC424";
const char *HITACHI_AC344_IR_PROTOCOL = "HITACHI_AC344";
const char *HITACHI_AC264_IR_PROTOCOL = "HITACHI_AC264";
const char *VOLTAS_IR_PROTOCOL = "VOLTAS";
const char *SAMSUNG_AC_IR_PROTOCOL = "SAMSUNG_AC";
const char *HAIER_AC_IR_PROTOCOL = "HAIER_AC";
const char *HAIER_AC176_IR_PROTOCOL = "HAIER_AC176";
const char *HAIER_AC160_IR_PROTOCOL = "HAIER_AC160";
const char *CARRIER_AC64_IR_PROTOCOL = "CARRIER_AC64";
const char *LG2_IR_PROTOCOL = "LG2";
const char *LG_IR_PROTOCOL = "LG";
const char *TOSHIBA_AC_IR_PROTOCOL = "TOSHIBA_AC";
const char *MITSUBISHI112_IR_PROTOCOL = "MITSUBISHI112";
const char *MITSUBISHI136_IR_PROTOCOL = "MITSUBISHI136";
const char *MITSUBISHI_AC_IR_PROTOCOL = "MITSUBISHI_AC";
const char *MITSUBISHI_HEAVY_88_IR_PROTOCOL = "MITSUBISHI_HEAVY_88";
const char *MITSUBISHI_HEAVY_152_IR_PROTOCOL = "MITSUBISHI_HEAVY_152";
const char *UNKNOWN_IR_PROTOCOL = "UNKNOWN";
const char *UNUSED_IR_PROTOCOL = "UNUSED";
const char *INVALID_IR_PROTOCOL = "INVALID";


/**
 * @brief Interrupt handler function that gets triggered when we
 * recv an IR signal
 * 
 */
static void IRAM_ATTR irRecvHandler(void *args)
{
    gpio_intr_disable(IR_RECV_GPIO);
    if (xTaskCreate(ir_recv_task, "ir_recv_task", 4096, NULL, 10, &ir_recv_task_handle) != pdPASS) {
        ESP_LOGE(IR_TAG, "Failed to create ir_recv_task");
        gpio_intr_enable(IR_RECV_GPIO);
    }
}

/**
 * @brief Thread that takes care of IR Decoding
 *  
 */
void ir_recv_task(void *args)
{
    ESP_LOGW(IR_TAG, "IR Signal Detected. Decoding ... ");
    ir_recv_init();
    vTaskDelay(pdMS_TO_TICKS(1000));
    gpio_intr_enable(IR_RECV_GPIO);
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
    gpio_intr_enable(IR_RECV_GPIO);

}