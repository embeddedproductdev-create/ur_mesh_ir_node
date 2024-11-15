#include "../../components/arduino/libraries/IRremoteESP8266/src/IRac.h"
#include "../../components/arduino/libraries/IRremoteESP8266/src/IRutils.h"
#include "../inc/ir.h"

/*IR Receiver Initializations*/
IRrecv irrecv(IR_RECV_GPIO, RECV_BUFFER_SIZE, KTIMEOUT, SAVE_BUFFER_FLAG);
decode_results results;
decode_type_t protocol_detected = UNKNOWN;

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
 * @brief Function that returns the detected protocol's name
 * @param protocol 
 * @return char* 
 */
char* get_protocol_string(uint16_t protocol)
{
    switch(protocol)
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
    switch(protocol)
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
 * @brief Function that configures the IR Receiver parameters like thresh and tolerance
 * helpful while decoding detected IR signals
 * 
 */
void ir_recv_configure()
{
    irrecv.setUnknownThreshold(12);
    irrecv.setTolerance(25);
    irrecv.enableIRIn();
}

void decode_ir()
{
    decode_results results;
    decode_type_t ir_protocol_detected;
    if(irrecv.decode(&results))
    {
        resultToHumanReadableBasic(&results, &ir_protocol_detected);
        IRAcUtils::resultAcToString(&results);
    }
}