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

// Initialization
bool restart_flag = false;
bool network_flag = false;
bool client_flag = false;
bool subscribe_flag = false;
bool mqtt_connected = false;
bool registered = false;
bool publishing_flag = false;

char subscribe_topic[MQTT_TOPIC_CHAR_LEN];
char publish_topic[MQTT_TOPIC_CHAR_LEN];

char LTE_UART_data[2048];
char NETWORK_CONNECTION_SUCCESSFUL_RESPONSE[30];

uint8_t mqtt_qos = 2; // 0 = atmost once | 1 = atleast once | 2 = exactly once
uint8_t mqtt_retain = 0;
uint8_t mqtt_msgid = 2;

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

void sendAT_Data(const char *data)
{
	int err = uart_write_bytes(UART_NUM_1, data, strlen(data));
	if (err != -1) ESP_LOGI(TAG, "AT Command sent : %s",data);
	else ESP_LOGE(TAG, "Error in sending AT command to the EC200!!!");
}

int8_t fetch_data_from_LTE_UART(uint32_t timeout)
{
	uint32_t time = esp_timer_get_time();
	while ((esp_timer_get_time()- time)/1000 < timeout)
	{
		uint32_t length = uart_read_bytes(UART_NUM_1, LTE_UART_data, BUF_SIZE, 100);
		if(length > 0)
		{
			return SUCCESS;
		}
	}
	return FAILURE;
}

int8_t check_network_open_response()
{
	if(strstr(NETWORK_CONNECTION_SUCCESSFUL_RESPONSE, LTE_UART_data))
	{
		ESP_LOGI(DEBUG_TAG, "Network open successful");
		return SUCCESS;
	}
	else
	{	
		ESP_LOGE(ERROR_TAG, "%s", LTE_UART_data);
		return FAILURE;
	}
}

int8_t send_network_open_command()
{
	char *buf = (char *)calloc(BUF_SIZE, sizeof(char));
	sprintf(buf, "%s%d,\"%s\",%d\r\n", MQTT_NETWORK_OPEN, mqtt_client_index, mqtt_server_ip, mqtt_port);
	if(send_AT_cmd(buf, "Sending Network Open Command")==SUCCESS)
	{
		network_flag = true;
		return SUCCESS;
	}
	else 
	{
		free(buf);
		return FAILURE;
	}
	
}

int8_t send_client_connect_command()
{
	char *buf = (char *)calloc(BUF_SIZE, sizeof(char));
	sprintf(buf, "%s%d,\"%s\",\"%s\",\"%s\"\r\n", MQTT_CLIENT_CONN, mqtt_client_index, mqtt_client_id, mqtt_broker_username, mqtt_broker_password);
	if(send_AT_cmd(buf, "Sending Client Connection Command")==SUCCESS)
	{
		client_flag = true;
		return SUCCESS;
	}
	else
	{
		ESP_LOGE(ERROR_TAG, "Client connection failed");
		free(buf);
		return FAILURE;
	}
	
}

int8_t send_subscribe_topic_command()
{
	char *buf = (char *)calloc(BUF_SIZE, sizeof(char));
	sprintf(buf, "%s%d,%d,\"%s\",%d\r\n", SUB_TO_TOPIC, mqtt_client_index, mqtt_msgid, subscribe_topic, mqtt_qos);
	if(send_AT_cmd(buf, "Sending Subscribe Topic Command")==SUCCESS)
	{
		subscribe_flag = true;
		return SUCCESS;
	}
	else
	{
		free(buf);
		return FAILURE;
	}
}

int8_t send_read_command()
{
	char *buf = (char *)calloc(BUF_SIZE, sizeof(char));
	sprintf(buf, "%s%d\r\n", READ_MSG_BUFFER, mqtt_client_index);
	send_AT_cmd(buf, "Sending Read Message Command");
	free(buf);
	return SUCCESS;
}

int8_t check_client_connection_response()
{
	return SUCCESS;
}

int8_t check_subsribe_response()
{
	return SUCCESS;
}

int8_t check_readmessage_response()
{
	return SUCCESS;
}

int8_t check_if_connection_was_reset(char *data)
{
	char check_string[30];
	sprintf(check_string, "+QMTSTAT: %d,1", mqtt_client_index);
	if (strstr(data, check_string))
	{
		printf("Received +QMTSTAT: 1 Error. Restarting LTE\r\n");
		restart_flag = true;
		return SUCCESS;
	}
	return FAILURE;
}

int8_t check_response(char *response, uint32_t timeout)
{
	uint8_t index = 0, j = 0;
	bool copy_flag = false;
	char *data = (char *)calloc(BUF_SIZE, sizeof(char));
	uint32_t time = esp_timer_get_time() / 1000ULL;
	while ((esp_timer_get_time() / 1000ULL) - time < timeout)
	{
		vTaskDelay(pdMS_TO_TICKS(50));
		uint32_t length = uart_read_bytes(UART_NUM_1, data, BUF_SIZE, 100);
		if (length > 0) 
		{
			if(strstr(data,"ERROR"))
			{
				free(data);
				return FAILURE;
			}
			ESP_LOGI(TAG, "Received string : %s", (char *)data);
			// if(check_if_connection_was_reset(data)==SUCCESS) return FAILURE;
			if(mqtt_connected)
			{
				for(index=0,j=0; data[index] != '\0'; index++)
				{
					if(data[index]=='{' && copy_flag == false && strstr(data, "}"))
						copy_flag = true;
					else
						continue;
					while(copy_flag)
					{
						ESP_LOGI(DEBUG_TAG, "Inside copying");
						vTaskDelay(pdMS_TO_TICKS(50));
						json_packet[j++] = data[index++];
						if(data[index]=='}')
						{
							json_packet[j] = data[index];
							copy_flag = false;
							break;
						}
					}
					break;
				}
				ESP_LOGI(DEBUG_TAG, "JSON_PACKET : %s", json_packet);
			}
			free(data);
			return SUCCESS;
		}
	}
	free(data);
	return FAILURE;
}

void MQTT_Config(uint8_t enable_ssl, uint8_t SSL_ctx_idx,
					uint16_t keep_alive,
					uint8_t clean_session,
					uint8_t msg_recv_mode, uint8_t msg_len_enable,
					uint8_t will_fg, uint8_t will_qos, uint8_t will_retain, char *will_topic, char *will_message)
{
	char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
	
	sprintf((char *)transmit_buffer, "%s,%d,%d\r\n", MQTT_VERSION, mqtt_client_index, 3);
	send_AT_cmd(transmit_buffer, "Setting MQTT version");
	
	sprintf((char *)transmit_buffer, "%s%d,%d\r\n", MQTT_KEEP_ALIVE, mqtt_client_index, keep_alive);
	send_AT_cmd(transmit_buffer, "Setting Keep Alive");

	sprintf((char *)transmit_buffer, "%s%d,%d,%d\r\n", MQTT_RECV_MODE, mqtt_client_index, msg_recv_mode, msg_len_enable);
	send_AT_cmd(transmit_buffer, "Setting RecvMode");

	sprintf((char *)transmit_buffer, "%s%d,%d,%d,%d,\"%s\",\"%s\"\r\n",MQTT_WILL_CONFIG,mqtt_client_index,will_fg,will_qos,will_retain,will_topic,will_message);
	send_AT_cmd(transmit_buffer, "Setting Will Parameters");

	sprintf((char *)transmit_buffer, "%s%d,%d\r\n", CLEAN_SESSION, mqtt_client_index, clean_session);
	send_AT_cmd(transmit_buffer, "Setting Clean Session");
}

uint8_t SubscribeTopic(int mqtt_client_index, int msgid, char *topic, int qos)
{
	char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
	sprintf((char *)transmit_buffer, "%s%d,%d,\"%s\",%d\r\n", SUB_TO_TOPIC, mqtt_client_index, msgid, topic, qos);
	sendAT_Data((char *)transmit_buffer);
	memset(transmit_buffer, '\0', strlen(transmit_buffer));
	sprintf((char *)transmit_buffer, "%s%d,%d,0\r\n", MQTT_SUB_RESPONSE, mqtt_client_index, msgid);
	if (check_response(MQTT_SUB_RESPONSE, 150 * MAX_WAIT_MS) == SUCCESS)
	{
		ESP_LOGI(TAG, "Subscribed to topic:%s\r\n", topic);
		free(transmit_buffer);
		subscribe_flag = 1;
		return SUCCESS;
	}
	client_flag = 0;
	subscribe_flag = 0;
	ESP_LOGI(TAG, "Could not Subscribe to Topic. \r\n");
	free(transmit_buffer);
	return FAILURE;
}

uint8_t UnsubscribeTopic(int mqtt_client_index, int msgid, char *topic)
{
	char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
	sprintf((char *)transmit_buffer, "%s%d,%d,\"%s\"\r\n", UNSUB_TO_TOPIC, mqtt_client_index, msgid, topic);
	sendAT_Data((char *)transmit_buffer);
	memset(transmit_buffer, '\0', strlen(transmit_buffer));
	sprintf((char *)transmit_buffer, "%s%d,%d,0\r\n", MQTT_UNSUB_RESPONSE, mqtt_client_index, msgid);
	if (check_response(OK_RESPONSE, 150 * MAX_WAIT_MS) == SUCCESS)
	{
		ESP_LOGI(TAG, "Unsubscribed from topic:%s\r\n", topic);
		free(transmit_buffer);
		subscribe_flag = 0;
		return SUCCESS;
	}
	ESP_LOGI(TAG, "Could not Unsubscribe from Topic. \r\n");
	free(transmit_buffer);
	return FAILURE;
}

int MQTT_NetworkOpen(int mqtt_client_index, char *hostname, uint32_t port)
{
	char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
	sprintf((char *)transmit_buffer, "%s%d,\"%s\",%ld\r\n", MQTT_NETWORK_OPEN, mqtt_client_index, hostname, port);
	sendAT_Data((char *)transmit_buffer);
	memset(transmit_buffer, '\0', strlen(transmit_buffer));
	sprintf((char *)transmit_buffer, "%s%d,0\r\n", MQTT_NETWORK_OPEN_RESPONSE, mqtt_client_index);
	free(transmit_buffer);
	return FAILURE;
}

uint8_t MQTT_NetworkClose(int mqtt_client_index)
{
	char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
	sprintf((char *)transmit_buffer, "%s%d\r\n", MQTT_NETWORK_CLOSE, mqtt_client_index);
	sendAT_Data((char *)transmit_buffer);
	memset(transmit_buffer, '\0', strlen(transmit_buffer));
	sprintf((char *)transmit_buffer, "%s%d,0\r\n", MQTT_NETWORK_CLOSE_RESPONSE, mqtt_client_index);
	if (check_response(transmit_buffer, 300) == SUCCESS)
	{
		ESP_LOGI(TAG, "Closed MQTT network");
		free(transmit_buffer);
		network_flag = 0;
		return SUCCESS;
	}
	ESP_LOGI(TAG, "Could not close MQTT network. \r\n");
	free(transmit_buffer);
	return FAILURE;
}

uint8_t MQTT_ClientConnect(int mqtt_client_index, char *username, char *passwd, char *clientID)
{
	char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
	sprintf((char *)transmit_buffer, "%s%d,\"%s\",\"%s\",\"%s\"\r\n", MQTT_CLIENT_CONN, mqtt_client_index, clientID, username, passwd);
	sendAT_Data((char *)transmit_buffer);
	memset(transmit_buffer, '\0', strlen(transmit_buffer));
	sprintf((char *)transmit_buffer, "%s%d,0,0\r\n", MQTT_CLIENT_CONN_RESPONSE, mqtt_client_index);
	if (check_response(transmit_buffer, 15 * MAX_WAIT_MS) == SUCCESS)
	{
		ESP_LOGI(TAG, "Connected client to broker: %s\r\n", username);
		free(transmit_buffer);
		client_flag = 1;
		return SUCCESS;
	}
	client_flag = 0;
	ESP_LOGI(TAG, "Could not Connect client to broker.\r\n");
	free(transmit_buffer);
	return FAILURE;
}

uint8_t MQTT_ClientDisconnect(int mqtt_client_index)
{
	char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
	sprintf((char *)transmit_buffer, "%s%d\r\n", MQTT_CLIENT_DISCONN, mqtt_client_index);
	sendAT_Data((char *)transmit_buffer);
	memset(transmit_buffer, '\0', strlen(transmit_buffer));
	sprintf((char *)transmit_buffer, "%s%d,0", MQTT_CLIENT_DISCONN_RESPONSE, mqtt_client_index);
	if (check_response(OK_RESPONSE, 2 * MAX_WAIT_MS) == SUCCESS)
	{
		ESP_LOGI(TAG, "Disconnected client from broker");
		free(transmit_buffer);
		client_flag = 0;
		return SUCCESS;
	}
	ESP_LOGI(TAG, "Could not Disconnect client from broker.\r\n");
	free(transmit_buffer);
	return FAILURE;
}

uint8_t PublishMessage(uint8_t mqtt_client_index, uint32_t msgid, uint8_t qos, uint8_t retain, char *topic, char *message)
{
	char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
	sprintf((char *)transmit_buffer, "%s%d,%ld,%d,%d,\"%s\",%d\r\n", PUB_MSG, mqtt_client_index, msgid, qos, retain, topic, strlen(message));
	sendAT_Data((char *)transmit_buffer);
	memset(transmit_buffer, '\0', strlen(transmit_buffer));
	sprintf((char *)transmit_buffer, "%s%d,%ld,0\r\n", MQTT_PUB_MSG_RESPONSE, mqtt_client_index, msgid);
	if (check_response(PROMPT, 1500) == SUCCESS)
	{
		uart_write_bytes(UART_NUM_1, message, strlen(message));
		if (check_response(transmit_buffer, 1500) == SUCCESS)
		{
			free(transmit_buffer);
			return SUCCESS;
		}
	}
	free(transmit_buffer);
	return FAILURE;
}

uint8_t ReadMessage(int mqtt_client_index)
{
	char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
	sprintf((char *)transmit_buffer, "%s%d\r\n", READ_MSG_BUFFER, mqtt_client_index);
	sendAT_Data((char *)transmit_buffer);
	if (check_response(OK_RESPONSE, 2 * MAX_WAIT_MS) == SUCCESS)
	{
		free(transmit_buffer);
		return SUCCESS;
	}
	ESP_LOGI(TAG, "Could not receive message.\r\n");
	free(transmit_buffer);
	return FAILURE;
}

int8_t send_AT_cmd(char *cmd, char *requestString)
{
	ESP_LOGI(DEBUG_TAG, "%s : ", requestString);
	int err = uart_write_bytes(UART_NUM_1, cmd, strlen(cmd));
	if (err != -1) ESP_LOGI(TAG, "AT Command sent : %s",cmd);
	else ESP_LOGE(TAG, "Error in sending AT command to the EC200!!!");
	return check_response(OK_RESPONSE, 3000);
}

void perform_AT_cmd_sequence()
{	
	/*Check SIM Insertion status*/
	send_AT_cmd("AT+QSIMSTAT=1\r","Enabling Sim card Insertion Status");
	send_AT_cmd("AT+QSIMSTAT?\r","Checking SIM card Insertion status");

	/*SIM Hot swapping*/
	send_AT_cmd("AT+QSIMDET=1,1\r", "Setting SIM Hot Swapping");
	send_AT_cmd("AT+QSIMDET?\r","Checking SIM Hot swapping status");

	/*Check If SIM is locked with a pin*/
	send_AT_cmd("AT+CPIN?\r","Checking if PIN is required for SIM operation");

	/*LTE related*/
	send_AT_cmd("ATI\r", "Requesting Product Information");
	send_AT_cmd("AT+GMR\r", "Reqeusting TA Firmware Revision Identification");
	send_AT_cmd("AT+GSN\r","Reqeusting International Mobile Equipment Identity and Serial Number");
	send_AT_cmd("AT+CPAS\r", "Reqeusting Mobile Equipment Activity Status");
	send_AT_cmd("AT+COPS=?\r", "Setting Operator Selection");
	send_AT_cmd("AT+CREG=2\r", "Setting CS Doman Network REgistration Status");
	send_AT_cmd("AT+CREG?\r","Reqeusting CS Domain Network Registration Status");
	send_AT_cmd("AT+COPS?\r", "Reqeusting Operator Selection");
	send_AT_cmd("AT+CSQ?\r","Reqeusting Signal Quality Report");
	send_AT_cmd("AT+QSPN\r", "Reqeusting Service Provider Name");
	send_AT_cmd("AT+QCSQ\r","Reqeusting Signal Strength");
}

/**
 * @brief Function that stops MQTT communication and starts AP mode
 * @param none
 * @retval none
 */
void reset_mqtt()
{
	MQTT_NetworkClose(mqtt_client_index);
#if (AP_PART_ENABLED)
	mqtt_params_fetched_flag = false;
	clear_mqtt_settings();
	create_AP_task();
#endif
	mqtt_connected = false;
}

/**
 * @brief Function to clearoff mqtt settings upon reset MQTT request
 * @param none
 * @retval none
 */
void clear_mqtt_settings()
{
	mqtt_params_fetched_flag = false;
	mqtt_client_index = 99;
	mqtt_port = 1;
	memset(mqtt_server_ip, 0, strlen(mqtt_server_ip));
	memset(mqtt_broker_username, 0, strlen(mqtt_broker_username));
	memset(mqtt_broker_password, 0, strlen(mqtt_broker_password));
	memset(mqtt_client_id, 0, strlen(mqtt_client_id));
}

/**
 * @brief Function which contains a queue of data that needs
 * to be sent back to cloud
 * @param none
 * @retval none
 */
int8_t publish_to_mqtt()
{
	if (PublishMessage(mqtt_client_index, mqtt_msgid, mqtt_qos, mqtt_retain, pubmesg_queue_head->topic, pubmesg_queue_head->message) == SUCCESS)
	{
		publishing_flag = false;
		return SUCCESS;
	}
	publishing_flag = false;
	return FAILURE;
}

void LTE_initialization(void)
{
	mqtt_connected = false;
	network_flag = false;
	client_flag = false;
	subscribe_flag = false;
	perform_AT_cmd_sequence();
	MQTT_Config(0, 0, 120, 1, 0, 1, 0, 2, 0, "will/topic", "Network Disconnected unexpectedly");
}

void establishMQTTConnection()
{
	LED_state = LED_STATE_MQTT_NOT_CONNECTED;
	uint8_t network_connect_retry_count = 0;
	uint8_t client_connect_retry_count = 0;
	uint8_t subscribe_retry_count = 0;
	network_flag = false;
	client_flag = false;
	subscribe_flag = false;
	while(network_connect_retry_count < RETRY_COUNT && !network_flag && mqtt_params_fetched_flag)
	{
		vTaskDelay(1);
		client_connect_retry_count = 0;
		printf("NETWORK_CONNECT_RETRY_COUNT : %d\n",network_connect_retry_count++);
		uint8_t ret_val = send_network_open_command();
		if(ret_val == 2) MQTT_NetworkClose(mqtt_client_index);
		if(network_flag)
		{
			while(client_connect_retry_count < RETRY_COUNT && !client_flag) {
				subscribe_retry_count = 0;
				printf("CLIENT_CONNECT_RETRY_COUNT : %d\n",client_connect_retry_count++);
				if(send_client_connect_command()==SUCCESS)
				{
					while(subscribe_retry_count < RETRY_COUNT && !subscribe_flag) {
						printf("SUBSCRIBE_RETRY_COUNT : %d\n",subscribe_retry_count++);

						if(send_subscribe_topic_command()==SUCCESS)
							mqtt_connected = true;
					}
					if(subscribe_retry_count >= RETRY_COUNT) client_flag = 0;
					client_connect_retry_count = 0;
				}
				else
					printf("CLIENT CONNECTION FAILED\n");
			}
			if(client_connect_retry_count >= RETRY_COUNT)
			{
				network_flag = false;
				MQTT_NetworkClose(mqtt_client_index);
				network_connect_retry_count = 0;
			}
		}
		if(network_connect_retry_count == 5 && !network_flag)
			network_connect_retry_count = 0;
	}
}

void init_topic_and_responses()
{
	sprintf(subscribe_topic, "%d/commands", GWY_SER_NO);
	sprintf(publish_topic, "%d/messages", GWY_SER_NO);
	sprintf(mqtt_client_id, "GWY00020/68ca9045-fa01-44ec-b043-9465c73a542d");
	snprintf(NETWORK_CONNECTION_SUCCESSFUL_RESPONSE, sizeof(NETWORK_CONNECTION_SUCCESSFUL_RESPONSE),"+QMTOPEN: %d,0",mqtt_client_index);
	// snprintf(CLIENT_CONNECTION_SUCCESSFUL_RESPONSE, sizeof(CLIENT_CONNECTION_SUCCESSFUL_RESPONSE), "",);
	// snprintf(SUBSCRIBE_TOPIC_SUCCESSFUL_RESPONSE, sizeof(SUBSCRIBE_TOPIC_SUCCESSFUL_RESPONSE), "",);
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

/**
 * @brief Thread that takes care of MQTT-LTE communication
 * @param args
 * @return void*
 */
void *LTE_task(void *args)
{
	init_topic_and_responses();
	LTE_gpio_configuration();
	powerCycleLTE();
	LTE_UART_INIT();
	LTE_initialization();
	establishMQTTConnection();
	while (1)
	{
		vTaskDelay(pdMS_TO_TICKS(50));
		if (send_read_command() == SUCCESS)
		{
			if (strlen(json_packet) > 5)
			{
				parse_json_packet();
				strcpy(json_packet, "");
			}
		}
		else LTE_restart();
	}
}