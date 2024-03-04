#ifndef MESH_MAIN_H
#define MESH_MAIN_H

#include "../Custom/main.h"

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_sensor_model_api.h"
#include "ble_mesh_example_init.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* FUNCTION DECLARATIONS */
void mesh_main_init(void);

#ifdef __cplusplus
}
#endif

#endif
