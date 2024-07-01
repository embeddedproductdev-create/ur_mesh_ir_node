/**
 * @file button.cpp
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to the User button
 * @version 0.6
 * @date 2024-03-05
 * @copyright Copyright (c) 2024
 */

/**
 * Current Button Press types
 * Single click = Puts the Device into Teaching mode
 * Double click = Puts the Device into Configuration mode
 * Long press (3s-6s) = Resets the MQTT connection
 * Long press (>8s) = Restarts the Device
 */

#include "../../inc/Custom/button.h"
#include "../../inc/IR/main_IR.h"

// We're including the below just to call reset node function
#include "../../inc/Mesh/ble_mesh_example_init.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_defs.h"

// Initialization - BUTTON
uint64_t beginTime = 0;
uint64_t pressedTime = 0;
uint64_t releasedTime = 0;
uint32_t pressedduration_ms = 0;
uint32_t pressed_duration_array[3] = {0, 0, 0};
uint8_t pressed_duration_array_index = 0;

void button_logic()
{
    if (pressed_duration_array[0] != 0)
    {
        /* Single Press */
        if (pressed_duration_array[0] < ONE_SEC_IN_MS && pressed_duration_array[1] == 0)
        {
            if (LOG_DATA)
                LOG_DATA = false;
            else
                LOG_DATA = true;
        }

        /* Double Press */
        else if (pressed_duration_array[0] < ONE_SEC_IN_MS && pressed_duration_array[1] != 0 && pressed_duration_array[1] < ONE_SEC_IN_MS)
        {
            esp_ble_mesh_node_local_reset();
        }

        /* Button held for 3s - 8s */
        else if (pressed_duration_array[0] >= ONE_SEC_IN_MS * 3 && pressed_duration_array[0] < ONE_SEC_IN_MS *8)
        {
            if (!teaching_mode && registered)
            {
                teaching_mode = true;
                teachMode_size_done = true;
                green_printf(BUTTON_DEBUG_TAG, "Start of Teaching Mode (Due to Button press)");
            }
            else
            {
                teaching_mode = false;
                teachMode_size_done = false;
                green_printf(BUTTON_DEBUG_TAG, "End of Teaching Mode (Due to Button Press)");
            }
        }

        /*  Button held for more than 8s */
        else if (pressed_duration_array[0] > ONE_SEC_IN_MS * 8) // Button held for more than 8s.
        {
            ESP.restart();
        }
    }
}

void calculate_button_press_time()
{
    beginTime = esp_timer_get_time();
    pressedTime = esp_timer_get_time();
    while (!digitalRead(USER_SWITCH)) // Do nothing until button is released
    {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    releasedTime = esp_timer_get_time();
    pressedduration_ms = (releasedTime - pressedTime) / 1000;
    pressed_duration_array[pressed_duration_array_index++] = pressedduration_ms;
    sprintf(button_log_buffer, "=-=-=-=-=-=-=- Button held time in seconds : %ld =-=-=-=-=-=-=", pressed_duration_array[0]);
    custom_printf(BUTTON_DEBUG_TAG, button_log_buffer, BLUE);
}

/**
 * @brief Thread that handles the Button press
 * @param args
 * @return void*
 */
void button_task(void *args)
{
    pinMode(USER_SWITCH, INPUT);
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        if (!digitalRead(USER_SWITCH)) // button is pressed
        {
            calculate_button_press_time();
            // wait for a second button press within 500ms of first button press
            while (((esp_timer_get_time() - beginTime) / 1000) < HALF_SEC_IN_MS)
            {
                vTaskDelay(pdMS_TO_TICKS(100));
                if (!digitalRead(USER_SWITCH))
                    calculate_button_press_time();
            }
        }
        button_logic();

        // Reset the timings and index
        pressed_duration_array_index = 0;
        memset(pressed_duration_array, 0, sizeof(pressed_duration_array));
    }
}