/**
 * @file main_IR_recv.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions related to the IR receiver part
 * @version 0.8
 * @date 2024-06-19
 * @copyright Copyright (c) 2024
 */
#include "../../inc/IR/main_IR.h"
#include "../../inc/LTE/LTE.h"
#include "../../inc/LTE/mqtt.h"
#include "../../inc/Custom/button.h"
#include "../../inc/Custom/printf_custom.h"

// Initialization - Receiver
IRrecv irrecv(IR_RECEIVER_PIN, RECV_BUFFER_SIZE, kTimeout, true);
decode_results results;
decode_type_t protocol_detected = UNKNOWN;
bool configured = false;
bool teaching_mode = false;
bool teaching_mode_done = false;
char protocol_chosen_str[15] = "";

// Initializaiton - Teaching mode
uint8_t convert_buffer[2];
uint16_t convert_data = 0;
uint16_t eeprom_addr = 0;
uint16_t eeprom_addr_cal = 0;
uint16_t custom_raw_buffer[NUM_OF_VALUES_PER_COMMAND];
uint8_t custom_raw_buffer_index = 0;
uint8_t temp_min_val = 19;
uint8_t temp_max_val = 28;
bool storing_IR_data_to_flash = 0;
bool teachMode_size_done = 0;

// Initialization - Transmitter
bool needToSendIRComamnd = false;
bool sending = false;
int16_t protocol_selected_num = UNKNOWN;

IRDaikinESP ac_daikin280(IR_TRANSMIT_PIN);
IRDaikin216 ac_daikin216(IR_TRANSMIT_PIN);
IRDaikin2 ac_daikin2(IR_TRANSMIT_PIN);
IRDaikin160 ac_daikin160(IR_TRANSMIT_PIN);
IRDaikin176 ac_daikin176(IR_TRANSMIT_PIN);
IRDaikin200 ac_daikin200(IR_TRANSMIT_PIN);
IRDaikin64 ac_daikinac64(IR_TRANSMIT_PIN);
IRDaikin152 ac_daikin152(IR_TRANSMIT_PIN);
IRDaikin128 ac_daikin128(IR_TRANSMIT_PIN);
IRHitachiAc ac_hitachi224(IR_TRANSMIT_PIN);
IRHitachiAc1 ac_hitachi104(IR_TRANSMIT_PIN);
IRHitachiAc424 ac_hitachi424(IR_TRANSMIT_PIN);
IRHitachiAc344 ac_hitachi344(IR_TRANSMIT_PIN);
IRHitachiAc264 ac_hitachi264(IR_TRANSMIT_PIN);
IRHitachiAc296 ac_hitachi296(IR_TRANSMIT_PIN);
IRVoltas ac_voltas(IR_TRANSMIT_PIN);
IRSamsungAc ac_samsung(IR_TRANSMIT_PIN);
IRHaierAC ac_haier(IR_TRANSMIT_PIN);
IRHaierAC176 ac_haier176(IR_TRANSMIT_PIN);
IRHaierAC160 ac_haier160(IR_TRANSMIT_PIN);
IRLgAc ac_lg(IR_TRANSMIT_PIN);
IRToshibaAC ac_toshiba(IR_TRANSMIT_PIN);
IRCarrierAc64 ac_carrier64(IR_TRANSMIT_PIN);
IRMitsubishi112 ac_mitsubishi112(IR_TRANSMIT_PIN);
IRMitsubishi136 ac_mitsubishi136(IR_TRANSMIT_PIN);
IRMitsubishiAC ac_mitsubishi144(IR_TRANSMIT_PIN);
IRMitsubishiHeavy88Ac ac_mitsubishi88(IR_TRANSMIT_PIN);
IRMitsubishiHeavy152Ac ac_mitsubishi152(IR_TRANSMIT_PIN);
IRsend ac_custom(IR_TRANSMIT_PIN);

/**
 * @brief Funtion to setup the IR Transmit part
 * @param none
 * @retval none
 */
void IR_transmit_setup()
{
    ac_daikin216.begin();
    ac_daikin280.begin();
    ac_daikin2.begin();
    ac_daikin160.begin();
    ac_daikin176.begin();
    ac_daikin200.begin();
    ac_daikinac64.begin();
    ac_daikin152.begin();
    ac_daikin128.begin();
    ac_hitachi296.begin();
    ac_hitachi224.begin();
    ac_hitachi104.begin();
    ac_hitachi424.begin();
    ac_hitachi344.begin();
    ac_hitachi264.begin();
    ac_voltas.begin();
    ac_samsung.begin();
    ac_haier.begin();
    ac_haier176.begin();
    ac_haier160.begin();
    ac_lg.begin();
    ac_toshiba.begin();
    ac_carrier64.begin();
    ac_mitsubishi112.begin();
    ac_mitsubishi136.begin();
    ac_mitsubishi144.begin();
    ac_mitsubishi88.begin();
    ac_mitsubishi152.begin();
    ac_custom.begin();
}

void IR_transmit(uint16_t protocol)
{
    switch (protocol)
    {
    case RAW:
        strcpy(protocol_chosen_str, "RAW");
        eeprom_read(EEPROM_SLAVE_ADDR, TEACH_DATA_LEN, convert_buffer, 2);
        convert_data = convert_8bit_to_16bit(convert_buffer);
        sprintf(ir_log_buffer, "Size of data : %d\t", convert_data);
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        eeprom_addr = TEACH_DATA_POFF;
        if (gwy_ac_control_t.power)
        {
            if (gwy_ac_control_t.temp > FETCH_ADDR_LOW)
                eeprom_addr = (TEACH_DATA_POFF + (MAX_OFFSET * (gwy_ac_control_t.temp - FETCH_ADDR_LOW)));
            else
                ESP_LOGE(IR_ERROR_TAG, "Not an valid temperature\r\n");
        }
        sprintf(ir_log_buffer, "EEPROM ADDR : %d\n", eeprom_addr);
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        if ((convert_data <= MAX_OFFSET) && (eeprom_addr <= TEACH_DATA_PON_32C) && ((gwy_ac_control_t.temp >= TEACHING_MODE_STARTING_TEMPERATURE) && ((gwy_ac_control_t.temp <= TEACHING_MODE_ENDING_TEMPERATURE) || !gwy_ac_control_t.power)))
        {
            read_from_memory(custom_raw_buffer, convert_data, eeprom_addr);
            ac_custom.sendRaw(custom_raw_buffer, convert_data / 2, 41);
        }
        else
        {
            sprintf(ir_log_buffer, "Memory is not configured correctly or data invalid!!!\r\n");
            white_printf(IR_DEBUG_TAG, ir_log_buffer);
        }
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
        ac_daikin280.enableOffTimer(gwy_ac_control_t.OffTimer);
        ac_daikin280.enableOnTimer(gwy_ac_control_t.OnTimer);
        ac_daikin280.setMode(ac_daikin280.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_daikin280.send();
        sprintf(ir_log_buffer, "Sending Daikin280");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case DAIKIN200:
        strcpy(protocol_chosen_str, "Daikin200");
        ac_daikin200.setPower(gwy_ac_control_t.power);
        ac_daikin200.setSwingHorizontal(gwy_ac_control_t.swingH);
        ac_daikin200.setFan(gwy_ac_control_t.fan);
        ac_daikin200.setTemp(gwy_ac_control_t.temp);
        ac_daikin200.setMode(ac_daikin200.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_daikin200.send();
        ESP_LOGI(IR_DEBUG_TAG, "Sending Daikin200\r\n");
        break;

    case DAIKIN216:
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
        ac_daikin216.setMode(ac_daikin216.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_daikin216.send();
        sprintf(ir_log_buffer, "Sending Daikin216");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case DAIKIN2:
        strcpy(protocol_chosen_str, "Daikin2");
        ac_daikin2.setPower(gwy_ac_control_t.power);
        ac_daikin2.setTemp(gwy_ac_control_t.temp);
        if (gwy_ac_control_t.swingH)
            ac_daikin2.setSwingHorizontal(kDaikin2SwingHAuto);
        else
            ac_daikin2.setSwingHorizontal(kDaikin2SwingHOff);
        if (gwy_ac_control_t.swingV)
            ac_daikin2.setSwingVertical(kDaikin2SwingVAuto);
        else
            ac_daikin2.setSwingVertical(kDaikin2SwingVOff);
        ac_daikin2.setFan(gwy_ac_control_t.fan);
        ac_daikin2.enableOffTimer(gwy_ac_control_t.OffTimer);
        ac_daikin2.enableOnTimer(gwy_ac_control_t.OnTimer);
        ac_daikin2.setMode(ac_daikin2.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_daikin2.send();
        sprintf(ir_log_buffer, "Sending Daikin2");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case DAIKIN160:
        strcpy(protocol_chosen_str, "Daikin160");
        ac_daikin160.setPower(gwy_ac_control_t.power);
        ac_daikin160.setTemp(gwy_ac_control_t.temp);
        if (gwy_ac_control_t.swingV)
            ac_daikin160.setSwingVertical(kDaikin160SwingVAuto);
        ac_daikin160.setFan(gwy_ac_control_t.fan);
        ac_daikin160.setMode(ac_daikin160.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_daikin160.send();
        sprintf(ir_log_buffer, "Sending Daikin160");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case DAIKIN176:
        strcpy(protocol_chosen_str, "Daikin176");
        ac_daikin176.setPower(gwy_ac_control_t.power);
        ac_daikin176.setTemp(gwy_ac_control_t.temp);
        if (gwy_ac_control_t.swingH)
            ac_daikin176.setSwingHorizontal(kDaikin176SwingHAuto);
        else
            ac_daikin176.setSwingHorizontal(kDaikin176SwingHOff);
        ac_daikin176.setFan(gwy_ac_control_t.fan);
        ac_daikin176.setMode(ac_daikin176.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_daikin176.send();
        sprintf(ir_log_buffer, "Sending Daikin176");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case DAIKIN64:
        strcpy(protocol_chosen_str, "Daikin64");
        ac_daikinac64.setPowerToggle(gwy_ac_control_t.power);
        ac_daikinac64.setTemp(gwy_ac_control_t.temp);
        ac_daikinac64.setSwingVertical(gwy_ac_control_t.swingV);
        ac_daikinac64.setFan(gwy_ac_control_t.fan);
        ac_daikinac64.setOnTime(gwy_ac_control_t.OffTimer);
        ac_daikinac64.setOffTime(gwy_ac_control_t.OnTimer);
        ac_daikinac64.setMode(ac_daikinac64.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_daikinac64.send();
        sprintf(ir_log_buffer, "Sending Daikin64");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case DAIKIN152:
        strcpy(protocol_chosen_str, "Daikin152");
        ac_daikin152.setPower(gwy_ac_control_t.power);
        ac_daikin152.setTemp(gwy_ac_control_t.temp);
        ac_daikin152.setSwingV(gwy_ac_control_t.swingV);
        ac_daikin152.setFan(gwy_ac_control_t.fan);
        ac_daikin152.setMode(ac_daikin152.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_daikin152.send();
        sprintf(ir_log_buffer, "Sending Daikin152");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case DAIKIN128:
        strcpy(protocol_chosen_str, "Daikin128");
        ac_daikin128.setPowerToggle(gwy_ac_control_t.power);
        ac_daikin128.setTemp(gwy_ac_control_t.temp);
        ac_daikin128.setSwingVertical(gwy_ac_control_t.swingV);
        ac_daikin128.setFan(gwy_ac_control_t.fan);
        ac_daikin128.setOffTimer(gwy_ac_control_t.OffTimer);
        ac_daikin128.setOnTimer(gwy_ac_control_t.OnTimer);
        ac_daikin128.setMode(ac_daikin128.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_daikin128.send();
        sprintf(ir_log_buffer, "Sending Daikin128");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HITACHI_AC296:
        strcpy(protocol_chosen_str, "Hitachi296");
        ac_hitachi296.setPower(gwy_ac_control_t.power);
        ac_hitachi296.setTemp(gwy_ac_control_t.temp);
        ac_hitachi296.setFan(gwy_ac_control_t.fan);
        ac_hitachi296.setMode(ac_hitachi296.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_hitachi296.send();
        sprintf(ir_log_buffer, "Sending Hitachi296");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HITACHI_AC:
        strcpy(protocol_chosen_str, "HitachiAc224");
        ac_hitachi224.setPower(gwy_ac_control_t.power);
        ac_hitachi224.setTemp(gwy_ac_control_t.temp);
        ac_hitachi224.setFan(gwy_ac_control_t.fan);
        ac_hitachi224.setSwingHorizontal(gwy_ac_control_t.swingH);
        ac_hitachi224.setSwingVertical(gwy_ac_control_t.swingV);
        ac_hitachi224.setMode(ac_hitachi224.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_hitachi224.send();
        sprintf(ir_log_buffer, "Sending HitachiAc224");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HITACHI_AC1:
        strcpy(protocol_chosen_str, "HitachiAc104");
        ac_hitachi104.setPower(gwy_ac_control_t.power);
        ac_hitachi104.setTemp(gwy_ac_control_t.temp);
        ac_hitachi104.setFan(gwy_ac_control_t.fan);
        ac_hitachi104.setSwingH(gwy_ac_control_t.swingH);
        ac_hitachi104.setSwingV(gwy_ac_control_t.swingV);
        ac_hitachi104.setOffTimer(gwy_ac_control_t.OffTimer);
        ac_hitachi104.setOnTimer(gwy_ac_control_t.OnTimer);
        ac_hitachi104.setMode(ac_hitachi104.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_hitachi104.send();
        sprintf(ir_log_buffer, "Sending HitachiAc104");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HITACHI_AC424:
        strcpy(protocol_chosen_str, "HitachiAc424");
        ac_hitachi424.setPower(gwy_ac_control_t.power);
        ac_hitachi424.setTemp(gwy_ac_control_t.temp);
        ac_hitachi424.setFan(gwy_ac_control_t.fan);
        ac_hitachi424.setSwingVToggle(gwy_ac_control_t.swingV);
        ac_hitachi424.setMode(ac_hitachi424.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_hitachi424.send();
        sprintf(ir_log_buffer, "Sending HitachiAc424");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HITACHI_AC344:
        strcpy(protocol_chosen_str, "HitachiAc344");
        ac_hitachi344.setSwingH(gwy_ac_control_t.swingH);
        ac_hitachi344.setSwingV(gwy_ac_control_t.swingV);
        ac_hitachi344.send();
        sprintf(ir_log_buffer, "Sending HitachiAc344");
        break;

    case HITACHI_AC264:
        strcpy(protocol_chosen_str, "HitachiAc264");
        ac_hitachi264.setFan(gwy_ac_control_t.fan);
        ac_hitachi264.send();
        sprintf(ir_log_buffer, "Sending HitachiAc264");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case VOLTAS:
        strcpy(protocol_chosen_str, "Voltas");
        ac_voltas.setPower(gwy_ac_control_t.power);
        ac_voltas.setTemp(gwy_ac_control_t.temp);
        ac_voltas.setSwingH(gwy_ac_control_t.swingH);
        ac_voltas.setSwingV(gwy_ac_control_t.swingV);
        ac_voltas.setFan(gwy_ac_control_t.fan);
        ac_voltas.setOffTime(gwy_ac_control_t.OffTimer);
        ac_voltas.setOnTime(gwy_ac_control_t.OnTimer);
        ac_voltas.setMode(ac_voltas.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_voltas.send();
        sprintf(ir_log_buffer, "Sending Voltas\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case SAMSUNG_AC:
        strcpy(protocol_chosen_str, "Samsung");
        ac_samsung.setPower(gwy_ac_control_t.power);
        ac_samsung.setTemp(gwy_ac_control_t.temp);
        ac_samsung.setSwingH(gwy_ac_control_t.swingH);
        ac_samsung.setSwing(gwy_ac_control_t.swingV);
        ac_samsung.setFan(gwy_ac_control_t.fan);
        ac_samsung.setOffTimer(gwy_ac_control_t.OffTimer);
        ac_samsung.setOnTimer(gwy_ac_control_t.OnTimer);
        ac_samsung.setMode(ac_samsung.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_samsung.send();
        sprintf(ir_log_buffer, "Sending Samsung\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HAIER_AC:
        strcpy(protocol_chosen_str, "Haier");
        // Regarding Power on/off and horizontal swing control we don't have library support
        ac_haier.setTemp(gwy_ac_control_t.temp);
        ac_haier.setSwingV(gwy_ac_control_t.swingV);
        ac_haier.setFan(gwy_ac_control_t.fan);
        ac_haier.setOffTimer(gwy_ac_control_t.OffTimer);
        ac_haier.setOnTimer(gwy_ac_control_t.OnTimer);
        ac_haier.setMode(ac_haier.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_haier.send();
        sprintf(ir_log_buffer, "Sending Haier\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HAIER_AC176:
        strcpy(protocol_chosen_str, "Haier176");
        ac_haier176.setPower(gwy_ac_control_t.power);
        ac_haier176.setTemp(gwy_ac_control_t.temp);
        ac_haier176.setSwingH(gwy_ac_control_t.swingH);
        ac_haier176.setSwingV(gwy_ac_control_t.swingV);
        ac_haier176.setFan(gwy_ac_control_t.fan);
        ac_haier176.setOffTimer(gwy_ac_control_t.OffTimer);
        ac_haier176.setOnTimer(gwy_ac_control_t.OnTimer);
        ac_haier176.setMode(ac_haier176.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_haier176.send();
        sprintf(ir_log_buffer, "Sending Haier176\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HAIER_AC160:
        // Regarding horizontal swing we don't have library support
        strcpy(protocol_chosen_str, "Haier160");
        ac_haier160.setPower(gwy_ac_control_t.power);
        ac_haier160.setTemp(gwy_ac_control_t.temp);
        ac_haier160.setSwingV(gwy_ac_control_t.swingV);
        ac_haier160.setFan(gwy_ac_control_t.fan);
        ac_haier160.setOffTimer(gwy_ac_control_t.OffTimer);
        ac_haier160.setOnTimer(gwy_ac_control_t.OnTimer);
        ac_haier160.setMode(ac_haier160.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_haier160.send();
        sprintf(ir_log_buffer, "Sending Haier160\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case CARRIER_AC64:
        // Regarding horizontal swing we don't have library support
        strcpy(protocol_chosen_str, "CarrierAC64");
        ac_carrier64.setPower(gwy_ac_control_t.power);
        ac_carrier64.setTemp(gwy_ac_control_t.temp);
        ac_carrier64.setSwingV(gwy_ac_control_t.swingV);
        ac_carrier64.setFan(gwy_ac_control_t.fan);
        ac_carrier64.setOffTimer(gwy_ac_control_t.OffTimer);
        ac_carrier64.setOnTimer(gwy_ac_control_t.OnTimer);
        ac_carrier64.setMode(ac_carrier64.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_carrier64.send();
        sprintf(ir_log_buffer, "Sending CarrierAC64\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case LG2:
    case LG:
        // Regarding on/off timer we don't have library support
        strcpy(protocol_chosen_str, "LG");
        ac_lg.setPower(gwy_ac_control_t.power);
        ac_lg.setTemp(gwy_ac_control_t.temp);
        ac_lg.setSwingH(gwy_ac_control_t.swingH);
        ac_lg.setSwingV(gwy_ac_control_t.swingV);
        ac_lg.setFan(gwy_ac_control_t.fan);
        ac_lg.setMode(ac_lg.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_lg.send();
        sprintf(ir_log_buffer, "Sending LG\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case TOSHIBA_AC:
        // Regarding on/off timer we don't have library support
        strcpy(protocol_chosen_str, "Toshiba");
        ac_toshiba.setPower(gwy_ac_control_t.power);
        ac_toshiba.setTemp(gwy_ac_control_t.temp);
        ac_toshiba.setFan(gwy_ac_control_t.fan);
        if (gwy_ac_control_t.swingV || gwy_ac_control_t.swingH)
        {
            /*Library seems like they only do swing on/off so that only i am having an
            condition check for both horizontal and vertical*/
            ac_toshiba.setSwing(kToshibaAcSwingOn);
        }
        else
        {
            ac_toshiba.setSwing(kToshibaAcSwingOff);
        }
        ac_toshiba.setMode(ac_toshiba.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_toshiba.send();
        sprintf(ir_log_buffer, "Sending Toshiba\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case MITSUBISHI112:
        // Regarding on/off timer we don't have library support
        strcpy(protocol_chosen_str, "Mitsubishi112");
        ac_mitsubishi112.setPower(gwy_ac_control_t.power);
        ac_mitsubishi112.setTemp(gwy_ac_control_t.temp);
        ac_mitsubishi112.setSwingH(gwy_ac_control_t.swingH);
        ac_mitsubishi112.setSwingV(gwy_ac_control_t.swingV);
        ac_mitsubishi112.setFan(gwy_ac_control_t.fan);
        ac_mitsubishi112.setMode(ac_mitsubishi112.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_mitsubishi112.send();
        sprintf(ir_log_buffer, "Sending Mitsubishi112\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case MITSUBISHI136:
        // Regarding on/off timer and horizontal swing we don't have library support
        strcpy(protocol_chosen_str, "Mitsubishi136");
        ac_mitsubishi136.setPower(gwy_ac_control_t.power);
        ac_mitsubishi136.setTemp(gwy_ac_control_t.temp);
        if (gwy_ac_control_t.swingV)
        {
            ac_mitsubishi136.setSwingV(kMitsubishi136SwingVAuto);
        }
        ac_mitsubishi136.setFan(gwy_ac_control_t.fan);
        ac_mitsubishi136.setMode(ac_mitsubishi136.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_mitsubishi136.send();
        sprintf(ir_log_buffer, "Sending Mitsubishi136\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case MITSUBISHI_AC:
        // Regarding on/off timer and  swing we don't have library support
        strcpy(protocol_chosen_str, "MitsubishiAc");
        ac_mitsubishi144.setPower(gwy_ac_control_t.power);
        ac_mitsubishi144.setTemp(gwy_ac_control_t.temp);
        ac_mitsubishi144.setFan(gwy_ac_control_t.fan);
        ac_mitsubishi144.setMode(ac_mitsubishi144.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_mitsubishi144.send();
        sprintf(ir_log_buffer, "Sending MitsubishiAc\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case MITSUBISHI_HEAVY_88:
        // Regarding on/off timer we don't have library support
        strcpy(protocol_chosen_str, "MitsubisiHvy88");
        ac_mitsubishi88.setPower(gwy_ac_control_t.power);
        ac_mitsubishi88.setTemp(gwy_ac_control_t.temp);
        ac_mitsubishi88.setFan(gwy_ac_control_t.fan);
        if (gwy_ac_control_t.swingH)
            ac_mitsubishi88.setSwingHorizontal(kMitsubishiHeavy88SwingHAuto);
        else
            ac_mitsubishi88.setSwingHorizontal(kMitsubishiHeavy88SwingHOff);
        if (gwy_ac_control_t.swingV)
            ac_mitsubishi88.setSwingVertical(kMitsubishiHeavy88SwingVAuto);
        else
            ac_mitsubishi88.setSwingVertical(kMitsubishiHeavy88SwingVOff);
        ac_mitsubishi88.setMode(ac_mitsubishi88.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_mitsubishi88.send();
        sprintf(ir_log_buffer, "Sending MitsubishiHeavy88\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case MITSUBISHI_HEAVY_152:
        // Regarding on/off timer we don't have library support
        strcpy(protocol_chosen_str, "MitsbisiHvy152");
        ac_mitsubishi152.setPower(gwy_ac_control_t.power);
        ac_mitsubishi152.setTemp(gwy_ac_control_t.temp);
        ac_mitsubishi152.setFan(gwy_ac_control_t.fan);
        if (gwy_ac_control_t.swingH)
            ac_mitsubishi152.setSwingHorizontal(kMitsubishiHeavy88SwingHAuto);
        else
            ac_mitsubishi152.setSwingHorizontal(kMitsubishiHeavy88SwingHOff);
        if (gwy_ac_control_t.swingV)
            ac_mitsubishi88.setSwingVertical(kMitsubishiHeavy88SwingVAuto);
        else
            ac_mitsubishi152.setSwingVertical(kMitsubishiHeavy88SwingVOff);
        ac_mitsubishi152.setMode(ac_mitsubishi152.convertMode((stdAc::opmode_t)gwy_ac_control_t.mode_val));
        ac_mitsubishi152.send();
        sprintf(ir_log_buffer, "Sending MitsubishiHeavy152\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    default:
        printf("Error in choosing the protocol for send");
        break;
    }
    needToSendIRComamnd = false;
}

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
    uint8_t temperature = 0;
    char temperature_in_string[10] = "";
    if (strstr(result_description_char_str, "Temp: "))
    {
        temperature_in_string[0] = *((strstr(result_description_char_str, "Temp: ")) + 6);
        temperature_in_string[1] = *((strstr(result_description_char_str, "Temp: ")) + 7);
        temperature = atoi(temperature_in_string);
        sprintf(ir_log_buffer, "Manually set Temperature (in string) : %s", temperature_in_string);
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        sprintf(ir_log_buffer, "Manually set Temperature (in int) : %d", temperature);
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
    }
#if (IS_GWY)
    white_printf(IR_DEBUG_TAG, "Sending Gwy AC Manual control ack");
    add_to_pubmesg_queue(result_description_char_str, publish_topic);
#endif
#if (!IS_GWY)
    send_locking_feature_ack_to_gwy(result_description_char_str);
#endif
    if (temperature != 0 && (temperature > gwy_ac_control_t.TempLockUpLimit || temperature < gwy_ac_control_t.TempLockLowLimit))
    {
        white_printf(IR_DEBUG_TAG, "Someone manually controlled the AC ... beyond limits ... reverting ");
        needToSendIRComamnd = true;
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
        vTaskDelay(1);
        if (needToSendIRComamnd) {
            IR_transmit(protocol_selected_num);
            sleep(1); // let's waste a second here and see if it avoid locking feature getting triggered
            continue;
        }
        if (esp_restart_flag)
            ESP.restart();
        if (irrecv.decode(&results))
        {
            char raw_buf_str[200];
            strcpy(raw_buf_str, (char *)resultToHumanReadableBasic(&results, &protocol_detected).c_str());
            String description = IRAcUtils::resultAcToString(&results);
            char result_description_char_str[200];
            strcpy(result_description_char_str, (char *)description.c_str());
            printf("IR RAW VALUES : { ");
            for (uint16_t i = 0; i < results.rawlen; i++)
            {
                printf("%d, ", results.rawbuf[i]);
            }
            printf("}\n");
            sprintf(ir_log_buffer, "%s", raw_buf_str);
            white_printf(IR_DEBUG_TAG, ir_log_buffer);
            if (description.length())
            {
                sprintf(ir_log_buffer, "%s", result_description_char_str);
                white_printf(IR_DEBUG_TAG, ir_log_buffer);
            }

            if (teaching_mode){
                protocol_selected_num = RAW;
                if(teachMode_size_done){
                    eeprom_addr_cal=0;
                    eeprom_write_byte(EEPROM_SLAVE_ADDR,EEPROM_CONF_FAC,TEACHING_FAC);
                    vTaskDelay(20/portTICK_PERIOD_MS);
                    if(results.rawlen>1)    convert_data=((results.rawlen)-1)*2;
                    convert_16bit_to_8bit(convert_data,convert_buffer);
                    eeprom_addr=TEACH_DATA_LEN;
                    eeprom_write(EEPROM_SLAVE_ADDR,eeprom_addr,convert_buffer,2);
                    vTaskDelay(20/portTICK_PERIOD_MS);
                    teachMode_size_done=false;
                }
                if(!eeprom_addr_cal) { 
                    eeprom_addr=TEACH_DATA_POFF;
                }
                else {
                    eeprom_addr=TEACH_DATA_POFF+((eeprom_addr_cal+(temp_min_val-16))*MAX_OFFSET);
                }
                if(eeprom_addr_cal<=(temp_max_val-temp_min_val+1)){
                    printf("EEPROM ADDR : %d",eeprom_addr);
                    ESP_LOGI(IR_DEBUG_TAG,"Writing Data...");
                    storing_IR_data_to_flash=true;
                    write_to_memory(results.rawbuf,results.rawlen-1,eeprom_addr);
                    if(eeprom_addr_cal==(temp_max_val-temp_min_val+1)){
                        configured = true;
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR, 0);
                        vTaskDelay(pdMS_TO_TICKS(5));
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR+1, protocol_selected_num);
                        vTaskDelay(pdMS_TO_TICKS(5));
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR, protocol_selected_num>>8);
                        vTaskDelay(pdMS_TO_TICKS(5));
                        teaching_mode = false;
                        teachMode_size_done=false;
                        storing_IR_data_to_flash=false;
                        ESP_LOGI(IR_DEBUG_TAG,"End of Teaching Mode");
                    }
                    else{
                        storing_IR_data_to_flash=false;
                        ESP_LOGI(IR_DEBUG_TAG,"Proceed for next");
                    }
                }  
                eeprom_addr_cal++;
            }

            /* AC Remote configuration process */
            //Here's where we need to add something like protocol_detected > something and < something
            //After modifying the decode_type_t enum in order to avoid unsupported remotes getting falsely recognized
            if (protocol_detected != UNKNOWN && protocol_detected != UNUSED && !configured && !teaching_mode)
            {
                protocol_selected_num = protocol_detected;
                eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR+1, protocol_selected_num);
                vTaskDelay(pdMS_TO_TICKS(5));
                eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR, protocol_selected_num>>8);
                vTaskDelay(pdMS_TO_TICKS(5));
#if (IS_GWY)
                if (registered)
                {
                    configured = true;
                    eeprom_write_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR, 0);
                    vTaskDelay(pdMS_TO_TICKS(5));
                    char pubmessage[PUBMESG_LEN];
                    sprintf(pubmessage, "{%s : %d, %s : %s, %s : %s, %s : %d}",
                            JSON_PACKET_ID_KEY, GWY_CONF_ACK,
                            JSON_ACK_NAME_KEY, GWY_CONF_ACK_NAME,
                            GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
                            ERROR_CODE_KEY, json_ack_err_code);
                    white_printf(IR_DEBUG_TAG, "Sending Gwy AC Remote Configuration Ack");
                    add_to_pubmesg_queue(pubmessage, publish_topic);
                }
#endif
#if (!IS_GWY)
                if (provisioned)
                {
                    configured = true;
                    eeprom_write_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR, 0);
                    vTaskDelay(pdMS_TO_TICKS(5));
                    send_AC_configuration_ack_to_gwy();
                }
#endif
            }
            /**
             * @brief Sending AC manual control ack or bringing back AC to within set Temperature limits as per Gwy AC Control packet should
             * occur only if the following conditions are met
             * 1) Device must be registered / configured
             * 2) The Identified protocol should match the protocol with which AC remote configuration process was done
             * 3) Device must not be in teaching mode
             * 4) Locking feature must be enabled in Gwy AC Control Packet 
             */
            if ((registered || configured) && (protocol_detected == protocol_selected_num || protocol_selected_num == RAW) && gwy_ac_control_t.Locking && !teaching_mode)
                locking_feature(result_description_char_str);
        }
    }
    vTaskDelete(NULL);
}
