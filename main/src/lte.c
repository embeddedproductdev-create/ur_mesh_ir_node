
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "../inc/lte.h"

#define SUCCESS 0
#define FAILURE -1

#define BAUD_RATE 921600

#define LTE_RESET_PIN 46
#define LTE_POWER_PIN 9
#define TXD_PIN 17
#define RXD_PIN 18
#define CTS_PIN 11
#define RTS_PIN 10
#define UART_BUF_SIZE 800

#define GWY_SER_NO_IN_STRING "GWY00002"

#define MQTT_SERVER_IP "54.215.188.103"
#define MQTT_PORT 1883
#define MQTT_BROKER_USERNAME "QmaxSystems"
#define MQTT_BROKER_PASSWORD "Qmax_mosquitto_!@#"

#define MQTT_TOPIC_CHAR_LEN 100
#define MQTT_CMD_RESP_LEN 70

#define RETRY_COUNT 5

/*Basic AT commands*/
const char *AT_CMD = "AT\r";
const char *CHECK_FIRMWARE_CMD = "ATI\r";
const char *CHECK_OPERATOR_SELECTION_CMD = "AT+COPS?\r";
const char *CHECK_DOMAIN_REG_CMD = "AT+CREG?\r";
const char *ENABLE_SIM_HOTSWAP_CMD = "AT+QSIMDET=1,0\r";
const char *PING_CMD = "AT+QPING=1,\"google.com\",4,1\r";
const char *PING_RESP = "+QPING:";
char SET_BAUD_RATE_CMD[MQTT_CMD_RESP_LEN];

/*PDP and TCP config*/
const char *TCP_CONFIG_CMD = "AT+QICSGP=1,1,\"airtelgprs.com\",\"\",\"\",0\r";
const char *PDP_CONTXT_ACT_CMD = "AT+QIACT=1\r";

/*MQTT Cmd and Resp*/
const char *CLEAN_SESSION_CMD = "AT+QMTCFG=\"session\",2,1\r";
const char *KEEP_ALIVE_CMD = "AT+QMTCFG=\"keepalive\",2,3000\r";
const char *MSG_RECV_MODE_CMD = "AT+QMTCFG=\"recv/mode\",2,0\r";

char MQTT_NETWORK_OPEN_CMD[MQTT_CMD_RESP_LEN];
const char *MQTT_NETWORK_OPEN = "AT+QMTOPEN=";
const char *MQTT_NETWORK_OPEN_RESP = "+QMTOPEN: 2,0";
const char *CHECK_NETWORK_CMD = "AT+QMTOPEN?\r";
char NETWORK_OK[MQTT_CMD_RESP_LEN];

const char *MQTT_NETWORK_CLOSE_CMD = "AT+QMTCLOSE=2";
const char *MQTT_NETWORK_CLOSE_RESP = "+QMTCLOSE: 2,0";

char MQTT_CLIENT_CONN_CMD[MQTT_CMD_RESP_LEN];
const char *MQTT_CLIENT_CONN_RESP = "+QMTCONN: 2,0,0";
const char *CHECK_CLIENT_CONN_CMD = "AT+QMTCONN?\r";
const char *MQTT_CLIENT_CONN_OK_RESP = "+QMTCONN: 2,3";
const char *MQTT_CLIENT_DISCONN_CMD = "AT+QMTDISC=2";

const char *MQTT_READ_MSG_CMD = "AT+QMTRECV=2";
const char *OK_RESP = "OK\r\n";

char MQTT_SUB_CMD[MQTT_CMD_RESP_LEN];
const char *MQTT_SUB_RESP = "+QMTSUB: 2,2,0";

const char *QMTSTAT_1_ERROR = "+QMTSTAT: 2,1";
const char *QMTOPEN_2_ERROR = "+QMTOPEN: 2,2";
const char *QMTOPEN_3_ERROR = "+QMTOPEN: 2,3";

char subscribe_topic[MQTT_TOPIC_CHAR_LEN];
char publish_topic[MQTT_TOPIC_CHAR_LEN];

bool need_to_activate_pdp = false;
bool mqtt_connected = false;
bool LOG_DATA  = true;

/**
 * @brief Function that takes care of sending an AT command to LTE, await response, check that response
 * is valid or not and perform necessary operation.
 * @param logging Flag that enables/disables logging of LTE-MQTT communication data
 * @param cmd The AT command that needs to be sent
 * @param requestString A string denoting what command is being sent. Gets logged if logging is enabled.
 * @param check_string String that needs to be compared against the response that we get from the LTE chip
 * @param response_wait_time_ms How long should the LTE chip wait for a response (in milliseconds)
 * @return 0=Success, -1=Failure
 */
int8_t send_cmd_and_check_response(bool logging, char *cmd,
								   const char *cmdName, const char *check_string, uint32_t timeout_ms)
{
	uart_flush_input(UART_NUM_1);
	if (uart_write_bytes(UART_NUM_1, cmd, strlen(cmd)) != FAILURE)
	{
		if(logging) ESP_LOGI(LTE_DEBUG_TAG, "Command sent : %s", cmd);
		if (fetch_and_check_data(logging, timeout_ms, check_string, cmdName) == SUCCESS)
		{
			return SUCCESS;
		}
		return FAILURE;
	}
	else
	{
		custom_printf(LTE_ERROR_TAG, "Error in sending AT command to the EC200!!!", RED);
		return FAILURE;
	}
}

/**
 * Function that takes care of maintaining the MQTT communication with the broker
 * and also publishing ACK message from pubmessage queue to the Cloud
 * @param none
 * @retval none
 */
void establishMQTTConnection()
{
	static uint8_t ping_fail_counter = 0;	
	if(need_to_activate_pdp)
	{
		send_cmd_and_check_response(LOG_DATA, TCP_CONFIG_CMD, "TCP_CONFIG_CMD", OK_RESP, 500);
		if(send_cmd_and_check_response(LOG_DATA, PDP_CONTXT_ACT_CMD, "PDP_CONTXT_ACT_CMD", OK_RESP, 500))
			need_to_activate_pdp = false;
		return;
	}
	if(send_cmd_and_check_response(LOG_DATA, MQTT_NETWORK_OPEN_CMD, "MQTT_NETWORK_OPEN_CMD", MQTT_NETWORK_OPEN_RESP, 1000)!=SUCCESS) return;
	if(send_cmd_and_check_response(LOG_DATA, MQTT_CLIENT_CONN_CMD, "MQTT_CLIENT_CONN_CMD", MQTT_CLIENT_CONN_RESP, 1000)!=SUCCESS) return;
	if(send_cmd_and_check_response(LOG_DATA, MQTT_SUB_CMD, "MQTT_SUB_CMD", MQTT_SUB_RESP, 1000)!=SUCCESS) return ;
	ESP_LOGI(LTE_TAG, "Resumed MQTT Connection");
	while(1)
	{
		send_cmd_and_check_response(LOG_DATA, MQTT_READ_MSG_CMD, "MQTT_READ_MSG_CMD", OK_RESP, 100);
		if (pubmesg_queue_head != NULL)
		{
			if (publish_to_mqtt() == SUCCESS) remove_from_pubmesg_queue();
		}
		if(send_cmd_and_check_response(LOG_DATA, PING_CMD, "PING_CMD", OK_RESP, 300)!=SUCCESS) {
			if(ping_fail_counter++ > RETRY_COUNT){
				ESP_LOGE(LTE_TAG, "Lost MQTT connection");
				ping_fail_counter = 0;
				mqtt_connected = false;
				LOG_DATA = true;
				break;
			}
			continue;
		}
		ping_fail_counter = 0;
		mqtt_connected = true;
		LOG_DATA = false;
	}
}

/**
 * @brief Function that takes care of configuring the MQTT communication
 * @param none
 * @retval none
 */
void MQTT_config()
{
	send_cmd_and_check_response(LOG_DATA, CLEAN_SESSION_CMD, "CLEAN_SESSION_CMD", OK_RESP, 1000);
	send_cmd_and_check_response(LOG_DATA, KEEP_ALIVE_CMD, "KEEP_ALIVE_CMD", OK_RESP, 3000);
}

/**
 * @brief Function that performs basic AT cmd checks at startup
 * @param none
 * @retval none
 */
void execute_general_AT_cmds()
{
	send_cmd_and_check_response(LOG_DATA, SET_BAUD_RATE_CMD, "SET_BAUD_RATE", OK_RESP, 200);
	send_cmd_and_check_response(LOG_DATA, "AT&V\r", "DISPLAY_CURRENT_CONFIGURATION", OK_RESP, 200);
	send_cmd_and_check_response(LOG_DATA, "ATE0\r", "TURN_OFF_ECHO_CMD", OK_RESP, 200);
	send_cmd_and_check_response(LOG_DATA, CHECK_FIRMWARE_CMD, "CHECK_FIRMWARE_CMD", OK_RESP, 200);
	send_cmd_and_check_response(LOG_DATA, CHECK_OPERATOR_SELECTION_CMD, "CHECK_OP_SEL_CMD", OK_RESP, 200);
	send_cmd_and_check_response(LOG_DATA, CHECK_DOMAIN_REG_CMD, "CHECK_DOMAIN_REG_CMD", OK_RESP, 200);
	send_cmd_and_check_response(LOG_DATA, ENABLE_SIM_HOTSWAP_CMD, "ENABLE_SIM_HOTSWAP_CMD", OK_RESP, 200);
}

/**
 * @brief Function that takes care of initializing certain
 * string variables that we use across the LTE communication
 * like Subscribe topic, publish topic, etc.,
 * @param none
 * @retval none
 */
void initialize_mqtt_cmd_strings()
{
	sprintf(subscribe_topic, "%s/commands", GWY_SER_NO_IN_STRING);
	sprintf(publish_topic, "%s/messages", GWY_SER_NO_IN_STRING);
    sprintf(SET_BAUD_RATE_CMD, "AT+IPR=%d\r", BAUD_RATE);
	sprintf(MQTT_NETWORK_OPEN_CMD, "%s2,\"%s\",%d\r", MQTT_NETWORK_OPEN, MQTT_SERVER_IP, MQTT_PORT);
	sprintf(NETWORK_OK, "+QMTOPEN: 2,\"%s\",%d", MQTT_SERVER_IP, MQTT_PORT);
	sprintf(MQTT_CLIENT_CONN_CMD, "%s2,\"%s\",\"%s\",\"%s\"\r", "AT+QMTCONN=", GWY_SER_NO_IN_STRING, MQTT_BROKER_USERNAME, MQTT_BROKER_USERNAME);
	sprintf(MQTT_SUB_CMD, "%s2,2,\"%s\",2\r", "AT+QMTSUB=", subscribe_topic);

}

/**
 * @brief Function that configures the POWER and RESET pins of LTE as Output
 * @param none
 * @retval none
 */
void lte_gpio_configuration()
{
	gpio_set_direction(LTE_RESET_PIN, GPIO_MODE_OUTPUT);
	gpio_set_direction(LTE_POWER_PIN, GPIO_MODE_OUTPUT);
}

/**
 * @brief Function that performs the power up sequence of LTE.
 * @param none
 * @retval none
 * @warning Logic is Inverted
 */
void power_cycle_lte()
{
	gpio_set_level(LTE_POWER_PIN, 0);
	vTaskDelay(pdMS_TO_TICKS(100));
	gpio_set_level(LTE_POWER_PIN, 1);
	vTaskDelay(pdMS_TO_TICKS(2500));
	gpio_set_level(LTE_POWER_PIN, 0);
}

/**
 * @brief configure esp32 uart
 * @param None
 * @retval None
 */
void lte_uart_init(void)
{
	const uart_config_t uart_config = {
		.baud_rate = BAUD_RATE,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB,
	};
	uart_driver_install(UART_NUM_1, UART_BUF_SIZE, 0, 0, NULL, 0);
	uart_param_config(UART_NUM_1, &uart_config);
	uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

/**
 * @brief Thread that takes care of Establishing and Maintaining the LTE communication
 * @param args 
 */
void lte_task(void *args)
{
	initialize_mqtt_cmd_strings();
	lte_gpio_configuration();
	power_cycle_lte();
	lte_uart_init();
	execute_general_AT_cmds();
	MQTT_config();
    while(1)
	{
		vTaskDelay(pdMS_TO_TICKS(500));
		establishMQTTConnection();
	}
}