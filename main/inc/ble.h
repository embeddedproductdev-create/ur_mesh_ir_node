#define BLE_NODE_COMM_TIMEOUT_MS 8000

void ble_init();
void ble_mesh_get_dev_uuid(uint8_t *dev_uuid);
esp_err_t bluetooth_init(void);
esp_err_t send_provision_request(char *macid);
void handle_sending_out_node_packets(CommandStruct *cmd_struct);