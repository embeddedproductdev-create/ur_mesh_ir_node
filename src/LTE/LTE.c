/**
 * @file LTE.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions related to LTE communication
 * @version 0.5
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
bool sending_at_cmd = false;

char subscribe_topic[MQTT_TOPIC_CHAR_LEN];
char publish_topic[MQTT_TOPIC_CHAR_LEN];

//Local variable Initialization
char QMTSTAT_1_ERROR[20];
char MQTT_NETWORK_OPEN_CMD[100];
char MQTT_NETWORK_CLOSE_CMD[100];
char MQTT_CLIENT_DISCONN_CMD[30];
char MQTT_CLIENT_CONN_CMD[300];
char MQTT_SUB_CMD[100];
char MQTT_READ_MSG_CMD[30];

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

/**
 * @brief Function that fetches data from LTE and checks if it is valid or not
 * @param cmd_id ID that is used to know what cmd was sent. This is being used instead of string comparisons which are expensive
 * @param timeout_ms How long should LTE wait for a response for the sent command. 
 * @param check_string String that is used to check against the response to know if it is valid or not 
 * @return int8_t 
 */
int8_t fetch_and_check_data(uint16_t timeout_ms, char *check_string)
{
	char *LTE_UART_data = (char *)calloc(BUF_SIZE, sizeof(char));
	uint32_t in_time = esp_timer_get_time();
	while((esp_timer_get_time()-in_time)/1000 < timeout_ms)
	{
		int length = uart_read_bytes(UART_NUM_1, LTE_UART_data, BUF_SIZE, 500);
		if(length > 0) 
		{
			if(check_response(LTE_UART_data, check_string)==SUCCESS)
			{
				if(LOG_LTE_DATA) ESP_LOGI(LTE_DEBUG_TAG, "%d bytes Data Received : %s", length, LTE_UART_data);
				/*If its the case of READ MESG, then we need to parse JSON*/
				if(strstr(LTE_UART_data, "{")){
					strcpy(LTE_UART_data, strstr(LTE_UART_data, "{"));
					parse_json_packet(LTE_UART_data);
				}
				free(LTE_UART_data); 
				return SUCCESS; 
			}
			else
			{
				if(LOG_LTE_DATA) ESP_LOGE(LTE_ERROR_TAG, "%d bytes of Data Received : %s", length, LTE_UART_data);
				free(LTE_UART_data); 
				return FAILURE; 
			}
		}
	}
	ESP_LOGE(LTE_ERROR_TAG, "No Data recevied from LTE");
	free(LTE_UART_data); 
	return FAILURE;
}

int8_t check_response(char *LTE_UART_data, char *check_string)
{
	if(strstr(LTE_UART_data, QMTSTAT_1_ERROR)) return FAILURE;
	if(strstr(LTE_UART_data, check_string)) {
		return SUCCESS;
	}
	return FAILURE;
}

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
char *cmdName, char *check_string, uint32_t timeout_ms)
{
	sending_at_cmd = true;
	if(logging) ESP_LOGI(LTE_DEBUG_TAG, "%s", cmdName);
	if (uart_write_bytes(UART_NUM_1, cmd, strlen(cmd)) != FAILURE) 
	{
		if(logging) ESP_LOGI(LTE_DEBUG_TAG, "Command sent : %s",cmd);
		if(fetch_and_check_data(timeout_ms, check_string)==SUCCESS)
		{ 
			sending_at_cmd = false;
			return SUCCESS;
		}
		sending_at_cmd = false;
		return FAILURE;
	}
	else 
	{
		ESP_LOGE(TAG, "Error in sending AT command to the EC200!!!");
		sending_at_cmd = false;
		return FAILURE;
	}
}

/**
 * @brief Function which contains a queue of data that needs
 * to be sent back to cloud
 * @param none
 * @retval none
 */
int8_t publish_to_mqtt()
{	
	publishing_flag = true;
	char MQTT_PUBLISH_MESG_CMD[PUBMESG_LEN];
	sprintf(MQTT_PUBLISH_MESG_CMD, "%s%d,%d,%d,%d,\"%s\",%d\r\n", PUBLISH_TO_MQTT, MQTT_CLIENT_INDEX, MQTT_MSGID, MQTT_QOS, MQTT_RETAIN, pubmesg_queue_head->topic, strlen(pubmesg_queue_head->message));
	if(!sending_at_cmd)
	{
		if(send_cmd_and_check_response(LOG_LTE_DATA, MQTT_PUBLISH_MESG_CMD, "PUBLISH_TO_MQTT", PROMPT, 1000) == SUCCESS)
		{
			if(uart_write_bytes(UART_NUM_1, pubmesg_queue_head->message,strlen(pubmesg_queue_head->message))!=FAILURE)
			{
				publishing_flag = false;
				return SUCCESS;
			}
			else
			{
				ESP_LOGE(QUEUE_DEBUG_TAG, "Publishing to MQTT Failed");
				return FAILURE;
			}
		}
	}
	return FAILURE;
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
	sprintf(MQTT_READ_MSG_CMD, "%s%d\r",READ_MQTT_MESSAGE,MQTT_CLIENT_INDEX);
	sprintf(MQTT_SUB_CMD,"%s%d,%d,\"%s\",%d\r",SUB_TO_TOPIC,MQTT_CLIENT_INDEX,MQTT_MSGID,subscribe_topic,MQTT_QOS);
	sprintf(QMTSTAT_1_ERROR, "+QMTSTAT:%d,1", MQTT_CLIENT_INDEX);
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

/**
 * @brief Function that takes care of maintainig the MQTT communication
 * @param none
 * @return 0=Success, -1=Failure 
 */
void establishMQTTConnection()
{
	static uint8_t retry_count = 0;
	if(!sending_at_cmd && !sending)
	{	
		if(LOG_LTE_DATA) ESP_LOGI(LTE_DEBUG_TAG, "network_flag(%d) | client_flag(%d) | sub_flag(%d) | mqtt_connected(%d)", network_flag, client_flag, subscribe_flag, mqtt_connected);
		//If we are stuck at retrying for more than RETRY_COUNT times, then it's better to power cycle the LTE
		if(retry_count > RETRY_COUNT) 
		{
			network_flag = 0; client_flag = 0; subscribe_flag = 0; retry_count = 0; 
			powerCycleLTE();
		}
		
		//See if we have connected with Network first
		else if(!network_flag) 
		{	
			if(send_cmd_and_check_response(LOG_LTE_DATA, MQTT_NETWORK_CLOSE_CMD, "MQTT_NETWORK_CLOSE", OK_RESPONSE, 1000)==FAILURE) retry_count++;
			else retry_count = 0;
			if(send_cmd_and_check_response(LOG_LTE_DATA, MQTT_NETWORK_OPEN_CMD, "MQTT_NETWORK_OPEN", OK_RESPONSE, 1000)==FAILURE)
			{ 
				retry_count++;
			}
			else {network_flag = 1; retry_count = 0;}
		}
		
		//Then see if we have established a client connection
		else if(network_flag && !client_flag)
		{ 
			send_cmd_and_check_response(LOG_LTE_DATA, MQTT_CLIENT_DISCONN_CMD, "MQTT_CLIENT_DISCONN_CMD", OK_RESPONSE, 1000);
			if(send_cmd_and_check_response(LOG_LTE_DATA, MQTT_CLIENT_CONN_CMD, "MQTT_CLIENT_CONN_CMD", OK_RESPONSE, 1000)==FAILURE)
			{
				retry_count++; network_flag = 0;
			} 
			else {client_flag = 1; retry_count = 0;}
		}
		
		//Check if we have also subscribed to the topic
		else if(network_flag && client_flag && !subscribe_flag)
		{
			if(send_cmd_and_check_response(LOG_LTE_DATA, MQTT_SUB_CMD, "MQTT_SUB_CMD", OK_RESPONSE, 1000)==FAILURE)
			{
				retry_count++; client_flag = 0;		
			}
			else {subscribe_flag = 1; mqtt_connected = 1; retry_count = 0;}
		} 
		
		//If everything looks good, then we are good to check if we have recvd any message from MQTT on the topic we have subscribed
		if(mqtt_connected && !publishing_flag) 
		{
			if(send_cmd_and_check_response(LOG_LTE_DATA, MQTT_READ_MSG_CMD, "MQTT_READ_MSG_CMD", OK_RESPONSE, 200)==FAILURE) {
				retry_count++; mqtt_connected = 0; subscribe_flag = 0;
			} 
			else retry_count = 0;
		}
	}
}

/**
 * @brief Thread that takes care of MQTT-LTE communication
 * @param args
 * @return void*
 */
void LTE_task(void *args)
{
	init_Strings();
	LTE_gpio_configuration();
	powerCycleLTE();
	LTE_UART_INIT();
	send_cmd_and_check_response(LOG_LTE_DATA, TURN_OFF_ECHO_CMD, "TURN_OFF_ECHO_CMD", OK_RESPONSE, 100);
	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(50));
		establishMQTTConnection();
	}
}