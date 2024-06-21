/**
 * @file Cloud.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to MQTT packet handling and Sending Acknowledgements
 * @version 0.8
 * @date 2024-04-16
 * @copyright Copyright (c) 2024
 */

#include "../../inc/LTE/LTE.h"

#if (IS_GWY)

// Initialization
int16_t json_ack_err_code = SUCCESS;
uint8_t json_packet_id = UNKNOWN_PACKET;
char json_packet[MQTT_PACKET_BUFF_SIZE];
cJSON *json_packet_j;

struct pub_mesg_struct *pubmesg_queue_head = NULL;
struct pub_mesg_struct *pubmesg_queue_tail = NULL;

/**
 * @brief Function that initializes the members of global strucutres with
 * values that will never changes
 * For examples, GWY SER NO is never going to change
 * For example, the JSON PACKET ID is never going to change
 * So it's better to initialize them with values at the start of application
 * @param none
 * @retval none
 */
void init_structures()
{
    /* GWY SER NO STRING */
    strcpy(gwy_registration_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_unregistration_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_conf_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_reconf_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_ac_control_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_locking_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_reset_mqtt_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_pub_conf_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_heartbeat_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_pub_conf_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_teaching_mode_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_debug_info_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);

    /* GWY - JSON PACKET IDs */
    gwy_registration_t.base_data.json_packet_id = GWY_REG_PACKET;
    gwy_unregistration_t.base_data.json_packet_id = GWY_UNREG_PACKET;
    gwy_conf_t.base_data.json_packet_id = GWY_CONF_ACK;
    gwy_reconf_t.base_data.json_packet_id = GWY_RECONF_PACKET;
    gwy_ac_control_t.base_data.json_packet_id = GWY_AC_CONTROL_PACKET;
    gwy_locking_t.base_data.json_packet_id = GWY_MANUAL_AC_CONTROL_ACK;
    gwy_reset_mqtt_t.base_data.json_packet_id = RESET_MQTT;
    gwy_pub_conf_t.base_data.json_packet_id = GWY_HEARTBEAT_PUB_CONF_PACKET;
    gwy_heartbeat_t.base_data.json_packet_id = GWY_HEARTBEAT_ACK;
    gwy_teaching_mode_t.base_data.json_packet_id = GWY_TEACHING_MODE_START_PACKET;
    gwy_debug_info_t.base_data.json_packet_id = GWY_DEBUG_INFO_PACKET;

    /* NODE - JSON PACKET IDs */
    provision_t.base_data.json_packet_id = NODE_PROV_PACKET;
    unprovision_t.base_data.json_packet_id = NODE_UNPROV_PACKET;
    node_conf_t.base_data.json_packet_id = NODE_CONF_PACKET;
    node_reconf_t.base_data.json_packet_id = NODE_RECONF_PACKET;
    node_ac_control_t.base_data.json_packet_id = NODE_AC_CONTROL_PACKET;
    node_locking_t.base_data.json_packet_id = NODE_MANUAL_AC_CONTROL_ACK;
    node_pub_conf_t.base_data.json_packet_id = NODE_HEARTBEAT_PUB_CONF_PACKET;
    node_heartbeat_t.base_data.json_packet_id = NODE_HEARTBEAT_ACK;
    node_debug_info_t.base_data.json_packet_id = NODE_DEBUG_INFO_PACKET;

    /* JSON ACK NAMES */
    strcpy(gwy_registration_t.base_data.ack_name, GWY_REG_ACK_NAME);
    strcpy(gwy_unregistration_t.base_data.ack_name, GWY_UNREG_ACK_NAME);
    strcpy(gwy_conf_t.base_data.ack_name, GWY_CONF_ACK_NAME);
    strcpy(gwy_reconf_t.base_data.ack_name, GWY_RECONF_ACK_NAME);
    strcpy(gwy_ac_control_t.base_data.ack_name, GWY_AC_CONTROL_ACK_NAME);
    strcpy(gwy_locking_t.base_data.ack_name, GWY_MANUAL_AC_CONTROL_ACK_NAME);
    strcpy(gwy_reset_mqtt_t.base_data.ack_name, GWY_RESET_MQTT_ACK_NAME);
    strcpy(gwy_pub_conf_t.base_data.ack_name, GWY_HEARTBEAT_PUB_CONF_ACK_NAME);
    strcpy(gwy_heartbeat_t.base_data.ack_name, GWY_HEARTBEAT_ACK_NAME);
    strcpy(gwy_debug_info_t.base_data.ack_name, GWY_DEBUG_INFO_ACK_NAME);
}

void fill_macid()
{
    char macid[17];
    strcpy(macid, cJSON_GetObjectItem(json_packet_j, MAC_ID_KEY)->valuestring);
    printf("MAC ID recvd from cloud: %s\n", macid);
    char hex_char_str[2];
    for (uint8_t index = 0, i = 0; index < 6; index++, i += 3)
    {
        strncat(hex_char_str, &macid[i], 1);
        strncat(hex_char_str, &macid[i + 1], 1);
        provision_t.macid[index] = strtol(hex_char_str, NULL, 16);
        strcpy(hex_char_str, "");
        printf("provision_t.macid[%d] : %x\n", index, provision_t.macid[index]);
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
    if(strlen(macid) != 17) {
        json_ack_err_code = INVALID_MAC_ID_LENGTH;
    }
    for(uint8_t i=0;i<17;i++)
    {
        if((macid[i] >= '0' && macid[i]<='9') ||
          (macid[i] >= 'a' && macid[i] <= 'f') ||
          (macid[i] >= 'A' && macid[i] <= 'F')) d++;
        else if((i==2 || i==5 || i==8 || i==11 || i==14) && macid[i] == ':')
            s++;
    }
    if(d==12 && s==5);
    else {
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
    ESP_LOGI(LTE_DEBUG_TAG, "Error checking received packet ... ");
    /* Common in all Packets */
    /*
        Json packet id should be between (0 and 10) or (100 and 110) or be 99.
    */
    if((json_packet_id >= 0 && json_packet_id <= 10) || json_packet_id == 99 || (json_packet_id >= 100 && json_packet_id <= 110));
    else {
        json_ack_err_code = JSON_PACKET_ID_UNKNOWN;
        return;
    }
    if (cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY));
    else {
        json_ack_err_code = MSG_SEQ_NO_NOT_FOUND;
        return;
    }
    if (cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY))
    {
        char gwysernostr[15];
        strcpy(gwysernostr, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
        if(!strcmp(gwysernostr, GWY_SER_NO_IN_STRING));
        else {
            json_ack_err_code = GWY_SER_NO_NOT_MATCHING;
            return;
        }
    }
    else {
        json_ack_err_code = GWY_SER_NO_NOT_FOUND;
        return;
    }

    /* Registration check */
    switch(json_packet_id)
    {
        case GWY_UNREG_PACKET:
        case GWY_AC_CONTROL_PACKET:
        case GWY_HEARTBEAT_PUB_CONF_PACKET:
        case GWY_RECONF_PACKET:
        case NODE_PROV_PACKET:
        case NODE_UNPROV_PACKET:
        case NODE_HEARTBEAT_PUB_CONF_PACKET:
        case NODE_RECONF_PACKET:
            if(!registered)
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
            if(!configured)
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
            if(cJSON_GetObjectItem(json_packet_j, NODE_SER_NO));
            else {
                json_ack_err_code = NODE_SER_NO_NOT_FOUND;
                return;
            }
            if(json_packet!=NODE_PROV_PACKET && cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY));
            else {
                json_ack_err_code = ELEMENT_ADDR_NOT_FOUND;
            }
    }

    switch (json_packet_id)
    {
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
            if(strlen(location) > LOCATION_STR_LEN) {
                json_ack_err_code = LOCATION_EXCEEDING_RANGE;
                return;
            }
        }
        else {
            json_ack_err_code = LOCATION_NOT_FOUND;
            return;
        }
        return;

    case GWY_AC_CONTROL_PACKET:
    case NODE_AC_CONTROL_PACKET:
        if (cJSON_GetObjectItem(json_packet_j, POWER_KEY))
        {
            uint8_t power = cJSON_GetObjectItem(json_packet_j, POWER_KEY)->valueint;
            if(power!=0 && power!=1) json_ack_err_code = POWER_EXCEEDING_RANGE;
        }
        else {
            json_ack_err_code = POWER_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, MODE_KEY))
        {
            char mode[10];
            strcpy(mode, cJSON_GetObjectItem(json_packet_j, MODE_KEY)->valuestring);
            if(!strcasecmp(mode, "Cool") ||
               !strcasecmp(mode, "Hot")  || 
               !strcasecmp(mode, "Auto") ||
               !strcasecmp(mode, "Dry")  ||
               !strcasecmp(mode, "Fan"));
            else {
                json_ack_err_code = MODE_EXCEEDING_RANGE;
            }
        }
        else {
            json_ack_err_code = MODE_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, FAN_SPEED_KEY))
        {
            uint8_t fanspeed = cJSON_GetObjectItem(json_packet_j, FAN_SPEED_KEY)->valueint;
            if(fanspeed >= 0 && fanspeed <= 5);
            else {
                json_ack_err_code = FANSPEED_EXCEEDING_RANGE;
                return;
            }
        }
        else {
            json_ack_err_code = FAN_SPEED_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, TEMPERATURE_KEY))
        {
            uint8_t temperature = cJSON_GetObjectItem(json_packet_j, TEMPERATURE_KEY)->valueint;
            if (temperature >= TEMP_ABS_LOW_LIMIT && temperature <= TEMP_ABS_UP_LIMIT);
            else {
                json_ack_err_code = TEMPERATURE_EXCEEDING_RANGE;
                return;
            }
        }
        else {
            json_ack_err_code = TEMPERATURE_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, SWING_H_KEY))
        {
            uint8_t swing = cJSON_GetObjectItem(json_packet_j, SWING_H_KEY)->valueint;
            if(swing!=0 && swing!=1) {
                json_ack_err_code = SWING_H_EXCEEDING_RANGE;
                return;
            }
        }
        else {
            json_ack_err_code = SWING_H_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, SWING_V_KEY))
        {
            uint8_t swing = cJSON_GetObjectItem(json_packet_j, SWING_V_KEY)->valueint;
            if(swing!=0 && swing!=1) {
                json_ack_err_code = SWING_V_EXCEEDING_RANGE;
                return;
            }
        }
        else {
            json_ack_err_code = SWING_V_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, ONTIMER_KEY))
        {
            uint8_t timer = cJSON_GetObjectItem(json_packet_j, ONTIMER_KEY)->valueint;
            if(timer >= 0 && timer <= 12);
            else {
                json_ack_err_code = ONTIMER_EXCEEDING_RANGE;
            }
        }
        else {
            json_ack_err_code = ONTIMER_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, OFFTIMER_KEY))
        {
            uint8_t timer = cJSON_GetObjectItem(json_packet_j, OFFTIMER_KEY)->valueint;
            if(timer >= 0 && timer <= 12);
            else {
                json_ack_err_code = OFFTIMER_EXCEEDING_RANGE;
            }
        }
        else {
            json_ack_err_code = OFFTIMER_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, AC_LOCKING_KEY))
        {
            uint8_t locking = cJSON_GetObjectItem(json_packet_j, AC_LOCKING_KEY)->valueint;
            if(locking != 0 && locking !=1) {
                json_ack_err_code = LOCKING_EXCEEDING_RANGE;
            }
        }
        else {
            json_ack_err_code = LOCKING_NOT_FOUND;
            return;
        }
        uint8_t tempLockUpLimit, tempLockLowLimit;
        if (cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_UP_LIMIT_KEY))
        {
            tempLockUpLimit = (cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_UP_LIMIT_KEY))->valueint;
            if (tempLockUpLimit > TEMP_ABS_UP_LIMIT)
            {
                json_ack_err_code = TEMP_LOCK_UP_LIMIT_EXCEEDS_ABS_TEMP_UP_LIMIT;
                return;
            }
        }
        else {
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
        else {
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
            if(strlen(location) > LOCATION_STR_LEN) {
                json_ack_err_code = LOCATION_EXCEEDING_RANGE;
                return;
            }
        }
        else {
            json_ack_err_code = LOCATION_NOT_FOUND;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, MAC_ID_KEY))
        {
            char macid[20];
            strcpy(macid, cJSON_GetObjectItem(json_packet_j, MAC_ID_KEY)->valuestring);
            isValidMacId(macid);
        }
        else {
            json_ack_err_code = MAC_ID_NOT_FOUND;
            return;
        }
        return;

    case GWY_HEARTBEAT_PUB_CONF_PACKET:
    case NODE_HEARTBEAT_PUB_CONF_PACKET:
        if (cJSON_GetObjectItem(json_packet_j, PUBLISH_PERIOD_KEY))
        {
            uint8_t PublishPeriodSec = cJSON_GetObjectItem(json_packet_j, PUBLISH_PERIOD_KEY)->valueint;
            if (PublishPeriodSec >= 10 && PublishPeriodSec <= 255);
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

    case RESET_MQTT:
        json_ack_err_code = FORBIDDEN_OPERATION;
        return;
    }
}

/**
 * @brief Function that fills the message that needs to be sent as ack to cloud
 * handles only the Gwy part. Ack for the Node part is being handled at the Gwy_mesh_main.c
 * @param json_id JSON PACKET ID
 * @retval none
 */
void handle_sending_ack_to_cloud(uint8_t json_id)
{
    char pubmessage[PUBMESG_LEN];
    switch (json_id)
    {
    case GWY_REG_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy Reg Ack");
        cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
        sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %s, %s : %s, %s : %d}",
                JSON_PACKET_ID_KEY, GWY_REG_PACKET,
                JSON_ACK_NAME_KEY, GWY_REG_ACK_NAME,
                MSG_SEQ_NO_KEY, gwy_registration_t.base_data.msg_seq_no,
                GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
                LOCATION_KEY, gwy_registration_t.base_data.location,
                ERROR_CODE_KEY, json_ack_err_code);
        add_to_pubmesg_queue(pubmessage, publish_topic);
        break;

    case GWY_UNREG_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy Unreg Ack");
        cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
        sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %s, %s : %s, %s : %d}",
                JSON_PACKET_ID_KEY, GWY_UNREG_PACKET,
                JSON_ACK_NAME_KEY, GWY_UNREG_ACK_NAME,
                MSG_SEQ_NO_KEY, gwy_unregistration_t.base_data.msg_seq_no,
                GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
                LOCATION_KEY, gwy_unregistration_t.base_data.location,
                ERROR_CODE_KEY, json_ack_err_code);
        add_to_pubmesg_queue(pubmessage, publish_topic);
        break;

    case GWY_AC_CONTROL_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy AC Control Ack");
        cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
        sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %s, %s : %d, %s : %s, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d}",
                JSON_PACKET_ID_KEY, GWY_AC_CONTROL_PACKET,
                JSON_ACK_NAME_KEY, GWY_AC_CONTROL_ACK_NAME,
                MSG_SEQ_NO_KEY, gwy_ac_control_t.base_data.msg_seq_no,
                GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
                POWER_KEY, gwy_ac_control_t.control.power,
                MODE_KEY, gwy_ac_control_t.control.mode_str,
                FAN_SPEED_KEY, gwy_ac_control_t.control.fan,
                TEMPERATURE_KEY, gwy_ac_control_t.control.temp,
                SWING_H_KEY, gwy_ac_control_t.control.swingH,
                SWING_V_KEY, gwy_ac_control_t.control.swingV,
                ONTIMER_KEY, gwy_ac_control_t.control.OnTimer,
                OFFTIMER_KEY, gwy_ac_control_t.control.OffTimer,
                AC_LOCKING_KEY, gwy_ac_control_t.control.Locking,
                ERROR_CODE_KEY, json_ack_err_code);
        add_to_pubmesg_queue(pubmessage, publish_topic);
        break;

    case GWY_RECONF_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy Reconf Ack");
        cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
        sprintf(pubmessage, "{%s : %d, %s : %ss, %s : %d, %s : %s, %s : %d}",
                JSON_PACKET_ID_KEY, GWY_RECONF_PACKET,
                JSON_ACK_NAME_KEY, GWY_RECONF_ACK_NAME,
                MSG_SEQ_NO_KEY, gwy_reconf_t.base_data.msg_seq_no,
                GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
                ERROR_CODE_KEY, json_ack_err_code);
        add_to_pubmesg_queue(pubmessage, publish_topic);
        break;

    case GWY_MANUAL_AC_CONTROL_ACK:
        sprintf(lte_log_buffer, "Sending Gwy Manual AC control Ack");
        cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
        sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %s, %s : %d, %s : %s, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d}",
                JSON_PACKET_ID_KEY, GWY_MANUAL_AC_CONTROL_ACK,
                JSON_ACK_NAME_KEY, GWY_MANUAL_AC_CONTROL_ACK_NAME,
                MSG_SEQ_NO_KEY, gwy_locking_t.base_data.msg_seq_no,
                GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
                POWER_KEY, gwy_locking_t.power,
                MODE_KEY, gwy_locking_t.mode_str,
                FAN_SPEED_KEY, gwy_locking_t.fan,
                TEMPERATURE_KEY, gwy_locking_t.temp,
                SWING_H_KEY, gwy_locking_t.swingH,
                SWING_V_KEY, gwy_locking_t.swingV,
                ONTIMER_KEY, gwy_locking_t.OnTimer,
                OFFTIMER_KEY, gwy_locking_t.OffTimer,
                AC_LOCKING_KEY, gwy_locking_t.Locking,
                ERROR_CODE_KEY, gwy_locking_t.base_data.error_code);
        add_to_pubmesg_queue(pubmessage, publish_topic);
        break;

    case GWY_HEARTBEAT_PUB_CONF_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy Pub conf Ack");
        cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
        sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %s, %s : %d, %s : %d}",
                JSON_PACKET_ID_KEY, GWY_HEARTBEAT_PUB_CONF_PACKET,
                JSON_ACK_NAME_KEY, GWY_HEARTBEAT_PUB_CONF_ACK_NAME,
                MSG_SEQ_NO_KEY, gwy_pub_conf_t.base_data.msg_seq_no,
                GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
                PUBLISH_PERIOD_KEY, gwy_pub_conf_t.pub_conf_period_in_sec,
                ERROR_CODE_KEY, json_ack_err_code);
        add_to_pubmesg_queue(pubmessage, publish_topic);
        break;

    case RESET_MQTT:
        sprintf(lte_log_buffer, "Sending Gwy Reset MQTT Ack");
        cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
        sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %s, %s : %d}",
                JSON_PACKET_ID_KEY, RESET_MQTT,
                JSON_ACK_NAME_KEY, GWY_RESET_MQTT_ACK_NAME,
                MSG_SEQ_NO_KEY, gwy_reset_mqtt_t.base_data.msg_seq_no,
                GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
                ERROR_CODE_KEY, json_ack_err_code);
        add_to_pubmesg_queue(pubmessage, publish_topic);
        break;
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
        else if (strcasecmp(gwy_ac_control_t.control.mode_str, "Heat") == 0)
            gwy_ac_control_t.control.mode_val = HEAT;
        else if (strcasecmp(gwy_ac_control_t.control.mode_str, "Fan") == 0)
            gwy_ac_control_t.control.mode_val = FAN;
    }
    else
    {
        if (strcasecmp(node_ac_control_t.mode_str, "Auto") == 0)
            gwy_ac_control_t.control.mode_val = AUTO;
        else if (strcasecmp(node_ac_control_t.mode_str, "Cool") == 0)
            gwy_ac_control_t.control.mode_val = COOL;
        else if (strcasecmp(node_ac_control_t.mode_str, "Dry") == 0)
            gwy_ac_control_t.control.mode_val = DRY;
        else if (strcasecmp(node_ac_control_t.mode_str, "Heat") == 0)
            gwy_ac_control_t.control.mode_val = HEAT;
        else if (strcasecmp(node_ac_control_t.mode_str, "Fan") == 0)
            gwy_ac_control_t.control.mode_val = FAN;
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
    case GWY_SER_NO_NOT_MATCHING:
        return "GWY_SER_NO_NOT_MATCHING";
    case LOCATION_NOT_FOUND:
        return "LOCATION_NOT_FOUND";
    case LOCATION_EXCEEDING_RANGE:
        return "LOCATION_EXCEEDING_RANGE";
    case NODE_COMM_TIMEOUT:
        return "NODE_COMM_TIMEOUT";
    case GWY_ALREADY_REG:
        return "GWY_ALREADY_REG";
    case NODE_ALREADY_PROV:
        return "NODE_ALREADY_PROV";
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
    case FORBIDDEN_OPERATION:
        return "FORBIDDEN_OPERATION";
    }
    return "UNKNOWN_ERROR_CODE";
}

/**
 * @brief parses the control packet recvd from MQTT and stores it in the control strucutre
 * @param None
 * @retval Error code
 */
void parse_json_packet(char *json_packet)
{
    /**
     * First get the json packet
     * convert it to parseable obj using the CJSON_parse function
     * Check if that object is null or not, if not null then continue
     * try to parse JSON_PACKET_ID_KEY from it. if not null and valid, continue
     * then error check other items based on the parsed JSON_PACKET_ID_KEY, if no error, then continue
     * No matter what the error code, ack needs to be sent back with details
     */
    json_ack_err_code = SUCCESS;
    json_packet_j = cJSON_Parse(json_packet);
    if ((json_packet_j != NULL) && cJSON_GetObjectItem(json_packet_j, JSON_PACKET_ID_KEY))
    {
        json_packet_id = cJSON_GetObjectItem(json_packet_j, JSON_PACKET_ID_KEY)->valueint;
    }
    else
    {
        json_ack_err_code = JSON_PACKET_ID_NOT_FOUND;
    }

    if(json_ack_err_code == SUCCESS) error_check_json(json_packet_id);

    // Pass through only if the recvd packet contains no error
    if (json_ack_err_code == SUCCESS)
    {
        switch (json_packet_id)
        {
        case GWY_REG_PACKET:
            cyan_printf(LTE_DEBUG_TAG, "Gwy Registration packet");
            gwy_registration_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(gwy_registration_t.base_data.location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            registered = true;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, REGISTERED_FLAG_FLASH_ADDR, false); // Logic is inverted in Flash. That's why we're writing false here
            vTaskDelay(pdMS_TO_TICKS(5));

        case GWY_AC_CONTROL_PACKET:
            gwy_ac_control_t.control.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            gwy_ac_control_t.control.power = cJSON_GetObjectItem(json_packet_j, POWER_KEY)->valueint;
            strcpy(gwy_ac_control_t.control.mode_str, cJSON_GetObjectItem(json_packet_j, MODE_KEY)->valuestring);
            get_mode_value("gwy");
            gwy_ac_control_t.control.fan = cJSON_GetObjectItem(json_packet_j, FAN_SPEED_KEY)->valueint;
            gwy_ac_control_t.control.temp = cJSON_GetObjectItem(json_packet_j, TEMPERATURE_KEY)->valueint;
            gwy_ac_control_t.control.swingH = cJSON_GetObjectItem(json_packet_j, SWING_H_KEY)->valueint;
            gwy_ac_control_t.control.swingV = cJSON_GetObjectItem(json_packet_j, SWING_V_KEY)->valueint;
            gwy_ac_control_t.control.OnTimer = cJSON_GetObjectItem(json_packet_j, ONTIMER_KEY)->valueint;
            gwy_ac_control_t.control.OffTimer = cJSON_GetObjectItem(json_packet_j, OFFTIMER_KEY)->valueint;
            gwy_ac_control_t.control.Locking = cJSON_GetObjectItem(json_packet_j, AC_LOCKING_KEY)->valueint;
            gwy_ac_control_t.control.TempLockLowLimit = cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_LOW_LIMIT_KEY)->valueint;
            gwy_ac_control_t.control.TempLockUpLimit = cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_UP_LIMIT_KEY)->valueint;
            needToSendIRComamnd = true;
            break;

        case NODE_AC_CONTROL_PACKET:
            cyan_printf(LTE_DEBUG_TAG, "Node AC Control packet");
            node_ac_control_t.base_data.request_in_time_us = esp_timer_get_time();
            node_ac_control_t.base_data.json_packet_id = json_packet_id;
            node_ac_control_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(node_ac_control_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            node_ac_control_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            node_ac_control_t.power = cJSON_GetObjectItem(json_packet_j, POWER_KEY)->valueint;
            strcpy(node_ac_control_t.mode_str, cJSON_GetObjectItem(json_packet_j, MODE_KEY)->valuestring);
            get_mode_value("node");
            node_ac_control_t.fan = cJSON_GetObjectItem(json_packet_j, FAN_SPEED_KEY)->valueint;
            node_ac_control_t.temp = cJSON_GetObjectItem(json_packet_j, TEMPERATURE_KEY)->valueint;
            node_ac_control_t.swingH = cJSON_GetObjectItem(json_packet_j, SWING_H_KEY)->valueint;
            node_ac_control_t.swingV = cJSON_GetObjectItem(json_packet_j, SWING_V_KEY)->valueint;
            node_ac_control_t.OnTimer = cJSON_GetObjectItem(json_packet_j, ONTIMER_KEY)->valueint;
            node_ac_control_t.OffTimer = cJSON_GetObjectItem(json_packet_j, OFFTIMER_KEY)->valueint;
            node_ac_control_t.Locking = cJSON_GetObjectItem(json_packet_j, AC_LOCKING_KEY)->valueint;
            node_ac_control_t.TempLockLowLimit = cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_LOW_LIMIT_KEY)->valueint;
            node_ac_control_t.TempLockUpLimit = cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_UP_LIMIT_KEY)->valueint;
            add_to_node_control_queue();
            break;
        
        case NODE_DEBUG_INFO_PACKET:
            cyan_printf(LTE_DEBUG_TAG, "Node Debug Info Packet");

            break;

        case GWY_DEBUG_INFO_PACKET:
            cyan_printf(LTE_DEBUG_TAG, "Gwy Debug Info Packet");
            break;

        case GWY_TEACHING_MODE_START_PACKET:
            cyan_printf(LTE_DEBUG_TAG, "Gwy Teaching Mode Start Packet");
            teaching_mode = true;
            teachMode_size_done = true;
            break;

        case GWY_RECONF_PACKET:
            cyan_printf(LTE_DEBUG_TAG, "Gwy Reconfiguration packet");
            gwy_reconf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            configured = false;
            break;

        case GWY_HEARTBEAT_PUB_CONF_PACKET:
            cyan_printf(LTE_DEBUG_TAG, "Gwy Publish configuration packet");
            gwy_pub_conf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            gwy_pub_conf_t.pub_conf_period_in_sec = cJSON_GetObjectItem(json_packet_j, PUBLISH_PERIOD_KEY)->valueint;
            delete_Temperature_data_publish_timer();
            create_Temperature_data_publish_timer();
            break;

        case NODE_PROV_PACKET:
            cyan_printf(LTE_DEBUG_TAG, "Node Provisioning packet");
            provision_t.base_data.request_in_time_us = esp_timer_get_time();
            provision_t.base_data.json_packet_id = json_packet_id;
            provision_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(provision_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            strcpy(provision_t.base_data.location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            fill_macid();
            add_to_prov_queue();
            break;

        case NODE_TEACHING_MODE_START_PACKET:
            cyan_printf(LTE_DEBUG_TAG, "Node Teaching Mode Start Packet");
            strcpy(node_teaching_mode_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            add_to_node_teaching_mode_queue();
            break;

        case NODE_RECONF_PACKET:
            cyan_printf(LTE_DEBUG_TAG, "Node Reconfiguration packet");
            node_reconf_t.base_data.request_in_time_us = esp_timer_get_time();
            node_reconf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(node_reconf_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            node_reconf_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            add_to_node_reconf_queue();
            break;

        case NODE_HEARTBEAT_PUB_CONF_PACKET:
            cyan_printf(LTE_DEBUG_TAG, "Node Publish configuratoin packet received");
            node_pub_conf_t.base_data.request_in_time_us = esp_timer_get_time();
            node_pub_conf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(node_pub_conf_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            node_pub_conf_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            node_pub_conf_t.pub_conf_period_in_sec = cJSON_GetObjectItem(json_packet_j, PUBLISH_PERIOD_KEY)->valueint;
            add_to_node_pub_conf_queue();
            break;

        case NODE_UNPROV_PACKET:
            cyan_printf(LTE_DEBUG_TAG, "Node Unprovisioning packet");
            unprovision_t.base_data.request_in_time_us = esp_timer_get_time();
            unprovision_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(unprovision_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            unprovision_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            add_to_unprov_queue();
            break;

        case GWY_UNREG_PACKET:
        //We still need to take care of erasing data and resetting the device back to factory settings upon receving this packet.
            cyan_printf(LTE_DEBUG_TAG, "Gwy Unregistration packet");
            gwy_unregistration_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(gwy_unregistration_t.base_data.location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            registered = false;
            configured = false;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, REGISTERED_FLAG_FLASH_ADDR, true); // Logic is inverted in Flash.
            vTaskDelay(pdMS_TO_TICKS(5));
            eeprom_write_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR, true); // Logic is inverted in Flash.
            vTaskDelay(pdMS_TO_TICKS(5));
            eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR, 0);
            vTaskDelay(pdMS_TO_TICKS(5));
            eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR + 1, 0);
            vTaskDelay(pdMS_TO_TICKS(5));
            break;
        }

        sprintf(lte_log_buffer, "Error Code : %s", get_err_string(json_ack_err_code));
        cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
    }
    handle_sending_ack_to_cloud(json_packet_id);
}

#endif