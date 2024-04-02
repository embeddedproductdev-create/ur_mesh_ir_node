/**
 * @file main_IR_recv.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions related to the IR receiver part
 * @version 0.1
 * @date 2024-02-29
 * @copyright Copyright (c) 2024
 */

#include "../../inc/IR/main_IR.h"
#include "../../inc/LTE/LTE.h"
#include "../../inc/LTE/mqtt.h"
#include "../../inc/Custom/button.h"

// Initialization - Receiver
bool configured = false;
bool teaching_mode = false;
IRrecv irrecv(IR_RECEIVER_PIN, RECV_BUFFER_SIZE, kTimeout, true);
decode_results results;
decode_type_t protocol_detected = UNKNOWN;
char protocol_chosen_str[15] = "";

uint16_t custom_raw_buffer[NUM_OF_COMMANDS][NUM_OF_VALUES_PER_COMMAND];
uint8_t custom_raw_buffer_index = 0;

// Initialization - Transmitter
bool needtosend = false;
bool sending = false;
int16_t protocol_selected_num = UNKNOWN;

IRDaikinESP ac_daikin280(IR_TRANSMIT_PIN);
IRDaikin216 ac_daikin216(IR_TRANSMIT_PIN);
IRHitachiAc296 ac_hitachi296(IR_TRANSMIT_PIN);
IRVoltas ac_voltas(IR_TRANSMIT_PIN);
IRsend custom_ac(IR_TRANSMIT_PIN);

/**
 * @brief Function that deals with the locking feature
 * If locking is enabled, then it checks if the set temperature was within locking limits, if not it will
 * set the ac back to prev state.
 * If locking is not enabled, then it will send ack to cloud to let user know that someone controlled AC with remote
 * @param result_description_char_str - String containing info about AC remote control
 * @retval none
 */
void locking_feature(char *result_description_char_str)
{
    if (registered)
    {
        ESP_LOGI(DEBUG_TAG, "Sending Gwy Locking feature ack\r\n");
        uint8_t temperature = 0;
        if (protocol_detected == protocol_selected_num) // Someone tried to control AC
        {
            if (temperature > gwy_ac_control_t.TempUpLimit || temperature < gwy_ac_control_t.TempLowLimit)
            {
                // Someone controlled the AC using remote with exceeding temperature limits
                IR_transmit(protocol_selected_num);
            }
            add_to_pubmesg_queue(result_description_char_str, publish_topic);
        }
    }
}

/**
 * @brief Thread task that handles the IR signals received. Detects and sets the IR tranmsmission protocol
 * also takes care of the IR transmission part.
 * @param args
 * @return void*
 */
void IR_receiver_task(void *args)
{
    IR_transmit_setup();
    irrecv.setUnknownThreshold(kMinUnknownSize);
    irrecv.setTolerance(kTolerancePercentage);
    irrecv.enableIRIn();
    while (1)
    {
        if (needtosend)
            IR_transmit(protocol_selected_num);
        if (esp_restart_flag)
            ESP.restart();
        vTaskDelay(1);
        if (irrecv.decode(&results))
        {
            char raw_buf_str[200];
            strcpy(raw_buf_str, (char *)resultToHumanReadableBasic(&results, &protocol_detected).c_str());
            String description = IRAcUtils::resultAcToString(&results);
            char result_description_char_str[200];
            strcpy(result_description_char_str, (char *)description.c_str());
#if (IR_RECV_LOG_ENABLED)
            ESP_LOGI(DEBUG_TAG, "%s", raw_buf_str);
            if (description.length())
                ESP_LOGI(DEBUG_TAG, "%s", result_description_char_str);
#endif

            if (teaching_mode)
            {
                if (custom_raw_buffer_index < NUM_OF_COMMANDS)
                {
                    for (uint16_t i = 0; i < 600; i++)
                    {
                        custom_raw_buffer[custom_raw_buffer_index][i] = results.rawbuf[i];
                    }
                    custom_raw_buffer_index++;
                    if(custom_raw_buffer_index == NUM_OF_COMMANDS)
                    {
                        ESP_LOGI(DEBUG_TAG, "End of Teaching mode \n");
                        teaching_mode = false;
                    }
                }
            }

            if (protocol_detected != UNKNOWN && protocol_detected != UNUSED && registered && mqtt_connected && !teaching_mode)
            {
                configured = true;
                protocol_selected_num = protocol_detected;
                char pubmessage[PUBMESG_LEN];
                sprintf(pubmessage, "%s : %d, %s : %s, %s : %d, %s : %d",
                        JSON_PACKET_ID, GWY_CONF_PACKET,
                        JSON_ACK_NAME, GWY_CONF_ACK,
                        GWYSERNO_STR, GWY_SER_NO,
                        ERROR_CODE_STR, json_ack_err_code);
                ESP_LOGI(DEBUG_TAG, "Sending Gwy Configuration ack\r\n");
                add_to_pubmesg_queue(pubmessage, publish_topic);
            }
            if (protocol_detected == protocol_selected_num && gwy_ac_control_t.Locking)
                locking_feature(result_description_char_str);
            yield();
        }
    }
    vTaskDelete(NULL);
}

/**
 * @brief Funtion to setup the IR Transmit part
 * @param none
 * @retval none
 */
void IR_transmit_setup()
{
    pinMode(IR_TRANSMIT_PIN, OUTPUT);
    ac_daikin216.begin();
    ac_daikin280.begin();
    ac_hitachi296.begin();
    custom_ac.begin();
}

uint8_t get_mode_num_daikin216()
{
    if (strcasecmp(node_ac_control_t.mode_str, "Auto") == 0)
        return kDaikinAuto;
    else if (strcasecmp(node_ac_control_t.mode_str, "Dry") == 0)
        return kDaikinDry;
    else if (strcasecmp(node_ac_control_t.mode_str, "Cool") == 0)
        return kDaikinCool;
    else if (strcasecmp(node_ac_control_t.mode_str, "Heat") == 0)
        return kDaikinHeat;
    else
        return kDaikinFan;
}

/**
 * @brief Function that handles the IR transmission part
 * @param none
 * @retval none
 */
void IR_transmit(uint16_t protocol_selected_num)
{
    switch (protocol_selected_num)
    {
    default:
        printf("Error in choosing the protocol for send\r\n");
        break;
        
    case DAIKIN216:
        printf("Sending Daikin216\n");
        strcpy(protocol_chosen_str, "Daikin216");
        ac_daikin216.setPower(gwy_ac_control_t.power);
        ac_daikin216.setTemp(gwy_ac_control_t.temp);
        if (gwy_ac_control_t.swingH)
            gwy_ac_control_t.swingH = kDaikinSwingOn;
        ac_daikin216.setSwingHorizontal(gwy_ac_control_t.swingH);
        if (gwy_ac_control_t.swingV)
            gwy_ac_control_t.swingV = kDaikinSwingOn;
        ac_daikin216.setSwingVertical(gwy_ac_control_t.swingV);
        ac_daikin216.setFan(gwy_ac_control_t.fan);
        ac_daikin216.setMode(get_mode_num_daikin216());
        sending = true;
        ESP_LOGI(DEBUG_TAG, "Sending Daikin216\r\n");
        ac_daikin216.send();
        break;

    case HITACHI_AC296:
        strcpy(protocol_chosen_str, "Hitachi296");
        ac_hitachi296.setPower(gwy_ac_control_t.power);
        ac_hitachi296.setTemp(gwy_ac_control_t.temp);
        sending = true;
        ac_hitachi296.send();
        ESP_LOGI(DEBUG_TAG, "Sending Hitachi296\r\n");
        break;

    case DAIKIN200:
        ESP_LOGI(ERROR_TAG, "Still in Development\r\n");
        strcpy(protocol_chosen_str, "Daikin200");
        break;

    case DAIKIN:
        strcpy(protocol_chosen_str, "Daikin280");
        ac_daikin280.setPower(gwy_ac_control_t.power);
        ac_daikin280.setTemp(gwy_ac_control_t.temp);
        if (gwy_ac_control_t.swingH)
            gwy_ac_control_t.swingH = kDaikinSwingOn;
        ac_daikin280.setSwingHorizontal(gwy_ac_control_t.swingH);
        if (gwy_ac_control_t.swingV)
            gwy_ac_control_t.swingV = kDaikinSwingOn;
        ac_daikin280.setSwingVertical(gwy_ac_control_t.swingV);
        ac_daikin280.setFan(gwy_ac_control_t.fan);
        sending = true;
        ac_daikin280.send();
        ESP_LOGI(DEBUG_TAG, "Sending Daikin280\r\n");
        break;

    case VOLTAS:
        ESP_LOGE(ERROR_TAG, "Still in Development\r\n");
        ac_voltas.setPower(gwy_ac_control_t.power);
        ac_voltas.send();
        strcpy(protocol_chosen_str, "Daikin");
        break;

    }
    needtosend = false;
    sending = false;
}
