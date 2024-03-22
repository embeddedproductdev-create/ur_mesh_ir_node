/**
 * @file heartbeat.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all definitions and MACROS related to
 * sending heartbeat messages to the cloud to let it know if
 * a device is alive or not
 * @version 0.1
 * @date 2024-03-19
 * @copyright Copyright (c) 2024
 */

#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "main.h"

/* GLOBAL VARIABLES */
extern uint32_t HBFreqInSecs;


#ifdef __cplusplus
extern "C" {
#endif
/* FUNCTION DECLARATIONS */
void *HB_task(void *args);
#ifdef __cplusplus
}
#endif

#endif