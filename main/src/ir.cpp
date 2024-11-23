#include "esp_log.h"
#include <stdbool.h>

#include "IRac.h"
#include <IRutils.h>
#include <IRremoteESP8266.h>

#include <main.h>
#include <ir.h>
#include <lte.h>
#include <flash.h>
#include <ble_new.h>
#include <led.h>
#include <json_maker.h>
#include <cJSON.h>

#define KHZ_41 41

const char *RAW_IR_PROTOCOL = "RAW";
const char *DAIKIN_IR_PROTOCOL = "DAIKIN280";
const char *DAIKIN200_IR_PROTOCOL = "DAIKIN200";
const char *DAIKIN216_IR_PROTOCOL = "DAIKIN216";
const char *DAIKIN2_IR_PROTOCOL = "DAIKIN2";
const char *DAIKIN160_IR_PROTOCOL = "DAIKIN160";
const char *DAIKIN176_IR_PROTOCOL = "DAIKIN176";
const char *DAIKIN64_IR_PROTOCOL = "DAIKIN64";
const char *DAIKIN152_IR_PROTOCOL = "DAIKIN152";
const char *DAIKIN128_IR_PROTOCOL = "DAIKIN128";
const char *HITACHI_AC296_IR_PROTOCOL = "HITACHI_AC296";
const char *HITACHI_AC_IR_PROTOCOL = "HITACHI_AC";
const char *HITACHI_AC1_IR_PROTOCOL = "HITACHI_AC1";
const char *HITACHI_AC424_IR_PROTOCOL = "HITACHI_AC424";
const char *HITACHI_AC344_IR_PROTOCOL = "HITACHI_AC344";
const char *HITACHI_AC264_IR_PROTOCOL = "HITACHI_AC264";
const char *VOLTAS_IR_PROTOCOL = "VOLTAS";
const char *SAMSUNG_AC_IR_PROTOCOL = "SAMSUNG_AC";
const char *HAIER_AC_IR_PROTOCOL = "HAIER_AC";
const char *HAIER_AC176_IR_PROTOCOL = "HAIER_AC176";
const char *HAIER_AC160_IR_PROTOCOL = "HAIER_AC160";
const char *CARRIER_AC64_IR_PROTOCOL = "CARRIER_AC64";
const char *LG2_IR_PROTOCOL = "LG2";
const char *LG_IR_PROTOCOL = "LG";
const char *TOSHIBA_AC_IR_PROTOCOL = "TOSHIBA_AC";
const char *MITSUBISHI112_IR_PROTOCOL = "MITSUBISHI112";
const char *MITSUBISHI136_IR_PROTOCOL = "MITSUBISHI136";
const char *MITSUBISHI_AC_IR_PROTOCOL = "MITSUBISHI_AC";
const char *MITSUBISHI_HEAVY_88_IR_PROTOCOL = "MITSUBISHI_HEAVY_88";
const char *MITSUBISHI_HEAVY_152_IR_PROTOCOL = "MITSUBISHI_HEAVY_152";
const char *UNKNOWN_IR_PROTOCOL = "UNKNOWN";
const char *UNUSED_IR_PROTOCOL = "UNUSED";
const char *INVALID_IR_PROTOCOL = "INVALID";

const char *TEACHING_MODE_SEQUENCE[] = {
    "Power - Off",                   // 0
    "Temperature - 18 | Power - On", // 1
    "Temperature - 19 | Power - On", // 2
    "Temperature - 20 | Power - On", // 3
    "Temperature - 21 | Power - On", // 4
    "Temperature - 22 | Power - On", // 5
    "Temperature - 23 | Power - On", // 6
    "Temperature - 24 | Power - On", // 7
    "Temperature - 25 | Power - On", // 8
    "Temperature - 26 | Power - On", // 9
    "Temperature - 27 | Power - On", // 10
    "Temperature - 28 | Power - On", // 11
    "Temperature - 29 | Power - On", // 12
    "Temperature - 30 | Power - On", // 13
    "Temperature - 31 | Power - On", // 14
    "Temperature - 32 | Power - On"  // 15
};

/*IR Receiver Initializations*/
IRrecv irrecv(IR_RECV_GPIO, RECV_BUFFER_SIZE, KTIMEOUT, SAVE_BUFFER_FLAG);
decode_results results;
const uint8_t kTolerancePercentage = 25;

manual_control ac_manual_control_t;
teaching_mode teaching_mode_t;

bool ac_remote_unsupported_flag = false;

/*IR Transmitter Initializations*/

/*AC Class Objects*/
IRDaikinESP ac_daikin280(IR_TRAN_GPIO);
IRDaikin216 ac_daikin216(IR_TRAN_GPIO);
IRDaikin2 ac_daikin2(IR_TRAN_GPIO);
IRDaikin160 ac_daikin160(IR_TRAN_GPIO);
IRDaikin176 ac_daikin176(IR_TRAN_GPIO);
IRDaikin200 ac_daikin200(IR_TRAN_GPIO);
IRDaikin64 ac_daikinac64(IR_TRAN_GPIO);
IRDaikin152 ac_daikin152(IR_TRAN_GPIO);
IRDaikin128 ac_daikin128(IR_TRAN_GPIO);
IRHitachiAc ac_hitachi224(IR_TRAN_GPIO);
IRHitachiAc1 ac_hitachi104(IR_TRAN_GPIO);
IRHitachiAc424 ac_hitachi424(IR_TRAN_GPIO);
IRHitachiAc344 ac_hitachi344(IR_TRAN_GPIO);
IRHitachiAc264 ac_hitachi264(IR_TRAN_GPIO);
IRHitachiAc296 ac_hitachi296(IR_TRAN_GPIO);
IRVoltas ac_voltas(IR_TRAN_GPIO);
IRSamsungAc ac_samsung(IR_TRAN_GPIO);
IRHaierAC ac_haier(IR_TRAN_GPIO);
IRHaierAC176 ac_haier176(IR_TRAN_GPIO);
IRHaierAC160 ac_haier160(IR_TRAN_GPIO);
IRLgAc ac_lg(IR_TRAN_GPIO);
IRToshibaAC ac_toshiba(IR_TRAN_GPIO);
IRCarrierAc64 ac_carrier64(IR_TRAN_GPIO);
IRMitsubishi112 ac_mitsubishi112(IR_TRAN_GPIO);
IRMitsubishi136 ac_mitsubishi136(IR_TRAN_GPIO);
IRMitsubishiAC ac_mitsubishi144(IR_TRAN_GPIO);
IRMitsubishiHeavy88Ac ac_mitsubishi88(IR_TRAN_GPIO);
IRMitsubishiHeavy152Ac ac_mitsubishi152(IR_TRAN_GPIO);
IRsend ac_custom(IR_TRAN_GPIO);

/**
 * @brief Function that initializes the ir Transmitter setup.
 * Calls the begin function of all AC class objects
 */
void ir_tran_setup()
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

/**
 * @brief Function that transmits the ir command to control AC
 *
 */
void ir_transmit()
{
    irrecv.pause();
    vTaskDelay(pdMS_TO_TICKS(1000));
    led_set_state(LED_STATE_SENDING_IR_COMMAND);
    switch (ir_protocol_num)
    {
    case RAW:
        if (!last_command.power)
            ac_custom.sendRaw(teachingModeIrCmds[0], teaching_mode_raw_len, KHZ_41);
        else
        {
            ac_custom.sendRaw(teachingModeIrCmds[last_command.temperature - teaching_mode_t.startingTemperature + 1], teaching_mode_raw_len, KHZ_41);
        }
        break;

    case DAIKIN:
        ac_daikin280.setPower(last_command.power);
        ac_daikin280.setTemp(last_command.temperature);
        if (last_command.swingh)
            last_command.swingh = kDaikinSwingOn;
        ac_daikin280.setSwingHorizontal(last_command.swingh);
        if (last_command.swingv)
            last_command.swingv = kDaikinSwingOn;
        ac_daikin280.setSwingVertical(last_command.swingv);
        ac_daikin280.setFan(ac_daikin280.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_daikin280.enableOffTimer(last_command.offtimer);
        ac_daikin280.enableOnTimer(last_command.ontimer);
        ac_daikin280.setMode(ac_daikin280.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_daikin280.send();
        break;

    case DAIKIN200:
        ac_daikin200.setPower(last_command.power);
        ac_daikin200.setSwingHorizontal(last_command.swingh);
        ac_daikin200.setFan(last_command.fanspeed);
        ac_daikin200.setTemp(last_command.temperature);
        ac_daikin200.setMode(ac_daikin200.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_daikin200.send();
        break;

    case DAIKIN216:
        ac_daikin216.setPower(last_command.power);
        ac_daikin216.setTemp(last_command.temperature);
        if (last_command.swingh)
            last_command.swingh = kDaikinSwingOn;
        ac_daikin216.setSwingHorizontal(last_command.swingh);
        if (last_command.swingv)
            last_command.swingv = kDaikinSwingOn;
        ac_daikin216.setSwingVertical(last_command.swingv);
        ac_daikin216.setFan(ac_daikin216.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_daikin216.setMode(ac_daikin216.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_daikin216.send();
        break;

        // case DAIKIN2:
        //     ac_daikin2.setPower(last_command.power);
        //     ac_daikin2.setTemp(last_command.temperature);
        //     if (last_command.swingh)
        //         ac_daikin2.setSwingHorizontal(kDaikin2swinghAuto);
        //     else
        //         ac_daikin2.setSwingHorizontal(kDaikin2swinghOff);
        //     if (last_command.swingv)
        //         ac_daikin2.setSwingVertical(kDaikin2swingvAuto);
        //     else
        //         ac_daikin2.setSwingVertical(kDaikin2swingvOff);
        //     ac_daikin2.setFan(ac_daikin2.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        //     ac_daikin2.enableOffTimer(last_command.offtimer);
        //     ac_daikin2.enableOnTimer(last_command.ontimer);
        //     ac_daikin2.setMode(ac_daikin2.convertMode((stdAc::opmode_t)last_command.mode_num));
        //     ac_daikin2.send();
        //     break;

        // case DAIKIN160:
        //     ac_daikin160.setPower(last_command.power);
        //     ac_daikin160.setTemp(last_command.temperature);
        //     if (last_command.swingv)
        //         ac_daikin160.setSwingVertical(kDaikin160swingvAuto);
        //     ac_daikin160.setFan(ac_daikin160.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        //     ac_daikin160.setMode(ac_daikin160.convertMode((stdAc::opmode_t)last_command.mode_num));
        //     ac_daikin160.send();
        //     break;

        // case DAIKIN176:
        //     ac_daikin176.setPower(last_command.power);
        //     ac_daikin176.setTemp(last_command.temperature);
        //     if (last_command.swingh)
        //         ac_daikin176.setSwingHorizontal(kDaikin176swinghAuto);
        //     else
        //         ac_daikin176.setSwingHorizontal(kDaikin176swinghOff);
        //     ac_daikin176.setFan(ac_daikin176.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        //     ac_daikin176.setMode(ac_daikin176.convertMode((stdAc::opmode_t)last_command.mode_num));
        //     ac_daikin176.send();
        //     break;

    case DAIKIN64:
        ac_daikinac64.setPowerToggle(last_command.power);
        ac_daikinac64.setTemp(last_command.temperature);
        ac_daikinac64.setSwingVertical(last_command.swingv);
        ac_daikinac64.setFan(ac_daikinac64.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_daikinac64.setOnTime(last_command.offtimer);
        ac_daikinac64.setOffTime(last_command.ontimer);
        ac_daikinac64.setMode(ac_daikinac64.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_daikinac64.send();
        break;

    case DAIKIN152:
        ac_daikin152.setPower(last_command.power);
        ac_daikin152.setTemp(last_command.temperature);
        ac_daikin152.setSwingV(last_command.swingv);
        ac_daikin152.setFan(ac_daikin152.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_daikin152.setMode(ac_daikin152.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_daikin152.send();
        break;

    case DAIKIN128:
        ac_daikin128.setPowerToggle(last_command.power);
        ac_daikin128.setTemp(last_command.temperature);
        ac_daikin128.setSwingVertical(last_command.swingv);
        ac_daikin128.setFan(ac_daikin128.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_daikin128.setOffTimer(last_command.offtimer);
        ac_daikin128.setOnTimer(last_command.ontimer);
        ac_daikin128.setMode(ac_daikin128.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_daikin128.send();
        break;

    case HITACHI_AC296:
        ac_hitachi296.setPower(last_command.power);
        ac_hitachi296.setTemp(last_command.temperature);
        ac_hitachi296.setFan(ac_hitachi296.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_hitachi296.setMode(ac_hitachi296.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_hitachi296.send();
        break;

    case HITACHI_AC:
        ac_hitachi224.setPower(last_command.power);
        ac_hitachi224.setTemp(last_command.temperature);
        ac_hitachi224.setFan(ac_hitachi224.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_hitachi224.setSwingHorizontal(last_command.swingh);
        ac_hitachi224.setSwingVertical(last_command.swingv);
        ac_hitachi224.setMode(ac_hitachi224.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_hitachi224.send();
        break;

    case HITACHI_AC1:
        ac_hitachi104.setPower(last_command.power);
        ac_hitachi104.setTemp(last_command.temperature);
        ac_hitachi104.setFan(ac_hitachi104.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_hitachi104.setSwingH(last_command.swingh);
        ac_hitachi104.setSwingV(last_command.swingv);
        ac_hitachi104.setOffTimer(last_command.offtimer);
        ac_hitachi104.setOnTimer(last_command.ontimer);
        ac_hitachi104.setMode(ac_hitachi104.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_hitachi104.send();
        break;

    case HITACHI_AC424:
        ac_hitachi424.setPower(last_command.power);
        ac_hitachi424.setTemp(last_command.temperature);
        ac_hitachi424.setFan(ac_hitachi424.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_hitachi424.setSwingVToggle(last_command.swingv);
        ac_hitachi424.setMode(ac_hitachi424.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_hitachi424.send();
        break;

    case HITACHI_AC344:
        ac_hitachi344.setSwingH(last_command.swingh);
        ac_hitachi344.setSwingV(last_command.swingv);
        ac_hitachi344.send();
        break;

    case HITACHI_AC264:
        ac_hitachi264.setFan(ac_hitachi264.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_hitachi264.send();
        break;

    case VOLTAS:
        ac_voltas.setPower(last_command.power);
        ac_voltas.setTemp(last_command.temperature);
        ac_voltas.setSwingH(last_command.swingh);
        ac_voltas.setSwingV(last_command.swingv);
        ac_voltas.setFan(ac_voltas.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_voltas.setOffTime(last_command.offtimer);
        ac_voltas.setOnTime(last_command.ontimer);
        ac_voltas.setMode(ac_voltas.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_voltas.send();
        break;

    case SAMSUNG_AC:
        ac_samsung.setPower(last_command.power);
        ac_samsung.setTemp(last_command.temperature);
        ac_samsung.setSwingH(last_command.swingh);
        ac_samsung.setSwing(last_command.swingv);
        ac_samsung.setFan(ac_samsung.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_samsung.setOffTimer(last_command.offtimer);
        ac_samsung.setOnTimer(last_command.ontimer);
        ac_samsung.setMode(ac_samsung.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_samsung.send();
        break;

    case HAIER_AC:
        ac_haier.setTemp(last_command.temperature);
        ac_haier.setSwingV(last_command.swingv);
        ac_haier.setFan(ac_haier.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_haier.setOffTimer(last_command.offtimer);
        ac_haier.setOnTimer(last_command.ontimer);
        ac_haier.setMode(ac_haier.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_haier.send();
        break;

    case HAIER_AC176:
        ac_haier176.setPower(last_command.power);
        ac_haier176.setTemp(last_command.temperature);
        ac_haier176.setSwingH(last_command.swingh);
        ac_haier176.setSwingV(last_command.swingv);
        ac_haier176.setFan(ac_haier176.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_haier176.setOffTimer(last_command.offtimer);
        ac_haier176.setOnTimer(last_command.ontimer);
        ac_haier176.setMode(ac_haier176.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_haier176.send();
        break;

    case HAIER_AC160:
        ac_haier160.setPower(last_command.power);
        ac_haier160.setTemp(last_command.temperature);
        ac_haier160.setSwingV(last_command.swingv);
        ac_haier160.setFan(ac_haier160.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_haier160.setOffTimer(last_command.offtimer);
        ac_haier160.setOnTimer(last_command.ontimer);
        ac_haier160.setMode(ac_haier160.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_haier160.send();
        break;

    case CARRIER_AC64:
        ac_carrier64.setPower(last_command.power);
        ac_carrier64.setTemp(last_command.temperature);
        ac_carrier64.setSwingV(last_command.swingv);
        ac_carrier64.setFan(ac_carrier64.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_carrier64.setOffTimer(last_command.offtimer);
        ac_carrier64.setOnTimer(last_command.ontimer);
        ac_carrier64.setMode(ac_carrier64.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_carrier64.send();
        break;

    case LG2:
    case LG:
        ac_lg.setPower(last_command.power);
        ac_lg.setTemp(last_command.temperature);
        ac_lg.setSwingH(last_command.swingh);
        ac_lg.setSwingV(last_command.swingv);
        ac_lg.setFan(ac_lg.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_lg.setMode(ac_lg.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_lg.send();
        break;

    case TOSHIBA_AC:
        ac_toshiba.setPower(last_command.power);
        ac_toshiba.setTemp(last_command.temperature);
        ac_toshiba.setFan(ac_toshiba.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        if (last_command.swingv || last_command.swingh)
            ac_toshiba.setSwing(kToshibaAcSwingOn);
        else
            ac_toshiba.setSwing(kToshibaAcSwingOff);
        ac_toshiba.setMode(ac_toshiba.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_toshiba.send();
        break;

    case MITSUBISHI112:
        ac_mitsubishi112.setPower(last_command.power);
        ac_mitsubishi112.setTemp(last_command.temperature);
        ac_mitsubishi112.setSwingH(last_command.swingh);
        ac_mitsubishi112.setSwingV(last_command.swingv);
        ac_mitsubishi112.setFan(ac_mitsubishi112.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_mitsubishi112.setMode(ac_mitsubishi112.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_mitsubishi112.send();
        break;

        // case MITSUBISHI136:
        //     ac_mitsubishi136.setPower(last_command.power);
        //     ac_mitsubishi136.setTemp(last_command.temperature);
        //     if (last_command.swingv)
        //     {
        //         ac_mitsubishi136.setSwingV(kMitsubishi136swingvAuto);
        //     }
        //     ac_mitsubishi136.setFan(ac_mitsubishi136.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        //     ac_mitsubishi136.setMode(ac_mitsubishi136.convertMode((stdAc::opmode_t)last_command.mode_num));
        //     ac_mitsubishi136.send();
        //     break;

    case MITSUBISHI_AC:
        ac_mitsubishi144.setPower(last_command.power);
        ac_mitsubishi144.setTemp(last_command.temperature);
        ac_mitsubishi144.setFan(ac_mitsubishi144.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        ac_mitsubishi144.setMode(ac_mitsubishi144.convertMode((stdAc::opmode_t)last_command.mode_num));
        ac_mitsubishi144.send();
        break;

        // case MITSUBISHI_HEAVY_88:
        //     ac_mitsubishi88.setPower(last_command.power);
        //     ac_mitsubishi88.setTemp(last_command.temperature);
        //     ac_mitsubishi88.setFan(ac_mitsubishi88.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        //     if (last_command.swingh)
        //         ac_mitsubishi88.setSwingHorizontal(kMitsubishiHeavy88swinghAuto);
        //     else
        //         ac_mitsubishi88.setSwingHorizontal(kMitsubishiHeavy88swinghOff);
        //     if (last_command.swingv)
        //         ac_mitsubishi88.setSwingVertical(kMitsubishiHeavy88swingvAuto);
        //     else
        //         ac_mitsubishi88.setSwingVertical(kMitsubishiHeavy88swingvOff);
        //     ac_mitsubishi88.setMode(ac_mitsubishi88.convertMode((stdAc::opmode_t)last_command.mode_num));
        //     ac_mitsubishi88.send();
        //     break;

        // case MITSUBISHI_HEAVY_152:
        //     ac_mitsubishi152.setPower(last_command.power);
        //     ac_mitsubishi152.setTemp(last_command.temperature);
        //     ac_mitsubishi152.setFan(ac_mitsubishi152.convertFan((stdAc::fanspeed_t)last_command.fanspeed));
        //     if (last_command.swingh)
        //         ac_mitsubishi152.setSwingHorizontal(kMitsubishiHeavy88swinghAuto);
        //     else
        //         ac_mitsubishi152.setSwingHorizontal(kMitsubishiHeavy88swinghOff);
        //     if (last_command.swingv)
        //         ac_mitsubishi88.setSwingVertical(kMitsubishiHeavy88swingvAuto);
        //     else
        //         ac_mitsubishi152.setSwingVertical(kMitsubishiHeavy88swingvOff);
        //     ac_mitsubishi152.setMode(ac_mitsubishi152.convertMode((stdAc::opmode_t)last_command.mode_num));
        //     ac_mitsubishi152.send();
        //     break;
    }
    irrecv.resume();
}

/**
 * @brief Function that returns the detected protocol's name
 * @param protocol
 * @return char*
 */
const char *get_protocol_string(int16_t protocol)
{
    switch (protocol)
    {
    case RAW:
        return RAW_IR_PROTOCOL;
    case DAIKIN:
        return DAIKIN_IR_PROTOCOL;
    case DAIKIN200:
        return DAIKIN200_IR_PROTOCOL;
    case DAIKIN216:
        return DAIKIN216_IR_PROTOCOL;
    case DAIKIN2:
        return DAIKIN2_IR_PROTOCOL;
    case DAIKIN160:
        return DAIKIN160_IR_PROTOCOL;
    case DAIKIN176:
        return DAIKIN176_IR_PROTOCOL;
    case DAIKIN64:
        return DAIKIN64_IR_PROTOCOL;
    case DAIKIN152:
        return DAIKIN152_IR_PROTOCOL;
    case DAIKIN128:
        return DAIKIN128_IR_PROTOCOL;
    case HITACHI_AC296:
        return HITACHI_AC296_IR_PROTOCOL;
    case HITACHI_AC:
        return HITACHI_AC_IR_PROTOCOL;
    case HITACHI_AC1:
        return HITACHI_AC1_IR_PROTOCOL;
    case HITACHI_AC424:
        return HITACHI_AC424_IR_PROTOCOL;
    case HITACHI_AC344:
        return HITACHI_AC344_IR_PROTOCOL;
    case HITACHI_AC264:
        return HITACHI_AC264_IR_PROTOCOL;
    case VOLTAS:
        return VOLTAS_IR_PROTOCOL;
    case SAMSUNG_AC:
        return SAMSUNG_AC_IR_PROTOCOL;
    case HAIER_AC:
        return HAIER_AC_IR_PROTOCOL;
    case HAIER_AC176:
        return HAIER_AC176_IR_PROTOCOL;
    case HAIER_AC160:
        return HAIER_AC160_IR_PROTOCOL;
    case CARRIER_AC64:
        return CARRIER_AC64_IR_PROTOCOL;
    case LG2:
        return LG2_IR_PROTOCOL;
    case LG:
        return LG_IR_PROTOCOL;
    case TOSHIBA_AC:
        return TOSHIBA_AC_IR_PROTOCOL;
    case MITSUBISHI112:
        return MITSUBISHI112_IR_PROTOCOL;
    case MITSUBISHI136:
        return MITSUBISHI136_IR_PROTOCOL;
    case MITSUBISHI_AC:
        return MITSUBISHI_AC_IR_PROTOCOL;
    case MITSUBISHI_HEAVY_88:
        return MITSUBISHI_HEAVY_88_IR_PROTOCOL;
    case MITSUBISHI_HEAVY_152:
        return MITSUBISHI_HEAVY_152_IR_PROTOCOL;
    case UNKNOWN:
        return UNKNOWN_IR_PROTOCOL;
    case UNUSED:
        return UNUSED_IR_PROTOCOL;
    default:
        return INVALID_IR_PROTOCOL;
    }
}

/**
 * @brief Function that checks if the detected IR signal is a supported ir protocol or not
 * If yes, then device is allowed to get configured as the detected protocol. If no, then
 * device will not get configured.
 * @param protocol
 * @return true
 * @return false
 */
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
 * @brief Function that fetches the info of Power, Temperature, Fanspeed and Mode
 * from received IR Signal.
 * @warning Both teaching mode and Lockign feature depend on the Temperature value that was detected,
 * if that fails, then it's FATAL
 * @retval true = If Temperature parse was success
 * @retval false = If Temperature parse was failure
 */
bool isFetchControlInfoSuccessful(const char *description)
{
    /*Initializing to Defaults*/
    strcpy(ac_manual_control_t.temperature, "");
    strcpy(ac_manual_control_t.power, "");
    strcpy(ac_manual_control_t.fan, "");
    strcpy(ac_manual_control_t.mode, "");

    ac_manual_control_t.power_value = -1;
    ac_manual_control_t.fanspeed_value = -1;
    ac_manual_control_t.temperature_value = -1;

    ac_manual_control_t.power_err = SUCCESS;
    ac_manual_control_t.fanspeed_err = SUCCESS;
    ac_manual_control_t.mode_err = SUCCESS;
    ac_manual_control_t.temperature_err = SUCCESS;

    /*Starting Decoding Power, Mode, Fanspeed, Temperature Info*/
    if (strstr(description, "Power"))
    {
        snprintf(ac_manual_control_t.power, sizeof(ac_manual_control_t.power), (strstr(description, "Power") + 7));
        if (strstr(ac_manual_control_t.power, "On"))
            ac_manual_control_t.power_value = 1;
        else
            ac_manual_control_t.power_value = 0;
    }
    else
    {
        ac_manual_control_t.power_err = POWER_NOT_AVAILABLE_IN_IR_SIGNAL_DECODED_STRING;
        ac_manual_control_t.power_value = -1;
    }

    if (strstr(description, "Mode"))
    {
        snprintf(ac_manual_control_t.mode, sizeof(ac_manual_control_t.mode), (strstr(description, "Mode") + 9));
        if (strstr(ac_manual_control_t.mode, COOL_MODE_STR))
            strcpy(ac_manual_control_t.mode, COOL_MODE_STR);
        else if (strstr(ac_manual_control_t.mode, HEAT_MODE_STR))
            strcpy(ac_manual_control_t.mode, HEAT_MODE_STR);
        else if (strstr(ac_manual_control_t.mode, DRY_MODE_STR))
            strcpy(ac_manual_control_t.mode, DRY_MODE_STR);
        else if (strstr(ac_manual_control_t.mode, AUTO_MODE_STR))
            strcpy(ac_manual_control_t.mode, AUTO_MODE_STR);
        else if (strstr(ac_manual_control_t.mode, FAN_MODE_STR))
            strcpy(ac_manual_control_t.mode, FAN_MODE_STR);
    }
    else
    {
        ac_manual_control_t.mode_err = MODE_NOT_AVAILABLE_IN_IR_SIGNAL_DECODED_STRING;
        strcpy(ac_manual_control_t.mode, "-1");
    }

    if (strstr(description, "Fan"))
    {
        snprintf(ac_manual_control_t.fan, sizeof(ac_manual_control_t.fan), (strstr(description, "Fan") + 5));
        ac_manual_control_t.fanspeed_value = atoi(ac_manual_control_t.fan);
    }
    else
    {
        ac_manual_control_t.fanspeed_err = FANSPEED_NOT_AVAILABLE_IN_IR_SIGNAL_DECODED_STRING;
        ac_manual_control_t.fanspeed_value = -1;
    }

    if (strstr(description, "Temp"))
    {
        snprintf(ac_manual_control_t.temperature, sizeof(ac_manual_control_t.temperature), (strstr(description, "Temp") + 6));
        ac_manual_control_t.temperature_value = atoi(ac_manual_control_t.temperature);
        return true;
    }
    else
    {
        ac_manual_control_t.temperature_err = TEMPERATURE_NOT_AVAILABLE_IN_IR_SIGNAL_DECODED_STRING;
        ac_manual_control_t.temperature_value = -1;
        return false;
    }
}

/**
 * @brief Function that performs the AC locking feature
 */
void locking_feature(const char *description)
{
    if (isFetchControlInfoSuccessful(description))
    {
        if (last_command.locking && !(ac_manual_control_t.temperature_value >= last_command.lowerTemperatureLimit &&
                                      ac_manual_control_t.temperature_value <= last_command.upperTemperatureLimit))
        {
            ESP_LOGW(IR_TAG, "AC temperature limit exceeded  (Current set Temperature : %d) | (Limits %d - %d)",
                     ac_manual_control_t.temperature_value,
                     last_command.lowerTemperatureLimit,
                     last_command.upperTemperatureLimit);
            ESP_LOGW(IR_TAG, "Setting AC Temperature to : %d", last_command.temperature);
            ir_transmit();
        }
        else
        {
            last_command.power = ac_manual_control_t.power_value;
            last_command.temperature = ac_manual_control_t.temperature_value;
            last_command.fanspeed = ac_manual_control_t.fanspeed_value;
            strcpy(last_command.mode_str, ac_manual_control_t.mode);
            set_blob_in_nvs_flash(IR_HANDLE, NVS_LAST_COMMAND_KEY, &last_command, sizeof(CommandStruct));
        }
    }
#if (IS_GWY)
    generate_ack(GWY_MANUAL_AC_CONTROL_ACK, NULL);
#endif

#if (!IS_GWY)
    send_manual_control_ack_to_provisioner(&ac_manual_control_t);
#endif
}

/**
 * @brief Function that initializes the value for teaching mode process
 * @param startingTemp Starting Temperature of Teaching Mode
 * @param endingTemp Ending Temperature of Teaching Mode
 */
void teaching_mode_init(uint8_t startingTemp, uint8_t endingTemp)
{
    teaching_in_progress = true;
    ESP_LOGW(IR_TAG, "Device Entered Teaching mode");
    update_led_status();
    teaching_mode_t.errorCode = ENTERED_TEACHING_MODE;
    teaching_mode_t.expectedTemperature = startingTemp;
    teaching_mode_t.commandIndex = startingTemp - MAX_LOW_TEMP;
    teaching_mode_t.remainingCommands = endingTemp - startingTemp + 2;
    strcpy(teaching_mode_t.lastCommand, "");
    strcpy(teaching_mode_t.nextCommand, TEACHING_MODE_SEQUENCE[0]);
    set_number_in_nvs_flash(IR_HANDLE, NVS_TEACHING_MODE_STARTING_TEMPERATURE_KEY, startingTemp, UINT8_SIZE);
    set_number_in_nvs_flash(IR_HANDLE, NVS_TEACHING_MODE_ENDING_TEMPERATURE_KEY, endingTemp, UINT8_SIZE);
#if (IS_GWY)
    generate_ack(GWY_TEACHING_MODE, NULL);
#endif
#if (!IS_GWY)
    send_teaching_mode_ack_to_provisioner(&teaching_mode_t);
#endif
}

/**
 * @brief Function that exits the device from teaching mode
 * @param success - Flag indicating if its a successful exit or forced exit
 * if successful exit, then teaching mode is completed and hence we can set
 * the configured flag, if not, then don't
 */
void exit_teaching_mode(bool success)
{
    if (success)
    {
        configured = true;
        ir_protocol_num = RAW;
        strcpy(ir_protocol, get_protocol_string(ir_protocol_num));
        set_number_in_nvs_flash(IR_HANDLE, NVS_IR_PROTOCOL_KEY, ir_protocol_num, INT16_SIZE);
        ESP_LOGI(IR_TAG, "Teaching mode completed successfully");
    }
    else ESP_LOGW(IR_TAG, "Quitting teaching mode without completion");
    strcpy(teaching_mode_t.nextCommand, "");
    teaching_in_progress = false;
    update_led_status();
    teaching_mode_t.errorCode = EXITED_TEACHING_MODE;
#if (IS_GWY)
    generate_ack(GWY_TEACHING_MODE, NULL);
#endif
#if (!IS_GWY)
    send_teaching_mode_ack_to_provisioner(&teaching_mode_t);
#endif
}

/**
 * @brief Function that performs the teaching process.
 * - 1) Detects IR Signal
 * - 2) Decodes information from IR Signal = Temperature, Power
 * - 3) Checks if the expected IR Signal
 * - 4) If Yes, saves it to nvs flash
 * - 5) Steps 1 to 4 is repeated until expected number of IR Signals have been received
 * - 6) Quits device from Teaching mode
 */
void perform_teaching_process(const char *description)
{
    /*Let's pause the IR Reception until we process the recevied ir signal*/
    irrecv.pause();

    if (isFetchControlInfoSuccessful(description) || teaching_mode_t.commandsReceived == 0)
    {
        ESP_LOGE(IR_TAG, "Detected Temperature : %d | Detected Power : %d | Required Temperature : %d | Required Power : %d",
                 ac_manual_control_t.temperature_value, ac_manual_control_t.power_value, teaching_mode_t.expectedTemperature, 0);
        /*If this is the first IR Signal*/
        if (teaching_mode_t.commandsReceived == 0)
        {
            /**
             * @warning We're only comparing power here and not temperature, because, for some AC remotes
             * like LG2, when sending power OFF command, the decoded string doesn't seem to contain temperature at all.
             *
             */
            if (ac_manual_control_t.power_value == 0)
            {
                set_number_in_nvs_flash(IR_HANDLE, NVS_RAWLEN_KEY, results.rawlen, UINT16_SIZE);

                /*Let's store raw values to nvs flash*/
                set_blob_in_nvs_flash(IR_HANDLE, NVS_TEACHING_MODE_CMD_KEYS[0], (const void *)results.rawbuf, results.rawlen - 1);

                /*We've received a valid first IR Signal*/
                teaching_mode_t.commandsReceived++;
                teaching_mode_t.commandIndex++;
                teaching_mode_t.remainingCommands--;
                teaching_mode_t.errorCode = SUCCESS;
                strcpy(teaching_mode_t.lastCommand, TEACHING_MODE_SEQUENCE[0]);
                strcpy(teaching_mode_t.nextCommand, TEACHING_MODE_SEQUENCE[teaching_mode_t.commandIndex]);
            }
            else
            {
                led_set_state(LED_STATE_INVALID_OPERATION);
                teaching_mode_t.errorCode = FAILURE;
            }
        }
        else
        {
            if (ac_manual_control_t.power_value == 1 && ac_manual_control_t.temperature_value == teaching_mode_t.expectedTemperature)
            {
                /*Let's store raw values to nvs flash*/
                set_blob_in_nvs_flash(IR_HANDLE, NVS_TEACHING_MODE_CMD_KEYS[teaching_mode_t.commandIndex], (const void *)results.rawbuf, results.rawlen - 1);

                /*We've received a valid first IR Signal*/
                teaching_mode_t.commandsReceived++;
                teaching_mode_t.commandIndex++;
                teaching_mode_t.remainingCommands--;
                teaching_mode_t.errorCode = SUCCESS;
                teaching_mode_t.expectedTemperature++;
                strcpy(teaching_mode_t.lastCommand, teaching_mode_t.nextCommand);
                strcpy(teaching_mode_t.nextCommand, TEACHING_MODE_SEQUENCE[teaching_mode_t.commandIndex]);
            }
            else
            {
                led_set_state(LED_STATE_INVALID_OPERATION);
                teaching_mode_t.errorCode = FAILURE;
            }
        }
    }
    else
    {
        led_set_state(LED_STATE_INVALID_OPERATION);
        teaching_mode_t.errorCode = TEMPERATURE_NOT_AVAILABLE_IN_IR_SIGNAL_DECODED_STRING;
    }
    if (teaching_mode_t.remainingCommands != 0)
#if (IS_GWY)
        generate_ack(GWY_TEACHING_MODE, NULL);
#endif
#if (!IS_GWY)
        send_teaching_mode_ack_to_provisioner(&teaching_mode_t);
#endif

    /*Teaching mode has been successfully completed*/
    else
        exit_teaching_mode(true);

    irrecv.resume();
}

/**
 * @brief Thread that takes care of decoding IR signals
 * @param args
 */
void ir_recv_task(void *args)
{
    irrecv.setUnknownThreshold(12);
    irrecv.setTolerance(kTolerancePercentage);
    irrecv.enableIRIn();
    while (1)
    {
        if (irrecv.decode(&results))
        {
            decode_type_t protocol = UNKNOWN;
            if (results.overflow)
                ESP_LOGW(IR_TAG, "IR Buffer overflow");
            ESP_LOGW(IR_TAG, "kTolerancePercentage : %d", kTolerancePercentage);
            ESP_LOGW(IR_TAG, "%s", resultToHumanReadableBasic(&results, &protocol).c_str());
            ESP_LOGW(IR_TAG, "Protocol Number : %d", protocol);
            String description = IRAcUtils::resultAcToString(&results);
            if (description.length())
                ESP_LOGW(IR_TAG, "%s\n", description.c_str());

            /**  @warning DO NOT CHANGE THE FOLLOWING ORDER
             * 1) locking Feature
             * 2) Teaching Mode
             * 3) AC Remote Configuration
             * 4) Unsupported AC Remote Check
             */

            /*Locking Feature*/
            if ((registered || provisioned) && configured && !teaching_in_progress &&
                ((protocol == ir_protocol_num) || (ir_protocol_num == RAW && results.rawlen == teaching_mode_raw_len)) &&
                description.length())
            {
                locking_feature(description.c_str());
                continue;
            }

            /*Teaching Mode*/
            else if (teaching_in_progress)
            {
                perform_teaching_process(description.c_str());
                continue;
            }

            /*AC Remote Configuration Process*/
            else if ((registered || provisioned) && !configured && protocol != UNKNOWN && !teaching_in_progress)
            {
                configured = true;
                update_led_status();
                ir_protocol_num = protocol;
                strcpy(ir_protocol, get_protocol_string(ir_protocol_num));
                set_number_in_nvs_flash(IR_HANDLE, NVS_IR_PROTOCOL_KEY, ir_protocol_num, INT16_SIZE);
                set_number_in_nvs_flash(GENERAL_HANDLE, NVS_CONFIGURED_KEY, 1, UINT8_SIZE);
#if (IS_GWY)
                generate_ack(GWY_CONF_ACK, NULL);
#endif
#if (!IS_GWY)
                send_ack_to_provisioner(NODE_CONF_ACK, NULL);
#endif
                ESP_LOGI(IR_TAG, "AC Remote configuration successful");
            }

            /*Unsupported AC Remote*/
            else if (!configured && protocol == UNKNOWN)
            {
                ac_remote_unsupported_flag = true;
                led_set_state(LED_STATE_INVALID_OPERATION);
                ESP_LOGD(IR_TAG, "AC Remote Unsupported");
#if (IS_GWY)
                generate_ack(GWY_CONF_ACK, NULL);
#endif
#if (!IS_GWY)
                send_ack_to_provisioner(NODE_CONF_ACK, NULL);
#endif
                ac_remote_unsupported_flag = false;
            }

            yield();
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/**
 * @brief Function that takes are of setting the configured to false,
 * and updating the flash. Common function to both provisioner and node
 * 
 */
void handle_reconfiguration()
{
    ir_protocol_num = -1;
    strcpy(ir_protocol, get_protocol_string(ir_protocol_num));
    set_number_in_nvs_flash(GENERAL_HANDLE, NVS_CONFIGURED_KEY, 0, UINT8_SIZE);
    set_number_in_nvs_flash(IR_HANDLE, NVS_IR_PROTOCOL_KEY, ir_protocol_num, INT16_SIZE);
    configured = 0; update_led_status();
}

/**
 * @brief Function that takes care of configuring teaching mode.
 * Common function for both provisioner and node.
 * @param cmd_struct 
 */
void handle_configuring_teaching_mode(CommandStruct *cmd_struct)
{
    teaching_mode_t.errorCode = cmd_struct->errorcode;
    teaching_mode_t.teachingStart = cmd_struct->teachingStart;
    teaching_mode_t.startingTemperature = cmd_struct->startingTemperature;
    teaching_mode_t.endingTemperature = cmd_struct->endingTemperature;
    if(teaching_mode_t.teachingStart) {
        teaching_mode_init(teaching_mode_t.startingTemperature, teaching_mode_t.endingTemperature);
    }
    else exit_teaching_mode(false);
}