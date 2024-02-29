/**
 * @file main_IR_recv.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions and definitions related to IR receiver part
 * @version 0.1
 * @date 2024-02-29
 * @copyright Copyright (c) 2024
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAIN_IR_RECV_H
#define MAIN_IR_RECV_H

#include "../Custom/main.h"

#define IR_RECEIVER_PIN 38
#define BAUD_RATE 115200
#define RECV_BUFFER_SIZE 1024
#define kTimeout 50
#define kMinUnknownSize 12
#define kTolerancePercentage kTolerance


/* GLOBAL VARIABLES */
extern bool configured;
extern char protocol_chosen[15];

/* FUNCTION DECLARATIONS */
void *IR_receiver_task(void *args);

#endif

#ifdef __cplusplus
}
#endif

