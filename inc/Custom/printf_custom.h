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
#define ANSI_COLOR_RESET   "\x1b[0m"

/* GLOBAL VARIABLES */
extern char log_buffer[2048];

/* FUNCTION DECLARATIONS */
void red_printf(char *tag, char *msg);
void green_printf(char *tag, char *msg);
void yellow_printf(char *tag, char *msg);
void blue_printf(char *tag, char *msg);
void magenta_printf(char *tag, char *msg);
void cyan_printf(char *tag, char *msg);