#ifndef ACCESS_POINT_H
#define ACCESS_POINT_H

#include "main.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>

#define AP_SSID "IR_BLE_Mesh_AP"
#define AP_PASSWORD "12345678"

/* GLOBAL VARIABLES */

/* FUNCTION DECLARATIONS */
void AP_task(void *args);

#endif
