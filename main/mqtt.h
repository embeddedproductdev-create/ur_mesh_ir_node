#ifndef MQTT_H_
#define MQTT_H_

#include "main.h"

#define MSGSEQNO_STR	"MsgSeqNo"
#define GWYSERNO_STR	"GwySerNo"
#define NODESERNO_STR	"NodeSerNo"
#define ELMNT_ADDR_STR  "elementAddr"
#define POWER_STR		"power"
#define MODE_STR		"mode"
#define FAN_STR			"fan"
#define TEMP_STR		"temp"
#define SWING_H_STR		"swingH"
#define SWING_V_STR		"swingV"
#define ONTIMER_STR		"OnTimer"
#define OFFTIMER_STR	"OffTimer"


/* STRUCTURE DEFINITIONS */
typedef struct control_struct
{
	uint16_t msg_seq_no;
	uint16_t gwy_ser_no;
	uint16_t node_ser_no;
	uint16_t elementAddr;
	bool power;
	char mode_str[15];
	uint8_t fan;
	uint8_t temp;
	bool swingH;
	bool swingV;
	uint16_t OnTimer;
	uint16_t OffTimer;
}control_t;

/*GLOBAL VARIABLE DECLARATIONS*/
extern char json_packet[100];
extern cJSON *json_packet_j;
extern control_t ac_control_t;

#endif
