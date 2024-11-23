#ifndef BLE_NEW_H
#define BLE_NEW_H

#include <main.h>
#include <lte.h>

#define BLE_NODE_COMM_TIMEOUT_MS 20000

/*Global Variables*/

/*Function Declarations*/
void ble_init();
void ble_mesh_get_dev_uuid(uint8_t *dev_uuid);
esp_err_t bluetooth_init(void);
void send_prov_ack_to_cloud(uint16_t elemaddr, char *node_name);
void send_cmd_to_node(CommandStruct *cmd);
void handle_ble_incoming(CommandStruct *ack);
void handle_cmds_from_provisioner(CommandStruct *cmd);
void provision_success_cb(uint16_t elemAddr);
void unprovision_success_cb();

#ifdef __cplusplus
extern "C" {
#endif
void send_ack_to_provisioner(uint16_t packetid, CommandStruct *ack);
void send_teaching_mode_ack_to_provisioner(teaching_mode *data);
void send_manual_control_ack_to_provisioner(manual_control *data);
#ifdef __cplusplus
}
#endif

#endif