#include "include/main.h"

/*GLOBAL VARIABLE INITIALIZATION*/
bool button_pressed = false;
char json_packet[100] = "";
cJSON *json_packet_j, *model_j, *protocol_j, *command_j, *value_j = NULL;

control_t ac_control_t;

void print_cmdrecv()
{
	printf("\t========================================\r\n");
	printf("\tModel    : %s\r\n",ac_control_t.model_str);
	printf("\tProtocol : %d\r\n",ac_control_t.protocol_num);
	printf("\tCommand  : %s\r\n",ac_control_t.command_str);
	printf("\tValue	   : %d\r\n",ac_control_t.cmd_value);
	printf("\t========================================\r\n");
}

void app_main(void)
{
	IR_init();

	data_init_Daikin200();
	data_init_Daikin280();
	data_init_Daikin216();
	data_init_Hitachi296();

	LTE_gpio_configuration();

	resetLte();
	LTE_initialization();
	ConnectToNetwork();

	while(1)
	{
		ReadMessage(CLIENT_IDX);
		if(strlen(json_packet) > 20)
		{
		printf("JSON_PACKET : %s\r\n",json_packet);
		json_packet_j = cJSON_Parse(json_packet);
		model_j = cJSON_GetObjectItemCaseSensitive(json_packet_j, MODEL);
		protocol_j = cJSON_GetObjectItemCaseSensitive(json_packet_j, PROTOCOL);
		command_j = cJSON_GetObjectItemCaseSensitive(json_packet_j, COMMAND);
		value_j = cJSON_GetObjectItemCaseSensitive(json_packet_j, VALUE);
		strcpy(ac_control_t.model_str, model_j->valuestring);
		strcpy(ac_control_t.command_str, command_j->valuestring);
		ac_control_t.protocol_num = protocol_j->valueint;
		ac_control_t.cmd_value = value_j->valueint;
		print_cmdrecv();
		strcpy(json_packet, ""); //Emptying the string
		control_AC();
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}




