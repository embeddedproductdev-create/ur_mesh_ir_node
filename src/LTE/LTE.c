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

#define SUCCESS 1
#define FAILURE 0
#define MAX_WAIT_MS  100
#define BUF_SIZE	2048

//Initialization
bool restart_flag = false;
bool network_flag = false;
bool client_flag = false;
bool subscribe_flag = false;

void LTE_setup()
{
	resetLte();
	LTE_initialization();
	establishMQTTConnection();
}

enum json_packet_enum json_packet_id = UNKNOWN_PACKET;
char json_packet[100];
cJSON *json_packet_j;
control_t ac_control_t;
gwy_reg_t gwy_registration_t;
gwy_unreg_t gwy_unregistration_t;
prov_t provision_t;
unprov_t unprovision_t;
reconf_t reconfigure_t;

void fill_macid()
{
	char macid[17];
	strcpy(macid, cJSON_GetObjectItemCaseSensitive(json_packet_j, MAC_ID_STR)->valuestring);
    char hex_char_str[2];
	for(uint8_t index=0, i=0; index<6; index++, i+=3)
	{
	    strncat(hex_char_str,&macid[i],1);
	    strncat(hex_char_str,&macid[i+1],1);
		provision_t.macid[index] = strtol(hex_char_str, NULL, 16);
		strcpy(hex_char_str, "");
		printf("macid[%d] : %x\r\n",index, provision_t.macid[index]);
	}
	printf("\r\n");
}

/**
  * @brief configure esp32 uart
  * @param None
  * @retval None
*/
void uart_init(void)
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

void sendAT_Data(const char* data)
{
    int err = uart_write_bytes(UART_NUM_1, data, strlen(data));
    if(err != -1) ;//ESP_LOGI(TAG, "AT Command sent : %s",data);
    else ESP_LOGE(TAG, "Error in sending AT command to the EC200!!!");
}

uint8_t check_response(char* response, uint32_t timeout)
{
		uint8_t index=0,j=0;
		bool copy_flag = false;
		char* data = (char*) calloc(BUF_SIZE,sizeof(char));
		uint32_t time = esp_timer_get_time()/1000ULL;
		while((esp_timer_get_time()/1000ULL) - time < timeout){
			uint32_t length = uart_read_bytes(UART_NUM_1, data, BUF_SIZE, 100);
			if(length>0){
				char check_string[30];
				sprintf(check_string, "+QMTSTAT: %d,1", CLIENT_IDX);
				if(strstr(data, check_string))
				{
					printf("Need to restart the LTE to re-establish MQTT connection\r\n");
					restart_flag = true;
					break;
				}
				if(strstr((const char* )data,(const char*)response)){
					// ESP_LOGI(TAG, "Received string : %s\n", (char *) data);
					for(index=0,j=0; data[index] != '\0'; index++)
					{
						if(data[index]=='{' && copy_flag == false)
							copy_flag = true;
						else
							continue;
						while(copy_flag)
						{
							vTaskDelay(1);
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
					free(data);
					return SUCCESS;
				}
			}
		}
		free(data);
		return FAILURE;
}


uint8_t Set_BaudRate(int baud_rate)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
		sprintf((char*)transmit_buffer,"%s%d\r\n",SET_BAUD_RATE,baud_rate);
	    sendAT_Data(transmit_buffer);
		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG, "Baud Rate Set\r\n");
			free(transmit_buffer);
			return SUCCESS;
		}
		ESP_LOGI(TAG, "Baud Rate configuration failed\r\n");
		free(transmit_buffer);
		return FAILURE;
}

uint8_t get_loacalTime()
{
	    sendAT_Data(GET_TIME);
		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			return SUCCESS;
		}
		ESP_LOGI(TAG, "Time synchronization failed\r\n");
		return FAILURE;
}

uint8_t MQTT_Config(uint8_t client_idx,
				  uint8_t enable_ssl, uint8_t SSL_ctx_idx,
				  uint16_t keep_alive,
				  uint8_t clean_session,
				  uint8_t msg_recv_mode,uint8_t msg_len_enable,
				  uint8_t will_fg, uint8_t will_qos, uint8_t will_retain, char* will_topic, char* will_message)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
//		sprintf((char*)transmit_buffer,"%s%d,%d,%d\r\n",ENABLE_SSL,client_idx,enable_ssl,SSL_ctx_idx);
//	    sendAT_Data(transmit_buffer);
//	    memset(transmit_buffer,'\0',strlen(transmit_buffer));
//		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
//			ESP_LOGI(TAG,"SSL Enabled");
//		}
		sprintf((char*)transmit_buffer,"%s4,%d\r\n",MQTT_VERSION,client_idx);
		sendAT_Data(transmit_buffer);
		memset(transmit_buffer,'\0',strlen(transmit_buffer));
		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"MQTT Version set");
		}
		sprintf((char*)transmit_buffer,"%s%d,%d\r\n",MQTT_KEEP_ALIVE,client_idx,keep_alive);
		sendAT_Data(transmit_buffer);
		memset(transmit_buffer,'\0',strlen(transmit_buffer));
		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"Keep_alive time set to %d s",keep_alive);
		}
		sprintf((char*)transmit_buffer,"%s%d,%d,%d\r\n",MQTT_RECV_MODE,client_idx,msg_recv_mode,msg_len_enable);
		sendAT_Data(transmit_buffer);
		memset(transmit_buffer,'\0',strlen(transmit_buffer));
		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"Receive mode configured");
		}
		sprintf((char*)transmit_buffer,"%s%d,%d,%d,%d,\"%s\",\"%s\"\r\n",MQTT_WILL_CONFIG,client_idx,will_fg,will_qos,will_retain,will_topic,will_message);
		sendAT_Data(transmit_buffer);
		memset(transmit_buffer,'\0',strlen(transmit_buffer));
		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"Will Parameters configured");
		}
		sprintf((char*)transmit_buffer,"%s%d,%d\r\n",CLEAN_SESSION,client_idx,clean_session);
		sendAT_Data(transmit_buffer);
		memset(transmit_buffer,'\0',strlen(transmit_buffer));
		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"Session Clean : %d",clean_session);
		}
		free(transmit_buffer);
		return SUCCESS;
}

uint8_t SubscribeTopic(int client_idx, int msgid, char* topic, int qos)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
		sprintf((char*)transmit_buffer,"%s%d,%d,\"%s\",%d\r\n",SUB_TO_TOPIC,client_idx,msgid,topic,qos);
	    sendAT_Data((char*)transmit_buffer);
	    memset(transmit_buffer,'\0',strlen(transmit_buffer));
	    sprintf((char*)transmit_buffer,"%s%d,%d,0\r\n",MQTT_SUB_RESPONSE,client_idx,msgid);
		if(check_response(MQTT_SUB_RESPONSE,150*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG, "Subscribed to topic:%s\r\n",topic);
			free(transmit_buffer);
			subscribe_flag=1;
			return SUCCESS;
		}
		client_flag = 0;
		subscribe_flag = 0;
		ESP_LOGI(TAG, "Could not Subscribe to Topic. \r\n");
		free(transmit_buffer);
		return FAILURE;
}

uint8_t UnsubscribeTopic(int client_idx, int msgid, char* topic)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
		sprintf((char*)transmit_buffer,"%s%d,%d,\"%s\"\r\n",UNSUB_TO_TOPIC,client_idx,msgid,topic);
	    sendAT_Data((char*)transmit_buffer);
	    memset(transmit_buffer,'\0',strlen(transmit_buffer));
	   	sprintf((char*)transmit_buffer,"%s%d,%d,0\r\n",MQTT_UNSUB_RESPONSE,client_idx,msgid);
		if(check_response(OK_RESPONSE,150*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG, "Unsubscribed from topic:%s\r\n",topic);
			free(transmit_buffer);
			subscribe_flag=0;
			return SUCCESS;
		}
		ESP_LOGI(TAG, "Could not Unsubscribe from Topic. \r\n");
		free(transmit_buffer);
		return FAILURE;
}

int MQTT_NetworkOpen(int client_idx, char* hostname, uint32_t port)
{
	char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
	sprintf((char*)transmit_buffer,"%s%d,\"%s\",%ld\r\n",MQTT_NETWORK_OPEN,client_idx,hostname,port);
	sendAT_Data((char*)transmit_buffer);
	memset(transmit_buffer,'\0',strlen(transmit_buffer));
	sprintf((char*)transmit_buffer,"%s%d,0\r\n",MQTT_NETWORK_OPEN_RESPONSE,client_idx);
	if(check_response(OK_RESPONSE,10*MAX_WAIT_MS)	==	SUCCESS ){
		ESP_LOGI(TAG, "Connected to network at:%s\r\n",hostname);
		free(transmit_buffer);
		network_flag = 1;
		return SUCCESS;
	}
	else{
		memset(transmit_buffer,'\0',strlen(transmit_buffer));
		sprintf((char*)transmit_buffer,"%s%d,2\r\n",MQTT_NETWORK_OPEN_RESPONSE,client_idx);
		if(check_response(transmit_buffer,10*MAX_WAIT_MS)	==	SUCCESS ){
			return 2;
		}
		network_flag = 0;
	}
	ESP_LOGI(TAG, "Could not Connect to network. \r\n");
	free(transmit_buffer);
	return FAILURE;
}

uint8_t MQTT_NetworkClose(int client_idx)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
		sprintf((char*)transmit_buffer,"%s%d\r\n",MQTT_NETWORK_CLOSE,client_idx);
	    sendAT_Data((char*)transmit_buffer);
	    memset(transmit_buffer,'\0',strlen(transmit_buffer));
	    sprintf((char*)transmit_buffer,"%s%d,0\r\n",MQTT_NETWORK_CLOSE_RESPONSE,client_idx);
		if(check_response(transmit_buffer,300*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG, "Closed MQTT network");
			free(transmit_buffer);
			network_flag = 0;
			return SUCCESS;
		}
		ESP_LOGI(TAG, "Could not close MQTT network. \r\n");
		free(transmit_buffer);
		return FAILURE;
}

uint8_t MQTT_ClientConnect(int client_idx, char* username, char* passwd, char* clientID)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
		sprintf((char*)transmit_buffer,"%s%d,\"%s\",\"%s\",\"%s\"\r\n",MQTT_CLIENT_CONN,client_idx,clientID,username,passwd);
		sendAT_Data((char*)transmit_buffer);
	    memset(transmit_buffer,'\0',strlen(transmit_buffer));
	    sprintf((char*)transmit_buffer,"%s%d,0,0\r\n",MQTT_CLIENT_CONN_RESPONSE,client_idx);
		if(check_response(transmit_buffer,2*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"Connected client to broker: %s\r\n",username);
			free(transmit_buffer);
			client_flag = 1;
			return SUCCESS;
		}
		client_flag = 0;
		ESP_LOGI(TAG,"Could not Connect client to broker.\r\n");
		free(transmit_buffer);
		return FAILURE;
}

uint8_t MQTT_ClientDisconnect(int client_idx)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
		sprintf((char*)transmit_buffer,"%s%d\r\n",MQTT_CLIENT_DISCONN,client_idx);
	    sendAT_Data((char*)transmit_buffer);
	    memset(transmit_buffer,'\0',strlen(transmit_buffer));
	    sprintf((char*)transmit_buffer,"%s%d,0",MQTT_CLIENT_DISCONN_RESPONSE,client_idx);
		if(check_response(OK_RESPONSE,2*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"Disconnected client from broker");
			free(transmit_buffer);
			client_flag = 0;
			return SUCCESS;
		}
		ESP_LOGI(TAG,"Could not Disconnect client from broker.\r\n");
		free(transmit_buffer);
		return FAILURE;
}

uint8_t PublishMessage(uint8_t client_idx, uint32_t msgid, uint8_t qos, uint8_t retain, char* topic)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
		char *msg = "Test";
		sprintf((char*)transmit_buffer,"%s%d,%ld,%d,%d,\"%s\",%d\r\n",PUB_MSG,client_idx,msgid,qos,retain,topic,strlen(msg));
	    sendAT_Data((char*)transmit_buffer);
	    memset(transmit_buffer,'\0',strlen(transmit_buffer));
	    sprintf((char*)transmit_buffer,"%s%d,%ld,0\r\n",MQTT_PUB_MSG_RESPONSE,client_idx,msgid);
		if(check_response(PROMPT,150*MAX_WAIT_MS)	==	SUCCESS ){
			uart_write_bytes(UART_NUM_1,msg,strlen(msg));
			if(check_response(transmit_buffer,150*MAX_WAIT_MS)	==	SUCCESS ){
				ESP_LOGI(TAG,"Published message:%s\r\n",msg);
				free(transmit_buffer);
				return SUCCESS;
			}
		}
		ESP_LOGI(TAG,"Could not publish message.\r\n");
		free(transmit_buffer);
		return FAILURE;
}

uint8_t ReadMessage(int client_idx)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
		sprintf((char*)transmit_buffer,"%s%d\r\n",READ_MSG_BUFFER ,client_idx);
	    sendAT_Data((char*)transmit_buffer);
		if(check_response(OK_RESPONSE,2*MAX_WAIT_MS) == SUCCESS ){
			free(transmit_buffer);
			return SUCCESS;
		}
		ESP_LOGI(TAG,"Could not receive message.\r\n");
		free(transmit_buffer);
		return FAILURE;
}

uint8_t SSL_config(uint8_t ssl_context_index, char* ca_cert, char* client_cert, char* client_key)
{
	char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));

	sprintf((char*)transmit_buffer,"%s\"UFS:ca.pem\",1464,100,0\r\n",FILE_UPLOAD);
    sendAT_Data(transmit_buffer);
    //Error_Report();
    memset(transmit_buffer,'\0',strlen(transmit_buffer));
	if(check_response(CONNECT_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
		uart_write_bytes(UART_NUM_1,ca_cert,strlen(ca_cert));
		if(check_response(FILE_UPLOAD_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"CA cerificate sent");
		}
	}

	sprintf((char*)transmit_buffer,"%s\"UFS:client.pem\",1269,100,0\r\n",FILE_UPLOAD);
    sendAT_Data(transmit_buffer);
    //Error_Report();
    memset(transmit_buffer,'\0',strlen(transmit_buffer));
	if(check_response(CONNECT_RESPONSE,50*MAX_WAIT_MS)	==	SUCCESS ){
		uart_write_bytes(UART_NUM_1,client_cert,strlen(client_cert));
		if(check_response(FILE_UPLOAD_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"CC cerificate sent");
		}
	}

	sprintf((char*)transmit_buffer,"%s\"UFS:client_key.pem\",1679,100,0\r\n",FILE_UPLOAD);
    sendAT_Data(transmit_buffer);
    //Error_Report();
    memset(transmit_buffer,'\0',strlen(transmit_buffer));
	if(check_response(CONNECT_RESPONSE,50*MAX_WAIT_MS)	==	SUCCESS ){
		uart_write_bytes(UART_NUM_1,client_key,strlen(client_key));
		if(check_response(FILE_UPLOAD_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"CK cerificate sent");
		}
	}

	sprintf((char*)transmit_buffer,"%s%d,\"UFS:ca.pem\"\r\n",SSL_CONFIG_CACERT,ssl_context_index);
	sendAT_Data(transmit_buffer);
	memset(transmit_buffer,'\0',strlen(transmit_buffer));
	if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
		ESP_LOGI(TAG,"CA certificate configured");
	}

	sprintf((char*)transmit_buffer,"%s%d,\"UFS:client.pem\"\r\n",SSL_CONFIG_CCCERT,ssl_context_index);
	sendAT_Data(transmit_buffer);
	memset(transmit_buffer,'\0',strlen(transmit_buffer));
	if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
		ESP_LOGI(TAG,"CC certificate configured");
	}

	sprintf((char*)transmit_buffer,"%s%d,\"UFS:client_key.pem\"\r\n",SSL_CONFIG_CLIKEY,ssl_context_index);
	sendAT_Data(transmit_buffer);
	memset(transmit_buffer,'\0',strlen(transmit_buffer));
	if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
		ESP_LOGI(TAG,"CK certificate configured");
	}

	sprintf((char*)transmit_buffer,"%s%d,2\r\n",SSL_CONFIG_AUTHMODE,ssl_context_index);
	sendAT_Data(transmit_buffer);
	memset(transmit_buffer,'\0',strlen(transmit_buffer));
	if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
		ESP_LOGI(TAG,"Server authentication mode");
	}

	sprintf((char*)transmit_buffer,"%s%d,4\r\n",SSL_CONFIG_AUTHVER,ssl_context_index);
	sendAT_Data(transmit_buffer);
	memset(transmit_buffer,'\0',strlen(transmit_buffer));
	if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
		ESP_LOGI(TAG,"SSL authentication version");
	}

	sprintf((char*)transmit_buffer,"%s%d,0xFFFF\r\n",SSL_CONFIG_CIPHERSUITE,ssl_context_index);
	sendAT_Data(transmit_buffer);
	memset(transmit_buffer,'\0',strlen(transmit_buffer));
	if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
		ESP_LOGI(TAG,"Cipher suite set");
	}

	sprintf((char*)transmit_buffer,"%s%d,1\r\n",SSL_CONFIG_AUTHTIME,ssl_context_index);
	sendAT_Data(transmit_buffer);
	memset(transmit_buffer,'\0',strlen(transmit_buffer));
	if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
		ESP_LOGI(TAG,"Authentication time ignored");
	}


	free(transmit_buffer);
	return SUCCESS;
}

uint8_t List_all_files(){
	sendAT_Data(FILE_LIST);
	if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)){
		ESP_LOGI(TAG,"All files listed");
		return SUCCESS;
	}
	return FAILURE;
}
uint8_t Delete_file(char* filename){
	char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
	sprintf((char*)transmit_buffer,"%s\"%s\"\r\n",FILE_DELETE,filename);
	sendAT_Data(transmit_buffer);
	if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)){
		ESP_LOGI(TAG,"File Deleted");
		return SUCCESS;
	}
	return FAILURE;
}

uint8_t Error_Report(){
	sendAT_Data(ERROR_REPORT);
	if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)){
		return SUCCESS;
	}
	return FAILURE;
}

uint8_t OT_command(char* cmd){
	sendAT_Data(cmd);
	if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)){
		return SUCCESS;
	}
	return FAILURE;
}



static void timer_callback(void* arg)
{
}

void timer_config()
{
	const esp_timer_create_args_t timer_args = {
	            .callback = &timer_callback,
	            .name = "timer"
	    };

	    esp_timer_handle_t periodic_timer;
	    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &periodic_timer));
	    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 500000));
}

void resetLte()
{
	ESP_LOGI(TAG, "Resetting LTE !!!");
	gpio_set_level(GPIO_LTE_ONOFF, 0);
	gpio_set_level(GPIO_LTE_RESET, 0);
	vTaskDelay(10); //100ms delay
	gpio_set_level(GPIO_LTE_ONOFF, 1);
	vTaskDelay(pdMS_TO_TICKS(2500)); //2.5s delay
	gpio_set_level(GPIO_LTE_ONOFF, 0);
	ESP_LOGI(TAG, "Resetting LTE complete");
}

void LTE_gpio_configuration()
{
	pinMode(GPIO_LTE_RESET, OUTPUT);
	pinMode(GPIO_LTE_ONOFF, OUTPUT);
}

void LTE_initialization(void)
{
    uart_init();
    timer_config();
    OT_command("ATE0\r\n");
    MQTT_Config(CLIENT_IDX,
    		1,2,
			10,
			1,
			0,1,
			1,0,0,"will/topic","Network Disconnected unexpectedly");
}


void establishMQTTConnection()
{
	uint8_t network_connect_retry_count = 0;
	uint8_t client_connect_retry_count = 0;
	uint8_t subscribe_retry_count = 0;
	network_flag = false;
	client_flag = false;
	subscribe_flag = false;
	while(network_connect_retry_count < RETRY_COUNT && !network_flag){
		vTaskDelay(1);
		client_connect_retry_count = 0;
		printf("NETWORK_CONNECT_RETRY_COUNT : %d\n",network_connect_retry_count++);
		uint8_t ret_val = MQTT_NetworkOpen(CLIENT_IDX,"54.215.188.103",1883);
		if(ret_val == 2) MQTT_NetworkClose(CLIENT_IDX);
		if(!network_flag) continue;
		if(ret_val == SUCCESS)
		{
			while(client_connect_retry_count < RETRY_COUNT && !client_flag) {
				subscribe_retry_count = 0;
				printf("CLIENT_CONNECT_RETRY_COUNT : %d\n",client_connect_retry_count++);
				if(MQTT_ClientConnect(CLIENT_IDX,"QmaxSystems","Qmax_mosquitto_!@#","AC_IR_CONTROL") == SUCCESS)
				{
					while(subscribe_retry_count < RETRY_COUNT && !subscribe_flag) {
						printf("SUBSCRIBE_RETRY_COUNT : %d\n",subscribe_retry_count++);
						SubscribeTopic(CLIENT_IDX,2,"Control_packet", 0);
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
				MQTT_NetworkClose(CLIENT_IDX);
				network_connect_retry_count = 0;
			}
		}
	}
}

/**
 * @brief parses the control packet recvd from MQTT and stores it in the control strucutre
 * @param None
 * @retval None
 */
void parse_json_packet()
{
	json_packet_j = cJSON_Parse(json_packet);
	if(json_packet_j != NULL)
	{
		json_packet_id = cJSON_GetObjectItemCaseSensitive(json_packet_j, JSON_PACKET_ID)->valueint;
		printf("Json_packet_id : %d\r\n",json_packet_id);
	// 	switch(json_packet_id)
	// 	{
	// 		case GWY_REG_PACKET:
	// 			printf("Gwy registration packet received\r\n");
	// 			if(json_packet_j != NULL)
	// 			{
	// 				gwy_registration_t.msg_seq_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, MSGSEQNO_STR)->valueint;
	// 				gwy_registration_t.gwy_ser_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, GWYSERNO_STR)->valueint;
	// 				strcpy(gwy_registration_t.location, cJSON_GetObjectItemCaseSensitive(json_packet_j, LOCATION_STR)->valuestring);
	// 			}
    //         	break;

	// 		case GWY_CONF_PACKET:
	// 			break;

	// 		case GWY_UNREG_PACKET:
	// 			printf("Gwy unregistration packet received\r\n");
	// 			if(json_packet_j != NULL)
	// 			{
	// 				gwy_unregistration_t.msg_seq_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, MSGSEQNO_STR)->valueint;
	// 				gwy_unregistration_t.gwy_ser_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, GWYSERNO_STR)->valueint;
	// 				strcpy(gwy_unregistration_t.location, cJSON_GetObjectItemCaseSensitive(json_packet_j, LOCATION_STR)->valuestring);
	// 			}
	// 			break;

	// 		case GWY_AC_CONTROL_PACKET:
	// 			break;

	// 		case GWY_AC_LOCKING_PACKET:
	// 			break;

	// 		case GWY_RECONF_PACKET:
	// 			break;

	// 		case NODE_PROV_PACKET:
	// 			printf("Node Provisioning packet received\r\n");
	// 			if(json_packet_j != NULL)
	// 			{
	// 				provision_t.msg_seq_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, MSGSEQNO_STR)->valueint;
	// 				provision_t.gwy_ser_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, GWYSERNO_STR)->valueint;
	// 				provision_t.node_ser_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, NODESERNO_STR)->valueint;
	// 				fill_macid();
	// 			}

	// 			break;

	// 		case NODE_CONF_PACKET:
	// 			break;

	// 		case NODE_UNPROV_PACKET:
	// 			printf("Node Unprovisioning packet received\r\n");
	// 			if(json_packet_j != NULL)
	// 			{
	// 				unprovision_t.msg_seq_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, MSGSEQNO_STR)->valueint;
	// 				unprovision_t.gwy_ser_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, GWYSERNO_STR)->valueint;
	// 				unprovision_t.node_ser_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, NODESERNO_STR)->valueint;
	// 				unprovision_t.elemnt_addr = cJSON_GetObjectItemCaseSensitive(json_packet_j, ELMNT_ADDR_STR)->valueint;
	// 			}
	// 			break;

	// 		case NODE_AC_CONTROL_PACKET:
	// 			printf("Node AC Control packet received\r\n");
	// 			if(json_packet_j != NULL)
	// 			{
	// 				ac_control_t.msg_seq_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, MSGSEQNO_STR)->valueint;
	// 				ac_control_t.gwy_ser_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, GWYSERNO_STR)->valueint;
	// 				ac_control_t.node_ser_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, NODESERNO_STR)->valueint;
	// 				ac_control_t.elementAddr = cJSON_GetObjectItemCaseSensitive(json_packet_j, ELMNT_ADDR_STR)->valueint;
	// 				ac_control_t.power = cJSON_GetObjectItemCaseSensitive(json_packet_j, POWER_STR)->valueint;
	// 				strcpy(ac_control_t.mode_str, cJSON_GetObjectItemCaseSensitive(json_packet_j, MODE_STR)->valuestring);
	// 				ac_control_t.fan = cJSON_GetObjectItemCaseSensitive(json_packet_j, FAN_STR)->valueint;
	// 				ac_control_t.temp = cJSON_GetObjectItemCaseSensitive(json_packet_j, TEMP_STR)->valueint;
	// 				ac_control_t.swingH = cJSON_GetObjectItemCaseSensitive(json_packet_j, SWING_H_STR)->valueint;
	// 				ac_control_t.swingV = cJSON_GetObjectItemCaseSensitive(json_packet_j, SWING_V_STR)->valueint;
	// 				ac_control_t.OnTimer = cJSON_GetObjectItemCaseSensitive(json_packet_j, ONTIMER_STR)->valueint;
	// 				ac_control_t.OffTimer = cJSON_GetObjectItemCaseSensitive(json_packet_j, OFFTIMER_STR)->valueint;
	// 				ac_control_t.Locking = cJSON_GetObjectItemCaseSensitive(json_packet_j, LOCKING_STR)->valueint;
	// 			}
	// 			break;

	// 		case NODE_AC_LOCKING_PACKET:
	// 			break;

	// 		case NODE_RECONF_PACKET:
	// 			printf("Node Reconfigure packet received\r\n");
	// 			if(json_packet_j != NULL)
	// 			{
	// 				reconfigure_t.msg_seq_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, MSGSEQNO_STR)->valueint;
	// 				reconfigure_t.gwy_ser_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, GWYSERNO_STR)->valueint;
	// 				reconfigure_t.node_ser_no = cJSON_GetObjectItemCaseSensitive(json_packet_j, NODESERNO_STR)->valueint;
	// 				reconfigure_t.elementAddr = cJSON_GetObjectItemCaseSensitive(json_packet_j, ELMNT_ADDR_STR)->valueint;
	// 			}
	// 			break;

	// 		default:
	// 			printf("UNKNOWN MQTT PACKET ERROR\r\n");
	// 	}
	}
	else
		printf("json_packet_j is NULL\r\n");
}

void *LTE_task(void *args)
{
    LTE_gpio_configuration();
    resetLte();
    LTE_initialization();
    establishMQTTConnection();
    while(1)
    {
		if(!restart_flag)
		{
			printf("Listening for MQTT request ...\r\n");
			vTaskDelay(pdMS_TO_TICKS(200));
			ReadMessage(CLIENT_IDX);
			if(strlen(json_packet) > 20)
			{
				parse_json_packet();
				strcpy(json_packet, "");
			}
		}
		else
		{
			LTE_setup();
			restart_flag = false;
		}
    }
}

