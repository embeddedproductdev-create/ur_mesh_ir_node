/*
 * SPDX-FileCopyrightText: 2017 Intel Corporation
 * SPDX-FileContributor: 2018-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <main.h>

#if(!IS_GWY)

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"

#include <ble_new.h>
#include <flash.h>
#include <led.h>
#include <lte.h>
#include <heartbeat.h>

#define BLE_TAG "BLE"

#define CID_ESP     0x02E5

#define ESP_BLE_MESH_VND_MODEL_ID_CLIENT    0x0000
#define ESP_BLE_MESH_VND_MODEL_ID_SERVER    0x0001

#define ESP_BLE_MESH_VND_MODEL_OP_SEND      ESP_BLE_MESH_MODEL_OP_3(0x00, CID_ESP)
#define ESP_BLE_MESH_VND_MODEL_OP_STATUS    ESP_BLE_MESH_MODEL_OP_3(0x01, CID_ESP)

static uint8_t dev_uuid[ESP_BLE_MESH_OCTET16_LEN] = { 0x32, 0x10 };

static esp_ble_mesh_cfg_srv_t config_server = {
    .relay = ESP_BLE_MESH_RELAY_DISABLED,
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
#if defined(CONFIG_BLE_MESH_FRIEND)
    .friend_state = ESP_BLE_MESH_FRIEND_ENABLED,
#else
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
#endif
#if defined(CONFIG_BLE_MESH_GATT_PROXY_SERVER)
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
#else
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif
    .default_ttl = 7,
    /* 3 transmissions with 20ms interval */
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
};

static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
};

// static const esp_ble_mesh_client_op_pair_t vnd_op_pair[] = {
//     {ESP_BLE_MESH_VND_MODEL_OP_SEND, ESP_BLE_MESH_VND_MODEL_OP_STATUS},
// };

// static esp_ble_mesh_client_t vendor_client = {
//     .op_pair_size = ARRAY_SIZE(vnd_op_pair),
//     .op_pair = vnd_op_pair,
// };

static esp_ble_mesh_model_op_t vnd_op[] = {
    ESP_BLE_MESH_MODEL_OP(ESP_BLE_MESH_VND_MODEL_OP_SEND, 2),
    ESP_BLE_MESH_MODEL_OP_END,
};

static esp_ble_mesh_model_t vnd_models[] = {
    ESP_BLE_MESH_VENDOR_MODEL(CID_ESP, ESP_BLE_MESH_VND_MODEL_ID_SERVER,
    vnd_op, NULL, NULL),
};

// static esp_ble_mesh_model_t vnd_models[] = {
//     ESP_BLE_MESH_VENDOR_MODEL(CID_ESP, ESP_BLE_MESH_VND_MODEL_ID_CLIENT,
//                               vnd_op, NULL, &vendor_client),
// };

static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, root_models, vnd_models),
};

static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

static esp_ble_mesh_prov_t provision = {
    .uuid = dev_uuid,
};

static void prov_complete(uint16_t net_idx, uint16_t addr, uint8_t flags, uint32_t iv_index)
{
    ESP_LOGI(BLE_TAG, "net_idx 0x%03x, addr 0x%04x", net_idx, addr);
    ESP_LOGI(BLE_TAG, "flags 0x%02x, iv_index 0x%08" PRIx32, flags, iv_index);
    provision_success_cb(addr);
}

static void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                                             esp_ble_mesh_prov_cb_param_t *param)
{
    switch (event) {
    case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
        ESP_LOGI(BLE_TAG, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
        ESP_LOGI(BLE_TAG, "ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT, err_code %d", param->node_prov_enable_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:
        ESP_LOGI(BLE_TAG, "ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT, bearer %s",
            param->node_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:
        ESP_LOGI(BLE_TAG, "ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT, bearer %s",
            param->node_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
        ESP_LOGI(BLE_TAG, "ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT");
        prov_complete(param->node_prov_complete.net_idx, param->node_prov_complete.addr,
            param->node_prov_complete.flags, param->node_prov_complete.iv_index);
        break;
    case ESP_BLE_MESH_NODE_PROV_RESET_EVT:
        ESP_LOGI(BLE_TAG, "ESP_BLE_MESH_NODE_PROV_RESET_EVT");
        break;
    case ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT:
        ESP_LOGI(BLE_TAG, "ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT, err_code %d", param->node_set_unprov_dev_name_comp.err_code);
        break;
    default:
        break;
    }
}

static void example_ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
                                              esp_ble_mesh_cfg_server_cb_param_t *param)
{
    if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT) {
        switch (param->ctx.recv_op) {
        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
            ESP_LOGI(BLE_TAG, "ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD");
            ESP_LOGI(BLE_TAG, "net_idx 0x%04x, app_idx 0x%04x",
                param->value.state_change.appkey_add.net_idx,
                param->value.state_change.appkey_add.app_idx);
            ESP_LOG_BUFFER_HEX("AppKey", param->value.state_change.appkey_add.app_key, 16);
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
            ESP_LOGI(BLE_TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND");
            ESP_LOGI(BLE_TAG, "elem_addr 0x%04x, app_idx 0x%04x, cid 0x%04x, mod_id 0x%04x",
                param->value.state_change.mod_app_bind.element_addr,
                param->value.state_change.mod_app_bind.app_idx,
                param->value.state_change.mod_app_bind.company_id,
                param->value.state_change.mod_app_bind.model_id);
            break;
        default:
            break;
        }
    }
}

static void example_ble_mesh_custom_model_cb(esp_ble_mesh_model_cb_event_t event,
                                             esp_ble_mesh_model_cb_param_t *param)
{
    switch (event) {
    case ESP_BLE_MESH_MODEL_OPERATION_EVT:
        if (param->model_operation.opcode == ESP_BLE_MESH_VND_MODEL_OP_SEND) {
            uint16_t tid = *(uint16_t *)param->model_operation.msg;
            ESP_LOGI(BLE_TAG, "Recv 0x%06" PRIx32 ", tid 0x%04x", param->model_operation.opcode, tid);
            handle_cmds_from_provisioner((CommandStruct *)param->model_operation.msg);
            esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0],
                    param->model_operation.ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS,
                    sizeof(tid), (uint8_t *)&tid);
            if (err) {
                ESP_LOGE(BLE_TAG, "Failed to send message 0x%06x", ESP_BLE_MESH_VND_MODEL_OP_STATUS);
            }
        }
        break;
    case ESP_BLE_MESH_MODEL_SEND_COMP_EVT:
        if (param->model_send_comp.err_code) {
            ESP_LOGE(BLE_TAG, "Failed to send message 0x%06" PRIx32, param->model_send_comp.opcode);
            break;
        }
        ESP_LOGI(BLE_TAG, "Send 0x%06" PRIx32, param->model_send_comp.opcode);
        break;
    
    case ESP_BLE_MESH_CLIENT_MODEL_RECV_PUBLISH_MSG_EVT:
        ESP_LOGI(BLE_TAG, "Receive publish message 0x%06" PRIx32, param->client_recv_publish_msg.opcode);
        break;

    default:
        break;
    }
}

static esp_err_t ble_mesh_init(void)
{
    esp_err_t err;

    esp_ble_mesh_register_prov_callback(example_ble_mesh_provisioning_cb);
    esp_ble_mesh_register_config_server_callback(example_ble_mesh_config_server_cb);
    esp_ble_mesh_register_custom_model_callback(example_ble_mesh_custom_model_cb);

    err = esp_ble_mesh_init(&provision, &composition);
    if (err != ESP_OK) {
        ESP_LOGE(BLE_TAG, "Failed to initialize mesh stack");
        return err;
    }

    err = esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
    if (err != ESP_OK) {
        ESP_LOGE(BLE_TAG, "Failed to enable mesh node");
        return err;
    }

    ESP_LOGI(BLE_TAG, "BLE Mesh Node initialized");

    return ESP_OK;
}

void ble_init(void)
{
    esp_err_t err;

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    err = bluetooth_init();
    if (err) {
        ESP_LOGE(BLE_TAG, "esp32_bluetooth_init failed (err %d)", err);
        return;
    }

    memcpy(dev_uuid + 2, esp_bt_dev_get_address(), BD_ADDR_LEN);

    /* Initialize the Bluetooth Mesh Subsystem */
    err = ble_mesh_init();
    if (err) {
        ESP_LOGE(BLE_TAG, "Bluetooth mesh init failed (err %d)", err);
    }
}

/**
 * =============================================================================
 * FUNCTIONS BELOW THIS SPACE ARE CUSTOM MADE AS PER REQUIREMENTS OF THE PROJECT
 * =============================================================================
 */

/**
 * @brief Function that sends acks to provisioner
 * @param packetid Type of ack
 * @param ptr Pointer to the ack contents
 */
void send_ack_to_provisioner(uint16_t packetid, CommandStruct *ack)
{
    esp_ble_mesh_msg_ctx_t ctx = {
        .addr = 0X0001,
        .app_idx = 0X0000,
        .net_idx = 0,
        .send_ttl = 3,
        .send_rel = false,
    };
    switch(packetid)
    {
        case NODE_UNPROV_PACKET:
            ESP_LOGI(BLE_TAG, "Sending Node Unprov ACK to Provisioner");
            break;

        case NODE_HEARTBEAT_ACK:
            ESP_LOGI(BLE_TAG, "Sending HB ACK to Provisioner");
            esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS, sizeof(CommandStruct), (uint8_t *)&last_command);
            if(err) ESP_LOGE(BLE_TAG, "Failed to ACK : %s", esp_err_to_name(err));
            break;

        default:
            ESP_LOGE(BLE_TAG, "Unknown ACK type : %d", packetid);
            return;
    }
    
}

/**
 * @brief Function that handles cmds from provisioner
 */
void handle_cmds_from_provisioner(CommandStruct *cmd)
{
    led_set_state(LED_STATE_CMD_RECVD);
    switch(cmd->packetid)
    {
        case NODE_AC_CONTROL_PACKET:
            ESP_LOGI(BLE_TAG, "Received Node AC Control Packet from Provisioner");
            break;

        default:
            ESP_LOGE(BLE_TAG, "Received Unknown CMD from Provisioner : %d", cmd->packetid);
            break;
    }
}

/**
 * @brief Function that takes care of things to be done after provisioning
 * is successful
 * 
 */
void provision_success_cb(uint16_t elemAddr)
{
    provisioned = true;
    last_command.packetid = NODE_HEARTBEAT_ACK;
    last_command.elemaddr = elemAddr;
    set_number_in_nvs_flash(GENERAL_HANDLE, NVS_PROVISIONED_KEY, 1, UINT8_SIZE);
    update_led_status();
    hb_timer_restart();
}

/**
 * @brief Function that takes care of things to be done after
 * unprovisioning is successful
 * 
 */
void unprovision_success_cb()
{
    esp_ble_mesh_node_local_reset();
    esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
    provisioned = false;
    set_number_in_nvs_flash(GENERAL_HANDLE, NVS_PROVISIONED_KEY, 0, UINT8_SIZE);
    update_led_status();
}

#endif