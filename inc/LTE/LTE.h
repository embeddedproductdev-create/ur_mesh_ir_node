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

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LTE_H
#define LTE_H

#include "../LTE/LTE.h"

// General commands
#define PROMPT					">"
#define PRODUCT_INFO			"ATI\r\n"
#define AT_CMD      			"AT\r\n"
#define GET_MANUFACTURER		"AT+CGMI\r\n"
#define GET_IMEI_NUM 	  		"AT+GSN=1\r\n"
#define FACTORY_RESET   		"AT&F0\r\n"
#define GET_CURRENT_CONFIG		"AT&V\r\n"
#define RESULT_CODE_ECHO_MODE	"ATQ0\r\n"
#define ERROR_FORMAT			"AT+CMEE=2\r\n"
#define ERROR_REPORT	        "AT+CEER\r\n"
#define INDICATOR_STATE         "AT+CIND=?\r\n"
#define GET_TIME				"AT+QLTS=2\r\n"

#define CLIENT_IDX 				2

// UART configuration
#define SET_DCD					"AT&C=0\r\n"
#define DTR_FUNCTION_MODE		"AT&D2\r\n"
#define HW_FLOW_CTRL      		"AT+IFC=2,2\r\n"
#define SET_FRAMING_FORMAT		"AT+ICF=3\r\n"
#define SET_BAUD_RATE			"AT+IPR="

//Status Control
#define ME_ACTIVITY_STATUS		"AT+CPAS=0\r\n"
#define NETWORK_SEARCH          "AT+QCFG=\"nwscanmode\",3,1\r\n"
#define SET_FREQUENCY_BAND      "AT+QCFG=\"band\,\r\n"
#define AIRPLANE_MODE 		    "AT+QCFG=\"airplanecontrol\",0\r\n"

//SIM Card Status
#define SIM_INSERT_STATUS       "AT+QSIMSTAT=1"

//Hardware commands
#define POWER_OFF   			"AT+QPOWD=0\r\n"
#define READ_CLOCK 				"AT+CCLK\r\n"
#define READ_BATTERY			"AT+CBC\r\n"
#define SLEEP_MODE   			"AT+QSCLK=2\r\n"
#define SYNC_SCAN_WIFI  		"AT+QWIFISCAN?\r\n"

//MQTT commands
#define MQTT_VERSION      		"AT+QMTCFG=\"version\","
#define MQTT_PDP	     		"AT+QMTCFG=\"pdpcid\","
#define ENABLE_SSL           	"AT+QMTCFG=\"ssl\","
#define MQTT_KEEP_ALIVE   		"AT+QMTCFG=\"keepalive\","
#define CLEAN_SESSION           "AT+QMTCFG=\"session\","
#define MQTT_WILL_CONFIG   		"AT+QMTCFG=\"will\","
#define MQTT_RECV_MODE    		"AT+QMTCFG=\"recv/mode\","
#define MQTT_SEND_MODE    		"AT+QMTCFG=\"send/mode\",1,0\r\n"
#define MQTT_HEARTBEAT    		"AT+QMTCFG=\"qmtping\",1\r\n"
#define MQTT_DATA_FORMAT  		"AT+QMTCFG=\"dataformat\",1,0,0\r\n"

#define MQTT_NETWORK_OPEN   	"AT+QMTOPEN="
#define MQTT_NETWORK_CLOSE  	"AT+QMTCLOSE="
#define MQTT_CLIENT_CONN   	 	"AT+QMTCONN="
#define MQTT_CLIENT_DISCONN 	"AT+QMTDISC="

//subscribe & publish
#define SUB_TO_TOPIC    		"AT+QMTSUB="
#define UNSUB_TO_TOPIC  		"AT+QMTUNS="
#define PUB_MSG 				"AT+QMTPUBEX="
#define READ_MSG_BUFFER  		"AT+QMTRECV="

// MQTT configuration Response
#define OK_RESPONSE				  "OK\r\n"
#define CONNECT_RESPONSE		  "CONNECT\r\n"
#define INDICATOR_STATE_RESPONSE  "+CIND: "
#define MQTT_VERSION_RESPONSE     "+QMTCFG:\"version\",3"
#define MQTT_SSL_RESPONSE		  "+QMTCFG:\"ssl\",1"
#define MQTT_KEEP_ALIVE_RESPONSE  "+QMTCFG:\"keepalive\",120"
#define MQTT_RECV_MODE_RESPONSE   "+QMTCFG:\"recv/mode\",0,0"
#define MQTT_HEARTBEAT_RESPONSE   "+QMTCFG:\"qmtping\",5"
#define MQTT_SEND_MODE_RESPONSE   "+QMTCFG:\"send/mode\",0"
#define MQTT_DATA_FORMAT_RESPONSE "+QMTCFG:\"dataformat\",0,0"

//MQTT responses
#define MQTT_NETWORK_OPEN_RESPONSE   "+QMTOPEN: "
#define MQTT_NETWORK_CLOSE_RESPONSE  "+QMTCLOSE: "
#define MQTT_CLIENT_CONN_RESPONSE    "+QMTCONN: "
#define MQTT_CLIENT_DISCONN_RESPONSE "+QMTDISC: "
#define MQTT_MSG_RECV_STATUS         "+QMTRECV: "
#define STATUS_MQTT_NETWORK	         "+QMTSTAT:1,1"
#define MQTT_SUB_RESPONSE		     "+QMTSUB: "
#define MQTT_UNSUB_RESPONSE		     "+QMTUNS: "
#define MQTT_PUB_MSG_RESPONSE        "+QMTPUBEX: "

//File commands
#define FILE_UPLOAD  			"AT+QFUPL="
#define FILE_LIST	  			"AT+QFLST=\"*\"\r\n"
#define FILE_DELETE	  			"AT+QFDEL="

//File response
#define FILE_UPLOAD_RESPONSE  	"+QFUPL: "

//SSL commands
#define SSL_SEC_WRITE           "AT+QSECWRITE="
#define SSL_SEC_READ            "AT+QSECREAD="
#define SSL_SEC_DELETE          "AT+QSECDEL="
#define SSL_CTX_INDEX	  		"AT+QSSLCFG=\"ctxindex\","
#define SSL_CONFIG_CACERT  		"AT+QSSLCFG=\"cacert\","
#define SSL_CONFIG_CCCERT  		"AT+QSSLCFG=\"clientcert\","
#define SSL_CONFIG_CLIKEY  		"AT+QSSLCFG=\"clientkey\","
#define SSL_CONFIG_AUTHMODE  	"AT+QSSLCFG=\"seclevel\","
#define SSL_CONFIG_AUTHVER  	"AT+QSSLCFG=\"sslversion\","
#define SSL_CONFIG_CIPHERSUITE  "AT+QSSLCFG=\"ciphersuite\","
#define SSL_CONFIG_AUTHTIME  	"AT+QSSLCFG=\"ignorelocaltime\","

//SSL response
#define SSL_SEC_WRITE_RESPONSE         "+QSECWRITE= "
#define SSL_SEC_READ_RESPONSE          "+QSECREAD= 1"

//LTE PINS
#define GPIO_LTE_RESET	46
#define GPIO_LTE_ONOFF	9
#define TXD_PIN (GPIO_NUM_18)
#define RXD_PIN (GPIO_NUM_17)
#define CTS_PIN (GPIO_NUM_11)
#define RTS_PIN (GPIO_NUM_10)
#define GPIO_OUTPUT_PIN_SEL (1ULL << GPIO_LTE_RESET) | (1ULL << GPIO_LTE_ONOFF);
#define RETRY_COUNT 2

/* GLOBAL VARIABLES */
extern bool restart_flag;
extern bool network_flag;
extern bool client_flag;
extern bool subscribe_flag;

/* FUNCTION DECLARATIONS */
void *LTE_task(void *args);
void LTE_setup(void);
void LTE_gpio_configuration(void);
void establishMQTTConnection(void);
void resetLte(void);
void sendAT_Data(const char* data);
void LTE_initialization(void);
uint8_t check_response(char* response, uint32_t timeout);
uint8_t MQTT_Config(uint8_t client_idx,
        uint8_t enable_ssl, uint8_t SSL_ctx_idx,
        uint16_t keep_alive,
        uint8_t clean_session,
        uint8_t msg_recv_mode,uint8_t msg_len_enable,
uint8_t will_fg, uint8_t will_qos, uint8_t will_retain, char* will_topic, char* will_message);
uint8_t SSL_config(uint8_t ssl_context_index, char* ca_cert, char* client_cert, char* client_key);
uint8_t SubscribeTopic(int client_idx, int msgid, char* topic, int qos);
uint8_t UnsubscribeTopic(int client_idx, int msgid, char* topic);
int MQTT_NetworkOpen(int client_idx, char* hostname, uint32_t port);
uint8_t MQTT_NetworkClose(int client_idx);
uint8_t MQTT_ClientConnect(int client_idx, char* username, char* passwd, char* clientID);
uint8_t MQTT_ClientDisconnect(int client_idx);
uint8_t PublishMessage(uint8_t client_idx, uint32_t msgid, uint8_t qos, uint8_t retain, char* topic);
uint8_t ReadMessage(int client_idx);
uint8_t Error_Report(void);
void parse_json_packet(void);
void fill_macid(void);

#ifdef __cplusplus
}
#endif

#endif

