/**
 * @file printf_custom.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains function definitions for custom printing logs
 * @version 0.6
 * @date 2024-04-20
 * @copyright Copyright (c) 2024
 */

#include <../../inc/Custom/printf_custom.h>

/*Initialization*/
char temp1[10];
char temp2[10];
char temp3[10];
char temp4[10];
char temp5[10];
char temp6[10];
char temp7[10];
char lte_log_buffer[2048];
char queue_log_buffer[2048];
char ir_log_buffer[2048];
char mesh_log_buffer[2048];
char temperature_log_buffer[2048];
char button_log_buffer[2048];

void white_printf(char *tag, char *msg)
{
    sprintf(temp1, "%lld", esp_timer_get_time());
    printf(ANSI_COLOR_WHITE"(%s) %s%s\n"ANSI_COLOR_RESET,temp1,tag,msg);
}

void red_printf(char *tag, char *msg)
{
    sprintf(temp2, "%lld", esp_timer_get_time());
    printf(ANSI_COLOR_RED"(%s) %s%s\n"ANSI_COLOR_RESET,temp2,tag,msg);
}

void green_printf(char *tag, char *msg)
{
    sprintf(temp3, "%lld", esp_timer_get_time());
    printf(ANSI_COLOR_GREEN"(%s) %s%s\n"ANSI_COLOR_RESET,temp3,tag,msg);
}

void blue_printf(char *tag, char *msg)
{
    sprintf(temp4, "%lld", esp_timer_get_time());
    printf(ANSI_COLOR_BLUE"(%s) %s%s\n"ANSI_COLOR_RESET,temp4,tag,msg);
}

void cyan_printf(char *tag, char *msg)
{
    sprintf(temp5, "%lld", esp_timer_get_time());
    printf(ANSI_COLOR_CYAN"(%s) %s%s\n"ANSI_COLOR_RESET,temp5,tag,msg);
}

void yellow_printf(char *tag, char *msg)
{
    sprintf(temp6, "%lld", esp_timer_get_time());
    printf(ANSI_COLOR_YELLOW"(%s) %s%s\n"ANSI_COLOR_RESET,temp6,tag,msg);
}

void magenta_printf(char *tag, char *msg)
{
    sprintf(temp7, "%lld", esp_timer_get_time());
    printf(ANSI_COLOR_MAGENTA"(%s) %s%s\n"ANSI_COLOR_RESET,temp7,tag,msg);
}

