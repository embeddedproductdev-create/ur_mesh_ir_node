/**
 * @file Cloud.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to MQTT packet handling and Sending Acknowledgements
 * @version 0.8.7
 * @date 2024-04-16
 * @copyright Copyright (c) 2024
 */

#include "../../inc/JSON/json_maker.h"
#include "../../inc/LTE/LTE.h"

// Initialization
jWriteControl_t jwc;
int32_t json_ack_err_code = SUCCESS;
int32_t json_packet_id = UNKNOWN_PACKET;
cJSON *json_packet_j;

/* JSON PACKET KEY STRINGS */
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
const char* TEMP_LOCK_LOW_LIMIT_KEY = "TempLockLowLimit";
const char* TEMP_LOCK_UP_LIMIT_KEY = "TempLockUpLimit";
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

/* JSON ACK NAMES */
const char* GWY_REG_ACK_NAME = "Gwy Registration ACK";
const char* GWY_UNREG_ACK_NAME = "Gwy Unregistration ACK";
const char* GWY_CONF_ACK_NAME = "Gwy AC Remote Configuration ACK";
const char* GWY_RECONF_ACK_NAME = "Gwy AC Remote Reconfiguration ACK";
const char* GWY_AC_CONTROL_ACK_NAME = "Gwy AC Control ACK";
const char* GWY_MANUAL_AC_CONTROL_ACK_NAME = "Gwy Manual AC control ACK";
const char* GWY_HEARTBEAT_ACK_NAME = "Gwy Heartbeat ACK";
const char* GWY_HEARTBEAT_PUB_CONF_ACK_NAME = "Gwy Heartbeat Publish Configuration ACK";
const char* GWY_TEACHING_MODE_START_ACK_NAME = "Gwy Teaching Mode Start ACK";
const char* GWY_TEACHING_MODE_END_ACK_NAME = "Gwy Teaching Mode End ACK";
const char* GWY_DEBUG_INFO_ACK_NAME = "Gwy Debug Info ACK";
const char* NODE_PROV_ACK_NAME = "Node Provisioning ACK";
const char* NODE_UNPROV_ACK_NAME = "Node Unprovisioing ACK";
const char* NODE_CONF_ACK_NAME = "Node AC Remote Configuration ACK";
const char* NODE_RECONF_ACK_NAME = "Node AC Remote Reconfiguration ACK";
const char* NODE_AC_CONTROL_ACK_NAME = "Node AC Control ACK";
const char* NODE_MANUAL_AC_CONTROL_ACK_NAME = "Node Manual AC Control ACK";
const char* NODE_HEARTBEAT_ACK_NAME = "Node Heartbeat ACK";
const char* NODE_HEARTBEAT_PUB_CONF_ACK_NAME = "Node Heartbeat Publish Configuration ACK";
const char* NODE_TEACHING_MODE_START_ACK_NAME = "Node Teaching Mode Start ACK";
const char* NODE_TEACHING_MODE_END_ACK_NAME = "Node Teaching Mode End ACK";
const char* NODE_DEBUG_INFO_ACK_NAME = "Node Debug Info ACK";

struct pub_mesg_struct *pubmesg_queue_head = NULL;
struct pub_mesg_struct *pubmesg_queue_tail = NULL;

/**
 * @brief Function that returns the Mode string based on Mode value
 * @param mode_value
 * @return char* String that denotes what mode it is.
 */
char *get_mode_string(uint8_t mode_value)
{
    switch (mode_value)
    {
    case COOL:
        return "Cool";
    case DRY:
        return "Dry";
    case HEAT:
        return "Hot";
    case FAN:
        return "Fan";
    case AUTO:
        return "Auto";
    }
    return "NULL";
}

#if (IS_GWY)

void fill_macid()
{
    char macid[17];
    strcpy(macid, cJSON_GetObjectItem(json_packet_j, MAC_ID_KEY)->valuestring);
    strcpy(provision_t.macid_str, macid);
    // printf("MAC ID recvd from cloud: %s\n", macid);
    char hex_char_str[2];
    for (uint8_t index = 0, i = 0; index < 6; index++, i += 3)
    {
        strncat(hex_char_str, &macid[i], 1);
        strncat(hex_char_str, &macid[i + 1], 1);
        provision_t.macid[index] = strtol(hex_char_str, NULL, 16);
        strcpy(hex_char_str, "");
        // printf("provision_t.macid[%d] : %x\n", index, provision_t.macid[index]);
    }
}

/**
 * @brief Function that checks the validity of a mac id string
 * @param macid The mac id string to be checked
 * @return true If valid
 * @return false If invalid
 */
void isValidMacId(char *macid)
{
    uint8_t d = 0, s = 0;
    if (strlen(macid) != 17)
    {
        json_ack_err_code = INVALID_MAC_ID_LENGTH;
    }
    for (uint8_t i = 0; i < 17; i++)
    {
        if ((macid[i] >= '0' && macid[i] <= '9') ||
            (macid[i] >= 'a' && macid[i] <= 'f') ||
            (macid[i] >= 'A' && macid[i] <= 'F'))
            d++;
        else if ((i == 2 || i == 5 || i == 8 || i == 11 || i == 14) && macid[i] == ':')
            s++;
    }
    if (d == 12 && s == 5)
        ;
    else
    {
        json_ack_err_code = MAC_ID_CONTAINS_INVALID_CHARS_OR_INVALID_FORMAT;
    }
}

/**
 * @brief This function verifies the data integrity of the json packet received
 * before trying to parse it
 * @param none
 * @retval none
 */
void error_check_json(uint8_t json_packet_id)
{
    cyan_printf(LTE_DEBUG_TAG, "Error checking received packet ... ");
    /* Common in all Packets */
    /*
        Json packet id should be between (0 and 10) or (100 and 110) or be 99.
    */
    if ((json_packet_id >= 0 && json_packet_id <= 11) || json_packet_id == 99 || (json_packet_id >= 100 && json_packet_id <= 111) || json_packet_id == 800);
    else
    {
        json_ack_err_code = JSON_PACKET_ID_UNKNOWN;
        return;
    }
    if (cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY))
    {
        int32_t msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
        if (msg_seq_no >= 0 && msg_seq_no <= 65535)
            ;
        else
        {
            json_ack_err_code = MSG_SEQ_NO_EXCEEDING_RANGE;
        }
    }
    else
    {
        json_ack_err_code = MSG_SEQ_NO_NOT_FOUND;
        return;
    }
    if (cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY))
    {
        char gwysernostr[15];
        strcpy(gwysernostr, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
        if (!strcmp(gwysernostr, GWY_SER_NO_IN_STRING))
            ;
        else
        {
            json_ack_err_code = GWY_SER_NO_INVALID;
            return;
        }
    }
    else
    {
        json_ack_err_code = GWY_SER_NO_NOT_FOUND;
        return;
    }

    /* Registration check */
    switch (json_packet_id)
    {
    case GWY_UNREG_PACKET:
    case GWY_AC_CONTROL_PACKET:
    case GWY_HEARTBEAT_PUB_CONF_PACKET:
    case GWY_RECONF_PACKET:
    case GWY_TEACHING_MODE_START_PACKET:
    case NODE_PROV_PACKET:
    case NODE_UNPROV_PACKET:
    case NODE_HEARTBEAT_PUB_CONF_PACKET:
    case NODE_RECONF_PACKET:
    case NODE_TEACHING_MODE_START_PACKET:
    case NODE_DEBUG_INFO_PACKET:
        if (!registered)
        {
            json_ack_err_code = GWY_NOT_REG;
            return;
        }
    }

    /* Configuration check */
    switch (json_packet_id)
    {
    case GWY_AC_CONTROL_PACKET:
    case GWY_RECONF_PACKET:
        if (!configured)
        {
            json_ack_err_code = GWY_NOT_CONFIGURED_WITH_AC_REMOTE;
            return;
        }
    }

    /* Node Serial Number & Element Addr check */
    switch (json_packet_id)
    {
    case NODE_PROV_PACKET:
    case NODE_UNPROV_PACKET:
    case NODE_RECONF_PACKET:
    case NODE_TEACHING_MODE_START_PACKET:
    case NODE_HEARTBEAT_PUB_CONF_PACKET:
    case NODE_DEBUG_INFO_PACKET:
    case NODE_AC_CONTROL_PACKET:
        if (cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY))
            ;
        else
        {
            json_ack_err_code = NODE_SER_NO_NOT_FOUND;
            return;
        }
        if (json_packet_id != NODE_PROV_PACKET && !cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY))
            json_ack_err_code = ELEMENT_ADDR_NOT_FOUND;
    }

    switch (json_packet_id)
    {
    case GWY_DEBUG_INFO_PACKET:
    case NODE_DEBUG_INFO_PACKET:
        if (cJSON_GetObjectItem(json_packet_j, RESET_DEVICE_KEY))
            ;
        else
        {
            json_ack_err_code = RESET_DEVICE_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, LOGGING_KEY))
            ;
        else
        {
            json_ack_err_code = LOGGING_FLAG_NOT_FOUND;
            return;
        }
        break;

    case GWY_REG_PACKET:
        if (registered)
        {
            json_ack_err_code = GWY_ALREADY_REG;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, LOCATION_KEY))
        {
            char location[LOCATION_STR_LEN];
            strcpy(location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            if (strlen(location) > LOCATION_STR_LEN)
            {
                json_ack_err_code = LOCATION_EXCEEDING_RANGE;
                return;
            }
        }
        else
        {
            json_ack_err_code = LOCATION_NOT_FOUND;
            return;
        }
        return;

    case GWY_AC_CONTROL_PACKET:
    case NODE_AC_CONTROL_PACKET:
        if (cJSON_GetObjectItem(json_packet_j, POWER_KEY))
        {
            uint8_t power = cJSON_GetObjectItem(json_packet_j, POWER_KEY)->valueint;
            if (power != 0 && power != 1)
                json_ack_err_code = POWER_EXCEEDING_RANGE;
        }
        else
        {
            json_ack_err_code = POWER_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, MODE_KEY))
        {
            char mode[10];
            strcpy(mode, cJSON_GetObjectItem(json_packet_j, MODE_KEY)->valuestring);
            if (!strcasecmp(mode, "Cool") ||
                !strcasecmp(mode, "Hot") ||
                !strcasecmp(mode, "Auto") ||
                !strcasecmp(mode, "Dry") ||
                !strcasecmp(mode, "Fan"))
                ;
            else
            {
                json_ack_err_code = MODE_EXCEEDING_RANGE;
            }
        }
        else
        {
            json_ack_err_code = MODE_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, FAN_SPEED_KEY))
        {
            int32_t fanspeed = cJSON_GetObjectItem(json_packet_j, FAN_SPEED_KEY)->valueint;
            if (fanspeed >= 1 && fanspeed <= 5)
                ;
            else
            {
                json_ack_err_code = FANSPEED_EXCEEDING_RANGE;
                return;
            }
        }
        else
        {
            json_ack_err_code = FAN_SPEED_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, TEMPERATURE_KEY))
        {
            int32_t temperature = cJSON_GetObjectItem(json_packet_j, TEMPERATURE_KEY)->valueint;
            if (temperature >= TEMP_ABS_LOW_LIMIT && temperature <= TEMP_ABS_UP_LIMIT)
                ;
            else
            {
                json_ack_err_code = TEMPERATURE_EXCEEDING_RANGE;
                return;
            }
        }
        else
        {
            json_ack_err_code = TEMPERATURE_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, SWING_H_KEY))
        {
            uint8_t swing = cJSON_GetObjectItem(json_packet_j, SWING_H_KEY)->valueint;
            if (swing != 0 && swing != 1)
            {
                json_ack_err_code = SWING_H_EXCEEDING_RANGE;
                return;
            }
        }
        else
        {
            json_ack_err_code = SWING_H_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, SWING_V_KEY))
        {
            uint8_t swing = cJSON_GetObjectItem(json_packet_j, SWING_V_KEY)->valueint;
            if (swing != 0 && swing != 1)
            {
                json_ack_err_code = SWING_V_EXCEEDING_RANGE;
                return;
            }
        }
        else
        {
            json_ack_err_code = SWING_V_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, ONTIMER_KEY))
        {
            int8_t timer = cJSON_GetObjectItem(json_packet_j, ONTIMER_KEY)->valueint;
            if (timer >= 0 && timer <= 12)
                ;
            else
            {
                json_ack_err_code = ONTIMER_EXCEEDING_RANGE;
            }
        }
        else
        {
            json_ack_err_code = ONTIMER_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, OFFTIMER_KEY))
        {
            int8_t timer = cJSON_GetObjectItem(json_packet_j, OFFTIMER_KEY)->valueint;
            if (timer >= 0 && timer <= 12)
                ;
            else
            {
                json_ack_err_code = OFFTIMER_EXCEEDING_RANGE;
            }
        }
        else
        {
            json_ack_err_code = OFFTIMER_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, AC_LOCKING_KEY))
        {
            int16_t locking = cJSON_GetObjectItem(json_packet_j, AC_LOCKING_KEY)->valueint;
            if (locking != 0 && locking != 1)
            {
                json_ack_err_code = LOCKING_EXCEEDING_RANGE;
            }
        }
        else
        {
            json_ack_err_code = LOCKING_NOT_FOUND;
            return;
        }
        int16_t tempLockUpLimit, tempLockLowLimit;
        if (cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_UP_LIMIT_KEY))
        {
            tempLockUpLimit = (cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_UP_LIMIT_KEY))->valueint;
            if (tempLockUpLimit > TEMP_ABS_UP_LIMIT)
            {
                json_ack_err_code = TEMP_LOCK_UP_LIMIT_EXCEEDS_ABS_TEMP_UP_LIMIT;
                return;
            }
        }
        else
        {
            json_ack_err_code = TEMP_LOCK_UP_LIMIT_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_LOW_LIMIT_KEY))
        {
            tempLockLowLimit = (cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_LOW_LIMIT_KEY))->valueint;
            if (tempLockLowLimit < TEMP_ABS_LOW_LIMIT)
            {
                json_ack_err_code = TEMP_LOCK_LOW_LIMIT_EXCEEDS_ABS_TEMP_LOW_LIMIT;
                return;
            }
        }
        else
        {
            json_ack_err_code = TEMP_LOCK_LOW_LIMIT_NOT_FOUND;
            return;
        }
        if (tempLockLowLimit > tempLockUpLimit)
        {
            json_ack_err_code = ILLOGICAL_LOCKING_TEMP_LIMIT;
            return;
        }
        return;

    case NODE_PROV_PACKET:
        if (cJSON_GetObjectItem(json_packet_j, LOCATION_KEY))
        {
            char location[LOCATION_STR_LEN];
            strcpy(location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            if (strlen(location) > LOCATION_STR_LEN)
            {
                json_ack_err_code = LOCATION_EXCEEDING_RANGE;
                return;
            }
        }
        else
        {
            json_ack_err_code = LOCATION_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, MAC_ID_KEY))
        {
            char macid[20];
            strcpy(macid, cJSON_GetObjectItem(json_packet_j, MAC_ID_KEY)->valuestring);
            isValidMacId(macid);
        }
        else
        {
            json_ack_err_code = MAC_ID_NOT_FOUND;
            return;
        }
        return;

    case GWY_HEARTBEAT_PUB_CONF_PACKET:
    case NODE_HEARTBEAT_PUB_CONF_PACKET:
        if (cJSON_GetObjectItem(json_packet_j, PUBLISH_PERIOD_KEY))
        {
            int32_t PublishPeriodSec = cJSON_GetObjectItem(json_packet_j, PUBLISH_PERIOD_KEY)->valueint;
            if (PublishPeriodSec >= 300 && PublishPeriodSec <= 65535)
                ;
            else
            {
                json_ack_err_code = PUBLISH_PERIOD_EXCEEDS_RANGE;
            }
        }
        else
        {
            json_ack_err_code = PUBLISH_PERIOD_NOT_FOUND;
            return;
        }
        return;
    }
}

void get_mode_value(char *device_type)
{
    if (strcmp(device_type, "gwy") == 0)
    {
        if (strcasecmp(gwy_ac_control_t.control.mode_str, "Auto") == 0)
            gwy_ac_control_t.control.mode_val = AUTO;
        else if (strcasecmp(gwy_ac_control_t.control.mode_str, "Cool") == 0)
            gwy_ac_control_t.control.mode_val = COOL;
        else if (strcasecmp(gwy_ac_control_t.control.mode_str, "Dry") == 0)
            gwy_ac_control_t.control.mode_val = DRY;
        else if (strcasecmp(gwy_ac_control_t.control.mode_str, "Hot") == 0)
            gwy_ac_control_t.control.mode_val = HEAT;
        else if (strcasecmp(gwy_ac_control_t.control.mode_str, "Fan") == 0)
            gwy_ac_control_t.control.mode_val = FAN;
        eeprom_write_byte(EEPROM_SLAVE_ADDR, MODE_FLASH_ADDR, gwy_ac_control_t.control.mode_val);
    }
    else
    {
        if (strcasecmp(node_ac_control_t.control.mode_str, "Auto") == 0)
            node_ac_control_t.control.mode_val = AUTO;
        else if (strcasecmp(node_ac_control_t.control.mode_str, "Cool") == 0)
            node_ac_control_t.control.mode_val = COOL;
        else if (strcasecmp(node_ac_control_t.control.mode_str, "Dry") == 0)
            node_ac_control_t.control.mode_val = DRY;
        else if (strcasecmp(node_ac_control_t.control.mode_str, "Hot") == 0)
            node_ac_control_t.control.mode_val = HEAT;
        else if (strcasecmp(node_ac_control_t.control.mode_str, "Fan") == 0)
            node_ac_control_t.control.mode_val = FAN;
        eeprom_write_byte(EEPROM_SLAVE_ADDR, MODE_FLASH_ADDR, node_ac_control_t.control.mode_val);
    }
}

char *get_err_string(int16_t err_code)
{
    switch (err_code)
    {
    case FAILURE:
        return "FAILURE";
    case SUCCESS:
        return "SUCCESS";
    case JSON_PACKET_ID_NOT_FOUND:
        return "JSON_PACKET_ID_NOT_FOUND";
    case JSON_PACKET_ID_UNKNOWN:
        return "JSON_PACKET_ID_UNKNOWN";
    case MSG_SEQ_NO_NOT_FOUND:
        return "MSG_SEQ_NO_NOT_FOUND";
    case GWY_SER_NO_NOT_FOUND:
        return "GWY_SER_NO_NOT_FOUND";
    case GWY_SER_NO_INVALID:
        return "GWY_SER_NO_INVALID";
    case LOCATION_NOT_FOUND:
        return "LOCATION_NOT_FOUND";
    case LOCATION_EXCEEDING_RANGE:
        return "LOCATION_EXCEEDING_RANGE";
    case NODE_COMM_TIMEOUT:
        return "NODE_COMM_TIMEOUT";
    case GWY_ALREADY_REG:
        return "GWY_ALREADY_REG";
    case GWY_NOT_REG:
        return "GWY_NOT_REG";
    case GWY_NOT_CONFIGURED_WITH_AC_REMOTE:
        return "GWY_NOT_CONFIGURED_WITH_AC_REMOTE";
    case NODE_SER_NO_NOT_FOUND:
        return "NODE_SER_NO_NOT_FOUND";
    case MAC_ID_NOT_FOUND:
        return "MAC_ID_NOT_FOUND";
    case ELEMENT_ADDR_NOT_FOUND:
        return "ELEMENT_ADDR_NOT_FOUND";
    case NODE_NOT_CONFIGURED_WITH_AC_REMOTE:
        return "NODE_NOT_CONFIGURED_WITH_AC_REMOTE";
    case MAC_ID_CONTAINS_INVALID_CHARS_OR_INVALID_FORMAT:
        return "MAC_ID_CONTAINS_INVALID_CHARS_OR_INVALID_FORMAT";
    case INVALID_MAC_ID_LENGTH:
        return "INVALID_MAC_ID_LENGTH";
    case POWER_NOT_FOUND:
        return "POWER_NOT_FOUND";
    case POWER_EXCEEDING_RANGE:
        return "POWER_EXCEEDING_RANGE";
    case MODE_NOT_FOUND:
        return "MODE_NOT_FOUND";
    case MODE_EXCEEDING_RANGE:
        return "MODE_EXCEEDING_RANGE";
    case FAN_SPEED_NOT_FOUND:
        return "FAN_SPEED_NOT_FOUND";
    case FANSPEED_EXCEEDING_RANGE:
        return "FANSPEED_EXCEEDING_RANGE";
    case TEMPERATURE_NOT_FOUND:
        return "TEMPERATURE_NOT_FOUND";
    case TEMPERATURE_EXCEEDING_RANGE:
        return "TEMPERATURE_EXCEEDING_RANGE";
    case SWING_H_NOT_FOUND:
        return "SWING_H_NOT_FOUND";
    case SWING_H_EXCEEDING_RANGE:
        return "SWING_H_EXCEEDING_RANGE";
    case SWING_V_NOT_FOUND:
        return "SWING_V_NOT_FOUND";
    case SWING_V_EXCEEDING_RANGE:
        return "SWING_V_EXCEEDING_RANGE";
    case ONTIMER_NOT_FOUND:
        return "ONTIMER_NOT_FOUND";
    case ONTIMER_EXCEEDING_RANGE:
        return "ONTIMER_EXCEEDING_RANGE";
    case OFFTIMER_NOT_FOUND:
        return "OFFTIMER_NOT_FOUND";
    case OFFTIMER_EXCEEDING_RANGE:
        return "OFFTIMER_EXCEEDING_RANGE";
    case LOCKING_NOT_FOUND:
        return "LOCKING_NOT_FOUND";
    case LOCKING_EXCEEDING_RANGE:
        return "LOCKING_EXCEEDING_RANGE";
    case TEMP_LOCK_UP_LIMIT_NOT_FOUND:
        return "TEMP_LOCK_UP_LIMIT_NOT_FOUND";
    case TEMP_LOCK_LOW_LIMIT_NOT_FOUND:
        return "TEMP_LOCK_LOW_LIMIT_NOT_FOUND";
    case TEMP_LOCK_UP_LIMIT_EXCEEDS_ABS_TEMP_UP_LIMIT:
        return "TEMP_LOCK_UP_LIMIT_EXCEEDS_ABS_TEMP_UP_LIMIT";
    case TEMP_LOCK_LOW_LIMIT_EXCEEDS_ABS_TEMP_LOW_LIMIT:
        return "TEMP_LOCK_LOW_LIMIT_EXCEEDS_ABS_TEMP_LOW_LIMIT";
    case ILLOGICAL_LOCKING_TEMP_LIMIT:
        return "ILLOGICAL_LOCKING_TEMP_LIMIT";
    case PUBLISH_PERIOD_NOT_FOUND:
        return "PUBLISH_PERIOD_NOT_FOUND";
    case PUBLISH_PERIOD_EXCEEDS_RANGE:
        return "PUBLISH_PERIOD_EXCEEDS_RANGE";
    case RESET_DEVICE_NOT_FOUND:
        return "RESET_DEVICE_NOT_FOUND";
    case LOGGING_FLAG_NOT_FOUND:
        return "LOGGING_FLAG_NOT_FOUND";
    case FORBIDDEN_OPERATION:
        return "FORBIDDEN_OPERATION";
    }
    return "UNKNOWN_ERROR_CODE";
}

/**
 * @brief Function that handles Gwy Registration house keeping things
 * @param none
 * @retval none
 */
void register_gwy()
{
    init_temperature_sensor();
    registered = true;
    eeprom_write_byte(EEPROM_SLAVE_ADDR, REGISTERED_FLAG_FLASH_ADDR, true);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, FACTORY_DEVICE_CHECK_FLASH_ADDR, 0x00);
}

/**
 * @brief Function that gets the JSON ACK NAME to be filled in the ACK sent to cloud
 * @param ack_name Pointer to the String where the ACK NAME needs to be filled in
 * @retval none
 */
void get_json_ack_name(char *ack_name)
{
    switch(json_packet_id)
    {
        case GWY_REG_PACKET:
            strcpy(ack_name, GWY_REG_ACK_NAME);
            break;
        case GWY_UNREG_PACKET:
            strcpy(ack_name, GWY_UNREG_ACK_NAME);
            break;
        case GWY_AC_CONTROL_PACKET:
            strcpy(ack_name, GWY_AC_CONTROL_ACK_NAME);
            break;
        case GWY_RECONF_PACKET:
            strcpy(ack_name, GWY_RECONF_ACK_NAME);
            break;
        case GWY_HEARTBEAT_PUB_CONF_PACKET:
            strcpy(ack_name, GWY_HEARTBEAT_PUB_CONF_ACK_NAME);
            break;
        case GWY_TEACHING_MODE_START_PACKET:
            strcpy(ack_name, GWY_TEACHING_MODE_START_ACK_NAME);
            break;
        case GWY_DEBUG_INFO_PACKET:
            strcpy(ack_name, GWY_DEBUG_INFO_ACK_NAME);
            break;
        case NODE_PROV_PACKET:
            strcpy(ack_name, NODE_PROV_ACK_NAME);
            break;
        case NODE_UNPROV_PACKET:
            strcpy(ack_name, NODE_UNPROV_ACK_NAME);
            break;
        case NODE_RECONF_PACKET:
            strcpy(ack_name, NODE_RECONF_ACK_NAME);
            break;
        case NODE_HEARTBEAT_PUB_CONF_PACKET:
            strcpy(ack_name, NODE_HEARTBEAT_PUB_CONF_ACK_NAME);
            break;
        case NODE_TEACHING_MODE_START_PACKET:
            strcpy(ack_name, NODE_TEACHING_MODE_START_ACK_NAME);
            break;
        case NODE_DEBUG_INFO_PACKET:
            strcpy(ack_name, NODE_DEBUG_INFO_ACK_NAME);
            break;
        default:
            strcpy(ack_name, "INVALID_ACK_NAME");
            break;
    }
}

/**
 * @brief function that takes care of sending back error packet with appropriate error code
 * @retval none
 */
void handle_sending_ack_to_cloud()
{
    char pubmessage[PUBMESG_LEN];
    jwOpen(&jwc, pubmessage, PUBMESG_LEN, JW_OBJECT, 1);
    switch(json_packet_id)
    {
        case NODE_AC_CONTROL_PACKET:
        custom_printf(LTE_DEBUG_TAG, "Sending Node AC Control ACK", CYAN);
        jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_AC_CONTROL_PACKET);
        jwObj_int(&jwc, MSG_SEQ_NO_KEY, node_ac_control_t.base_data.msg_seq_no);
        jwObj_string(&jwc, GWY_SER_NO_KEY, node_ac_control_t.base_data.gwy_ser_no_str);
        jwObj_string(&jwc, NODE_SER_NO_KEY, node_ac_control_t.base_data.node_ser_no_str);
        jwObj_int(&jwc, ERROR_CODE_KEY, json_ack_err_code);
        break;

    case GWY_AC_CONTROL_PACKET:
        custom_printf(LTE_DEBUG_TAG, "Sending Gwy AC Control ACK", CYAN);
        jwObj_int(&jwc, JSON_PACKET_ID_KEY, GWY_AC_CONTROL_PACKET);
        jwObj_string(&jwc, MSG_SEQ_NO_KEY, gwy_ac_control_t.base_data.msg_seq_no);
        jwObj_string(&jwc, GWY_SER_NO_KEY, gwy_ac_control_t.base_data.gwy_ser_no_str);
        jwObj_int(&jwc, ERROR_CODE_KEY, json_ack_err_code);
        break;

    case GWY_DEBUG_INFO_PACKET:
        custom_printf(LTE_DEBUG_TAG, "Sending Gwy Debug Info ACK", CYAN);
        sprintf(gwy_debug_info_t.firmware, "%d.%d.%d", MAJ_VERSION, MIN_VERSION, INTERNAL_MIN_VERSION);
        sprintf(gwy_debug_info_t.uptimestr, "%0.2f", (esp_timer_get_time() / (3600.00 * 1000000.00)));
        jwObj_int(&jwc, JSON_PACKET_ID_KEY, GWY_DEBUG_INFO_PACKET);
        jwObj_int(&jwc, MSG_SEQ_NO_KEY, gwy_debug_info_t.base_data.msg_seq_no);
        jwObj_string(&jwc, GWY_SER_NO_KEY, gwy_debug_info_t.base_data.gwy_ser_no_str);
        jwObj_string(&jwc, FIRMWARE_VERSION_KEY, gwy_debug_info_t.firmware);
        jwObj_int(&jwc, REGISTERED_KEY, registered);
        jwObj_string(&jwc, PROTOCOL_SEL_NUM_KEY, get_protocol_string(protocol_selected_num));
        jwObj_int(&jwc, PUBLISH_PERIOD_KEY, gwy_heartbeat_pub_conf_t.pub_conf_period_in_sec);
        jwObj_int(&jwc, PUBLISH_MESG_QUEUE_COUNT_KEY, get_pubmesg_queue_count(pubmesg_queue_head));
        jwObj_int(&jwc, PROV_QUEUE_COUNT_KEY, get_prov_queue_count(prov_queue_head));
        jwObj_int(&jwc, UNPROV_QUEUE_COUNT_KEY, get_unprov_queue_count(unprov_queue_head));
        jwObj_int(&jwc, AC_CONTROL_QUEUE_COUNT_KEY, get_ac_control_queue_count(node_ac_control_queue_head));
        jwObj_int(&jwc, RECONF_QUEUE_COUNT_KEY, get_reconf_queue_count(node_reconf_queue_head));
        jwObj_int(&jwc, PUB_CONF_QUEUE_COUNT_KEY, get_heartbeat_pub_conf_queue_count(node_pub_conf_queue_head));
        jwObj_int(&jwc, TEACHING_MODE_QUEUE_COUNT_KEY, get_teaching_mode_queue_count(node_teaching_mode_queue_head));
        jwObj_int(&jwc, DEBUG_INFO_QUEUE_COUNT_KEY, get_debug_info_queue_count(node_debug_info_queue_head));
        jwObj_string(&jwc, DEVICE_UPTIME_KEY, gwy_debug_info_t.uptimestr);
        jwObj_int(&jwc, LOGGING_KEY, LOG_DATA);
        jwObj_int(&jwc, RESET_DEVICE_KEY, gwy_debug_info_t.resetDevice);
        jwObj_int(&jwc, ERROR_CODE_KEY, json_ack_err_code);
        jwClose(&jwc);
        add_to_pubmesg_queue(pubmessage, publish_topic);
        /*Only after filling up the ACK message, we must reset the device.*/
        if (gwy_debug_info_t.resetDevice)
            factory_reset_device();
        return;

    case NODE_DEBUG_INFO_PACKET:
        custom_printf(LTE_DEBUG_TAG, "Sending Node Debug Info ACK", CYAN);
        jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_DEBUG_INFO_PACKET);
        jwObj_int(&jwc, MSG_SEQ_NO_KEY, node_debug_info_t.base_data.msg_seq_no);
        jwObj_string(&jwc, GWY_SER_NO_KEY, node_debug_info_t.base_data.gwy_ser_no_str);
        jwObj_string(&jwc, NODE_SER_NO_KEY, node_debug_info_t.base_data.node_ser_no_str);
        jwObj_int(&jwc, ELEMENT_ADDR_KEY, node_debug_info_t.base_data.elementAddr);
        jwObj_string(&jwc, FIRMWARE_VERSION_KEY, node_debug_info_t.firmware);
        jwObj_string(&jwc, PROTOCOL_SEL_NUM_KEY, node_debug_info_t.protocol);
        jwObj_int(&jwc, PUBLISH_PERIOD_KEY, node_heartbeat_pub_conf_t.pub_conf_period_in_sec);
        jwObj_string(&jwc, DEVICE_UPTIME_KEY, node_debug_info_t.uptimestr);
        jwObj_int(&jwc, LOGGING_KEY, node_debug_info_t.logging);
        jwObj_int(&jwc, RESET_DEVICE_KEY, node_debug_info_t.resetDevice);
        jwObj_int(&jwc, ERROR_CODE_KEY, json_ack_err_code);
        break;

    case NODE_TEACHING_MODE_START_PACKET:
        custom_printf(LTE_DEBUG_TAG, "Sending Node Teaching Mode Start ACK", CYAN);
        jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_TEACHING_MODE_START_PACKET);
        jwObj_int(&jwc, MSG_SEQ_NO_KEY, node_teaching_mode_t.base_data.msg_seq_no);
        jwObj_string(&jwc, GWY_SER_NO_KEY, node_teaching_mode_t.base_data.gwy_ser_no_str);
        jwObj_string(&jwc, NODE_SER_NO_KEY, node_teaching_mode_t.base_data.node_ser_no_str);
        jwObj_int(&jwc, ERROR_CODE_KEY, json_ack_err_code);
        break;

    case GWY_TEACHING_MODE_START_PACKET:
        custom_printf(LTE_DEBUG_TAG, "Sending Gwy Teaching Mode Start ACK", CYAN);
        jwObj_int(&jwc, JSON_PACKET_ID_KEY, GWY_TEACHING_MODE_START_PACKET);
        jwObj_string(&jwc, MSG_SEQ_NO_KEY, gwy_teaching_mode_t.base_data.msg_seq_no);
        jwObj_string(&jwc, GWY_SER_NO_KEY, gwy_teaching_mode_t.base_data.gwy_ser_no_str);
        jwObj_int(&jwc, ERROR_CODE_KEY, json_ack_err_code);
        break;

    case NODE_RECONF_PACKET:
        custom_printf(LTE_DEBUG_TAG, "Sending Node Reconfiguration ACK", CYAN);
        jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_RECONF_PACKET);
        jwObj_string(&jwc, MSG_SEQ_NO_KEY, node_reconf_t.base_data.msg_seq_no);
        jwObj_string(&jwc, GWY_SER_NO_KEY, node_reconf_t.base_data.gwy_ser_no_str);
        jwObj_int(&jwc, ERROR_CODE_KEY, json_ack_err_code);
        break;

    case GWY_RECONF_PACKET:
        custom_printf(LTE_DEBUG_TAG, "Sending Gwy Reconfiguration ACK", CYAN);
        jwObj_int(&jwc, JSON_PACKET_ID_KEY, GWY_RECONF_PACKET);
        jwObj_string(&jwc, MSG_SEQ_NO_KEY, gwy_reconf_t.base_data.msg_seq_no);
        jwObj_string(&jwc, GWY_SER_NO_KEY, gwy_reconf_t.base_data.gwy_ser_no_str);
        jwObj_int(&jwc, ERROR_CODE_KEY, json_ack_err_code);
        break;

    case NODE_HEARTBEAT_PUB_CONF_PACKET:
        custom_printf(LTE_DEBUG_TAG, "Sending Node Heartbeat Publish Configuration ACK", CYAN);
        jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_HEARTBEAT_PUB_CONF_PACKET);
        jwObj_int(&jwc, MSG_SEQ_NO_KEY, node_heartbeat_pub_conf_t.base_data.msg_seq_no);
        jwObj_string(&jwc, GWY_SER_NO_KEY, node_heartbeat_pub_conf_t.base_data.gwy_ser_no_str);
        jwObj_string(&jwc, NODE_SER_NO_KEY, node_heartbeat_pub_conf_t.base_data.node_ser_no_str);
        jwObj_int(&jwc, ERROR_CODE_KEY, json_ack_err_code);
        break;

    case GWY_HEARTBEAT_PUB_CONF_PACKET:
        custom_printf(LTE_DEBUG_TAG, "Sending Gwy Heartbeat Publish Confiugration ACK", CYAN);
        jwObj_int(&jwc, JSON_PACKET_ID_KEY, GWY_HEARTBEAT_PUB_CONF_PACKET);
        jwObj_string(&jwc, MSG_SEQ_NO_KEY, gwy_heartbeat_pub_conf_t.base_data.msg_seq_no);
        jwObj_string(&jwc, GWY_SER_NO_KEY, gwy_heartbeat_pub_conf_t.base_data.gwy_ser_no_str);
        jwObj_int(&jwc, ERROR_CODE_KEY, json_ack_err_code);
        break;

    case NODE_PROV_PACKET:
        custom_printf(LTE_DEBUG_TAG, "Sending Node Provision ACK", CYAN);
        jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_PROV_PACKET);
        jwObj_int(&jwc, MSG_SEQ_NO_KEY, provision_t.base_data.msg_seq_no);
        jwObj_string(&jwc, GWY_SER_NO_KEY, provision_t.base_data.gwy_ser_no_str);
        jwObj_string(&jwc, NODE_SER_NO_KEY, provision_t.base_data.node_ser_no_str);
        jwObj_int(&jwc, ERROR_CODE_KEY, json_ack_err_code);
        break;

    case GWY_REG_PACKET:
        custom_printf(LTE_DEBUG_TAG, "Sending Gwy Registration ACK", CYAN);
        jwObj_int(&jwc, JSON_PACKET_ID_KEY, GWY_REG_PACKET);
        jwObj_int(&jwc, MSG_SEQ_NO_KEY, gwy_registration_t.base_data.msg_seq_no);
        jwObj_string(&jwc, GWY_SER_NO_KEY, gwy_registration_t.base_data.gwy_ser_no_str);
        jwObj_int(&jwc, ERROR_CODE_KEY, json_ack_err_code);
        break;

    case NODE_UNPROV_PACKET:
        custom_printf(LTE_DEBUG_TAG, "Sending Node Unprovision ACK", CYAN);
        jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_UNPROV_PACKET);
        jwObj_int(&jwc, MSG_SEQ_NO_KEY, unprovision_t.base_data.msg_seq_no);
        jwObj_string(&jwc, GWY_SER_NO_KEY, unprovision_t.base_data.gwy_ser_no_str);
        jwObj_string(&jwc, NODE_SER_NO_KEY, unprovision_t.base_data.node_ser_no_str);
        jwObj_int(&jwc, ERROR_CODE_KEY, json_ack_err_code);
        break;

    case GWY_UNREG_PACKET:
        custom_printf(LTE_DEBUG_TAG, "Sending Gwy Unregistration ACK", CYAN);
        jwObj_int(&jwc, JSON_PACKET_ID_KEY, GWY_UNREG_PACKET);
        jwObj_int(&jwc, MSG_SEQ_NO_KEY, gwy_unregistration_t.base_data.msg_seq_no);
        jwObj_string(&jwc, GWY_SER_NO_KEY, gwy_unregistration_t.base_data.gwy_ser_no_str);
        jwObj_int(&jwc, ERROR_CODE_KEY, json_ack_err_code);
        if (json_ack_err_code == SUCCESS)
            factory_reset_device();
        break;
    }
    jwClose(&jwc);
    add_to_pubmesg_queue(pubmessage, publish_topic);
}

/**
 * @brief Function that takes care of parsing the received JSON string using cJSON library and stores the information into respective structures
 * @param None
 * @retval None
 */
void parse_json_packet(char *json_packet)
{
    /**
     * First get the json packet
     * convert it to parseable obj using the CJSON_parse function
     * Check if that object is null or not, if not null then continue
     * try to parse JSON_PACKET_ID_KEY from it. if not null and valid, continue
     * then error check other items based on the parsed JSON_PACKET_ID_KEY, if no error, then continue
     * No matter what the error code, ACK needs to be sent back with details
     */
    json_ack_err_code = SUCCESS;
    json_packet_j = cJSON_Parse(json_packet);
    if ((json_packet_j != NULL) && cJSON_GetObjectItem(json_packet_j, JSON_PACKET_ID_KEY))
    {
        json_packet_id = cJSON_GetObjectItem(json_packet_j, JSON_PACKET_ID_KEY)->valueint;
    }
    else
    {
        if (json_packet_j == NULL)
        {
            custom_printf(LTE_DEBUG_TAG, "Invalid JSON Packet", RED);
            json_ack_err_code = JSON_PACKET_INVALID;
            add_to_pubmesg_queue("{\"ErrorCode\" : 48}", publish_topic);
            return;
        }
        else if(!cJSON_GetObjectItem(json_packet_j, JSON_PACKET_ID_KEY))
        {
            custom_printf(LTE_DEBUG_TAG, "Packet ID not found", RED);
            json_ack_err_code = JSON_PACKET_ID_NOT_FOUND;
            add_to_pubmesg_queue("{\"ErrorCode\" : 1}", publish_topic);
            return;
        }
    }

    /**
     * @brief If the parsing of JSON string was successful and there was json_packet_id in it
     * Then the next step is to error check the JSON object before starting to store it into structure members
     */
    if (json_ack_err_code == SUCCESS)
        error_check_json(json_packet_id);

    /**
     * @brief If the above error_check_json function doesn't find any errors in the JSON string,
     * Then it's safe to start storing data into structures.
     */
    if (json_ack_err_code == SUCCESS)
    {
        switch (json_packet_id)
        {
        case NODE_AC_CONTROL_PACKET:
            ESP_LOGI(LTE_DEBUG_TAG, "Node AC Control Packet");
            node_ac_control_t.base_data.request_in_time_us = esp_timer_get_time(); // Note the time as this is being used by queue
            node_ac_control_t.base_data.json_packet_id = json_packet_id;
            node_ac_control_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(node_ac_control_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            strcpy(node_ac_control_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            node_ac_control_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            node_ac_control_t.control.power = cJSON_GetObjectItem(json_packet_j, POWER_KEY)->valueint;
            strcpy(node_ac_control_t.control.mode_str, cJSON_GetObjectItem(json_packet_j, MODE_KEY)->valuestring);
            get_mode_value("node");
            node_ac_control_t.control.fanSpeed = cJSON_GetObjectItem(json_packet_j, FAN_SPEED_KEY)->valueint;
            node_ac_control_t.control.temp = cJSON_GetObjectItem(json_packet_j, TEMPERATURE_KEY)->valueint;
            node_ac_control_t.control.swingH = cJSON_GetObjectItem(json_packet_j, SWING_H_KEY)->valueint;
            node_ac_control_t.control.swingV = cJSON_GetObjectItem(json_packet_j, SWING_V_KEY)->valueint;
            node_ac_control_t.control.OnTimer = cJSON_GetObjectItem(json_packet_j, ONTIMER_KEY)->valueint;
            node_ac_control_t.control.OffTimer = cJSON_GetObjectItem(json_packet_j, OFFTIMER_KEY)->valueint;
            node_ac_control_t.control.Locking = cJSON_GetObjectItem(json_packet_j, AC_LOCKING_KEY)->valueint;
            node_ac_control_t.control.TempLockLowLimit = cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_LOW_LIMIT_KEY)->valueint;
            node_ac_control_t.control.TempLockUpLimit = cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_UP_LIMIT_KEY)->valueint;
            add_to_ac_control_queue();
            break;

        case GWY_AC_CONTROL_PACKET:
            custom_printf(LTE_DEBUG_TAG, "Gwy AC Control Packet", CYAN);
            gwy_ac_control_t.base_data.json_packet_id = json_packet_id;
            gwy_ac_control_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(gwy_ac_control_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            gwy_ac_control_t.control.power = cJSON_GetObjectItem(json_packet_j, POWER_KEY)->valueint;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, POWER_FLASH_ADDR, gwy_ac_control_t.control.power);

            strcpy(gwy_ac_control_t.control.mode_str, cJSON_GetObjectItem(json_packet_j, MODE_KEY)->valuestring);
            get_mode_value("gwy");
            eeprom_write_byte(EEPROM_SLAVE_ADDR, MODE_FLASH_ADDR, gwy_ac_control_t.control.mode_val);

            gwy_ac_control_t.control.fanSpeed = cJSON_GetObjectItem(json_packet_j, FAN_SPEED_KEY)->valueint;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, FAN_FLASH_ADDR, gwy_ac_control_t.control.fanSpeed);

            gwy_ac_control_t.control.temp = cJSON_GetObjectItem(json_packet_j, TEMPERATURE_KEY)->valueint;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, TEMPERATURE_FLASH_ADDR, gwy_ac_control_t.control.temp);

            gwy_ac_control_t.control.swingH = cJSON_GetObjectItem(json_packet_j, SWING_H_KEY)->valueint;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, SWINGH_FLASH_ADDR, gwy_ac_control_t.control.swingH);

            gwy_ac_control_t.control.swingV = cJSON_GetObjectItem(json_packet_j, SWING_V_KEY)->valueint;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, SWINGV_FLASH_ADDR, gwy_ac_control_t.control.swingV);

            gwy_ac_control_t.control.OnTimer = cJSON_GetObjectItem(json_packet_j, ONTIMER_KEY)->valueint;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, ONTIMER_FLASH_ADDR, gwy_ac_control_t.control.OnTimer);

            gwy_ac_control_t.control.OffTimer = cJSON_GetObjectItem(json_packet_j, OFFTIMER_KEY)->valueint;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, OFFTIMER_FLASH_ADDR, gwy_ac_control_t.control.OffTimer);

            gwy_ac_control_t.control.Locking = cJSON_GetObjectItem(json_packet_j, AC_LOCKING_KEY)->valueint;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, LOCKING_FLASH_ADDR, gwy_ac_control_t.control.Locking);

            gwy_ac_control_t.control.TempLockLowLimit = cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_LOW_LIMIT_KEY)->valueint;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, TEMPLOCKLOWLIMIT_FLASH_ADDR, gwy_ac_control_t.control.TempLockLowLimit);

            gwy_ac_control_t.control.TempLockUpLimit = cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_UP_LIMIT_KEY)->valueint;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, TEMPLOCKUPLIMIT_FLASH_ADDR, gwy_ac_control_t.control.TempLockUpLimit);

            needToSendIRComamnd = true;
            break;

        case NODE_DEBUG_INFO_PACKET:
            custom_printf(LTE_DEBUG_TAG, "Node Debug Info Packet", CYAN);
            node_debug_info_t.base_data.request_in_time_us = esp_timer_get_time(); // Note the time as this is being used by queue
            node_debug_info_t.base_data.json_packet_id = json_packet_id;
            node_debug_info_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(node_debug_info_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            strcpy(node_debug_info_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            node_debug_info_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            add_to_debug_info_queue();
            break;

        case GWY_DEBUG_INFO_PACKET:
            custom_printf(LTE_DEBUG_TAG, "Gwy Debug Info Packet", CYAN);
            gwy_debug_info_t.base_data.json_packet_id = json_packet_id;
            strcpy(gwy_debug_info_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            gwy_debug_info_t.resetDevice = cJSON_GetObjectItem(json_packet_j, RESET_DEVICE_KEY)->valueint;
            gwy_debug_info_t.logging = cJSON_GetObjectItem(json_packet_j, LOGGING_KEY)->valueint;
            if (gwy_debug_info_t.logging)
                LOG_DATA = true;
            else
                LOG_DATA = false;
            // The resetDevice is used after sending the ack for this packet
            break;

        case NODE_RECONF_PACKET:
            custom_printf(LTE_DEBUG_TAG, "Node Reconfiguration Packet", CYAN);
            node_reconf_t.base_data.request_in_time_us = esp_timer_get_time(); // Note the time as this is being used by queue
            node_reconf_t.base_data.json_packet_id = json_packet_id;
            node_reconf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(node_reconf_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            strcpy(node_reconf_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            node_reconf_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            add_to_reconf_queue();
            break;

        case GWY_RECONF_PACKET:
            custom_printf(LTE_DEBUG_TAG, "Gwy Reconfiguration Packet", CYAN);
            gwy_reconf_t.base_data.json_packet_id = json_packet_id;
            strcpy(gwy_reconf_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            gwy_reconf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            configured = false;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR, false);
            break;

        case NODE_HEARTBEAT_PUB_CONF_PACKET:
            custom_printf(LTE_DEBUG_TAG, "Node Heartbeat Publish configuration Packet", CYAN);
            node_heartbeat_pub_conf_t.base_data.request_in_time_us = esp_timer_get_time(); // Note the time here as it is being used by queue
            node_heartbeat_pub_conf_t.base_data.json_packet_id = json_packet_id;
            node_heartbeat_pub_conf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(node_heartbeat_pub_conf_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            strcpy(node_heartbeat_pub_conf_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            node_heartbeat_pub_conf_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            node_heartbeat_pub_conf_t.pub_conf_period_in_sec = cJSON_GetObjectItem(json_packet_j, PUBLISH_PERIOD_KEY)->valueint;
            add_to_heartbeat_pub_conf_queue();
            break;

        case GWY_HEARTBEAT_PUB_CONF_PACKET:
            custom_printf(LTE_DEBUG_TAG, "Gwy Heartbeat Publish configuration Packet", CYAN);
            gwy_heartbeat_pub_conf_t.base_data.json_packet_id = json_packet_id;
            strcpy(gwy_heartbeat_pub_conf_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            gwy_heartbeat_pub_conf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            gwy_heartbeat_pub_conf_t.pub_conf_period_in_sec = cJSON_GetObjectItem(json_packet_j, PUBLISH_PERIOD_KEY)->valueint;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, HB_PUB_CONF_PERIOD_ADDR_LO, gwy_heartbeat_pub_conf_t.pub_conf_period_in_sec);
            eeprom_write_byte(EEPROM_SLAVE_ADDR, HB_PUB_CONF_PERIOD_ADDR_HI, gwy_heartbeat_pub_conf_t.pub_conf_period_in_sec>>8);

            delete_Temperature_data_publish_timer();
            create_Temperature_data_publish_timer();
            break;

        case NODE_TEACHING_MODE_START_PACKET:
            custom_printf(LTE_DEBUG_TAG, "Node Teaching Mode Start Packet", CYAN);
            node_teaching_mode_t.base_data.request_in_time_us = esp_timer_get_time(); // Note the time here as it is being used by queue
            node_teaching_mode_t.base_data.json_packet_id = json_packet_id;
            strcpy(node_teaching_mode_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            strcpy(node_teaching_mode_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            node_teaching_mode_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            add_to_teaching_mode_queue();
            break;

        case GWY_TEACHING_MODE_START_PACKET:
            custom_printf(LTE_DEBUG_TAG, "Gwy Teaching Mode Start Packet", CYAN);
            gwy_teaching_mode_t.base_data.json_packet_id = json_packet_id;
            strcpy(gwy_teaching_mode_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            teaching_mode = true;
            teachMode_size_done = true;
            break;

        case NODE_PROV_PACKET:
            custom_printf(LTE_DEBUG_TAG, "Node Provisioning Packet", CYAN);
            provision_t.base_data.request_in_time_us = esp_timer_get_time(); // Note the time here as this is being used by queue
            provision_t.base_data.json_packet_id = json_packet_id;
            provision_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(provision_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            strcpy(provision_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            strcpy(provision_t.location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            fill_macid();
            add_to_prov_queue();
            break;

        case GWY_REG_PACKET:
            custom_printf(LTE_DEBUG_TAG, "Gwy Registration Packet", CYAN);
            gwy_registration_t.base_data.json_packet_id = json_packet_id;
            strcpy(gwy_registration_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            gwy_registration_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(gwy_registration_t.location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            register_gwy();
            break;

        case NODE_UNPROV_PACKET:
            custom_printf(LTE_DEBUG_TAG, "Node Unprovisioning Packet", CYAN);
            unprovision_t.base_data.request_in_time_us = esp_timer_get_time(); // Note the time here as this is being used by queue
            unprovision_t.base_data.json_packet_id = json_packet_id;
            unprovision_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(unprovision_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            strcpy(unprovision_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            strcpy(unprovision_t.location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            unprovision_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            add_to_unprov_queue();
            break;

        case GWY_UNREG_PACKET:
            custom_printf(LTE_DEBUG_TAG, "Gwy Unregistration Packet", CYAN);
            gwy_unregistration_t.base_data.json_packet_id = json_packet_id;
            strcpy(gwy_unregistration_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            gwy_unregistration_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(gwy_unregistration_t.location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            // factory resetting after receiving this packet is taken care after sending out Unregistration ACK
            break;

        case GWY_OTA_UPDATE:
            custom_printf(LTE_DEBUG_TAG, "Gwy OTA Update Packet", CYAN);
            gwy_ota_t.base_data.json_packet_id = json_packet_id;
            gwy_ota_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(gwy_ota_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            strcpy(gwy_ota_t.link, cJSON_GetObjectItem(json_packet_j, LINK_KEY)->valuestring);
            ota_update();
            break;
        }
    }
    if (json_ack_err_code == JSON_PACKET_ID_UNKNOWN)
    {
        add_to_pubmesg_queue("{\"ErrorCode\" : 2}", publish_topic);
        return;
    };
    sprintf(lte_log_buffer, "Error Code : %s", get_err_string(json_ack_err_code));
    custom_printf(LTE_ERROR_TAG, lte_log_buffer, RED);

    // Handle sending back ACK for gwy related packets here &
    // Handle sending back ACK for node related packets here only if the packet contains errors
    // Else it is taken care at the mesh side.
    if((json_packet_id >= 0 && json_packet_id < MAX_GWY_PACKET_ID-1 ) || 
      (json_packet_id >= 100 && json_packet_id <= MAX_NODE_PACKET_ID-1 && json_ack_err_code !=SUCCESS))
        handle_sending_ack_to_cloud();
}

#endif