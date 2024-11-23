#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "inttypes.h"

#include <button.h>
#include <main.h>
#include <ble_new.h>
#include <led.h>
#include <lte.h>
#include <ir.h>
#include <flash.h>

#define DEBOUNCE_TIME_MS 10
#define DOUBLE_PRESS_TIME_MS 450
#define LONG_PRESS_1S_MS 1000
#define LONG_PRESS_3S_MS 3000
#define MAX_BUTTON_PRESSES 3

#define BUTTON_GPIO 12

typedef enum
{
    MULTI_PRESS,
    SINGLE_PRESS,
    DOUBLE_PRESS,
    TRIPLE_PRESS,
    LONG_PRESS_1S,
    LONG_PRESS_3S,
    UNKNOWN_PRESS
} press_type_t;

typedef struct button_press_event
{
    TickType_t pressedTicks;
    TickType_t releasedTicks;
    uint32_t pressedDuration_ms;
} button_press_t;

press_type_t detected_press = UNKNOWN_PRESS;

/**
 * @brief Button Interrupt handler that creates Button thread
 * @param args
 * @retval none
 */
static void IRAM_ATTR button_task_handler(void *args)
{
    gpio_isr_handler_remove(BUTTON_GPIO);
    if (xTaskCreate(button_task, "button_task", 4096, NULL, 10, &button_task_handle) != pdPASS)
    {
        ESP_LOGE(BUTTON_TAG, "Failed to create button task");
        gpio_isr_handler_add(BUTTON_GPIO, button_task_handler, NULL);
    }
}

/**
 * @brief Function that takes care of differentiating the type of button press
 * @param none
 * @retval none
 */
void process_press_type(button_press_t *button_press_array, uint8_t *press_count)
{
    if (powerDownInProgress)
        return;
    if (button_press_array[0].pressedDuration_ms >= LONG_PRESS_3S_MS)
    {
        detected_press = LONG_PRESS_3S;
    }
    else if (button_press_array[0].pressedDuration_ms >= LONG_PRESS_1S_MS)
    {
        detected_press = LONG_PRESS_1S;
    }
    else
    {
        detected_press = MULTI_PRESS;
    }

    switch (detected_press)
    {
    case MULTI_PRESS:
        switch (*press_count)
        {
        case 1:
#if (IS_GWY)
            if (!powerDownInProgress)
            {
                powerDownLTE();
                esp_restart();
            }
#endif
#if (!IS_GWY)
            esp_restart();
#endif
            break;

        case 2:
            factory_reset_device();
            break;

        case 3:
            break;
        }
        break;

    case LONG_PRESS_3S:
        break;

    case LONG_PRESS_1S:
        if (!teaching_in_progress)
            teaching_mode_init(MAX_LOW_TEMP, MAX_HIGH_TEMP);
        else
            exit_teaching_mode(false);
        break;

    default:
        break;
    }

    gpio_isr_handler_add(BUTTON_GPIO, button_task_handler, NULL);
    vTaskDelete(NULL);
}

/**
 * @brief Button handling task that differentiates between single, double, triple, and long presses.
 */
void button_task(void *args)
{
    uint8_t press_count = 0;
    button_press_t button_press_array[MAX_BUTTON_PRESSES];
    for (int i = 0; i < MAX_BUTTON_PRESSES; i++)
    {
        // If button is not being pressed, then we can go ahead into processing
        if (gpio_get_level(BUTTON_GPIO))
            break;
        // Calculate the button press duration
        button_press_array[i].pressedTicks = xTaskGetTickCount();
        while (!gpio_get_level(BUTTON_GPIO))
            vTaskDelay(pdMS_TO_TICKS(1)); // Wait here until button is released
        button_press_array[i].releasedTicks = xTaskGetTickCount();
        button_press_array[i].pressedDuration_ms = (button_press_array[i].releasedTicks - button_press_array[i].pressedTicks) * portTICK_PERIOD_MS;
        if (button_press_array[i].pressedDuration_ms > DEBOUNCE_TIME_MS)
            press_count++;
        // After a button release, let's wait for 500ms before starting to register the next button press
        while (((xTaskGetTickCount() - button_press_array[i].releasedTicks) * portTICK_PERIOD_MS) < DOUBLE_PRESS_TIME_MS && gpio_get_level(BUTTON_GPIO))
            ;
    }
    process_press_type(button_press_array, &press_count);
}

/**
 * @brief Function that takes care of initializing the button interrupt
 * @param none
 * @retval none
 */
void button_intr_init()
{
    gpio_reset_pin(BUTTON_GPIO);
    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BUTTON_GPIO, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(BUTTON_GPIO, button_task_handler, NULL);
    gpio_intr_enable(BUTTON_GPIO);
}