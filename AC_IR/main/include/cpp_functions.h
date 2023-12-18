/*
 * cpp_functions.h
 *
 *  Created on: 18-Dec-2023
 *      Author: EmbeddedDevelopment
 */

#ifndef MAIN_CPP_FUNCTIONS_H_
#define MAIN_CPP_FUNCTIONS_H_

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef enum{
  Vol_up,
  Vol_down,
  Up,
  Down,
  Left,
  Right,
  Home,
  Power,
  Back
}commands;

EXTERNC void recv_Ir_commands(void);
//EXTERNC void send_Ir_commands(commands command);
EXTERNC void loop(void);

#endif /* MAIN_CPP_FUNCTIONS_H_ */
