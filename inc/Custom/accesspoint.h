#ifndef ACCESS_POINT_H
#define ACCESS_POINT_H

#include "main.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>

typedef struct mqtt_params{
    char mqtt_server_ip[10];
    char mqtt_server_port[5];
    char mqtt_client_index[2];
    char mqtt_server_name[15];
    char mqtt_server_pwd[20];
    char mqtt_tab_name[20];
}mqtt_params_t;

/* GLOBAL VARIABLES */
extern const char ap_ssid[30];
extern const char ap_password[30];
extern mqtt_params_t mqtt_params;

/* FUNCTION DECLARATIONS */
void AP_task(void *args);

#endif
