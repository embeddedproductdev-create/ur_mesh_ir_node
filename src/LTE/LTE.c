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
#include "../../inc/LTE/mqtt.h"

#define SUCCESS 0
#define FAILURE -1
#define MAX_WAIT_MS  100
#define BUF_SIZE	2048

//Initialization
bool restart_flag = false;
bool network_flag = false;
bool client_flag = false;
bool subscribe_flag = false;
bool mqtt_connected = false;
bool registered  = false;
bool publishing_flag = false;

//sensor model send data when this variable set
bool send_control_packet = false;

int16_t json_ack_err_code = SUCCESS;
uint8_t json_packet_id = UNKNOWN_PACKET;
char json_packet[MQTT_PACKET_BUFF_SIZE];
cJSON *json_packet_j;

control_t gwy_ac_control_t;
control_t node_ac_control_t;
gwy_reg_t gwy_registration_t;
gwy_unreg_t gwy_unregistration_t;
prov_t provision_t;
unprov_t unprovision_t;
reconf_t gwy_reconfigure_t;
reconf_t node_reconfigure_t;
mqtt_reset_t gwy_reset_mqtt_t;

struct pub_mesg_struct *pubmesg_head_ptr = NULL;
struct pub_mesg_struct *pubmesg_tail_ptr = NULL;

char subscribe_topic[MQTT_TOPIC_CHAR_LEN];
char publish_topic[MQTT_TOPIC_CHAR_LEN];

uint8_t mqtt_qos = 2; //0 = atmost once | 1 = atleast once | 2 = exactly once
uint8_t mqtt_retain = 0;
uint8_t mqtt_msgid = 2;

void LTE_setup()
{
	resetLte();
	LTE_initialization();
	establishMQTTConnection();
}

void fill_macid()
{
	char macid[17];
	strcpy(macid, cJSON_GetObjectItem(json_packet_j, MAC_ID_STR)->valuestring);
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
				// ESP_LOGI(TAG, "Received string : %s\n", (char *) data);
				char check_string[30];
				sprintf(check_string, "+QMTSTAT: %d,1", mqtt_client_index);
				if(strstr(data, check_string))
				{
					printf("Need to restart the LTE to re-establish MQTT connection\r\n");
					restart_flag = true;
					break;
				}
				if(strstr((const char* )data,(const char*)response)){
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

uint8_t MQTT_Config(uint8_t mqtt_client_index,
				  uint8_t enable_ssl, uint8_t SSL_ctx_idx,
				  uint16_t keep_alive,
				  uint8_t clean_session,
				  uint8_t msg_recv_mode,uint8_t msg_len_enable,
				  uint8_t will_fg, uint8_t will_qos, uint8_t will_retain, char* will_topic, char* will_message)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
//		sprintf((char*)transmit_buffer,"%s%d,%d,%d\r\n",ENABLE_SSL,mqtt_client_index,enable_ssl,SSL_ctx_idx);
//	    sendAT_Data(transmit_buffer);
//	    memset(transmit_buffer,'\0',strlen(transmit_buffer));
//		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
//			ESP_LOGI(TAG,"SSL Enabled");
//		}
		sprintf((char*)transmit_buffer,"%s4,%d\r\n",MQTT_VERSION,mqtt_client_index);
		sendAT_Data(transmit_buffer);
		memset(transmit_buffer,'\0',strlen(transmit_buffer));
		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"MQTT Version set");
		}
		sprintf((char*)transmit_buffer,"%s%d,%d\r\n",MQTT_KEEP_ALIVE,mqtt_client_index,keep_alive);
		sendAT_Data(transmit_buffer);
		memset(transmit_buffer,'\0',strlen(transmit_buffer));
		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"Keep_alive time set to %d s",keep_alive);
		}
		sprintf((char*)transmit_buffer,"%s%d,%d,%d\r\n",MQTT_RECV_MODE,mqtt_client_index,msg_recv_mode,msg_len_enable);
		sendAT_Data(transmit_buffer);
		memset(transmit_buffer,'\0',strlen(transmit_buffer));
		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"Receive mode configured");
		}
		sprintf((char*)transmit_buffer,"%s%d,%d,%d,%d,\"%s\",\"%s\"\r\n",MQTT_WILL_CONFIG,mqtt_client_index,will_fg,will_qos,will_retain,will_topic,will_message);
		sendAT_Data(transmit_buffer);
		memset(transmit_buffer,'\0',strlen(transmit_buffer));
		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"Will Parameters configured");
		}
		sprintf((char*)transmit_buffer,"%s%d,%d\r\n",CLEAN_SESSION,mqtt_client_index,clean_session);
		sendAT_Data(transmit_buffer);
		memset(transmit_buffer,'\0',strlen(transmit_buffer));
		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS)	==	SUCCESS ){
			ESP_LOGI(TAG,"Session Clean : %d",clean_session);
		}
		free(transmit_buffer);
		return SUCCESS;
}

uint8_t SubscribeTopic(int mqtt_client_index, int msgid, char* topic, int qos)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
		sprintf((char*)transmit_buffer,"%s%d,%d,\"%s\",%d\r\n",SUB_TO_TOPIC,mqtt_client_index,msgid,topic,qos);
	    sendAT_Data((char*)transmit_buffer);
	    memset(transmit_buffer,'\0',strlen(transmit_buffer));
	    sprintf((char*)transmit_buffer,"%s%d,%d,0\r\n",MQTT_SUB_RESPONSE,mqtt_client_index,msgid);
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

uint8_t UnsubscribeTopic(int mqtt_client_index, int msgid, char* topic)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
		sprintf((char*)transmit_buffer,"%s%d,%d,\"%s\"\r\n",UNSUB_TO_TOPIC,mqtt_client_index,msgid,topic);
	    sendAT_Data((char*)transmit_buffer);
	    memset(transmit_buffer,'\0',strlen(transmit_buffer));
	   	sprintf((char*)transmit_buffer,"%s%d,%d,0\r\n",MQTT_UNSUB_RESPONSE,mqtt_client_index,msgid);
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

int MQTT_NetworkOpen(int mqtt_client_index, char* hostname, uint32_t port)
{
	char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
	sprintf((char*)transmit_buffer,"%s%d,\"%s\",%ld\r\n",MQTT_NETWORK_OPEN,mqtt_client_index,hostname,port);
	sendAT_Data((char*)transmit_buffer);
	memset(transmit_buffer,'\0',strlen(transmit_buffer));
	sprintf((char*)transmit_buffer,"%s%d,0\r\n",MQTT_NETWORK_OPEN_RESPONSE,mqtt_client_index);
	if(check_response(OK_RESPONSE,100)	==	SUCCESS ){
		ESP_LOGI(TAG, "Connected to network at:%s\r\n",hostname);
		free(transmit_buffer);
		network_flag = 1;
		return SUCCESS;
	}
	else{
		memset(transmit_buffer,'\0',strlen(transmit_buffer));
		sprintf((char*)transmit_buffer,"%s%d,2\r\n",MQTT_NETWORK_OPEN_RESPONSE,mqtt_client_index);
		if(check_response(transmit_buffer,100)	==	SUCCESS ){
			return 2;
		}
		network_flag = 0;
	}
	ESP_LOGI(TAG, "Could not Connect to network. \r\n");
	free(transmit_buffer);
	return FAILURE;
}

uint8_t MQTT_NetworkClose(int mqtt_client_index)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
		sprintf((char*)transmit_buffer,"%s%d\r\n",MQTT_NETWORK_CLOSE,mqtt_client_index);
	    sendAT_Data((char*)transmit_buffer);
	    memset(transmit_buffer,'\0',strlen(transmit_buffer));
	    sprintf((char*)transmit_buffer,"%s%d,0\r\n",MQTT_NETWORK_CLOSE_RESPONSE,mqtt_client_index);
		if(check_response(transmit_buffer,300) ==	SUCCESS ){
			ESP_LOGI(TAG, "Closed MQTT network");
			free(transmit_buffer);
			network_flag = 0;
			return SUCCESS;
		}
		ESP_LOGI(TAG, "Could not close MQTT network. \r\n");
		free(transmit_buffer);
		return FAILURE;
}

uint8_t MQTT_ClientConnect(int mqtt_client_index, char* username, char* passwd, char* clientID)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
		sprintf((char*)transmit_buffer,"%s%d,\"%s\",\"%s\",\"%s\"\r\n",MQTT_CLIENT_CONN,mqtt_client_index,clientID,username,passwd);
		sendAT_Data((char*)transmit_buffer);
	    memset(transmit_buffer,'\0',strlen(transmit_buffer));
	    sprintf((char*)transmit_buffer,"%s%d,0,0\r\n",MQTT_CLIENT_CONN_RESPONSE,mqtt_client_index);
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

uint8_t MQTT_ClientDisconnect(int mqtt_client_index)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
		sprintf((char*)transmit_buffer,"%s%d\r\n",MQTT_CLIENT_DISCONN,mqtt_client_index);
	    sendAT_Data((char*)transmit_buffer);
	    memset(transmit_buffer,'\0',strlen(transmit_buffer));
	    sprintf((char*)transmit_buffer,"%s%d,0",MQTT_CLIENT_DISCONN_RESPONSE,mqtt_client_index);
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

uint8_t PublishMessage(uint8_t mqtt_client_index, uint32_t msgid, uint8_t qos, uint8_t retain, char* topic, char* message)
{
	char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
	sprintf((char*)transmit_buffer,"%s%d,%ld,%d,%d,\"%s\",%d\r\n",PUB_MSG,mqtt_client_index,msgid,qos,retain,topic,strlen(message));
	sendAT_Data((char*)transmit_buffer);
	memset(transmit_buffer,'\0',strlen(transmit_buffer));
	sprintf((char*)transmit_buffer,"%s%d,%ld,0\r\n",MQTT_PUB_MSG_RESPONSE,mqtt_client_index,msgid);
	if(check_response(PROMPT,150)==SUCCESS ){
		uart_write_bytes(UART_NUM_1,message,strlen(message));
		if(check_response(transmit_buffer,1500)	==	SUCCESS ){
			free(transmit_buffer);
			return SUCCESS;
		}
	}
	free(transmit_buffer);
	return FAILURE;
}

uint8_t ReadMessage(int mqtt_client_index)
{
		char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
		sprintf((char*)transmit_buffer,"%s%d\r\n",READ_MSG_BUFFER ,mqtt_client_index);
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
    MQTT_Config(mqtt_client_index,
    		1,2,
			10,
			1,
			0,1,
			1,0,0,"will/topic","Network Disconnected unexpectedly");
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
	while(network_connect_retry_count < RETRY_COUNT && !network_flag)
	{
		vTaskDelay(1);
		client_connect_retry_count = 0;
		printf("NETWORK_CONNECT_RETRY_COUNT : %d\n",network_connect_retry_count++);
		uint8_t ret_val = MQTT_NetworkOpen(mqtt_client_index, mqtt_ip_address, mqtt_port);
		if(ret_val == 2) MQTT_NetworkClose(mqtt_client_index);
		if(network_flag)
		{
			while(client_connect_retry_count < RETRY_COUNT && !client_flag) {
				subscribe_retry_count = 0;
				printf("CLIENT_CONNECT_RETRY_COUNT : %d\n",client_connect_retry_count++);
				if(MQTT_ClientConnect(mqtt_client_index, mqtt_broker_username, mqtt_broker_password, mqtt_broker_tabname) == SUCCESS)
				{
					while(subscribe_retry_count < RETRY_COUNT && !subscribe_flag) {
						printf("SUBSCRIBE_RETRY_COUNT : %d\n",subscribe_retry_count++);

						if(SubscribeTopic(mqtt_client_index,2,subscribe_topic, 0)==SUCCESS)
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

/**
 * @brief This function verifies the data integrity of the json packet received
 * before trying to parse it
 * @param none
 * @retval none
 */
void error_check_json(uint8_t json_packet_id)
{
	//Check params common in all packets first
	if(cJSON_GetObjectItem(json_packet_j, MSGSEQNO_STR));
	else { json_ack_err_code = INVALID_MSG_SEQ_NO; return; }
	if(cJSON_GetObjectItem(json_packet_j, GWYSERNO_STR));
	else { json_ack_err_code = INVALID_GWY_SER_NO; return; }

	switch(json_packet_id)
	{
		case GWY_REG_PACKET:
		case GWY_UNREG_PACKET:
			if(cJSON_GetObjectItem(json_packet_j, LOCATION_STR));
			else { json_ack_err_code = INVALID_LOCATION_STR; return; }
			return;

		case GWY_AC_CONTROL_PACKET:
			if(cJSON_GetObjectItem(json_packet_j, POWER_STR));
			else { json_ack_err_code = INVALID_POWER_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, MODE_STR));
			else { json_ack_err_code = INVALID_MODE_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, FAN_STR));
			else { json_ack_err_code = INVALID_FAN_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, TEMP_STR));
			else { json_ack_err_code = INVALID_TEMP_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, SWING_H_STR));
			else { json_ack_err_code = INVALID_SWING_H_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, SWING_V_STR));
			else { json_ack_err_code = INVALID_SWING_V_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, ONTIMER_STR));
			else { json_ack_err_code = INVALID_ONTIMER_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, OFFTIMER_STR));
			else { json_ack_err_code = INVALID_OFFTIMER_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, LOCKING_STR));
			else { json_ack_err_code = INVALID_LOCKING_STR; return; }
			return;

		case NODE_PROV_PACKET:
			if(cJSON_GetObjectItem(json_packet_j, NODESERNO_STR));
			else { json_ack_err_code = INVALID_NODESERNO_STR; return; }
			return;

		case NODE_UNPROV_PACKET:
		case NODE_RECONF_PACKET:
			if(cJSON_GetObjectItem(json_packet_j, NODESERNO_STR));
			else { json_ack_err_code = INVALID_NODESERNO_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, ELMNT_ADDR_STR));
			else { json_ack_err_code = INVALID_ELMNT_ADDR_STR; return; }
			return;

		case NODE_AC_CONTROL_PACKET:
			if(cJSON_GetObjectItem(json_packet_j, NODESERNO_STR));
			else { json_ack_err_code = INVALID_NODESERNO_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, ELMNT_ADDR_STR));
			else { json_ack_err_code = INVALID_ELMNT_ADDR_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, POWER_STR));
			else { json_ack_err_code = INVALID_POWER_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, MODE_STR));
			else { json_ack_err_code = INVALID_MODE_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, FAN_STR));
			else { json_ack_err_code = INVALID_FAN_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, TEMP_STR));
			else { json_ack_err_code = INVALID_TEMP_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, SWING_H_STR));
			else { json_ack_err_code = INVALID_SWING_H_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, SWING_V_STR));
			else { json_ack_err_code = INVALID_SWING_V_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, ONTIMER_STR));
			else { json_ack_err_code = INVALID_ONTIMER_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, OFFTIMER_STR));
			else { json_ack_err_code = INVALID_OFFTIMER_STR; return; }
			if(cJSON_GetObjectItem(json_packet_j, LOCKING_STR));
			else { json_ack_err_code = INVALID_LOCKING_STR; return; }
			return;

		case RESET_MQTT:
			return;
	}
}

/**
 * @brief parses the control packet recvd from MQTT and stores it in the control strucutre
 * @param None
 * @retval Error code
 */
int16_t parse_json_packet()
{
	/**
	 * First get the json packet
	 * convert it to parseable obj using the CJSON_parse function
	 * Check if that object is null or not, if not null then continue
	 * try to parse json_packet_id from it. if not null and valid, continue
	 * then error check other items based on the parsed json_packet_id, if no error, then continue
	 * No matter what the error code, ack needs to be sent back with details
	 */
	json_ack_err_code = SUCCESS;
	json_packet_j = cJSON_Parse(json_packet);
	if((json_packet_j != NULL) && cJSON_GetObjectItem(json_packet_j, JSON_PACKET_ID))
	{
		json_packet_id = cJSON_GetObjectItem(json_packet_j, JSON_PACKET_ID)->valueint;
	}
	else
	{
		json_ack_err_code = INVALID_JSON_PACKET_ID;
		return json_ack_err_code;
	}

	char pubmessage[PUBMESG_LEN];

	error_check_json(json_packet_id);
	if(json_ack_err_code == SUCCESS)
	{
		switch(json_packet_id)
		{
			case GWY_REG_PACKET:
				ESP_LOGI(DEBUG_TAG, "Gwy Registration packet\r\n");
				gwy_registration_t.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSGSEQNO_STR)->valueint;
				gwy_registration_t.gwy_ser_no = cJSON_GetObjectItem(json_packet_j, GWYSERNO_STR)->valueint;
				strcpy(gwy_registration_t.location, cJSON_GetObjectItem(json_packet_j, LOCATION_STR)->valuestring);
				registered = true;
				break;

			case GWY_UNREG_PACKET:
				ESP_LOGI(DEBUG_TAG, "Gwy Unregistration packet\r\n");
				gwy_unregistration_t.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSGSEQNO_STR)->valueint;
				gwy_unregistration_t.gwy_ser_no = cJSON_GetObjectItem(json_packet_j, GWYSERNO_STR)->valueint;
				strcpy(gwy_unregistration_t.location, cJSON_GetObjectItem(json_packet_j, LOCATION_STR)->valuestring);
				registered = false;
				configured = false;
				break;

			case GWY_AC_CONTROL_PACKET:
				ESP_LOGI(DEBUG_TAG, "Gwy AC Control packet\r\n");
				gwy_ac_control_t.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSGSEQNO_STR)->valueint;
				gwy_ac_control_t.gwy_ser_no = cJSON_GetObjectItem(json_packet_j, GWYSERNO_STR)->valueint;
				gwy_ac_control_t.power = cJSON_GetObjectItem(json_packet_j, POWER_STR)->valueint;
				strcpy(gwy_ac_control_t.mode_str, cJSON_GetObjectItem(json_packet_j, MODE_STR)->valuestring);
				gwy_ac_control_t.fan = cJSON_GetObjectItem(json_packet_j, FAN_STR)->valueint;
				gwy_ac_control_t.temp = cJSON_GetObjectItem(json_packet_j, TEMP_STR)->valueint;
				gwy_ac_control_t.swingH = cJSON_GetObjectItem(json_packet_j, SWING_H_STR)->valueint;
				gwy_ac_control_t.swingV = cJSON_GetObjectItem(json_packet_j, SWING_V_STR)->valueint;
				gwy_ac_control_t.OnTimer = cJSON_GetObjectItem(json_packet_j, ONTIMER_STR)->valueint;
				gwy_ac_control_t.OffTimer = cJSON_GetObjectItem(json_packet_j, OFFTIMER_STR)->valueint;
				gwy_ac_control_t.Locking = cJSON_GetObjectItem(json_packet_j, LOCKING_STR)->valueint;
				if(configured) needtosend = true;
				break;

			case GWY_RECONF_PACKET:
				ESP_LOGI(DEBUG_TAG, "Gwy Reconfiguration packet\r\n");
				gwy_reconfigure_t.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSGSEQNO_STR)->valueint;
				gwy_reconfigure_t.gwy_ser_no = cJSON_GetObjectItem(json_packet_j, GWYSERNO_STR)->valueint;
				configured = false;
				break;

			case NODE_PROV_PACKET:
				ESP_LOGI(DEBUG_TAG, "Node Provisioning packet\r\n");
				provision_t.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSGSEQNO_STR)->valueint;
				provision_t.gwy_ser_no = cJSON_GetObjectItem(json_packet_j, GWYSERNO_STR)->valueint;
				provision_t.node_ser_no = cJSON_GetObjectItem(json_packet_j, NODESERNO_STR)->valueint;
				strcpy(provision_t.location, cJSON_GetObjectItem(json_packet_j, LOCATION_STR)->valuestring);
				fill_macid();
				break;

			case NODE_UNPROV_PACKET:
				ESP_LOGI(DEBUG_TAG, "Node Unprovisioning packet\r\n");
				unprovision_t.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSGSEQNO_STR)->valueint;
				unprovision_t.gwy_ser_no = cJSON_GetObjectItem(json_packet_j, GWYSERNO_STR)->valueint;
				unprovision_t.node_ser_no = cJSON_GetObjectItem(json_packet_j, NODESERNO_STR)->valueint;
				unprovision_t.elemnt_addr = cJSON_GetObjectItem(json_packet_j, ELMNT_ADDR_STR)->valueint;
				break;

			case NODE_AC_CONTROL_PACKET:
				ESP_LOGI(DEBUG_TAG, "Node AC Control packet\r\n");
				node_ac_control_t.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSGSEQNO_STR)->valueint;
				node_ac_control_t.gwy_ser_no = cJSON_GetObjectItem(json_packet_j, GWYSERNO_STR)->valueint;
				node_ac_control_t.node_ser_no = cJSON_GetObjectItem(json_packet_j, NODESERNO_STR)->valueint;
				node_ac_control_t.elementAddr = cJSON_GetObjectItem(json_packet_j, ELMNT_ADDR_STR)->valueint;
				node_ac_control_t.power = cJSON_GetObjectItem(json_packet_j, POWER_STR)->valueint;
				strcpy(node_ac_control_t.mode_str, cJSON_GetObjectItem(json_packet_j, MODE_STR)->valuestring);
				node_ac_control_t.fan = cJSON_GetObjectItem(json_packet_j, FAN_STR)->valueint;
				node_ac_control_t.temp = cJSON_GetObjectItem(json_packet_j, TEMP_STR)->valueint;
				node_ac_control_t.swingH = cJSON_GetObjectItem(json_packet_j, SWING_H_STR)->valueint;
				node_ac_control_t.swingV = cJSON_GetObjectItem(json_packet_j, SWING_V_STR)->valueint;
				node_ac_control_t.OnTimer = cJSON_GetObjectItem(json_packet_j, ONTIMER_STR)->valueint;
				node_ac_control_t.OffTimer = cJSON_GetObjectItem(json_packet_j, OFFTIMER_STR)->valueint;
				node_ac_control_t.Locking = cJSON_GetObjectItem(json_packet_j, LOCKING_STR)->valueint;
				send_control_packet = true;
				break;

			case NODE_RECONF_PACKET:
				ESP_LOGI(DEBUG_TAG, "Node Reconfiguration packet\r\n");
				node_reconfigure_t.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSGSEQNO_STR)->valueint;
				node_reconfigure_t.gwy_ser_no = cJSON_GetObjectItem(json_packet_j, GWYSERNO_STR)->valueint;
				node_reconfigure_t.node_ser_no = cJSON_GetObjectItem(json_packet_j, NODESERNO_STR)->valueint;
				node_reconfigure_t.elementAddr = cJSON_GetObjectItem(json_packet_j, ELMNT_ADDR_STR)->valueint;
				break;

			case RESET_MQTT:
				ESP_LOGI(DEBUG_TAG, "Reset MQTT packet\r\n");
				gwy_reset_mqtt_t.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSGSEQNO_STR)->valueint;
				gwy_reset_mqtt_t.gwy_ser_no = cJSON_GetObjectItem(json_packet_j, GWYSERNO_STR)->valueint;
				LED_state = LED_STATE_MQTT_NOT_CONNECTED;
				reset_mqtt();
				break;

			default:
				ESP_LOGI(ERROR_TAG, "Unknown MQTT packet received in parse_json_packet\r\n");
		}
	}
	printf("Error code after storing details in structure : %d\n",json_ack_err_code);
	switch(json_packet_id)
	{
		case GWY_REG_PACKET:
			sprintf(pubmessage, "{%s : %d, %s : %d, %s : %d, %s : %s, %s : %d}",
				JSON_PACKET_ID, json_packet_id,
				MSGSEQNO_STR, gwy_registration_t.msg_seq_no,
				GWYSERNO_STR, gwy_registration_t.gwy_ser_no,
				LOCATION_STR, gwy_registration_t.location,
				ERROR_CODE_STR, json_ack_err_code);
			break;

		case GWY_UNREG_PACKET:
			sprintf(pubmessage, "{%s : %d, %s : %d, %s : %d, %s : %s, %s : %d}",
				JSON_PACKET_ID, json_packet_id,
				MSGSEQNO_STR, gwy_unregistration_t.msg_seq_no,
				GWYSERNO_STR, gwy_unregistration_t.gwy_ser_no,
				LOCATION_STR, gwy_unregistration_t.location,
				ERROR_CODE_STR, json_ack_err_code);
			break;

		case GWY_AC_CONTROL_PACKET:
			sprintf(pubmessage, "{%s : %d, %s : %d, %s : %d, %s : %d, %s : %s, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d}",
				JSON_PACKET_ID, json_packet_id,
				MSGSEQNO_STR, gwy_ac_control_t.msg_seq_no,
				GWYSERNO_STR, gwy_ac_control_t.gwy_ser_no,
				POWER_STR, gwy_ac_control_t.power,
				MODE_STR, gwy_ac_control_t.mode_str,
				FAN_STR, gwy_ac_control_t.fan,
				TEMP_STR, gwy_ac_control_t.temp,
				SWING_H_STR, gwy_ac_control_t.swingH,
				SWING_V_STR, gwy_ac_control_t.swingV,
				ONTIMER_STR, gwy_ac_control_t.OnTimer,
				OFFTIMER_STR, gwy_ac_control_t.OffTimer,
				LOCKING_STR, gwy_ac_control_t.Locking,
				ERROR_CODE_STR, json_ack_err_code);
			break;
	}
	add_to_pubmesg_queue(pubmessage, publish_topic);
	return json_ack_err_code;
}

void reset_mqtt()
{
	registered = false;
	configured = false;
	mqtt_connected = false;
}

/**
 * @brief Function which contains a queue of data that needs
 * to be sent back to cloud
 * @param none
 * @retval none
 */
int8_t publish_to_mqtt()
{
	if(PublishMessage(mqtt_client_index, mqtt_msgid, mqtt_qos, mqtt_retain, pubmesg_head_ptr->topic, pubmesg_head_ptr->message)==SUCCESS)
	{
		publishing_flag = false;
		return SUCCESS;
	}
	publishing_flag = false;
	return FAILURE;
}

void remove_from_pubmesg_queue()
{
	if(pubmesg_head_ptr->next == NULL)
	{
		pubmesg_head_ptr->next = NULL;
		pubmesg_head_ptr->prev = NULL;
		pubmesg_head_ptr = NULL;
		pubmesg_tail_ptr = NULL;
		return;
	}
	pubmesg_head_ptr = pubmesg_head_ptr->next;
	free(pubmesg_head_ptr->prev);
	pubmesg_head_ptr->prev = NULL;
}

void add_to_pubmesg_queue(char *msg, char *topic)
{
	struct pub_mesg_struct *pubmesg_node = (struct pub_mesg_struct *)malloc(sizeof(struct pub_mesg_struct));
	if(pubmesg_node!=NULL)
	{
		//Adding very first element to queue
		if(pubmesg_head_ptr == NULL && pubmesg_tail_ptr == NULL)
		{
			pubmesg_head_ptr = pubmesg_node;
			pubmesg_node->prev = NULL;
		}
		else
			pubmesg_node->prev = pubmesg_tail_ptr;
		pubmesg_node->next = NULL;
		pubmesg_tail_ptr = pubmesg_node;
		strcpy(pubmesg_node->message,msg);
		pubmesg_node->topic = topic;
	}
	else
		printf("Error in memory allocation while trying to add to queue ...\n");
}

void *LTE_task(void *args)
{
    LTE_gpio_configuration();
    resetLte();
    LTE_initialization();
	sprintf(subscribe_topic, "%d/commands", GWY_SER_NO);
	sprintf(publish_topic, "%d/messages", GWY_SER_NO);
    establishMQTTConnection();
    while(1)
    {
		vTaskDelay(1);
		if(!restart_flag && mqtt_params_fetched_flag)
		{
			if(pubmesg_head_ptr!=NULL)
			{
				publishing_flag = true;
				if(publish_to_mqtt(pubmesg_head_ptr->topic, pubmesg_head_ptr->message)==SUCCESS)
					remove_from_pubmesg_queue();
			}
			else
			{
				ReadMessage(mqtt_client_index);
				if(strlen(json_packet) > 5)
				{
					// ESP_LOGI(DEBUG_TAG, json_packet);
					if(parse_json_packet()==SUCCESS);
					else
						ESP_LOGI(ERROR_TAG, "Error code : %d", json_ack_err_code);
					memset(json_packet, 0, sizeof(json_packet));
				}
			}
		}
		else
		{
			mqtt_connected = false;
			restart_flag = false;
			if(mqtt_params_fetched_flag)
				LTE_setup();
		}
    }
}

void *publish_task(void *args)
{
	while(1)
	{
		vTaskDelay(1);
		if(mqtt_connected)
		{

			// PublishMessage(mqtt_client_index, mqtt_msgid, mqtt_qos, mqtt_retain, "IR_PUB_TOPIC", "Hello");
			// printf("Publishing hello ... \n");
			sleep(1);
		}
	}

}

