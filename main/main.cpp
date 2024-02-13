#include "Arduino.h"
#include "main.h"
#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include "mesh_main.h"


bool configured = false;
bool sending = false;
bool needtosend = false;
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
    void mesh_init(void);
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
            if(ac_control_t.swingH) ac_control_t.swingH = kDaikinSwingOn;
            ac_daikin216.setSwingHorizontal(ac_control_t.swingH);
            if(ac_control_t.swingV) ac_control_t.swingV = kDaikinSwingOn;
            ac_daikin216.setSwingVertical(ac_control_t.swingV);
            ac_daikin216.setFan(ac_control_t.fan);
            ac_daikin216.send();
            printf("Sending Daikin216\r\n");
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
        // Check if the IR code has been received.
        if (irrecv.decode(&results) && !sending) {
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
        // printf("configured : %d | needtosend : %d | sending : %d\r\n",configured, needtosend, sending);
        if(configured && needtosend && !sending)
        {
            send_func();
            printf("Protcol Chosen : %s\r\n", protocol_chosen);
        }
    }
}

void send_prov_packet()
{
    bool flag = false;
    while(!flag)
    {
        for(int8_t i=10; i>0; i--)
        {
            printf("provisioning in %d seconds ...\r\n",i);
            delay(1000);
        }
        flag = true;
    }
    provision_t.gwy_ser_no = 1;
    provision_t.macid[0] = 0x78;
    provision_t.macid[1] = 0x21;
    provision_t.macid[2] = 0x84;
    provision_t.macid[3] = 0xb8;
    provision_t.macid[4] = 0xf0;
    provision_t.macid[5] = 0x56;
    provision_t.msg_seq_no = 1;
    provision_t.node_ser_no = 1;
    strcpy(provision_t.timestamp, "abcd");
    handle_cloud_packets(NODE_PROV_PACKET);
}

void send_unprov_packet()
{
    bool flag = false;
    while(!flag)
    {
        for(int8_t i=20; i>0; i--)
        {
            printf("Unprovisioning in %d seconds ...\r\n",i);
            delay(1000);
        }
        flag = true;
    }
    unprovision_t.gwy_ser_no = 1;
    unprovision_t.elemnt_addr = 0x0005;
    unprovision_t.msg_seq_no = 1;
    unprovision_t.node_ser_no = 1;
    strcpy(unprovision_t.timestamp, "abcd");
    handle_cloud_packets(NODE_UNPROV_PACKET);
}

void app_main(void)
{
    mesh_init();

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

    // if(pthread_create(&recv_tid, NULL, recv_and_send_task, NULL)!=0){
    //     perror("Error in creating recv_task : ");
    // }
    // if(pthread_create(&LTE_tid, NULL, LTE_task, NULL)!=0){
    //     perror("Error in creating LTE_task : ");
    // }
    send_prov_packet();
    send_unprov_packet();
}