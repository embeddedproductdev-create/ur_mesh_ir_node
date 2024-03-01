/**
 * @file main_IR_recv.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions and definitions related to IR receiver part
 * @version 0.1
 * @date 2024-02-29
 * @copyright Copyright (c) 2024
 */

#ifndef MAIN_IR_H
#define MAIN_IR_H

#include "../Custom/main.h"
#include <assert.h>
#include <IRrecv.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <IRremoteESP8266.h>

/* RECEIVER */
#define IR_RECEIVER_PIN 38
#define RECV_BUFFER_SIZE 1024
#define kTimeout 50
#define kMinUnknownSize 12
#define kTolerancePercentage kTolerance

/* TRANSMITTER */
#define USER_SWITCH 12
#define IR_TRANSMIT_PIN 7

/* GLOBAL VARIABLES - RECEIVER */
extern bool configured;
extern char protocol_chosen_str[15];

/* GLOBAL VARIABLES - TRANSMITTER */

/* FUNCTION DECLARATIONS - RECEIVER */
void *IR_receiver_task(void *args);

/* FUNCTION DECLARATIONS - TRANMSMITTER */
void *button_task(void *args);
void IR_transmit(uint16_t protocol_detected, char *protocol_chosen_str);



#endif

