/**
 * @file accesspoint.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to the Access point hosting for MQTT parameter initialization
 * @version 0.1
 * @date 2024-03-07
 * @ref https://www.upesy.com/blogs/tutorials/how-create-a-wifi-acces-point-with-esp32
 * @ref https://randomnerdtutorials.com/esp32-access-point-ap-web-Server/
 * @copyright Copyright (c) 2024
 */

#include "../../inc/Custom/accesspoint.h"

//Initialization
char mqtt_ip_address[16] = "54.215.188.103";
uint16_t mqtt_port = 1883;
uint8_t mqtt_client_index = 2;
char mqtt_username[30] = "QmaxSystems";
char mqtt_password[30] = "Qmax_mosquitto_!@#";
char mqtt_tab_name[30] = "AC_IR_CONTROL";
bool mqtt_params_fetched_flag = false;

const char ap_ssid[30] = "AP testing";
const char ap_password[30] = "12345678";

uint16_t GWY_SER_NO = 1;

/* IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

// HTML & CSS contents which display on web server
String WEBPAGE = "<!DOCTYPE html>\
<html>\
<body>\
<h1>My First Web Server with ESP32 - AP Mode &#128522;</h1>\
</body>\
</html>";

void AP_task(void *args)
{
    configASSERT(((uint32_t) args) == 1);
    // Create SoftAP
    WiFi.softAP(ap_ssid, ap_password);
    WiFi.softAPConfig(local_ip, gateway, subnet);

    server.begin();
    delay(100);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        printf("Inside AP Task\n");
    }
}

