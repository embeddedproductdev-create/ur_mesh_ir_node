#define BLE_NODE_COMM_TIMEOUT_MS 20000

/*Global Variables*/
extern uint16_t prov_req_msgseqno;
extern uint16_t prov_req_elemaddr;

/*Function declarations*/
void ble_init();
void ble_mesh_get_dev_uuid(uint8_t *dev_uuid);
esp_err_t bluetooth_init(void);
esp_err_t send_provision_request(char *macid);
void handle_ble_outgoing(CommandStruct *cmd_struct);