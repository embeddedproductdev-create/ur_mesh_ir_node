#ifndef BLE_NEW_H
#define BLE_NEW_H

#define BLE_NODE_COMM_TIMEOUT_MS 20000

/*Global Variables*/

/*Provisioner Function declarations*/
void ble_init();
void ble_mesh_get_dev_uuid(uint8_t *dev_uuid);
esp_err_t bluetooth_init(void);

/*Node Function declarations*/
#ifdef __cplusplus
extern "C" {
#endif

void send_ack_to_provisioner(uint16_t packetid, void *ptr);

#ifdef __cplusplus
}
#endif
void provision_success_cb();
void unprovision_success_cb();
#endif