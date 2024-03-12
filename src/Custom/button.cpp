/**
 * @file button.cpp
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to the User button
 * @version 0.1
 * @date 2024-03-05
 * @copyright Copyright (c) 2024
 */

#include "../../inc/Custom/button.h"

//Initialization - BUTTON
uint32_t pressedTime = 0;
uint32_t releasedTime = 0;
uint32_t pressedduration_ms = 0;
bool esp_restart_flag = false;

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
        vTaskDelay(1);
        if(!digitalRead(USER_SWITCH)) //button is pressed
        {
            pressedTime = esp_timer_get_time();
            while(!digitalRead(USER_SWITCH)) //Do nothing until button is released
            {
                vTaskDelay(1);
                ;
            }
            releasedTime = esp_timer_get_time();
            pressedduration_ms = (releasedTime - pressedTime)/1000;
            if(pressedduration_ms > LONG_PRESS_1S_MS*3 && pressedduration_ms < LONG_PRESS_1S_MS*6)
                reset_mqtt();
            else if(pressedduration_ms > LONG_PRESS_1S_MS*8)
                esp_restart_flag = true;
        }
    }
}