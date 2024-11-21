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
#include "esp_ble_mesh_sensor_model_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"

#include <ble.h>
#include <flash.h>
#include <led.h>
#include <ir.h>
#include <heartbeat.h>

#define BLE_TAG "BLE"

#define CID_ESP 0x02E5

static uint8_t dev_uuid[16] = { 0xdd, 0xdd };

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

static struct esp_ble_mesh_key
{
    uint16_t net_idx;
    uint16_t app_idx;
    uint8_t app_key[ESP_BLE_MESH_OCTET16_LEN];
} prov_key;

/* Disable OOB security for SILabs Android app */
static esp_ble_mesh_prov_t provision = {
    .uuid = dev_uuid,
#if 0
    .output_size = 4,
    .output_actions = ESP_BLE_MESH_DISPLAY_NUMBER,
    .input_actions = ESP_BLE_MESH_PUSH,
    .input_size = 4,
#else
    .output_size = 0,
    .output_actions = 0,
#endif
};

static void prov_complete(uint16_t net_idx, uint16_t addr, uint8_t flags, uint32_t iv_index)
{
    ESP_LOGI(BLE_TAG, "net_idx: 0x%04x, addr: 0x%04x", net_idx, addr);
    ESP_LOGI(BLE_TAG, "flags: 0x%02x, iv_index: 0x%08" PRIx32, flags, iv_index);
}

static void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                                             esp_ble_mesh_prov_cb_param_t *param)
{
    switch (event) {
    case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
        ESP_LOGI(BLE_TAG, "ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT");
        prov_complete(param->node_prov_complete.net_idx, param->node_prov_complete.addr,
            param->node_prov_complete.flags, param->node_prov_complete.iv_index);
        provision_success_cb();
        break;

    case ESP_BLE_MESH_NODE_PROV_RESET_EVT:
        ESP_LOGI(BLE_TAG, "ESP_BLE_MESH_NODE_PROV_RESET_EVT : %s", __func__);
        unprovision_success_cb();
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

        case ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD:
            ESP_LOGI(BLE_TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD");
            ESP_LOGI(BLE_TAG, "elem_addr 0x%04x, sub_addr 0x%04x, cid 0x%04x, mod_id 0x%04x",
                param->value.state_change.mod_sub_add.element_addr,
                param->value.state_change.mod_sub_add.sub_addr,
                param->value.state_change.mod_sub_add.company_id,
                param->value.state_change.mod_sub_add.model_id);
            break;

        case ESP_BLE_MESH_MODEL_OP_NODE_RESET:
            ESP_LOGI(BLE_TAG, "ESP_BLE_MESH_MODEL_OP_NODE_RESET : %s",__func__);
            break;

        default:
            break;
        }
    }
}


NET_BUF_SIMPLE_DEFINE_STATIC(sensor_data_0, 100);

static esp_ble_mesh_sensor_state_t sensor_states[1] = {
    [0] = {
        .sensor_property_id = 0x0001,
        .sensor_data.raw_value = &sensor_data_0,
    },
};

/* 20 octets is large enough to hold two Sensor Descriptor state values. */
ESP_BLE_MESH_MODEL_PUB_DEFINE(sensor_pub, 20, ROLE_NODE);
static esp_ble_mesh_sensor_srv_t sensor_server = {
    .rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .state_count = ARRAY_SIZE(sensor_states),
    .states = sensor_states,
};

ESP_BLE_MESH_MODEL_PUB_DEFINE(sensor_setup_pub, 20, ROLE_NODE);
static esp_ble_mesh_sensor_setup_srv_t sensor_setup_server = {
    .rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .state_count = ARRAY_SIZE(sensor_states),
    .states = sensor_states,
};

static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_SENSOR_SRV(&sensor_pub, &sensor_server),
    ESP_BLE_MESH_MODEL_SENSOR_SETUP_SRV(&sensor_setup_pub, &sensor_setup_server),
};
static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, root_models, ESP_BLE_MESH_MODEL_NONE),
};

static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

static uint16_t example_ble_mesh_get_sensor_data(esp_ble_mesh_sensor_state_t *state, uint8_t *data)
{
    uint8_t mpid_len, data_len = 0;
    uint32_t mpid;

    if (state == NULL || data == NULL)
    {
        ESP_LOGE(BLE_TAG, "%s, Invalid parameter", __func__);
        return 0;
    }

    if (state->sensor_data.length == ESP_BLE_MESH_SENSOR_DATA_ZERO_LEN)
    {
        /* For zero-length sensor data, the length is 0x7F, and the format is Format B. */
        mpid = ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID(state->sensor_data.length, state->sensor_property_id);
        mpid_len = ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID_LEN;
        data_len = 0;
    }
    else
    {
        if (state->sensor_data.format == ESP_BLE_MESH_SENSOR_DATA_FORMAT_A)
        {
            mpid = ESP_BLE_MESH_SENSOR_DATA_FORMAT_A_MPID(state->sensor_data.length, state->sensor_property_id);
            mpid_len = ESP_BLE_MESH_SENSOR_DATA_FORMAT_A_MPID_LEN;
        }
        else
        {
            mpid = ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID(state->sensor_data.length, state->sensor_property_id);
            mpid_len = ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID_LEN;
        }
        /* Use "state->sensor_data.length + 1" because the length of sensor data is zero-based. */
        data_len = state->sensor_data.length + 1;
    }
    mpid_len = 29;
    data_len = 125;
    memcpy(data, state->sensor_data.raw_value->data, data_len);

    return (data_len);
}

static void example_ble_mesh_send_sensor_status(/*int aesp_ble_mesh_sensor_server_cb_param_t *param*/)
{
    uint8_t *status = NULL;
    uint16_t buf_size = 0;
    uint16_t length = 0;
    int i;

    for (i = 0; i < ARRAY_SIZE(sensor_states); i++)
    {
        esp_ble_mesh_sensor_state_t *state = &sensor_states[i];
        if (state->sensor_data.length == ESP_BLE_MESH_SENSOR_DATA_ZERO_LEN)
        {
            buf_size += ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID_LEN;
        }
        else
        {
            /* Use "state->sensor_data.length + 1" because the length of sensor data is zero-based. */
            if (state->sensor_data.format == ESP_BLE_MESH_SENSOR_DATA_FORMAT_A)
            {
                buf_size += ESP_BLE_MESH_SENSOR_DATA_FORMAT_A_MPID_LEN + state->sensor_data.length + 1;
            }
            else
            {
                buf_size += ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID_LEN + state->sensor_data.length + 1;
            }
        }
    }

    // status = calloc(1, BLE_BUF_SIZE);
    status = calloc(1, 500);
    if (!status)
    {
        ESP_LOGE(BLE_TAG, "No memory for sensor status!");
        return;
    }

    for (i = 0; i < 1; i++)
    // for (i = 0; i < 1 ARRAY_SIZE(sensor_states); i++)
    {
        length += example_ble_mesh_get_sensor_data(&sensor_states[i], status + length);
    }
    goto send;

send:
    if(true) {
        ESP_LOG_BUFFER_HEX("Sensor Data", status, length);
    }
    sensor_server.model->pub->publish_addr = 0x01;
    sensor_server.model->pub->app_idx = prov_key.app_idx;
    sensor_server.model->pub->ttl = 2;
    esp_ble_mesh_model_publish(sensor_server.model, ESP_BLE_MESH_MODEL_OP_SENSOR_STATUS, length, status, ROLE_NODE);

    free(status);
}


esp_err_t bluetooth_init(void)
{
    esp_err_t ret;

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(BLE_TAG, "%s initialize controller failed", __func__);
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(BLE_TAG, "%s enable controller failed", __func__);
        return ret;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(BLE_TAG, "%s init bluetooth failed", __func__);
        return ret;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(BLE_TAG, "%s enable bluetooth failed", __func__);
        return ret;
    }
    return ret;
}

static esp_err_t ble_mesh_init(void)
{
    esp_err_t err = ESP_OK;

    esp_ble_mesh_register_prov_callback(example_ble_mesh_provisioning_cb);
    esp_ble_mesh_register_config_server_callback(example_ble_mesh_config_server_cb);
    
    err = esp_ble_mesh_init(&provision, &composition);
    if (err != ESP_OK) {
        ESP_LOGE(BLE_TAG, "Failed to initialize mesh stack (err %d)", err);
        return err;
    }

    err = esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
    if (err != ESP_OK) {
        ESP_LOGE(BLE_TAG, "Failed to enable mesh node (err %d)", err);
        return err;
    }

    ESP_LOGI(BLE_TAG, "BLE Mesh Node initialized");

    return err;
}

void ble_mesh_get_dev_uuid(uint8_t *dev_uuid)
{
    if (dev_uuid == NULL) {
        ESP_LOGE(BLE_TAG, "%s, Invalid device uuid", __func__);
        return;
    }
    
    /* Copy device address to the device uuid with offset equals to 2 here.
     * The first two bytes is used for matching device uuid by Provisioner.
     * And using device address here is to avoid using the same device uuid
     * by different unprovisioned devices.
     */
    memcpy(dev_uuid + 2, esp_bt_dev_get_address(), BD_ADDR_LEN);
}

/**
 * @brief Function that initializes the BLE Mesh for Node
 */
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

    ble_mesh_get_dev_uuid(dev_uuid);

    /* Initialize the Bluetooth Mesh Subsystem */
    err = ble_mesh_init();
    if (err) {
        ESP_LOGE(BLE_TAG, "Bluetooth mesh init failed (err %d)", err);
    }
}


/*FUNCTIONS BELOW THIS ARE DEFINED BY QMAX - FOR PROJECT'S PURPOSE*/

/**
 * @brief Function that sends ack to provisioner
 */
void send_ack_to_provisioner(uint16_t packetid, CommandStruct *cmd_struct)
{
    switch(packetid)
    {
        case NODE_CONF_PACKET:
            ESP_LOGI(BLE_TAG, "Sending AC Remote Configuration ACK");
            cmd_struct->packetid = NODE_CONF_PACKET;
            if(ac_remote_unsupported_flag) cmd_struct->errorcode = AC_REMOTE_UNSUPPORTED;
            else cmd_struct->errorcode = SUCCESS;
            cmd_struct->irProtocolNum = ir_protocol_num;
            sensor_states[0].sensor_data.raw_value->data = (uint8_t *)cmd_struct;
            break;

        case NODE_AC_CONTROL_PACKET:
            ESP_LOGI(BLE_TAG, "Sending Node AC Control ACK");
            sensor_states[0].sensor_data.raw_value->data = (uint8_t *)cmd_struct;
            break;
        
        case NODE_MANUAL_AC_CONTROL_ACK:
            ESP_LOGI(BLE_TAG, "Sending Node Manual AC Control ACK");
            sensor_states[0].sensor_data.raw_value->data = (uint8_t *)&ac_manual_control_t;
            break;
        
        case NODE_RECONF_PACKET:
            ESP_LOGI(BLE_TAG, "Sending Node Reconf ACK");
            sensor_states[0].sensor_data.raw_value->data = (uint8_t *)cmd_struct;
            break;
        
        case NODE_HEARTBEAT_ACK:
            ESP_LOGI(BLE_TAG, "Sending Node HB ACK");
            last_command.packetid = NODE_HEARTBEAT_ACK;
            sensor_states[0].sensor_data.raw_value->data = (uint8_t *)&last_command;
            break;
        
        case NODE_HEARTBEAT_PUB_CONF_PACKET:
            ESP_LOGI(BLE_TAG, "Sending Node HB PubConf ACK");
            sensor_states[0].sensor_data.raw_value->data = (uint8_t *)cmd_struct;
            break;
        
        case NODE_TEACHING_MODE:
            ESP_LOGI(BLE_TAG, "Sending Node Teaching Mode ACK");
            teaching_mode_t.packetid = NODE_TEACHING_MODE;
            sensor_states[0].sensor_data.raw_value->data = (uint8_t *)&teaching_mode_t;
            break;
        
        case NODE_DEBUG_INFO_PACKET:
            ESP_LOGI(BLE_TAG, "Sending Node Debug Info ACK");
            cmd_struct->provisioned = provisioned;
            cmd_struct->majversion = MAJ_VERSION;
            cmd_struct->minversion = MIN_VERSION;
            cmd_struct->patchversion = PATCH_VERSION;
            cmd_struct->deviceUpTimeMs = (xTaskGetTickCount()*portTICK_PERIOD_MS)/3600000.00;
            cmd_struct->irProtocolNum = ir_protocol_num;
            cmd_struct->configured = configured;
            sensor_states[0].sensor_data.raw_value->data = (uint8_t *)cmd_struct;
            break;
        
        default:
            break;
    }
    
    example_ble_mesh_send_sensor_status();
}

/**
 * @brief Function that errorchecks the command that was recevied from provisioner
 * @param cmd_struct The command strucutre that was received
 */
void error_check_cmd_recvd_from_provisioner(CommandStruct *cmd_struct)
{
    if(!provisioned) {cmd_struct->errorcode = NODE_NOT_PROVISIONED; return;}

    switch (cmd_struct->packetid)
    {
    case NODE_RECONF_PACKET:
        if(teaching_in_progress) {cmd_struct->errorcode = DEVICE_ALREADY_IN_TEACHING_MODE; return;}
        break; 
    
    case NODE_AC_CONTROL_PACKET:
        if(!configured) {cmd_struct->errorcode = NODE_NOT_CONFIGURED; return;}
        break;
    
    case NODE_TEACHING_MODE:
        if(cmd_struct->teachingStart && teaching_in_progress) {cmd_struct->errorcode = DEVICE_ALREADY_IN_TEACHING_MODE; return;}
        if(!cmd_struct->teachingStart && !teaching_in_progress) {cmd_struct->errorcode = DEVICE_NOT_IN_TEACHING_MODE; return;}
        break;

    default:
        break;
    }
}

/**
 * @brief Function that handles the commands received from Provisioner
 * @note Provision/Unprovision requests are handled separately
 */
void handle_cmds_from_provisioner(CommandStruct *cmd_struct)
{
    error_check_cmd_recvd_from_provisioner(cmd_struct);
    ESP_LOGI(BLE_TAG, "ErrorCode : %d - %s", cmd_struct->errorcode, get_error_code_name(cmd_struct->errorcode));
    
    if(cmd_struct->errorcode == SUCCESS)
    {
        switch(cmd_struct->packetid)
        {
            case NODE_RECONF_PACKET:
                handle_reconfiguration(cmd_struct);
                break;
            
            case NODE_AC_CONTROL_PACKET:
                handle_ac_control(cmd_struct);
                break;
            
            case NODE_HEARTBEAT_PUB_CONF_PACKET:
                handle_setting_hb_publish_configuration(cmd_struct);
                break;
            
            case NODE_DEBUG_INFO_PACKET:
                send_ack_to_provisioner(cmd_struct->packetid, cmd_struct);
                break;
            
            case NODE_TEACHING_MODE:
                handle_configuring_teaching_mode(cmd_struct);
                break;
            
            default:
                ESP_LOGE(BLE_TAG, "Unknown command received from Provisioner : %d",cmd_struct->packetid);
                break;
        }
    }
    else
    {
        led_set_state(LED_STATE_INVALID_OPERATION);
        send_ack_to_provisioner(cmd_struct->packetid, cmd_struct);
    }
}

/**
 * @brief Callback function that gets called when provisioning of node is successful
 */
void provision_success_cb()
{
    provisioned = true;
    set_number_in_nvs_flash(GENERAL_HANDLE, NVS_PROVISIONED_KEY, 1, UINT8_SIZE);
    update_led_status();
}

/**
 * @brief Function that unprovisions the node
 */
void unprovision_success_cb()
{
    ESP_LOGW(BLE_TAG, "Unprovisioning Node ...");
    esp_ble_mesh_node_local_reset();
    esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
    provisioned = false;
    set_number_in_nvs_flash(GENERAL_HANDLE, NVS_PROVISIONED_KEY, 0, UINT8_SIZE);
    update_led_status();
}

#endif