/**
 * @file main_IR_recv.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions and definitions related to IR receiver part
 * @version 0.1
 * @date 2024-06-19
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
#define NUM_OF_VALUES_PER_COMMAND 600

/* TRANSMITTER */
#define IR_TRANSMIT_PIN 7

/* TEACHING MODE */
#define TEACHING_MODE_STARTING_TEMPERATURE 19
#define TEACHING_MODE_ENDING_TEMPERATURE 28

/* GLOBAL VARIABLES - RECEIVER */
extern char protocol_chosen_str[15];
extern uint16_t custom_raw_buffer[NUM_OF_VALUES_PER_COMMAND];
extern uint8_t custom_raw_buffer_index;

/* GLOBAL VARIABLES - TRANSMITTER */

/* FUNCTION DECLARATIONS - RECEIVER */
void IR_receiver_task(void *args);

/* FUNCTION DECLARATIONS - TRANMSMITTER */
void IR_transmit_setup();
void IR_transmit(uint16_t protocol_detected);

#endif

