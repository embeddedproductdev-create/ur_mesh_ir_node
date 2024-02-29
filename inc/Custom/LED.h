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

enum LED_STATES{
    LED_STATE_IDLE,
    LED_STATE_REGISTERED,
    LED_STATE_UNREGISTERED,
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

