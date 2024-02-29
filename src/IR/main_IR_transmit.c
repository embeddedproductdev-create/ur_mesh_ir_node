/**
 * @file main_IR_send.c
 * @author Kulasekaran (kulasekaran@qmaxys.com)
 * @brief This file contains all functions related to the IR sending part
 * @version 0.1
 * @date 2024-02-29
 * @copyright Copyright (c) 2024
 */

#include "../../inc/IR/main_IR_send.h"

#define IR_TRANSMIT_PIN 7

//Initialization
bool sending = false;
bool needtosend = false;

IRDaikinESP ac_daikin280(IR_TRANSMIT_PIN);
IRDaikin216 ac_daikin216(IR_TRANSMIT_PIN);
IRHitachiAc296 ac_hitachi296(IR_TRANSMIT_PIN);
IRVoltas ac_voltas(IR_TRANSMIT_PIN);

/**
 * @brief Funtion to setup the IR Transmit part
 * @param none
 * @retval none
 */
void IR_transmit_setup()
{
    pinMode(USER_SWITCH, INPUT);
    ac_daikin216.begin();
    ac_daikin280.begin();
    ac_hitachi296.begin();
}
/**
 * @brief Function that handles the IR transmission part
 * @param none
 * @retval none
 */
void IR_transmit()
{
    sending = true;
    switch(protocol_detected)
    {
        default:
            printf("Error in choosing the protocol for send\r\n");
            return;
        case DAIKIN216:
            strcpy(protocol_chosen, "Daikin216");
            ac_daikin216.setPower(ac_control_t.power);
            ac_daikin216.setTemp(ac_control_t.temp);
            if(ac_control_t.swingH) ac_control_t.swingH = kDaikinSwingOn;
            ac_daikin216.setSwingHorizontal(ac_control_t.swingH);
            if(ac_control_t.swingV) ac_control_t.swingV = kDaikinSwingOn;
            ac_daikin216.setSwingVertical(ac_control_t.swingV);
            ac_daikin216.setFan(ac_control_t.fan);
            ac_daikin216.send();
            printf("Sending Daikin216 with Power = %d\r\n",ac_control_t.power);
            break;
        case DAIKIN200:
            strcpy(protocol_chosen, "Daikin200");
            printf("\r\n");
            return;
        case DAIKIN:
            strcpy(protocol_chosen, "Daikin280");
            ac_daikin280.setPower(ac_control_t.power);
            ac_daikin280.setTemp(ac_control_t.temp);
            if(ac_control_t.swingH) ac_control_t.swingH = kDaikinSwingOn;
            ac_daikin280.setSwingHorizontal(ac_control_t.swingH);
            if(ac_control_t.swingV) ac_control_t.swingV = kDaikinSwingOn;
            ac_daikin280.setSwingVertical(ac_control_t.swingV);
            ac_daikin280.setFan(ac_control_t.fan);
            ac_daikin280.send();
            printf("Sending Daikin280\r\n");
            break;
        case HITACHI_AC296:
            strcpy(protocol_chosen, "Hitachi296");
            ac_hitachi296.setPower(ac_control_t.power);
            ac_hitachi296.setTemp(ac_control_t.temp);
            // ac_hitachi296.setSwingHorizontal(ac_control_t.swingH);
            // ac_hitachi296.setSwingVertical(ac_control_t.swingV);
            ac_hitachi296.setFan(ac_control_t.fan-3);
            ac_hitachi296.send();
            printf("Sending Hitachi296\r\n");
            break;
        case VOLTAS:
            strcpy(protocol_chosen, "Voltas");
            ac_voltas.send();
            printf("Protcol Chosen Voltas\r\n");
            break;
    }
    sending = false;
    needtosend = false;
}