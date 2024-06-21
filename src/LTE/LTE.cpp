/**
 * @file LTE.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions related to LTE communication
 * @version 0.8
 * @date 2024-06-19
 * @link https://evelta.com/content/datasheets/027-EC200UCNAA.pdf <-- Hardware design document link
 * @link https://auroraevernet.ru/upload/iblock/c81/rfhactu9l14ymr9cxt3pebdqxfu39h5v.pdf <-- MQTT AT commands manual
 * @copyright Copyright (c) 2024
 */

#include "../../inc/LTE/LTE.h"

#define SUCCESS 0
#define FAILURE -1

// Global Variable Initialization

uint8_t MQTT_CLIENT_INDEX = 5;

char mqtt_client_id[100];

bool LOG_DATA = true;

bool network_flag = false;
bool client_flag = false;
bool subscribe_flag = false;
bool mqtt_connected = false;
bool registered = false;
bool publishing_flag = false;
bool hold_adding_to_pubmesg = false;
bool need_to_activate_pdp = false;

char subscribe_topic[MQTT_TOPIC_CHAR_LEN];
char publish_topic[MQTT_TOPIC_CHAR_LEN];

// Local variable Initialization
char QMTSTAT_1_ERROR[20];
char QMTOPEN_2_ERROR[20];
char QMTOPEN_3_ERROR[20];

/*Basic AT commands*/
char AT_CMD[5];
char CHECK_FIRMWARE_CMD[5];
char CHECK_OPERATOR_SELECTION_CMD[10];
char CHECK_DOMAIN_REG_CMD[10];

/*PDP and TCP config*/
char TCP_CONFIG_CMD[50];
char PDP_CONTXT_ACT_CMD[20];	

/*MQTTConfig*/
char CLEAN_SESSION_CMD[50];
char KEEP_ALIVE_CMD[50];
char MSG_RECV_MODE_CMD[50];

/*NetworkOpening*/
char MQTT_NETWORK_OPEN_CMD[100];
char MQTT_NETWORK_OPEN_RESP[100];
char CHECK_NETWORK[15];
char NETWORK_OK[100];
char MQTT_NETWORK_CLOSE_CMD[100];

/*ClientConnection*/
char CHECK_CLIENT_CONN[15];
char CLIENT_CONN_OK[100];
char MQTT_CLIENT_DISCONN_CMD[30];
char MQTT_CLIENT_CONN_CMD[300];
char MQTT_CLIENT_CONN_RESP[300];

/*Subscription*/
char MQTT_SUB_CMD[100];
char MQTT_SUB_RESP[100];
char MQTT_READ_MSG_CMD[30];

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
	uart_driver_install(UART_NUM_1, 256, 0, 0, NULL, 0);
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
int8_t fetch_and_check_data(uint16_t timeout_ms, char *check_string, char *cmd_name)
{
	static uint8_t rotate_client_index_counter = 0;
	static uint8_t long_run_issue_counter = 0;

	char *LTE_UART_data = (char *)calloc(BUF_SIZE, sizeof(char));
	if (LTE_UART_data == NULL)
	{
		red_printf(LTE_ERROR_TAG, "Memory allocation failed for LTE_uart_data");
		return FAILURE;
	}

	if(LOG_DATA) {
		sprintf(lte_log_buffer, "HEAP FREE : %ld",ESP.getFreeHeap());
	}
	cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
	uint32_t in_time = esp_timer_get_time();
	while ((esp_timer_get_time() - in_time) / 1000 < timeout_ms)
	{
		int length = uart_read_bytes(UART_NUM_1, LTE_UART_data, BUF_SIZE, pdMS_TO_TICKS(100));
		if (length > 0)
		{
			//reset the counters
			rotate_client_index_counter = 0; 
			long_run_issue_counter = 0;
			if(LOG_DATA) {
				sprintf(lte_log_buffer, "AT_CMD sent : %s | BUF_LEN : %d", cmd_name, strlen(LTE_UART_data));
				cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
			}
			// It's safe to add to pubmesg now that we've received some data over UART from LTE
			hold_adding_to_pubmesg = false;
			if (check_response(LTE_UART_data, check_string) == SUCCESS)
			{
				sprintf(lte_log_buffer, "%d bytes Data Received : %s", length, LTE_UART_data);
				cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
				/*If its the case of READ MESG, then we need to parse JSON*/
				if (strstr(LTE_UART_data, "{"))
				{
					strcpy(LTE_UART_data, strstr(LTE_UART_data, "{"));
					ESP_LOGI(LTE_DEBUG_TAG, "Packet received : %s",LTE_UART_data);
					parse_json_packet(LTE_UART_data);
				}
				free(LTE_UART_data);
				return SUCCESS;
			}
			else
			{
				sprintf(lte_log_buffer, "%d bytes of Data Received : %s", length, LTE_UART_data);
				red_printf(LTE_ERROR_TAG, lte_log_buffer);
				rotate_client_index_counter++;
				if(rotate_client_index_counter > RETRY_COUNT)
				{
					rotate_client_index();
				}
				free(LTE_UART_data);
				return FAILURE;
			}
		}
	}
	free(LTE_UART_data);
	sprintf(lte_log_buffer, "No Data | data_len : %d", strlen(LTE_UART_data));
	red_printf(LTE_ERROR_TAG, lte_log_buffer);

	// To avoid mem leak due to LTE no reponse issue, we're using this flag to hold publishing more to the pubmesg queue,
	// cuz it's of no use, when LTE is not responding. This leads to loss of data. We need to fix this later
	hold_adding_to_pubmesg = true;
	long_run_issue_counter++;
	if(long_run_issue_counter > 30) ESP.restart();
	return FAILURE;
}

int8_t check_response(char *uart_data, char *check_string)
{
	if (strstr(uart_data, QMTSTAT_1_ERROR))
		return FAILURE;
	if (strstr(uart_data, QMTOPEN_2_ERROR))
		return FAILURE;
	if (strstr(uart_data, QMTOPEN_3_ERROR))
	{
		need_to_activate_pdp = true;
		return FAILURE;
	}
	if (strstr(uart_data, check_string))
	{
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
	//For safety reasons and in attemp to prevent the uart_read_bytes not responding after 1 hour issue, let's do the below
	//before we send each command
	uart_flush_input(UART_NUM_1);

	if (uart_write_bytes(UART_NUM_1, cmd, strlen(cmd)) != FAILURE)
	{
		if (fetch_and_check_data(timeout_ms, check_string, cmdName) == SUCCESS)
		{
			return SUCCESS;
		}
		return FAILURE;
	}
	else
	{
		sprintf(lte_log_buffer, "Error in sending AT command to the EC200!!!");
		red_printf(LTE_ERROR_TAG, lte_log_buffer);
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
	if (send_cmd_and_check_response(LOG_DATA, MQTT_PUBLISH_MESG_CMD, "PUBLISH_TO_MQTT", PROMPT, 1000) == SUCCESS)
	{
		if (uart_write_bytes(UART_NUM_1, pubmesg_queue_head->message, strlen(pubmesg_queue_head->message)) != FAILURE)
		{
			sleep(1); // We'll waste a second here to see if it helps with QMTSTAT 1 error
			publishing_flag = false;
			sprintf(queue_log_buffer, "Published :");
			yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
			sprintf(queue_log_buffer, "%s", pubmesg_queue_head->message);
			yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
			return SUCCESS;
		}
		else
		{
			sprintf(queue_log_buffer, "Publishing to MQTT Failed");
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
			return FAILURE;
		}
	}
	else
	{
		sprintf(queue_log_buffer, "PUBLISH_PROMPT not received");
		red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
		return FAILURE;
	}
	return FAILURE;
}

/**
 * @brief Function to initialize AT commands that will never change
 * @param none
 * @retval none
 */
void init_const_AT_cmd_strings()
{
	/*Basic*/
	sprintf(AT_CMD, "AT\r");
	sprintf(CHECK_FIRMWARE_CMD, "ATI\r");
	sprintf(CHECK_OPERATOR_SELECTION_CMD, "AT+COPS?\r");
	sprintf(CHECK_DOMAIN_REG_CMD, "AT+CREG?");

	/*Configuring TCP/IP parameters*/
	sprintf(TCP_CONFIG_CMD, "AT+QICSGP=2,1,\"airtelgprs.com\",\"\",\"\",0\r");

	/*Activating PDP context*/
	sprintf(PDP_CONTXT_ACT_CMD, "AT+QIACT=2\r");
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

	/*CleanSession configuration*/
	sprintf(CLEAN_SESSION_CMD, "AT+QMTCFG=\"session\",%d,1\r", MQTT_CLIENT_INDEX);

	/*KeepAlive configuration*/
	sprintf(KEEP_ALIVE_CMD, "AT+QMTCFG=\"keepalive\",%d,3000\r", MQTT_CLIENT_INDEX);

	/*MsgRecvMode configuration*/
	sprintf(MSG_RECV_MODE_CMD, "AT+QMTCFG=\"recv/mode\",%d,0\r", MQTT_CLIENT_INDEX);

	/*Network Open*/
	sprintf(MQTT_NETWORK_OPEN_CMD, "%s%d,\"%s\",%d\r", MQTT_NETWORK_OPEN, MQTT_CLIENT_INDEX, mqtt_server_ip, mqtt_port);
	sprintf(MQTT_NETWORK_OPEN_RESP, "+QMTOPEN: %d,0", MQTT_CLIENT_INDEX);

	/*CheckNetwork*/
	sprintf(CHECK_NETWORK, "AT+QMTOPEN?\r");
	sprintf(NETWORK_OK, "+QMTOPEN: %d,\"%s\",%d", MQTT_CLIENT_INDEX, mqtt_server_ip, mqtt_port);

	sprintf(MQTT_NETWORK_CLOSE_CMD, "%s%d\r", MQTT_NETWORK_CLOSE, MQTT_CLIENT_INDEX);

	/*CheckClientConn*/
	sprintf(CHECK_CLIENT_CONN, "AT+QMTCONN?\r");
	sprintf(CLIENT_CONN_OK, "+QMTCONN: %d,3", MQTT_CLIENT_INDEX);

	/*Client Connection*/
	sprintf(MQTT_CLIENT_CONN_CMD, "%s%d,\"%s\",\"%s\",\"%s\"\r", MQTT_CLIENT_CONN, MQTT_CLIENT_INDEX, mqtt_client_id, mqtt_broker_username, mqtt_broker_password);
	sprintf(MQTT_CLIENT_CONN_RESP, "+QMTCONN: %d,0,0", MQTT_CLIENT_INDEX);

	sprintf(MQTT_CLIENT_DISCONN_CMD, "%s%d\r", MQTT_CLIENT_DISCONN, MQTT_CLIENT_INDEX);
	sprintf(MQTT_READ_MSG_CMD, "%s%d\r", READ_MQTT_MESSAGE, MQTT_CLIENT_INDEX);

	/*Subscription*/
	sprintf(MQTT_SUB_CMD, "%s%d,%d,\"%s\",%d\r", SUB_TO_TOPIC, MQTT_CLIENT_INDEX, MQTT_MSGID, subscribe_topic, MQTT_QOS);
	sprintf(MQTT_SUB_RESP, "+QMTSUB: %d,%d,0", MQTT_CLIENT_INDEX, MQTT_MSGID);

	sprintf(QMTSTAT_1_ERROR, "+QMTSTAT: %d,1", MQTT_CLIENT_INDEX);
	sprintf(QMTOPEN_2_ERROR, "+QMTOPEN: %d,2", MQTT_CLIENT_INDEX);
	sprintf(QMTOPEN_3_ERROR, "+QMTOPEN: %d,3", MQTT_CLIENT_INDEX);
}

/**
 * @brief Function that performs the power up sequence of LTE.
 * @param none
 * @retval none
 * @warning Logic is Inverted
 */
void powerCycleLTE()
{
	sprintf(lte_log_buffer, "Power cycling LTE starts");
	cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
	gpio_set_level((gpio_num_t)GPIO_LTE_ONOFF, 0);
	vTaskDelay(pdMS_TO_TICKS(100));
	gpio_set_level((gpio_num_t)GPIO_LTE_ONOFF, 1);
	vTaskDelay(pdMS_TO_TICKS(2500));
	gpio_set_level((gpio_num_t)GPIO_LTE_ONOFF, 0);
	sprintf(lte_log_buffer, "Power cycling LTE end");
	cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
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
 * @brief Function that takes care of switching the client index after
 * multiple failure to MQTT Network Open command
 * @param none
 * @retval none
 */
void rotate_client_index()
{
	uint8_t old_client_index = MQTT_CLIENT_INDEX;
	if (MQTT_CLIENT_INDEX == 5)
		MQTT_CLIENT_INDEX = 0;
	else
		MQTT_CLIENT_INDEX++;
	snprintf(lte_log_buffer, sizeof(lte_log_buffer), "Old Client Index : %d | New Client Index : %d", old_client_index, MQTT_CLIENT_INDEX);
	cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
	init_Strings();
}

/**
 * @brief Function that performs basic AT cmd checks
 * @param none
 * @retval none
 */
void basic_LTE_checks()
{
	send_cmd_and_check_response(LOG_DATA, AT_CMD, "AT_CMD", OK_RESPONSE, 1000);
	send_cmd_and_check_response(LOG_DATA, CHECK_FIRMWARE_CMD, "CHECK_FIRMWARE_CMD", OK_RESPONSE, 1000);
	send_cmd_and_check_response(LOG_DATA, CHECK_OPERATOR_SELECTION_CMD, "CHECK_OP_SEL_CMD", OK_RESPONSE, 1000);
	send_cmd_and_check_response(LOG_DATA, CHECK_DOMAIN_REG_CMD, "CHECK_DOMAIN_REG_CMD", OK_RESPONSE, 1000);
}

/**
 * Function that takes care of maintaining the MQTT communication with the broker
 * @param none
 * @retval none
 */
void establishMQTTConnectionNew()
{
	/**
	 * Steps that need to be followed to establishing a working MQTT communication with the broker
	 * - Network Open
	 * - Client Connection
	 * - Subscribe to a Topic
	 * - Start checking for msgs from Broker
	 */
	static uint8_t retry_count = 0;
	//If we had recvd QMTOPEN 3 error, then it means PDP has not been activated and no point in trying further
	//Let's actiate it first
	// if(need_to_activate_pdp)
	// {
	// 	while(1)
	// 	{
	// 		vTaskDelay(1);
	// 		send_cmd_and_check_response(LOG_DATA, TCP_CONFIG_CMD, "TCP_CONFIG_CMD", OK_RESPONSE, 1000);
	// 		send_cmd_and_check_response(LOG_DATA, PDP_CONTXT_ACT_CMD, "PDP_ACTIVATION_CMD", OK_RESPONSE, 1000);
	// 		if(send_cmd_and_check_response(LOG_DATA, MQTT_NETWORK_OPEN_CMD, "MQTT_NETWORK_OPEN", MQTT_NETWORK_OPEN_RESP, 1000) != SUCCESS);
	// 		else 
	// 		{
	// 			need_to_activate_pdp = false;
	// 			break;
	// 		}
	// 	}
	// }

	//Printing the whole heap information
	// multi_heap_info_t *heap_data = NULL;
	// heap_caps_get_info(heap_data, MALLOC_CAP_DEFAULT);
	// printf("Allocated Blocks : %d\nFree Blocks : %d\nLargest Free Block : %d\nMinimum Free Block : %d\nTotal Allocated Bytes : %d\nTotal Free Bytes : %d\nTotal Blocks  %d\n",
	// heap_data->allocated_blocks, heap_data->free_blocks, heap_data->largest_free_block, 
	// heap_data->minimum_free_bytes, heap_data->total_allocated_bytes, heap_data->total_free_bytes, heap_data->total_blocks);

	if (send_cmd_and_check_response(LOG_DATA, CHECK_NETWORK, "CHECK_NETWORK", NETWORK_OK, 1000) == SUCCESS)
	{
		if (send_cmd_and_check_response(LOG_DATA, CHECK_CLIENT_CONN, "CHECK_CLIENT_CONN", CLIENT_CONN_OK, 1000) == SUCCESS)
		{
			if (send_cmd_and_check_response(LOG_DATA, MQTT_SUB_CMD, "MQTT_SUB", OK_RESPONSE, 1000) == SUCCESS)
			{
				while(1)
				{
					vTaskDelay(pdMS_TO_TICKS(50));
					// vTaskDelay(pdMS_TO_TICKS(1000)); //Adding a 1s delay to see if this improves the time after which the LTE long run issue occurs
					if (send_cmd_and_check_response(LOG_DATA, MQTT_READ_MSG_CMD, "MQTT_READ_MSG_CMD", OK_RESPONSE, 1000) == SUCCESS)
					{
						mqtt_connected = true;
					}
					else
					{
						mqtt_connected=false;
						sprintf(lte_log_buffer, "Unexpected MQTT disconnection");
						red_printf(LTE_ERROR_TAG, lte_log_buffer);
						break;
					}
					// Don't try to publish in the middle of sending an IR command or while sending another AT command
					// Some form of synchronization is required here.
					if (pubmesg_queue_head != NULL && mqtt_connected)
					{
						if (publish_to_mqtt() == SUCCESS)
						{
							remove_from_pubmesg_queue();
							snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Successfully published and removed from Queue");
							yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
						}
						else
						{
							mqtt_connected=false;
							snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Failed to publish to MQTT");
							red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
							break;
						}
					}
				}
			}
		}
		else
		{
			mqtt_connected = false;
			retry_count++;
			send_cmd_and_check_response(LOG_DATA, MQTT_CLIENT_CONN_CMD, "MQTT_CLIENT_CONN", MQTT_CLIENT_CONN_RESP, 1000);
		}
	}
	else
	{
		mqtt_connected = false;
		if(send_cmd_and_check_response(LOG_DATA, MQTT_NETWORK_OPEN_CMD, "MQTT_NETWORK_OPEN", MQTT_NETWORK_OPEN_RESP, 1000) != SUCCESS)
			retry_count++;
	}
	// If we've retried and failed more than RETRY_COUNT times, it's better we rotate client_index before retrying again.
	if (retry_count > RETRY_COUNT)
	{
		retry_count = 0;
		rotate_client_index();
	}
}

/**
 * @brief Function that takes care of maintainig the MQTT communication
 * @param none
 * @return none
 */
void establishMQTTConnection()
{
	static uint8_t retry_count = 0;
	// Do not try to send an AT command in the following cases
	// Already an AT command is in progress
	// IR command is being sent out
	sprintf(lte_log_buffer, "network_flag(%d) | client_flag(%d) | sub_flag(%d) | mqtt_connected(%d)", network_flag, client_flag, subscribe_flag, mqtt_connected);
	cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
	// If we are stuck at retrying for more than RETRY_COUNT times, then it's better to power cycle the LTE
	if (retry_count > RETRY_COUNT)
	{
		network_flag = 0;
		client_flag = 0;
		subscribe_flag = 0;
		retry_count = 0;
		rotate_client_index();
		powerCycleLTE();
	}

	// See if we have connected with Network first
	else if (!network_flag)
	{
		if (send_cmd_and_check_response(LOG_DATA, MQTT_NETWORK_CLOSE_CMD, "MQTT_NETWORK_CLOSE", OK_RESPONSE, 1000) == FAILURE)
			retry_count++;
		else
			retry_count = 0;
		if (send_cmd_and_check_response(LOG_DATA, MQTT_NETWORK_OPEN_CMD, "MQTT_NETWORK_OPEN", MQTT_NETWORK_OPEN_RESP, 1000) == FAILURE)
		{
			retry_count++;
		}
		else
		{
			network_flag = 1;
			retry_count = 0;
		}
	}

	// Then see if we have established a client connection
	else if (network_flag && !client_flag)
	{
		send_cmd_and_check_response(LOG_DATA, MQTT_CLIENT_DISCONN_CMD, "MQTT_CLIENT_DISCONN_CMD", OK_RESPONSE, 1000);
		if (send_cmd_and_check_response(LOG_DATA, MQTT_CLIENT_CONN_CMD, "MQTT_CLIENT_CONN_CMD", MQTT_CLIENT_CONN_RESP, 4000) == FAILURE)
		{
			retry_count++;
			network_flag = 0;
		}
		else
		{
			client_flag = 1;
			retry_count = 0;
		}
	}

	// Check if we have also subscribed to the topic
	else if (network_flag && client_flag && !subscribe_flag)
	{
		if (send_cmd_and_check_response(LOG_DATA, MQTT_SUB_CMD, "MQTT_SUB_CMD", MQTT_SUB_RESP, 1000) == FAILURE)
		{
			retry_count++;
			client_flag = 0;
		}
		else
		{
			subscribe_flag = 1;
			mqtt_connected = 1;
			retry_count = 0;
		}
	}

	// If everything looks good, then we are good to check if we have recvd any message from MQTT on the topic we have subscribed
	if (mqtt_connected && !publishing_flag)
	{
		if (send_cmd_and_check_response(LOG_DATA, MQTT_READ_MSG_CMD, "MQTT_READ_MSG_CMD", OK_RESPONSE, 200) == FAILURE)
		{
			retry_count++;
			mqtt_connected = 0;
			subscribe_flag = 0;
		}
		else
			retry_count = 0;
	}

	// Don't try to publish in the middle of sending an IR command or while sending another AT command
	// Some form of synchronization is required here.
	if (pubmesg_queue_head != NULL && mqtt_connected)
	{
		if (publish_to_mqtt() == SUCCESS)
		{
			remove_from_pubmesg_queue();
			snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Successfully published and removed from Queue");
			yellow_printf(QUEUE_DEBUG_TAG, queue_log_buffer);
		}
		else
		{
			snprintf(queue_log_buffer, sizeof(queue_log_buffer), "Failed to publish to MQTT");
			red_printf(QUEUE_ERROR_TAG, queue_log_buffer);
		}
	}
}

/**
 * @brief Function that takes care of configuring the MQTT communication
 * @param none
 * @retval none
 */
void MQTT_config()
{
	while (send_cmd_and_check_response(LOG_DATA, CLEAN_SESSION_CMD, "CLEAN_SESSION_CMD", OK_RESPONSE, 1000) != SUCCESS)
		;
	while (send_cmd_and_check_response(LOG_DATA, KEEP_ALIVE_CMD, "KEEP_ALIVE_CMD", OK_RESPONSE, 3000) != SUCCESS)
		;
}

/**
 * @brief Thread that takes care of MQTT-LTE communication
 * @param args
 * @return void*
 */
void LTE_task(void *args)
{
	init_const_AT_cmd_strings();
	init_Strings();
	LTE_gpio_configuration();
	powerCycleLTE();
	LTE_UART_INIT();
	basic_LTE_checks();
	MQTT_config();
	// send_cmd_and_check_response(LOG_DATA, TURN_OFF_ECHO_CMD, "TURN_OFF_ECHO_CMD", OK_RESPONSE, 100);
	// send_cmd_and_check_response(LOG_DATA, "AT+CMEE=2\r\n", "TURN ON VERBOSE LOGGING", OK_RESPONSE, 100);
	while(1)
	{
		vTaskDelay(pdMS_TO_TICKS(2000));
		establishMQTTConnectionNew();
	}
}