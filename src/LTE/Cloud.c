/**
 * @file Cloud.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to MQTT packet handling and Sending Acknowledgements
 * @version 0.8
 * @date 2024-04-16
 * @copyright Copyright (c) 2024
 */

#include "../../inc/LTE/LTE.h"

// Initialization
int32_t json_ack_err_code = SUCCESS;
int32_t json_packet_id = UNKNOWN_PACKET;
char json_packet[MQTT_PACKET_BUFF_SIZE];
cJSON *json_packet_j;

struct pub_mesg_struct *pubmesg_queue_head = NULL;
struct pub_mesg_struct *pubmesg_queue_tail = NULL;

/**
 * @brief Function that returns the Mode string based on Mode value
 * @param mode_value 
 * @return char* String that denotes what mode it is.
 */
char* get_mode_string(uint8_t mode_value)
{
    switch(mode_value)
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


/**
 * @brief Function that takes care of erasing the data in device and set it up as factory device
 * Maybe we can implement some security check before going on to clear data. Also, resetting Device
 * for now doesn't erase the MQTT settings. Need to discuss on this later.
 * @param none
 * @retval none
 */
void factory_reset_device()
{
    ESP_LOGI(MAIN_DEBUG_TAG, "Factory resetting device !!!!!!!!!!!!!!!!!!!!!!");

    registered = false;
    protocol_selected_num = -1;
    configured = false;

    //Factory Device 
    eeprom_write_byte(EEPROM_SLAVE_ADDR, FACTORY_DEVICE_CHECK_FLASH_ADDR, 0xFF);

    //Serial Number
    eeprom_write_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_HI, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_MID, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_LO, 0);

#if (IS_GWY)
    //Registered
    eeprom_write_byte(EEPROM_SLAVE_ADDR, REGISTERED_FLAG_FLASH_ADDR, 0);
#endif

    //Configured
    eeprom_write_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR, 0);
    
    //Protocol Selected Number
    eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR_HI, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR_LO, 0);

    eeprom_write_byte(EEPROM_SLAVE_ADDR, RAWLEN_ADDR_HI, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, RAWLEN_ADDR_LO, 0);

    //Publish Period
    eeprom_write_byte(EEPROM_SLAVE_ADDR, HB_PUB_CONF_PERIOD_ADDR, DEFAULT_HEARTBEAT_PUB_CONF_PERIOD_SEC);
    gwy_pub_conf_t.pub_conf_period_in_sec = DEFAULT_HEARTBEAT_PUB_CONF_PERIOD_SEC;
    node_heartbeat_pub_conf_t.pub_conf_period_in_sec = DEFAULT_HEARTBEAT_PUB_CONF_PERIOD_SEC;

    //AC Control Settings
    eeprom_write_byte(EEPROM_SLAVE_ADDR, POWER_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, MODE_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, FAN_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, TEMPERATURE_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, SWINGH_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, SWINGV_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, LOCKING_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, TEMPLOCKLOWLIMIT_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, TEMPLOCKLOWLIMIT_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, ONTIMER_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, OFFTIMER_FLASH_ADDR, 0);
}


#if (IS_GWY)

void fill_macid()
{
    char macid[17];
    strcpy(macid, cJSON_GetObjectItem(json_packet_j, MAC_ID_KEY)->valuestring);
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
    cyan_printf(LTE_DEBUG_TAG, "Error checking received packet ... ");
    /* Common in all Packets */
    /*
        Json packet id should be between (0 and 10) or (100 and 110) or be 99.
    */
    if((json_packet_id >= 0 && json_packet_id <= 10) || json_packet_id == 99 || (json_packet_id >= 100 && json_packet_id <= 110) || json_packet_id == 800);
    else {
        json_ack_err_code = JSON_PACKET_ID_UNKNOWN;
        add_to_pubmesg_queue("JSON_PACKET_ID_UNKNOWN", publish_topic);
        return;
    }
    if (cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)) {
        int32_t msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
        if(msg_seq_no>=0 && msg_seq_no<=65535);
        else{
            json_ack_err_code = MSG_SEQ_NO_EXCEEDING_RANGE;
        }
    }
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
            json_ack_err_code = GWY_SER_NO_INVALID;
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
            if(cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY));
            else {
                json_ack_err_code = NODE_SER_NO_NOT_FOUND;
                return;
            }
            if(json_packet_id != NODE_PROV_PACKET && !cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)) 
                json_ack_err_code = ELEMENT_ADDR_NOT_FOUND;
    }

    switch (json_packet_id)
    {
    case GWY_DEBUG_INFO_PACKET:
        if(cJSON_GetObjectItem(json_packet_j, RESET_DEVICE_KEY));
        else {
            json_ack_err_code = RESET_DEVICE_NOT_FOUND;
            return;
        }
        if(cJSON_GetObjectItem(json_packet_j, LOGGING_KEY));
        else {
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
            int8_t fanspeed = cJSON_GetObjectItem(json_packet_j, FAN_SPEED_KEY)->valueint;
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
            int8_t temperature = cJSON_GetObjectItem(json_packet_j, TEMPERATURE_KEY)->valueint;
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
            int8_t timer = cJSON_GetObjectItem(json_packet_j, ONTIMER_KEY)->valueint;
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
            int8_t timer = cJSON_GetObjectItem(json_packet_j, OFFTIMER_KEY)->valueint;
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
            int16_t locking = cJSON_GetObjectItem(json_packet_j, AC_LOCKING_KEY)->valueint;
            if(locking != 0 && locking !=1) {
                json_ack_err_code = LOCKING_EXCEEDING_RANGE;
            }
        }
        else {
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
            int16_t PublishPeriodSec = cJSON_GetObjectItem(json_packet_j, PUBLISH_PERIOD_KEY)->valueint;
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
    }
}

/**
 * @brief Function that fills the message that needs to be sent as ACK to cloud
 * handles only the Gwy part. ACK for the Node part is being handled at the Gwy_mesh_main.c unless, the 
 * packet had errors
 * @param json_id JSON PACKET ID
 * @retval none
 */
void handle_sending_ack_to_cloud(uint8_t json_id)
{
    char pubmessage[PUBMESG_LEN];
    switch (json_id)
    {
    case NODE_AC_CONTROL_PACKET:
        sprintf(lte_log_buffer, "Sending Node AC Control ACK");
        custom_printf(LTE_DEBUG_TAG, lte_log_buffer, CYAN);
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %ld}",
            JSON_PACKET_ID_KEY, NODE_AC_CONTROL_PACKET,
            JSON_ACK_NAME_KEY, NODE_AC_CONTROL_ACK_NAME,
            MSG_SEQ_NO_KEY, node_ac_control_t.base_data.msg_seq_no,
            GWY_SER_NO_KEY, node_ac_control_t.base_data.gwy_ser_no_str,
            NODE_SER_NO_KEY, node_ac_control_t.base_data.elementAddr,
            POWER_KEY, node_ac_control_t.control.power,
            MODE_KEY, node_ac_control_t.control.mode_str,
            FAN_SPEED_KEY, node_ac_control_t.control.fanSpeed,
            TEMPERATURE_KEY, node_ac_control_t.control.temp,
            SWING_H_KEY, node_ac_control_t.control.swingH,
            SWING_V_KEY, node_ac_control_t.control.swingV,
            ONTIMER_KEY, node_ac_control_t.control.OnTimer,
            OFFTIMER_KEY, node_ac_control_t.control.OffTimer,
            AC_LOCKING_KEY, node_ac_control_t.control.Locking,
            TEMP_LOCK_UP_LIMIT_KEY, node_ac_control_t.control.TempLockUpLimit,
            TEMP_LOCK_LOW_LIMIT_KEY, node_ac_control_t.control.TempLockLowLimit,
            ERROR_CODE_KEY, json_ack_err_code);
        break;

    case GWY_AC_CONTROL_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy AC Control ACK");
        custom_printf(LTE_DEBUG_TAG, lte_log_buffer, CYAN);
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %ld}",
            JSON_PACKET_ID_KEY, GWY_AC_CONTROL_PACKET,
            JSON_ACK_NAME_KEY, GWY_AC_CONTROL_ACK_NAME,
            MSG_SEQ_NO_KEY, gwy_ac_control_t.base_data.msg_seq_no,
            GWY_SER_NO_KEY, gwy_ac_control_t.base_data.gwy_ser_no_str,
            POWER_KEY, gwy_ac_control_t.control.power,
            MODE_KEY, gwy_ac_control_t.control.mode_str,
            FAN_SPEED_KEY, gwy_ac_control_t.control.fanSpeed,
            TEMPERATURE_KEY, gwy_ac_control_t.control.temp,
            SWING_H_KEY, gwy_ac_control_t.control.swingH,
            SWING_V_KEY, gwy_ac_control_t.control.swingV,
            ONTIMER_KEY, gwy_ac_control_t.control.OnTimer,
            OFFTIMER_KEY, gwy_ac_control_t.control.OffTimer,
            AC_LOCKING_KEY, gwy_ac_control_t.control.Locking,
            TEMP_LOCK_UP_LIMIT_KEY, gwy_ac_control_t.control.TempLockUpLimit,
            TEMP_LOCK_LOW_LIMIT_KEY, gwy_ac_control_t.control.TempLockLowLimit,
            ERROR_CODE_KEY, json_ack_err_code);
        break;
    
    case GWY_DEBUG_INFO_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy Debug Info ACK");
        custom_printf(LTE_DEBUG_TAG, lte_log_buffer, CYAN);
        sprintf(gwy_debug_info_t.firmware, "%d.%d.%d",MAJ_VERSION, MIN_VERSION, INTERNAL_MIN_VERSION);
        sprintf(gwy_debug_info_t.uptimestr, "%0.2f", (esp_timer_get_time()/(3600.00*1000000.00)));
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %d, \"%s\" : %ld}",
            JSON_PACKET_ID_KEY, GWY_DEBUG_INFO_PACKET,
            JSON_ACK_NAME_KEY, GWY_DEBUG_INFO_ACK_NAME,
            MSG_SEQ_NO_KEY, gwy_debug_info_t.base_data.msg_seq_no,
            GWY_SER_NO_KEY, gwy_debug_info_t.base_data.gwy_ser_no_str,
            FIRMWARE_VERSION_KEY, gwy_debug_info_t.firmware,
            REGISTERED_KEY, registered,
            PROTOCOL_SEL_NUM_KEY, protocol_selected_num,
            PUBLISH_MESG_QUEUE_COUNT_KEY, get_pubmesg_queue_count(pubmesg_queue_head),
            PROV_QUEUE_COUNT_KEY, get_prov_queue_count(prov_queue_head),
            UNPROV_QUEUE_COUNT_KEY, get_unprov_queue_count(unprov_queue_head),
            AC_CONTROL_QUEUE_COUNT_KEY, get_ac_control_queue_count(node_ac_control_queue_head),
            RECONF_QUEUE_COUNT_KEY, get_reconf_queue_count(node_reconf_queue_head),
            PUB_CONF_QUEUE_COUNT_KEY, get_heartbeat_pub_conf_queue_count(node_pub_conf_queue_head),
            TEACHING_MODE_QUEUE_COUNT_KEY, get_teaching_mode_queue_count(node_teaching_mode_queue_head),
            DEBUG_INFO_QUEUE_COUNT_KEY, get_debug_info_queue_count(node_debug_info_queue_head),
            DEVICE_UPTIME_KEY, gwy_debug_info_t.uptimestr,
            LOGGING_KEY, LOG_DATA,
            RESET_DEVICE_KEY, gwy_debug_info_t.resetDevice,
            ERROR_CODE_KEY, json_ack_err_code);
        //Only after filling up the ACK message, we must reset the device.
        if(gwy_debug_info_t.resetDevice) factory_reset_device();
        break;
    
    case NODE_DEBUG_INFO_PACKET:
        sprintf(lte_log_buffer, "Sending Node Debug Info ACK");
        custom_printf(LTE_DEBUG_TAG, lte_log_buffer, CYAN);
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %d, \"%s\" : %ld}",
            JSON_PACKET_ID_KEY, NODE_DEBUG_INFO_PACKET,
            JSON_ACK_NAME_KEY, NODE_DEBUG_INFO_ACK_NAME,
            MSG_SEQ_NO_KEY, node_debug_info_t.base_data.msg_seq_no,
            GWY_SER_NO_KEY, node_debug_info_t.base_data.gwy_ser_no_str,
            NODE_SER_NO_KEY, node_debug_info_t.base_data.node_ser_no_str,
            ELEMENT_ADDR_KEY, node_debug_info_t.base_data.elementAddr,
            FIRMWARE_VERSION_KEY, node_debug_info_t.firmware,
            PROTOCOL_SEL_NUM_KEY, node_debug_info_t.protocol,
            DEVICE_UPTIME_KEY, node_debug_info_t.uptimestr,
            LOGGING_KEY, node_debug_info_t.logging,
            RESET_DEVICE_KEY, node_debug_info_t.resetDevice,
            ERROR_CODE_KEY, json_ack_err_code);
        break;
    
    case NODE_TEACHING_MODE_START_PACKET:
        sprintf(lte_log_buffer, "Sending Node Teaching Mode Start ACK");
        custom_printf(LTE_DEBUG_TAG, lte_log_buffer, CYAN);
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %ld}",
            JSON_PACKET_ID_KEY, NODE_TEACHING_MODE_START_PACKET,
            JSON_ACK_NAME_KEY, NODE_TEACHING_MODE_START_ACK_NAME,
            MSG_SEQ_NO_KEY, node_teaching_mode_t.base_data.msg_seq_no,
            GWY_SER_NO_KEY, node_teaching_mode_t.base_data.gwy_ser_no_str,
            NODE_SER_NO_KEY, node_teaching_mode_t.base_data.node_ser_no_str,
            ELEMENT_ADDR_KEY, node_teaching_mode_t.base_data.elementAddr,
            ERROR_CODE_KEY, json_ack_err_code);
        break;
    
    case GWY_TEACHING_MODE_START_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy Teaching Mode Start ACK");
        custom_printf(LTE_DEBUG_TAG, lte_log_buffer, CYAN);
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : %ld}",
            JSON_PACKET_ID_KEY, GWY_TEACHING_MODE_START_PACKET,
            JSON_ACK_NAME_KEY, GWY_TEACHING_MODE_START_ACK_NAME,
            MSG_SEQ_NO_KEY, gwy_teaching_mode_t.base_data.msg_seq_no,
            GWY_SER_NO_KEY, gwy_teaching_mode_t.base_data.gwy_ser_no_str,
            ERROR_CODE_KEY, json_ack_err_code);
        break;
    
    case NODE_RECONF_PACKET:
        sprintf(lte_log_buffer, "Sending Node Reconfiguration ACK");
        custom_printf(LTE_DEBUG_TAG, lte_log_buffer, CYAN);
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %ld}",
            JSON_PACKET_ID_KEY, NODE_RECONF_PACKET,
            JSON_ACK_NAME_KEY, NODE_RECONF_ACK_NAME,
            MSG_SEQ_NO_KEY, node_reconf_t.base_data.msg_seq_no,
            GWY_SER_NO_KEY, node_reconf_t.base_data.gwy_ser_no_str,
            NODE_SER_NO_KEY, node_reconf_t.base_data.elementAddr,
            ERROR_CODE_KEY, json_ack_err_code);
        break;

    case GWY_RECONF_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy Reconfiguration ACK");
        custom_printf(LTE_DEBUG_TAG, lte_log_buffer, CYAN);
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : %ld}",
            JSON_PACKET_ID_KEY, GWY_RECONF_PACKET,
            JSON_ACK_NAME_KEY, GWY_RECONF_ACK_NAME,
            MSG_SEQ_NO_KEY, gwy_reconf_t.base_data.msg_seq_no,
            GWY_SER_NO_KEY, gwy_reconf_t.base_data.gwy_ser_no_str,
            ERROR_CODE_KEY, json_ack_err_code);
        break;
    
    case NODE_HEARTBEAT_PUB_CONF_PACKET:
        sprintf(lte_log_buffer, "Sending Node Heartbeat Publish Configuration ACK");
        custom_printf(LTE_DEBUG_TAG, lte_log_buffer, CYAN);
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %d, \"%s\" : %ld}",
            JSON_PACKET_ID_KEY, NODE_HEARTBEAT_ACK,
            JSON_ACK_NAME_KEY, NODE_HEARTBEAT_ACK_NAME,
            MSG_SEQ_NO_KEY, node_heartbeat_pub_conf_t.base_data.msg_seq_no,
            GWY_SER_NO_KEY, node_heartbeat_pub_conf_t.base_data.gwy_ser_no_str,
            NODE_SER_NO_KEY, node_heartbeat_pub_conf_t.base_data.node_ser_no_str,
            ELEMENT_ADDR_KEY, node_heartbeat_pub_conf_t.base_data.elementAddr,
            PUBLISH_PERIOD_KEY, node_heartbeat_pub_conf_t.pub_conf_period_in_sec,
            ERROR_CODE_KEY, json_ack_err_code);
        break;

    case GWY_HEARTBEAT_PUB_CONF_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy Heartbeat Publish Confiugration ACK");
        custom_printf(LTE_DEBUG_TAG, lte_log_buffer, CYAN);
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %ld}",
            JSON_PACKET_ID_KEY, GWY_HEARTBEAT_PUB_CONF_PACKET,
            JSON_ACK_NAME_KEY, GWY_HEARTBEAT_PUB_CONF_ACK_NAME,
            MSG_SEQ_NO_KEY, gwy_pub_conf_t.base_data.msg_seq_no,
            GWY_SER_NO_KEY, gwy_pub_conf_t.base_data.gwy_ser_no_str,
            PUBLISH_PERIOD_KEY, gwy_pub_conf_t.pub_conf_period_in_sec,
            ERROR_CODE_KEY, json_ack_err_code);
        break;

    case NODE_PROV_PACKET:
        sprintf(lte_log_buffer, "Sending Node Provision ACK");
        custom_printf(LTE_DEBUG_TAG, lte_log_buffer, CYAN);
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld}",
            JSON_PACKET_ID_KEY, NODE_PROV_PACKET,
            JSON_ACK_NAME_KEY, NODE_PROV_ACK_NAME,
            MSG_SEQ_NO_KEY, provision_t.base_data.msg_seq_no,
            GWY_SER_NO_KEY, provision_t.base_data.gwy_ser_no_str,
            NODE_SER_NO_KEY, provision_t.base_data.node_ser_no_str,
            ELEMENT_ADDR_KEY, provision_t.base_data.elementAddr,
            LOCATION_KEY, provision_t.location,
            APP_KEY_INDEX, provision_t.appindex,
            APP_KEY, provision_t.appkey,
            NET_KEY_INDEX, provision_t.netindex,
            NET_KEY, provision_t.netkey,
            ERROR_CODE_KEY, json_ack_err_code);
        break;

    case GWY_REG_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy Registration ACK");
        custom_printf(LTE_DEBUG_TAG, lte_log_buffer, CYAN);
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %ld}",
            JSON_PACKET_ID_KEY, GWY_REG_PACKET,
            JSON_ACK_NAME_KEY, GWY_REG_ACK_NAME,
            MSG_SEQ_NO_KEY, gwy_registration_t.base_data.msg_seq_no,
            GWY_SER_NO_KEY, gwy_registration_t.base_data.gwy_ser_no_str,
            LOCATION_KEY, gwy_registration_t.location,
            ERROR_CODE_KEY, json_ack_err_code);
        break;
    
    case NODE_UNPROV_PACKET:
        sprintf(lte_log_buffer, "Sending Node Unprovision ACK");
        custom_printf(LTE_DEBUG_TAG, lte_log_buffer, CYAN);
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld}",
            JSON_PACKET_ID_KEY, NODE_UNPROV_PACKET,
            JSON_ACK_NAME_KEY, NODE_UNPROV_ACK_NAME,
            MSG_SEQ_NO_KEY, unprovision_t.base_data.msg_seq_no,
            GWY_SER_NO_KEY, unprovision_t.base_data.gwy_ser_no_str,
            NODE_SER_NO_KEY, unprovision_t.base_data.node_ser_no_str,
            ELEMENT_ADDR_KEY, unprovision_t.base_data.elementAddr,
            LOCATION_KEY, unprovision_t.location,
            ERROR_CODE_KEY, json_ack_err_code);
        break;

    case GWY_UNREG_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy Unregistration ACK");
        custom_printf(LTE_DEBUG_TAG, lte_log_buffer, CYAN);
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %ld, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %ld}",
            JSON_PACKET_ID_KEY, GWY_UNREG_PACKET,
            JSON_ACK_NAME_KEY, GWY_UNREG_ACK_NAME,
            MSG_SEQ_NO_KEY, gwy_unregistration_t.base_data.msg_seq_no,
            GWY_SER_NO_KEY, gwy_unregistration_t.base_data.gwy_ser_no_str,
            LOCATION_KEY, gwy_unregistration_t.location,
            ERROR_CODE_KEY, json_ack_err_code);
        if(json_ack_err_code == SUCCESS) factory_reset_device();
        break;
    }
    add_to_pubmesg_queue(pubmessage, publish_topic);
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
        if(json_packet_j == NULL) {
            red_printf(LTE_DEBUG_TAG, "Failure in parsing the JSON string");
            return;
        }
        json_ack_err_code = JSON_PACKET_ID_NOT_FOUND;
        add_to_pubmesg_queue("{\"ErrorCode\" : 1}", publish_topic);
    }

    /**
     * @brief If the parsing of JSON string was successful and there was json_packet_id in it
     * Then the next step is to error check the JSON object before starting to store it into structure members
     */
    if(json_ack_err_code == SUCCESS) error_check_json(json_packet_id);

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
            node_ac_control_t.base_data.request_in_time_us = esp_timer_get_time(); //Note the time as this is being used by queue
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
            ESP_LOGI(LTE_DEBUG_TAG, "Gwy AC Control Packet");
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
            ESP_LOGI(LTE_DEBUG_TAG, "Node Debug Info Packet");
            node_debug_info_t.base_data.request_in_time_us = esp_timer_get_time(); //Note the time as this is being used by queue
            node_debug_info_t.base_data.json_packet_id = json_packet_id;
            node_debug_info_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(node_debug_info_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            strcpy(node_debug_info_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            node_debug_info_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            add_to_debug_info_queue();
            break;

        case GWY_DEBUG_INFO_PACKET:
            ESP_LOGI(LTE_DEBUG_TAG, "Gwy Debug Info Packet");
            gwy_debug_info_t.base_data.json_packet_id = json_packet_id;
            strcpy(gwy_debug_info_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            gwy_debug_info_t.resetDevice = cJSON_GetObjectItem(json_packet_j, RESET_DEVICE_KEY)->valueint;
            gwy_debug_info_t.logging = cJSON_GetObjectItem(json_packet_j, LOGGING_KEY)->valueint;
            if(gwy_debug_info_t.logging) LOG_DATA = true;
            else LOG_DATA = false;
            //The resetDevice is used after sending the ack for this packet
            break;

        case NODE_RECONF_PACKET:
            ESP_LOGI(LTE_DEBUG_TAG, "Node Reconfiguration Packet");
            node_reconf_t.base_data.request_in_time_us = esp_timer_get_time(); //Note the time as this is being used by queue
            node_reconf_t.base_data.json_packet_id = json_packet_id;
            node_reconf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(node_reconf_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            strcpy(node_reconf_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            node_reconf_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            add_to_reconf_queue();
            break;
        
        case GWY_RECONF_PACKET:
            ESP_LOGI(LTE_DEBUG_TAG, "Gwy Reconfiguration Packet");
            gwy_reconf_t.base_data.json_packet_id = json_packet_id;
            strcpy(gwy_reconf_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            gwy_reconf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            configured = false;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR, false);
            break;

        case NODE_HEARTBEAT_PUB_CONF_PACKET:
            ESP_LOGI(LTE_DEBUG_TAG, "Node Heartbeat Publish configuration Packet");
            node_heartbeat_pub_conf_t.base_data.request_in_time_us = esp_timer_get_time(); //Note the time here as it is being used by queue
            node_heartbeat_pub_conf_t.base_data.json_packet_id = json_packet_id;
            node_heartbeat_pub_conf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(node_heartbeat_pub_conf_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            strcpy(node_heartbeat_pub_conf_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            node_heartbeat_pub_conf_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            node_heartbeat_pub_conf_t.pub_conf_period_in_sec = cJSON_GetObjectItem(json_packet_j, PUBLISH_PERIOD_KEY)->valueint;
            add_to_heartbeat_pub_conf_queue();
            break;
        
        case GWY_HEARTBEAT_PUB_CONF_PACKET:
            ESP_LOGI(LTE_DEBUG_TAG, "Gwy Heartbeat Publish configuration Packet");
            gwy_pub_conf_t.base_data.json_packet_id = json_packet_id;
            strcpy(gwy_pub_conf_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            gwy_pub_conf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            gwy_pub_conf_t.pub_conf_period_in_sec = cJSON_GetObjectItem(json_packet_j, PUBLISH_PERIOD_KEY)->valueint;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, HB_PUB_CONF_PERIOD_ADDR, gwy_pub_conf_t.pub_conf_period_in_sec);
    
            delete_Temperature_data_publish_timer();
            create_Temperature_data_publish_timer();
            break;

        case NODE_TEACHING_MODE_START_PACKET:
            ESP_LOGI(LTE_DEBUG_TAG, "Node Teaching Mode Start Packet");
            node_teaching_mode_t.base_data.request_in_time_us = esp_timer_get_time(); //Note the time here as it is being used by queue
            node_teaching_mode_t.base_data.json_packet_id = json_packet_id;
            strcpy(node_teaching_mode_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            strcpy(node_teaching_mode_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            node_teaching_mode_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            add_to_teaching_mode_queue();
            break;

        case GWY_TEACHING_MODE_START_PACKET:
            ESP_LOGI(LTE_DEBUG_TAG, "Gwy Teaching Mode Start Packet");
            gwy_teaching_mode_t.base_data.json_packet_id = json_packet_id;
            strcpy(gwy_teaching_mode_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            teaching_mode = true;
            teachMode_size_done = true;
            break;

        case NODE_PROV_PACKET:
            ESP_LOGI(LTE_DEBUG_TAG, "Node Provisioning Packet");
            provision_t.base_data.request_in_time_us = esp_timer_get_time(); //Note the time here as this is being used by queue
            provision_t.base_data.json_packet_id = json_packet_id;
            provision_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(provision_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            strcpy(provision_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            strcpy(provision_t.location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            fill_macid();
            add_to_prov_queue();
            break;
        
        case GWY_REG_PACKET:
            ESP_LOGI(LTE_DEBUG_TAG, "Gwy Registration Packet");
            gwy_registration_t.base_data.json_packet_id = json_packet_id;
            strcpy(gwy_registration_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            gwy_registration_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(gwy_registration_t.location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            registered = true;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, REGISTERED_FLAG_FLASH_ADDR, true);
    
            eeprom_write_byte(EEPROM_SLAVE_ADDR, FACTORY_DEVICE_CHECK_FLASH_ADDR, 0x00);
    
            break;

        case NODE_UNPROV_PACKET:
            ESP_LOGI(LTE_DEBUG_TAG, "Node Unprovisioning Packet");
            unprovision_t.base_data.request_in_time_us = esp_timer_get_time(); //Note the time here as this is being used by queue
            unprovision_t.base_data.json_packet_id = json_packet_id;
            unprovision_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(unprovision_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            strcpy(unprovision_t.base_data.node_ser_no_str, cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valuestring);
            strcpy(unprovision_t.location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            unprovision_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELEMENT_ADDR_KEY)->valueint;
            add_to_unprov_queue();
            break;

        case GWY_UNREG_PACKET:
            ESP_LOGI(LTE_DEBUG_TAG, "Gwy Unregistration Packet");
            gwy_unregistration_t.base_data.json_packet_id = json_packet_id;
            strcpy(gwy_unregistration_t.base_data.gwy_ser_no_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
            gwy_unregistration_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            strcpy(gwy_unregistration_t.location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            //factory resetting after receiving this packet is taken care at the place of sending ack
            break;

        case SET_GWY_SER_NO: //Only for developer use //Not mentioned in document
            ESP_LOGI(LTE_DEBUG_TAG, "Set GwySerNo Packet");
            GWY_SER_NO = cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valueint;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_LO, GWY_SER_NO);
    
            eeprom_write_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_MID, GWY_SER_NO>>8);
    
            eeprom_write_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_HI, GWY_SER_NO>>16);
    
            esp_restart_flag = true;
            break;
        }
    }
    if(json_ack_err_code == JSON_PACKET_ID_UNKNOWN) {
        add_to_pubmesg_queue("{\"ErrorCode\" : 2}", publish_topic);
    };
    sprintf(lte_log_buffer, "Error Code : %s", get_err_string(json_ack_err_code));
    custom_printf(LTE_ERROR_TAG, lte_log_buffer, RED);

    //Handle sending back ACK for gwy related packets here &
    //Handle sending back ACK for node related packets here only if the packet contains errors
    //Else it is taken care at the mesh side.
    if((json_packet_id>=100 && json_ack_err_code != SUCCESS) || json_packet_id <= 10)
        handle_sending_ack_to_cloud(json_packet_id);
}

#endif