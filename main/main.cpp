#include "Arduino.h"
#include "main.h"
#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

bool configured = false;

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;

char json_packet[100];

extern "C"
{
    void app_main(void);
    void LTE_gpio_configuration();
    void ConnectToNetwork();
    void subscribe();
    void resetLte();
    void sendAT_Data(const char* data);
    void LTE_initialization(void);
    uint8_t check_response(char* response, uint32_t timeout);
    uint8_t MQTT_Config(uint8_t client_idx,
            uint8_t enable_ssl, uint8_t SSL_ctx_idx,
            uint16_t keep_alive,
            uint8_t clean_session,
            uint8_t msg_recv_mode,uint8_t msg_len_enable,
    uint8_t will_fg, uint8_t will_qos, uint8_t will_retain, char* will_topic, char* will_message);
    uint8_t SSL_config(uint8_t ssl_context_index, char* ca_cert, char* client_cert, char* client_key);
    uint8_t SubscribeTopic(int client_idx, int msgid, char* topic, int qos);
    uint8_t UnsubscribeTopic(int client_idx, int msgid, char* topic);
    int MQTT_NetworkOpen(int client_idx, char* hostname, uint32_t port);
    uint8_t MQTT_NetworkClose(int client_idx);
    uint8_t MQTT_ClientConnect(int client_idx, char* username, char* passwd, char* clientID);
    uint8_t MQTT_ClientDisconnect(int client_idx);
    uint8_t PublishMessage(uint8_t client_idx, uint32_t msgid, uint8_t qos, uint8_t retain, char* topic);
    uint8_t ReadMessage(int client_idx);
    uint8_t Error_Report();
}

void *recv_task(void *args)
{
    while(1)
    {
        vTaskDelay(1);
        // Check if the IR code has been received.
        if (irrecv.decode(&results) && !configured) {
            // Display a crude timestamp.
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
            Serial.print(resultToHumanReadableBasic(&results));
            // Display any extra A/C info if we have it.
            String description = IRAcUtils::resultAcToString(&results);
            if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
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

void send_task()
{
    if(configured)
    {
        ;
    }
}

void *LTE_task(void *args)
{
    LTE_gpio_configuration();
    resetLte();
    LTE_initialization();
    ConnectToNetwork();
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        ReadMessage(CLIENT_IDX);
        if(strlen(json_packet) > 20)
        {
            Serial.println(json_packet);
            strcpy(json_packet, "");
            // memset(json_packet,'', sizeof(json_packet));
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

    if(pthread_create(&recv_tid, NULL, recv_task, NULL)!=0){
        perror("Error in creating recv_task : ");
    }
    if(pthread_create(&LTE_tid, NULL, LTE_task, NULL)!=0){
        perror("Error in creating LTE_task : ");
    }
}
