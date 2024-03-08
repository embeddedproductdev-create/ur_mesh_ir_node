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
#define POWER_STR		"Power"
#define MODE_STR		"Mode"
#define FAN_STR			"Fan"
#define TEMP_STR		"Temp"
#define SWING_H_STR		"SwingH"
#define SWING_V_STR		"SwingV"
#define ONTIMER_STR		"OnTimer"
#define OFFTIMER_STR	"OffTimer"
#define LOCKING_STR 	"Locking"

#define MQTT_PACKET_BUFF_SIZE 500
#define LOCATION_STR_LEN 20

/* STRUCTURE DEFINITIONS */

typedef struct mqtt_reset_struct{
	uint16_t msg_seq_no;
	uint16_t gwy_ser_no;
}mqtt_reset_t;

typedef struct gwy_reg_struct{
	uint16_t msg_seq_no;
	uint16_t gwy_ser_no;
	char location[LOCATION_STR_LEN];
}gwy_reg_t;

typedef struct gwy_unreg_struct{
	uint16_t msg_seq_no;
	uint16_t gwy_ser_no;
	char location[LOCATION_STR_LEN];
}gwy_unreg_t;

typedef struct reconf_struct{
	uint16_t msg_seq_no;
	uint16_t gwy_ser_no;
	uint16_t node_ser_no;
	uint16_t elementAddr;
}reconf_t;
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
	char location[LOCATION_STR_LEN];
}prov_t;

typedef struct unprov_struct{
	uint16_t msg_seq_no;
	uint16_t gwy_ser_no;
	uint16_t node_ser_no;
	uint8_t elemnt_addr;
	char location[LOCATION_STR_LEN];
}unprov_t;

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
	RESET_MQTT,
	UNKNOWN_PACKET = 99
};

enum ERROR_CODES{
	FAILURE = -1,
	SUCCESS,
	INVALID_JSON_PACKET_ID,
	INVALID_MSG_SEQ_NO,
	INVALID_GWY_SER_NO,
	INVALID_LOCATION_STR,
	INVALID_POWER_STR,
	INVALID_MODE_STR,
	INVALID_FAN_STR,
	INVALID_TEMP_STR,
	INVALID_SWING_H_STR,
	INVALID_SWING_V_STR,
	INVALID_ONTIMER_STR,
	INVALID_OFFTIMER_STR,
	INVALID_LOCKING_STR,
	INVALID_NODESERNO_STR,
	INVALID_ELMNT_ADDR_STR,
	UNKNOWN_ERROR_CODE = 999
};

/*GLOBAL VARIABLE DECLARATIONS*/
extern char json_packet[MQTT_PACKET_BUFF_SIZE];
extern cJSON *json_packet_j;
extern uint8_t json_packet_id;

extern int16_t error_code;
extern bool send_control_packet;

extern bool restart_flag;
extern bool network_flag;
extern bool client_flag;
extern bool subscribe_flag;

extern mqtt_reset_t gwy_mqtt_reset_t;

extern gwy_reg_t gwy_registration_t;
extern gwy_unreg_t gwy_unregistration_t;

extern prov_t provision_t;
extern unprov_t unprovision_t;

extern control_t gwy_ac_control_t;
extern control_t node_ac_control_t;

extern reconf_t node_reconfigure_t;
extern reconf_t gwy_reconfigure_t;

/* MQTT parameters */
extern char mqtt_ip_address[16];
extern uint16_t mqtt_port;
extern uint8_t mqtt_client_index;
extern char mqtt_username[30];
extern char mqtt_password[30];
extern char mqtt_tab_name[30];
extern bool mqtt_params_fetched_flag;

/* FUNCTION DECLARATIONS */

#ifdef __cplusplus
extern "C" {
#endif

int16_t parse_json_packet(void);
void fill_macid(void);
void error_check_json(uint8_t json_packet_id);

#ifdef __cplusplus
}
#endif

#endif
