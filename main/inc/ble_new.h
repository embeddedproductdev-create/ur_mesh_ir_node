#ifndef BLE_NEW_H
#define BLE_NEW_H

#include <main.h>
#include <lte.h>

#define BLE_NODE_COMM_TIMEOUT_MS 20000

/*Global Variables*/

/*Provisioner Function declarations*/
void ble_init();
void ble_mesh_get_dev_uuid(uint8_t *dev_uuid);
esp_err_t bluetooth_init(void);
void send_prov_ack_to_cloud(uint16_t elemaddr, char *node_name);

/*Node Function declarations*/
#ifdef __cplusplus
extern "C" {
#endif
void send_cmd_to_node(CommandStruct *cmd); //Since this is in lte.c and is common, we just have to keep it here

#if(IS_GWY)
void handle_ble_incoming(CommandStruct *ack);
#else
void send_ack_to_provisioner(uint16_t packetid, CommandStruct *ack);
void handle_cmds_from_provisioner(CommandStruct *cmd);
#endif

#ifdef __cplusplus
}
#endif
void provision_success_cb(uint16_t elemAddr);
void unprovision_success_cb();
#endif