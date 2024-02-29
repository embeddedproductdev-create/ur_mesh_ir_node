/**
 * @file LED.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions related to the handling of the On-Board LED
 * @version 0.1
 * @date 2024-02-29
 * @copyright Copyright (c) 2024
 */

#include "../../inc/Custom/LED.h"

//Initialization
uint8_t LED_state = 0;

void LED_initial_setup()
{
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
}

/**
 * @brief Thread function that handles the LED state
 * @param args
 * @return void*
 */
void *LED_task(void *args)
{
    while(1)
    {
        LED_initial_setup()
        switch(LED_state)
        {
            case LED_STATE_IDLE:
                break;
            case LED_STATE_UNREGISTERED:
                digitalWrite(RED_LED_PIN, HIGH);
                vTaskDelay(pdMS_TO_TICKS(1000));
                digitalWrite(RED_LED_PIN, LOW);
                vTaskDelay(pdMS_TO_TICKS(1000));
                break;
            default:
                printf("Unknown LED state in LED_task function\n");
                break;
        }
    }
}