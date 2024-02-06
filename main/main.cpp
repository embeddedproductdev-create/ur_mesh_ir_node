#include "Arduino.h"
#include "main.h"
#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

bool configured = false;
bool sending = false;
bool needtosend = true;
bool restart_flag = false;

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;
decode_type_t protocol_detected = UNKNOWN;
char protocol_chosen[15] = "";

/*Object initializations*/
IRDaikinESP ac_daikin280(kSendPin);
IRDaikin216 ac_daikin216(kSendPin);
IRHitachiAc296 ac_hitachi296(kSendPin);
IRVoltas ac_voltas(kSendPin);

extern "C"
{
    void app_main(void);
}

void send_func()
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
            ac_daikin216.setSwingHorizontal(ac_control_t.swingH);
            ac_daikin216.setSwingVertical(ac_control_t.swingV);
            ac_daikin216.setFan(ac_control_t.fan);
            ac_daikin216.send();
            printf("Protocol Chosen Daikin216\r\n");
            break;
        case DAIKIN200:
            strcpy(protocol_chosen, "Daikin200");
            printf("Protocol Chosen Daikin200\r\n");
            return;
        case DAIKIN:
            strcpy(protocol_chosen, "Daikin280");
            ac_daikin280.setPower(ac_control_t.power);
            ac_daikin280.send();
            printf("Protocol Chosen Daikin280\r\n");
            break;
        case HITACHI_AC296:
            strcpy(protocol_chosen, "Hitachi296");
            ac_hitachi296.send();
            printf("Protocol Chosen Hitachi296\r\n");
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

void *LTE_task(void *args)
{
    LTE_gpio_configuration();
    resetLte();
    LTE_initialization();
    establishMQTTConnection();
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        if(network_flag && client_flag && subscribe_flag &&!restart_flag)
            ReadMessage(CLIENT_IDX);
        else
        {
            printf("NETWORK_FLAG : %d | CLIENT_FLAG : %d | SUBSCRIBE_FLAG : %d",network_flag, client_flag, subscribe_flag);
            LTE_part();
        }
        if(strlen(json_packet) > 20)
        {
            parse_json_packet();
            strcpy(json_packet, "");
        }
    }
}

void *recv_and_send_task(void *args)
{
    while(1)
    {
        vTaskDelay(10);
        if(!configured && !sending)
        {
            // Check if the IR code has been received.
            if (irrecv.decode(&results)) {
                // Display a crude timestamp.;
                uint32_t now = millis();
                Serial.printf(D_STR_TIMESTAMP " : %06lu.%03lu\n", now / 1000, now % 1000);
                // Check if we got an IR message that was to big for our capture buffer.
                if (results.overflow)
                Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
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
        else if(configured && needtosend && !sending)
        {
            printf("Protcol Chosen : %s\r\n", protocol_chosen);
            send_func();
        }
    }
}


void app_main(void)
{
    Serial.begin(kBaudRate);
    while(!Serial)
        delay(50);
    Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
    irrecv.setUnknownThreshold(kMinUnknownSize);
    irrecv.setTolerance(kTolerancePercentage);
    irrecv.enableIRIn();

    pthread_t recv_tid;
    pthread_t LTE_tid;

    ac_daikin216.begin();
    ac_daikin280.begin();
    ac_hitachi296.begin();

    if(pthread_create(&recv_tid, NULL, recv_and_send_task, NULL)!=0){
        perror("Error in creating recv_task : ");
    }
    if(pthread_create(&LTE_tid, NULL, LTE_task, NULL)!=0){
        perror("Error in creating LTE_task : ");
    }

}