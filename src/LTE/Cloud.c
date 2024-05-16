/**
 * @file Cloud.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to MQTT packet handling and Sending Acknowledgements
 * @version 0.6
 * @date 2024-04-16
 * @copyright Copyright (c) 2024
 */

#include "../../inc/LTE/LTE.h"

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
    /* GWY SER NO */
    gwy_registration_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_unregistration_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_conf_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_reconf_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_ac_control_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_locking_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_reset_mqtt_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_pub_conf_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_temperature_data_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_pub_conf_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_teaching_mode_t.base_data.gwy_ser_no = GWY_SER_NO;

    /* GWY SER NO STRING */
    strcpy(gwy_registration_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_unregistration_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_conf_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_reconf_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_ac_control_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_locking_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_reset_mqtt_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_pub_conf_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_temperature_data_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_pub_conf_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);
    strcpy(gwy_teaching_mode_t.base_data.gwy_ser_no_str, GWY_SER_NO_IN_STRING);

    /* GWY - JSON PACKET IDs */
    gwy_registration_t.base_data.json_packet_id = GWY_REG_PACKET;
    gwy_unregistration_t.base_data.json_packet_id = GWY_UNREG_PACKET;
    gwy_conf_t.base_data.json_packet_id = GWY_CONF_PACKET;
    gwy_reconf_t.base_data.json_packet_id = GWY_RECONF_PACKET;
    gwy_ac_control_t.base_data.json_packet_id = GWY_AC_CONTROL_PACKET;
    gwy_locking_t.base_data.json_packet_id = GWY_AC_LOCKING_PACKET;
    gwy_reset_mqtt_t.base_data.json_packet_id = RESET_MQTT;
    gwy_pub_conf_t.base_data.json_packet_id = GWY_PUB_CONF_PACKET;
    gwy_temperature_data_t.base_data.json_packet_id = GWY_TEMPERATURE_DATA_PACKET;
    gwy_teaching_mode_t.base_data.json_packet_id = GWY_TEACHING_MODE_START_PACKET;

    /* NODE - JSON PACKET IDs */
    provision_t.base_data.json_packet_id = NODE_PROV_PACKET;
    unprovision_t.base_data.json_packet_id = NODE_UNPROV_PACKET;
    node_conf_t.base_data.json_packet_id = NODE_CONF_PACKET;
    node_reconf_t.base_data.json_packet_id = NODE_RECONF_PACKET;
    node_ac_control_t.base_data.json_packet_id = NODE_AC_CONTROL_PACKET;
    node_locking_t.base_data.json_packet_id = NODE_AC_LOCKING_PACKET;
    node_pub_conf_t.base_data.json_packet_id = NODE_PUB_CONF_PACKET;
    node_temperature_data_t.base_data.json_packet_id = NODE_TEMPERATURE_DATA_PACKET;

    /* JSON ACK NAMES */
    strcpy(gwy_registration_t.base_data.ack_name, GWY_REG_ACK);
    strcpy(gwy_unregistration_t.base_data.ack_name, GWY_UNREG_ACK);
    strcpy(gwy_conf_t.base_data.ack_name, GWY_CONF_ACK);
    strcpy(gwy_reconf_t.base_data.ack_name, GWY_RECONF_ACK);
    strcpy(gwy_ac_control_t.base_data.ack_name, GWY_AC_CONTROL_ACK);
    strcpy(gwy_locking_t.base_data.ack_name, GWY_LOCKING_ACK);
    strcpy(gwy_reset_mqtt_t.base_data.ack_name, GWY_RESET_MQTT_ACK);
    strcpy(gwy_pub_conf_t.base_data.ack_name, GWY_PUB_CONF_ACK);
    strcpy(gwy_temperature_data_t.base_data.ack_name, GWY_TEMPERATURE_DATA_ACK);
}

void fill_macid()
{
    char macid[17];
    strcpy(macid, cJSON_GetObjectItem(json_packet_j, MAC_ID_KEY)->valuestring);
    char hex_char_str[2];
    for (uint8_t index = 0, i = 0; index < 6; index++, i += 3)
    {
        strncat(hex_char_str, &macid[i], 1);
        strncat(hex_char_str, &macid[i + 1], 1);
        provision_t.macid[index] = strtol(hex_char_str, NULL, 16);
        strcpy(hex_char_str, "");
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
    // Checking keys that are common in all JSON packets
    if (cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY))
        ;
    else
    {
        json_ack_err_code = INVALID_MSG_SEQ_NO;
        return;
    }
    if (cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY))
        ;
    else
    {
        json_ack_err_code = INVALID_GWY_SER_NO;
        return;
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
            ;
        else
        {
            json_ack_err_code = INVALID_LOCATION_KEY;
            return;
        }
        return;

    case GWY_UNREG_PACKET:
        if (!registered)
        {
            json_ack_err_code = GWY_ALREADY_UNREG;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, LOCATION_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_LOCATION_KEY;
            return;
        }
        return;

    case GWY_AC_CONTROL_PACKET:
    case NODE_AC_CONTROL_PACKET:
        if (json_packet_id == GWY_AC_CONTROL_PACKET)
        {
            if (!registered)
            {
                json_ack_err_code = GWY_NOT_REG;
                return;
            }
            if (!configured)
            {
                json_ack_err_code = GWY_NOT_CONF;
                return;
            }
        }
        else
        {
            if (cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY))
                ;
            else
            {
                json_ack_err_code = INVALID_NODE_SER_NO;
                return;
            }
        }
        if (cJSON_GetObjectItem(json_packet_j, POWER_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_POWER;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, MODE_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_MODE;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, FAN_SPEED_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_FAN_SPEED;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, TEMPERATURE_KEY))
            ;

        else
        {
            json_ack_err_code = INVALID_TEMPERATURE;
            return;
        }
        uint8_t temperature = cJSON_GetObjectItem(json_packet_j, TEMPERATURE_KEY)->valueint;
        if (temperature < TEMPERATURE_LOWER_LIMIT)
        {
            json_ack_err_code = EXCEEDING_TEMP_LOWER_LIMIT;
            return;
        }
        if (temperature > TEMPERATURE_UPPER_LIMIT)
        {
            json_ack_err_code = EXCEEDING_TEMP_UPPER_LIMIT;
            return;
        }

        if (cJSON_GetObjectItem(json_packet_j, SWING_H_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_SWING_H;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, SWING_V_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_SWING_V;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, ONTIMER_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_ONTIMER;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, OFFTIMER_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_OFFTIMER;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, AC_LOCKING_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_LOCKING;
            return;
        }

        if (cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_UP_LIMIT_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_TEMP_UPPER_LIMIT;
            return;
        }
        uint8_t temp_upper_limit = (cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_UP_LIMIT_KEY))->valueint;
        if (temp_upper_limit > TEMPERATURE_UPPER_LIMIT)
        {
            json_ack_err_code = LOCKING_TEMP_UP_LIMIT_EXCEEDING_TEMP_UP_LIMIT;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_LOW_LIMIT_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_TEMP_LOWER_LIMIT;
            return;
        }
        uint8_t temp_lower_limit = (cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_LOW_LIMIT_KEY))->valueint;
        if (temp_lower_limit < TEMPERATURE_LOWER_LIMIT)
        {
            json_ack_err_code = LOCKING_TEMP_LOW_LIMIT_EXCEEDING_TEMP_LOW_LIMIT;
            return;
        }
        if (temp_lower_limit > temp_upper_limit)
        {
            json_ack_err_code = ILLOGICAL_LOCKING_TEMP_LIMIT;
            return;
        }

        return;

    case NODE_PROV_PACKET:
        if (cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_NODE_SER_NO;
            return;
        }
        return;

    case NODE_UNPROV_PACKET:
    case NODE_RECONF_PACKET:
        if (cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_NODE_SER_NO;
            return;
        }
        if (cJSON_GetObjectItem(json_packet_j, ELMNT_ADDR_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_ELMNT_ADDR;
            return;
        }
        return;

    case GWY_PUB_CONF_PACKET:
    case NODE_PUB_CONF_PACKET:
        if (cJSON_GetObjectItem(json_packet_j, PUBLISH_PERIOD_KEY))
            ;
        else
        {
            json_ack_err_code = INVALID_PUBLISH_PERIOD;
            return;
        }
        return;

    case RESET_MQTT:
        return;
    }
}

/**
 * @brief Function that fills the message that needs to be sent as ack to cloud
 * handles only the Gwy part.
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
                JSON_ACK_NAME_KEY, GWY_REG_ACK,
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
                JSON_ACK_NAME_KEY, GWY_UNREG_ACK,
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
                JSON_ACK_NAME_KEY, GWY_AC_CONTROL_ACK,
                MSG_SEQ_NO_KEY, gwy_ac_control_t.base_data.msg_seq_no,
                GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
                POWER_KEY, gwy_ac_control_t.power,
                MODE_KEY, gwy_ac_control_t.mode_str,
                FAN_SPEED_KEY, gwy_ac_control_t.fan,
                TEMPERATURE_KEY, gwy_ac_control_t.temp,
                SWING_H_KEY, gwy_ac_control_t.swingH,
                SWING_V_KEY, gwy_ac_control_t.swingV,
                ONTIMER_KEY, gwy_ac_control_t.OnTimer,
                OFFTIMER_KEY, gwy_ac_control_t.OffTimer,
                AC_LOCKING_KEY, gwy_ac_control_t.Locking,
                ERROR_CODE_KEY, json_ack_err_code);
        add_to_pubmesg_queue(pubmessage, publish_topic);
        break;

    case GWY_RECONF_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy Reconf Ack");
        cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
        sprintf(pubmessage, "{%s : %d, %s : %ss, %s : %d, %s : %s, %s : %d}",
                JSON_PACKET_ID_KEY, GWY_RECONF_PACKET,
                JSON_ACK_NAME_KEY, GWY_RECONF_ACK,
                MSG_SEQ_NO_KEY, gwy_reconf_t.base_data.msg_seq_no,
                GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
                ERROR_CODE_KEY, json_ack_err_code);
        add_to_pubmesg_queue(pubmessage, publish_topic);
        break;

    case GWY_AC_LOCKING_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy AC Locking Ack");
        cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
        sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %s, %s : %d, %s : %s, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d, %s : %d}",
                JSON_PACKET_ID_KEY, GWY_AC_LOCKING_PACKET,
                JSON_ACK_NAME_KEY, GWY_LOCKING_ACK,
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

    case GWY_PUB_CONF_PACKET:
        sprintf(lte_log_buffer, "Sending Gwy Pub conf Ack");
        cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
        sprintf(pubmessage, "{%s : %d, %s : %s, %s : %d, %s : %s, %s : %d, %s : %d}",
                JSON_PACKET_ID_KEY, GWY_PUB_CONF_PACKET,
                JSON_ACK_NAME_KEY, GWY_PUB_CONF_ACK,
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
                JSON_ACK_NAME_KEY, GWY_RESET_MQTT_ACK,
                MSG_SEQ_NO_KEY, gwy_reset_mqtt_t.base_data.msg_seq_no,
                GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
                ERROR_CODE_KEY, json_ack_err_code);
        break;
        add_to_pubmesg_queue(pubmessage, publish_topic);
    }
    
}

void get_mode_value(char *device_type)
{
    if (strcmp(device_type, "gwy")==0)
    {
        if (strcasecmp(gwy_ac_control_t.mode_str, "Auto") == 0)
            gwy_ac_control_t.mode_val = AUTO;
        else if (strcasecmp(gwy_ac_control_t.mode_str, "Cool") == 0)
            gwy_ac_control_t.mode_val = COOL;
        else if (strcasecmp(gwy_ac_control_t.mode_str, "Dry") == 0)
            gwy_ac_control_t.mode_val = DRY;
        else if (strcasecmp(gwy_ac_control_t.mode_str, "Heat") == 0)
            gwy_ac_control_t.mode_val = HEAT;
        else if (strcasecmp(gwy_ac_control_t.mode_str, "Fan") == 0)
            gwy_ac_control_t.mode_val = FAN;
    }
    else
    {
        if (strcasecmp(node_ac_control_t.mode_str, "Auto") == 0)
            gwy_ac_control_t.mode_val = AUTO;
        else if (strcasecmp(node_ac_control_t.mode_str, "Cool") == 0)
            gwy_ac_control_t.mode_val = COOL;
        else if (strcasecmp(node_ac_control_t.mode_str, "Dry") == 0)
            gwy_ac_control_t.mode_val = DRY;
        else if (strcasecmp(node_ac_control_t.mode_str, "Heat") == 0)
            gwy_ac_control_t.mode_val = HEAT;
        else if (strcasecmp(node_ac_control_t.mode_str, "Fan") == 0)
            gwy_ac_control_t.mode_val = FAN;
    }
}

uint16_t get_gwy_ser_no()
{
    char *ptr;
    uint16_t gwy_ser_no;
    char gwy_ser_no_in_str[8];
    strcpy(gwy_ser_no_in_str, cJSON_GetObjectItem(json_packet_j, GWY_SER_NO_KEY)->valuestring);
    char gwy_ser_no_alone_str[5];
    strncpy(gwy_ser_no_alone_str, gwy_ser_no_in_str + 3, 5);
    gwy_ser_no = strtol(gwy_ser_no_alone_str, &ptr, 10);
    return gwy_ser_no;
}

char *get_err_string(int16_t err_code)
{
    switch (err_code)
    {
    case FAILURE:
        return "FAILURE";
    case SUCCESS:
        return "SUCCESS";
    case INVALID_JSON_PACKET_ID:
        return "INVALID_JSON_PACKET_ID";
    case INVALID_MSG_SEQ_NO:
        return "INVALID_MSG_SEQ_NO";
    case INVALID_GWY_SER_NO:
        return "INVALID_GWY_SER_NO";
    case INVALID_NODE_SER_NO:
        return "INVALID_NODE_SER_NO";
    case INVALID_LOCATION_KEY:
        return "INVALID_LOCATION_KEY";
    case NODE_TIMEOUT:
        return "NODE_TIMEOUT";
    case GWY_ALREADY_REG:
        return "GWY_ALREADY_REG";
    case GWY_ALREADY_UNREG:
        return "GWY_ALREADY_UNREG";
    case NODE_ALREADY_PROV:
        return "NODE_ALREADY_PROV";
    case NODE_ALREADY_UNPROV:
        return "NODE_ALREADY_UNPROV";
    case GWY_ALREADY_UNCONF:
        return "GWY_ALREADY_UNCONF";
    case NODE_ALREADY_UNCONF:
        return "NODE_ALREADY_UNCONF";
    case GWY_NOT_REG:
        return "GWY_NOT_REG";
    case GWY_NOT_CONF:
        return "GWY_NOT_CONF";
    case INVALID_POWER:
        return "INVALID_POWER";
    case INVALID_MODE:
        return "INVALID_MODE";
    case INVALID_FAN_SPEED:
        return "INVALID_FAN_SPEED";
    case INVALID_TEMPERATURE:
        return "INVALID_TEMPERATURE";
    case INVALID_SWING_H:
        return "INVALID_SWING_H";
    case INVALID_SWING_V:
        return "INVALID_SWING_V";
    case INVALID_ONTIMER:
        return "INVALID_ONTIMER";
    case INVALID_OFFTIMER:
        return "INVALID_OFFTIMER";
    case INVALID_LOCKING:
        return "INVALID_LOCKING";
    case INVALID_TEMP_UPPER_LIMIT:
        return "INVALID_TEMP_UPPER_LIMIT";
    case INVALID_TEMP_LOWER_LIMIT:
        return "INVALID_TEMP_LOWER_LIMIT";
    case LOCKING_TEMP_UP_LIMIT_EXCEEDING_TEMP_UP_LIMIT:
        return "LOCKING_TEMP_UP_LIMIT_EXCEEDING_TEMP_UP_LIMIT";
    case LOCKING_TEMP_LOW_LIMIT_EXCEEDING_TEMP_LOW_LIMIT:
        return "LOCKING_TEMP_LOW_LIMIT_EXCEEDING_TEMP_LOW_LIMIT";
    case ILLOGICAL_LOCKING_TEMP_LIMIT:
        return "ILLOGICAL_LOCKING_TEMP_LIMIT";
    case INVALID_ELMNT_ADDR:
        return "INVALID_ELMNT_ADDR";
    case EXCEEDING_TEMP_LOWER_LIMIT:
        return "EXCEEDING_TEMP_LOWER_LIMIT";
    case EXCEEDING_TEMP_UPPER_LIMIT:
        return "EXCEEDING_TEMP_UPPER_LIMIT";
    case INVALID_PUBLISH_PERIOD:
        return "INVALID_PUBLISH_PERIOD";
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
        json_ack_err_code = INVALID_JSON_PACKET_ID;
        ESP_LOGE(LTE_ERROR_TAG, "Unable to parse UART data into JSON");
        return;
    }

    error_check_json(json_packet_id);
    if (json_ack_err_code == SUCCESS)
    {
        switch (json_packet_id)
        {
        case GWY_REG_PACKET:
            sprintf(lte_log_buffer, "Gwy Registration packet");
            cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
            gwy_registration_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            gwy_registration_t.base_data.gwy_ser_no = get_gwy_ser_no();
            strcpy(gwy_registration_t.base_data.location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            gwy_registration_t.base_data.request_in_time_us = esp_timer_get_time();
            registered = true;
            break;

        case GWY_UNREG_PACKET:
            sprintf(lte_log_buffer, "Gwy Unregistration packet");
            cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
            gwy_unregistration_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            gwy_unregistration_t.base_data.gwy_ser_no = get_gwy_ser_no();
            strcpy(gwy_unregistration_t.base_data.location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            gwy_unregistration_t.base_data.request_in_time_us = esp_timer_get_time();
            registered = false;
            configured = false;
            break;

        case GWY_AC_CONTROL_PACKET:
            sprintf(lte_log_buffer, "Gwy AC Control packet");
            cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
            // filling the default values for temp up and low limit
            gwy_ac_control_t.TempLowLimit = TEMPERATURE_LOWER_LIMIT;
            gwy_ac_control_t.TempUpLimit = TEMPERATURE_UPPER_LIMIT;
            gwy_ac_control_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            gwy_ac_control_t.base_data.gwy_ser_no = get_gwy_ser_no();
            gwy_ac_control_t.power = cJSON_GetObjectItem(json_packet_j, POWER_KEY)->valueint;
            strcpy(gwy_ac_control_t.mode_str, cJSON_GetObjectItem(json_packet_j, MODE_KEY)->valuestring);
            get_mode_value("gwy");
            gwy_ac_control_t.fan = cJSON_GetObjectItem(json_packet_j, FAN_SPEED_KEY)->valueint;
            gwy_ac_control_t.temp = cJSON_GetObjectItem(json_packet_j, TEMPERATURE_KEY)->valueint;
            gwy_ac_control_t.swingH = cJSON_GetObjectItem(json_packet_j, SWING_H_KEY)->valueint;
            gwy_ac_control_t.swingV = cJSON_GetObjectItem(json_packet_j, SWING_V_KEY)->valueint;
            gwy_ac_control_t.OnTimer = cJSON_GetObjectItem(json_packet_j, ONTIMER_KEY)->valueint;
            gwy_ac_control_t.OffTimer = cJSON_GetObjectItem(json_packet_j, OFFTIMER_KEY)->valueint;
            gwy_ac_control_t.Locking = cJSON_GetObjectItem(json_packet_j, AC_LOCKING_KEY)->valueint;
            gwy_ac_control_t.TempLowLimit = cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_LOW_LIMIT_KEY)->valueint;
            gwy_ac_control_t.TempUpLimit = cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_UP_LIMIT_KEY)->valueint;
            if (configured)
                needToSendIRComamnd = true;
            else
                ESP_LOGE(LTE_ERROR_TAG, "Gwy not configured yet, Can't control AC");
            break;

        case GWY_RECONF_PACKET:
            sprintf(lte_log_buffer, "Gwy Reconfiguration packet");
            cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
            gwy_reconf_t.base_data.request_in_time_us = esp_timer_get_time();
            gwy_reconf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            gwy_reconf_t.base_data.gwy_ser_no = get_gwy_ser_no();
            configured = false;
            break;

        case GWY_PUB_CONF_PACKET:
            sprintf(lte_log_buffer, "Gwy Publish configuration packet");
            cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
            gwy_pub_conf_t.base_data.request_in_time_us = esp_timer_get_time();
            gwy_pub_conf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            gwy_pub_conf_t.base_data.gwy_ser_no = get_gwy_ser_no();
            gwy_pub_conf_t.pub_conf_period_in_sec = cJSON_GetObjectItem(json_packet_j, PUBLISH_PERIOD_KEY)->valueint;
            create_Temperature_data_publish_timer();
            break;

        case NODE_PROV_PACKET:
            sprintf(lte_log_buffer, "Node Provisioning packet");
            cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
            provision_t.base_data.request_in_time_us = esp_timer_get_time();
            provision_t.base_data.json_packet_id = json_packet_id;
            provision_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            provision_t.base_data.gwy_ser_no = get_gwy_ser_no();
            provision_t.base_data.node_ser_no = cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valueint;
            strcpy(provision_t.base_data.location, cJSON_GetObjectItem(json_packet_j, LOCATION_KEY)->valuestring);
            fill_macid();
            add_to_prov_queue();
            break;

        case NODE_UNPROV_PACKET:
            sprintf(lte_log_buffer, "Node Unprovisioning packet");
            cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
            unprovision_t.base_data.request_in_time_us = esp_timer_get_time();
            unprovision_t.base_data.json_packet_id = json_packet_id;
            unprovision_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            unprovision_t.base_data.gwy_ser_no = get_gwy_ser_no();
            unprovision_t.base_data.node_ser_no = cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valueint;
            unprovision_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELMNT_ADDR_KEY)->valueint;
            break;

        case NODE_AC_CONTROL_PACKET:
            sprintf(lte_log_buffer, "Node AC Control packet");
            cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
            node_ac_control_t.base_data.request_in_time_us = esp_timer_get_time();
            node_ac_control_t.base_data.json_packet_id = json_packet_id;
            node_ac_control_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            node_ac_control_t.base_data.gwy_ser_no = get_gwy_ser_no();
            node_ac_control_t.base_data.node_ser_no = cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valueint;
            node_ac_control_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELMNT_ADDR_KEY)->valueint;
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
            node_ac_control_t.TempLowLimit = cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_LOW_LIMIT_KEY)->valueint;
            node_ac_control_t.TempUpLimit = cJSON_GetObjectItem(json_packet_j, TEMP_LOCK_UP_LIMIT_KEY)->valueint;
            break;

        case NODE_RECONF_PACKET:
            sprintf(lte_log_buffer, "Node Reconfiguration packet");
            cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
            node_reconf_t.base_data.request_in_time_us = esp_timer_get_time();
            node_reconf_t.base_data.json_packet_id = json_packet_id;
            node_reconf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            node_reconf_t.base_data.gwy_ser_no = get_gwy_ser_no();
            node_reconf_t.base_data.node_ser_no = cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valueint;
            node_reconf_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELMNT_ADDR_KEY)->valueint;
            break;

        case NODE_PUB_CONF_PACKET:
            sprintf(lte_log_buffer, "Node Publish configuratoin packet received");
            cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
            node_pub_conf_t.base_data.request_in_time_us = esp_timer_get_time();
            node_pub_conf_t.base_data.json_packet_id = json_packet_id;
            node_pub_conf_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            node_pub_conf_t.base_data.gwy_ser_no = get_gwy_ser_no();
            node_pub_conf_t.base_data.node_ser_no = cJSON_GetObjectItem(json_packet_j, NODE_SER_NO_KEY)->valueint;
            node_pub_conf_t.base_data.elementAddr = cJSON_GetObjectItem(json_packet_j, ELMNT_ADDR_KEY)->valueint;
            node_pub_conf_t.pub_conf_period_in_sec = cJSON_GetObjectItem(json_packet_j, PUBLISH_PERIOD_KEY)->valueint;
            break;

        case RESET_MQTT:
            sprintf(lte_log_buffer, "Reset MQTT packet");
            cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
            gwy_reset_mqtt_t.base_data.json_packet_id = json_packet_id;
            gwy_reset_mqtt_t.base_data.msg_seq_no = cJSON_GetObjectItem(json_packet_j, MSG_SEQ_NO_KEY)->valueint;
            gwy_reset_mqtt_t.base_data.gwy_ser_no = get_gwy_ser_no();
#if (AP_PART_ENABLED)
            LED_state = LED_STATE_AP_MODE;
            reset_mqtt();
#endif
#if (!AP_PART_ENABLED)
            ESP_LOGE(LTE_ERROR_TAG, "AP mode is not enabled. So skipping reset of MQTT");
#endif
            break;

        default:
            sprintf(lte_log_buffer, "Unknown MQTT packet received in parse_json_packet");
            cyan_printf(LTE_DEBUG_TAG, lte_log_buffer);
        }
    }
    sprintf(lte_log_buffer, "Error Code : %s", get_err_string(json_ack_err_code));
    handle_sending_ack_to_cloud(json_packet_id);
}