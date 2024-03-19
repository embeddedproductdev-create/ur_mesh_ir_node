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

// Initialization
#if(AP_PART_ENABLED)
char mqtt_ip_address[16];
uint16_t mqtt_port;
uint8_t mqtt_client_index;
char mqtt_broker_username[30];
char mqtt_broker_password[30];
char mqtt_broker_tabname[30];
bool mqtt_params_fetched_flag = false;
#endif

#if(!AP_PART_ENABLED)
char mqtt_ip_address[16] = "54.215.188.103";
uint16_t mqtt_port = 1883;
uint8_t mqtt_client_index = 3;
char mqtt_broker_username[30] = "QmaxSystems";
char mqtt_broker_password[30] = "Qmax_mosquitto_!@#";
char mqtt_broker_tabname[30] = "AC_IR_CONTROL";
bool mqtt_params_fetched_flag = true;
#endif

uint16_t GWY_SER_NO = 1;

/* IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

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

const char END_AP_WEBPAGE[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html><head>
  <title>Gateway MQTT configuration</title>
  <h1>Gateway MQTT configuration successful</h1>
  <h3>Turning off MQTT configuration mode<h3>
</body></html>
)rawliteral";

/**
 * @brief Handler for NotFound case
 * @param request
 * @retval none
 */
void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

/**
 * @brief Thread that runs the AP mode for Gateway MQTT configurations
 * @param args
 * @retval none
 */
void AP_task(void *args)
{
  configASSERT(((uint32_t)args) == 1);
  ESP_LOGI(DEBUG_TAG, "Restarted AP_Task\n");
  nvs_flash_init();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD, 6, 0, 5);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  server.begin();
  delay(100);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", WEBPAGE, NULL); });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("serverip") &&
        request->hasParam("port") &&
        request->hasParam("clientid") &&
        request->hasParam("servername") &&
        request->hasParam("password") &&
        request->hasParam("tabname")) {
      strcpy(mqtt_ip_address, request->getParam("serverip")->value().c_str());
      mqtt_port = atoi(request->getParam("port")->value().c_str());
      mqtt_client_index = atoi(request->getParam("clientid")->value().c_str());
      strcpy(mqtt_broker_username, request->getParam("servername")->value().c_str());
      strcpy(mqtt_broker_password, request->getParam("password")->value().c_str());
      strcpy(mqtt_broker_tabname, request->getParam("tabname")->value().c_str());
      request->send(200, "text/html", END_AP_WEBPAGE);
      vTaskDelay(pdMS_TO_TICKS(500)); //Just maybe the website needs to load... We'll remove this later
      mqtt_params_fetched_flag = true;
      WiFi.mode(WIFI_MODE_STA);
    }
    else
    {
      request->send_P(404, "text/html", "Not Enough Parameters.. Redirecting to Homepage in 3 seconds", NULL);
      delay(3);
      request->send_P(200, "text/html", WEBPAGE, NULL);
      mqtt_params_fetched_flag = false;
    } });

  while (1 && !mqtt_params_fetched_flag)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  vTaskDelete(NULL);
}
