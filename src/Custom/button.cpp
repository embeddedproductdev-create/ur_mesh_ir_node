/**
 * @file button.cpp
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to the User button
 * @version 0.1
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

//Initialization - BUTTON
uint32_t beginTime = 0;
uint32_t pressedTime = 0;
uint32_t releasedTime = 0;
uint32_t pressedduration_ms = 0;
uint32_t pressed_duration_array[3] = {0,0,0};
uint8_t pressed_duration_array_index = 0;

void button_logic()
{
    if(pressed_duration_array[0]!=0)
    {
    if(pressed_duration_array[0]<ONE_SEC_IN_MS && pressed_duration_array[1]==0) //Single press
    {
        if(LOG_LTE_DATA) LOG_LTE_DATA = false;
        else LOG_LTE_DATA = true;
    } 
    else if(pressed_duration_array[0]<ONE_SEC_IN_MS && pressed_duration_array[1]!=0 && pressed_duration_array[1] < ONE_SEC_IN_MS) //Double press
    {
        ESP_LOGI(IR_DEBUG_TAG, "protocol_selected_num : %d",DAIKIN);
        IR_transmit(DAIKIN);
    }
    else if(pressed_duration_array[0] > ONE_SEC_IN_MS*3 && pressed_duration_array[0] < ONE_SEC_IN_MS*7) //Button held for 3 to 7 seconds
        configured = false;
    else if(pressed_duration_array[0] >= ONE_SEC_IN_MS*8) //Button held for 8 seconds or more
        esp_restart_flag = true;
    }
}

void calculate_button_press_time()
{
    beginTime = esp_timer_get_time();
    pressedTime = esp_timer_get_time();
    while(!digitalRead(USER_SWITCH)) //Do nothing until button is released
    {
        vTaskDelay(pdMS_TO_TICKS(1));
        ;
    }
    releasedTime = esp_timer_get_time();
    pressedduration_ms = (releasedTime - pressedTime)/1000;
    pressed_duration_array[pressed_duration_array_index++] = pressedduration_ms;
}

/**
 * @brief Thread that handles the Button press
 * @param args
 * @return void*
 */
void *button_task(void *args)
{
    pinMode(USER_SWITCH, INPUT);
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        if(!digitalRead(USER_SWITCH)) //button is pressed
        {
            calculate_button_press_time();
            //wait for a second button press within 500ms of first button press
            while(((esp_timer_get_time()-beginTime)/1000) < HALF_SEC_IN_MS)
            {
                vTaskDelay(pdMS_TO_TICKS(100));
                if(!digitalRead(USER_SWITCH))
                    calculate_button_press_time();
            }
        }
        button_logic();

        //Reset the timings and index
        pressed_duration_array_index=0;
        memset(pressed_duration_array, 0, sizeof(pressed_duration_array));
    }
}