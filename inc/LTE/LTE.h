/**
 * @file LTE.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions and definitons related to LTE communication
 * @version 0.1
 * @date 2024-02-29
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef LTE_H
#define LTE_H

#include "../Custom/main.h" 

#define MAX_WAIT_MS 100
#define BUF_SIZE 2048

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

/*GENERAL COMMANDS*/
#define PROMPT ">"
#define GET_PRODUCT_INFO "ATI\r"
#define GET_FW_REVISION "AT+GMR\r"
#define GET_ME_SERIAL_NO "AT+GSN\r"
#define GET_ME_ACTIVITY_STATUS "AT+CPAS\r"
#define ENABLE_NETWORK_REGISTRATION "AT+CREG=2\r"
#define GET_NETWORK_REGISTRATION_STATUS "AT+CREG?\r"
#define LIST_ALL_OPERATORS "AT+COPS=?\r"
#define SET_CURRENT_OPERATOR_STATUS "AT+COPS=0,2,0,0\r"
#define GET_CURRENT_OPERATOR_STATUS "AT+COPS?\r"
#define GET_SERVICE_PROVIDER_NAME "AT+QSPN\r"
#define GET_SIGNAL_STRENGTH "AT+QCSQ\r"
#define GET_MANUFACTURER "AT+CGMI\r\n"
#define GET_IMEI_NUM "AT+GSN=1\r\n"
#define FACTORY_RESET "AT&F0\r\n"
#define GET_CURRENT_CONFIG "AT&V\r\n"
#define GET_TIME "AT+QLTS=2\r\n"
#define TURN_OFF_ECHO_CMD "ATE0\r"

/*MQTT COMMANDS*/
#define SET_MQTT_VERSION "AT+QMTCFG=\"version\","
#define MQTT_PDP "AT+QMTCFG=\"pdpcid\","
#define ENABLE_SSL "AT+QMTCFG=\"ssl\","
#define SET_KEEP_ALIVE "AT+QMTCFG=\"keepalive\","
#define SET_CLEAN_SESSION "AT+QMTCFG=\"session\","
#define MQTT_WILL_CONFIG "AT+QMTCFG=\"will\","
#define SET_MSG_RECV_MODE "AT+QMTCFG=\"recv/mode\","
#define MQTT_SEND_MODE "AT+QMTCFG=\"send/mode\",1,0\r\n"
#define MQTT_HEARTBEAT "AT+QMTCFG=\"qmtping\",1\r\n"
#define MQTT_DATA_FORMAT "AT+QMTCFG=\"dataformat\",1,0,0\r\n"
#define MQTT_NETWORK_OPEN "AT+QMTOPEN="
#define MQTT_NETWORK_CLOSE "AT+QMTCLOSE="
#define MQTT_CLIENT_CONN "AT+QMTCONN="
#define MQTT_CLIENT_DISCONN "AT+QMTDISC="
#define MQTT_CLIENT_DISCONN_CHECK "AT+QMTDISC=?\r"
#define SUB_TO_TOPIC "AT+QMTSUB="
#define UNSUB_TO_TOPIC "AT+QMTUNS="
#define PUBLISH_TO_MQTT "AT+QMTPUBEX="
#define READ_MQTT_MESSAGE "AT+QMTRECV="
#define MQTT_CHECK_READ_BUFFER_CMD "AT+QMTRECV?\r"

/*MQTT RESPONSES*/
//#define MQTT_NETWORK_OPEN_RESPONSE "+QMTOPEN: "
#define MQTT_NETWORK_CLOSE_RESPONSE "+QMTCLOSE: "
//#define MQTT_CLIENT_CONN_RESPONSE "+QMTCONN: "
#define MQTT_CLIENT_DISCONN_RESPONSE "+QMTDISC: "
#define MQTT_READ_MSG_RESPONSE "+QMTRECV: "
#define STATUS_MQTT_NETWORK "+QMTSTAT:1,1"
//#define MQTT_SUB_RESPONSE "+QMTSUB: "
#define MQTT_UNSUB_RESPONSE "+QMTUNS: "
//#define MQTT_PUB_MSG_RESPONSE "+QMTPUBEX: "
#define OK_RESPONSE "OK\r\n"
#define CONNECT_RESPONSE "CONNECT\r\n"
#define INDICATOR_STATE_RESPONSE "+CIND: "
#define MQTT_VERSION_RESPONSE "+QMTCFG:\"version\",4"
#define MQTT_SSL_RESPONSE "+QMTCFG:\"ssl\",1"
#define MQTT_KEEP_ALIVE_RESPONSE "+QMTCFG:\"keepalive\",120"
#define MQTT_RECV_MODE_RESPONSE "+QMTCFG:\"recv/mode\",0,0"
#define MQTT_HEARTBEAT_RESPONSE "+QMTCFG:\"qmtping\",5"
#define MQTT_SEND_MODE_RESPONSE "+QMTCFG:\"send/mode\",0"
#define MQTT_DATA_FORMAT_RESPONSE "+QMTCFG:\"dataformat\",0,0"

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
extern char LTE_UART_data[2048];
extern bool LOG_DATA;
extern bool sending_at_cmd;

#ifdef __cplusplus
extern "C"
{
#endif

    /* FUNCTION DECLARATIONS */
    void LTE_task(void *args);
    void LTE_restart(void);
    void LTE_gpio_configuration(void);
    void powerCycleLTE(void);
    void establishMQTTConnection(void);
    void establishMQTTConnectionNew(void);
    void get_mode_value();
    uint16_t get_gwy_ser_no();
    void init_structures();
    void init_Strings();
    int8_t fetch_and_check_data(uint16_t timeout_ms, char *check_string);
    int8_t send_cmd_and_check_response(
        bool logging,
        char *cmd, 
        char *cmdName,
        char *check_string, 
        uint32_t timeout_ms
    );
    int8_t check_response(char *data, char *response_check_string);
    void rotate_client_index();
    
#ifdef __cplusplus
}
#endif

#endif
