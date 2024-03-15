#ifndef MESH_MAIN_H
#define MESH_MAIN_H

#include "../Custom/main.h"

/*
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_sensor_model_api.h"
#include "ble_mesh_example_init.h"
*/
//#include "ble_mesh_example_nvs.h"


#ifdef __cplusplus
extern "C"
{
#endif

/* FUNCTION DECLARATIONS */
void mesh_main_init(void);
//void example_ble_mesh_send_sensor_status();
//extern esp_ble_mesh_sensor_state_t sensor_states[2];
/* sending data through sensor server model*/
void *send_data_task(void *args);

#ifdef __cplusplus
}
#endif

#endif
