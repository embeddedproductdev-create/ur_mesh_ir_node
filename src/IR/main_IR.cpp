/**
 * @file main_IR_recv.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions related to the IR receiver part
 * @version 0.1
 * @date 2024-02-29
 * @copyright Copyright (c) 2024
 */

#include "../../inc/IR/main_IR.h"
#include "../../inc/Mesh/mesh_main.h"


//Initialization - Receiver
bool configured = false;
IRrecv irrecv(IR_RECEIVER_PIN, RECV_BUFFER_SIZE, kTimeout, true);
decode_results results;
decode_type_t protocol_detected = UNKNOWN;
char protocol_chosen_str[15] = "";

//Initialization - Transmitter
bool needtosend = false;
bool sending = false;
int16_t protocol_selected_num = UNKNOWN;

IRDaikinESP ac_daikin280(IR_TRANSMIT_PIN);
IRDaikin216 ac_daikin216(IR_TRANSMIT_PIN);
IRHitachiAc296 ac_hitachi296(IR_TRANSMIT_PIN);
IRVoltas ac_voltas(IR_TRANSMIT_PIN);

//Initialization - BUTTON
bool last_button_state = 0;
bool current_button_state = 0;
uint32_t pressedTime = 0;
uint32_t releasedTime = 0;
uint32_t pressduration = 0;
bool esp_restart_flag = false;

/**
 * @brief Thread that handles the IR signals received. Detects and sets the IR tranmsmission protocol
 * @param args
 * @return void*
 */
void *IR_receiver_task(void *args)
{
    IR_transmit_setup();
    Serial.begin(BAUD_RATE);
    while(!Serial)
        delay(50);
    #if(IR_RECV_LOG_ENABLED)
        Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", IR_RECEIVER_PIN);
    #endif
    irrecv.setUnknownThreshold(kMinUnknownSize);
    irrecv.setTolerance(kTolerancePercentage);
    irrecv.enableIRIn();

    while(1)
    {
        vTaskDelay(1);
        if(esp_restart_flag)
            ESP.restart();
        if (irrecv.decode(&results)) {
            #if(IR_RECV_LOG_ENABLED)
                // Serial.println(D_STR_TIMESTAMP " : %06lu.%03lu\n", now / 1000, now % 1000);
            #endif

            if (results.overflow)
            {
                #if(IR_RECV_LOG_ENABLED)
                // Serial.println(D_WARN_BUFFERFULL "\n", RECV_BUFFER_SIZE);
                #endif
            }

            #if(IR_RECV_LOG_ENABLED)
                Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_STR "\n");
            #endif

            if (kTolerancePercentage != kTolerance)
            {
            #if(IR_RECV_LOG_ENABLED)
                Serial.printf(D_STR_TOLERANCE " : %d%%\n", kTolerancePercentage);
            #endif
            }

            #if(IR_RECV_LOG_ENABLED)
                const char *result_char_str = (resultToHumanReadableBasic(&results, &protocol_detected)).c_str();
                ESP_LOGI(DEBUG_TAG, "%s",result_char_str);
                String description = IRAcUtils::resultAcToString(&results);
                const char *result_description_char_str = description.c_str();
                if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
                ESP_LOGI(DEBUG_TAG, "%s", result_description_char_str);

            #endif

            if(protocol_detected != UNKNOWN && protocol_detected != UNUSED && registered)
            {
                configured = true;
                protocol_selected_num = protocol_detected;
            }
            printf("protocol_detected : %d\n",protocol_detected);
            yield();
            Serial.println(resultToSourceCode(&results));
            Serial.println();
            yield();
        }
    }
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
}

/**
 * @brief Function that handles the IR transmission part
 * @param none
 * @retval none
 */
void IR_transmit(uint16_t protocol_selected_num, char *protocol_chosen_str)
{
    switch(protocol_selected_num)
    {
        default:
            printf("Error in choosing the protocol for send\r\n");
            break;

        case DAIKIN216:
            strcpy(protocol_chosen_str, "Daikin216");
            ac_daikin216.setPower(gwy_ac_control_t.power);
            ac_daikin216.setTemp(gwy_ac_control_t.temp);
            if(gwy_ac_control_t.swingH) gwy_ac_control_t.swingH = kDaikinSwingOn;
            ac_daikin216.setSwingHorizontal(gwy_ac_control_t.swingH);
            if(gwy_ac_control_t.swingV) gwy_ac_control_t.swingV = kDaikinSwingOn;
            ac_daikin216.setSwingVertical(gwy_ac_control_t.swingV);
            ac_daikin216.setFan(gwy_ac_control_t.fan);
            sending = true;
            ac_daikin216.send();
            break;

        case DAIKIN200:
            ESP_LOGI(ERROR_TAG, "Still in Development\r\n");
            strcpy(protocol_chosen_str, "Daikin200");
            break;

        case DAIKIN:
            strcpy(protocol_chosen_str, "Daikin280");
            ac_daikin280.setPower(gwy_ac_control_t.power);
            ac_daikin280.setTemp(gwy_ac_control_t.temp);
            if(gwy_ac_control_t.swingH) gwy_ac_control_t.swingH = kDaikinSwingOn;
            ac_daikin280.setSwingHorizontal(gwy_ac_control_t.swingH);
            if(gwy_ac_control_t.swingV) gwy_ac_control_t.swingV = kDaikinSwingOn;
            ac_daikin280.setSwingVertical(gwy_ac_control_t.swingV);
            ac_daikin280.setFan(gwy_ac_control_t.fan);
            sending = true;
            ac_daikin280.send();
            ESP_LOGI(DEBUG_TAG, "Sending Daikin280\r\n");
            break;

        case HITACHI_AC296:
            strcpy(protocol_chosen_str, "Hitachi296");
            ac_hitachi296.setPower(gwy_ac_control_t.power);
            ac_hitachi296.setTemp(gwy_ac_control_t.temp);
            ac_hitachi296.setFan(gwy_ac_control_t.fan);
            sending = true;
            ac_hitachi296.send();
            ESP_LOGI(DEBUG_TAG, "Sending Hitachi296\r\n");
            break;

        case VOLTAS:
            strcpy(protocol_chosen_str, "Voltas");
            sending = true;
            ac_voltas.send();
            ESP_LOGI(DEBUG_TAG, "Protcol Chosen Voltas\r\n");
            break;
    }
    needtosend = false;
    sending = false;
}

/**
 * @brief Thread that handles the Button press
 * @param args
 * @return void*
 */
void *button_task(void *args)
{
    pinMode(USER_SWITCH, INPUT);
    while(1)
    {
        vTaskDelay(1);
        if(!digitalRead(USER_SWITCH)) //button is pressed
        {
            pressedTime = esp_timer_get_time();
            while(!digitalRead(USER_SWITCH)) //Do nothing until button is released
            {
                vTaskDelay(1);
                ;
            }
            releasedTime = esp_timer_get_time();
            if((releasedTime - pressedTime)/1000 > SHORT_PRESS_DURATION_MS)
                esp_restart_flag = true;
            else
                send_control_packet = true;
        }
    }
}


