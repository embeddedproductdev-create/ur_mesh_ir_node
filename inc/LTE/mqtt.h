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

/* JSON PACKET KEY STRINGS */
#define JSON_PACKET_ID_KEY "JsonPacketID"
#define JSON_ACK_NAME_KEY "JsonAckName"
#define JSON_ACK_SEQ_NO_KEY "JsonAckSeqNo"
#define MSG_SEQ_NO_KEY "MsgSeqNo"
#define GWY_SER_NO_KEY "GwySerNo"
#define NODE_SER_NO_KEY "NodeSerNo"
#define LOCATION_KEY "Location"
#define ELMNT_ADDR_KEY "ElementAddr"
#define MAC_ID_KEY "MacId"
#define POWER_KEY "Power"
#define MODE_KEY "Mode"
#define FAN_SPEED_KEY "Fan"
#define TEMPERATURE_KEY "Temp"
#define SWING_H_KEY "SwingH"
#define SWING_V_KEY "SwingV"
#define ONTIMER_KEY "OnTimer"
#define OFFTIMER_KEY "OffTimer"
#define AC_LOCKING_KEY "Locking"
#define TEMP_LOW_LIMIT_KEY "TempLowLimit"
#define TEMP_UP_LIMIT_KEY "TempUpLimit"
#define ERROR_CODE_KEY "ErrorCode"
#define TEMPERATURE_DATA_KEY "Temperature"
#define PUBLISH_PERIOD_KEY "PublishPeriodSec"

/* JSON ACK NAMES */
#define GWY_REG_ACK "Gwy Registration Ack"
#define GWY_UNREG_ACK "Gwy Unregistration Ack"
#define GWY_CONF_ACK "Gwy Configuration Ack"
#define GWY_RECONF_ACK "Gwy Reconfiguration Ack"
#define GWY_AC_CONTROL_ACK "Gwy AC Control Ack"
#define GWY_LOCKING_ACK "Gwy Locking Feature Ack"
#define GWY_TEMPERATURE_DATA_ACK "Gwy Temperature Data Ack"
#define GWY_PUB_CONF_ACK "Gwy Publish Configuration Ack"
#define GWY_RESET_MQTT_ACK "Gwy Reset MQTT Ack"
#define NODE_PROV_ACK "Node Provisioning Ack"
#define NODE_UNPROV_ACK "Node Unprovisioing Ack"
#define NODE_CONF_ACK "Node Configuration Ack"
#define NODE_RECONF_ACK "Node Reconfiguration Ack"
#define NODE_AC_CONTROL_ACK "Node AC Control Ack"
#define NODE_LOCKING_ACK "Node Locking Feature Ack"
#define NODE_TEMPERATURE_DATA_ACK "Node Temperature Data Ack"
#define NODE_PUB_CONF_ACK "Node Publish Configuration Ack"

#define MQTT_PACKET_BUFF_SIZE 500
#define LOCATION_KEY_LEN 20
#define MQTT_PACKET_NAME_LEN 40
#define PUBMESG_QUEUE_LIMIT 20
#define PUBMESG_LEN 300
#define MQTT_TOPIC_CHAR_LEN 20
#define NODE_TIMEOUT_INTERVAL_US 10000000

#define TEMPERATURE_LOWER_LIMIT 18
#define TEMPERATURE_UPPER_LIMIT 32

/* STRUCTURE DEFINITIONS */

struct base_data_t
{
	uint8_t json_packet_id;
	uint16_t msg_seq_no;
	uint16_t ack_seq_no;
	uint16_t gwy_ser_no;
	char gwy_ser_no_str[9];
	uint16_t node_ser_no;
	char node_ser_no_str[7];
	uint16_t elementAddr;
	uint16_t error_code;
	uint32_t request_in_time_us;
	char ack_name[MQTT_PACKET_NAME_LEN];
	char location[LOCATION_KEY_LEN];
};

typedef struct mqtt_reset_struct
{
	struct base_data_t base_data;
} mqtt_reset_t;

typedef struct gwy_reg_struct
{
	struct base_data_t base_data;
} gwy_reg_t;

typedef struct gwy_unreg_struct
{
	struct base_data_t base_data;
} gwy_unreg_t;

typedef struct reconf_struct
{
	struct base_data_t base_data;
	struct reconf_struct *next;
	struct reconf_struct *prev;
} reconf_t;

typedef struct control_struct
{
	struct base_data_t base_data;
	bool power;
	char mode_str[15];
	uint8_t mode_val;
	uint8_t fan;
	uint8_t temp;
	bool swingH;
	bool swingV;
	uint16_t OnTimer;
	uint16_t OffTimer;
	bool Locking;
	uint8_t TempUpLimit;
	uint8_t TempLowLimit;
	struct control_struct *next;
	struct control_struct *prev;
} control_t;

typedef struct prov_struct
{
	struct base_data_t base_data;
	uint8_t macid[6];
	struct prov_struct *next;
	struct prov_struct *prev;
	uint8_t appkey[16];
	uint8_t netkey[16];
	uint8_t appindex;
	uint8_t netindex;
} prov_t;

typedef struct unprov_struct
{
	struct base_data_t base_data;
	struct unprov_struct *next;
	struct unprov_struct *prev;
} unprov_t;

typedef struct pub_conf_struct
{
	struct base_data_t base_data;
	uint16_t pub_conf_period_in_sec;
	struct pub_conf_struct *next;
	struct pub_conf_struct *prev;
} pub_conf_t;

typedef struct TEMPERATURE_DATA_KEYuct
{
	struct base_data_t base_data;
	uint8_t measured_temperature;
} temperature_data_t;

struct pub_mesg_struct
{
	char message[PUBMESG_LEN];
	char *topic;
	struct pub_mesg_struct *next;
	struct pub_mesg_struct *prev;
};

enum json_packet_enum
{
	/* GWY PACKETS */
	GWY_REG_PACKET,
	GWY_CONF_PACKET,
	GWY_UNREG_PACKET,
	GWY_AC_CONTROL_PACKET,
	GWY_AC_LOCKING_PACKET,
	GWY_RECONF_PACKET,
	GWY_TEMPERATURE_DATA_PACKET,
	GWY_PUB_CONF_PACKET,
	RESET_MQTT,

	/* NODE PACKETS */
	NODE_PROV_PACKET = 100,
	NODE_CONF_PACKET,
	NODE_UNPROV_PACKET,
	NODE_AC_CONTROL_PACKET,
	NODE_AC_LOCKING_PACKET,
	NODE_RECONF_PACKET,
	NODE_TEMPERATURE_DATA_PACKET,
	NODE_PUB_CONF_PACKET,

	/* MISC PACKETS */
	UNKNOWN_PACKET = 99
};

enum ERROR_CODES
{

	// Basic
	FAILURE = -1,
	SUCCESS,
	INVALID_JSON_PACKET_ID_KEY,
	INVALID_MSG_SEQ_NO,
	INVALID_GWY_SER_NO,
	INVALID_NODE_SER_NO,
	INVALID_LOCATION_KEY,
	NODE_TIMEOUT,

	// Gwy Registration & Unregistration
	GWY_ALREADY_REG = 100,
	GWY_ALREADY_UNREG,

	// Node Prov & Unprov
	NODE_ALREADY_PROV = 200,
	NODE_ALREADY_UNPROV,

	// Gwy AC config & Node AC config
	// Gwy AC reconf & Node AC reconf
	GWY_ALREADY_UNCONF = 300,
	NODE_ALREADY_UNCONF,

	// Gwy AC control & Node AC control
	GWY_NOT_REG = 400,
	GWY_NOT_CONF,
	INVALID_POWER,
	INVALID_MODE,
	INVALID_FAN_SPEED,
	INVALID_TEMPERATURE,
	INVALID_SWING_H,
	INVALID_SWING_V,
	INVALID_ONTIMER,
	INVALID_OFFTIMER,
	INVALID_LOCKING,
	INVALID_TEMP_UPPER_LIMIT,
	INVALID_TEMP_LOWER_LIMIT,
	LOCKING_TEMP_UP_LIMIT_EXCEEDING_TEMP_UP_LIMIT,
	LOCKING_TEMP_LOW_LIMIT_EXCEEDING_TEMP_LOW_LIMIT,
	ILLOGICAL_LOCKING_TEMP_LIMIT,
	INVALID_ELMNT_ADDR,
	EXCEEDING_TEMP_LOWER_LIMIT,
	EXCEEDING_TEMP_UPPER_LIMIT,

	//Publish Configuration
	INVALID_PUBLISH_PERIOD = 500,

	UNKNOWN_ERROR_CODE = 999
};

enum Modes
{
	AUTO,
	COOL,
	HEAT,
	DRY,
	FAN
};

/*GLOBAL VARIABLES */

/*json*/
extern char json_packet[MQTT_PACKET_BUFF_SIZE];
extern cJSON *json_packet_j;
extern uint8_t json_packet_id;
extern int16_t json_ack_err_code;

/*flags*/
extern bool send_control_packet;
extern bool restart_flag;
extern bool network_flag;
extern bool client_flag;
extern bool subscribe_flag;

/*===============GWY====================*/

/*gwy registration & unregistration*/
extern gwy_reg_t gwy_registration_t;
extern gwy_unreg_t gwy_unregistration_t;

/*gwy ac remote configuration*/
extern reconf_t gwy_conf_t;

/*gwy ac remote reconfiguration*/
extern reconf_t gwy_reconf_t;

/*gwy ac control and ac locking*/
extern control_t gwy_ac_control_t;
extern control_t gwy_locking_t;

/*gwy temperature data*/
extern temperature_data_t gwy_temperature_data_t;

/*gwy publish configuration*/
extern pub_conf_t gwy_pub_conf_t;

/*reset mqtt*/
extern mqtt_reset_t gwy_reset_mqtt_t;

/*===============NODE====================*/

/*node provision*/
extern prov_t provision_t;
extern prov_t *prov_queue_head;
extern prov_t *prov_queue_tail;

/*node Unprovision*/
extern unprov_t unprovision_t;
extern unprov_t *unprov_queue_head;
extern unprov_t *unprov_queue_tail;

/*node ac control*/
extern control_t node_ac_control_t;
extern control_t *node_ac_control_queue_head;
extern control_t *node_ac_control_queue_tail;

/*node ac locking*/
extern control_t node_locking_t;

/*node ac remote reconfiguration*/
extern reconf_t node_reconf_t;
extern reconf_t *node_reconf_queue_head;
extern reconf_t *node_reconf_queue_tail;

/*node temperature data*/
extern temperature_data_t node_temperature_data_t;

/*node publish configuration*/
extern pub_conf_t node_pub_conf_t;
extern pub_conf_t *node_pub_conf_queue_head;
extern pub_conf_t *node_pub_conf_queue_tail;

/*===================================*/

/*pubmesg*/
extern struct pub_mesg_struct *pubmesg_queue_head;
extern struct pub_mesg_struct *pubmesg_queue_tail;

// SUBSCRIBE TOPICS
extern char subscribe_topic[MQTT_TOPIC_CHAR_LEN];

// PUBLISH TOPICS
extern char publish_topic[MQTT_TOPIC_CHAR_LEN];

/* MQTT parameters */
extern char mqtt_ip_address[16];
extern uint16_t mqtt_port;
extern uint8_t mqtt_client_index;
extern char mqtt_broker_username[30];
extern char mqtt_broker_password[30];
extern char mqtt_broker_tabname[30];
extern bool mqtt_params_fetched_flag;
extern bool publishing_flag;

/* FUNCTION DECLARATIONS */
void parse_json_packet(void);
void fill_macid(void);
void handle_sending_ack_to_cloud(uint8_t json_id);
void error_check_json(uint8_t json_packet_id);

#endif
