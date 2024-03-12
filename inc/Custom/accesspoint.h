#ifndef ACCESS_POINT_H
#define ACCESS_POINT_H

#include "main.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>

/* GLOBAL VARIABLES */
extern const char ap_ssid[30];
extern const char ap_password[30];

/* FUNCTION DECLARATIONS */
void AP_task(void *args);
void create_AP_task();
#endif
