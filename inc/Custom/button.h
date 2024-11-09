/**
 * @file button.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains definitions and functions related to user button
 * @version 0.1
 * @date 2024-03-05
 * @copyright Copyright (c) 2024
 */

#ifndef BUTTON_H
#define BUTTON_H

#include "main.h"
#include "accesspoint.h"

#define USER_SWITCH 12
#define ONE_SEC_IN_MS 1000
#define HALF_SEC_IN_MS 500

/* GLOBAL VARIABLES */
extern bool reset_node;

/* FUNCTION DECLARATIONS */
void clear_mqtt_settings();
void button_intr_init();
#endif