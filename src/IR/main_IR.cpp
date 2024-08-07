/**
 * @file main_IR_recv.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions related to the IR receiver part
 * @version 0.8.7
 * @date 2024-07-19
 * @copyright Copyright (c) 2024
 */
#include "../../inc/Custom/main.h"
#include "../../inc/IR/main_IR.h"
#include "../../inc/LTE/LTE.h"
#include "../../inc/LTE/mqtt.h"
#include "../../inc/Custom/button.h"
#include "../../inc/Custom/printf_custom.h"

// Initialization - Receiver
IRrecv irrecv(IR_RECEIVER_PIN, RECV_BUFFER_SIZE, kTimeout, true);
decode_results results;
decode_type_t protocol_detected = UNKNOWN;
bool no_recv_function_flag = false;
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
uint16_t teaching_mode_rawlen = 0;

// Initialization - Transmitter
bool needToSendIRComamnd = false;
bool glow_purple = false;
bool sending = false;
int16_t protocol_selected_num = UNKNOWN;
uint8_t taskapprovalcount = 0;

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

char *get_protocol_string(uint16_t protocol)
{
    switch (protocol)
    {
    case RAW:
        return "RAW";
    case DAIKIN:
        return "DAIKIN280";
    case DAIKIN200:
        return "DAIKIN200";
    case DAIKIN216:
        return "DAIKIN216";
    case DAIKIN2:
        return "DAIKIN2";
    case DAIKIN160:
        return "DAIKIN160";
    case DAIKIN176:
        return "DAIKIN176";
    case DAIKIN64:
        return "DAIKIN64";
    case DAIKIN152:
        return "DAIKIN152";
    case DAIKIN128:
        return "DAIKIN128";
    case HITACHI_AC296:
        return "HITACHI_AC296";
    case HITACHI_AC:
        return "HITACHI_AC";
    case HITACHI_AC1:
        return "HITACHI_AC1";
    case HITACHI_AC424:
        return "HITACHI_AC424";
    case HITACHI_AC344:
        return "HITACHI_AC344";
    case HITACHI_AC264:
        return "HITACHI_AC264";
    case VOLTAS:
        return "VOLTAS";
    case SAMSUNG_AC:
        return "SAMSUNG_AC";
    case HAIER_AC:
        return "HAIER_AC";
    case HAIER_AC176:
        return "HAIER_AC176";
    case HAIER_AC160:
        return "HAIER_AC160";
    case CARRIER_AC64:
        return "CARRIER_AC64";
    case LG2:
        return "LG2";
    case LG:
        return "LG";
    case TOSHIBA_AC:
        return "TOSHIBA_AC";
    case MITSUBISHI112:
        return "MITSUBISHI112";
    case MITSUBISHI136:
        return "MITSUBISHI136";
    case MITSUBISHI_AC:
        return "MITSUBISHI_AC";
    case MITSUBISHI_HEAVY_88:
        return "MITSUBISHI_HEAVY_88";
    case MITSUBISHI_HEAVY_152:
        return "MITSUBISHI_HEAVY_152";
    case UNKNOWN:
        return "UNKNOWN";
    case UNUSED:
        return "UNUSED";
    default:
        return "INVALID";
    }
    return "";
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
        if (gwy_ac_control_t.control.power)
        {
            if (gwy_ac_control_t.control.temp > FETCH_ADDR_LOW)
                eeprom_addr = (TEACH_DATA_POFF + (MAX_OFFSET * (gwy_ac_control_t.control.temp - FETCH_ADDR_LOW)));
            else
                ESP_LOGE(IR_ERROR_TAG, "Not an valid temperature\r\n");
        }
        sprintf(ir_log_buffer, "EEPROM ADDR : %d\n", eeprom_addr);
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        if ((convert_data <= MAX_OFFSET) && (eeprom_addr <= TEACH_DATA_PON_32C) && ((gwy_ac_control_t.control.temp >= TEACHING_MODE_STARTING_TEMPERATURE) && ((gwy_ac_control_t.control.temp <= TEACHING_MODE_ENDING_TEMPERATURE) || !gwy_ac_control_t.control.power)))
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
        ac_daikin280.setPower(gwy_ac_control_t.control.power);
        ac_daikin280.setTemp(gwy_ac_control_t.control.temp);
        if (gwy_ac_control_t.control.swingH)
            gwy_ac_control_t.control.swingH = kDaikinSwingOn;
        ac_daikin280.setSwingHorizontal(gwy_ac_control_t.control.swingH);
        if (gwy_ac_control_t.control.swingV)
            gwy_ac_control_t.control.swingV = kDaikinSwingOn;
        ac_daikin280.setSwingVertical(gwy_ac_control_t.control.swingV);
        ac_daikin280.setFan(ac_daikin280.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_daikin280.enableOffTimer(gwy_ac_control_t.control.OffTimer);
        ac_daikin280.enableOnTimer(gwy_ac_control_t.control.OnTimer);
        ac_daikin280.setMode(ac_daikin280.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_daikin280.send();
        white_printf(IR_DEBUG_TAG, "Sending Daikin280");
        break;

    case DAIKIN200:
        strcpy(protocol_chosen_str, "Daikin200");
        ac_daikin200.setPower(gwy_ac_control_t.control.power);
        ac_daikin200.setSwingVertical(gwy_ac_control_t.control.swingV);
        ac_daikin200.setFan(gwy_ac_control_t.control.fanSpeed);
        ac_daikin200.setTemp(gwy_ac_control_t.control.temp);
        ac_daikin200.setMode(ac_daikin200.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_daikin200.send();
        ESP_LOGI(IR_DEBUG_TAG, "Sending Daikin200\r\n");
        break;

    case DAIKIN216:
        // If the mode is not "Cool", then we must only set mode. We must not try to set any other thing.
        if (gwy_ac_control_t.control.mode_val != COOL)
        {
            ac_daikin216.setMode(ac_daikin280.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
            ac_daikin216.send();
            return;
        }
        strcpy(protocol_chosen_str, "Daikin216");
        ac_daikin216.setPower(gwy_ac_control_t.control.power);
        ac_daikin216.setTemp(gwy_ac_control_t.control.temp);
        if (gwy_ac_control_t.control.swingH)
            gwy_ac_control_t.control.swingH = kDaikinSwingOn;
        ac_daikin216.setSwingHorizontal(gwy_ac_control_t.control.swingH);
        if (gwy_ac_control_t.control.swingV)
            gwy_ac_control_t.control.swingV = kDaikinSwingOn;
        ac_daikin216.setSwingVertical(gwy_ac_control_t.control.swingV);
        ac_daikin216.setFan(ac_daikin216.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_daikin216.setMode(ac_daikin216.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_daikin216.send();
        sprintf(ir_log_buffer, "Sending Daikin216");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case DAIKIN2:
        strcpy(protocol_chosen_str, "Daikin2");
        ac_daikin2.setPower(gwy_ac_control_t.control.power);
        ac_daikin2.setTemp(gwy_ac_control_t.control.temp);
        if (gwy_ac_control_t.control.swingH)
            ac_daikin2.setSwingHorizontal(kDaikin2SwingHAuto);
        else
            ac_daikin2.setSwingHorizontal(kDaikin2SwingHOff);
        if (gwy_ac_control_t.control.swingV)
            ac_daikin2.setSwingVertical(kDaikin2SwingVAuto);
        else
            ac_daikin2.setSwingVertical(kDaikin2SwingVOff);
        ac_daikin2.setFan(ac_daikin2.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_daikin2.enableOffTimer(gwy_ac_control_t.control.OffTimer);
        ac_daikin2.enableOnTimer(gwy_ac_control_t.control.OnTimer);
        ac_daikin2.setMode(ac_daikin2.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_daikin2.send();
        sprintf(ir_log_buffer, "Sending Daikin2");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case DAIKIN160:
        strcpy(protocol_chosen_str, "Daikin160");
        ac_daikin160.setPower(gwy_ac_control_t.control.power);
        ac_daikin160.setTemp(gwy_ac_control_t.control.temp);
        if (gwy_ac_control_t.control.swingV)
            ac_daikin160.setSwingVertical(kDaikin160SwingVAuto);
        ac_daikin160.setFan(ac_daikin160.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_daikin160.setMode(ac_daikin160.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_daikin160.send();
        sprintf(ir_log_buffer, "Sending Daikin160");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case DAIKIN176:
        strcpy(protocol_chosen_str, "Daikin176");
        ac_daikin176.setPower(gwy_ac_control_t.control.power);
        ac_daikin176.setTemp(gwy_ac_control_t.control.temp);
        if (gwy_ac_control_t.control.swingH)
            ac_daikin176.setSwingHorizontal(kDaikin176SwingHAuto);
        else
            ac_daikin176.setSwingHorizontal(kDaikin176SwingHOff);
        ac_daikin176.setFan(ac_daikin176.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_daikin176.setMode(ac_daikin176.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_daikin176.send();
        sprintf(ir_log_buffer, "Sending Daikin176");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case DAIKIN64:
        strcpy(protocol_chosen_str, "Daikin64");
        ac_daikinac64.setPowerToggle(gwy_ac_control_t.control.power);
        ac_daikinac64.setTemp(gwy_ac_control_t.control.temp);
        ac_daikinac64.setSwingVertical(gwy_ac_control_t.control.swingV);
        ac_daikinac64.setFan(ac_daikinac64.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_daikinac64.setOnTime(gwy_ac_control_t.control.OffTimer);
        ac_daikinac64.setOffTime(gwy_ac_control_t.control.OnTimer);
        ac_daikinac64.setMode(ac_daikinac64.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_daikinac64.send();
        sprintf(ir_log_buffer, "Sending Daikin64");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case DAIKIN152:
        strcpy(protocol_chosen_str, "Daikin152");
        ac_daikin152.setPower(gwy_ac_control_t.control.power);
        ac_daikin152.setTemp(gwy_ac_control_t.control.temp);
        ac_daikin152.setSwingV(gwy_ac_control_t.control.swingV);
        ac_daikin152.setFan(ac_daikin152.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_daikin152.setMode(ac_daikin152.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_daikin152.send();
        sprintf(ir_log_buffer, "Sending Daikin152");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case DAIKIN128:
        strcpy(protocol_chosen_str, "Daikin128");
        ac_daikin128.setPowerToggle(gwy_ac_control_t.control.power);
        ac_daikin128.setTemp(gwy_ac_control_t.control.temp);
        ac_daikin128.setSwingVertical(gwy_ac_control_t.control.swingV);
        ac_daikin128.setFan(ac_daikin128.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_daikin128.setOffTimer(gwy_ac_control_t.control.OffTimer);
        ac_daikin128.setOnTimer(gwy_ac_control_t.control.OnTimer);
        ac_daikin128.setMode(ac_daikin128.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_daikin128.send();
        sprintf(ir_log_buffer, "Sending Daikin128");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HITACHI_AC296:
        strcpy(protocol_chosen_str, "Hitachi296");
        ac_hitachi296.setPower(gwy_ac_control_t.control.power);
        ac_hitachi296.setTemp(gwy_ac_control_t.control.temp);
        ac_hitachi296.setFan(ac_hitachi296.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_hitachi296.setMode(ac_hitachi296.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_hitachi296.send();
        sprintf(ir_log_buffer, "Sending Hitachi296");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HITACHI_AC:
        strcpy(protocol_chosen_str, "HitachiAc224");
        ac_hitachi224.setPower(gwy_ac_control_t.control.power);
        ac_hitachi224.setTemp(gwy_ac_control_t.control.temp);
        ac_hitachi224.setFan(ac_hitachi224.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_hitachi224.setSwingHorizontal(gwy_ac_control_t.control.swingH);
        ac_hitachi224.setSwingVertical(gwy_ac_control_t.control.swingV);
        ac_hitachi224.setMode(ac_hitachi224.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_hitachi224.send();
        sprintf(ir_log_buffer, "Sending HitachiAc224");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HITACHI_AC1:
        strcpy(protocol_chosen_str, "HitachiAc104");
        ac_hitachi104.setPower(gwy_ac_control_t.control.power);
        ac_hitachi104.setTemp(gwy_ac_control_t.control.temp);
        ac_hitachi104.setFan(ac_hitachi104.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_hitachi104.setSwingH(gwy_ac_control_t.control.swingH);
        ac_hitachi104.setSwingV(gwy_ac_control_t.control.swingV);
        ac_hitachi104.setOffTimer(gwy_ac_control_t.control.OffTimer);
        ac_hitachi104.setOnTimer(gwy_ac_control_t.control.OnTimer);
        ac_hitachi104.setMode(ac_hitachi104.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_hitachi104.send();
        sprintf(ir_log_buffer, "Sending HitachiAc104");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HITACHI_AC424:
        strcpy(protocol_chosen_str, "HitachiAc424");
        ac_hitachi424.setPower(gwy_ac_control_t.control.power);
        ac_hitachi424.setTemp(gwy_ac_control_t.control.temp);
        ac_hitachi424.setFan(ac_hitachi424.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_hitachi424.setSwingVToggle(gwy_ac_control_t.control.swingV);
        ac_hitachi424.setMode(ac_hitachi424.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_hitachi424.send();
        sprintf(ir_log_buffer, "Sending HitachiAc424");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HITACHI_AC344:
        strcpy(protocol_chosen_str, "HitachiAc344");
        ac_hitachi344.setSwingH(gwy_ac_control_t.control.swingH);
        ac_hitachi344.setSwingV(gwy_ac_control_t.control.swingV);
        ac_hitachi344.send();
        sprintf(ir_log_buffer, "Sending HitachiAc344");
        break;

    case HITACHI_AC264:
        strcpy(protocol_chosen_str, "HitachiAc264");
        ac_hitachi264.setFan(ac_hitachi264.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_hitachi264.send();
        sprintf(ir_log_buffer, "Sending HitachiAc264");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case VOLTAS:
        strcpy(protocol_chosen_str, "Voltas");
        ac_voltas.setPower(gwy_ac_control_t.control.power);
        ac_voltas.setTemp(gwy_ac_control_t.control.temp);
        ac_voltas.setSwingH(gwy_ac_control_t.control.swingH);
        ac_voltas.setSwingV(gwy_ac_control_t.control.swingV);
        ac_voltas.setFan(ac_voltas.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_voltas.setOffTime(gwy_ac_control_t.control.OffTimer);
        ac_voltas.setOnTime(gwy_ac_control_t.control.OnTimer);
        ac_voltas.setMode(ac_voltas.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_voltas.send();
        sprintf(ir_log_buffer, "Sending Voltas\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case SAMSUNG_AC:
        strcpy(protocol_chosen_str, "Samsung");
        ac_samsung.setPower(gwy_ac_control_t.control.power);
        ac_samsung.setTemp(gwy_ac_control_t.control.temp);
        ac_samsung.setSwingH(gwy_ac_control_t.control.swingH);
        ac_samsung.setSwing(gwy_ac_control_t.control.swingV);
        ac_samsung.setFan(ac_samsung.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_samsung.setOffTimer(gwy_ac_control_t.control.OffTimer);
        ac_samsung.setOnTimer(gwy_ac_control_t.control.OnTimer);
        ac_samsung.setMode(ac_samsung.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_samsung.send();
        sprintf(ir_log_buffer, "Sending Samsung\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HAIER_AC:
        strcpy(protocol_chosen_str, "Haier");
        // Regarding Power on/off and horizontal swing control we don't have library support
        ac_haier.setTemp(gwy_ac_control_t.control.temp);
        ac_haier.setSwingV(gwy_ac_control_t.control.swingV);
        ac_haier.setFan(ac_haier.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_haier.setOffTimer(gwy_ac_control_t.control.OffTimer);
        ac_haier.setOnTimer(gwy_ac_control_t.control.OnTimer);
        ac_haier.setMode(ac_haier.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_haier.send();
        sprintf(ir_log_buffer, "Sending Haier\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HAIER_AC176:
        strcpy(protocol_chosen_str, "Haier176");
        ac_haier176.setPower(gwy_ac_control_t.control.power);
        ac_haier176.setTemp(gwy_ac_control_t.control.temp);
        ac_haier176.setSwingH(gwy_ac_control_t.control.swingH);
        ac_haier176.setSwingV(gwy_ac_control_t.control.swingV);
        ac_haier176.setFan(ac_haier176.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_haier176.setOffTimer(gwy_ac_control_t.control.OffTimer);
        ac_haier176.setOnTimer(gwy_ac_control_t.control.OnTimer);
        ac_haier176.setMode(ac_haier176.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_haier176.send();
        sprintf(ir_log_buffer, "Sending Haier176\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case HAIER_AC160:
        // Regarding horizontal swing we don't have library support
        strcpy(protocol_chosen_str, "Haier160");
        ac_haier160.setPower(gwy_ac_control_t.control.power);
        ac_haier160.setTemp(gwy_ac_control_t.control.temp);
        ac_haier160.setSwingV(gwy_ac_control_t.control.swingV);
        ac_haier160.setFan(ac_haier160.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_haier160.setOffTimer(gwy_ac_control_t.control.OffTimer);
        ac_haier160.setOnTimer(gwy_ac_control_t.control.OnTimer);
        ac_haier160.setMode(ac_haier160.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_haier160.send();
        sprintf(ir_log_buffer, "Sending Haier160\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case CARRIER_AC64:
        // Regarding horizontal swing we don't have library support
        strcpy(protocol_chosen_str, "CarrierAC64");
        ac_carrier64.setPower(gwy_ac_control_t.control.power);
        ac_carrier64.setTemp(gwy_ac_control_t.control.temp);
        ac_carrier64.setSwingV(gwy_ac_control_t.control.swingV);
        ac_carrier64.setFan(ac_carrier64.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_carrier64.setOffTimer(gwy_ac_control_t.control.OffTimer);
        ac_carrier64.setOnTimer(gwy_ac_control_t.control.OnTimer);
        ac_carrier64.setMode(ac_carrier64.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_carrier64.send();
        sprintf(ir_log_buffer, "Sending CarrierAC64\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case LG2:
    case LG:
        // Regarding on/off timer we don't have library support
        strcpy(protocol_chosen_str, "LG");
        ac_lg.setPower(gwy_ac_control_t.control.power);
        ac_lg.setTemp(gwy_ac_control_t.control.temp);
        ac_lg.setSwingH(gwy_ac_control_t.control.swingH);
        ac_lg.setSwingV(gwy_ac_control_t.control.swingV);
        ac_lg.setFan(ac_lg.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_lg.setMode(ac_lg.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_lg.send();
        sprintf(ir_log_buffer, "Sending LG\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case TOSHIBA_AC:
        // Regarding on/off timer we don't have library support
        strcpy(protocol_chosen_str, "Toshiba");
        ac_toshiba.setPower(gwy_ac_control_t.control.power);
        ac_toshiba.setTemp(gwy_ac_control_t.control.temp);
        ac_toshiba.setFan(ac_toshiba.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        if (gwy_ac_control_t.control.swingV || gwy_ac_control_t.control.swingH)
        {
            /*Library seems like they only do swing on/off so that only i am having an
            condition check for both horizontal and vertical*/
            ac_toshiba.setSwing(kToshibaAcSwingOn);
        }
        else
        {
            ac_toshiba.setSwing(kToshibaAcSwingOff);
        }
        ac_toshiba.setMode(ac_toshiba.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_toshiba.send();
        sprintf(ir_log_buffer, "Sending Toshiba\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case MITSUBISHI112:
        // Regarding on/off timer we don't have library support
        strcpy(protocol_chosen_str, "Mitsubishi112");
        ac_mitsubishi112.setPower(gwy_ac_control_t.control.power);
        ac_mitsubishi112.setTemp(gwy_ac_control_t.control.temp);
        ac_mitsubishi112.setSwingH(gwy_ac_control_t.control.swingH);
        ac_mitsubishi112.setSwingV(gwy_ac_control_t.control.swingV);
        ac_mitsubishi112.setFan(ac_mitsubishi112.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_mitsubishi112.setMode(ac_mitsubishi112.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_mitsubishi112.send();
        sprintf(ir_log_buffer, "Sending Mitsubishi112\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case MITSUBISHI136:
        // Regarding on/off timer and horizontal swing we don't have library support
        strcpy(protocol_chosen_str, "Mitsubishi136");
        ac_mitsubishi136.setPower(gwy_ac_control_t.control.power);
        ac_mitsubishi136.setTemp(gwy_ac_control_t.control.temp);
        if (gwy_ac_control_t.control.swingV)
        {
            ac_mitsubishi136.setSwingV(kMitsubishi136SwingVAuto);
        }
        ac_mitsubishi136.setFan(ac_mitsubishi136.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_mitsubishi136.setMode(ac_mitsubishi136.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_mitsubishi136.send();
        sprintf(ir_log_buffer, "Sending Mitsubishi136\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case MITSUBISHI_AC:
        // Regarding on/off timer and  swing we don't have library support
        strcpy(protocol_chosen_str, "MitsubishiAc");
        ac_mitsubishi144.setPower(gwy_ac_control_t.control.power);
        ac_mitsubishi144.setTemp(gwy_ac_control_t.control.temp);
        ac_mitsubishi144.setFan(ac_mitsubishi144.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        ac_mitsubishi144.setMode(ac_mitsubishi144.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_mitsubishi144.send();
        sprintf(ir_log_buffer, "Sending MitsubishiAc\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case MITSUBISHI_HEAVY_88:
        // Regarding on/off timer we don't have library support
        strcpy(protocol_chosen_str, "MitsubisiHvy88");
        ac_mitsubishi88.setPower(gwy_ac_control_t.control.power);
        ac_mitsubishi88.setTemp(gwy_ac_control_t.control.temp);
        ac_mitsubishi88.setFan(ac_mitsubishi88.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        if (gwy_ac_control_t.control.swingH)
            ac_mitsubishi88.setSwingHorizontal(kMitsubishiHeavy88SwingHAuto);
        else
            ac_mitsubishi88.setSwingHorizontal(kMitsubishiHeavy88SwingHOff);
        if (gwy_ac_control_t.control.swingV)
            ac_mitsubishi88.setSwingVertical(kMitsubishiHeavy88SwingVAuto);
        else
            ac_mitsubishi88.setSwingVertical(kMitsubishiHeavy88SwingVOff);
        ac_mitsubishi88.setMode(ac_mitsubishi88.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_mitsubishi88.send();
        sprintf(ir_log_buffer, "Sending MitsubishiHeavy88\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    case MITSUBISHI_HEAVY_152:
        // Regarding on/off timer we don't have library support
        strcpy(protocol_chosen_str, "MitsbisiHvy152");
        ac_mitsubishi152.setPower(gwy_ac_control_t.control.power);
        ac_mitsubishi152.setTemp(gwy_ac_control_t.control.temp);
        ac_mitsubishi152.setFan(ac_mitsubishi152.convertFan((stdAc::fanspeed_t)gwy_ac_control_t.control.fanSpeed));
        if (gwy_ac_control_t.control.swingH)
            ac_mitsubishi152.setSwingHorizontal(kMitsubishiHeavy88SwingHAuto);
        else
            ac_mitsubishi152.setSwingHorizontal(kMitsubishiHeavy88SwingHOff);
        if (gwy_ac_control_t.control.swingV)
            ac_mitsubishi88.setSwingVertical(kMitsubishiHeavy88SwingVAuto);
        else
            ac_mitsubishi152.setSwingVertical(kMitsubishiHeavy88SwingVOff);
        ac_mitsubishi152.setMode(ac_mitsubishi152.convertMode((stdAc::opmode_t)gwy_ac_control_t.control.mode_val));
        ac_mitsubishi152.send();
        sprintf(ir_log_buffer, "Sending MitsubishiHeavy152\n");
        white_printf(IR_DEBUG_TAG, ir_log_buffer);
        break;

    default:
        sprintf(ir_log_buffer, "Error in choosing the protocol for send");
        custom_printf(IR_ERROR_TAG, ir_log_buffer, RED);
        break;
    }
}

bool is_supported_remote(uint16_t protocol)
{
    switch (protocol)
    {
    case DAIKIN:
    case DAIKIN200:
    case DAIKIN216:
    case DAIKIN2:
    case DAIKIN160:
    case DAIKIN176:
    case DAIKIN64:
    case DAIKIN152:
    case DAIKIN128:
    case HITACHI_AC296:
    case HITACHI_AC:
    case HITACHI_AC1:
    case HITACHI_AC424:
    case HITACHI_AC344:
    case HITACHI_AC264:
    case VOLTAS:
    case SAMSUNG_AC:
    case HAIER_AC:
    case HAIER_AC176:
    case HAIER_AC160:
    case CARRIER_AC64:
    case LG2:
    case LG:
    case TOSHIBA_AC:
    case MITSUBISHI112:
    case MITSUBISHI136:
    case MITSUBISHI_AC:
    case MITSUBISHI_HEAVY_88:
    case MITSUBISHI_HEAVY_152:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Function to decipher and fill the information about Manual AC control into manual_ac_control_t structures
 * We're only going to decipher Power, Mode, Fan, Temperature. Swing and Timer data are not supported.
 * @param result_description_char_str The string containing the manual containing information
 * @retval none
 */
void fetch_data_from_manual_control(char *input_string)
{
    char temperature[3] = "";
    char power[4] = "";
    char fan[2] = "";
    char mode[5] = "";

#if (IS_GWY)
    // Fetch Power
    if (strstr(input_string, "Power"))
    {
        snprintf(power, sizeof(power), (strstr(input_string, "Power") + 7));
        ESP_LOGI(IR_DEBUG_TAG, "Power detected : %s", power);
        if (strstr(power, "On,"))
            gwy_manual_ac_control_t.control.power = 1;
        else
            gwy_manual_ac_control_t.control.power = 0;
    }
    else red_printf(IR_ERROR_TAG, "Power missing in result_description_str");

    // Fetch Mode
    if (strstr(input_string, "Mode"))
    {
        snprintf(mode, sizeof(mode), (strstr(input_string, "Mode:") + 9));
        ESP_LOGI(IR_DEBUG_TAG, "Mode detected : %s", mode);
        if (strstr(mode, "Cool"))
            strcpy(gwy_manual_ac_control_t.control.mode_str, "Cool");
        else if (strstr(mode, "Heat"))
            strcpy(gwy_manual_ac_control_t.control.mode_str, "Hot");
        else if (strstr(mode, "Dry"))
            strcpy(gwy_manual_ac_control_t.control.mode_str, "Dry");
        else if (strstr(mode, "Auto"))
            strcpy(gwy_manual_ac_control_t.control.mode_str, "Auto");
        else if (strstr(mode, "Fan"))
            strcpy(gwy_manual_ac_control_t.control.mode_str, "Fan");
    }
    else red_printf(IR_ERROR_TAG, "Mode missing in result_description_str");

    // Fetch Fan
    if (strstr(input_string, "Fan"))
    {
        snprintf(fan, sizeof(fan), (strstr(input_string, "Fan") + 5));
        ESP_LOGI(IR_DEBUG_TAG, "Fan detected : %s", fan);
        gwy_manual_ac_control_t.control.fanSpeed = atoi(fan);
    }
    else red_printf(IR_ERROR_TAG, "Fan missing in result_description_str");

    // Fetch Temperature
    if (strstr(input_string, "Temp"))
    {
        snprintf(temperature, sizeof(temperature), (strstr(input_string, "Temp") + 6));
        ESP_LOGI(IR_DEBUG_TAG, "Temperature detected : %s", temperature);
        gwy_manual_ac_control_t.control.temp = atoi(temperature);
    }
    else red_printf(IR_ERROR_TAG, "Temp missing in result_description_str");
#endif

#if (!IS_GWY)
    // Fetch Power
    if (strstr(input_string, "Power"))
    {
        snprintf(power, sizeof(power), (strstr(input_string, "Power") + 7));
        ESP_LOGI(IR_DEBUG_TAG, "Power detected : %s", power);
        if (!strcmp(power, "On,"))
            node_manual_ac_control_t.control.power = 1;
        else
            node_manual_ac_control_t.control.power = 0;
    }
    else red_printf(IR_ERROR_TAG, "Power missing in result_description_str");

    // Fetch Mode
    if (strstr(input_string, "Mode"))
    {
        snprintf(mode, sizeof(mode), (strstr(input_string, "Mode:") + 9));
        ESP_LOGI(IR_DEBUG_TAG, "Mode detected : %s", mode);
        if (strstr(mode, "Cool"))
            strcpy(node_manual_ac_control_t.control.mode_str, "Cool");
        else if (strstr(mode, "Heat"))
            strcpy(node_manual_ac_control_t.control.mode_str, "Hot");
        else if (strstr(mode, "Dry"))
            strcpy(node_manual_ac_control_t.control.mode_str, "Dry");
        else if (strstr(mode, "Auto"))
            strcpy(node_manual_ac_control_t.control.mode_str, "Auto");
        else if (strstr(mode, "Fan"))
            strcpy(node_manual_ac_control_t.control.mode_str, "Fan");
    }
    else red_printf(IR_ERROR_TAG, "Mode missing in result_description_str");
    
    // Fetch Fan
    if (strstr(input_string, "Fan"))
    {
        snprintf(fan, sizeof(fan), (strstr(input_string, "Fan") + 5));
        ESP_LOGI(IR_DEBUG_TAG, "Fan detected : %s", fan);
        node_manual_ac_control_t.control.fanSpeed = atoi(fan);
    }
    else red_printf(IR_ERROR_TAG, "Fan missing in result_description_str");

    // Fetch Temperature
    if (strstr(input_string, "Temp"))
    {
        snprintf(temperature, sizeof(temperature), (strstr(input_string, "Temp") + 6));
        ESP_LOGI(IR_DEBUG_TAG, "Temperature detected : %s", temperature);
        node_manual_ac_control_t.control.temp = atoi(temperature);
    }
    else red_printf(IR_ERROR_TAG, "Temp missing in result_description_str");
#endif
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
    ESP_LOGI(IR_DEBUG_TAG, "Inside Locking Feature function");
    fetch_data_from_manual_control(result_description_char_str);
    if (protocol_selected_num == RAW)
    {
#if (IS_GWY)
        gwy_manual_ac_control_t.control.power = 0;
        gwy_manual_ac_control_t.control.temp = 0;
        gwy_manual_ac_control_t.control.fanSpeed = 0;
        strcpy(gwy_manual_ac_control_t.control.mode_str, "");
        goto here;
#endif

#if (!IS_GWY)
        node_manual_ac_control_t.control.power = 0;
        node_manual_ac_control_t.control.temp = 0;
        node_manual_ac_control_t.control.fanSpeed = 0;
        strcpy(node_manual_ac_control_t.control.mode_str, "");
        goto here;
#endif
    }
    else
    {
#if (IS_GWY)
        if (gwy_manual_ac_control_t.control.temp <= gwy_ac_control_t.control.TempLockUpLimit && gwy_manual_ac_control_t.control.temp >= gwy_ac_control_t.control.TempLockLowLimit);
#endif
#if (!IS_GWY)
        if (node_manual_ac_control_t.control.temp <= node_ac_control_t.control.TempLockUpLimit && node_manual_ac_control_t.control.temp >= node_ac_control_t.control.TempLockLowLimit);
#endif
        else
        {
            ESP_LOGI(IR_DEBUG_TAG, "Reverting AC back to permissible limits ... ");
            needToSendIRComamnd = true;
        }
    }

here:
#if (IS_GWY)
    white_printf(IR_DEBUG_TAG, "Sending Gwy Manual AC control ack");
    char pubmessage[PUBMESG_LEN];
    sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : \"%s\", \"%s\" : %d, \"%s\" : %d}",
            JSON_PACKET_ID_KEY, GWY_MANUAL_AC_CONTROL_ACK,
            JSON_ACK_NAME_KEY, GWY_MANUAL_AC_CONTROL_ACK_NAME,
            GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
            POWER_KEY, gwy_manual_ac_control_t.control.power,
            MODE_KEY, gwy_manual_ac_control_t.control.mode_str,
            FAN_SPEED_KEY, gwy_manual_ac_control_t.control.fanSpeed,
            TEMPERATURE_KEY, gwy_manual_ac_control_t.control.temp);
    add_to_pubmesg_queue(pubmessage, publish_topic);
#endif
#if (!IS_GWY)
    send_manual_ac_control_ack_to_gwy();
#endif
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
        if (esp_restart_flag)
            ESP.restart();
        if (needToSendIRComamnd)
        {
            irrecv.pause();
            custom_printf(IR_DEBUG_TAG, "Firing IR signal", WHITE);
            IR_transmit(protocol_selected_num);
            // sleep(1); // Let's wait a second before resume to avoid scattering IR signals getting false detected as Manual AC control.
            irrecv.resume();
            needToSendIRComamnd = false;
        }
        if (irrecv.decode(&results))
        {
            resultToHumanReadableBasic(&results, &protocol_detected).c_str();
            String description = IRAcUtils::resultAcToString(&results);
            char result_description_char_str[200];
            strcpy(result_description_char_str, (char *)description.c_str());

            // Print out received IR signal
            ESP_LOGI(IR_DEBUG_TAG, "IR RAW VALUES : { ");
            for (uint16_t i = 0; i < results.rawlen; i++)
            {
                printf("%d, ", results.rawbuf[i]);
            }
            ESP_LOGI(IR_DEBUG_TAG, "}\n");

            if (description.length())
                ESP_LOGI(IR_DEBUG_TAG, "%s", result_description_char_str);

            /**
             * @brief Sending AC manual control ack or bringing back AC to within set Temperature limits as per Gwy AC Control packet should
             * occur only if the following conditions are met
             * 1) Device must be registered / provisioned
             * 2) The Identified protocol should match the protocol with which AC remote configuration process was done
             * 3) Device must not be in teaching mode
             * 4) Locking feature must be enabled in Gwy AC Control Packet
             * 5) Device must be configured
             */
            // Also need to have this before the configuration part of code, or else, just after configuring, device will send manual ac control ack
            if ((registered || provisioned) &&
                (configured) &&
                (protocol_detected == protocol_selected_num || (protocol_selected_num == RAW && teaching_mode_rawlen == results.rawlen)) &&
                (gwy_ac_control_t.control.Locking || node_ac_control_t.control.Locking) &&
                !teaching_mode)
                locking_feature(result_description_char_str);
            else
            {
                ESP_LOGI(IR_DEBUG_TAG, "registered | provisioned || configured : %d | %d || %d", registered, provisioned, configured);
                ESP_LOGI(IR_DEBUG_TAG, "teaching_mode_rawlen : %d | results.rawlen : %d", teaching_mode_rawlen, results.rawlen);
                ESP_LOGI(IR_DEBUG_TAG, "gwy_ac_control_t.Locking | node_ac_control_t.Locking: %d | %d", gwy_ac_control_t.control.Locking, node_ac_control_t.control.Locking);
                ESP_LOGI(IR_DEBUG_TAG, "teaching_mode : %d", teaching_mode);
                ESP_LOGI(IR_DEBUG_TAG, "protocol_selected_num == RAW && teaching_mode_rawlen == results.rawlen : %d", (protocol_selected_num == RAW) && (teaching_mode_rawlen == results.rawlen));
                ESP_LOGI(IR_DEBUG_TAG, "protocol_selected_num : %d", protocol_selected_num);
            }

            if (teaching_mode)
            {
                if (teaching_mode_rawlen == 0)
                {
                    teaching_mode_rawlen = results.rawlen; // Let's save the number of bytes received during teaching mode command, this will help us at manual controla ack
                    eeprom_write_byte(EEPROM_SLAVE_ADDR, RAWLEN_ADDR_HI, teaching_mode_rawlen >> 8);
                    eeprom_write_byte(EEPROM_SLAVE_ADDR, RAWLEN_ADDR_LO, teaching_mode_rawlen);
                }
                if (teachMode_size_done)
                {
                    eeprom_addr_cal = 0;
                    eeprom_write_byte(EEPROM_SLAVE_ADDR, EEPROM_CONF_FAC, TEACHING_FAC);
                    vTaskDelay(20 / portTICK_PERIOD_MS);
                    if (results.rawlen > 1)
                        convert_data = ((results.rawlen) - 1) * 2;
                    convert_16bit_to_8bit(convert_data, convert_buffer);
                    eeprom_addr = TEACH_DATA_LEN;
                    eeprom_write(EEPROM_SLAVE_ADDR, eeprom_addr, convert_buffer, 2);
                    vTaskDelay(20 / portTICK_PERIOD_MS);
                    teachMode_size_done = false;
                }
                if (!eeprom_addr_cal)
                {
                    eeprom_addr = TEACH_DATA_POFF;
                }
                else
                {
                    eeprom_addr = TEACH_DATA_POFF + ((eeprom_addr_cal + (temp_min_val - 16)) * MAX_OFFSET);
                }
                if (eeprom_addr_cal <= (temp_max_val - temp_min_val + 1))
                {
                    printf("EEPROM ADDR : %d", eeprom_addr);
                    ESP_LOGI(IR_DEBUG_TAG, "Writing Data...");
                    storing_IR_data_to_flash = true;
                    write_to_memory(results.rawbuf, results.rawlen - 1, eeprom_addr);
                    if (eeprom_addr_cal == (temp_max_val - temp_min_val + 1))
                    {
                        ESP_LOGI(IR_DEBUG_TAG, "End of Teaching Mode");
                        configured = true;
                        teachMode_size_done = false;
                        storing_IR_data_to_flash = false;
                        protocol_selected_num = RAW;
                        teaching_mode = false;
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR, true);
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR_LO, protocol_selected_num);
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR_HI, protocol_selected_num >> 8);

                        //reset the AC control structure settings
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, MODE_FLASH_ADDR, 0);
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, FAN_FLASH_ADDR,0);
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, SWINGH_FLASH_ADDR,0);
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, SWINGV_FLASH_ADDR,0);
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, ONTIMER_FLASH_ADDR,0);
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, OFFTIMER_FLASH_ADDR,0);
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, LOCKING_FLASH_ADDR,0);
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, TEMPLOCKLOWLIMIT_FLASH_ADDR,0);
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, TEMPLOCKUPLIMIT_FLASH_ADDR,0);
#if (IS_GWY)
                        strcpy(gwy_ac_control_t.control.mode_str,"");
                        gwy_ac_control_t.control.fanSpeed = 0;
                        gwy_ac_control_t.control.swingH = 0;
                        gwy_ac_control_t.control.swingV = 0;
                        gwy_ac_control_t.control.OnTimer = 0;
                        gwy_ac_control_t.control.OffTimer = 0;
                        gwy_ac_control_t.control.Locking = 0;
                        gwy_ac_control_t.control.TempLockLowLimit = 0;
                        gwy_ac_control_t.control.TempLockUpLimit = 0;

                        char pubmessage[PUBMESG_LEN];
                        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : \"%s\"}",
                                JSON_PACKET_ID_KEY, GWY_TEACHING_MODE_END_ACK,
                                JSON_ACK_NAME_KEY, GWY_TEACHING_MODE_END_ACK_NAME,
                                GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING);
                        add_to_pubmesg_queue(pubmessage, publish_topic);
#endif
#if (!IS_GWY)
                        strcpy(node_ac_control_t.control.mode_str,"");
                        node_ac_control_t.control.fanSpeed = 0;
                        node_ac_control_t.control.swingH = 0;
                        node_ac_control_t.control.swingV = 0;
                        node_ac_control_t.control.OnTimer = 0;
                        node_ac_control_t.control.OffTimer = 0;
                        node_ac_control_t.control.Locking = 0;
                        node_ac_control_t.control.TempLockLowLimit = 0;
                        node_ac_control_t.control.TempLockUpLimit = 0;

                        send_teaching_mode_end_ack_to_gwy();
#endif
                    }
                    else
                    {
                        storing_IR_data_to_flash = false;
                        ESP_LOGI(IR_DEBUG_TAG, "Proceed for next");
                    }
                }
                eeprom_addr_cal++;
            }

            /* AC Remote configuration process */
            // Here's where we need to add something like protocol_detected > something and < something
            // After modifying the decode_type_t enum in order to avoid unsupported remotes getting falsely recognized
            if (!configured && !teaching_mode)
            {
#if (IS_GWY)
                if (registered)
                {
                    if (is_supported_remote(protocol_detected))
                    {
                        protocol_selected_num = protocol_detected;
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR_LO, protocol_selected_num);
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR_HI, protocol_selected_num >> 8);
                        configured = true;
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR, true);
                        //Just turning these off, because of the case, where the device could have gotten from teaching mode to configurtion mode and then gets configured.
                        teaching_mode = false;
                        teachMode_size_done = false;
                    }
                    else unsupported_remote_flag = true;
                    
                    char pubmessage[PUBMESG_LEN];
                    if (!unsupported_remote_flag)
                    {
                        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d}",
                            JSON_PACKET_ID_KEY, GWY_CONF_ACK,
                            JSON_ACK_NAME_KEY, GWY_CONF_ACK_NAME,
                            GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
                            ERROR_CODE_KEY, 0);
                    }
                    else
                    {
                        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d}",
                            JSON_PACKET_ID_KEY, GWY_CONF_ACK,
                            JSON_ACK_NAME_KEY, GWY_CONF_ACK_NAME,
                            GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
                            ERROR_CODE_KEY, AC_REMOTE_UNSUPPORTED);
                    }
                    custom_printf(IR_DEBUG_TAG, "Sending Gwy AC Remote Configuration ACK", WHITE);
                    add_to_pubmesg_queue(pubmessage, publish_topic);
                }
#endif
#if (!IS_GWY)
                if (provisioned)
                {
                    if (is_supported_remote(protocol_detected))
                    {
                        protocol_selected_num = protocol_detected;
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR_LO, protocol_selected_num);
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR_HI, protocol_selected_num >> 8);
                        configured = true;
                        eeprom_write_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR, true);
                        //Just turning these off, because of the case, where the device could have gotten from teaching mode to configurtion mode and then gets configured.
                        teaching_mode = false;
                        teachMode_size_done = false;
                    }
                    else unsupported_remote_flag = true;

                    send_AC_configuration_ack_to_gwy();
                }
#endif
                continue;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    vTaskDelete(NULL);
}
