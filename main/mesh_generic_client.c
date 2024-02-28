#include "mesh_main.h"

static void example_ble_mesh_generic_client_cb(esp_ble_mesh_generic_client_cb_event_t event,
                                               esp_ble_mesh_generic_client_cb_param_t *param)
{
    ESP_LOGI(TAG, "Generic client, event %u, error code %d, opcode is 0x%04x",
        event, param->error_code, param->params->opcode);

    uint8_t ackUartTx[20] = {0};

    switch (event) {
    case ESP_BLE_MESH_GENERIC_CLIENT_GET_STATE_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_CLIENT_GET_STATE_EVT");
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET) {
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET, onoff %d", param->status_cb.onoff_status.present_onoff);
        }
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS) {
        	ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, onoff %d", param->status_cb.onoff_status.present_onoff);
        }
        break;
    case ESP_BLE_MESH_GENERIC_CLIENT_SET_STATE_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_CLIENT_SET_STATE_EVT");
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET) {
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET, onoff %d, Address: %04x", param->status_cb.onoff_status.present_onoff, param->params->ctx.addr);
            setAckUartCommon(ackUartTx, param->params->ctx.addr, RELCTRL_ACK, ACK_SUCCESS, param->status_cb.onoff_status.present_onoff);
            uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
        }
        break;
    case ESP_BLE_MESH_GENERIC_CLIENT_PUBLISH_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_CLIENT_PUBLISH_EVT");
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS) {
        	ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS, onoff %d, src_addr - 0x%04x", param->status_cb.onoff_status.present_onoff, param->params->ctx.addr);
        }
//        uint8_t uart_tx[20] = {0};
//        setCommonUartTxData(uart_tx, param->params->ctx.addr);
//        uart_tx[1] = RELCTRL_ACK;
//        uart_tx[4] = param->status_cb.onoff_status.present_onoff;
//        uart_write_bytes(UART_NUM_2, (const char*)uart_tx, 20);
        break;
    case ESP_BLE_MESH_GENERIC_CLIENT_TIMEOUT_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_CLIENT_TIMEOUT_EVT");
        setAckUartCommon(ackUartTx, param->params->ctx.addr, RELCTRL_ACK, ACK_FAIL, NA);
        uart_write_bytes(UART_NUM_2, (const char*)ackUartTx, 20);
        break;

    default:
        break;
    }
}