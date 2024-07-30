/**
 * @file LTE.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions and definitons related to LTE communication
 * @version 0.1
 * @date 2024-07-19
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef LTE_H
#define LTE_H

#include "../Custom/main.h" 

#define MAX_WAIT_MS 100
#define BUF_SIZE 1024

/*CME ERRORS*/
#define CME_ERROR_10 "+CME ERROR: 10"

/*SIM AT COMMANDS*/
#define ENABLE_SIM_INSERTION_STATUS "AT+QSIMSTAT=1\r"
#define GET_SIM_INSERTION_STATUS "AT+QSIMSTAT?\r"
#define SIM_INSERTION_STATUS_RESPONSE "+QSIMSTAT: 1,1"
#define ENABLE_SIM_HOT_SWAPPING "AT+QSIMDET=1,1\r"
#define GET_SIM_HOT_SWAPPING_STATUS "AT+QSIMDET?\r"
#define GET_SIM_PIN_LOCK_STATUS "AT+CPIN?\r"

/*SIM COMMAND RESPONSES*/
#define SIM_INSERTION_RESPONSE "+QSIMSTAT: 1,1"

/*MQTT COMMANDS*/
#define PROMPT ">"
#define MQTT_NETWORK_OPEN "AT+QMTOPEN="
#define MQTT_CLIENT_CONN "AT+QMTCONN="
#define PUBLISH_TO_MQTT "AT+QMTPUBEX="
#define MQTT_NETWORK_CLOSE "AT+QMTCLOSE="
#define READ_MQTT_MESSAGE "AT+QMTRECV="
#define MQTT_CLIENT_DISCONN "AT+QMTDISC="
#define SUB_TO_TOPIC "AT+QMTSUB="

/*MQTT RESPONSES*/
#define STATUS_MQTT_NETWORK "+QMTSTAT:1,1"
#define OK_RESPONSE "OK\r\n"

// LTE PINS
#define GPIO_LTE_RESET 46
#define GPIO_LTE_ONOFF 9
#define TXD_PIN 17
#define RXD_PIN 18
#define CTS_PIN 11
#define RTS_PIN 10
#define GPIO_OUTPUT_PIN_SEL (1ULL << GPIO_LTE_RESET) | (1ULL << GPIO_LTE_ONOFF);
#define RETRY_COUNT 5

enum AT_cmd_id{
    MQTT_KEEP_ALIVE_CMD_ID,
    MQTT_RECV_MODE_CMD_ID,
    MQTT_WILL_CONFIG_CMD_ID,
    MQTT_CLEAN_SESSION_CMD_ID,
    MQTT_NETWORK_OPEN_CMD_ID,
    MQTT_NETWORK_CLOSE_CMD_ID,
    MQTT_CLIENT_CONN_CMD_ID,
    MQTT_CLIENT_DISCONN_CMD_ID,
    MQTT_SUB_CMD_ID,
    MQTT_READ_MSG_CMD_ID,
    MQTT_PUBLISH_MESG_CMD_ID
};

/* GLOBAL VARIABLES */
extern char LTE_UART_data[1024];
extern bool LOG_DATA;

/* GLOBAL VARIABLES - TEACHING MODE*/
extern bool teachMode_size_done;

#ifdef __cplusplus
extern "C"
{
#endif

    /* FUNCTION DECLARATIONS */
    void LTE_UART_INIT();
    void LTE_task(void *args);
    void LTE_restart(void);
    void LTE_gpio_configuration(void);
    void MQTT_config();
    void powerCycleLTE(void);
    void basic_LTE_checks();
    void establishMQTTConnection(void);
    void establishMQTTConnectionNew(void);
    void get_mode_value();
    uint16_t get_gwy_ser_no();
    void init_structures();
    void init_const_AT_cmd_strings();
    void init_Strings();
    int8_t fetch_and_check_data(uint16_t timeout_ms, char *check_string, char *cmd_name);
    int8_t send_cmd_and_check_response(
        bool logging,
        char *cmd, 
        char *cmdName,
        char *check_string, 
        uint32_t timeout_ms
    );
    int8_t check_response(char *data, char *response_check_string);
    void rotate_client_index();
    void factory_reset_device();
    char* get_mode_string(uint8_t mode_value);
    char* get_protocol_string(uint16_t protocol);
    void register_gwy();
#ifdef __cplusplus
}
#endif

#endif
