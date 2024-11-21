#ifndef BLE_H
#define BLE_H

#include <lte.h>

#define BLE_NODE_COMM_TIMEOUT_MS 20000

/*Global Variables*/
extern uint16_t prov_success_elemAddr;
extern uint16_t prov_req_msgseqno;

/*Function declarations*/
void ble_init();
void ble_mesh_get_dev_uuid(uint8_t *dev_uuid);
esp_err_t bluetooth_init(void);
esp_err_t send_provision_request(char *macid);

void handle_ble_outgoing(CommandStruct *cmd_struct);
void handle_ble_incoming();

void provision_success_cb(uint16_t elemaddr);

#ifdef __cplusplus
extern "C" {
#endif
void send_ack_to_provisioner(uint16_t packetid, CommandStruct *cmd_struct);
#ifdef __cplusplus
}
#endif

#endif
