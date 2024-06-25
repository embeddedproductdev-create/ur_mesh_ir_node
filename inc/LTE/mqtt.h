/**
 * @file mqtt.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains variables and definitions related to MQTT
 * @version 0.1
 * @date 2024-06-19
 * @copyright Copyright (c) 2024
 */

#ifndef MQTT_H_
#define MQTT_H_

#include "../Custom/main.h"

/* JSON PACKET KEY STRINGS */
#define JSON_PACKET_ID_KEY "JsonPacketID"
#define JSON_ACK_NAME_KEY "JsonAckName"
#define MSG_SEQ_NO_KEY "MsgSeqNo"
#define GWY_SER_NO_KEY "GwySerNo"
#define NODE_SER_NO_KEY "NodeSerNo"
#define LOCATION_KEY "Location"
#define APP_KEY_INDEX "AppKeyIndex"
#define APP_KEY "AppKey"
#define NET_KEY_INDEX "NetKeyIndex"
#define NET_KEY "NetKey"
#define ELEMENT_ADDR_KEY "ElementAddr"
#define MAC_ID_KEY "MacId"
#define MODE_KEY "Mode"
#define POWER_KEY "Power"
#define FAN_SPEED_KEY "FanSpeed"
#define TEMPERATURE_KEY "Temperature"
#define SWING_H_KEY "SwingH"
#define SWING_V_KEY "SwingV"
#define ONTIMER_KEY "OnTimer"
#define OFFTIMER_KEY "OffTimer"
#define AC_LOCKING_KEY "Locking"
#define TEMP_LOCK_LOW_LIMIT_KEY "TempLockLowLimit"
#define TEMP_LOCK_UP_LIMIT_KEY "TempLockUpLimit"
#define TEMP_LOCK_UP_LIMIT_KEY "TempLockUpLimit"
#define TEMP_LOCK_LOW_LIMIT_KEY "TempLockLowLimit"
#define ERROR_CODE_KEY "ErrorCode"
#define AMBIENT_TEMPERATURE_DATA_KEY "AmbientTemperature"
#define PUBLISH_PERIOD_KEY "PublishPeriodSec"
#define FIRMWARE_VERSION_KEY "FirmwareVersion"
#define REGISTERED_KEY "Registered"
#define PROTOCOL_SEL_NUM_KEY "Protocol"
#define PUBLISH_MESG_QUEUE_COUNT_KEY "PubMsgQueueCount"
#define PROV_QUEUE_COUNT_KEY "ProvQueueCount"
#define UNPROV_QUEUE_COUNT_KEY "UnProvQueueCount"
#define AC_CONTROL_QUEUE_COUNT_KEY "ACControlQueueCount"
#define RECONF_QUEUE_COUNT_KEY "ReconfQueueCount"
#define PUB_CONF_QUEUE_COUNT_KEY "PubConfQueueCount"
#define TEACHING_MODE_QUEUE_COUNT_KEY "TeachingModeQueueCount"
#define DEBUG_INFO_QUEUE_COUNT_KEY "DebugInfoQueueCount"
#define DEVICE_UPTIME_KEY "DeviceUpTimeHrs"
#define LOGGING_KEY "Logging"
#define RESET_DEVICE_KEY "ResetDevice"

/* JSON ACK NAMES */
#define GWY_REG_ACK_NAME "Gwy Registration ACK"
#define GWY_UNREG_ACK_NAME "Gwy Unregistration ACK"
#define GWY_CONF_ACK_NAME "Gwy AC Remote Configuration ACK"
#define GWY_RECONF_ACK_NAME "Gwy AC Remote Reconfiguration ACK"
#define GWY_AC_CONTROL_ACK_NAME "Gwy AC Control ACK"
#define GWY_MANUAL_AC_CONTROL_ACK_NAME "Gwy Manual AC control ACK"
#define GWY_HEARTBEAT_ACK_NAME "Gwy Heartbeat ACK"
#define GWY_HEARTBEAT_PUB_CONF_ACK_NAME "Gwy Heartbeat Publish Configuration ACK"
#define GWY_TEACHING_MODE_START_ACK_NAME "Gwy Teaching Mode Start ACK"
#define GWY_TEACHING_MODE_END_ACK_NAME "Gwy Teaching Mode End ACK"
#define GWY_DEBUG_INFO_ACK_NAME "Gwy Debug "
#define NODE_PROV_ACK_NAME "Node Provisioning ACK"
#define NODE_UNPROV_ACK_NAME "Node Unprovisioing ACK"
#define NODE_CONF_ACK_NAME "Node Configuration ACK"
#define NODE_RECONF_ACK_NAME "Node Reconfiguration ACK"
#define NODE_AC_CONTROL_ACK_NAME "Node AC Control ACK"
#define NODE_MANUAL_AC_CONTROL_ACK_NAME "Node Manual AC Control ACK"
#define NODE_HEARTBEAT_ACK_NAME "Node Heartbeat ACK"
#define NODE_HEARTBEAT_PUB_CONF_ACK_NAME "Node Heartbeat Publish Configuration ACK"
#define NODE_TEACHING_MODE_START_ACK_NAME "Node Teaching Mode Start ACK"
#define NODE_TEACHING_MODE_END_ACK_NAME "Node Teaching Mode End ACK"
#define NODE_DEBUG_INFO_ACK_NAME "Node Debug Info ACK"

/*AT command related*/
extern uint8_t MQTT_CLIENT_INDEX;
#define MQTT_VERSION 3 // 3 = 3.1, 4 = 3.1.1
#define MQTT_QOS 2 // 0 = atmost once | 1 = atleast once | 2 = exactly once
#define MQTT_RETAIN 0
#define MQTT_MSGID 2
#define MQTT_ENABLE_SSL 0
#define MQTT_SSL_CTX_INDEX 0
#define MQTT_KEEP_ALIVE_S 120
#define MQTT_CLEAN_SESSION 1
#define MQTT_MSG_RECV_MODE 0
#define MQTT_MSG_LEN_ENABLE 1
#define MQTT_WILL_FLAG 1
#define MQTT_WILL_QOS 2
#define MQTT_WILL_RETAIN 0
#define MQTT_WILL_TOPIC "will/topic"
#define MQTT_WILL_MESSAGE "Network Disconnected Unexpectedly"
#define MQTT_RECV_ID 2

/*JSON related*/
#define MQTT_PACKET_BUFF_SIZE 1024
#define LOCATION_STR_LEN 30
#define MQTT_PACKET_NAME_LEN 100
#define PUBMESG_QUEUE_LIMIT 20
#define PUBMESG_LEN 1024
#define MQTT_TOPIC_CHAR_LEN 100
#define NODE_COMM_TIMEOUT_INTERVAL_US (10*1000000) //10seconds
#define MIN_PUB_CONF_LIMIT 10

#define TEMP_ABS_LOW_LIMIT 18
#define TEMP_ABS_UP_LIMIT 32

#define DEFAULT_HEARTBEAT_PUB_CONF_PERIOD_SEC 10

/* STRUCTURE DEFINITIONS */
struct base_data_t
{
	uint8_t json_packet_id;
	uint16_t msg_seq_no;
	uint32_t gwy_ser_no;
	char gwy_ser_no_str[9];
	uint32_t node_ser_no;
	char node_ser_no_str[7];
	uint16_t elementAddr;
	uint16_t error_code;
	uint32_t request_in_time_us;
	bool request_sent_to_node_flag;
	char ack_name[MQTT_PACKET_NAME_LEN];
	char location[LOCATION_STR_LEN];
};

struct ac_control_params_t
{
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
	uint8_t TempLockUpLimit;
	uint8_t TempLockLowLimit;
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
	struct ac_control_params_t control;
	struct control_struct *next;
	struct control_struct *prev;
} control_t;

typedef struct teaching_mode_struct
{
	struct base_data_t base_data;
	struct teaching_mode_struct *next;
	struct teaching_mode_struct *prev;
} teaching_mode_t;

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
	uint8_t pub_conf_period_in_sec;
	struct pub_conf_struct *next;
	struct pub_conf_struct *prev;
} pub_conf_t;

typedef struct heartbeat_struct_t
{
	struct base_data_t base_data;
	struct ac_control_params_t control;
	uint8_t measured_temperature;
} heartbeat_t;

typedef struct manual_ac_control_ack_t
{
	struct base_data_t base_data;
	struct ac_control_params_t control;
} manual_ac_control_t;

typedef struct pub_mesg_struct
{
	char message[PUBMESG_LEN];
	char *topic;
	struct pub_mesg_struct *next;
	struct pub_mesg_struct *prev;
}pubmesg_t;

typedef struct debug_info_struct
{
	struct base_data_t base_data;
	bool logging;
	bool resetDevice;
	struct debug_info_struct *next;
	struct debug_info_struct *prev;
}debug_info_t;

enum json_packet_enum
{
	/* GWY PACKETS */
	GWY_REG_PACKET,
	GWY_CONF_ACK,
	GWY_UNREG_PACKET,
	GWY_AC_CONTROL_PACKET,
	GWY_MANUAL_AC_CONTROL_ACK,
	GWY_RECONF_PACKET,
	GWY_HEARTBEAT_ACK,
	GWY_HEARTBEAT_PUB_CONF_PACKET,
	GWY_TEACHING_MODE_START_PACKET,
	GWY_TEACHING_MODE_END_ACK,
	GWY_DEBUG_INFO_PACKET,

	/* NODE PACKETS */
	NODE_PROV_PACKET = 100,
	NODE_CONF_PACKET,
	NODE_UNPROV_PACKET,
	NODE_AC_CONTROL_PACKET,
	NODE_MANUAL_AC_CONTROL_ACK,
	NODE_RECONF_PACKET,
	NODE_HEARTBEAT_ACK,
	NODE_HEARTBEAT_PUB_CONF_PACKET,
	NODE_TEACHING_MODE_START_PACKET,
	NODE_TEACHING_MODE_END_ACK,
	NODE_DEBUG_INFO_PACKET,

	/* MISC PACKETS */
	UNKNOWN_PACKET = 9999
};

enum ERROR_CODES
{
	// Basic
	FAILURE = -1,
	SUCCESS,
	JSON_PACKET_ID_NOT_FOUND,
	JSON_PACKET_ID_UNKNOWN,
	MSG_SEQ_NO_NOT_FOUND,
	GWY_SER_NO_NOT_FOUND,
	GWY_SER_NO_INVALID,
	LOCATION_NOT_FOUND,
	LOCATION_EXCEEDING_RANGE,
	NODE_COMM_TIMEOUT,
	GWY_NOT_REG,
	GWY_ALREADY_REG,
	MAC_ID_NOT_FOUND,
	MAC_ID_CONTAINS_INVALID_CHARS_OR_INVALID_FORMAT,
	INVALID_MAC_ID_LENGTH,
	NODE_ALREADY_PROV,
	NODE_SER_NO_NOT_FOUND,
	ELEMENT_ADDR_NOT_FOUND,
	GWY_NOT_CONFIGURED_WITH_AC_REMOTE,
	NODE_NOT_CONFIGURED_WITH_AC_REMOTE,
	POWER_NOT_FOUND,
	POWER_EXCEEDING_RANGE,
	MODE_NOT_FOUND,
	MODE_EXCEEDING_RANGE,
	FAN_SPEED_NOT_FOUND,
	FANSPEED_EXCEEDING_RANGE,
	TEMPERATURE_NOT_FOUND,
	TEMPERATURE_EXCEEDING_RANGE,
	SWING_H_NOT_FOUND,
	SWING_H_EXCEEDING_RANGE,
	SWING_V_NOT_FOUND,
	SWING_V_EXCEEDING_RANGE,
	ONTIMER_NOT_FOUND,
	ONTIMER_EXCEEDING_RANGE,
	OFFTIMER_NOT_FOUND,
	OFFTIMER_EXCEEDING_RANGE,
	LOCKING_NOT_FOUND,
	LOCKING_EXCEEDING_RANGE,
	TEMP_LOCK_UP_LIMIT_NOT_FOUND,
	TEMP_LOCK_LOW_LIMIT_NOT_FOUND,
	TEMP_LOCK_UP_LIMIT_EXCEEDS_ABS_TEMP_UP_LIMIT,
	TEMP_LOCK_LOW_LIMIT_EXCEEDS_ABS_TEMP_LOW_LIMIT,
	ILLOGICAL_LOCKING_TEMP_LIMIT,
	PUBLISH_PERIOD_NOT_FOUND,
	PUBLISH_PERIOD_EXCEEDS_RANGE,
	MSG_SEQ_NO_EXCEEDING_RANGE,
	NODE_SER_NO_INVALID,
	RESET_DEVICE_NOT_FOUND,
	LOGGING_FLAG_NOT_FOUND,
	FORBIDDEN_OPERATION = 999,
	UNKNOWN_ERROR_CODE = 9999
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
extern cJSON *json_packet_j;
extern uint8_t json_packet_id;
extern int16_t json_ack_err_code;

/*flags*/
extern bool send_control_packet;
extern bool network_flag;
extern bool client_flag;
extern bool subscribe_flag;
extern bool hold_adding_to_pubmesg;

/*===============GWY====================*/

/*gwy registration & unregistration*/
extern gwy_reg_t gwy_registration_t;
extern gwy_unreg_t gwy_unregistration_t;

/*gwy ac remote configuration*/
extern reconf_t gwy_conf_t;

/*gwy ac remote reconfiguration*/
extern reconf_t gwy_reconf_t;

/*gwy ac control and manual control*/
extern control_t gwy_ac_control_t;
extern manual_ac_control_t gwy_manual_ac_control_t;

/*gwy temperature data*/
extern heartbeat_t gwy_heartbeat_t;

/*gwy publish configuration*/
extern pub_conf_t gwy_pub_conf_t;

/*teaching mode*/
extern teaching_mode_t gwy_teaching_mode_t;

/*Debug info*/
extern debug_info_t gwy_debug_info_t;

/*===============NODE====================*/

/*node provision*/
extern prov_t provision_t;
extern prov_t *prov_queue_head;
extern prov_t *prov_queue_tail;

/*node ac remote configuration*/
extern reconf_t node_conf_t;

/*node Unprovision*/
extern unprov_t unprovision_t;
extern unprov_t *unprov_queue_head;
extern unprov_t *unprov_queue_tail;

/*node ac control*/
extern control_t node_ac_control_t;
extern control_t *node_ac_control_queue_head;
extern control_t *node_ac_control_queue_tail;

/*node ac manual control*/
extern manual_ac_control_t node_manual_ac_control_t;

/*node ac remote reconfiguration*/
extern reconf_t node_reconf_t;
extern reconf_t *node_reconf_queue_head;
extern reconf_t *node_reconf_queue_tail;

/*node temperature data*/
extern heartbeat_t node_heartbeat_t;

/*node publish configuration*/
extern pub_conf_t node_hearbeat_pub_conf_t;
extern pub_conf_t *node_pub_conf_queue_head;
extern pub_conf_t *node_pub_conf_queue_tail;

/*node teaching mode*/
extern teaching_mode_t node_teaching_mode_t;
extern teaching_mode_t *node_teaching_mode_queue_head;
extern teaching_mode_t *node_teaching_mode_queue_tail;

/*node debug info*/
extern debug_info_t node_debug_info_t;
extern debug_info_t *node_debug_info_queue_head;
extern debug_info_t *node_debug_info_queue_tail;

/*===================================*/

/*pubmesg*/
extern pubmesg_t *pubmesg_queue_head;
extern pubmesg_t *pubmesg_queue_tail;

// SUBSCRIBE TOPICS
extern char subscribe_topic[MQTT_TOPIC_CHAR_LEN];

// PUBLISH TOPICS
extern char publish_topic[MQTT_TOPIC_CHAR_LEN];

/* MQTT parameters */
extern char mqtt_server_ip[16];
extern uint16_t mqtt_port;
extern char mqtt_broker_username[30];
extern char mqtt_broker_password[100];
extern char mqtt_client_id[100];
extern bool mqtt_params_fetched_flag;
extern bool publishing_flag;

/* FUNCTION DECLARATIONS */
#ifdef __cplusplus
extern "C"
{
#endif
void parse_json_packet(char *json_packet);
void fill_macid(void);
void handle_sending_ack_to_cloud(uint8_t json_id);
void error_check_json(uint8_t json_packet_id);
void isValidMacId(char *macid);
#ifdef __cplusplus
}
#endif

#endif
