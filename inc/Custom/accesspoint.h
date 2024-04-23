#ifndef ACCESS_POINT_H
#define ACCESS_POINT_H

#include "main.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>

#define AP_SSID "IR_BLE_Mesh_AP"
#define AP_PASSWORD "12345678"

#define QMAX_IP "54.215.188.103"
#define QMAX_MQTT_PORT 1883
#define QMAX_BROKER_NAME "QmaxSystems"
#define QMAX_BROKER_PASSWORD "Qmax_mosquitto_!@#"

#define UNIMATION_IP "3.7.8.183"
#define UNIMATION_MQTT_PORT 1883
#define UNIMATION_BROKER_NAME "unimaqtt"
#define UNIMATION_BROKER_PASSWORD "T5DRIIJEBgfhjsrFkaDERkgJhswMwk4"

/* GLOBAL VARIABLES */

/* FUNCTION DECLARATIONS */
void AP_task(void *args);

#endif
