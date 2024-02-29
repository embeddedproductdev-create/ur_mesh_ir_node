/**
 * @file main_IR_send.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions and definitions related to IR transmission part
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

/* GLOBAL VARIABLES */
extern bool sending;
extern bool needtosend;

/* FUNCTION DECLARATIONS */
void IR_transmit();

#endif

#ifdef __cplusplus
}
#endif

