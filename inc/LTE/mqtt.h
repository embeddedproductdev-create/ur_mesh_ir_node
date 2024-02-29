/**
 * @file mqtt.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains variables and definitions related to MQTT
 * @version 0.1
 * @date 2024-02-29
 * @copyright Copyright (c) 2024
 */

#ifndef MQTT_H_
#define MQTT_H_

#include "../Custom/main.h"

#define JSON_PACKET_ID  "JsonPacketID"
#define MSGSEQNO_STR	"MsgSeqNo"
#define GWYSERNO_STR	"GwySerNo"
#define NODESERNO_STR	"NodeSerNo"
#define LOCATION_STR    "Location"
#define ELMNT_ADDR_STR  "ElementAddr"
#define MAC_ID_STR 		"MacId"
#define POWER_STR		"power"
#define MODE_STR		"mode"
#define FAN_STR			"fan"
#define TEMP_STR		"temp"
#define SWING_H_STR		"swingH"
#define SWING_V_STR		"swingV"
#define ONTIMER_STR		"OnTimer"
#define OFFTIMER_STR	"OffTimer"
#define LOCKING_STR 	"Locking"

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
	bool Locking;
}control_t;

typedef struct prov_struct{
	uint16_t msg_seq_no;
	uint16_t gwy_ser_no;
	uint16_t node_ser_no;
	uint8_t macid[6];
	char timestamp[15];
}prov_t;

typedef struct unprov_struct{
	uint16_t msg_seq_no;
	uint16_t gwy_ser_no;
	uint16_t node_ser_no;
	uint8_t elemnt_addr;
	char timestamp[15];
}unprov_t;

typedef struct gwy_reg_struct{
	uint16_t msg_seq_no;
	uint16_t gwy_ser_no;
	char location[15];
}gwy_reg_t;

typedef struct gwy_unreg_struct{
	uint16_t msg_seq_no;
	uint16_t gwy_ser_no;
	char location[15];
}gwy_unreg_t;

typedef struct reconf_struct{
	uint16_t msg_seq_no;
	uint16_t gwy_ser_no;
	uint16_t node_ser_no;
	uint16_t elementAddr;
}reconf_t;

enum json_packet_enum {
	GWY_REG_PACKET,
	GWY_CONF_PACKET,
	GWY_UNREG_PACKET,
	GWY_AC_CONTROL_PACKET,
	GWY_AC_LOCKING_PACKET,
	GWY_RECONF_PACKET,
	NODE_PROV_PACKET,
	NODE_CONF_PACKET,
	NODE_UNPROV_PACKET,
	NODE_AC_CONTROL_PACKET,
	NODE_AC_LOCKING_PACKET,
	NODE_RECONF_PACKET,
	UNKNOWN_PACKET = 99
};

/*GLOBAL VARIABLE DECLARATIONS*/
extern char json_packet[100];
extern cJSON *json_packet_j;

extern gwy_reg_t gwy_registration_t;
extern gwy_unreg_t gwy_unregistration_t;
extern prov_t provision_t;
extern unprov_t unprovision_t;
extern control_t ac_control_t;
extern reconf_t reconfigure_t;

#endif
