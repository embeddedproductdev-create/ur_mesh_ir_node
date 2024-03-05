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
uint8_t LED_state = 2;

void LED_initial_setup()
{
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, HIGH);
}

/**
 * @brief Thread function that handles the LED state
 * @param args
 * @return void*
 */
void *LED_task(void *args)
{
    LED_initial_setup();
    while(1)
    {
        mqtt_connected = true;
        registered = true;
        configured = true;
        protocol_selected_num = 61; // Daikin216
        vTaskDelay(1);
        if(sending)
            LED_state = LED_STATE_SENDING_IR_COMMAND;
        else if(!mqtt_connected)
            LED_state = LED_STATE_MQTT_NOT_CONNECTED;
        if(mqtt_connected && !registered)
            LED_state = LED_STATE_UNREGISTERED;
        else if(mqtt_connected && registered && configured)
            LED_state = LED_STATE_IDLE;
        else if(mqtt_connected && registered && !configured)
            LED_state = LED_STATE_UNCONFIGURED;

        //LED logics are inverted so HIGH = LOW, LOW = HIGH
        //Turn off LEDs at start
        digitalWrite(RED_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, HIGH);
        digitalWrite(BLUE_LED_PIN, HIGH);

        switch(LED_state)
        {
            case LED_STATE_IDLE: //Solid GREEN
                digitalWrite(GREEN_LED_PIN, LOW);
                break;

            case LED_STATE_MQTT_NOT_CONNECTED: //Solid RED
                digitalWrite(RED_LED_PIN, LOW);
                break;

            case LED_STATE_UNREGISTERED: //Blinking RED and BLUE
                digitalWrite(RED_LED_PIN, HIGH);
                digitalWrite(BLUE_LED_PIN, LOW);
                vTaskDelay(pdMS_TO_TICKS(SLOW_BLINK_MS));
                digitalWrite(RED_LED_PIN, LOW);
                digitalWrite(BLUE_LED_PIN, HIGH);
                vTaskDelay(pdMS_TO_TICKS(SLOW_BLINK_MS));
                break;

            case LED_STATE_UNCONFIGURED: //Blinking BLUE
                digitalWrite(BLUE_LED_PIN, LOW);
                vTaskDelay(pdMS_TO_TICKS(SLOW_BLINK_MS));
                digitalWrite(BLUE_LED_PIN, HIGH);
                vTaskDelay(pdMS_TO_TICKS(SLOW_BLINK_MS));
                break;

            case LED_STATE_SENDING_IR_COMMAND:
                digitalWrite(GREEN_LED_PIN, LOW);
                digitalWrite(RED_LED_PIN, LOW);
                digitalWrite(BLUE_LED_PIN, LOW);
                vTaskDelay(pdMS_TO_TICKS(200));
                break;

            default:
                printf("Unknown LED state in LED_task function\n");
                break;
        }
    }
}