
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef MESH_MAIN_H
#define MESH_MAIN_H

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "nvs_flash.h"

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"
#include "esp_ble_mesh_sensor_model_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"
#include "esp_ble_mesh_proxy_api.h"

#include "ble_mesh_example_init.h"
#include "mqtt.h"
#include "board.h"

#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_intr_alloc.h"
#include "soc/uart_struct.h"
#include "soc/uart_reg.h"
#include "esp_err.h"

#define GPIO_RESET		GPIO_NUM_26
#define MSK_GPIO_RESET	(1ULL << GPIO_RESET)

#define CID_ESP             0x02E5

#define PROV_OWN_ADDR       0x0001

#define MSG_SEND_TTL        3
#define MSG_SEND_REL        false
#define MSG_TIMEOUT         0
#define MSG_ROLE            ROLE_PROVISIONER

#define COMP_DATA_PAGE_0    0x00

#define APP_KEY_IDX         0x0000
#define APP_KEY_OCTET       0x12

#define COMP_DATA_1_OCTET(msg, offset)      (msg[offset])
#define COMP_DATA_2_OCTET(msg, offset)      (msg[offset + 1] << 8 | msg[offset])

/* STRUCUTRE DECLARATIONS */
typedef struct {
	uint8_t resetCount;
	uint8_t resetPressed;
	uint8_t resetTimerStarted;
	uint8_t urlReset;
	uint8_t urlSet;
	uint8_t gwyDbReset;
	uint8_t gwySet;
}prov;

// static struct esp_ble_mesh_key {
//     uint16_t net_idx;
//     uint16_t app_idx;
//     uint8_t  app_key[ESP_BLE_MESH_OCTET16_LEN];
// } prov_key;

/* FUNCTION DECLARATIONS */
void handle_cloud_packets(enum json_packet_enum json_packet_id);
// static void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                                            //  esp_ble_mesh_prov_cb_param_t *param);
// void example_ble_mesh_config_client_cb(esp_ble_mesh_cfg_client_cb_event_t event,
//                                               esp_ble_mesh_cfg_client_cb_param_t *param);
// static void example_ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
//                                               esp_ble_mesh_cfg_server_cb_param_t *param);
// static void example_ble_mesh_sensor_client_cb(esp_ble_mesh_sensor_client_cb_event_t event,
//                                               esp_ble_mesh_sensor_client_cb_param_t *param);
// static void example_ble_mesh_generic_client_cb(esp_ble_mesh_generic_client_cb_event_t event,
//                                                esp_ble_mesh_generic_client_cb_param_t *param);
#endif

#ifdef __cplusplus
}
#endif