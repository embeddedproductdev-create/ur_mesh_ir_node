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
#include <ir.h>
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
    attach_elemAddr_to_structures(addr);
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
                provision_success_cb();
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
            // esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0],
            //         param->model_operation.ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS,
            //         sizeof(tid), (uint8_t *)&tid);
            // if (err) {
            //     ESP_LOGE(BLE_TAG, "Failed to send message 0x%06x", ESP_BLE_MESH_VND_MODEL_OP_STATUS);
            // }
        }
        break;
    case ESP_BLE_MESH_MODEL_SEND_COMP_EVT:
        if (param->model_send_comp.err_code) {
            ESP_LOGE(BLE_TAG, "Failed to send message 0x%06" PRIx32, param->model_send_comp.opcode);
            break;
        }
        // ESP_LOGI(BLE_TAG, "Send 0x%06" PRIx32, param->model_send_comp.opcode);
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
 * @brief Function that sends the Teaching mode ACK to provisioner
 */
void send_teaching_mode_ack_to_provisioner()
{
    esp_ble_mesh_msg_ctx_t ctx = {
        .addr = 0X0001,
        .app_idx = 0X0000,
        .net_idx = 0,
        .send_ttl = 3,
        .send_rel = false,
    };
    teaching_mode_t.packetid = NODE_TEACHING_MODE;
    strcpy(teaching_mode_t.deviceName, serialNoStr);
    teaching_mode_t.bleErrorCode = SUCCESS;
    ESP_LOGI(BLE_TAG, "Sending Node Teaching Mode ACK to Provisioner");
    esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS, sizeof(teaching_mode), (uint8_t *)&teaching_mode_t);
    if(err) ESP_LOGE(BLE_TAG, "Failed to ACK : %s", esp_err_to_name(err));
}

/**
 * @brief Function that sends the AC Manual Control ACK to provisioner
 */
void send_manual_control_ack_to_provisioner()
{
    esp_ble_mesh_msg_ctx_t ctx = {
        .addr = 0X0001,
        .app_idx = 0X0000,
        .net_idx = 0,
        .send_ttl = 3,
        .send_rel = false,
    };
    strcpy(ac_manual_control_t.deviceName, serialNoStr);
    ac_manual_control_t.packetid = NODE_MANUAL_AC_CONTROL_ACK;
    ESP_LOGI(BLE_TAG, "Sending Node Manual AC Control ACK to Provisioner");
    esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS, sizeof(manual_control), (uint8_t *)&ac_manual_control_t);
    if(err) ESP_LOGE(BLE_TAG, "Failed to ACK : %s", esp_err_to_name(err));
}

/**
 * @brief Function that sends acks to provisioner
 * @param packetid Type of ack
 * @param ptr Pointer to the ack contents
 */
void send_ack_to_provisioner(uint16_t packetid, CommandStruct *ack)
{
    esp_err_t err;
    esp_ble_mesh_msg_ctx_t ctx = {
        .addr = 0X0001,
        .app_idx = 0X0000,
        .net_idx = 0,
        .send_ttl = 3,
        .send_rel = false,
    };

    if(ack!=NULL) { //This check is needed in some cases like HEARTBEAT ACK, CONF ACK, where ack will be NULL
        ack->bleErrorCode = SUCCESS; //We need to set this here, so that when ack is received at provisioner end, it won't be junk data
        strcpy(ack->deviceName, serialNoStr);
    }

    switch(packetid)
    {
        case NODE_PROV_PACKET:
            ESP_LOGI(BLE_TAG, "Sending Node Prov ACK to Provisioner");

            CommandStruct provack;
            provack.elemaddr = last_command.elemaddr;
            provack.packetid = NODE_PROV_PACKET;
            provack.errorcode = SUCCESS;
            strcpy(provack.deviceName, serialNoStr);
            err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS, sizeof(CommandStruct), (uint8_t *)&provack);
            if(err) ESP_LOGE(BLE_TAG, "Failed to ACK : %s", esp_err_to_name(err));
            break;

        case NODE_UNPROV_PACKET:
            ESP_LOGI(BLE_TAG, "Sending Node Unprov ACK to Provisioner");

            /*Unprovisioning from button press case*/
            if(ack == NULL) {
                CommandStruct unprovack;
                unprovack.packetid = NODE_UNPROV_PACKET;
                unprovack.elemaddr = last_command.elemaddr;
                unprovack.msgseqno = BUTTON_PRESS_MSGSEQNO;
                unprovack.bleErrorCode = SUCCESS;
                strcpy(unprovack.deviceName, serialNoStr);
                err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS, sizeof(CommandStruct), (uint8_t *)&unprovack);
                if(err) ESP_LOGE(BLE_TAG, "Failed to ACK : %s", esp_err_to_name(err));
            }
            break;
        
        case NODE_CONF_ACK:
            ESP_LOGI(BLE_TAG, "Sending Node AC Remote Configuration ACK to Provisioner");
            err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS, sizeof(CommandStruct), (uint8_t *)ack);
            if(err) ESP_LOGE(BLE_TAG, "Failed to ACK : %s", esp_err_to_name(err));
            break;
        
        case NODE_AC_CONTROL_PACKET:
            ESP_LOGI(BLE_TAG, "Sending Node AC Control ACK to Provisioner");
            err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS, sizeof(CommandStruct), (uint8_t *)ack);
            if(err) ESP_LOGE(BLE_TAG, "Failed to ACK : %s", esp_err_to_name(err));
            break;
        
        case NODE_RECONF_PACKET:
            ESP_LOGI(BLE_TAG, "Sending Node AC Remote Reconfiguration ACK to Provisioner");
            err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS, sizeof(CommandStruct), (uint8_t *)ack);
            if(err) ESP_LOGE(BLE_TAG, "Failed to ACK : %s", esp_err_to_name(err));
            break;
        
        case NODE_HEARTBEAT_PUB_CONF_PACKET:
            ESP_LOGI(BLE_TAG, "Sending Node Heartbeat Publish Configuration ACK to Provisioner");
            err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS, sizeof(CommandStruct), (uint8_t *)ack);
            if(err) ESP_LOGE(BLE_TAG, "Failed to ACK : %s", esp_err_to_name(err));
            break;

        case NODE_HEARTBEAT_ACK:
            ESP_LOGI(BLE_TAG, "Sending HB ACK to Provisioner");
            strcpy(last_command.deviceName, serialNoStr);
            last_command.packetid = NODE_HEARTBEAT_ACK; //We need to set this each time here, since we update the last command when we receive AC control packet.
            err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS, sizeof(CommandStruct), (uint8_t *)&last_command);
            if(err) ESP_LOGE(BLE_TAG, "Failed to ACK : %s", esp_err_to_name(err));
            break;
        
        case NODE_TEACHING_MODE_CMD_SELECTION_PACKET:
            ESP_LOGI(BLE_TAG, "Sending Node Teaching Mode Command Selection ACK to Provisioner");
            err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS, sizeof(CommandStruct), (uint8_t *)ack);
            if(err) ESP_LOGE(BLE_TAG, "Failed to ACK : %s", esp_err_to_name(err));
            break;
        
        case NODE_DEBUG_INFO_PACKET:
            ESP_LOGI(BLE_TAG, "Sending Node Debug Info ACK to Provisioner");
            ack->majversion = MAJ_VERSION;
            ack->minversion = MIN_VERSION;
            ack->patchversion = PATCH_VERSION;
            ack->deviceUpTimeHrs = (xTaskGetTickCount()*portTICK_PERIOD_MS)/3600000.00;
            ack->publishPeriodSec = publishPeriod;
            ack->irProtocolNum = ir_protocol_num;
            err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS, sizeof(CommandStruct), (uint8_t *)ack);
            if(err) ESP_LOGE(BLE_TAG, "Failed to ACK : %s", esp_err_to_name(err));
            break;
        
        case NODE_GENERAL_PACKET:
            ESP_LOGI(BLE_TAG, "Sending Node General ACK to Provisioner");
            err = esp_ble_mesh_server_model_send_msg(&vnd_models[0], &ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS, sizeof(CommandStruct), (uint8_t *)ack);
            if(err) ESP_LOGE(BLE_TAG, "Failed to ACK : %s", esp_err_to_name(err));
            break;

        default:
            ESP_LOGE(BLE_TAG, "Unknown ACK type %d in %s", packetid, __func__);
            return;
    }
}

/**
 * @brief Function that error checks the cmd that was received from Provisioner
 * @param cmd The command to be error checked
 * @retval true command received is valid 
 * @retval false command received is invalid
 */
void error_check_cmd(CommandStruct *cmd)
{
    switch(cmd->packetid)
    {
        case NODE_TEACHING_MODE:
            if(teaching_in_progress && cmd->teachingStart) {cmd->errorcode = DEVICE_ALREADY_IN_TEACHING_MODE; return;}
            if(!teaching_in_progress && cmd->errorcode) {cmd->errorcode = DEVICE_NOT_IN_TEACHING_MODE; return;}
            break;
        
        case NODE_TEACHING_MODE_CMD_SELECTION_PACKET:
            if(!(cmd->temperature >= teaching_mode_t.startingTemperature && cmd->temperature <= teaching_mode_t.endingTemperature))\
            {
                ESP_LOGW(BLE_TAG, "cmd->Temperature : %d | teaching_mode_t.startingTemperature : %d | teaching_mode_t.endingTemperature : %d",cmd->temperature, teaching_mode_t.startingTemperature, teaching_mode_t.endingTemperature);
                cmd->errorcode = TEMPERATURE_EXCEEDING_RANGE;
            }
            break;
        
        case NODE_AC_CONTROL_PACKET:
        case NODE_RECONF_PACKET:
            if(!configured) {cmd->errorcode = DEVICE_NOT_CONFIGURED_WITH_AC_REMOTE; return;}
            break;
        default:
            break;
    }
}

/**
 * @brief Function that handles cmds from provisioner
 */
void handle_cmds_from_provisioner(CommandStruct *cmd)
{
    cmd->errorcode = SUCCESS;
    error_check_cmd(cmd);
    ESP_LOGE(BLE_TAG, "ErrorCode : %s - %d", get_error_code_name(cmd->errorcode), cmd->errorcode);
    if(cmd->errorcode == SUCCESS)
    {
        led_set_state(LED_STATE_CMD_RECVD);
        switch(cmd->packetid)
        {
            case NODE_UNPROV_PACKET:
                ESP_LOGI(BLE_TAG, "Received Node Unprovision Packet from Provisioner");
                send_ack_to_provisioner(NODE_UNPROV_PACKET, cmd);
                unprovision_success_cb();
                break;

            case NODE_RECONF_PACKET:
                ESP_LOGI(BLE_TAG, "Received Node AC Remote Reconfguration Packet from Provisioner");
                handle_reconfiguration();
                send_ack_to_provisioner(cmd->packetid, cmd);
                break;
            
            case NODE_HEARTBEAT_PUB_CONF_PACKET:
                ESP_LOGI(BLE_TAG, "Recieved Node HeartBeat Publish Configuration Packet from Provisioner");
                handle_setting_hb_publish_configuration(cmd->publishPeriodSec);
                send_ack_to_provisioner(cmd->packetid, cmd);
                break;
            
            case NODE_TEACHING_MODE:
                ESP_LOGI(BLE_TAG, "Received Node Teaching Mode Packet from Provisioner");
                handle_configuring_teaching_mode(DUE_TO_MQTT_CMD, cmd);
                break;
            
            case NODE_TEACHING_MODE_CMD_SELECTION_PACKET:
                ESP_LOGI(BLE_TAG, "Received Node Teaching Mode Cmd Selection Packet from Provisioner");
                teaching_mode_t.power = cmd->power;
                teaching_mode_t.temperature = cmd->temperature;
                send_ack_to_provisioner(cmd->packetid, cmd);
                break;

            case NODE_AC_CONTROL_PACKET:
                ESP_LOGI(BLE_TAG, "Received Node AC Control Packet from Provisioner");
                handle_ac_control(cmd);
                send_ack_to_provisioner(cmd->packetid, cmd);
                break;
            
            case NODE_DEBUG_INFO_PACKET:
                ESP_LOGI(BLE_TAG, "Received Node Debug Info Packet from Provisioner");
                if(cmd->resetDevice) {
                    cmd->errorcode = factory_reset_device();
                    send_ack_to_provisioner(cmd->packetid, cmd);
                    powerCycleDevice(DUE_TO_MQTT_CMD);
                    break;
                }
                if(cmd->restartDevice) {
                    send_ack_to_provisioner(cmd->packetid, cmd);
                    powerCycleDevice(DUE_TO_MQTT_CMD);
                    break;
                }
                send_ack_to_provisioner(cmd->packetid, cmd);
                break;

            default:
                ESP_LOGE(BLE_TAG, "Received Unknown CMD from Provisioner : %d", cmd->packetid);
                break;
        }
    }
    else
    {
        led_set_state(LED_STATE_INVALID_OPERATION);

        if(cmd->packetid == NODE_TEACHING_MODE) {
            teaching_mode_t.errorCode = cmd->errorcode;
            send_teaching_mode_ack_to_provisioner();
            return;
        }
        
        send_ack_to_provisioner(cmd->packetid, cmd);
    }
}

/**
 * @brief Function that attaches the value of elemAddr after provisioning to structures used for ACK
 * @param elemAddr 
 */
void attach_elemAddr_to_structures(uint16_t elemAddr)
{
    last_command.elemaddr = elemAddr;
    ac_manual_control_t.elemAddr = elemAddr;
    teaching_mode_t.elemAddr = elemAddr;
}

/**
 * @brief Function that takes care of things to be done after provisioning
 * is successful
 * 
 */
void provision_success_cb()
{
    provisioned = true; update_led_status();
    send_ack_to_provisioner(NODE_PROV_PACKET, NULL);

    hb_timer_start();
    set_number_in_nvs_flash(GENERAL_HANDLE, NVS_PROVISIONED_KEY, 1, UINT8_SIZE);
    
    if(xTaskCreate(ir_recv_task, "IR Recv Task", IR_THREAD_STACK_SIZE, NULL, 2, &ir_recv_task_handle) != pdPASS)
    {
        ESP_LOGE(BLE_TAG, "IR Recv Task Creation Failed");
        construct_general_ack(IR_TASK_CREATION_FAILED);
    }
}

/**
 * @brief Function that takes care of things to be done after
 * unprovisioning is successful
 * 
 */
void unprovision_success_cb()
{
    ESP_LOGW(BLE_TAG, "Unprovisioning node");

    provisioned = false; led_set_state(LED_STATE_NODE_UNPROV_UNADV);

    /*Let's stop BLE Advertisement*/
    esp_ble_mesh_node_local_reset();
    esp_ble_mesh_node_prov_disable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);

    /*Let's stop IR Receiver task*/
    vTaskDelete(ir_recv_task_handle);

    /*Let's erase flash*/
    factory_reset_device();

    hb_timer_stop();
}

#endif