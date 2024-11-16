#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PUBLISH_QUEUE_SIZE 10
#define COMMAND_QUEUE_SIZE 10
#define UART_EVENT_QUEUE_SIZE 10

#define MIN_PUBLISH_PERIOD_SEC 300

#define SERIAL_NO_LEN 10
#define IR_PROTOCOL_NAME_LEN 20
#define LOCATION_STR_LEN 20
#define MAX_MODE_STR_LEN 7
#define MQTT_TOPIC_CHAR_LEN 100
#define MQTT_CMD_RESP_LEN 200

#define DEFAULT_DEVICE_SER_NO "DEV99999"
#define DEFAULT_DEVICE_LOCATION_STR "EARTH :)"

#define COOL_MODE_STR "Cool"
#define DRY_MODE_STR "Dry"
#define HEAT_MODE_STR "Heat"
#define FAN_MODE_STR "Fan"
#define AUTO_MODE_STR "Auto"

extern const char* JSON_PACKET_ID_KEY;
extern const char* JSON_ACK_NAME_KEY;
extern const char* MSG_SEQ_NO_KEY;
extern const char* GWY_SER_NO_KEY;
extern const char* NODE_SER_NO_KEY;
extern const char* LOCATION_KEY;
extern const char* APP_KEY_INDEX;
extern const char* APP_KEY;
extern const char* NET_KEY_INDEX;
extern const char* NET_KEY;
extern const char* ELEMENT_ADDR_KEY;
extern const char* MAC_ID_KEY;
extern const char* MODE_KEY;
extern const char* POWER_KEY;
extern const char* FAN_SPEED_KEY;
extern const char* TEMPERATURE_KEY;
extern const char* SWING_H_KEY;
extern const char* SWING_V_KEY;
extern const char* ONTIMER_KEY;
extern const char* OFFTIMER_KEY;
extern const char* AC_LOCKING_KEY;
extern const char* UPPER_TEMPERATURE_LIMIT_KEY;
extern const char* LOWER_TEMPERATURE_LIMIT_KEY;
extern const char* ERROR_CODE_KEY;
extern const char* AMBIENT_TEMPERATURE_DATA_KEY;
extern const char* PUBLISH_PERIOD_KEY;
extern const char* FIRMWARE_VERSION_KEY;
extern const char* REGISTERED_KEY;
extern const char* PROTOCOL_SEL_NUM_KEY;
extern const char* PUBLISH_MESG_QUEUE_COUNT_KEY;
extern const char* PROV_QUEUE_COUNT_KEY;
extern const char* UNPROV_QUEUE_COUNT_KEY;
extern const char* AC_CONTROL_QUEUE_COUNT_KEY;
extern const char* RECONF_QUEUE_COUNT_KEY;
extern const char* PUB_CONF_QUEUE_COUNT_KEY;
extern const char* TEACHING_MODE_QUEUE_COUNT_KEY;
extern const char* DEBUG_INFO_QUEUE_COUNT_KEY;
extern const char* DEVICE_UPTIME_KEY;
extern const char* LOGGING_KEY;
extern const char* RESET_DEVICE_KEY;
extern const char* LINK_KEY;
extern const char* TEACHING_START_KEY;

extern char subscribe_topic[MQTT_TOPIC_CHAR_LEN];
extern char publish_topic[MQTT_TOPIC_CHAR_LEN];

typedef enum
{
    INT8,
    UINT8,
    INT16,
    UINT16,
    INT32,
    UINT32,
    INT64,
    UINT64
}sizes_t;

typedef enum 
{
	// Basic
    QMTSTAT_1_ERRORCODE=-4,
    QMTOPEN_2_ERRORCODE=-3,
    QMTOPEN_3_ERRORCODE=-2,
	FAILURE = -1,
	SUCCESS,
    JSON_PACKET_INVALID,
	MISSING_PACKET_ID,
	PACKET_ID_EXCEEDING_RANGE,
	MISSING_MSG_SEQ_NO,
    MSG_SEQ_NO_EXCEEDING_RANGE,
	MISSING_LOCATION,
    LOCATION_EXCEEDING_RANGE,
    NODE_COMM_TIMEOUT,
    GWY_NOT_REG,
    GWY_ALREADY_REG,
    MISSING_MACID,
    INVALID_MACID,
    MISSING_ELEMENT_ADDR,
    ELEMENT_ADDR_EXCEEDING_RANGE,
    GWY_NOT_CONFIGURED,
    NODE_NOT_CONFIGURED,
    MISSING_POWER,
    POWER_EXCEEDING_RANGE,
    MISSING_MODE,
    MODE_EXCEEDING_RANGE,
    MISSING_FAN_SPEED,
    FAN_SPEED_EXCEEDING_RANGE,
    MISSING_TEMPERATURE,
    TEMPERATURE_EXCEEDING_RANGE,
    MISSING_SWINGH,
    SWINGH_EXCEEDING_RANGE,
    MISSING_SWINGV,
    SWINGV_EXCEEDING_RANGE,
    MISSING_ONTIMER,
    ONTIMER_EXCEEDING_RANGE,
    MISSING_OFFTIMER,
    OFFTIMER_EXCEEDING_RANGE,
    MISSING_LOCKING,
    LOCKING_EXCEEDING_RANGE,
    MISSING_TEMPERATURE_UPPER_LIMIT,
    TEMPERATURE_UPPER_LIMIT_EXCEEDING_RANGE,
    MISSING_TEMPERATURE_LOWER_LIMIT,
    TEMPERATURE_LOWER_LIMIT_EXCEEDING_RANGE,
    INVALID_TEMPERATURE_LOCKING_LIMITS,
    MISSING_PUBLISH_PERIOD,
    PUBLISH_PERIOD_EXCEEDING_RANGE,
    MISSING_RESET_DEVICE,
    RESET_DEVICE_EXCEEDING_RANGE,
    MISSING_LOGGING,
    LOGGING_EXCEEDING_RANGE,
    AC_REMOTE_UNSUPPORTED,
    MISSING_TEACHING_START,
    TEACHING_START_EXCEEDING_RANGE,
    ENTERED_TEACHING_MODE,
    EXITED_TEACHING_MODE,
    DEVICE_ALREADY_IN_TEACHING_MODE,
}error_codes;

typedef enum
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
	GWY_TEACHING_MODE,
	GWY_DEBUG_INFO_PACKET,
	MAX_GWY_PACKET_ID,

	/* NODE PACKETS */
	NODE_PROV_PACKET = 100,
	NODE_CONF_PACKET,
	NODE_UNPROV_PACKET,
	NODE_AC_CONTROL_PACKET,
	NODE_MANUAL_AC_CONTROL_ACK,
	NODE_RECONF_PACKET,
	NODE_HEARTBEAT_ACK,
	NODE_HEARTBEAT_PUB_CONF_PACKET,
	NODE_TEACHING_MODE,
	NODE_DEBUG_INFO_PACKET,
	MAX_NODE_PACKET_ID,

    /*MISC*/
    TEST_PACKET = 999
}mqtt_packets;

typedef struct
{
    TickType_t timestamp;
    error_codes errorcode;
    mqtt_packets packetid;
    uint16_t msgseqno;
    uint16_t elemaddr;
    uint8_t power;
    uint8_t temperature;
    uint8_t ambientTemperature;
    uint8_t fanspeed;
    uint8_t mode_num;
    char mode_str[MAX_MODE_STR_LEN];
    uint8_t swingh;
    uint8_t swingv;
    uint8_t locking;
    uint16_t ontimer;
    uint16_t offtimer;
    uint8_t upperTemperatureLimit;
    uint8_t lowerTemperatureLimit;
    uint16_t publishPeriodSec;
    uint8_t resetDevice;
    uint8_t logging;
    uint8_t teachingStart;
} CommandStruct;

extern CommandStruct last_command;

/*Global Variables*/
extern TaskHandle_t lte_task_handle;

/*Global Variables*/
extern char serialNoStr[SERIAL_NO_LEN];
extern bool mqtt_connected;
extern uint8_t registered;
extern uint8_t provisioned;
extern uint8_t configured;
extern bool sending_ir_command;
extern bool teaching_in_progress;
extern char ir_protocol[IR_PROTOCOL_NAME_LEN];
extern uint16_t publishPeriod;
extern char device_location_str[LOCATION_STR_LEN];
extern uint16_t teaching_mode_raw_len;
extern int16_t ir_protocol_num;
extern uint8_t newDevice;
extern bool powerDownInProgress;

/*Function Declarations*/
void lte_task(void *args);
void uart_event_task(void *pvParameters);
error_codes check_response(char *uart_data, const char *check_string);
error_codes fetch_and_check_data(bool logging, uint16_t timeout_ms, const char *check_string, const char *cmd_name);
error_codes send_cmd_and_check_response(bool logging, const char *cmd, const char *cmdName, const char *check_string, uint32_t timeout_ms);
void powerUpLTE();
void powerDownLTE();
void enqueue_for_publish(char *ack_json);