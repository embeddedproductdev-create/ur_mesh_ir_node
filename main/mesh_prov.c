#include "mesh_main.h"

static void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                                             esp_ble_mesh_prov_cb_param_t *param)
{
	uint8_t ackUartTx[20] = {0};

    switch (event) {
    case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_ENABLE_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_PROV_ENABLE_COMP_EVT, err_code %d", param->provisioner_prov_enable_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_DISABLE_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_PROV_DISABLE_COMP_EVT, err_code %d", param->provisioner_prov_disable_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_RECV_UNPROV_ADV_PKT_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_RECV_UNPROV_ADV_PKT_EVT");
        recv_unprov_adv_pkt(param->provisioner_recv_unprov_adv_pkt.dev_uuid, param->provisioner_recv_unprov_adv_pkt.addr,
                            param->provisioner_recv_unprov_adv_pkt.addr_type, param->provisioner_recv_unprov_adv_pkt.oob_info,
                            param->provisioner_recv_unprov_adv_pkt.adv_type, param->provisioner_recv_unprov_adv_pkt.bearer);
        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_LINK_OPEN_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_PROV_LINK_OPEN_EVT, bearer %s",
            param->provisioner_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_LINK_CLOSE_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_PROV_LINK_CLOSE_EVT, bearer %s, reason 0x%02x",
            param->provisioner_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT", param->provisioner_prov_link_close.reason);
        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_COMPLETE_EVT:
        prov_complete(param->provisioner_prov_complete.node_idx, param->provisioner_prov_complete.device_uuid,
                      param->provisioner_prov_complete.unicast_addr, param->provisioner_prov_complete.element_num,
                      param->provisioner_prov_complete.netkey_idx);
        param->provisioner_prov_complete.unicast_addr += param->provisioner_prov_complete.element_num;
        param->provisioner_prov_complete.node_idx++;
        break;
    case ESP_BLE_MESH_PROVISIONER_ADD_UNPROV_DEV_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_ADD_UNPROV_DEV_COMP_EVT, err_code %d", param->provisioner_add_unprov_dev_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_SET_DEV_UUID_MATCH_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_SET_DEV_UUID_MATCH_COMP_EVT, err_code %d", param->provisioner_set_dev_uuid_match_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_SET_NODE_NAME_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_SET_NODE_NAME_COMP_EVT, err_code %d", param->provisioner_set_node_name_comp.err_code);
        if (param->provisioner_set_node_name_comp.err_code == 0) {
            const char *name = esp_ble_mesh_provisioner_get_node_name(param->provisioner_set_node_name_comp.node_index);
            if (name) {
                ESP_LOGI(TAG, "Node %d name %s", param->provisioner_set_node_name_comp.node_index, name);
            }
        }
        break;
    case ESP_BLE_MESH_PROVISIONER_ADD_LOCAL_APP_KEY_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_ADD_LOCAL_APP_KEY_COMP_EVT, err_code %d", param->provisioner_add_app_key_comp.err_code);
        if (param->provisioner_add_app_key_comp.err_code == 0) {
            prov_key.app_idx = param->provisioner_add_app_key_comp.app_idx;
            esp_err_t err = esp_ble_mesh_provisioner_bind_app_key_to_local_model(PROV_OWN_ADDR, prov_key.app_idx,
                                ESP_BLE_MESH_MODEL_ID_SENSOR_CLI, ESP_BLE_MESH_CID_NVAL);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to bind AppKey to sensor client");
            }
            err = esp_ble_mesh_provisioner_bind_app_key_to_local_model(PROV_OWN_ADDR, prov_key.app_idx,
                                ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_CLI, ESP_BLE_MESH_CID_NVAL);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to bind AppKey to sensor client");
            }
        }
        break;
    case ESP_BLE_MESH_PROVISIONER_BIND_APP_KEY_TO_MODEL_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_BIND_APP_KEY_TO_MODEL_COMP_EVT, err_code %d", param->provisioner_bind_app_key_to_model_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_STORE_NODE_COMP_DATA_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_STORE_NODE_COMP_DATA_COMP_EVT, err_code %d", param->provisioner_store_node_comp_data_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_DELETE_NODE_WITH_ADDR_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROVISIONER_DELETE_NODE_ADDR_COMP_EVT, DELETED NODE - 0x%04x, err_code %d", param->provisioner_delete_node_with_addr_comp.unicast_addr, param->provisioner_delete_node_with_addr_comp.err_code);
        setAckUartCommon(ackUartTx, param->provisioner_delete_node_with_addr_comp.unicast_addr, PROVDEL_ACK, ACK_SUCCESS, NA);
        uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
        break;
    case ESP_BLE_MESH_NODE_PROXY_GATT_ENABLE_COMP_EVT:
    	ESP_LOGI(TAG, "ESP_BLE_MESH_PROXY_GATT_ENABLE_COMP_EVT!!!");
    	break;
    case ESP_BLE_MESH_PROXY_CLIENT_CONNECTED_EVT:
    	proxy_t.connected = 1;
    	proxy_t.handle = param->proxy_client_connected.conn_handle;
    	ESP_LOGI(TAG, "ESP_BLE_MESH_PROXY_CLIENT_CONNECTED_EVT: %x:%x:%x:%x:%x:%x | Conn_handle - %d", param->proxy_client_connected.addr[0], param->proxy_client_connected.addr[1], param->proxy_client_connected.addr[2], \
    																				param->proxy_client_connected.addr[3], param->proxy_client_connected.addr[4], param->proxy_client_connected.addr[5], param->proxy_client_connected.conn_handle);
    	break;
    case ESP_BLE_MESH_PROXY_CLIENT_CONNECT_COMP_EVT:
    	ESP_LOGI(TAG, "ESP_BLE_MESH_PROXY_CLIENT_CONNECTED_EVT: %x:%x:%x:%x:%x:%x | Error_code - %d", param->proxy_client_connect_comp.addr[0], param->proxy_client_connect_comp.addr[1], param->proxy_client_connect_comp.addr[2], \
    																									param->proxy_client_connect_comp.addr[3], param->proxy_client_connect_comp.addr[4], param->proxy_client_connect_comp.addr[5], param->proxy_client_connect_comp.err_code);
    	proxy_t.connected = 1;
    	break;
    case ESP_BLE_MESH_PROXY_CLIENT_DISCONNECT_COMP_EVT:
    	ESP_LOGI(TAG, "ESP_BLE_MESH_PROXY_CLIENT_DISCONNECT_COMP_EVT!!! Conn_handle - %d | Err_code - %d", param->proxy_client_disconnect_comp.conn_handle, param->proxy_client_disconnect_comp.err_code);
    	break;
    case ESP_BLE_MESH_PROXY_CLIENT_DISCONNECTED_EVT:
    	ESP_LOGI(TAG, "ESP_BLE_MESH_PROXY_CLIENT_DISCONNECTED_EVT: %x:%x:%x:%x:%x:%x | Reason: %d | Conn_handle - %d", param->proxy_client_disconnected.addr[0], param->proxy_client_disconnected.addr[1], param->proxy_client_disconnected.addr[2], \
																															param->proxy_client_disconnected.addr[3], param->proxy_client_disconnected.addr[4], param->proxy_client_disconnected.addr[5], \
																																param->proxy_client_disconnected.reason, param->proxy_client_disconnected.conn_handle);
    	break;
    default:
        break;
    }
}