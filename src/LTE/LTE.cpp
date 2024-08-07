/**
 * @file LTE.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions related to LTE communication
 * @version 0.8.7
 * @date 2024-07-19
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

char LTE_UART_data[BUF_SIZE];

bool LOG_DATA = false;

bool network_flag = false;
bool client_flag = false;
bool subscribe_flag = false;
bool mqtt_connected = false;
bool registered = false;
bool publishing_flag = false;
bool hold_adding_to_pubmesg = false;
bool need_to_rotate_client_index = false;
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
char ENABLE_SIM_HOTSWAP_CMD[20];
char PING_CMD[30];
char PING_RESP[10];

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
char MQTT_NETWORK_CLOSE_RESP[35];

/*ClientConnection*/
char CHECK_CLIENT_CONN[15];
char CLIENT_CONN_OK[100];
char MQTT_CLIENT_DISCONN_CMD[30];
char MQTT_CLIENT_CONN_CMD[300];
char MQTT_CLIENT_CONN_RESP[300];

/*Subscription*/
char MQTT_SUB_CMD[200];
char MQTT_SUB_RESP[100];
char MQTT_READ_MSG_CMD[30];

/*OTA*/
char START_UDP_SERVICE_CMD[500];
char START_UDP_SERVICE_RESP[20];
char UDP_RECV_CMD[20];
char UDP_RECV_RESP[1500];
char CHECK_SOCKET_STATE_CMD[20];

/**
 * @brief configure esp32 uart
 * @param None
 * @retval None
 */
void LTE_UART_INIT(void)
{
	const uart_config_t uart_config = {
		.baud_rate = 921600,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB,
	};
	uart_driver_install(UART_NUM_1, BUF_SIZE, 0, 0, NULL, 0);
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
int8_t fetch_and_check_data(bool logging, uint16_t timeout_ms, char *check_string, char *cmd_name)
{
	static uint8_t long_run_issue_counter = 0;
	// char *LTE_UART_data = (char *)calloc(BUF_SIZE, sizeof(char));
	bzero(LTE_UART_data, BUF_SIZE);
	// if (LTE_UART_data == NULL)
	// {
	// 	sprintf(lte_log_buffer, "Memory allocation failed for LTE_uart_data");
	// 	custom_printf(LTE_DEBUG_TAG, lte_log_buffer, RED);
	// 	return FAILURE;
	// }
	uart_flush(UART_NUM_1);
	int length = uart_read_bytes(UART_NUM_1, LTE_UART_data, BUF_SIZE, timeout_ms);
	if (length > 0)
	{
		long_run_issue_counter = 0;
		/*It's safe to add to pubmesg now that we've received some data over UART from LTE*/
		hold_adding_to_pubmesg = false;
		if(logging) ESP_LOGI(LTE_DEBUG_TAG, "Received : %s", LTE_UART_data);
		if (check_response(LTE_UART_data, check_string) == SUCCESS)
		{
			/*If its the case of READ MESG, then we need to parse JSON*/
			if (strstr(LTE_UART_data, "{"))
			{
				strcpy(LTE_UART_data, strstr(LTE_UART_data, "{"));
				custom_printf(LTE_DEBUG_TAG, LTE_UART_data, CYAN);
				parse_json_packet(LTE_UART_data);
			}
			// free(LTE_UART_data);
			return SUCCESS;
		}
		else
		{
			// free(LTE_UART_data);
			return FAILURE;
		}
	}
	// free(LTE_UART_data);
	sprintf(lte_log_buffer, "No Data | data_len : %d", strlen(LTE_UART_data));
	custom_printf(LTE_ERROR_TAG, lte_log_buffer, RED);

	// To avoid mem leak due to LTE no reponse issue, we're using this flag to hold publishing more to the pubmesg queue,
	// cuz it's of no use, when LTE is not responding. This leads to loss of data. We need to fix this later
	hold_adding_to_pubmesg = true;
	long_run_issue_counter++;
	if(long_run_issue_counter > 30) ESP.restart();
	return FAILURE;
}

int8_t check_response(char *uart_data, char *check_string)
{
	if (strstr(uart_data, check_string)) return SUCCESS;
	if (strstr(uart_data, QMTSTAT_1_ERROR))
		return FAILURE;
	if (strstr(uart_data, QMTOPEN_2_ERROR)){
		need_to_rotate_client_index = true;
		return FAILURE;
	}
	if (strstr(uart_data, QMTOPEN_3_ERROR))
	{
		need_to_activate_pdp = true;
		return FAILURE;
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
		red_printf(LTE_ERROR_TAG, "Error in sending AT command to the EC200!!!");
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
			publishing_flag = false;
			sleep(1);
			return SUCCESS;
		}
		else
		{
			sprintf(lte_log_buffer, "Publishing to MQTT Failed");
			custom_printf(LTE_ERROR_TAG, lte_log_buffer, RED);
			return FAILURE;
		}
	}
	else
	{
		sprintf(lte_log_buffer, "PUBLISH_PROMPT not received");
		custom_printf(LTE_ERROR_TAG, lte_log_buffer, RED);
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
	sprintf(CHECK_DOMAIN_REG_CMD, "AT+CREG?\r");

	/*Hot Swap*/
	sprintf(ENABLE_SIM_HOTSWAP_CMD, "AT+QSIMDET=1,0\r");

	/*Configuring TCP/IP parameters*/
	sprintf(TCP_CONFIG_CMD, "AT+QICSGP=1,1,\"airtelgprs.com\",\"\",\"\",0\r");

	/*Activating PDP context*/
	sprintf(PDP_CONTXT_ACT_CMD, "AT+QIACT=1\r");

	/*Ping Command*/
	sprintf(PING_CMD, "AT+QPING=1,\"google.com\"\r");
	sprintf(PING_RESP, "+QPING:");
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
	sprintf(MQTT_NETWORK_CLOSE_RESP, "+QMTCLOSE: %d,0",MQTT_CLIENT_INDEX);

	/*Check Client Connection*/
	sprintf(CHECK_CLIENT_CONN, "AT+QMTCONN?\r");
	sprintf(CLIENT_CONN_OK, "+QMTCONN: %d,3", MQTT_CLIENT_INDEX);

	/*Make Client Connection*/
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

	/*OTA*/
	sprintf(START_UDP_SERVICE_CMD, "AT+QIOPEN=1,0,\"UDP SERVICE\",\"54.215.188.103\",0,1881,0\r");
	sprintf(START_UDP_SERVICE_RESP, "AT+QIOPEN:");
	sprintf(UDP_RECV_CMD, "AT+QIRD=1\r");
	sprintf(UDP_RECV_RESP, "+QIRD:");
	sprintf(CHECK_SOCKET_STATE_CMD, "AT+QISTATE?\r");
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
void execute_general_AT_cmds()
{
	if(send_cmd_and_check_response(LOG_DATA, "AT+IPR=921600\r", "SET_BAUD_RATE", OK_RESPONSE, 200)!=SUCCESS)
	{
		uart_set_baudrate(UART_NUM_1, 115200);
		while(send_cmd_and_check_response(LOG_DATA, "AT+IPR=921600\r", "SET_BAUD_RATE", OK_RESPONSE, 200)!=SUCCESS);
		uart_set_baudrate(UART_NUM_1, 921600);
	}
	send_cmd_and_check_response(LOG_DATA, "AT&V\r", "DISPLAY_CURRENT_CONFIGURATION", OK_RESPONSE, 200);
	send_cmd_and_check_response(LOG_DATA, "ATE0\r", "TURN_OFF_ECHO_CMD", OK_RESPONSE, 200);
	send_cmd_and_check_response(LOG_DATA, CHECK_FIRMWARE_CMD, "CHECK_FIRMWARE_CMD", OK_RESPONSE, 200);
	send_cmd_and_check_response(LOG_DATA, CHECK_OPERATOR_SELECTION_CMD, "CHECK_OP_SEL_CMD", OK_RESPONSE, 200);
	send_cmd_and_check_response(LOG_DATA, CHECK_DOMAIN_REG_CMD, "CHECK_DOMAIN_REG_CMD", OK_RESPONSE, 200);
	send_cmd_and_check_response(LOG_DATA, ENABLE_SIM_HOTSWAP_CMD, "ENABLE_SIM_HOTSWAP_CMD", OK_RESPONSE, 200);
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
	if(need_to_rotate_client_index)
	{
		need_to_rotate_client_index = false;
		send_cmd_and_check_response(LOG_DATA, MQTT_NETWORK_CLOSE_CMD, "MQTT_NETWORK_CLOSE_CMD", MQTT_NETWORK_CLOSE_RESP, 500);
	}	
	if(need_to_activate_pdp)
	{
		send_cmd_and_check_response(LOG_DATA, TCP_CONFIG_CMD, "TCP_CONFIG_CMD", OK_RESPONSE, 1000);
		if(send_cmd_and_check_response(LOG_DATA, PDP_CONTXT_ACT_CMD, "PDP_CONTXT_ACT_CMD", OK_RESPONSE, 1000))
			need_to_activate_pdp = false;
		return;
	}
	if(send_cmd_and_check_response(LOG_DATA, MQTT_NETWORK_OPEN_CMD, "MQTT_NETWORK_OPEN_CMD", MQTT_NETWORK_OPEN_RESP, 500)!=SUCCESS) return;
	if(send_cmd_and_check_response(LOG_DATA, MQTT_CLIENT_CONN_CMD, "MQTT_CLIENT_CONN_CMD", MQTT_CLIENT_CONN_RESP, 500)!=SUCCESS) return;
	if(send_cmd_and_check_response(LOG_DATA, MQTT_SUB_CMD, "MQTT_SUB_CMD", MQTT_SUB_RESP, 500)!=SUCCESS) return ;
	while(1)
	{
		send_cmd_and_check_response(LOG_DATA, MQTT_READ_MSG_CMD, "MQTT_READ_MSG_CMD", OK_RESPONSE, 100);
		if (pubmesg_queue_head != NULL)
		{
			if (publish_to_mqtt() == SUCCESS) remove_from_pubmesg_queue();
		}
		if(send_cmd_and_check_response(LOG_DATA, PING_CMD, "PING_CMD", OK_RESPONSE, 100)!=SUCCESS) {
			if(ping_fail_counter++ > RETRY_COUNT){
				custom_printf(LTE_ERROR_TAG, "Lost MQTT connection", RED);
				ping_fail_counter = 0;
				mqtt_connected = false;
				break;
			}
			continue;
		}
		ping_fail_counter = 0;
		mqtt_connected = true;
	}
}

/**
 * @brief Function that takes care of configuring the MQTT communication
 * @param none
 * @retval none
 */
void MQTT_config()
{
	send_cmd_and_check_response(LOG_DATA, CLEAN_SESSION_CMD, "CLEAN_SESSION_CMD", OK_RESPONSE, 1000);
	send_cmd_and_check_response(LOG_DATA, KEEP_ALIVE_CMD, "KEEP_ALIVE_CMD", OK_RESPONSE, 3000);
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
	execute_general_AT_cmds();
	MQTT_config();
	// send_cmd_and_check_response(LOG_DATA, TURN_OFF_ECHO_CMD, "TURN_OFF_ECHO_CMD", OK_RESPONSE, 100);
	// while(!send_cmd_and_check_response(LOG_DATA, "AT+QLTS=2\r\n", "GET LOCAL TIME", "+QLTS:", 300)){
	// vTaskDelay(1);
	while(1)
	{
		vTaskDelay(1);
		establishMQTTConnection();
	}
}

/**
 * @brief Function that takes care of handling the OTA update
 * @param none
 * @retval none
 */
void ota_update()
{
    ota_in_progress = true;
    while(ota_in_progress)
    {
        //1. Start a UDP service.
        //2. Receive data from remote.
        //3. End the UDP service.
        //4. Boot from the newly downloaded firmware.
		vTaskDelay(pdMS_TO_TICKS(5));
        // ESP_LOGI(LTE_DEBUG_TAG, "OTA update in progress ... ");
        while(!send_cmd_and_check_response(LOG_DATA, START_UDP_SERVICE_CMD, "START_UDP_SERVICE", START_UDP_SERVICE_RESP, 150000)) vTaskDelay(pdMS_TO_TICKS(50));
		while(1)
		{
			send_cmd_and_check_response(LOG_DATA, CHECK_SOCKET_STATE_CMD,"CHECK_SOCKET_STATE_CMD",OK_RESPONSE,1000);
			send_cmd_and_check_response(LOG_DATA, UDP_RECV_CMD, "UDP_REC_CMD", UDP_RECV_RESP, 1000);
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}
    // esp_err_t ret = ESP_OK;
    // esp_ota_begin(const esp_partition_t *partition, size_t image_size, esp_ota_handle_t *out_handle);
    // esp_ota_write(esp_ota_handle_t handle, const void *data, size_t size);
    // esp_ota_end(esp_ota_handle_t handle);
    // esp_ota_abort(esp_ota_handle_t handle);
    // esp_ota_get_boot_partition(void);
}