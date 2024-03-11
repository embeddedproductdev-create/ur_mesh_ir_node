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

mqtt_params_t mqtt_params;

const char ap_ssid[30] = "AP testing";
const char ap_password[30] = "12345678";

uint16_t GWY_SER_NO = 1;

/* IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(80);

const char WEBPAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Gateway MQTT configuration</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Times New Roman; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem; color: #FF0000;}
  </style>
  </head><body>
  <h2>Gateway MQTT configuration</h2>
  <form action="/get">
    MQTT ServerIP: <input type="text" name="serverip"><br><br>
    MQTT ServerPort: <input type="number" name="port"><br><br>
    MQTT ClientIDX: <input type="number" name="clientid"><br><br>
    MQTT ServerName: <input type="text" name="servername"><br><br>
    MQTT ServerPassword: <input type="text" name="password"><br><br>
    MQTT TabName: <input type="text" name="tabname"><br><br>
    <input type="submit" name="Submit">
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void AP_task(void *args)
{
    configASSERT(((uint32_t) args) == 1);
    nvs_flash_init();

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password, 6, 0, 5);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    server.begin();
    delay(100);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", WEBPAGE, NULL);
  });

    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String serverip;
    String serverport;
    String clientindex;
    String mqttservername;
    String mqttpassword;
    String mqtttabname;

    if (request->hasParam("serverip") &&
        request->hasParam("port") &&
        request->hasParam("clientid") &&
        request->hasParam("servername") &&
        request->hasParam("password") &&
        request->hasParam("tabname")) {
      serverip = request->getParam("serverip")->value();
      serverport = request->getParam("port")->value();
      clientindex = request->getParam("clientid")->value();
      mqttservername = request->getParam("servername")->value();
      mqttpassword = request->getParam("password")->value();
      mqtttabname = request->getParam("tabname")->value();
      printf("Server IP : %s\n",serverip.c_str());
      printf("Server Port : %s\n",serverport.c_str());
      printf("Client Index : %s\n",clientindex.c_str());
      printf("Broker Name : %s\n",mqttservername.c_str());
      printf("Broker password : %s\n",mqttpassword.c_str());
      printf("Tab Name : %s\n",mqtttabname.c_str());
      mqtt_params_fetched_flag = true;
    }
    else
    {
      request->send_P(404, "text/html", "Not Enough Parameters.. Redirecting to Homepage in 5 seconds", NULL);
      delay(3);
      request->send_P(200, "text/html", WEBPAGE, NULL);
      mqtt_params_fetched_flag = false;
    }

    if(mqtt_params_fetched_flag)
      request->send(200, "text/html", "Parameters fetched successfully");
  });

    while(1 && !mqtt_params_fetched_flag)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
}

