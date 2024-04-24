/**
 * @file printf_custom.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains function definitions for custom printing logs
 * @version 0.5
 * @date 2024-04-20
 * @copyright Copyright (c) 2024
 */

#include <../../inc/Custom/printf_custom.h>

/*Initialization*/
char lte_log_buffer[2048];
char queue_log_buffer[2048];
char ir_log_buffer[2048];
char mesh_log_buffer[2048];
char temperature_log_buffer[2048];

void red_printf(char* tag, char *msg)
{
    printf(ANSI_COLOR_RED"%s%s\n"ANSI_COLOR_RESET,tag,msg);
}

void green_printf(char* tag, char *msg)
{
    printf(ANSI_COLOR_GREEN"%s%s\n"ANSI_COLOR_RESET,tag,msg);
}

void blue_printf(char* tag, char *msg)
{
    printf(ANSI_COLOR_BLUE"%s%s\n"ANSI_COLOR_RESET,tag,msg);
}

void cyan_printf(char* tag, char *msg)
{
    printf(ANSI_COLOR_CYAN"%s%s\n"ANSI_COLOR_RESET,tag,msg);
}

void yellow_printf(char* tag, char *msg)
{
    printf(ANSI_COLOR_YELLOW"%s%s\n"ANSI_COLOR_RESET,tag,msg);
}

void white_printf(char* tag, char *msg)
{
    printf(ANSI_COLOR_WHITE"%s%s\n"ANSI_COLOR_RESET,tag,msg);
}

