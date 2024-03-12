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
bool esp_restart_flag = false;

void clear_mqtt_settings()
{
    mqtt_params_fetched_flag = false;
    mqtt_client_index = 99;
    mqtt_port = 1;
    memset(mqtt_ip_address, 0, strlen(mqtt_ip_address));
    memset(mqtt_broker_username, 0, strlen(mqtt_broker_username));
    memset(mqtt_broker_password, 0, strlen(mqtt_broker_password));
    memset(mqtt_broker_tabname, 0, strlen(mqtt_broker_tabname));
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
            if((releasedTime - pressedTime)/1000 > LONG_PRESS_1S_MS*5 && (releasedTime - pressedTime)/1000 < LONG_PRESS_1S_MS*6)
            {
                MQTT_NetworkClose(mqtt_client_index);
                clear_mqtt_settings();
                create_AP_task();
            }
            else if((releasedTime - pressedTime)/1000 > LONG_PRESS_1S_MS*8)
                esp_restart_flag = true;
        }
    }
}