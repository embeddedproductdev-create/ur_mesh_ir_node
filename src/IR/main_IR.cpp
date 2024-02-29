/**
 * @file main_IR_recv.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions related to the IR receiver part
 * @version 0.1
 * @date 2024-02-29
 * @copyright Copyright (c) 2024
 */

#include "../../inc/IR/main_IR.h"

//Initialization - Receiver
bool configured = false;
IRrecv irrecv(IR_RECEIVER_PIN, RECV_BUFFER_SIZE, kTimeout, true);
decode_results results;
decode_type_t protocol_detected = UNKNOWN;
char protocol_chosen[15] = "";

//Initialization - Transmitter
bool sending = false;
bool needtosend = false;

IRDaikinESP ac_daikin280(IR_TRANSMIT_PIN);
IRDaikin216 ac_daikin216(IR_TRANSMIT_PIN);
IRHitachiAc296 ac_hitachi296(IR_TRANSMIT_PIN);
IRVoltas ac_voltas(IR_TRANSMIT_PIN);

/**
 * @brief Thread that handles the IR signals received. Detects and sets the IR tranmsmission protocol
 * @param args
 * @return void*
 */
void *IR_receiver_task(void *args)
{
    Serial.begin(BAUD_RATE);
    while(!Serial)
        delay(50);
    Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", IR_RECEIVER_PIN);
    irrecv.setUnknownThreshold(kMinUnknownSize);
    irrecv.setTolerance(kTolerancePercentage);
    irrecv.enableIRIn();

    while(1)
    {
        vTaskDelay(10);
        // Check if the IR code has been received.
        if (irrecv.decode(&results)) {
            // Display a crude timestamp.;
            uint32_t now = millis();
            printf(D_STR_TIMESTAMP " : %06lu.%03lu\n", now / 1000, now % 1000);
            // Check if we got an IR message that was to big for our capture buffer.
            if (results.overflow)
            printf(D_WARN_BUFFERFULL "\n", RECV_BUFFER_SIZE);
            // Display the library version the message was captured with.
            Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_STR "\n");
            // Display the tolerance percentage if it has been change from the default.
            if (kTolerancePercentage != kTolerance)
            Serial.printf(D_STR_TOLERANCE " : %d%%\n", kTolerancePercentage);
            // Display the basic output of what we found.
            Serial.print(resultToHumanReadableBasic(&results, &protocol_detected));
            // Display any extra A/C info if we have it.
            String description = IRAcUtils::resultAcToString(&results);
            if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
            if(protocol_detected != UNKNOWN && protocol_detected != UNUSED)
                configured = true;
            printf("protocol_detected : %d\n",protocol_detected);
            yield();  // Feed the WDT as the text output can take a while to print.
        #if LEGACY_TIMING_INFO
            // Output legacy RAW timing info of the result.
            Serial.println(resultToTimingInfo(&results));
            yield();  // Feed the WDT (again)
        #endif  // LEGACY_TIMING_INFO
            // Output the results as source code
            Serial.println(resultToSourceCode(&results));
            Serial.println();    // Blank line between entries
            yield();             // Feed the WDT (again)
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
    ac_daikin216.begin();
    ac_daikin280.begin();
    ac_hitachi296.begin();
}

/**
 * @brief Function that handles the IR transmission part
 * @param none
 * @retval none
 */
void IR_transmit(uint16_t protocol_detected, char *protocol_chosen)
{
    sending = true;
    switch(protocol_detected)
    {
        default:
            printf("Error in choosing the protocol for send\r\n");
            return;
        case DAIKIN216:
            strcpy(protocol_chosen, "Daikin216");
            ac_daikin216.setPower(gwy_ac_control_t.power);
            ac_daikin216.setTemp(gwy_ac_control_t.temp);
            if(gwy_ac_control_t.swingH) gwy_ac_control_t.swingH = kDaikinSwingOn;
            ac_daikin216.setSwingHorizontal(gwy_ac_control_t.swingH);
            if(gwy_ac_control_t.swingV) gwy_ac_control_t.swingV = kDaikinSwingOn;
            ac_daikin216.setSwingVertical(gwy_ac_control_t.swingV);
            ac_daikin216.setFan(gwy_ac_control_t.fan);
            ac_daikin216.send();
            printf("Sending Daikin216 with Power = %d\r\n",gwy_ac_control_t.power);
            break;
        case DAIKIN200:
            strcpy(protocol_chosen, "Daikin200");
            printf("\r\n");
            return;
        case DAIKIN:
            strcpy(protocol_chosen, "Daikin280");
            ac_daikin280.setPower(gwy_ac_control_t.power);
            ac_daikin280.setTemp(gwy_ac_control_t.temp);
            if(gwy_ac_control_t.swingH) gwy_ac_control_t.swingH = kDaikinSwingOn;
            ac_daikin280.setSwingHorizontal(gwy_ac_control_t.swingH);
            if(gwy_ac_control_t.swingV) gwy_ac_control_t.swingV = kDaikinSwingOn;
            ac_daikin280.setSwingVertical(gwy_ac_control_t.swingV);
            ac_daikin280.setFan(gwy_ac_control_t.fan);
            ac_daikin280.send();
            printf("Sending Daikin280\r\n");
            break;
        case HITACHI_AC296:
            strcpy(protocol_chosen, "Hitachi296");
            ac_hitachi296.setPower(gwy_ac_control_t.power);
            ac_hitachi296.setTemp(gwy_ac_control_t.temp);
            // ac_hitachi296.setSwingHorizontal(gwy_ac_control_t.swingH);
            // ac_hitachi296.setSwingVertical(gwy_ac_control_t.swingV);
            ac_hitachi296.setFan(gwy_ac_control_t.fan-3);
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

/**
 * @brief Thread that handles the Button press
 * @param args
 * @return void*
 */
void *button_task(void *args)
{
    pinMode(USER_SWITCH, INPUT);
    protocol_detected = DAIKIN216;
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        if(!digitalRead(USER_SWITCH)) //Inverted logic as per the schematic
        {
            IR_transmit(protocol_detected, protocol_chosen);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

