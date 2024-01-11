#include "include/main.h"

uint8_t fill_control_structure()
{
	if(strlen(json_packet)>20)
	{
		json_t.json_packet_j = cJSON_Parse(json_packet);
		ac_control_t.msg_seq_no = cJSON_GetObjectItemCaseSensitive(json_t.json_packet_j, MSGSEQNO)->valueint;
		ac_control_t.gwy_ser_no = cJSON_GetObjectItemCaseSensitive(json_t.json_packet_j, GWYSERNO)->valueint;
		ac_control_t.node_ser_no = cJSON_GetObjectItemCaseSensitive(json_t.json_packet_j, NODESERNO)->valueint;
		strcpy(ac_control_t.model_str, cJSON_GetObjectItemCaseSensitive(json_t.json_packet_j, MODEL)->valuestring);
		ac_control_t.protocol_no = cJSON_GetObjectItemCaseSensitive(json_t.json_packet_j, PROTOCOL)->valueint;
		strcpy(ac_control_t.power_str, cJSON_GetObjectItemCaseSensitive(json_t.json_packet_j, POWER)->valuestring);
		ac_control_t.mode = cJSON_GetObjectItemCaseSensitive(json_t.json_packet_j, MODE)->valueint;
		ac_control_t.fan = cJSON_GetObjectItemCaseSensitive(json_t.json_packet_j, FAN)->valueint;
		ac_control_t.temp = cJSON_GetObjectItemCaseSensitive(json_t.json_packet_j, TEMP)->valueint;
		strcpy(ac_control_t.swingH_str, cJSON_GetObjectItemCaseSensitive(json_t.json_packet_j, SWING_H)->valuestring);
		strcpy(ac_control_t.swingV_str, cJSON_GetObjectItemCaseSensitive(json_t.json_packet_j, SWING_V)->valuestring);
		ac_control_t.OnTimer = cJSON_GetObjectItemCaseSensitive(json_t.json_packet_j, ONTIMER)->valueint;
		ac_control_t.OffTimer = cJSON_GetObjectItemCaseSensitive(json_t.json_packet_j, OFFTIMER)->valueint;
		ac_control_t.configured_flag = cJSON_GetObjectItemCaseSensitive(json_t.json_packet_j, CONF_FLAG)->valueint;
		if(!ac_control_t.configured_flag)
			configured = false;
		return 1;
	}
	else
	{
		printf("json_packet length is too short\r\n");
		return 0;
	}
}

void parse_and_control()
{
	if(fill_control_structure())
	{
		if(!configured)
		{
			printf("Node configuration in process ... \r\n");
			if(control_AC())
				printf("Node has been configured\r\n");
			else
				printf("Node configuration failed\r\n");
		}
		else
		{
			printf("Node is configured to control using %s",curr_selected_protocol);
			(*control_ptr)();
		}
	}
}



