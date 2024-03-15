/**
 * @file LED.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions and definitions related to on-Board LED
 * @version 0.1
 * @date 2024-02-29
 * @copyright Copyright (c) 2024
 */

#ifndef LED_H
#define LED_H

#include "main.h"

#define RED_LED_PIN 21
#define GREEN_LED_PIN 14
#define BLUE_LED_PIN 13

#define SLOW_BLINK_MS 500
#define FAST_BLINK_MS 200

enum LED_STATES{
    LED_STATE_IDLE,
    LED_STATE_REGISTERED,
    LED_STATE_UNREGISTERED,
    LED_STATE_UNCONFIGURED,
    LED_STATE_MQTT_CONNECTED,
    LED_STATE_MQTT_NOT_CONNECTED,
    LED_STATE_SENDING_IR_COMMAND,
    LED_STATE_AP_MODE,
    LED_STATE_TEACHING_MODE
};

/* GLOBAL VARIABLES */
extern uint8_t LED_state;

#ifdef __cplusplus
extern "C" {
#endif

/* FUNCTION DECLARATIONS */
void *LED_task(void *args);
void LED_initial_setup();

#ifdef __cplusplus
}
#endif

#endif

