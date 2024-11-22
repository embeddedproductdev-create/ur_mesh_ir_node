#ifndef BLE_H
#define BLE_H

#include <lte.h>

#define BLE_NODE_COMM_TIMEOUT_MS 20000

/*Global Variables*/
extern uint16_t prov_success_elemAddr;
extern uint16_t prov_req_msgseqno;
extern uint16_t unprov_success_elemAddr;
extern uint16_t unprov_req_msgseqno;

/*Function declarations*/
void ble_init();
void ble_mesh_get_dev_uuid(uint8_t *dev_uuid);
esp_err_t bluetooth_init(void);
void send_provision_request(char *macid, CommandStruct *cmd_struct);

void handle_ble_outgoing(CommandStruct *cmd_struct);
void handle_ble_incoming();

#if(IS_GWY)
void provision_success_cb(uint16_t elemaddr);
void reset_prov_match();
#endif
#if(!IS_GWY)
void handle_cmds_from_provisioner(CommandStruct *cmd_struct);
void provision_success_cb();
void unprovision_success_cb();
#endif

#ifdef __cplusplus
extern "C" {
#endif
void send_ack_to_provisioner(uint16_t packetid, CommandStruct *cmd_struct);
#ifdef __cplusplus
}
#endif

#endif
