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
void send_prov_packet_to_node(prov_t *prov_packet);
void send_unprov_packet_to_node(unprov_t *unprov_packet);
void send_reconf_packet_to_node(reconf_t *reconf_packet);
void send_ac_control_packet_to_node(control_t *control_packet);
void send_pub_conf_packet_to_node(pub_conf_t *pub_conf_packet);

#ifdef __cplusplus
}
#endif

#endif
