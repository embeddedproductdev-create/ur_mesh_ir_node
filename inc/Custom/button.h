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

#define USER_SWITCH 12
#define SHORT_PRESS_DURATION_MS 500
#define LONG_PRESS_DUARTION_MS 1500

/* GLOBAL VARIABLES */
extern bool esp_restart_flag;

/* FUNCTION DECLARATIONS */
void *button_task(void *args);

#endif