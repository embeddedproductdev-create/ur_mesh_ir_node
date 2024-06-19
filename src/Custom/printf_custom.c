/**
 * @file printf_custom.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains function definitions for custom printing logs
 * @version 0.8
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

char log_buffer[2048];
char lte_log_buffer[2048];
char queue_log_buffer[2048];
char ir_log_buffer[2048];
char mesh_log_buffer[2048];
char temperature_log_buffer[2048];
char button_log_buffer[2048];

char MAIN_DEBUG_TAG[20] = "[MAIN_DEBUG]   : ";
char MAIN_ERROR_TAG[20] = "[MAIN_ERROR]   : ";
char LTE_DEBUG_TAG[20] = "[LTE_DEBUG]    : ";
char LTE_ERROR_TAG[20] = "[LTE_ERROR]    : ";
char LED_DEBUG_TAG[20] = "[LED_DEBUG]    : ";
char LED_ERROR_TAG[20] = "[LED_ERROR]    : ";
char QUEUE_DEBUG_TAG[20] = "[QUEUE_DEBUG]  : ";
char QUEUE_ERROR_TAG[20] = "[QUEUE_ERROR]  : ";
char AP_DEBUG_TAG[20] = "[AP_DEBUG]     : ";
char AP_ERROR_TAG[20] = "[AP_ERROR]     : ";
char BUTTON_DEBUG_TAG[20] = "[BUTTON_DEBUG] : ";
char BUTTON_ERROR_TAG[20] = "[BUTTON_ERROR] : ";
char IR_DEBUG_TAG[20] = "[IR_DEBUG]     : ";
char IR_ERROR_TAG[20] = "[IR_ERROR]     : ";
char MESH_DEBUG_TAG[20] = "[MESH_DEBUG]  ";
char MESH_ERROR_TAG[20] = "[MESH_ERROR]  ";
char TEMPERATURE_DEBUG_TAG[20] = "[TEMP_DEBUG]   : ";
char TEMPERATURE_ERROR_TAG[20] = "[TEMP_ERROR]   : ";
char SETUP_TAG[20] = "[SETUP_DEBUG]  : ";

void white_printf(char *tag, char *msg)
{
    if(LOG_DATA) {
    sprintf(temp1, "%lld", esp_timer_get_time());
    printf(ANSI_COLOR_WHITE "(%s) %s%s\n" ANSI_COLOR_RESET, temp1, tag, msg);
    memset(ir_log_buffer, 0, sizeof(ir_log_buffer));
    }
}

void red_printf(char *tag, char *msg)
{
    sprintf(temp2, "%lld", esp_timer_get_time());
    printf(ANSI_COLOR_RED "(%s) %s%s\n" ANSI_COLOR_RESET, temp2, tag, msg);
    memset(log_buffer, 0, sizeof(log_buffer));
    memset(lte_log_buffer, 0, sizeof(lte_log_buffer));
    memset(queue_log_buffer, 9, sizeof(queue_log_buffer));
    memset(ir_log_buffer, 0, sizeof(ir_log_buffer));
    memset(temperature_log_buffer, 0, sizeof(temperature_log_buffer));
    memset(button_log_buffer, 0, sizeof(button_log_buffer));
}

void green_printf(char *tag, char *msg)
{
    if (LOG_DATA) {
    sprintf(temp3, "%lld", esp_timer_get_time());
    printf(ANSI_COLOR_GREEN "(%s) %s%s\n" ANSI_COLOR_RESET, temp3, tag, msg);
    memset(button_log_buffer, 0, sizeof(button_log_buffer));
    }
}

void blue_printf(char *tag, char *msg)
{
    if (LOG_DATA) {
    sprintf(temp4, "%lld", esp_timer_get_time());
    printf(ANSI_COLOR_BLUE "(%s) %s%s\n" ANSI_COLOR_RESET, temp4, tag, msg);
    }
}

void cyan_printf(char *tag, char *msg)
{
    if (LOG_DATA) {
    sprintf(temp5, "%lld", esp_timer_get_time());
    printf(ANSI_COLOR_CYAN "(%s) %s%s\n" ANSI_COLOR_RESET, temp5, tag, msg);
    memset(lte_log_buffer, 0, sizeof(lte_log_buffer));
    }
}

void yellow_printf(char *tag, char *msg)
{
    if (LOG_DATA) {
    sprintf(temp6, "%lld", esp_timer_get_time());
    printf(ANSI_COLOR_YELLOW "(%s) %s%s\n" ANSI_COLOR_RESET, temp6, tag, msg);
    memset(queue_log_buffer, 9, sizeof(queue_log_buffer));
    }
}

void magenta_printf(char *tag, char *msg)
{
    if (LOG_DATA) {
    sprintf(temp7, "%lld", esp_timer_get_time());
    printf(ANSI_COLOR_MAGENTA "(%s) %s%s\n" ANSI_COLOR_RESET, temp7, tag, msg);
    memset(temperature_log_buffer, 0, sizeof(temperature_log_buffer));
    }
}
