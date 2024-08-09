/**
 * @file printf_custom.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains MACROS and function declarations for custom printing logs
 * @version 0.5
 * @date 2024-04-20
 * @copyright Copyright (c) 2024
 */

#include "main.h"

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1b[37m"
#define RESET   "\x1b[0m"

/* GLOBAL VARIABLES */
extern char log_buffer[2048];
extern char lte_log_buffer[2048];
extern char queue_log_buffer[2048];
extern char ir_log_buffer[2048];
extern char mesh_log_buffer[2048];
extern char temperature_log_buffer[2048];
extern char button_log_buffer[2048];

extern char MAIN_DEBUG_TAG[20];
extern char MAIN_ERROR_TAG[20];
extern char LTE_DEBUG_TAG[20];
extern char LTE_ERROR_TAG[20];
extern char LED_DEBUG_TAG[20];
extern char LED_ERROR_TAG[20];
extern char QUEUE_DEBUG_TAG[20];
extern char QUEUE_ERROR_TAG[20];
extern char AP_DEBUG_TAG[20];
extern char AP_ERROR_TAG[20];
extern char BUTTON_DEBUG_TAG[20];
extern char BUTTON_ERROR_TAG[20];
extern char IR_DEBUG_TAG[20];
extern char IR_ERROR_TAG[20];
extern char MESH_DEBUG_TAG[20];
extern char MESH_ERROR_TAG[20];
extern char TEMPERATURE_DEBUG_TAG[20];
extern char TEMPERATURE_ERROR_TAG[20];
extern char SETUP_TAG[20];

/* FUNCTION DECLARATIONS */
#ifdef __cplusplus
extern "C" {
#endif
void white_printf(char *tag, char *msg);
void blue_printf(char *tag, char *msg);
void red_printf(char *tag, char *msg);
void green_printf(char *tag, char *msg);
void yellow_printf(char *tag, char *msg);
void magenta_printf(char *tag, char *msg);
void cyan_printf(char *tag, char *msg);
void custom_printf(char *tag, const char *msg, const char *color);
#ifdef __cplusplus
}
#endif


