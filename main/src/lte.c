
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_system.h"

#include "../inc/cJSON.h"
#include "../inc/json_maker.h"
#include "../inc/lte.h"
#include "../inc/led.h"
#include "../inc/main.h"
#include "../inc/heartbeat.h"
#include "../inc/flash.h"
#include "../inc/ir.h"

#define BAUD_RATE 115200

#define LTE_TAG "LTE"

#define LTE_RESET_PIN 46
#define LTE_POWER_PIN 9
#define TXD_PIN 17
#define RXD_PIN 18
#define CTS_PIN 11
#define RTS_PIN 10
#define UART_BUF_SIZE 800

#define MIN_LTE_RESP_WAIT_MS 500

#define MQTT_TOPIC_CHAR_LEN 100
#define MQTT_CMD_RESP_LEN 200
#define MQTT_ACK_SIZE 1024
#define RETRY_COUNT 5
#define NETWORK_CHECK_INTERVAL_TICKS pdMS_TO_TICKS(60000) //60s once

/*MQTT Configuration parameters*/
#define MQTT_CLIENT_INDEX 2
#define WILL_MSG "Device disconnected unexpectedly"
#define WILL_MSG_LEN strlen(WILL_MSG)
#define MQTT_MSG_ID 2
#define MQTT_QOS 2
#define MQTT_WILL_QOS 2
#define MQTT_WILL_RETAIN 1
#define MQTT_WILL_FLAG 1
#define MQTT_KEEP_ALIVE_S 10

char LTE_UART_data[UART_BUF_SIZE];

/*Basic AT commands*/
const char *AT_CMD = "AT\r";
const char *CHECK_FIRMWARE_CMD = "ATI\r";
const char *CHECK_OPERATOR_SELECTION_CMD = "AT+COPS?\r";
const char *CHECK_DOMAIN_REG_CMD = "AT+CREG?\r";
const char *ENABLE_SIM_HOTSWAP_CMD = "AT+QSIMDET=1,0\r";
const char *PING_CMD = "AT+QPING=1,\"google.com\",4,1\r";
const char *PING_RESP = "+QPING:";
const char *POWER_DOWN_CMD = "AT+QPOWD\r";
char SET_BAUD_RATE_CMD[MQTT_CMD_RESP_LEN];

/*PDP and TCP config*/
const char *TCP_CONFIG_CMD = "AT+QICSGP=1,1,\"airtelgprs.com\",\"\",\"\",0\r";
const char *PDP_CONTXT_ACT_CMD = "AT+QIACT=1\r";

/*MQTT Cmd and Resp*/
const char *CLEAN_SESSION_CMD = "AT+QMTCFG=\"session\",2,1\r";
char KEEP_ALIVE_CMD[MQTT_CMD_RESP_LEN];
const char *MSG_RECV_MODE_CMD = "AT+QMTCFG=\"recv/mode\",2,0\r";
char WILL_CMD[MQTT_CMD_RESP_LEN];

char MQTT_NETWORK_OPEN_CMD[MQTT_CMD_RESP_LEN];
char MQTT_NETWORK_OPEN_RESP[MQTT_CMD_RESP_LEN];
const char *MQTT_NETWORK_CHECK_CMD = "AT+QMTOPEN?\r";
char MQTT_NETWORK_CHECK_RESP[MQTT_CMD_RESP_LEN];

const char *MQTT_NETWORK_CLOSE_CMD = "AT+QMTCLOSE=2\r";
const char *MQTT_NETWORK_CLOSE_RESP = "+QMTCLOSE: 2,0";

char MQTT_CLIENT_CONN_CMD[MQTT_CMD_RESP_LEN];
const char *MQTT_CLIENT_CONN_RESP = "+QMTCONN: 2,0,0";
const char *CHECK_CLIENT_CONN_CMD = "AT+QMTCONN?\r";
const char *MQTT_CLIENT_CONN_OK_RESP = "+QMTCONN: 2,3";
const char *MQTT_CLIENT_DISCONN_CMD = "AT+QMTDISC=2";

const char *MQTT_READ_MSG_CMD = "AT+QMTRECV=2\r";
const char *OK_RESP = "OK\r\n";

char MQTT_SUB_CMD[MQTT_CMD_RESP_LEN];
const char *MQTT_SUB_RESP = "+QMTSUB: 2,2,0";

const char *QMTSTAT_1_ERROR = "+QMTSTAT: 2,1";
const char *QMTOPEN_2_ERROR = "+QMTOPEN: 2,2";
const char *QMTOPEN_3_ERROR = "+QMTOPEN: 2,3";

/*MQTT Packet JSON Keys*/
const char* JSON_PACKET_ID_KEY = "JsonPacketID";
const char* JSON_ACK_NAME_KEY = "JsonAckName";
const char* MSG_SEQ_NO_KEY = "MsgSeqNo";
const char* GWY_SER_NO_KEY = "GwySerNo";
const char* NODE_SER_NO_KEY = "NodeSerNo";
const char* LOCATION_KEY = "Location";
const char* APP_KEY_INDEX = "AppKeyIndex";
const char* APP_KEY = "AppKey";
const char* NET_KEY_INDEX = "NetKeyIndex";
const char* NET_KEY = "NetKey";
const char* ELEMENT_ADDR_KEY = "ElementAddr";
const char* MAC_ID_KEY = "MacId";
const char* MODE_KEY = "Mode";
const char* POWER_KEY = "Power";
const char* FAN_SPEED_KEY = "FanSpeed";
const char* TEMPERATURE_KEY = "Temperature";
const char* SWING_H_KEY = "SwingH";
const char* SWING_V_KEY = "SwingV";
const char* ONTIMER_KEY = "OnTimer";
const char* OFFTIMER_KEY = "OffTimer";
const char* AC_LOCKING_KEY = "Locking";
const char* UPPER_TEMPERATURE_LIMIT_KEY = "TempLockLowLimit";
const char* LOWER_TEMPERATURE_LIMIT_KEY = "TempLockUpLimit";
const char* ERROR_CODE_KEY = "ErrorCode";
const char* AMBIENT_TEMPERATURE_DATA_KEY = "AmbientTemperature";
const char* PUBLISH_PERIOD_KEY = "PublishPeriodSec";
const char* FIRMWARE_VERSION_KEY = "FirmwareVersion";
const char* REGISTERED_KEY = "Registered";
const char* PROTOCOL_SEL_NUM_KEY = "Protocol";
const char* PUBLISH_MESG_QUEUE_COUNT_KEY = "PubMsgQueueCount";
const char* PROV_QUEUE_COUNT_KEY = "ProvQueueCount";
const char* UNPROV_QUEUE_COUNT_KEY = "UnProvQueueCount";
const char* AC_CONTROL_QUEUE_COUNT_KEY = "ACControlQueueCount";
const char* RECONF_QUEUE_COUNT_KEY = "ReconfQueueCount";
const char* PUB_CONF_QUEUE_COUNT_KEY = "PubConfQueueCount";
const char* TEACHING_MODE_QUEUE_COUNT_KEY = "TeachingModeQueueCount";
const char* DEBUG_INFO_QUEUE_COUNT_KEY = "DebugInfoQueueCount";
const char* DEVICE_UPTIME_KEY = "DeviceUpTimeHrs";
const char* LOGGING_KEY = "Logging";
const char* RESET_DEVICE_KEY = "ResetDevice";
const char* LINK_KEY = "Link";
const char* TEACHING_START_KEY = "TeachingStart";

char subscribe_topic[MQTT_TOPIC_CHAR_LEN];
char publish_topic[MQTT_TOPIC_CHAR_LEN];

bool need_to_activate_pdp = false;
bool LOG_DATA  = true;
bool powerDownInProgress = false;

TickType_t lastNetworkCheckedTime = 0;

// Queues
QueueHandle_t publish_queue;
QueueHandle_t command_queue;

/**
 * @brief Function that publishes messages to MQTT
 * @param ack 
 * @return int 
 */
int mqtt_publish(char *ack, char *topic)
{
    char MQTT_PUBLISH_MESG_CMD[1024];
	sprintf(MQTT_PUBLISH_MESG_CMD, "AT+QMTPUBEX=2,2,2,0,\"%s\",%d\r\n", topic, strlen(ack));
	if (send_cmd_and_check_response(LOG_DATA, MQTT_PUBLISH_MESG_CMD, "PUBLISH_TO_MQTT", ">", 1000) == SUCCESS)
	{
		if (uart_write_bytes(UART_NUM_1, ack, strlen(ack)) != FAILURE)
        {
            return SUCCESS;
        }
        else {
            ESP_LOGE(LTE_TAG, "Failed to write publish message to LTE UART");
            return FAILURE;
        }
	}
	else
	{
        ESP_LOGE(LTE_TAG, "Publish Prompt not received");
		return FAILURE;
	}
	return FAILURE;
}

// Publishes ACK messages from the publish queue to MQTT broker
void publish_from_queue() {
    char *ack_message;
    while (xQueueReceive(publish_queue, &ack_message, 0) == pdPASS) {
        if (mqtt_publish(ack_message, publish_topic) != SUCCESS) {
            ESP_LOGE(LTE_TAG, "Failed to publish ACK message to MQTT broker.\n");
        }
        else {
            ESP_LOGI(LTE_TAG, "Published \n%s",ack_message);
            free(ack_message);
        }
    }
}

void enqueue_for_publish(char *ack_json) {
    if (xQueueSend(publish_queue, &ack_json, portMAX_DELAY) != pdPASS) {
        ESP_LOGE(LTE_TAG, "Publish Queue Full. Failed to enqueue ACK.\n");
    }
}

/**
 * @brief Function that generates the ack to be sent to cloud
 * @param cmd_struct 
 */
void generate_ack(CommandStruct *cmd_struct)
{
    char *buffer = (char *)malloc(1024);
    if(!buffer) {
        ESP_LOGE(LTE_TAG, "Memory Allocation failed for buffer | Can't generate ack");
        return;
    }
    struct jWriteControl *jwc = (struct jWriteControl *)malloc(sizeof(struct jWriteControl));
    if(!jwc) {
        ESP_LOGE(LTE_TAG, "Memory Allocation failed for jwc | Can't generate ack");
        free(buffer);
        return;
    }
    jwOpen(jwc, buffer, 1024, JW_OBJECT, 1);
    jwObj_int(jwc, JSON_PACKET_ID_KEY, cmd_struct->packetid);
    jwObj_int(jwc, MSG_SEQ_NO_KEY, cmd_struct->msgseqno);
    jwObj_int(jwc, ERROR_CODE_KEY, cmd_struct->errorcode);
    jwClose(jwc);
    enqueue_for_publish(buffer);
    free(jwc);
}

/**
 * @brief Get the error code name
 * @param code 
 * @return const char* 
 */
const char* get_error_code_name(error_codes code) {
    static const char* error_code_names[] = {
        "FAILURE",
        "SUCCESS",
        "JSON_PACKET_INVALID",
        "MISSING_PACKET_ID",
        "PACKET_ID_EXCEEDING_RANGE",
        "MISSING_MSG_SEQ_NO",
        "MSG_SEQ_NO_EXCEEDING_RANGE",
        "MISSING_LOCATION",
        "LOCATION_EXCEEDING_RANGE",
        "NODE_COMM_TIMEOUT",
        "GWY_NOT_REG",
        "GWY_ALREADY_REG",
        "MISSING_MACID",
        "INVALID_MACID",
        "MISSING_ELEMENT_ADDR",
        "ELEMENT_ADDR_EXCEEDING_RANGE",
        "GWY_NOT_CONFIGURED",
        "NODE_NOT_CONFIGURED",
        "MISSING_POWER",
        "POWER_EXCEEDING_RANGE",
        "MISSING_MODE",
        "MODE_EXCEEDING_RANGE",
        "MISSING_FAN_SPEED",
        "FAN_SPEED_EXCEEDING_RANGE",
        "MISSING_TEMPERATURE",
        "TEMPERATURE_EXCEEDING_RANGE",
        "MISSING_SWINGH",
        "SWINGH_EXCEEDING_RANGE",
        "MISSING_SWINGV",
        "SWINGV_EXCEEDING_RANGE",
        "MISSING_ONTIMER",
        "ONTIMER_EXCEEDING_RANGE",
        "MISSING_OFFTIMER",
        "OFFTIMER_EXCEEDING_RANGE",
        "MISSING_LOCKING",
        "LOCKING_EXCEEDING_RANGE",
        "MISSING_TEMPERATURE_UPPER_LIMIT",
        "TEMPERATURE_UPPER_LIMIT_EXCEEDING_RANGE",
        "MISSING_TEMPERATURE_LOWER_LIMIT",
        "TEMPERATURE_LOWER_LIMIT_EXCEEDING_RANGE",
        "INVALID_TEMPERATURE_LOCKING_LIMITS",
        "MISSING_PUBLISH_PERIOD",
        "PUBLISH_PERIOD_EXCEEDING_RANGE",
        "MISSING_RESET_DEVICE",
        "RESET_DEVICE_EXCEEDING_RANGE",
        "MISSING_LOGGING",
        "LOGGING_EXCEEDING_RANGE",
        "AC_REMOTE_UNSUPPORTED",
        "MISSING_TEACHING_START",
        "TEACHING_START_EXCEEDING_RANGE",
        "ENTERED_TEACHING_MODE",
        "EXITED_TEACHING_MODE",
        "DEVICE_ALREADY_IN_TEACHING_MODE",
    };
    
    int index = code + 1; // Adjust index for negative `FAILURE` as -1

    if (index >= 0 && index < sizeof(error_code_names) / sizeof(error_code_names[0])) {
        return error_code_names[index];
    }
    return "UNKNOWN_ERROR";
}

/**
 * @brief Function that validates the MacID
 * @param macid 
 * @return true 
 * @return false 
 */
bool isValidMacId(char *macid)
{
    uint8_t d = 0, s = 0;
    if (strlen(macid) != 17) return false;
    for (uint8_t i = 0; i < 17; i++)
    {
        if ((macid[i] >= '0' && macid[i] <= '9') ||
            (macid[i] >= 'a' && macid[i] <= 'f') ||
            (macid[i] >= 'A' && macid[i] <= 'F'))
            d++;
        else if ((i == 2 || i == 5 || i == 8 || i == 11 || i == 14) && macid[i] == ':')
            s++;
    }
    if (d == 12 && s == 5) return true;
    else return false;
    
}

/**
 * @brief Function that validates the command received from MQTT
 * @param json_obj Parsed MQTT command json object
 * @param cmd_struct Strucutre to which the information will get stored
 * @return int 
 */
void error_check_json(cJSON *json_obj, CommandStruct *cmd_struct)
{
    cmd_struct->errorcode = SUCCESS;

    // PacketId will be present in all packets, so let's check that first
	if (cJSON_GetObjectItem(json_obj, JSON_PACKET_ID_KEY) == NULL)
	{
		cmd_struct->errorcode = MISSING_PACKET_ID;
		return;
	}
	mqtt_packets packetid = cJSON_GetObjectItem(json_obj, JSON_PACKET_ID_KEY)->valueint;

	if ( 
        !(packetid>=0 && packetid<MAX_GWY_PACKET_ID) && 
        !(packetid>=100 && packetid<MAX_NODE_PACKET_ID) &&
        !(packetid == TEST_PACKET)
        )
	{
		cmd_struct->errorcode = PACKET_ID_EXCEEDING_RANGE;
		return;
	}
	cmd_struct->packetid = packetid;

    // If it's a test packet, then need not worry about error handling.
    if(packetid == TEST_PACKET){
		ESP_LOGI(LTE_TAG, "Received TEST PACKET");
		return;
	}

    // MsgSeqNo will be present in all packets, so let's check that first
    if (cJSON_GetObjectItem(json_obj, MSG_SEQ_NO_KEY))
    {
        int32_t msg_seq_no = cJSON_GetObjectItem(json_obj, MSG_SEQ_NO_KEY)->valueint;
        if (msg_seq_no < 0 && msg_seq_no > 65535) {
            cmd_struct->errorcode = MSG_SEQ_NO_EXCEEDING_RANGE;
            return;
        }
        cmd_struct->msgseqno = msg_seq_no;
    }
    else
    {
        cmd_struct->errorcode = MISSING_MSG_SEQ_NO;
        return;
    }

    // If it's a Node packet (but not prov packet), let's check for element addr
    if((cmd_struct->packetid>=100 && cmd_struct->packetid<MAX_NODE_PACKET_ID) && 
        cmd_struct->packetid != NODE_PROV_PACKET)
    {
        if (!cJSON_GetObjectItem(json_obj, ELEMENT_ADDR_KEY)) {cmd_struct->errorcode = MISSING_ELEMENT_ADDR; return;}
        else {
            int elemaddr = cJSON_GetObjectItem(json_obj, ELEMENT_ADDR_KEY)->valueint;
            if(elemaddr<=2 || elemaddr>=65535) {cmd_struct->errorcode = ELEMENT_ADDR_EXCEEDING_RANGE; return;}
            else {
                cmd_struct->elemaddr = elemaddr;
            }
        }
    }

    // We need to check registration packet before checking other packets
    if (cmd_struct->packetid == GWY_REG_PACKET)
    {
        if(registered) cmd_struct->errorcode = GWY_ALREADY_REG;
        else {
            int length = 0;
            if(!cJSON_GetObjectItem(json_obj, LOCATION_KEY)) {cmd_struct->errorcode = MISSING_LOCATION; return;}
            length = strlen(cJSON_GetObjectItem(json_obj, LOCATION_KEY)->valuestring);
            if(length==0 || length>LOCATION_STR_LEN) {cmd_struct->errorcode = LOCATION_EXCEEDING_RANGE; return;}
            strcpy(device_location_str, cJSON_GetObjectItem(json_obj, LOCATION_KEY)->valuestring);
        }
        return;
    }

    // Now before answering any other packet types, Gwy should have been registered
    if (!registered) {
        cmd_struct->errorcode = GWY_NOT_REG;
        return;
    }

    if(cmd_struct->packetid == GWY_RECONF_PACKET && !configured) {
        cmd_struct->errorcode = GWY_NOT_CONFIGURED;
        return;
    }

    if(cmd_struct->packetid == GWY_AC_CONTROL_PACKET || cmd_struct->packetid == NODE_AC_CONTROL_PACKET)
    {
        if(cmd_struct->packetid == GWY_AC_CONTROL_PACKET && !configured) {
            cmd_struct->errorcode = GWY_NOT_CONFIGURED;
            return;
        }

        int power, temperature, fanspeed, swingh, swingv, locking, ontimer, offtimer, upperTemperatureLimit, lowerTemperatureLimit;
        char mode[6] = "";

        // First make sure the required Keys are available.
        if(!cJSON_GetObjectItem(json_obj, POWER_KEY)) {cmd_struct->errorcode = MISSING_POWER; return;}
        else power = cJSON_GetObjectItem(json_obj, POWER_KEY)->valueint;
        if(!cJSON_GetObjectItem(json_obj, TEMPERATURE_KEY)) {cmd_struct->errorcode = MISSING_TEMPERATURE; return;}
        else temperature = cJSON_GetObjectItem(json_obj, TEMPERATURE_KEY)->valueint;
        if(!cJSON_GetObjectItem(json_obj, FAN_SPEED_KEY)) {cmd_struct->errorcode = MISSING_FAN_SPEED; return;}
        else fanspeed = cJSON_GetObjectItem(json_obj, FAN_SPEED_KEY)->valueint;
        if(!cJSON_GetObjectItem(json_obj, MODE_KEY)) {cmd_struct->errorcode = MISSING_MODE; return;}
        else strcpy(mode, cJSON_GetObjectItem(json_obj, MODE_KEY)->valuestring);
        if(!cJSON_GetObjectItem(json_obj, SWING_H_KEY)) {cmd_struct->errorcode = MISSING_SWINGH; return;}
        else swingh = cJSON_GetObjectItem(json_obj, SWING_H_KEY)->valueint;
        if(!cJSON_GetObjectItem(json_obj, SWING_V_KEY)) {cmd_struct->errorcode = MISSING_SWINGV; return;}
        else swingv = cJSON_GetObjectItem(json_obj, SWING_V_KEY)->valueint;
        if(!cJSON_GetObjectItem(json_obj, AC_LOCKING_KEY)) {cmd_struct->errorcode = MISSING_LOCKING; return;}
        else locking = cJSON_GetObjectItem(json_obj, AC_LOCKING_KEY)->valueint;
        if(!cJSON_GetObjectItem(json_obj, ONTIMER_KEY)) {cmd_struct->errorcode = MISSING_ONTIMER; return;}
        else ontimer = cJSON_GetObjectItem(json_obj, ONTIMER_KEY)->valueint;
        if(!cJSON_GetObjectItem(json_obj, OFFTIMER_KEY)) {cmd_struct->errorcode = MISSING_OFFTIMER; return;}
        else offtimer = cJSON_GetObjectItem(json_obj, OFFTIMER_KEY)->valueint;
        if(!cJSON_GetObjectItem(json_obj, UPPER_TEMPERATURE_LIMIT_KEY)) {cmd_struct->errorcode = MISSING_TEMPERATURE_UPPER_LIMIT; return;}
        else upperTemperatureLimit = cJSON_GetObjectItem(json_obj, UPPER_TEMPERATURE_LIMIT_KEY)->valueint;
        if(!cJSON_GetObjectItem(json_obj, LOWER_TEMPERATURE_LIMIT_KEY)) {cmd_struct->errorcode = MISSING_TEMPERATURE_LOWER_LIMIT; return;}
        else lowerTemperatureLimit = cJSON_GetObjectItem(json_obj, LOWER_TEMPERATURE_LIMIT_KEY)->valueint;

        // Now make sure all values are within range
        if(power!=0 && power !=1) {cmd_struct->errorcode = POWER_EXCEEDING_RANGE; return;}
        else cmd_struct->power = power;
        if(temperature<18 && temperature>32) {cmd_struct->errorcode = TEMPERATURE_EXCEEDING_RANGE; return;}
        else cmd_struct->temperature = temperature;
        if(!(fanspeed>=0 && fanspeed<=5)) {cmd_struct->errorcode = FAN_SPEED_EXCEEDING_RANGE; return;}
        else cmd_struct->fanspeed = fanspeed;
        if( strcasecmp(mode, "Cool") ||
            strcasecmp(mode, "Hot") ||
            strcasecmp(mode, "Auto") ||
            strcasecmp(mode, "Dry") ||
            strcasecmp(mode, "Fan")
            )
        {
            cmd_struct->errorcode = MODE_EXCEEDING_RANGE;
        }
        else strcpy(cmd_struct->mode_str, mode);
        if(swingh!=0 && swingh!=1) {cmd_struct->errorcode = SWINGH_EXCEEDING_RANGE; return;}
        else cmd_struct->swingh = swingh;
        if(swingv!=0 && swingv!=1) {cmd_struct->errorcode = SWINGV_EXCEEDING_RANGE; return;}
        else cmd_struct->swingv = swingv;
        if(locking!=0 && locking!=1) {cmd_struct->errorcode = LOCKING_EXCEEDING_RANGE; return;}
        else cmd_struct->locking = locking;
        if(!(ontimer>=0 && ontimer<=12)) {cmd_struct->errorcode = ONTIMER_EXCEEDING_RANGE; return;}
        else cmd_struct->ontimer = ontimer;
        if(!(offtimer>=0 && offtimer<=12)) {cmd_struct->errorcode = OFFTIMER_EXCEEDING_RANGE; return;}
        else cmd_struct->offtimer = offtimer;
        if(upperTemperatureLimit<18 || upperTemperatureLimit>32) {cmd_struct->errorcode = TEMPERATURE_UPPER_LIMIT_EXCEEDING_RANGE; return;}
        else cmd_struct->upperTemperatureLimit = upperTemperatureLimit;
        if(lowerTemperatureLimit<18 || lowerTemperatureLimit>32) {cmd_struct->errorcode = TEMPERATURE_LOWER_LIMIT_EXCEEDING_RANGE; return;}
        else cmd_struct->lowerTemperatureLimit = lowerTemperatureLimit;
        if(upperTemperatureLimit<lowerTemperatureLimit) {cmd_struct->errorcode = INVALID_TEMPERATURE_LOCKING_LIMITS; return;}    
        return;
    }

    if(cmd_struct->packetid == GWY_HEARTBEAT_PUB_CONF_PACKET || cmd_struct->packetid == NODE_HEARTBEAT_PUB_CONF_PACKET)
    {
        if(!cJSON_GetObjectItem(json_obj, PUBLISH_PERIOD_KEY)) {cmd_struct->errorcode = MISSING_PUBLISH_PERIOD; return;}
        else {
            int publishperiod = cJSON_GetObjectItem(json_obj, PUBLISH_PERIOD_KEY)->valueint;
            if(!(publishperiod>= MIN_PUBLISH_PERIOD_SEC && publishperiod <= 65535)) cmd_struct->errorcode = PUBLISH_PERIOD_EXCEEDING_RANGE;
            else cmd_struct->publishPeriodSec = publishperiod;
            return;
        }
    }

    if(cmd_struct->packetid == NODE_PROV_PACKET)
    {
        if(!cJSON_GetObjectItem(json_obj, MAC_ID_KEY)) {cmd_struct->errorcode = MISSING_MACID; return;}
        else {
            char macid[20];
            strcpy(macid, cJSON_GetObjectItem(json_obj, MAC_ID_KEY)->valuestring);
            if(!isValidMacId(macid)) cmd_struct->errorcode = INVALID_MACID;
            return;
        }
    }

    if(cmd_struct->packetid == GWY_TEACHING_MODE || cmd_struct->packetid == NODE_TEACHING_MODE)
    {
        if(cmd_struct->packetid == GWY_TEACHING_MODE && 
           teaching_in_progress) {cmd_struct->errorcode = DEVICE_ALREADY_IN_TEACHING_MODE; return;}
        if(!cJSON_GetObjectItem(json_obj, TEACHING_START_KEY)) {cmd_struct->errorcode = MISSING_TEACHING_START; return;}
        else {
            int teaching_start = cJSON_GetObjectItem(json_obj, TEACHING_START_KEY)->valueint;
            if(teaching_start!=0 && teaching_start!=1)  {cmd_struct->errorcode = TEACHING_START_EXCEEDING_RANGE; return;}
            else cmd_struct->teachingStart = teaching_start;
            return;
        }
    }

    if(cmd_struct->packetid == GWY_DEBUG_INFO_PACKET || cmd_struct->packetid == NODE_DEBUG_INFO_PACKET)
    {
        int logging, reset;
        if(!cJSON_GetObjectItem(json_obj, LOGGING_KEY)) {cmd_struct->errorcode = MISSING_LOGGING; return;}
        if(!cJSON_GetObjectItem(json_obj, RESET_DEVICE_KEY)) {cmd_struct->errorcode = MISSING_RESET_DEVICE; return;}
        logging = cJSON_GetObjectItem(json_obj, LOGGING_KEY)->valueint;
        reset = cJSON_GetObjectItem(json_obj, RESET_DEVICE_KEY)->valueint;
        if(logging!=0 && logging!=1) {cmd_struct->errorcode = LOGGING_EXCEEDING_RANGE; return;}
        if(reset!=0 && reset!=1) {cmd_struct->errorcode = RESET_DEVICE_EXCEEDING_RANGE; return;}
    }
}

/**
 * @brief Function that generates Debug Info ACK
 * 
 */
void generate_debug_info_ack(CommandStruct *cmd_struct)
{
    char *buffer = (char *)malloc(1024);
    if(!buffer) {
        ESP_LOGE(LTE_TAG, "Memory Allocation failed for buffer | Can't generate ack");
        return;
    }
    struct jWriteControl *jwc = (struct jWriteControl *)malloc(sizeof(struct jWriteControl));
    if(!jwc) {
        ESP_LOGE(LTE_TAG, "Memory Allocation failed for jwc | Can't generate ack");
        free(buffer);
        return;
    }
    char version[10], uptime[10];
    sprintf(version, "%d.%d.%d",MAJ_VERSION, MIN_VERSION, PATCH_VERSION);
    sprintf(uptime, "%0.2f", ((xTaskGetTickCount()*portTICK_PERIOD_MS)/3600000.00));
    jwOpen(jwc, buffer, 1024, JW_OBJECT, 1);
    jwObj_int(jwc, JSON_PACKET_ID_KEY, cmd_struct->packetid);
    jwObj_int(jwc, MSG_SEQ_NO_KEY, cmd_struct->msgseqno);
    jwObj_string(jwc, FIRMWARE_VERSION_KEY, version);
    jwObj_int(jwc, REGISTERED_KEY, registered);
    jwObj_string(jwc, PROTOCOL_SEL_NUM_KEY, ir_protocol);
    jwObj_int(jwc, PUBLISH_PERIOD_KEY, publishPeriod);
    jwObj_string(jwc, DEVICE_UPTIME_KEY, uptime);
    jwObj_int(jwc, ERROR_CODE_KEY, cmd_struct->errorcode);
    jwClose(jwc);
    enqueue_for_publish(buffer);
}

void parse_json()
{
    led_set_state(LED_STATE_MQTT_CMD_RECVD);
    CommandStruct cmd_struct;

    cJSON *json_obj = cJSON_Parse(LTE_UART_data);
    if(json_obj == NULL) {
        ESP_LOGE(LTE_TAG, "MQTT command parsing failed");
        return;
    }

    error_check_json(json_obj, &cmd_struct);
    ESP_LOGE(LTE_TAG, "Error Code : %s - %d",get_error_code_name(cmd_struct.errorcode), cmd_struct.errorcode);

    if(cmd_struct.errorcode == SUCCESS)
    {
        cmd_struct.timestamp = xTaskGetTickCount();

        // Add only Node based packets into Queue
        if(cmd_struct.packetid>=100 && cmd_struct.packetid<MAX_NODE_PACKET_ID) {
            if (xQueueSend(command_queue, &cmd_struct, portMAX_DELAY) != pdPASS) {
                ESP_LOGE(LTE_TAG, "Enqueing into Command Queue failed");
                return;
            }
            return;
        }

        switch(cmd_struct.packetid)
        {
            case GWY_REG_PACKET:
                registered = 1; update_led_status();
                set_str_in_nvs_flash(GENERAL_HANDLE, NVS_DEVICE_LOCATION_KEY, device_location_str);
                set_number_in_nvs_flash(GENERAL_HANDLE, NVS_REGISTERED_KEY, 1, UINT8);
                hb_timer_start();
                break;

            case GWY_UNREG_PACKET:
                registered = 0; update_led_status();
                set_number_in_nvs_flash(general_nvs_handle, NVS_REGISTERED_KEY, 0, UINT8);
                hb_timer_stop();
                break;

            case GWY_AC_CONTROL_PACKET:
                sending_ir_command = true; update_led_status();
                last_command.power = cmd_struct.power;
                last_command.temperature = cmd_struct.temperature;
                last_command.fanspeed = cmd_struct.fanspeed;
                strcpy(last_command.mode_str, cmd_struct.mode_str);
                last_command.swingh = cmd_struct.swingh;
                last_command.swingv = cmd_struct.swingv;
                last_command.locking = cmd_struct.locking;
                last_command.upperTemperatureLimit = cmd_struct.upperTemperatureLimit;
                last_command.lowerTemperatureLimit = cmd_struct.lowerTemperatureLimit;
                ir_transmit();
                sleep(1);
                set_last_ac_cmd_in_nvs_flash();
                sending_ir_command = false; update_led_status();
                break;

            case GWY_RECONF_PACKET:
                configured = 0; update_led_status();
                set_number_in_nvs_flash(general_nvs_handle, NVS_CONFIGURED_KEY, 0, UINT8);
                break;

            case GWY_HEARTBEAT_PUB_CONF_PACKET:
                if(publishPeriod == cmd_struct.publishPeriodSec);
                else {
                    publishPeriod = cmd_struct.publishPeriodSec;
                    set_number_in_nvs_flash(general_nvs_handle, NVS_PUBPERIOD_KEY, publishPeriod, UINT16);
                    hb_timer_restart();
                    send_cmd_and_check_response(LOG_DATA, WILL_CMD, "WILL_CMD", OK_RESP, MIN_LTE_RESP_WAIT_MS);
                }
                break;

            case GWY_DEBUG_INFO_PACKET:
                generate_debug_info_ack(&cmd_struct);
                return;

            default:
                break;
        }
        generate_ack(&cmd_struct);
    }
    else
    {
        generate_ack(&cmd_struct);
    }
}

error_codes check_response(char *uart_data, const char *check_string)
{
    if (strstr(uart_data, QMTOPEN_2_ERROR))
		return QMTOPEN_2_ERRORCODE;
	else if (strstr(uart_data, QMTOPEN_3_ERROR))
		return QMTOPEN_3_ERRORCODE;
	else if (strstr(uart_data, QMTSTAT_1_ERROR))
		return QMTSTAT_1_ERRORCODE;
    else if (strstr(uart_data, check_string)) 
        return SUCCESS;
	else return FAILURE;
}

/**
 * @brief Function that fetches data from LTE and checks if it is valid or not
 * @param cmd_id ID that is used to know what cmd was sent. This is being used instead of string comparisons which are expensive
 * @param timeout_ms How long should LTE wait for a response for the sent command.
 * @param check_string String that is used to check against the response to know if it is valid or not
 * @return int8_t
 */
error_codes fetch_and_check_data(bool logging, uint16_t timeout_ms, const char *check_string, const char *cmd_name)
{
    error_codes rc = FAILURE;
    bzero(LTE_UART_data, UART_BUF_SIZE);
    uart_flush(UART_NUM_1);
    int length = uart_read_bytes(UART_NUM_1, LTE_UART_data, UART_BUF_SIZE, pdMS_TO_TICKS(timeout_ms));
    if (length > 0)
    {
        if(logging) ESP_LOGI(LTE_TAG, "Received : %s", LTE_UART_data);
        if ((rc=check_response(LTE_UART_data, check_string)) == SUCCESS)
        {
            if (strstr(LTE_UART_data, "{"))
            {
                strcpy(LTE_UART_data, strstr(LTE_UART_data, "{"));
                ESP_LOGW(LTE_TAG, "%s", LTE_UART_data);
                parse_json();
            }
        }
    }
    else 
    {
        ESP_LOGE(LTE_TAG, "No Data | data_len : %d", strlen(LTE_UART_data));
    }
    return rc;
}

/**
 * @brief Function that takes care of sending an AT command to LTE, await response, check that response
 * is valid or not and perform necessary operation.
 * @param logging Flag that enables/disables logging of LTE-MQTT communication data
 * @param cmd The AT command that needs to be sent
 * @param requestString A string denoting what command is being sent. Gets logged if logging is enabled.
 * @param check_string String that needs to be compared against the response that we get from the LTE chip
 * @param response_wait_time_ms How long should the LTE chip wait for a response (in milliseconds)
 * @return 0=Success, -1=Failure
 */
error_codes send_cmd_and_check_response(bool logging, const char *cmd,
								   const char *cmdName, const char *check_string, uint32_t timeout_ms)
{
    error_codes rc = FAILURE;
	uart_flush_input(UART_NUM_1);
	if (uart_write_bytes(UART_NUM_1, cmd, strlen(cmd)) != FAILURE)
	{
		if(logging) ESP_LOGI(LTE_TAG, "Command sent : %s", cmd);
		rc = fetch_and_check_data(logging, timeout_ms, check_string, cmdName);
	}
	else
	{
		ESP_LOGE(LTE_TAG, "Error in sending AT command to LTE!!!");
	}
    return rc;
}

/**
 * Function that takes care of maintaining the MQTT communication with the broker
 * and also publishing ACK message from pubmessage queue to the Cloud
 * @param none
 * @retval none
 */
void establishMQTTConnection()
{
    error_codes rc;
	static uint8_t ping_fail_counter = 0;	
	if(need_to_activate_pdp)
	{
		send_cmd_and_check_response(LOG_DATA, TCP_CONFIG_CMD, "TCP_CONFIG_CMD", OK_RESP, MIN_LTE_RESP_WAIT_MS);
		if(send_cmd_and_check_response(LOG_DATA, PDP_CONTXT_ACT_CMD, "PDP_CONTXT_ACT_CMD", OK_RESP, 5000)!=SUCCESS)
            return;
	}
	if((rc=send_cmd_and_check_response(LOG_DATA, MQTT_NETWORK_OPEN_CMD, "MQTT_NETWORK_OPEN_CMD", MQTT_NETWORK_OPEN_RESP, MIN_LTE_RESP_WAIT_MS))==SUCCESS);
    else {
        switch(rc)
        {
            case QMTOPEN_2_ERRORCODE:
                ESP_LOGE(LTE_TAG, "QMTOPEN_2_ERRORCODE");
                send_cmd_and_check_response(LOG_DATA, MQTT_NETWORK_CLOSE_CMD, "MQTT_NETWORK_CLOSE_CMD", MQTT_NETWORK_CLOSE_RESP, MIN_LTE_RESP_WAIT_MS);
                return;

            case QMTOPEN_3_ERRORCODE:
                ESP_LOGE(LTE_TAG, "QMTOPEN_3_ERRORCODE");
                need_to_activate_pdp = true;
                return;

            default:
                break;
        }
    }
	
    if(send_cmd_and_check_response(LOG_DATA, MQTT_CLIENT_CONN_CMD, "MQTT_CLIENT_CONN_CMD", MQTT_CLIENT_CONN_RESP, MIN_LTE_RESP_WAIT_MS)==SUCCESS);
    else {
        ;
    }
	
    if(send_cmd_and_check_response(LOG_DATA, MQTT_SUB_CMD, "MQTT_SUB_CMD", MQTT_SUB_RESP, MIN_LTE_RESP_WAIT_MS)!=SUCCESS) return ;
	ESP_LOGI(LTE_TAG, "Resumed MQTT Connection");
    need_to_activate_pdp = false; mqtt_connected = true; update_led_status(); LOG_DATA = false;
    if(registered) hb_timer_start();
	while(!powerDownInProgress)
	{
		if(send_cmd_and_check_response(LOG_DATA, MQTT_READ_MSG_CMD, "MQTT_READ_MSG_CMD", OK_RESP, MIN_LTE_RESP_WAIT_MS)==SUCCESS){;}
        if(xTaskGetTickCount()-lastNetworkCheckedTime > NETWORK_CHECK_INTERVAL_TICKS){
            
            lastNetworkCheckedTime = xTaskGetTickCount();
            if(send_cmd_and_check_response(LOG_DATA, MQTT_NETWORK_CHECK_CMD, "MQTT_NETWORK_CHECK_CMD", MQTT_NETWORK_CHECK_RESP, MIN_LTE_RESP_WAIT_MS)!=SUCCESS)
            {
                ESP_LOGE(LTE_TAG, "Lost MQTT connection");
                return;
            }
            else
            {
                char msg[20];
                sprintf(msg, "%s is alive", serialNoStr);
                if(mqtt_publish(msg, publish_topic)==SUCCESS){;}
                else ESP_LOGE(LTE_TAG, "Alive message publish failed");
            }
        }
        if(send_cmd_and_check_response(LOG_DATA, PING_CMD, "PING_CMD", OK_RESP, 50)!=SUCCESS) {
			if(ping_fail_counter++ > RETRY_COUNT){
				ESP_LOGE(LTE_TAG, "Lost MQTT connection");
				ping_fail_counter = 0;
				mqtt_connected = false; update_led_status();
                if(registered) hb_timer_stop();
				LOG_DATA = true;
				break;
			}
			continue;
		}
        ping_fail_counter = 0;
        publish_from_queue();
        vTaskDelay(pdMS_TO_TICKS(10));
	}
}

/**
 * @brief Function that takes care of configuring the MQTT communication
 * @param none
 * @retval none
 */
void MQTT_config()
{
	send_cmd_and_check_response(LOG_DATA, CLEAN_SESSION_CMD, "CLEAN_SESSION_CMD", OK_RESP, MIN_LTE_RESP_WAIT_MS);
	send_cmd_and_check_response(LOG_DATA, KEEP_ALIVE_CMD, "KEEP_ALIVE_CMD", OK_RESP, MIN_LTE_RESP_WAIT_MS);
    send_cmd_and_check_response(LOG_DATA, WILL_CMD, "WILL_CMD", OK_RESP, MIN_LTE_RESP_WAIT_MS);
}

/**
 * @brief Function that performs basic AT cmd checks at startup
 * @param none
 * @retval none
 */
void execute_general_AT_cmds()
{    
	send_cmd_and_check_response(LOG_DATA, "AT&V\r", "DISPLAY_CURRENT_CONFIGURATION", OK_RESP, MIN_LTE_RESP_WAIT_MS);
	send_cmd_and_check_response(LOG_DATA, "ATE0\r", "TURN_OFF_ECHO_CMD", OK_RESP, MIN_LTE_RESP_WAIT_MS);
	send_cmd_and_check_response(LOG_DATA, CHECK_FIRMWARE_CMD, "CHECK_FIRMWARE_CMD", OK_RESP, MIN_LTE_RESP_WAIT_MS);
	send_cmd_and_check_response(LOG_DATA, CHECK_OPERATOR_SELECTION_CMD, "CHECK_OP_SEL_CMD", OK_RESP, MIN_LTE_RESP_WAIT_MS);
	send_cmd_and_check_response(LOG_DATA, CHECK_DOMAIN_REG_CMD, "CHECK_DOMAIN_REG_CMD", OK_RESP, MIN_LTE_RESP_WAIT_MS);
	send_cmd_and_check_response(LOG_DATA, ENABLE_SIM_HOTSWAP_CMD, "ENABLE_SIM_HOTSWAP_CMD", OK_RESP, MIN_LTE_RESP_WAIT_MS);
}

/**
 * @brief Function that takes care of initializing certain
 * string variables that we use across the LTE communication
 * like Subscribe topic, publish topic, etc.,
 * @param none
 * @retval none
 */
void initialize_mqtt_cmd_strings()
{
    sprintf(KEEP_ALIVE_CMD, "AT+QMTCFG=\"keepalive\",%d,%d\r", MQTT_CLIENT_INDEX, publishPeriod+5);
	sprintf(subscribe_topic, "%s/command", serialNoStr);
	sprintf(publish_topic, "%s/message", serialNoStr);
    sprintf(SET_BAUD_RATE_CMD, "AT+IPR=%d\r", BAUD_RATE);
	sprintf(MQTT_NETWORK_OPEN_CMD, "AT+QMTOPEN=%d,\"%s\",%d\r", MQTT_CLIENT_INDEX, MQTT_SERVER_IP, MQTT_PORT);
    sprintf(MQTT_NETWORK_OPEN_RESP, "+QMTOPEN: %d,0", MQTT_CLIENT_INDEX);
	sprintf(MQTT_NETWORK_CHECK_RESP, "+QMTOPEN: %d,\"%s\",%d", MQTT_CLIENT_INDEX, MQTT_SERVER_IP, MQTT_PORT);
	sprintf(MQTT_CLIENT_CONN_CMD, "AT+QMTCONN=%d,\"%s_abcd\",\"%s\",\"%s\"\r", MQTT_CLIENT_INDEX, serialNoStr, MQTT_BROKER_USERNAME, MQTT_BROKER_PASSWORD);
	sprintf(MQTT_SUB_CMD, "AT+QMTSUB=%d,%d,\"%s\",%d\r",MQTT_CLIENT_INDEX, MQTT_MSG_ID, subscribe_topic, MQTT_QOS);
    sprintf(WILL_CMD, "AT+QMTCFG=\"will\",%d,%d,%d,%d,\"%s\",\"%s\"\r",
    MQTT_CLIENT_INDEX, 
    MQTT_WILL_FLAG, 
    MQTT_WILL_QOS,
    MQTT_WILL_RETAIN,
    publish_topic, 
    WILL_MSG);
}

/**
 * @brief Function that configures the POWER and RESET pins of LTE as Output
 * @param none
 * @retval none
 */
void lte_gpio_configuration()
{
	gpio_set_direction(LTE_RESET_PIN, GPIO_MODE_OUTPUT);
	gpio_set_direction(LTE_POWER_PIN, GPIO_MODE_OUTPUT);
}

/**
 * @brief Funcion that performs Power Down of LTE using AT+QPOWD cmd
 * 
 */
void powerDownLTE()
{
    powerDownInProgress = true;
    led_set_state(LED_STATE_LTE_POWERING_DOWN);
    while(send_cmd_and_check_response(LOG_DATA, POWER_DOWN_CMD, "POWER_DOWN_CMD", OK_RESP, MIN_LTE_RESP_WAIT_MS)!=SUCCESS)
        vTaskDelay(pdMS_TO_TICKS(500));
    ESP_LOGI(LTE_TAG, "LTE Power-down Sequence done");
    for(int i=30;i>0;i--)
    {
        ESP_LOGI(LTE_TAG, "Restarting in %d seconds ... ",i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    esp_restart();
}

/**
 * @brief Function that performs the power up sequence of LTE.
 * @param none
 * @retval none
 * @warning Logic is Inverted
 */
void powerUpLTE()
{
	gpio_set_level(LTE_POWER_PIN, 0);
	vTaskDelay(pdMS_TO_TICKS(100));
	gpio_set_level(LTE_POWER_PIN, 1);
	vTaskDelay(pdMS_TO_TICKS(2500));
	gpio_set_level(LTE_POWER_PIN, 0);
    ESP_LOGI(LTE_TAG," LTE Power-Up Sequence Done");
}

/**
 * @brief configure esp32 uart
 * @param None
 * @retval None
 */
void lte_uart_init(void)
{
	const uart_config_t uart_config = {
		.baud_rate = BAUD_RATE,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_APB,
	};
	// uart_driver_install(UART_NUM_1, UART_BUF_SIZE, UART_BUF_SIZE, 10, &uart_event_queue, 0);
    uart_driver_install(UART_NUM_1, UART_BUF_SIZE, 0, 0, NULL, 0);
	uart_param_config(UART_NUM_1, &uart_config);
	uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

/**
 * @brief Thread that takes care of Establishing and Maintaining the LTE communication
 * @param args 
 */
void lte_task(void *args)
{
    publish_queue = xQueueCreate(PUBLISH_QUEUE_SIZE, sizeof(char*));
    command_queue = xQueueCreate(COMMAND_QUEUE_SIZE, sizeof(CommandStruct));

	initialize_mqtt_cmd_strings();
	lte_gpio_configuration();
	powerUpLTE();
	lte_uart_init();
	execute_general_AT_cmds();
    MQTT_config();
    while(1)
	{
        if(powerDownInProgress) {
            vTaskDelay(pdMS_TO_TICKS(30000));
            continue;
        }
        establishMQTTConnection();
        if(powerDownInProgress) continue;
        powerUpLTE();
	    MQTT_config();
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}