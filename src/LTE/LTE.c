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

bool restart_flag = false;
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
char MQTT_CLIENT_CONN_CMD[300];
char MQTT_SUB_CMD[100];

char LTE_UART_data[2048];
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

int8_t fetch_data_from_LTE_UART(uint16_t timeout_ms)
{
	uint32_t in_time = esp_timer_get_time();
	while ((esp_timer_get_time()- in_time)/1000 < timeout_ms)
	{
		uint32_t length = uart_read_bytes(UART_NUM_1, LTE_UART_data, BUF_SIZE, 100);
		if(length > 0) {
			ESP_LOGI(LTE_DEBUG_TAG, "Received : %s", LTE_UART_data);
			return SUCCESS;
		}
	}
	return FAILURE;
}

void MQTT_Config()
{	
	char cmd[100];
	sprintf(cmd, "%s,%d,%d",SET_MQTT_VERSION, MQTT_CLIENT_INDEX, MQTT_VERSION);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, cmd, "SET_MQTT_VERSION", OK_RESPONSE, 300);
	
	sprintf(cmd, "%s%d,%d\r\n", SET_KEEP_ALIVE, MQTT_CLIENT_INDEX, MQTT_KEEP_ALIVE_S);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, cmd, "SET_MQTT_KEEP_ALIVE", OK_RESPONSE, 300);

	sprintf(cmd, "%s%d,%d,%d\r\n", SET_MSG_RECV_MODE, MQTT_CLIENT_INDEX, MQTT_MSG_RECV_MODE, MQTT_MSG_LEN_ENABLE);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, cmd, "SET_MSG_RECV_MODE", OK_RESPONSE, 300);

	sprintf(cmd, 
	"%s%d,%d,%d,%d,\"%s\",\"%s\"\r\n",
	MQTT_WILL_CONFIG,
	MQTT_CLIENT_INDEX,
	MQTT_WILL_FLAG,
	MQTT_WILL_QOS,
	MQTT_WILL_RETAIN,
	MQTT_WILL_TOPIC,
	MQTT_WILL_MESSAGE);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, cmd, "MQTT_WILL_CONFIG", OK_RESPONSE, 300);

	sprintf(cmd, "%s%d,%d\r\n", SET_CLEAN_SESSION, MQTT_CLIENT_INDEX, MQTT_CLEAN_SESSION);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, cmd, "SET_CLEAN_SESSION", OK_RESPONSE, 300);
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

int8_t check_response(char *response_check_string)
{
	if(strstr(LTE_UART_data, response_check_string))
	{
		// if(strstr(LTE_UART_data, "{") && strstr(LTE_UART_data, "}"))
		// {
		// 	if(split_out_JSON_part()==SUCCESS) return SUCCESS;
		// 	else return FAILURE;
		// }
		return SUCCESS;
	}
	return FAILURE;
}

int8_t send_cmd_and_check_response(bool log_sent_command, char *cmd, char *requestString, char *response_check_string, uint32_t response_wait_time_ms)
{
	if(log_sent_command) ESP_LOGI(LTE_DEBUG_TAG, "%s : ", requestString);
	if (uart_write_bytes(UART_NUM_1, cmd, strlen(cmd)) != FAILURE) 
	{
		if(log_sent_command) ESP_LOGI(TAG, "Command being sent : %s",cmd);
		if(fetch_data_from_LTE_UART(response_wait_time_ms)==SUCCESS) 
		{
			if(check_response(response_check_string)==SUCCESS) return SUCCESS;
			else return FAILURE;
		}
		else return FAILURE;
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
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, ENABLE_SIM_INSERTION_STATUS,"ENABLE_SIM_INSERTION_STATUS", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, GET_SIM_INSERTION_STATUS,"GET_SIM_INSERTION_STATUS", OK_RESPONSE, 300);

	/*SIM Hot swapping*/
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, ENABLE_SIM_HOT_SWAPPING, "ENABLE_SIM_HOT_SWAPPING",OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, GET_SIM_HOT_SWAPPING_STATUS,"GET_SIM_HOT_SWAPPING_STATUS", OK_RESPONSE, 300);

	/*Check If SIM is locked with a pin*/
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, GET_SIM_PIN_LOCK_STATUS,"GET_SIM_PIN_LOCK_STATUS", OK_RESPONSE, 300);

	/*LTE related*/
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, GET_PRODUCT_INFO, "GET_PRODUCT_INFO", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, GET_FW_REVISION, "GET_FW_REVISION", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, GET_ME_SERIAL_NO,"GET_ME_SERIAL_NO", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, GET_ME_ACTIVITY_STATUS, "GET_ME_ACTIVITY_STATUS", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, ENABLE_NETWORK_REGISTRATION, "ENABLE_NETWORK_REGISTRATION", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, GET_NETWORK_REGISTRATION_STATUS,"GET_NETWORK_REGISTRATION_STATUS", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, GET_CURRENT_OPERATOR_STATUS, "GET_CURRENT_OPERATOR_STATUS", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, GET_SERVICE_PROVIDER_NAME, "GET_SERVICE_PROVIDER_NAME", OK_RESPONSE, 300);
	send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, GET_SIGNAL_STRENGTH, "GET_SIGNAL_STRENGTH", OK_RESPONSE, 300);
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
	if (send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, cmd, "PUBLISH_TO_MQTT", OK_RESPONSE, 1500) == SUCCESS)
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
	sprintf(mqtt_client_id, "%s/68ca9045-fa01-44ec-b043-9465c73a542d", GWY_SER_NO_IN_STRING);
	sprintf(MQTT_NETWORK_OPEN_CMD,"%s%d,\"%s\",%d\r\n", MQTT_NETWORK_OPEN, MQTT_CLIENT_INDEX, mqtt_server_ip, mqtt_port);
	sprintf(MQTT_NETWORK_CLOSE_CMD,"%s%d\r\n",MQTT_NETWORK_CLOSE,MQTT_CLIENT_INDEX);
	sprintf(MQTT_CLIENT_CONN_CMD,"%s%d,\"%s\",\"%s\",\"%s\"\r\n",MQTT_CLIENT_CONN,MQTT_CLIENT_INDEX,mqtt_client_id,mqtt_broker_username,mqtt_broker_password);
	sprintf(MQTT_SUB_CMD,"%s%d,%d,\"%s\",%d\r\n",SUB_TO_TOPIC,MQTT_CLIENT_INDEX,MQTT_MSGID,subscribe_topic,MQTT_QOS);
}

void LTE_restart()
{
	mqtt_connected = false;
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
	ESP_LOGI(TAG, "Power Cycling LTE !!!");
	gpio_set_level(GPIO_LTE_ONOFF, 0);
	gpio_set_level(GPIO_LTE_RESET, 0);
	vTaskDelay(pdMS_TO_TICKS(100));
	gpio_set_level(GPIO_LTE_ONOFF, 1);
	vTaskDelay(pdMS_TO_TICKS(2500));
	gpio_set_level(GPIO_LTE_ONOFF, 0);
	ESP_LOGI(TAG, "Power Cycling LTE complete");
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
	while(send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, MQTT_NETWORK_OPEN_CMD, "MQTT_NETWORK_OPEN", OK_RESPONSE, 1500)!=SUCCESS)
	{vTaskDelay(pdMS_TO_TICKS(50));}
	while(send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, MQTT_CLIENT_CONN_CMD, "MQTT_NETWORK_OPEN", OK_RESPONSE, 1500)!=SUCCESS)
	{vTaskDelay(pdMS_TO_TICKS(50));}
	while(send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, MQTT_SUB_CMD, "MQTT_NETWORK_OPEN", OK_RESPONSE, 1500)!=SUCCESS)
	{vTaskDelay(pdMS_TO_TICKS(50));}
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
	LTE_initialization();
	establishMQTTConnection();
	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(50));
		if(send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, GET_SIM_INSERTION_STATUS, "GET_SIM_INSERTION_STATUS", SIM_INSERTION_RESPONSE, 300) == SUCCESS);
		else 
		{
			ESP_LOGE(LTE_DEBUG_TAG, "+QSIMSTAT: 1,0 ==> SIM Not Inserted ==> Restarting LTE");
			LTE_restart();
		}
		if(send_cmd_and_check_response(LOG_SENT_COMMAND_FLAG, READ_MQTT_MESSAGE, "READ_MQTT_MESSAGE", OK_RESPONSE, 1500) == SUCCESS)
		{
			if(strstr(LTE_UART_data, "{") && strstr(LTE_UART_data, "}")) parse_json_packet();
		}
		else LTE_restart();
	}
}