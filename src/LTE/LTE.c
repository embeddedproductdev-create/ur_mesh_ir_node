/**
 * @file LTE.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions related to LTE communication
 * @version 0.1
 * @date 2024-02-29
 * @link https://evelta.com/content/datasheets/027-EC200UCNAA.pdf <-- Hardware design document link
 * @link https://auroraevernet.ru/upload/iblock/c81/rfhactu9l14ymr9cxt3pebdqxfu39h5v.pdf <-- MQTT AT commands manual
 * @copyright Copyright (c) 2024
 */

#include "../../inc/LTE/LTE.h"

#define SUCCESS 0
#define FAILURE -1
#define MAX_WAIT_MS 100
#define BUF_SIZE 2048

// Global Variable Initialization
char mqtt_client_id[100];

bool LOG_LTE_DATA = true;

bool network_flag = false;
bool client_flag = false;
bool subscribe_flag = false;
bool mqtt_connected = false;
bool registered = false;
bool publishing_flag = false;

char subscribe_topic[MQTT_TOPIC_CHAR_LEN];
char publish_topic[MQTT_TOPIC_CHAR_LEN];

//Local variable Initialization
char MQTT_NETWORK_OPEN_CMD[100];
char MQTT_NETWORK_CLOSE_CMD[100];
char MQTT_CLIENT_DISCONN_CMD[300];
char MQTT_CLIENT_CONN_CMD[300];
char MQTT_SUB_CMD[100];
char MQTT_READ_MSG_CMD[100];

char NETWORK_CONNECTION_SUCCESSFUL_RESPONSE[30];

/**
 * @brief configure esp32 uart
 * @param None
 * @retval None
 */
void LTE_UART_INIT(void)
{
	const uart_config_t uart_config = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB,
	};
	uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
	uart_param_config(UART_NUM_1, &uart_config);
	uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int8_t fetch_data_from_LTE_UART(char *cmd, uint16_t timeout_ms, char *check_string)
{
	char *LTE_UART_data = (char *)calloc(BUF_SIZE, sizeof(char));
	uint32_t in_time = esp_timer_get_time();
	while((esp_timer_get_time()-in_time)/1000 < timeout_ms)
	{
		int length = uart_read_bytes(UART_NUM_1, LTE_UART_data, BUF_SIZE, 500);
		if(length > 0) {
			if(strstr(LTE_UART_data, "{")) {
				ESP_LOGI(LTE_DEBUG_TAG, "Copying UART data into json_packet");
				strcpy(json_packet, strstr(LTE_UART_data, "{"));
			} 
			if(LOG_LTE_DATA) ESP_LOGI(LTE_DEBUG_TAG, "%d bytes Data Received : %s", length, LTE_UART_data);
			if(check_response(LTE_UART_data, check_string)==SUCCESS)
			{
				if(!strcmp(cmd,MQTT_NETWORK_OPEN_CMD)) network_flag = 1;
				else if(!strcmp(cmd,MQTT_CLIENT_CONN_CMD)) client_flag = 1;
				else if(!strcmp(cmd,MQTT_SUB_CMD)) subscribe_flag = 1;
				return SUCCESS;
			}
			if(!strcmp(cmd,MQTT_NETWORK_OPEN_CMD)) network_flag = 0;
			else if(!strcmp(cmd,MQTT_CLIENT_CONN_CMD)) {network_flag = 0; client_flag = 0;}
			else if(!strcmp(cmd,MQTT_SUB_CMD)) {network_flag = 0; client_flag = 0; subscribe_flag = 0;}
			return FAILURE;
		}
	}
	free(LTE_UART_data);
	if(LOG_LTE_DATA) ESP_LOGI(LTE_DEBUG_TAG, "JSON_PACKET after freeing LTE_UART_data : %s", json_packet);
	if(LOG_LTE_DATA) ESP_LOGE(LTE_ERROR_TAG, "No data received");
	return FAILURE;
}

void MQTT_Config()
{	
	char cmd[100];
	// sprintf(cmd, "%s,%d,%d",SET_MQTT_VERSION, MQTT_CLIENT_INDEX, MQTT_VERSION);
	// send_cmd_and_check_response(LOG_LTE_DATA, cmd, "SET_MQTT_VERSION", OK_RESPONSE, 300);
	
	sprintf(cmd, "%s%d,%d\r\n", SET_KEEP_ALIVE, MQTT_CLIENT_INDEX, MQTT_KEEP_ALIVE_S);
	send_cmd_and_check_response(LOG_LTE_DATA, cmd, "SET_MQTT_KEEP_ALIVE", OK_RESPONSE, 300);

	sprintf(cmd, "%s%d,%d,%d\r\n", SET_MSG_RECV_MODE, MQTT_CLIENT_INDEX, MQTT_MSG_RECV_MODE, MQTT_MSG_LEN_ENABLE);
	send_cmd_and_check_response(LOG_LTE_DATA, cmd, "SET_MSG_RECV_MODE", OK_RESPONSE, 300);

	sprintf(cmd, 
	"%s%d,%d,%d,%d,\"%s\",\"%s\"\r\n",
	MQTT_WILL_CONFIG,
	MQTT_CLIENT_INDEX,
	MQTT_WILL_FLAG,
	MQTT_WILL_QOS,
	MQTT_WILL_RETAIN,
	MQTT_WILL_TOPIC,
	MQTT_WILL_MESSAGE);
	send_cmd_and_check_response(LOG_LTE_DATA, cmd, "MQTT_WILL_CONFIG", OK_RESPONSE, 300);

	sprintf(cmd, "%s%d,%d\r\n", SET_CLEAN_SESSION, MQTT_CLIENT_INDEX, MQTT_CLEAN_SESSION);
	send_cmd_and_check_response(LOG_LTE_DATA, cmd, "SET_CLEAN_SESSION", OK_RESPONSE, 300);
}

// int8_t split_out_JSON_part()
// {
// 	uint8_t index=0,j=0;
// 	bool copy_flag = false;
// 	for(index=0,j=0; LTE_UART_data[index] != '\0'; index++)
// 	{
// 		if(LTE_UART_data[index]=='{' && copy_flag == false)
// 			copy_flag = true;
// 		else
// 			continue;
// 		while(copy_flag)
// 		{
// 			vTaskDelay(1);
// 			json_packet[j++] = LTE_UART_data[index++];
// 			if(LTE_UART_data[index]=='}')
// 			{
// 				json_packet[j] = LTE_UART_data[index];
// 				copy_flag = false;
// 				break;
// 			}
// 		}
// 		ESP_LOGI(LTE_DEBUG_TAG, "JSON_PACKET : %s", json_packet);
// 		strcpy(LTE_UART_data, "");
// 		break;
// 	}
// }

int8_t check_response(char *LTE_UART_data, char *response_check_string)
{
	if(strstr(LTE_UART_data, CME_ERROR_10)) {
		free(LTE_UART_data);
		ESP_LOGE(LTE_ERROR_TAG, "FATAL: SIM NOT INSERTED");
		esp_restart_flag = true;
	}
	if(strstr(LTE_UART_data, response_check_string)) {
		strcpy(json_packet, LTE_UART_data);
		free(LTE_UART_data);
		return SUCCESS;
	}
	free(LTE_UART_data);
	return FAILURE;
}

int8_t send_cmd_and_check_response(bool log_sent_command, char *cmd, char *requestString, char *response_check_string, uint32_t response_wait_time_ms)
{
	if(log_sent_command) ESP_LOGI(LTE_DEBUG_TAG, "%s", requestString);
	if (uart_write_bytes(UART_NUM_1, cmd, strlen(cmd)) != FAILURE) 
	{
		if(log_sent_command) ESP_LOGI(LTE_DEBUG_TAG, "Command being sent : %s",cmd);
		if(fetch_data_from_LTE_UART(cmd, response_wait_time_ms, response_check_string)==SUCCESS) return SUCCESS;
		if(!strcmp(cmd,MQTT_NETWORK_OPEN_CMD)) network_flag = 0;
		else if(!strcmp(cmd,MQTT_CLIENT_CONN_CMD)) {network_flag = 0; client_flag = 0;}
		else if(!strcmp(cmd,MQTT_SUB_CMD)) {network_flag = 0; client_flag = 0; subscribe_flag = 0;}
		return FAILURE;
	}
	else 
	{
		ESP_LOGE(TAG, "Error in sending AT command to the EC200!!!");
		return FAILURE;
	}
}

void initial_AT_cmd_seq()
{	
	/*Check SIM Insertion status*/
	send_cmd_and_check_response(LOG_LTE_DATA, ENABLE_SIM_INSERTION_STATUS,"ENABLE_SIM_INSERTION_STATUS", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_LTE_DATA, GET_SIM_INSERTION_STATUS,"GET_SIM_INSERTION_STATUS", SIM_INSERTION_STATUS_RESPONSE, 300);

	/*SIM Hot swapping*/
	send_cmd_and_check_response(LOG_LTE_DATA, ENABLE_SIM_HOT_SWAPPING, "ENABLE_SIM_HOT_SWAPPING",OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_LTE_DATA, GET_SIM_HOT_SWAPPING_STATUS,"GET_SIM_HOT_SWAPPING_STATUS", OK_RESPONSE, 300);

	/*Check If SIM is locked with a pin*/
	send_cmd_and_check_response(LOG_LTE_DATA, GET_SIM_PIN_LOCK_STATUS,"GET_SIM_PIN_LOCK_STATUS", OK_RESPONSE, 300);

	/*LTE related*/
	send_cmd_and_check_response(LOG_LTE_DATA, GET_PRODUCT_INFO, "GET_PRODUCT_INFO", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_LTE_DATA, GET_FW_REVISION, "GET_FW_REVISION", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_LTE_DATA, GET_ME_SERIAL_NO,"GET_ME_SERIAL_NO", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_LTE_DATA, GET_ME_ACTIVITY_STATUS, "GET_ME_ACTIVITY_STATUS", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_LTE_DATA, ENABLE_NETWORK_REGISTRATION, "ENABLE_NETWORK_REGISTRATION", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_LTE_DATA, LIST_ALL_OPERATORS, "LIST_ALL_OPERATORS", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_LTE_DATA, GET_NETWORK_REGISTRATION_STATUS,"GET_NETWORK_REGISTRATION_STATUS", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_LTE_DATA, SET_CURRENT_OPERATOR_STATUS, "GET_CURRENT_OPERATOR_STATUS", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_LTE_DATA, GET_CURRENT_OPERATOR_STATUS, "GET_CURRENT_OPERATOR_STATUS", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_LTE_DATA, GET_SERVICE_PROVIDER_NAME, "GET_SERVICE_PROVIDER_NAME", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_LTE_DATA, GET_SIGNAL_STRENGTH, "GET_SIGNAL_STRENGTH", OK_RESPONSE, 300);
}

/**
 * @brief Function which contains a queue of data that needs
 * to be sent back to cloud
 * @param none
 * @retval none
 */
int8_t publish_to_mqtt()
{
	char cmd[PUBMESG_LEN];
	sprintf(cmd, "%s%d,%d,%d,%d,\"%s\",%d\r\n", PUBLISH_TO_MQTT, MQTT_CLIENT_INDEX, MQTT_MSGID, MQTT_QOS, MQTT_RETAIN, pubmesg_queue_head->topic, strlen(pubmesg_queue_head->message));
	if (send_cmd_and_check_response(LOG_LTE_DATA, cmd, "PUBLISH_TO_MQTT", OK_RESPONSE, 1500) == SUCCESS)
	{
		publishing_flag = false;
		return SUCCESS;
	}
	publishing_flag = false;
	return FAILURE;
}

void LTE_initialization(void)
{
	initial_AT_cmd_seq();
	MQTT_Config();
	establishMQTTConnection();
}

/**
 * @brief Function that takes care of initializing certain 
 * string variables that we use across the LTE communication
 * like Subscribe topic, publish topic, etc.,
 * @param none
 * @retval none
 */
void init_Strings()
{
	sprintf(subscribe_topic, "%s/commands", GWY_SER_NO_IN_STRING);
	sprintf(publish_topic, "%s/messages", GWY_SER_NO_IN_STRING);
	sprintf(mqtt_client_id, "%s/07ebf099-2d3e-451e-9f8b-0cf34838a246", GWY_SER_NO_IN_STRING);
	sprintf(MQTT_NETWORK_OPEN_CMD,"%s%d,\"%s\",%d\r", MQTT_NETWORK_OPEN, MQTT_CLIENT_INDEX, mqtt_server_ip, mqtt_port);
	sprintf(MQTT_NETWORK_CLOSE_CMD,"%s%d\r",MQTT_NETWORK_CLOSE,MQTT_CLIENT_INDEX);
	sprintf(MQTT_CLIENT_CONN_CMD,"%s%d,\"%s\",\"%s\",\"%s\"\r",MQTT_CLIENT_CONN,MQTT_CLIENT_INDEX,mqtt_client_id,mqtt_broker_username,mqtt_broker_password);
	sprintf(MQTT_CLIENT_DISCONN_CMD,"%s%d\r",MQTT_CLIENT_DISCONN,MQTT_CLIENT_INDEX);
	sprintf(MQTT_READ_MSG_CMD, "%s%d",READ_MQTT_MESSAGE,MQTT_CLIENT_INDEX);
	sprintf(MQTT_SUB_CMD,"%s%d,%d,\"%s\",%d\r",SUB_TO_TOPIC,MQTT_CLIENT_INDEX,MQTT_MSGID,subscribe_topic,MQTT_QOS);
}

void LTE_restart()
{
	powerCycleLTE();
	LTE_initialization();
}

/**
 * @brief Function that performs the power up sequence of LTE.
 * @param none
 * @retval none
 * @warning Logic is Inverted
 */
void powerCycleLTE()
{
	// ESP_LOGI(LTE_DEBUG_TAG, "Power Cycling LTE Starts !!!");
	gpio_set_level(GPIO_LTE_ONOFF, 0);
	gpio_set_level(GPIO_LTE_RESET, 0);
	vTaskDelay(pdMS_TO_TICKS(100));
	gpio_set_level(GPIO_LTE_ONOFF, 1);
	vTaskDelay(pdMS_TO_TICKS(2500));
	gpio_set_level(GPIO_LTE_ONOFF, 0);
	// ESP_LOGI(LTE_DEBUG_TAG, "Power Cycling LTE complete");
}

/**
 * @brief Function that configures the POWER and RESET pins of LTE as Output
 * @param none
 * @retval none
 */
void LTE_gpio_configuration()
{
	pinMode(GPIO_LTE_RESET, OUTPUT);
	pinMode(GPIO_LTE_ONOFF, OUTPUT);
}

int8_t establishMQTTConnection()
{
	static uint8_t retry_count = 0;
	// ESP_LOGI(LTE_DEBUG_TAG, "retry_count : %d\n | network_flag : %d | client_flag : %d | subscribe_flag : %d\n | mqtt_connected_flag : %d",
	// retry_count, network_flag, client_flag, subscribe_flag, mqtt_connected);
	if(!mqtt_connected && retry_count++ > RETRY_COUNT) {retry_count = 0; powerCycleLTE();}
	else if(!network_flag) 
	{	
		send_cmd_and_check_response(LOG_LTE_DATA, MQTT_NETWORK_CLOSE_CMD, "MQTT_NETWORK_CLOSE", OK_RESPONSE, 1000);
		send_cmd_and_check_response(LOG_LTE_DATA, MQTT_NETWORK_OPEN_CMD, "MQTT_NETWORK_OPEN", OK_RESPONSE, 1000);
	}
	else if(network_flag && !client_flag)
	{ 
		send_cmd_and_check_response(LOG_LTE_DATA, MQTT_CLIENT_DISCONN_CMD, "MQTT_CLIENT_DISCONN_CMD", OK_RESPONSE, 1000);
		send_cmd_and_check_response(LOG_LTE_DATA, MQTT_CLIENT_CONN_CMD, "MQTT_CLIENT_CONN_CMD", OK_RESPONSE, 1000);
	}
	else if(network_flag && client_flag && !subscribe_flag) send_cmd_and_check_response(LOG_LTE_DATA, MQTT_SUB_CMD, "MQTT_SUB_CMD", OK_RESPONSE, 1000);
	else if(network_flag && client_flag && subscribe_flag) mqtt_connected = true;
	if(mqtt_connected) send_cmd_and_check_response(LOG_LTE_DATA, MQTT_READ_MSG_CMD, "MQTT_READ_MSG_CMD", OK_RESPONSE, 100);
	return SUCCESS;
}

/**
 * @brief Thread that takes care of MQTT-LTE communication
 * @param args
 * @return void*
 */
void *LTE_task(void *args)
{
	init_Strings();
	LTE_gpio_configuration();
	powerCycleLTE();
	LTE_UART_INIT();
	MQTT_Config();
	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(50));
		establishMQTTConnection();
		if(strlen(json_packet) > 5) parse_json_packet();
		memset(json_packet, "\0", sizeof(json_packet));
	}
}