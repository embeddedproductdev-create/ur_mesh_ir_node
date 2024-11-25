#ifndef BLE_NEW_H
#define BLE_NEW_H

#include "esp_ble_mesh_defs.h"

#include <main.h>
#include <lte.h>

#define BLE_NODE_COMM_TIMEOUT_MS 20000

/*Global Variables*/

/*Function Declarations*/
void ble_init();
void ble_mesh_get_dev_uuid(uint8_t *dev_uuid);
esp_err_t bluetooth_init(void);
void send_cmd_to_node(CommandStruct *cmd);
void handle_cmds_from_provisioner(CommandStruct *cmd);
void provision_success_cb();
void unprovision_success_cb();
void attach_elemAddr_to_structures(uint16_t elemAddr);
void handle_ble_incoming(esp_ble_mesh_model_cb_param_t *param);
#ifdef __cplusplus
extern "C" {
#endif
void send_ack_to_provisioner(uint16_t packetid, CommandStruct *ack);
void send_teaching_mode_ack_to_provisioner();
void send_manual_control_ack_to_provisioner();
#ifdef __cplusplus
}
#endif

#endif