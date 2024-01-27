#ifndef MAIN_INCLUDE_MQTT_H_
#define MAIN_INCLUDE_MQTT_H_

#include "main.h"

#define MSGSEQNO_STR	"MsgSeqNo"
#define GWYSERNO_STR	"GwySerNo"
#define NODESERNO_STR	"NodeSerNo"
#define MODEL_STR		"Model"
#define PROTOCOL_STR	"Protocol"
#define POWER_STR		"power"
#define MODE_STR		"mode"
#define FAN_STR			"fan"
#define TEMP_STR		"temp"
#define SWING_H_STR		"swingH"
#define SWING_V_STR		"swingV"
#define ONTIMER_STR		"OnTimer"
#define OFFTIMER_STR	"OffTimer"
#define CONF_FLAG_STR	"ConfiguredFlag"


/* STRUCTURE DEFINITIONS */
typedef struct json_control_struct
{
	cJSON *json_packet_j;
	cJSON *msg_seq_no_j;
	cJSON *gwy_ser_no_j;
	cJSON *node_ser_no_j;
	cJSON *model_j;
	cJSON *protocol_j;
	cJSON *power_j;
	cJSON *mode_j;
	cJSON *fan_j;
	cJSON *temp_j;
	cJSON *swingH_j;
	cJSON *swingV_j;
	cJSON *OnTimer_j;
	cJSON *OffTimer_j;
}json_control_t;

typedef struct control_struct
{
	uint16_t msg_seq_no;
	uint16_t gwy_ser_no;
	uint16_t node_ser_no;
	char model_str[10];
	uint16_t protocol_no;
	char power_str[3];
	uint8_t mode;
	uint8_t fan;
	uint8_t temp;
	char swingH_str[3];
	char swingV_str[3];
	uint16_t OnTimer;
	uint16_t OffTimer;
	uint8_t configured_flag;
}control_t;

/*GLOBAL VARIABLE DECLARATIONS*/
extern char json_packet[100];
extern json_control_t json_t;
extern control_t ac_control_t;

/*FUNCTION DECLARATIONS*/
void parse_and_control(void);
uint8_t fill_control_structure(void);
#endif /* MAIN_INCLUDE_MQTT_H_ */
