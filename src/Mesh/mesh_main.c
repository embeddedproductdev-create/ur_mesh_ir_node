/* main.c - Application main entry point */

/*
 * SPDX-FileCopyrightText: 2017 Intel Corporation
 * SPDX-FileContributor: 2018-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_sensor_model_api.h"

//#include "ble_mesh_example_init.h"
//#include "board.h"
#include "esp_err.h"

void ble_mesh_get_dev_uuid(uint8_t *dev_uuid);

esp_err_t bluetooth_init(void);

#define TAG "EXAMPLE"

#define CID_ESP     0x02E5

#define PROV_OWN_ADDR       0x0001

#define MSG_SEND_TTL        3
#define MSG_SEND_REL        false
#define MSG_TIMEOUT         0
#define MSG_ROLE            ROLE_PROVISIONER

#define COMP_DATA_PAGE_0    0x00

#define APP_KEY_IDX         0x0000
#define APP_KEY_OCTET       0x12

#define COMP_DATA_1_OCTET(msg, offset)      (msg[offset])
#define COMP_DATA_2_OCTET(msg, offset)      (msg[offset + 1] << 8 | msg[offset])

/* Sensor Property ID */
#define SENSOR_PROPERTY_ID_0        0x0056  /* Present Indoor Ambient Temperature */
#define SENSOR_PROPERTY_ID_1        0x005B  /* Present Outdoor Ambient Temperature */

/* The characteristic of the two device properties is "Temperature 8", which is
 * used to represent a measure of temperature with a unit of 0.5 degree Celsius.
 * Minimum value: -64.0, maximum value: 63.5.
 * A value of 0xFF represents 'value is not known'.
 */
static int8_t indoor_temp = 40;     /* Indoor temperature is 20 Degrees Celsius */
static int8_t outdoor_temp = 60;    /* Outdoor temperature is 30 Degrees Celsius */

#define SENSOR_POSITIVE_TOLERANCE   ESP_BLE_MESH_SENSOR_UNSPECIFIED_POS_TOLERANCE
#define SENSOR_NEGATIVE_TOLERANCE   ESP_BLE_MESH_SENSOR_UNSPECIFIED_NEG_TOLERANCE
#define SENSOR_SAMPLE_FUNCTION      ESP_BLE_MESH_SAMPLE_FUNC_UNSPECIFIED
#define SENSOR_MEASURE_PERIOD       ESP_BLE_MESH_SENSOR_NOT_APPL_MEASURE_PERIOD
#define SENSOR_UPDATE_INTERVAL      ESP_BLE_MESH_SENSOR_NOT_APPL_UPDATE_INTERVAL

static uint8_t dev_uuid[ESP_BLE_MESH_OCTET16_LEN] = { 0xdd, 0xdd };
static uint16_t server_address = ESP_BLE_MESH_ADDR_UNASSIGNED;
static uint16_t sensor_prop_id;

uint8_t binded=0,prov=0;

static struct esp_ble_mesh_key {
    uint16_t net_idx;
    uint16_t app_idx;
    uint8_t  app_key[ESP_BLE_MESH_OCTET16_LEN];
} prov_key;

static void example_ble_mesh_set_msg_common(esp_ble_mesh_client_common_param_t *common,
                                            esp_ble_mesh_node_t *node,
                                            esp_ble_mesh_model_t *model, uint32_t opcode)
{
    common->opcode = opcode;
    common->model = model;
    common->ctx.net_idx = prov_key.net_idx;
    common->ctx.app_idx = prov_key.app_idx;
    common->ctx.addr = node->unicast_addr;
    common->ctx.send_ttl = MSG_SEND_TTL;
    common->ctx.send_rel = MSG_SEND_REL;
    common->msg_timeout = MSG_TIMEOUT;
    common->msg_role = MSG_ROLE;
}

static esp_ble_mesh_cfg_srv_t config_server = {
    .relay = ESP_BLE_MESH_RELAY_ENABLED,
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

static esp_ble_mesh_client_t config_client;
static esp_ble_mesh_client_t sensor_client;

NET_BUF_SIMPLE_DEFINE_STATIC(sensor_data_0, 40);
NET_BUF_SIMPLE_DEFINE_STATIC(sensor_data_1, 40);

/*NET_BUF_SIMPLE_DEFINE_STATIC(sensor_data_2, 30);
NET_BUF_SIMPLE_DEFINE_STATIC(sensor_data_3, 30);*/



static esp_ble_mesh_sensor_state_t sensor_states[2] = {
    /* Mesh Model Spec:
     * Multiple instances of the Sensor states may be present within the same model,
     * provided that each instance has a unique value of the Sensor Property ID to
     * allow the instances to be differentiated. Such sensors are known as multisensors.
     * In this example, two instances of the Sensor states within the same model are
     * provided.
     */
    [0] = {
        /* Mesh Model Spec:
         * Sensor Property ID is a 2-octet value referencing a device property
         * that describes the meaning and format of data reported by a sensor.
         * 0x0000 is prohibited.
         */
        .sensor_property_id = SENSOR_PROPERTY_ID_0,
        /* Mesh Model Spec:
         * Sensor Descriptor state represents the attributes describing the sensor
         * data. This state does not change throughout the lifetime of an element.
         */
        .descriptor.positive_tolerance = SENSOR_POSITIVE_TOLERANCE,
        .descriptor.negative_tolerance = SENSOR_NEGATIVE_TOLERANCE,
        .descriptor.sampling_function = SENSOR_SAMPLE_FUNCTION,
        .descriptor.measure_period = SENSOR_MEASURE_PERIOD,
        .descriptor.update_interval = SENSOR_UPDATE_INTERVAL,
        .sensor_data.format = ESP_BLE_MESH_SENSOR_DATA_FORMAT_A,
        .sensor_data.length = 0, /* 0 represents the length is 1 */
        .sensor_data.raw_value = &sensor_data_0,
    },
    [1] = {
        .sensor_property_id = SENSOR_PROPERTY_ID_1,
        .descriptor.positive_tolerance = SENSOR_POSITIVE_TOLERANCE,
        .descriptor.negative_tolerance = SENSOR_NEGATIVE_TOLERANCE,
        .descriptor.sampling_function = SENSOR_SAMPLE_FUNCTION,
        .descriptor.measure_period = SENSOR_MEASURE_PERIOD,
        .descriptor.update_interval = SENSOR_UPDATE_INTERVAL,
        .sensor_data.format = ESP_BLE_MESH_SENSOR_DATA_FORMAT_A,
        .sensor_data.length = 0, /* 0 represents the length is 1 */
        .sensor_data.raw_value = &sensor_data_1,
    },

};

/* 20 octets is large enough to hold two Sensor Descriptor state values. */
ESP_BLE_MESH_MODEL_PUB_DEFINE(sensor_pub, 90, ROLE_NODE);
static esp_ble_mesh_sensor_srv_t sensor_server = {
    .rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .state_count = ARRAY_SIZE(sensor_states),
    .states = sensor_states,
};

ESP_BLE_MESH_MODEL_PUB_DEFINE(sensor_setup_pub, 90, ROLE_NODE);
static esp_ble_mesh_sensor_setup_srv_t sensor_setup_server = {
    .rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .state_count = ARRAY_SIZE(sensor_states),
    .states = sensor_states,
};

ESP_BLE_MESH_MODEL_PUB_DEFINE(client_pub, 90, ROLE_NODE);
static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_SENSOR_SRV(&sensor_pub, &sensor_server),
    ESP_BLE_MESH_MODEL_SENSOR_SETUP_SRV(&sensor_setup_pub, &sensor_setup_server),
	ESP_BLE_MESH_MODEL_CFG_CLI(&config_client),
    ESP_BLE_MESH_MODEL_SENSOR_CLI(&client_pub, &sensor_client),
};

static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, root_models, ESP_BLE_MESH_MODEL_NONE),
};

static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

static esp_ble_mesh_prov_t provision = {
    .uuid = dev_uuid,
	 .output_size = 0,
	    .output_actions = 0,
};

static void prov_complete(uint16_t net_idx, uint16_t addr, uint8_t flags, uint32_t iv_index)
{
    ESP_LOGI(TAG, "net_idx 0x%03x, addr 0x%04x", net_idx, addr);
    ESP_LOGI(TAG, "flags 0x%02x, iv_index 0x%08" PRIx32, flags, iv_index);
    //board_led_operation(LED_G, LED_OFF);

    /* Initialize the indoor and outdoor temperatures for each sensor.  */
   // net_buf_simple_add_u8(&sensor_data_0, indoor_temp);
   // net_buf_simple_add_u8(&sensor_data_1, outdoor_temp);
}

static void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                                             esp_ble_mesh_prov_cb_param_t *param)
{
    switch (event) {
    case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT, err_code %d", param->node_prov_enable_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT, bearer %s",
            param->node_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT, bearer %s",
            param->node_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT");
        prov_complete(param->node_prov_complete.net_idx, param->node_prov_complete.addr,
            param->node_prov_complete.flags, param->node_prov_complete.iv_index);
        break;
    case ESP_BLE_MESH_NODE_PROV_RESET_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_PROV_RESET_EVT");
        break;
    case ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT, err_code %d", param->node_set_unprov_dev_name_comp.err_code);
        break;
    default:
        break;
    }
}

static void example_ble_mesh_parse_node_comp_data(const uint8_t *data, uint16_t length)
{
    uint16_t cid, pid, vid, crpl, feat;
    uint16_t loc, model_id, company_id;
    uint8_t nums, numv;
    uint16_t offset;
    int i;

    cid = COMP_DATA_2_OCTET(data, 0);
    pid = COMP_DATA_2_OCTET(data, 2);
    vid = COMP_DATA_2_OCTET(data, 4);
    crpl = COMP_DATA_2_OCTET(data, 6);
    feat = COMP_DATA_2_OCTET(data, 8);
    offset = 10;

    ESP_LOGI(TAG, "********************** Composition Data Start **********************");
    ESP_LOGI(TAG, "* CID 0x%04x, PID 0x%04x, VID 0x%04x, CRPL 0x%04x, Features 0x%04x *", cid, pid, vid, crpl, feat);
    for (; offset < length; ) {
        loc = COMP_DATA_2_OCTET(data, offset);
        nums = COMP_DATA_1_OCTET(data, offset + 2);
        numv = COMP_DATA_1_OCTET(data, offset + 3);
        offset += 4;
        ESP_LOGI(TAG, "* Loc 0x%04x, NumS 0x%02x, NumV 0x%02x *", loc, nums, numv);
        for (i = 0; i < nums; i++) {
            model_id = COMP_DATA_2_OCTET(data, offset);
            ESP_LOGI(TAG, "* SIG Model ID 0x%04x *", model_id);
            offset += 2;
        }
        for (i = 0; i < numv; i++) {
            company_id = COMP_DATA_2_OCTET(data, offset);
            model_id = COMP_DATA_2_OCTET(data, offset + 2);
            ESP_LOGI(TAG, "* Vendor Model ID 0x%04x, Company ID 0x%04x *", model_id, company_id);
            offset += 4;
        }
    }
    ESP_LOGI(TAG, "*********************** Composition Data End ***********************");
}

static void example_ble_mesh_config_client_cb(esp_ble_mesh_cfg_client_cb_event_t event,
                                              esp_ble_mesh_cfg_client_cb_param_t *param)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_cfg_client_set_state_t set = {0};
    static uint16_t wait_model_id, wait_cid;
    esp_ble_mesh_node_t *node = NULL;
    esp_err_t err = ESP_OK;

    ESP_LOGI(TAG, "Config client, event %u, addr 0x%04x, opcode 0x%04" PRIx32,
        event, param->params->ctx.addr, param->params->opcode);

    if (param->error_code) {
        ESP_LOGE(TAG, "Send config client message failed (err %d)", param->error_code);
        return;
    }

    node = esp_ble_mesh_provisioner_get_node_with_addr(param->params->ctx.addr);
    if (!node) {
        ESP_LOGE(TAG, "Node 0x%04x not exists", param->params->ctx.addr);
        return;
    }

    switch (event) {
    case ESP_BLE_MESH_CFG_CLIENT_GET_STATE_EVT:
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET) {
            ESP_LOG_BUFFER_HEX("Composition data", param->status_cb.comp_data_status.composition_data->data,
                param->status_cb.comp_data_status.composition_data->len);
            example_ble_mesh_parse_node_comp_data(param->status_cb.comp_data_status.composition_data->data,
                param->status_cb.comp_data_status.composition_data->len);
            err = esp_ble_mesh_provisioner_store_node_comp_data(param->params->ctx.addr,
                param->status_cb.comp_data_status.composition_data->data,
                param->status_cb.comp_data_status.composition_data->len);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to store node composition data");
                break;
            }

            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
            set.app_key_add.net_idx = prov_key.net_idx;
            set.app_key_add.app_idx = prov_key.app_idx;
            memcpy(set.app_key_add.app_key, prov_key.app_key, ESP_BLE_MESH_OCTET16_LEN);
            err = esp_ble_mesh_config_client_set_state(&common, &set);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to send Config AppKey Add");
            }
        }
        break;
    case ESP_BLE_MESH_CFG_CLIENT_SET_STATE_EVT:
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD) {
            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
            set.model_app_bind.element_addr = node->unicast_addr;
            set.model_app_bind.model_app_idx = prov_key.app_idx;
            set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SRV;
            set.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
            err = esp_ble_mesh_config_client_set_state(&common, &set);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to send Config Model App Bind");
                return;
            }
            wait_model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SRV;
            wait_cid = ESP_BLE_MESH_CID_NVAL;
        } else if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND) {
            if (param->status_cb.model_app_status.model_id == ESP_BLE_MESH_MODEL_ID_SENSOR_SRV &&
                param->status_cb.model_app_status.company_id == ESP_BLE_MESH_CID_NVAL) {
                example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
                set.model_app_bind.element_addr = node->unicast_addr;
                set.model_app_bind.model_app_idx = prov_key.app_idx;
                set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV;
                set.model_app_bind.company_id = ESP_BLE_MESH_CID_NVAL;
                err = esp_ble_mesh_config_client_set_state(&common, &set);
                if (err) {
                    ESP_LOGE(TAG, "Failed to send Config Model App Bind");
                    return;
                }
                wait_model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV;
                wait_cid = ESP_BLE_MESH_CID_NVAL;
            } else if (param->status_cb.model_app_status.model_id == ESP_BLE_MESH_MODEL_ID_SENSOR_SETUP_SRV &&
                param->status_cb.model_app_status.company_id == ESP_BLE_MESH_CID_NVAL) {
                ESP_LOGW(TAG, "Provision and config successfully");
            }
        }
        break;
    case ESP_BLE_MESH_CFG_CLIENT_PUBLISH_EVT:
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_STATUS) {
            ESP_LOG_BUFFER_HEX("Composition data", param->status_cb.comp_data_status.composition_data->data,
                param->status_cb.comp_data_status.composition_data->len);
        }
        break;
    case ESP_BLE_MESH_CFG_CLIENT_TIMEOUT_EVT:
        switch (param->params->opcode) {
        case ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET: {
            esp_ble_mesh_cfg_client_get_state_t get = {0};
            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET);
            get.comp_data_get.page = COMP_DATA_PAGE_0;
            err = esp_ble_mesh_config_client_get_state(&common, &get);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to send Config Composition Data Get");
            }
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
            set.app_key_add.net_idx = prov_key.net_idx;
            set.app_key_add.app_idx = prov_key.app_idx;
            memcpy(set.app_key_add.app_key, prov_key.app_key, ESP_BLE_MESH_OCTET16_LEN);
            err = esp_ble_mesh_config_client_set_state(&common, &set);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to send Config AppKey Add");
            }
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
            set.model_app_bind.element_addr = node->unicast_addr;
            set.model_app_bind.model_app_idx = prov_key.app_idx;
            set.model_app_bind.model_id = wait_model_id;
            set.model_app_bind.company_id = wait_cid;
            err = esp_ble_mesh_config_client_set_state(&common, &set);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to send Config Model App Bind");
            }
            break;
        default:
            break;
        }
        break;
    default:
        ESP_LOGE(TAG, "Invalid config client event %u", event);
        break;
    }
}



struct example_sensor_descriptor {
    uint16_t sensor_prop_id;
    uint32_t pos_tolerance:12,
             neg_tolerance:12,
             sample_func:8;
    uint8_t  measure_period;
    uint8_t  update_interval;
} __attribute__((packed));

static void example_ble_mesh_send_sensor_descriptor_status(esp_ble_mesh_sensor_server_cb_param_t *param)
{
    struct example_sensor_descriptor descriptor = {0};
    uint8_t *status = NULL;
    uint16_t length = 0;
    esp_err_t err;
    int i;

    status = calloc(1, ARRAY_SIZE(sensor_states) * ESP_BLE_MESH_SENSOR_DESCRIPTOR_LEN);
    if (!status) {
        ESP_LOGE(TAG, "No memory for sensor descriptor status!");
        return;
    }

    if (param->value.get.sensor_descriptor.op_en == false) {
        /* Mesh Model Spec:
         * Upon receiving a Sensor Descriptor Get message with the Property ID field
         * omitted, the Sensor Server shall respond with a Sensor Descriptor Status
         * message containing the Sensor Descriptor states for all sensors within the
         * Sensor Server.
         */
        for (i = 0; i < ARRAY_SIZE(sensor_states); i++) {
            descriptor.sensor_prop_id = sensor_states[i].sensor_property_id;
            descriptor.pos_tolerance = sensor_states[i].descriptor.positive_tolerance;
            descriptor.neg_tolerance = sensor_states[i].descriptor.negative_tolerance;
            descriptor.sample_func = sensor_states[i].descriptor.sampling_function;
            descriptor.measure_period = sensor_states[i].descriptor.measure_period;
            descriptor.update_interval = sensor_states[i].descriptor.update_interval;
            memcpy(status + length, &descriptor, ESP_BLE_MESH_SENSOR_DESCRIPTOR_LEN);
            length += ESP_BLE_MESH_SENSOR_DESCRIPTOR_LEN;
        }
        goto send;
    }

    for (i = 0; i < ARRAY_SIZE(sensor_states); i++) {
        if (param->value.get.sensor_descriptor.property_id == sensor_states[i].sensor_property_id) {
            descriptor.sensor_prop_id = sensor_states[i].sensor_property_id;
            descriptor.pos_tolerance = sensor_states[i].descriptor.positive_tolerance;
            descriptor.neg_tolerance = sensor_states[i].descriptor.negative_tolerance;
            descriptor.sample_func = sensor_states[i].descriptor.sampling_function;
            descriptor.measure_period = sensor_states[i].descriptor.measure_period;
            descriptor.update_interval = sensor_states[i].descriptor.update_interval;
            memcpy(status, &descriptor, ESP_BLE_MESH_SENSOR_DESCRIPTOR_LEN);
            length = ESP_BLE_MESH_SENSOR_DESCRIPTOR_LEN;
            goto send;
        }
    }

    /* Mesh Model Spec:
     * When a Sensor Descriptor Get message that identifies a sensor descriptor
     * property that does not exist on the element, the Descriptor field shall
     * contain the requested Property ID value and the other fields of the Sensor
     * Descriptor state shall be omitted.
     */
    memcpy(status, &param->value.get.sensor_descriptor.property_id, ESP_BLE_MESH_SENSOR_PROPERTY_ID_LEN);
    length = ESP_BLE_MESH_SENSOR_PROPERTY_ID_LEN;

send:
    ESP_LOG_BUFFER_HEX("Sensor Descriptor", status, length);

    err = esp_ble_mesh_server_model_send_msg(param->model, &param->ctx,
            ESP_BLE_MESH_MODEL_OP_SENSOR_DESCRIPTOR_STATUS, length, status);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Sensor Descriptor Status");
    }
    free(status);
}

static void example_ble_mesh_send_sensor_cadence_status(esp_ble_mesh_sensor_server_cb_param_t *param)
{
    esp_err_t err;

    /* Sensor Cadence state is not supported currently. */
    err = esp_ble_mesh_server_model_send_msg(param->model, &param->ctx,
            ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_STATUS,
            ESP_BLE_MESH_SENSOR_PROPERTY_ID_LEN,
            (uint8_t *)&param->value.get.sensor_cadence.property_id);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Sensor Cadence Status");
    }
}

static void example_ble_mesh_send_sensor_settings_status(esp_ble_mesh_sensor_server_cb_param_t *param)
{
    esp_err_t err;

    /* Sensor Setting state is not supported currently. */
    err = esp_ble_mesh_server_model_send_msg(param->model, &param->ctx,
            ESP_BLE_MESH_MODEL_OP_SENSOR_SETTINGS_STATUS,
            ESP_BLE_MESH_SENSOR_PROPERTY_ID_LEN,
            (uint8_t *)&param->value.get.sensor_settings.property_id);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Sensor Settings Status");
    }
}

struct example_sensor_setting {
    uint16_t sensor_prop_id;
    uint16_t sensor_setting_prop_id;
} __attribute__((packed));

static void example_ble_mesh_send_sensor_setting_status(esp_ble_mesh_sensor_server_cb_param_t *param)
{
    struct example_sensor_setting setting = {0};
    esp_err_t err;

    /* Mesh Model Spec:
     * If the message is sent as a response to the Sensor Setting Get message or
     * a Sensor Setting Set message with an unknown Sensor Property ID field or
     * an unknown Sensor Setting Property ID field, the Sensor Setting Access
     * field and the Sensor Setting Raw field shall be omitted.
     */

    setting.sensor_prop_id = param->value.get.sensor_setting.property_id;
    setting.sensor_setting_prop_id = param->value.get.sensor_setting.setting_property_id;

    err = esp_ble_mesh_server_model_send_msg(param->model, &param->ctx,
            ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_STATUS,
            sizeof(setting), (uint8_t *)&setting);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Sensor Setting Status");
    }
}

static uint16_t example_ble_mesh_get_sensor_data(esp_ble_mesh_sensor_state_t *state, uint8_t *data)
{
    uint8_t mpid_len = 0, data_len = 0;
    uint32_t mpid = 0;

    if (state == NULL || data == NULL) {
        ESP_LOGE(TAG, "%s, Invalid parameter", __func__);
        return 0;
    }

    if (state->sensor_data.length == ESP_BLE_MESH_SENSOR_DATA_ZERO_LEN) {
        /* For zero-length sensor data, the length is 0x7F, and the format is Format B. */
        mpid = ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID(state->sensor_data.length, state->sensor_property_id);
        mpid_len = ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID_LEN;
        data_len = 0;
    } else {
        if (state->sensor_data.format == ESP_BLE_MESH_SENSOR_DATA_FORMAT_A) {
            mpid = ESP_BLE_MESH_SENSOR_DATA_FORMAT_A_MPID(state->sensor_data.length, state->sensor_property_id);
            mpid_len = ESP_BLE_MESH_SENSOR_DATA_FORMAT_A_MPID_LEN;
        } else {
            mpid = ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID(state->sensor_data.length, state->sensor_property_id);
            mpid_len = ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID_LEN;
        }
        /* Use "state->sensor_data.length + 1" because the length of sensor data is zero-based. */
        data_len = state->sensor_data.length + 1;
    }
     mpid=0xe00e;
     mpid_len=2;
    // net_buf_simple_add_u8(&sensor_data_0, 11);
     mpid=0xe00e;
       /*  net_buf_simple_add_u8(&sensor_data_0, 11);
         //BT_ERR("data len %d", data_len);
         net_buf_simple_add_u8(&sensor_data_0, 11);
        // BT_ERR("data len %d", data_len);
         net_buf_simple_add_u8(&sensor_data_0, 11);
        // BT_ERR("data len %d", data_len);
         net_buf_simple_add_u8(&sensor_data_0, 11);
         net_buf_simple_add_u8(&sensor_data_0, 11);
         net_buf_simple_add_u8(&sensor_data_0, 11);
         net_buf_simple_add_u8(&sensor_data_0, 11);
         net_buf_simple_add_u8(&sensor_data_0, 12);
         net_buf_simple_add_u8(&sensor_data_0, 11);

         net_buf_simple_add_u8(&sensor_data_0, 10);

                  net_buf_simple_add_u8(&sensor_data_0, 11);
                  //BT_ERR("data len %d", data_len);
                  net_buf_simple_add_u8(&sensor_data_0, 11);
                 // BT_ERR("data len %d", data_len);
                  net_buf_simple_add_u8(&sensor_data_0, 11);
                 // BT_ERR("data len %d", data_len);
                  net_buf_simple_add_u8(&sensor_data_0, 11);
                  net_buf_simple_add_u8(&sensor_data_0, 11);
                  net_buf_simple_add_u8(&sensor_data_0, 11);
                  net_buf_simple_add_u8(&sensor_data_0, 11);
                  net_buf_simple_add_u8(&sensor_data_0, 10);
                  net_buf_simple_add_u8(&sensor_data_0, 11);
                  net_buf_simple_add_u8(&sensor_data_0, 11);
                       mpid=0xe00e;
                           net_buf_simple_add_u8(&sensor_data_0, 11);
                           //BT_ERR("data len %d", data_len);
                           net_buf_simple_add_u8(&sensor_data_0, 11);
                          // BT_ERR("data len %d", data_len);
                           net_buf_simple_add_u8(&sensor_data_0, 11);
                          // BT_ERR("data len %d", data_len);
                           net_buf_simple_add_u8(&sensor_data_0, 11);
                           net_buf_simple_add_u8(&sensor_data_0, 11);
                           net_buf_simple_add_u8(&sensor_data_0, 11);
                           net_buf_simple_add_u8(&sensor_data_0, 11);
                           net_buf_simple_add_u8(&sensor_data_0, 12);
                           net_buf_simple_add_u8(&sensor_data_0, 11);*/
     //uint8_t datas[10]={1,1,1,1};
     data_len=29;
     BT_ERR("data len %d", data_len);
    memcpy(data, &mpid, mpid_len);
    memcpy(data + mpid_len,state->sensor_data.raw_value->data, data_len);

    return (mpid_len + data_len);
}

static void example_ble_mesh_send_sensor_status(/*int a*/esp_ble_mesh_sensor_server_cb_param_t *param)
{
    uint8_t *status = NULL;
    uint16_t buf_size = 0;
    uint16_t length = 0;
    uint32_t mpid = 0;
    esp_err_t err;
    int i;

    /**
     * Sensor Data state from Mesh Model Spec
     * |--------Field--------|-Size (octets)-|------------------------Notes-------------------------|
     * |----Property ID 1----|-------2-------|--ID of the 1st device property of the sensor---------|
     * |-----Raw Value 1-----|----variable---|--Raw Value field defined by the 1st device property--|
     * |----Property ID 2----|-------2-------|--ID of the 2nd device property of the sensor---------|
     * |-----Raw Value 2-----|----variable---|--Raw Value field defined by the 2nd device property--|
     * | ...... |
     * |----Property ID n----|-------2-------|--ID of the nth device property of the sensor---------|
     * |-----Raw Value n-----|----variable---|--Raw Value field defined by the nth device property--|
     */
    for (i = 0; i < ARRAY_SIZE(sensor_states); i++) {
        esp_ble_mesh_sensor_state_t *state = &sensor_states[i];
        if (state->sensor_data.length == ESP_BLE_MESH_SENSOR_DATA_ZERO_LEN) {
            buf_size += ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID_LEN;
        } else {
            /* Use "state->sensor_data.length + 1" because the length of sensor data is zero-based. */
            if (state->sensor_data.format == ESP_BLE_MESH_SENSOR_DATA_FORMAT_A) {
                buf_size += ESP_BLE_MESH_SENSOR_DATA_FORMAT_A_MPID_LEN + state->sensor_data.length + 1;
            } else {
                buf_size += ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID_LEN + state->sensor_data.length + 1;
            }
        }
    }

   // status = calloc(1, buf_size);
    status = calloc(1, 70);
    BT_ERR("buff size %d", buf_size);
    if (!status) {
        ESP_LOGE(TAG, "No memory for sensor status!");
        return;
    }

//    if (param->value.get.sensor_data.op_en == false) {
        /* Mesh Model Spec:
         * If the message is sent as a response to the Sensor Get message, and if the
         * Property ID field of the incoming message is omitted, the Marshalled Sensor
         * Data field shall contain data for all device properties within a sensor.
         */
        for (i = 0; i < ARRAY_SIZE(sensor_states); i++) {
            length += example_ble_mesh_get_sensor_data(&sensor_states[i], status + length);
        }
        goto send;
//    }

    /* Mesh Model Spec:
     * Otherwise, the Marshalled Sensor Data field shall contain data for the requested
     * device property only.
     */
/*    for (i = 0; i < ARRAY_SIZE(sensor_states); i++) {
        if (param->value.get.sensor_data.property_id == sensor_states[i].sensor_property_id) {
            length = example_ble_mesh_get_sensor_data(&sensor_states[i], status);
            goto send;
        }
    }*/

    /* Mesh Model Spec:
     * Or the Length shall represent the value of zero and the Raw Value field shall
     * contain only the Property ID if the requested device property is not recognized
     * by the Sensor Server.
     */
   /* mpid = ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID(ESP_BLE_MESH_SENSOR_DATA_ZERO_LEN,
            param->value.get.sensor_data.property_id);
    memcpy(status, &mpid, ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID_LEN);
    length = ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID_LEN;*/

send:
    ESP_LOG_BUFFER_HEX("Sensor Data", status, length);
  //  sensor_server.model->pub->publish_addr=0x0318;
   //     sensor_server.model->pub->app_idx=0000;
        ESP_LOGE(TAG, "Node pub addr 0x%04x ", sensor_server.model->pub->publish_addr);
        ESP_LOGE(TAG, "Node ap idx addr 0x%04x ", sensor_server.model->pub->app_idx);
    err = esp_ble_mesh_model_publish(sensor_server.model, ESP_BLE_MESH_MODEL_OP_SENSOR_STATUS, length, status, ROLE_NODE);
    /*err = esp_ble_mesh_server_model_send_msg(param->model, &param->ctx,
    		ESP_BLE_MESH_MODEL_OP_SENSOR_STATUS, length, status);*/
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Sensor Status");
    }
    free(status);
}

static void example_ble_mesh_send_sensor_column_status(esp_ble_mesh_sensor_server_cb_param_t *param)
{
    uint8_t *status = NULL;
    uint16_t length = 0;
    esp_err_t err;

    length = ESP_BLE_MESH_SENSOR_PROPERTY_ID_LEN +param->value.get.sensor_column.raw_value_x->len;

    status = calloc(1, length);
    if (!status) {
        ESP_LOGE(TAG, "No memory for sensor column status!");
        return;
    }

    memcpy(status, &param->value.get.sensor_column.property_id, ESP_BLE_MESH_SENSOR_PROPERTY_ID_LEN);
    memcpy(status + ESP_BLE_MESH_SENSOR_PROPERTY_ID_LEN, param->value.get.sensor_column.raw_value_x->data,
        param->value.get.sensor_column.raw_value_x->len);

    err = esp_ble_mesh_server_model_send_msg(param->model, &param->ctx,
            ESP_BLE_MESH_MODEL_OP_SENSOR_COLUMN_STATUS, length, status);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Sensor Column Status");
    }
    free(status);
}

static void example_ble_mesh_send_sensor_series_status(esp_ble_mesh_sensor_server_cb_param_t *param)
{
    esp_err_t err;

    err = esp_ble_mesh_server_model_send_msg(param->model, &param->ctx,
            ESP_BLE_MESH_MODEL_OP_SENSOR_SERIES_STATUS,
            ESP_BLE_MESH_SENSOR_PROPERTY_ID_LEN,
            (uint8_t *)&param->value.get.sensor_series.property_id);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send Sensor Column Status");
    }
}

void example_ble_mesh_send_sensor_message(uint32_t opcode)
{
    esp_ble_mesh_sensor_client_get_state_t get = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_node_t *node = NULL;
    esp_err_t err = ESP_OK;

    node = esp_ble_mesh_provisioner_get_node_with_addr(server_address);
    if (node == NULL) {
        ESP_LOGE(TAG, "Node 0x%04x not exists", server_address);
        return;
    }

    example_ble_mesh_set_msg_common(&common, node, sensor_client.model, opcode);
    switch (opcode) {
    case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_GET:
        get.cadence_get.property_id = sensor_prop_id;
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTINGS_GET:
        get.settings_get.sensor_property_id = sensor_prop_id;
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_SERIES_GET:
        get.series_get.property_id = sensor_prop_id;
        break;
    default:
        break;
    }

    err = esp_ble_mesh_sensor_client_get_state(&common, &get);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send sensor message 0x%04" PRIx32, opcode);
    }
}

static void example_ble_mesh_sensor_timeout(uint32_t opcode)
{
    switch (opcode) {
    case ESP_BLE_MESH_MODEL_OP_SENSOR_DESCRIPTOR_GET:
        ESP_LOGW(TAG, "Sensor Descriptor Get timeout, opcode 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_GET:
        ESP_LOGW(TAG, "Sensor Cadence Get timeout, opcode 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_SET:
        ESP_LOGW(TAG, "Sensor Cadence Set timeout, opcode 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTINGS_GET:
        ESP_LOGW(TAG, "Sensor Settings Get timeout, opcode 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_GET:
        ESP_LOGW(TAG, "Sensor Setting Get timeout, opcode 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_SET:
        ESP_LOGW(TAG, "Sensor Setting Set timeout, opcode 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_GET:
        ESP_LOGW(TAG, "Sensor Get timeout, 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_COLUMN_GET:
        ESP_LOGW(TAG, "Sensor Column Get timeout, opcode 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_SERIES_GET:
        ESP_LOGW(TAG, "Sensor Series Get timeout, opcode 0x%04" PRIx32, opcode);
        break;
    default:
        ESP_LOGE(TAG, "Unknown Sensor Get/Set opcode 0x%04" PRIx32, opcode);
        return;
    }

    example_ble_mesh_send_sensor_message(opcode);
}
int configured=0;
static void example_ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
                                              esp_ble_mesh_cfg_server_cb_param_t *param)
{
    if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT) {
        switch (param->ctx.recv_op) {
        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD");
            ESP_LOGI(TAG, "net_idx 0x%04x, app_idx 0x%04x",
                param->value.state_change.appkey_add.net_idx,
                param->value.state_change.appkey_add.app_idx);
            ESP_LOG_BUFFER_HEX("AppKey", param->value.state_change.appkey_add.app_key, 16);
            //configured=1;

            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND");
            ESP_LOGI(TAG, "elemm_addr 0x%04x, app_idx 0x%04x, cid 0x%04x, mod_id 0x%04x",
                param->value.state_change.mod_app_bind.element_addr,
                param->value.state_change.mod_app_bind.app_idx,
                param->value.state_change.mod_app_bind.company_id,
                param->value.state_change.mod_app_bind.model_id);
           // binded=1;
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD");
            ESP_LOGI(TAG, "elem_addr 0x%04x, sub_addr 0x%04x, cid 0x%04x, mod_id 0x%04x",
                param->value.state_change.mod_sub_add.element_addr,
                param->value.state_change.mod_sub_add.sub_addr,
                param->value.state_change.mod_sub_add.company_id,
                param->value.state_change.mod_sub_add.model_id);
            prov=1;
            configured=1;
            //configured=1;
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET:
			ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_SUB_ADD");
			ESP_LOGI(TAG, " pub_addr 0x%04x,elem_addr 0x%04x, cid 0x%04x, mod_id 0x%04x",
				param->value.state_change.mod_pub_set.pub_addr,
				param->value.state_change.mod_sub_add.element_addr,
				param->value.state_change.mod_sub_add.company_id,
				param->value.state_change.mod_sub_add.model_id);
			  //  esp_ble_mesh_cfg_server_cb_param_t params;
				//example_ble_mesh_send_sensor_status(&params);
				//configured=1;

            break;
        default:
            break;
        }
    }
}

static void example_ble_mesh_sensor_server_cb(esp_ble_mesh_sensor_server_cb_event_t event,
                                              esp_ble_mesh_sensor_server_cb_param_t *param)
{
    ESP_LOGI(TAG, "Sensor server, event %d, src 0x%04x, dst 0x%04x, model_id 0x%04x",
        event, param->ctx.addr, param->ctx.recv_dst, param->model->model_id);
    ESP_LOGI(TAG, "Sensor server, event %d, src 0x%04x, dst 0x%04x, model_id 0x%04x",
            event, param->ctx.addr, param->ctx.recv_dst, param->model->model_id);
  /*  ESP_LOGI(TAG, "Sensor server, event %d, src 0x%04x, dst 0x%04x, model_id 0x%04x",
            event, param->ctx.addr, param->ctx.recv_dst, param->model->model_id);
    ESP_LOGI(TAG, "Sensor server, event %d, src 0x%04x, dst 0x%04x, model_id 0x%04x",
            event, param->ctx.addr, param->ctx.recv_dst, param->model->model_id);*/

    switch (event) {
    case ESP_BLE_MESH_SENSOR_SERVER_RECV_GET_MSG_EVT:
        switch (param->ctx.recv_op) {
        case ESP_BLE_MESH_MODEL_OP_SENSOR_DESCRIPTOR_GET:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_DESCRIPTOR_GET");
            example_ble_mesh_send_sensor_descriptor_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_GET:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_GET");
            example_ble_mesh_send_sensor_cadence_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTINGS_GET:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_SETTINGS_GET");
            example_ble_mesh_send_sensor_settings_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_GET:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_SETTINGS_GET");
            example_ble_mesh_send_sensor_setting_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_GET:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_GET");
            example_ble_mesh_send_sensor_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_COLUMN_GET:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_COLUMN_GET");
            example_ble_mesh_send_sensor_column_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SERIES_GET:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_SERIES_GET");
            example_ble_mesh_send_sensor_series_status(param);
            break;
        default:
            ESP_LOGE(TAG, "Unknown Sensor Get opcode 0x%04" PRIx32, param->ctx.recv_op);
            return;
        }
        break;
    case ESP_BLE_MESH_SENSOR_SERVER_RECV_SET_MSG_EVT:
        switch (param->ctx.recv_op) {
        case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_SET:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_SET");
            example_ble_mesh_send_sensor_cadence_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_SET_UNACK:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_SET_UNACK");
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_SET:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_SET");
            example_ble_mesh_send_sensor_setting_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_SET_UNACK:
            ESP_LOGI(TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_SET_UNACK");
            break;
        default:
            ESP_LOGE(TAG, "Unknown Sensor Set opcode 0x%04" PRIx32, param->ctx.recv_op);
            break;
        }
        break;
    default:
        ESP_LOGE(TAG, "Unknown Sensor Server event %d", event);
        break;
    }
}

static void example_ble_mesh_sensor_client_cb(esp_ble_mesh_sensor_client_cb_event_t event,
                                              esp_ble_mesh_sensor_client_cb_param_t *param)
{
    esp_ble_mesh_node_t *node = NULL;

    ESP_LOGI(TAG, "Sensor client, event %u, addr 0x%04x", event, param->params->ctx.addr);

    if (param->error_code) {
        ESP_LOGE(TAG, "Send sensor client message failed (err %d)", param->error_code);
        return;
    }

   /* node = esp_ble_mesh_provisioner_get_node_with_addr(param->params->ctx.addr);
    if (!node) {
        ESP_LOGE(TAG, "Node 0x%04x not exists", param->params->ctx.addr);
        return;
    }*/
    //if(ESP_BLE_MESH_MODEL_OP_SENSOR_STATUS==param->params->opcode)
    {
    	ESP_LOG_BUFFER_HEX("Sensor Data", param->status_cb.sensor_status.marshalled_sensor_data->data,
    	                    param->status_cb.sensor_status.marshalled_sensor_data->len);

    }
    switch (event) {
    case ESP_BLE_MESH_SENSOR_CLIENT_GET_STATE_EVT:
        switch (param->params->opcode) {
        case ESP_BLE_MESH_MODEL_OP_SENSOR_DESCRIPTOR_GET:
            ESP_LOGI(TAG, "Sensor Descriptor Status, opcode 0x%04" PRIx32, param->params->ctx.recv_op);
            if (param->status_cb.descriptor_status.descriptor->len != ESP_BLE_MESH_SENSOR_SETTING_PROPERTY_ID_LEN &&
                param->status_cb.descriptor_status.descriptor->len % ESP_BLE_MESH_SENSOR_DESCRIPTOR_LEN) {
                ESP_LOGE(TAG, "Invalid Sensor Descriptor Status length %d", param->status_cb.descriptor_status.descriptor->len);
                return;
            }
            if (param->status_cb.descriptor_status.descriptor->len) {
                ESP_LOG_BUFFER_HEX("Sensor Descriptor", param->status_cb.descriptor_status.descriptor->data,
                    param->status_cb.descriptor_status.descriptor->len);
                /* If running with sensor server example, sensor client can get two Sensor Property IDs.
                 * Currently we use the first Sensor Property ID for the following demonstration.
                 */
                sensor_prop_id = param->status_cb.descriptor_status.descriptor->data[1] << 8 |
                                 param->status_cb.descriptor_status.descriptor->data[0];
            }
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_GET:
            ESP_LOGI(TAG, "Sensor Cadence Status, opcode 0x%04" PRIx32 ", Sensor Property ID 0x%04x",
                param->params->ctx.recv_op, param->status_cb.cadence_status.property_id);
            ESP_LOG_BUFFER_HEX("Sensor Cadence", param->status_cb.cadence_status.sensor_cadence_value->data,
                param->status_cb.cadence_status.sensor_cadence_value->len);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTINGS_GET:
            ESP_LOGI(TAG, "Sensor Settings Status, opcode 0x%04" PRIx32 ", Sensor Property ID 0x%04x",
                param->params->ctx.recv_op, param->status_cb.settings_status.sensor_property_id);
            ESP_LOG_BUFFER_HEX("Sensor Settings", param->status_cb.settings_status.sensor_setting_property_ids->data,
                param->status_cb.settings_status.sensor_setting_property_ids->len);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_GET:
            ESP_LOGI(TAG, "Sensor Setting Status, opcode 0x%04" PRIx32 ", Sensor Property ID 0x%04x, Sensor Setting Property ID 0x%04x",
                param->params->ctx.recv_op, param->status_cb.setting_status.sensor_property_id,
                param->status_cb.setting_status.sensor_setting_property_id);
            if (param->status_cb.setting_status.op_en) {
                ESP_LOGI(TAG, "Sensor Setting Access 0x%02x", param->status_cb.setting_status.sensor_setting_access);
                ESP_LOG_BUFFER_HEX("Sensor Setting Raw", param->status_cb.setting_status.sensor_setting_raw->data,
                    param->status_cb.setting_status.sensor_setting_raw->len);
            }
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_GET:
            ESP_LOGI(TAG, "Sensor Status, opcode 0x%04" PRIx32, param->params->ctx.recv_op);
            if (param->status_cb.sensor_status.marshalled_sensor_data->len) {
                ESP_LOG_BUFFER_HEX("Sensor Data", param->status_cb.sensor_status.marshalled_sensor_data->data,
                    param->status_cb.sensor_status.marshalled_sensor_data->len);
                uint8_t *data = param->status_cb.sensor_status.marshalled_sensor_data->data;
                uint16_t length = 0;
                for (; length < param->status_cb.sensor_status.marshalled_sensor_data->len; ) {
                    uint8_t fmt = ESP_BLE_MESH_GET_SENSOR_DATA_FORMAT(data);
                    uint8_t data_len = ESP_BLE_MESH_GET_SENSOR_DATA_LENGTH(data, fmt);
                    uint16_t prop_id = ESP_BLE_MESH_GET_SENSOR_DATA_PROPERTY_ID(data, fmt);
                    uint8_t mpid_len = (fmt == ESP_BLE_MESH_SENSOR_DATA_FORMAT_A ?
                                        ESP_BLE_MESH_SENSOR_DATA_FORMAT_A_MPID_LEN : ESP_BLE_MESH_SENSOR_DATA_FORMAT_B_MPID_LEN);
                    ESP_LOGI(TAG, "Format %s, length 0x%02x, Sensor Property ID 0x%04x",
                        fmt == ESP_BLE_MESH_SENSOR_DATA_FORMAT_A ? "A" : "B", data_len, prop_id);
                    if (data_len != ESP_BLE_MESH_SENSOR_DATA_ZERO_LEN) {
                        ESP_LOG_BUFFER_HEX("Sensor Data", data + mpid_len, data_len + 1);
                        length += mpid_len + data_len + 1;
                        data += mpid_len + data_len + 1;
                    } else {
                        length += mpid_len;
                        data += mpid_len;
                    }
                }
            }
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_COLUMN_GET:
            ESP_LOGI(TAG, "Sensor Column Status, opcode 0x%04" PRIx32 ", Sensor Property ID 0x%04x",
                param->params->ctx.recv_op, param->status_cb.column_status.property_id);
            ESP_LOG_BUFFER_HEX("Sensor Column", param->status_cb.column_status.sensor_column_value->data,
                param->status_cb.column_status.sensor_column_value->len);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SERIES_GET:
            ESP_LOGI(TAG, "Sensor Series Status, opcode 0x%04" PRIx32 ", Sensor Property ID 0x%04x",
                param->params->ctx.recv_op, param->status_cb.series_status.property_id);
            ESP_LOG_BUFFER_HEX("Sensor Series", param->status_cb.series_status.sensor_series_value->data,
                param->status_cb.series_status.sensor_series_value->len);
            break;
        default:
            ESP_LOGE(TAG, "Unknown Sensor Get opcode 0x%04" PRIx32, param->params->ctx.recv_op);
            break;
        }
        break;
    case ESP_BLE_MESH_SENSOR_CLIENT_SET_STATE_EVT:
        switch (param->params->opcode) {
        case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_SET:
            ESP_LOGI(TAG, "Sensor Cadence Status, opcode 0x%04" PRIx32 ", Sensor Property ID 0x%04x",
                param->params->ctx.recv_op, param->status_cb.cadence_status.property_id);
            ESP_LOG_BUFFER_HEX("Sensor Cadence", param->status_cb.cadence_status.sensor_cadence_value->data,
                param->status_cb.cadence_status.sensor_cadence_value->len);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_SET:
            ESP_LOGI(TAG, "Sensor Setting Status, opcode 0x%04" PRIx32 ", Sensor Property ID 0x%04x, Sensor Setting Property ID 0x%04x",
                param->params->ctx.recv_op, param->status_cb.setting_status.sensor_property_id,
                param->status_cb.setting_status.sensor_setting_property_id);
            if (param->status_cb.setting_status.op_en) {
                ESP_LOGI(TAG, "Sensor Setting Access 0x%02x", param->status_cb.setting_status.sensor_setting_access);
                ESP_LOG_BUFFER_HEX("Sensor Setting Raw", param->status_cb.setting_status.sensor_setting_raw->data,
                    param->status_cb.setting_status.sensor_setting_raw->len);
            }
            break;
        default:
            ESP_LOGE(TAG, "Unknown Sensor Set opcode 0x%04" PRIx32, param->params->ctx.recv_op);
            break;
        }
        break;
    case ESP_BLE_MESH_SENSOR_CLIENT_PUBLISH_EVT:
        break;
    case ESP_BLE_MESH_SENSOR_CLIENT_TIMEOUT_EVT:
        example_ble_mesh_sensor_timeout(param->params->opcode);
    default:
        break;
    }
}

static esp_err_t ble_mesh_init(void)
{
    esp_err_t err;

    esp_ble_mesh_register_prov_callback(example_ble_mesh_provisioning_cb);
    esp_ble_mesh_register_config_server_callback(example_ble_mesh_config_server_cb);
    esp_ble_mesh_register_sensor_server_callback(example_ble_mesh_sensor_server_cb);
    esp_ble_mesh_register_config_client_callback(example_ble_mesh_config_client_cb);
    esp_ble_mesh_register_sensor_client_callback(example_ble_mesh_sensor_client_cb);

    err = esp_ble_mesh_init(&provision, &composition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize mesh stack");
        return err;
    }

    err = esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable mesh node");
        return err;
    }

    //board_led_operation(LED_G, LED_ON);

    ESP_LOGI(TAG, "BLE Mesh sensor server initialized");

    return ESP_OK;
}

void mesh_main_init(void)
{
    esp_err_t err;

    ESP_LOGI(TAG, "Initializing...");

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

   // board_init();

    err = bluetooth_init();
    if (err) {
        ESP_LOGE(TAG, "esp32_bluetooth_init failed (err %d)", err);
        return;
    }

    ble_mesh_get_dev_uuid(dev_uuid);
    ESP_LOGE(TAG, "Bluetooth mac addr ");
    int i;
    for(i=0;i<16;i++){
    	ESP_LOGE(TAG, " %d :", dev_uuid[i]);
    }
    /* Initialize the Bluetooth Mesh Subsystem */
    err = ble_mesh_init();
    //net_buf_simple_add_u8(&sensor_data_0, 11);
    /*net_buf_simple_add_u8(&sensor_data_0, 11);
    net_buf_simple_add_u8(&sensor_data_0, 11);
    net_buf_simple_add_u8(&sensor_data_0, 11);
    net_buf_simple_add_u8(&sensor_data_0, 11);
    net_buf_simple_add_u8(&sensor_data_0, 11);
    net_buf_simple_add_u8(&sensor_data_0, 11);
    net_buf_simple_add_u8(&sensor_data_0, 11);
    net_buf_simple_add_u8(&sensor_data_0, 11);
    net_buf_simple_add_u8(&sensor_data_0, 11);
    net_buf_simple_add_u8(&sensor_data_0, 11);
    net_buf_simple_add_u8(&sensor_data_0, 11);*/


   // net_buf_simple_add_u8(&sensor_data_0, 11);
   // net_buf_simple_add_u8(&sensor_data_0, 11);
   // net_buf_simple_add_u8(&sensor_data_0, 11);
    //esp_ble_mesh_cfg_server_cb_param_t param;
    if (err) {
        ESP_LOGE(TAG, "Bluetooth mesh init failed (err %d)", err);

    }
    esp_ble_mesh_cfg_server_cb_param_t paramss;
    //example_ble_mesh_send_sensor_status(&params);
    sensor_server.model->pub->publish_addr=0x0333;
    //ESP_LOGE(TAG, "client appidx 0x%04x ", sensor_client.model->pub->app_idx);
    //ESP_LOGE(TAG, "Node ap idx addr 0x%04x ", sensor_server.model->pub->app_idx);

    //sensor_server.model->pub->app_idx=0000;
    sensor_client.model->pub->app_idx=0x0000;
    ESP_LOGE(TAG, "after binding client appidx 0x%04x ", sensor_client.model->pub->app_idx);
    #if 0
    while(1)
    {
    	if(gpio_get_level(15)==0)
    	{   //gpio_set_level()
    			/*net_buf_simple_add_u8(&sensor_data_0, 11);
    		    net_buf_simple_add_u8(&sensor_data_0, 11);
    		    net_buf_simple_add_u8(&sensor_data_0, 11);
    		    net_buf_simple_add_u8(&sensor_data_0, 11);
    		    net_buf_simple_add_u8(&sensor_data_0, 11);
    		    net_buf_simple_add_u8(&sensor_data_0, 11);
    		    net_buf_simple_add_u8(&sensor_data_0, 11);
    		    net_buf_simple_add_u8(&sensor_data_0, 11);
    		    net_buf_simple_add_u8(&sensor_data_0, 11);
    		    net_buf_simple_add_u8(&sensor_data_0, 11);
    		    net_buf_simple_add_u8(&sensor_data_0, 11);*/
    		for(i=0;i<28;i++){
    			sensor_states[0].sensor_data.raw_value->data[i]=0xbc;
    		}

    		ESP_LOGE(TAG, "Gpio detected");
    		example_ble_mesh_send_sensor_status(&paramss);
    		//configured=0;
    		vTaskDelay(pdMS_TO_TICKS(1000));

    	}
    	if(binded){
    		ESP_LOGE(TAG, "after binding client appidx 0x%04x ", sensor_client.model->pub->app_idx);binded=0;
    	}
    	/*if(prov){

    		err = esp_ble_mesh_provisioner_bind_app_key_to_local_model(0x0332, 0,
    		                                    ESP_BLE_MESH_MODEL_ID_SENSOR_CLI, ESP_BLE_MESH_CID_NVAL);
    			if (err != ESP_OK) {
    				ESP_LOGE(TAG, "Failed to bind AppKey to sensor client");
    			}else{
    				ESP_LOGE(TAG, "sucess to bind AppKey to sensor client");
    			}

    			prov=0;
    	}*/

    	vTaskDelay(pdMS_TO_TICKS(100));
    }
    #endif
}
