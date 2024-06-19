/**
 * @file LED.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions related to the handling of the On-Board LED
 * @version 0.8
 * @date 2024-06-19
 * @copyright Copyright (c) 2024
 */

#include "../../inc/Custom/LED.h"

// Initialization
uint8_t LED_state;

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
void LED_task(void *args)
{
    LED_initial_setup();
    while (1)
    {
        vTaskDelay(1);
        if (show_boot_indication) 
            LED_state = LED_STATE_DEVICE_BOOT_SUCCESSFUL;
        else if (storing_IR_data_to_flash)
            LED_state = LED_STATE_OFF;
        else if (needToSendIRComamnd)
            LED_state = LED_STATE_SENDING_IR_COMMAND;
        else if (teaching_mode)
            LED_state = LED_STATE_TEACHING_MODE;
#if (IS_GWY)
        else if (!mqtt_params_fetched_flag)
            LED_state = LED_STATE_AP_MODE;
        else if (!mqtt_connected)
            LED_state = LED_STATE_MQTT_NOT_CONNECTED;
        else if (!registered)
            LED_state = LED_STATE_UNREGISTERED;
        else if (!configured)
            LED_state = LED_STATE_UNCONFIGURED;
        else
            LED_state = LED_STATE_IDLE;
#endif
#if (!IS_GWY)
        else if (!provisioned)
            LED_state = LED_STATE_UNPROVISIONED;
        else if (!configured)
            LED_state = LED_STATE_UNCONFIGURED;
        else LED_state = LED_STATE_IDLE;
#endif

        // LED logics are inverted so HIGH = OFF, LOW = ON
        // Turn off LEDs at start
        digitalWrite(RED_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, HIGH);
        digitalWrite(BLUE_LED_PIN, HIGH);

        switch (LED_state)
        {
        case LED_STATE_OFF:
            //Since by default we're turing off all LEDs for every cycle
            //we don't need to add any statements to this case.
            break;
        
        case LED_STATE_SENDING_IR_COMMAND: // Solid Purple
            digitalWrite(RED_LED_PIN, LOW);
            digitalWrite(BLUE_LED_PIN, LOW);
            break;
        
        case LED_STATE_IDLE: // Solid GREEN
            digitalWrite(GREEN_LED_PIN, LOW);
            break;
        
        case LED_STATE_DEVICE_BOOT_SUCCESSFUL: // Toggle RED + GREEN + BLUE
            digitalWrite(RED_LED_PIN, LOW);
            sleep(1);
            digitalWrite(RED_LED_PIN, HIGH);
            digitalWrite(GREEN_LED_PIN, LOW);
            sleep(1);
            digitalWrite(GREEN_LED_PIN, HIGH);
            digitalWrite(BLUE_LED_PIN, LOW);
            sleep(1);
            show_boot_indication = false;
            break;

        case LED_STATE_TEACHING_MODE:
            digitalWrite(BLUE_LED_PIN, LOW);
            vTaskDelay(FAST_BLINK_MS);
            digitalWrite(BLUE_LED_PIN, HIGH);
            vTaskDelay(FAST_BLINK_MS);
            break;

        case LED_STATE_MQTT_NOT_CONNECTED: // Solid RED
            digitalWrite(RED_LED_PIN, LOW);
            break;

        case LED_STATE_UNPROVISIONED:
        case LED_STATE_UNREGISTERED: // Blinking RED and BLUE
            digitalWrite(RED_LED_PIN, HIGH);
            digitalWrite(BLUE_LED_PIN, LOW);
            vTaskDelay(pdMS_TO_TICKS(SLOW_BLINK_MS));
            digitalWrite(RED_LED_PIN, LOW);
            digitalWrite(BLUE_LED_PIN, HIGH);
            vTaskDelay(pdMS_TO_TICKS(SLOW_BLINK_MS));
            break;

        case LED_STATE_UNCONFIGURED: // Blinking BLUE
            digitalWrite(BLUE_LED_PIN, LOW);
            vTaskDelay(pdMS_TO_TICKS(SLOW_BLINK_MS));
            digitalWrite(BLUE_LED_PIN, HIGH);
            vTaskDelay(pdMS_TO_TICKS(SLOW_BLINK_MS));
            break;

        case LED_STATE_AP_MODE: // RGB toggle
            digitalWrite(RED_LED_PIN, LOW);
            vTaskDelay(SLOW_BLINK_MS);
            digitalWrite(RED_LED_PIN, HIGH);
            digitalWrite(GREEN_LED_PIN, LOW);
            vTaskDelay(SLOW_BLINK_MS);
            digitalWrite(GREEN_LED_PIN, HIGH);
            digitalWrite(BLUE_LED_PIN, LOW);
            vTaskDelay(SLOW_BLINK_MS);
            digitalWrite(BLUE_LED_PIN, HIGH);
            break;

        default:
            red_printf(LED_ERROR_TAG,"Unknown LED state in LED_task function\n");
            break;
        }
    }
}