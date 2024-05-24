/**
 * @file printf_custom.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains MACROS and function declarations for custom printing logs
 * @version 0.5
 * @date 2024-04-20
 * @copyright Copyright (c) 2024
 */

#include "main.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define MAIN_DEBUG_TAG          "[MAIN_DEBUG]   : "
#define MAIN_ERROR_TAG          "[MAIN_ERROR]   : "
#define LTE_DEBUG_TAG           "[LTE_DEBUG]    : "
#define LTE_ERROR_TAG           "[LTE_ERROR]    : "
#define LED_DEBUG_TAG           "[LED_DEBUG]    : "
#define LED_ERROR_TAG           "[LED_ERROR]    : "
#define QUEUE_DEBUG_TAG         "[QUEUE_DEBUG]  : "
#define QUEUE_ERROR_TAG         "[QUEUE_ERROR]  : "
#define AP_DEBUG_TAG            "[AP_DEBUG]     : "
#define AP_ERROR_TAG            "[AP_ERROR]     : "
#define BUTTON_DEBUG_TAG        "[BUTTON_DEBUG] : "
#define BUTTON_ERROR_TAG        "[BUTTON_ERROR] : "
#define IR_DEBUG_TAG            "[IR_DEBUG]     : "
#define IR_ERROR_TAG            "[IR_ERROR]     : "
#define MESH_DEBUG_TAG          "[MESH_DEBUG]  " 
#define MESH_ERROR_TAG          "[MESH_ERROR]  "  
#define TEMPERATURE_DEBUG_TAG   "[TEMP_DEBUG]   : "
#define TEMPERATURE_ERROR_TAG   "[TEMP_ERROR]   : "
#define SETUP_TAG               "[SETUP_DEBUG]  : "

/* GLOBAL VARIABLES */
extern char lte_log_buffer[2048];
extern char queue_log_buffer[2048];
extern char ir_log_buffer[2048];
extern char mesh_log_buffer[2048];
extern char temperature_log_buffer[2048];
extern char button_log_buffer[2048];

/* FUNCTION DECLARATIONS */
#ifdef __cplusplus
extern "C" {
#endif
void white_printf(char *tag, char *msg);
void blue_printf(char *tag, char *msg);
#ifdef __cplusplus
}
#endif

void red_printf(char *tag, char *msg);
void green_printf(char *tag, char *msg);
void yellow_printf(char *tag, char *msg);
void magenta_printf(char *tag, char *msg);
void cyan_printf(char *tag, char *msg);
