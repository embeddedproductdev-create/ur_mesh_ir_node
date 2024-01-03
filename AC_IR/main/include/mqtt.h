#ifndef MAIN_INCLUDE_MQTT_H_
#define MAIN_INCLUDE_MQTT_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "cJSON.h"

#define MODEL		"Model"
#define PROTOCOL	"Protocol"
#define COMMAND		"Command"
#define VALUE		"Value"

/*GLOBAL VARIABLE DECLARATIONS*/
extern char json_packet[100], model_str[10], command_str[10];
extern cJSON *json_packet_j, *model_j, *protocol_j, *command_j, *value_j;

typedef struct control_cmd
{
	char model_str[10], command_str[10];
	uint16_t protocol_num;
	uint8_t cmd_value;
}control_t;

extern control_t ac_control_t;

#endif /* MAIN_INCLUDE_MQTT_H_ */
