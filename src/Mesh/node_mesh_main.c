/**
 * @file mesh_main.c
 * @author Adhikesavan (Adhikesavan@qmaxsys.com)
 * @brief This file contains all ble-mesh related functions
 * @version 0.1
 * @date 2024-03-02
 * @copyright Copyright (c) 2024
 */

#include "../../inc/mesh/mesh_main.h"
#include "../../inc/Mesh/ble_mesh_example_init.h"

#include "esp_err.h"
// #include "btc_ble_mesh_prov.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_defs.h"

#include <stdio.h>
#include <string.h>
#include <sdkconfig.h>

#ifdef CONFIG_BT_BLUEDROID_ENABLED
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#endif

#ifdef CONFIG_BT_NIMBLE_ENABLED
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#endif

#if (!IS_GWY)

control_t *vendor_node_ac_control_t; /* TID contained in the vendor message */
reconf_t *vendor_node_reconfigure_t;
teaching_mode_t *vendor_node_teaching_mode_t;
pub_conf_t *vendor_node_hearbeat_pub_conf_t;
debug_info_t *vendor_node_debug_info_t;
uint8_t *BLE_recvd_data;

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_sensor_model_api.h"

// Initialization
uint16_t ELEMENT_ADDR = 0;
bool provisioned = false;

#define CID_ESP 0x02E5

#define PROV_OWN_ADDR 0x0001

#define MSG_SEND_TTL 3
#define MSG_SEND_REL false
#define MSG_TIMEOUT 0
#define MSG_ROLE ROLE_PROVISIONER

#define COMP_DATA_PAGE_0 0x00

#define APP_KEY_IDX 0x0000
#define APP_KEY_OCTET 0x12

#define COMP_DATA_1_OCTET(msg, offset) (msg[offset])
#define COMP_DATA_2_OCTET(msg, offset) (msg[offset + 1] << 8 | msg[offset])

/* Sensor Property ID */
#define SENSOR_PROPERTY_ID_0 0x0056 /* Present Indoor Ambient Temperature */
#define SENSOR_PROPERTY_ID_1 0x005B /* Present Outdoor Ambient Temperature */

/* The characteristic of the two device properties is "Temperature 8", which is
 * used to represent a measure of temperature with a unit of 0.5 degree Celsius.
 * Minimum value: -64.0, maximum value: 63.5.
 * A value of 0xFF represents 'value is not known'.
 */
static int8_t indoor_temp = 40;  /* Indoor temperature is 20 Degrees Celsius */
static int8_t outdoor_temp = 60; /* Outdoor temperature is 30 Degrees Celsius */

#define SENSOR_POSITIVE_TOLERANCE ESP_BLE_MESH_SENSOR_UNSPECIFIED_POS_TOLERANCE
#define SENSOR_NEGATIVE_TOLERANCE ESP_BLE_MESH_SENSOR_UNSPECIFIED_NEG_TOLERANCE
#define SENSOR_SAMPLE_FUNCTION ESP_BLE_MESH_SAMPLE_FUNC_UNSPECIFIED
#define SENSOR_MEASURE_PERIOD ESP_BLE_MESH_SENSOR_NOT_APPL_MEASURE_PERIOD
#define SENSOR_UPDATE_INTERVAL ESP_BLE_MESH_SENSOR_NOT_APPL_UPDATE_INTERVAL

static uint8_t dev_uuid[ESP_BLE_MESH_OCTET16_LEN] = {0xcd, 0xdc};
static uint16_t server_address = ESP_BLE_MESH_ADDR_UNASSIGNED;
static uint16_t sensor_prop_id;

uint8_t binded = 0, prov = 0;

static struct esp_ble_mesh_key
{
    uint16_t net_idx;
    uint16_t app_idx;
    uint8_t app_key[ESP_BLE_MESH_OCTET16_LEN];
} prov_key;

typedef struct node_struct
{
    uint16_t msg_seq_no;
    uint16_t gwy_ser_no;
    uint16_t node_ser_no;
    uint8_t element_address;
    // char location[LOCATION_STR_LEN];
} node_details_t;

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

    ESP_LOGI(MESH_DEBUG_TAG, "********************** Composition Data Start **********************");
    ESP_LOGI(MESH_DEBUG_TAG, "* CID 0x%04x, PID 0x%04x, VID 0x%04x, CRPL 0x%04x, Features 0x%04x *", cid, pid, vid, crpl, feat);
    for (; offset < length;)
    {
        loc = COMP_DATA_2_OCTET(data, offset);
        nums = COMP_DATA_1_OCTET(data, offset + 2);
        numv = COMP_DATA_1_OCTET(data, offset + 3);
        offset += 4;
        ESP_LOGI(MESH_DEBUG_TAG, "* Loc 0x%04x, NumS 0x%02x, NumV 0x%02x *", loc, nums, numv);
        for (i = 0; i < nums; i++)
        {
            model_id = COMP_DATA_2_OCTET(data, offset);
            ESP_LOGI(MESH_DEBUG_TAG, "* SIG Model ID 0x%04x *", model_id);
            offset += 2;
        }
        for (i = 0; i < numv; i++)
        {
            company_id = COMP_DATA_2_OCTET(data, offset);
            model_id = COMP_DATA_2_OCTET(data, offset + 2);
            ESP_LOGI(MESH_DEBUG_TAG, "* Vendor Model ID 0x%04x, Company ID 0x%04x *", model_id, company_id);
            offset += 4;
        }
    }
    ESP_LOGI(MESH_DEBUG_TAG, "*********************** Composition Data End ***********************");
}

struct example_sensor_descriptor
{
    uint16_t sensor_prop_id;
    uint32_t pos_tolerance : 12,
        neg_tolerance : 12,
        sample_func : 8;
    uint8_t measure_period;
    uint8_t update_interval;
} __attribute__((packed));

static void example_ble_mesh_send_sensor_descriptor_status(esp_ble_mesh_sensor_server_cb_param_t *param)
{
    struct example_sensor_descriptor descriptor = {0};
    uint8_t *status = NULL;
    uint16_t length = 0;
    esp_err_t err;
    int i;

    status = calloc(1, ARRAY_SIZE(sensor_states) * ESP_BLE_MESH_SENSOR_DESCRIPTOR_LEN);
    if (!status)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "No memory for sensor descriptor status!");
        return;
    }

    if (param->value.get.sensor_descriptor.op_en == false)
    {
        /* Mesh Model Spec:
         * Upon receiving a Sensor Descriptor Get message with the Property ID field
         * omitted, the Sensor Server shall respond with a Sensor Descriptor Status
         * message containing the Sensor Descriptor states for all sensors within the
         * Sensor Server.
         */
        for (i = 0; i < ARRAY_SIZE(sensor_states); i++)
        {
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

    for (i = 0; i < ARRAY_SIZE(sensor_states); i++)
    {
        if (param->value.get.sensor_descriptor.property_id == sensor_states[i].sensor_property_id)
        {
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
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "Failed to send Sensor Descriptor Status");
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
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "Failed to send Sensor Cadence Status");
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
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "Failed to send Sensor Settings Status");
    }
}

struct example_sensor_setting
{
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
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "Failed to send Sensor Setting Status");
    }
}

static uint16_t example_ble_mesh_get_sensor_data(esp_ble_mesh_sensor_state_t *state, uint8_t *data)
{
    uint8_t mpid_len = 0, data_len = 0;
    uint32_t mpid = 0;

    if (state == NULL || data == NULL)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "%s, Invalid parameter", __func__);
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
    // mpid=0xe00e;
    mpid_len = 2;
    // net_buf_simple_add_u8(&sensor_data_0, 11);
    // mpid=0xe00e;

    data_len = 29;
    BT_ERR("data len %d", data_len);
    // memcpy(data, &mpid, mpid_len);
    memcpy(data, state->sensor_data.raw_value->data, data_len);

    return (data_len);
}

static void example_ble_mesh_send_sensor_status(/*int aesp_ble_mesh_sensor_server_cb_param_t *param*/)
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

    // status = calloc(1, buf_size);
    status = calloc(1, 70);
    BT_ERR("buff size %d", buf_size);
    if (!status)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "No memory for sensor status!");
        return;
    }

    for (i = 0; i < ARRAY_SIZE(sensor_states); i++)
    {
        length += example_ble_mesh_get_sensor_data(&sensor_states[i], status + length);
    }
    goto send;

send:
    ESP_LOG_BUFFER_HEX("Sensor Data", status, length);
    sensor_server.model->pub->publish_addr = 0x01;
    ESP_LOGI(MESH_DEBUG_TAG, "Node pub addr 0x%04x ", sensor_server.model->pub->publish_addr);
    err = esp_ble_mesh_model_publish(sensor_server.model, ESP_BLE_MESH_MODEL_OP_SENSOR_STATUS, length, status, ROLE_NODE);
    /* esp_ble_mesh_msg_ctx_t cntx;
     cntx.net_idx=0;
     cntx.app_idx=0;
     cntx.addr=0001;
     cntx.send_rel=1;
     cntx.send_ttl=2;
     cntx.srv_send=true;*/

    free(status);
}

static void example_ble_mesh_send_sensor_column_status(esp_ble_mesh_sensor_server_cb_param_t *param)
{
    uint8_t *status = NULL;
    uint16_t length = 0;
    esp_err_t err;

    length = ESP_BLE_MESH_SENSOR_PROPERTY_ID_LEN + param->value.get.sensor_column.raw_value_x->len;

    status = calloc(1, length);
    if (!status)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "No memory for sensor column status!");
        return;
    }

    memcpy(status, &param->value.get.sensor_column.property_id, ESP_BLE_MESH_SENSOR_PROPERTY_ID_LEN);
    memcpy(status + ESP_BLE_MESH_SENSOR_PROPERTY_ID_LEN, param->value.get.sensor_column.raw_value_x->data,
           param->value.get.sensor_column.raw_value_x->len);

    err = esp_ble_mesh_server_model_send_msg(param->model, &param->ctx,
                                             ESP_BLE_MESH_MODEL_OP_SENSOR_COLUMN_STATUS, length, status);
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "Failed to send Sensor Column Status");
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
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "Failed to send Sensor Column Status");
    }
}

void example_ble_mesh_send_sensor_message(uint32_t opcode)
{
    esp_ble_mesh_sensor_client_get_state_t get = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_node_t *node = NULL;
    esp_err_t err = ESP_OK;

    node = esp_ble_mesh_provisioner_get_node_with_addr(server_address);
    if (node == NULL)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "Node 0x%04x not exists", server_address);
        return;
    }

    example_ble_mesh_set_msg_common(&common, node, sensor_client.model, opcode);
    switch (opcode)
    {
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
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "Failed to send sensor message 0x%04" PRIx32, opcode);
    }
}

static void example_ble_mesh_sensor_timeout(uint32_t opcode)
{
    switch (opcode)
    {
    case ESP_BLE_MESH_MODEL_OP_SENSOR_DESCRIPTOR_GET:
        ESP_LOGW(MESH_DEBUG_TAG, "Sensor Descriptor Get timeout, opcode 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_GET:
        ESP_LOGW(MESH_DEBUG_TAG, "Sensor Cadence Get timeout, opcode 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_SET:
        ESP_LOGW(MESH_DEBUG_TAG, "Sensor Cadence Set timeout, opcode 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTINGS_GET:
        ESP_LOGW(MESH_DEBUG_TAG, "Sensor Settings Get timeout, opcode 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_GET:
        ESP_LOGW(MESH_DEBUG_TAG, "Sensor Setting Get timeout, opcode 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_SET:
        ESP_LOGW(MESH_DEBUG_TAG, "Sensor Setting Set timeout, opcode 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_GET:
        ESP_LOGW(MESH_DEBUG_TAG, "Sensor Get timeout, 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_COLUMN_GET:
        ESP_LOGW(MESH_DEBUG_TAG, "Sensor Column Get timeout, opcode 0x%04" PRIx32, opcode);
        break;
    case ESP_BLE_MESH_MODEL_OP_SENSOR_SERIES_GET:
        ESP_LOGW(MESH_DEBUG_TAG, "Sensor Series Get timeout, opcode 0x%04" PRIx32, opcode);
        break;
    default:
        ESP_LOGE(MESH_DEBUG_TAG, "Unknown Sensor Get/Set opcode 0x%04" PRIx32, opcode);
        return;
    }

    example_ble_mesh_send_sensor_message(opcode);
}

static void example_ble_mesh_sensor_server_cb(esp_ble_mesh_sensor_server_cb_event_t event,
                                              esp_ble_mesh_sensor_server_cb_param_t *param)
{
    ESP_LOGI(MESH_DEBUG_TAG, "Sensor server, event %d, src 0x%04x, dst 0x%04x, model_id 0x%04x",
             event, param->ctx.addr, param->ctx.recv_dst, param->model->model_id);
    ESP_LOGI(MESH_DEBUG_TAG, "Sensor server, event %d, src 0x%04x, dst 0x%04x, model_id 0x%04x",
             event, param->ctx.addr, param->ctx.recv_dst, param->model->model_id);

    switch (event)
    {
    case ESP_BLE_MESH_SENSOR_SERVER_RECV_GET_MSG_EVT:
        switch (param->ctx.recv_op)
        {
        case ESP_BLE_MESH_MODEL_OP_SENSOR_DESCRIPTOR_GET:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_DESCRIPTOR_GET");
            example_ble_mesh_send_sensor_descriptor_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_GET:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_GET");
            example_ble_mesh_send_sensor_cadence_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTINGS_GET:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_SETTINGS_GET");
            example_ble_mesh_send_sensor_settings_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_GET:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_SETTINGS_GET");
            example_ble_mesh_send_sensor_setting_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_GET:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_GET");
            example_ble_mesh_send_sensor_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_COLUMN_GET:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_COLUMN_GET");
            example_ble_mesh_send_sensor_column_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SERIES_GET:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_SERIES_GET");
            example_ble_mesh_send_sensor_series_status(param);
            break;
        default:
            ESP_LOGE(MESH_DEBUG_TAG, "Unknown Sensor Get opcode 0x%04" PRIx32, param->ctx.recv_op);
            return;
        }
        break;
    case ESP_BLE_MESH_SENSOR_SERVER_RECV_SET_MSG_EVT:
        switch (param->ctx.recv_op)
        {
        case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_SET:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_SET");
            example_ble_mesh_send_sensor_cadence_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_SET_UNACK:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_CADENCE_SET_UNACK");
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_SET:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_SET");
            example_ble_mesh_send_sensor_setting_status(param);
            break;
        case ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_SET_UNACK:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_SENSOR_SETTING_SET_UNACK");
            break;
        default:
            ESP_LOGE(MESH_DEBUG_TAG, "Unknown Sensor Set opcode 0x%04" PRIx32, param->ctx.recv_op);
            break;
        }
        break;
    default:
        ESP_LOGE(MESH_DEBUG_TAG, "Unknown Sensor Server event %d", event);
        break;
    }
}

#ifdef CONFIG_BT_BLUEDROID_ENABLED

void ble_mesh_get_dev_uuid(uint8_t *dev_uuid)
{
    if (dev_uuid == NULL)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "%s, Invalid device uuid", __func__);
        return;
    }

    /* Copy device address to the device uuid with offset equals to 2 here.
     * The first two bytes is used for matching device uuid by Provisioner.
     * And using device address here is to avoid using the same device uuid
     * by different unprovisioned devices.
     */
    memcpy(dev_uuid + 2, esp_bt_dev_get_address(), BD_ADDR_LEN);
}

esp_err_t bluetooth_init(void)
{
    esp_err_t ret;

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "%s initialize controller failed", __func__);
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "%s enable controller failed", __func__);
        return ret;
    }
    ret = esp_bluedroid_init();
    if (ret)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "%s init bluetooth failed", __func__);
        return ret;
    }
    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "%s enable bluetooth failed", __func__);
        return ret;
    }

    return ret;
}
#endif /* CONFIG_BT_BLUEDROID_ENABLED */
// void ble_mesh_get_dev_uuid(uint8_t *dev_uuid);

// esp_err_t bluetooth_init(void);

#define ESP_BLE_MESH_VND_MODEL_ID_CLIENT 0x0000
#define ESP_BLE_MESH_VND_MODEL_ID_SERVER 0x0001

#define ESP_BLE_MESH_VND_MODEL_OP_SEND ESP_BLE_MESH_MODEL_OP_3(0x00, CID_ESP)
#define ESP_BLE_MESH_VND_MODEL_OP_STATUS ESP_BLE_MESH_MODEL_OP_3(0x01, CID_ESP)

static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_SENSOR_SRV(&sensor_pub, &sensor_server),
    ESP_BLE_MESH_MODEL_SENSOR_SETUP_SRV(&sensor_setup_pub, &sensor_setup_server),
};

static esp_ble_mesh_model_op_t vnd_op[] = {
    ESP_BLE_MESH_MODEL_OP(ESP_BLE_MESH_VND_MODEL_OP_SEND, 2),
    ESP_BLE_MESH_MODEL_OP_END,
};

static esp_ble_mesh_model_t vnd_models[] = {
    ESP_BLE_MESH_VENDOR_MODEL(CID_ESP, ESP_BLE_MESH_VND_MODEL_ID_SERVER,
                              vnd_op, NULL, NULL),
};

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
    .output_size = 0,
    .output_actions = 0,
};

static void prov_complete(uint16_t net_idx, uint16_t addr, uint8_t flags, uint32_t iv_index)
{
    ESP_LOGI(MESH_DEBUG_TAG, "net_idx 0x%03x, addr 0x%04x", net_idx, addr);
    ESP_LOGI(MESH_DEBUG_TAG, "flags 0x%02x, iv_index 0x%08" PRIx32, flags, iv_index);
}

static void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                                             esp_ble_mesh_prov_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_NODE_PROV_ENABLE_COMP_EVT, err_code %d", param->node_prov_enable_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_NODE_PROV_LINK_OPEN_EVT, bearer %s",
                 param->node_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_NODE_PROV_LINK_CLOSE_EVT, bearer %s",
                 param->node_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT");
        prov_complete(param->node_prov_complete.net_idx, param->node_prov_complete.addr,
                      param->node_prov_complete.flags, param->node_prov_complete.iv_index);
        break;
    case ESP_BLE_MESH_NODE_PROV_RESET_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_NODE_PROV_RESET_EVT");
        send_unprovisioned_ack_to_gwy();
        esp_ble_mesh_node_local_reset();
        provisioned = false;
        ELEMENT_ADDR = 0;
        vTaskDelay(pdMS_TO_TICKS(100));
        esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
        break;
    case ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_NODE_SET_UNPROV_DEV_NAME_COMP_EVT, err_code %d", param->node_set_unprov_dev_name_comp.err_code);
        break;
    default:
        ESP_LOGE(MESH_DEBUG_TAG, "Unknown prov cb event");
        break;
    }
}

static void example_ble_mesh_config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
                                              esp_ble_mesh_cfg_server_cb_param_t *param)
{
    if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT)
    {
        switch (param->ctx.recv_op)
        {
        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD");
            ESP_LOGI(MESH_DEBUG_TAG, "net_idx 0x%04x, app_idx 0x%04x",
                     param->value.state_change.appkey_add.net_idx,
                     param->value.state_change.appkey_add.app_idx);
            ESP_LOG_BUFFER_HEX("AppKey", param->value.state_change.appkey_add.app_key, 16);
            break;

        case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND");
            ESP_LOGI(MESH_DEBUG_TAG, "elem_addr 0x%04x, app_idx 0x%04x, cid 0x%04x, mod_id 0x%04x",
                     param->value.state_change.mod_app_bind.element_addr,
                     param->value.state_change.mod_app_bind.app_idx,
                     param->value.state_change.mod_app_bind.company_id,
                     param->value.state_change.mod_app_bind.model_id);
            provisioned = true;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, PROVISIONED_FLAG_FLASH_ADDR, false);
            vTaskDelay(pdMS_TO_TICKS(5));
            ELEMENT_ADDR = param->value.state_change.mod_app_bind.element_addr;
            provision_t.base_data.json_packet_id = NODE_PROV_PACKET;
            provision_t.base_data.elementAddr = ELEMENT_ADDR;
            send_provisioned_ack_to_gwy();
            break;

        case ESP_BLE_MESH_MODEL_OP_NODE_RESET:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_NODE_RESET");
            esp_ble_mesh_node_local_reset();
            provisioned = false;
            eeprom_write_byte(EEPROM_SLAVE_ADDR, PROVISIONED_FLAG_FLASH_ADDR, true);
            vTaskDelay(pdMS_TO_TICKS(5));
            ELEMENT_ADDR = 0;
            send_unprovisioned_ack_to_gwy();
            vTaskDelay(pdMS_TO_TICKS(100));
            esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
            break;

        case ESP_BLE_MESH_MODEL_OP_HEARTBEAT_PUB_SET:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_NODE_RESET");
            break;

        case ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET:
            ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_OP_MODEL_PUB_SET");
            break;

        default:
            break;
        }
    }
}

static void store_data_to_node_structures()
{
    switch (BLE_recvd_data[0])
    {
    case NODE_AC_CONTROL_PACKET:
        ESP_LOGI(MESH_DEBUG_TAG, "NODE AC CONTROL PACKET");
        vendor_node_ac_control_t = BLE_recvd_data;
        node_ac_control_t = *vendor_node_ac_control_t;

        //Error Checks
        if(strcmp(node_ac_control_t.base_data.node_ser_no_str, NODE_SER_NO_IN_STRING) != 0) 
            node_ac_control_t.base_data.error_code = NODE_SER_NO_INVALID;
        if (!configured) 
            node_ac_control_t.base_data.error_code = NODE_NOT_CONFIGURED_WITH_AC_REMOTE;
        
        //Store AC Settings to Flash
        eeprom_write_byte(EEPROM_SLAVE_ADDR, POWER_FLASH_ADDR, node_ac_control_t.control.OffTimer);
        vTaskDelay(pdMS_TO_TICKS(5));
        eeprom_write_byte(EEPROM_SLAVE_ADDR, MODE_FLASH_ADDR, node_ac_control_t.control.mode_val);
        vTaskDelay(pdMS_TO_TICKS(5));
        eeprom_write_byte(EEPROM_SLAVE_ADDR, FAN_FLASH_ADDR, node_ac_control_t.control.fan);
        vTaskDelay(pdMS_TO_TICKS(5));
        eeprom_write_byte(EEPROM_SLAVE_ADDR, TEMPERATURE_FLASH_ADDR, node_ac_control_t.control.temp);
        vTaskDelay(pdMS_TO_TICKS(5));
        eeprom_write_byte(EEPROM_SLAVE_ADDR, SWINGH_FLASH_ADDR, node_ac_control_t.control.swingH);
        vTaskDelay(pdMS_TO_TICKS(5));
        eeprom_write_byte(EEPROM_SLAVE_ADDR, SWINGV_FLASH_ADDR, node_ac_control_t.control.swingV);
        vTaskDelay(pdMS_TO_TICKS(5));
        eeprom_write_byte(EEPROM_SLAVE_ADDR, ONTIMER_FLASH_ADDR, node_ac_control_t.control.OnTimer);
        vTaskDelay(pdMS_TO_TICKS(5));
        eeprom_write_byte(EEPROM_SLAVE_ADDR, OFFTIMER_FLASH_ADDR, node_ac_control_t.control.OffTimer);
        vTaskDelay(pdMS_TO_TICKS(5));
        eeprom_write_byte(EEPROM_SLAVE_ADDR, LOCKING_FLASH_ADDR, node_ac_control_t.control.Locking);
        vTaskDelay(pdMS_TO_TICKS(5));
        eeprom_write_byte(EEPROM_SLAVE_ADDR, TEMPLOCKLOWLIMIT_FLASH_ADDR, node_ac_control_t.control.TempLockLowLimit);
        vTaskDelay(pdMS_TO_TICKS(5));
        eeprom_write_byte(EEPROM_SLAVE_ADDR, TEMPLOCKUPLIMIT_FLASH_ADDR, node_ac_control_t.control.TempLockUpLimit);
        vTaskDelay(pdMS_TO_TICKS(5));
    
        sensor_states[0].sensor_data.raw_value->data = &node_ac_control_t;
        example_ble_mesh_send_sensor_status();
        if(node_ac_control_t.base_data.error_code == 0) needToSendIRComamnd = true;
        break;

    case NODE_DEBUG_INFO_PACKET:
        ESP_LOGI(MESH_DEBUG_TAG, "NODE DEBUG INFO PACKET");
        vendor_node_debug_info_t = BLE_recvd_data;
        node_debug_info_t = *vendor_node_debug_info_t;

        //Error Checks
        if(strcmp(node_debug_info_t.base_data.node_ser_no_str, NODE_SER_NO_IN_STRING) != 0) 
            node_debug_info_t.base_data.error_code = NODE_SER_NO_INVALID;
        // fetch_debug_info(); //This needs to be developed
        sensor_states[0].sensor_data.raw_value->data = &node_debug_info_t;
        example_ble_mesh_send_sensor_status();
        break;

    case NODE_RECONF_PACKET:
        ESP_LOGI(MESH_DEBUG_TAG, "NODE RECONF PACKET");
        vendor_node_reconfigure_t = BLE_recvd_data;
        node_reconf_t = *vendor_node_reconfigure_t;

        //Error Checks
        if(strcmp(node_reconf_t.base_data.node_ser_no_str, NODE_SER_NO_IN_STRING) != 0) 
            node_reconf_t.base_data.error_code = NODE_SER_NO_INVALID;
        if (!configured)
            node_reconf_t.base_data.error_code = NODE_NOT_CONFIGURED_WITH_AC_REMOTE;
        
        if(node_reconf_t.base_data.error_code == 0) configured = false;

        sensor_states[0].sensor_data.raw_value->data = &node_reconf_t;
        example_ble_mesh_send_sensor_status();
        break;

    case NODE_HEARTBEAT_PUB_CONF_PACKET:
        ESP_LOGI(MESH_DEBUG_TAG, "NODE PUB CONF PACKET");
        vendor_node_hearbeat_pub_conf_t = BLE_recvd_data;
        node_hearbeat_pub_conf_t = *vendor_node_hearbeat_pub_conf_t;

        //Error Checks
        if(strcmp(node_hearbeat_pub_conf_t.base_data.node_ser_no_str, NODE_SER_NO_IN_STRING) != 0) 
            node_hearbeat_pub_conf_t.base_data.error_code = NODE_SER_NO_INVALID;

        if(node_hearbeat_pub_conf_t.base_data.error_code == 0) {
            eeprom_write_byte(EEPROM_SLAVE_ADDR, HB_PUB_CONF_PERIOD_ADDR, gwy_pub_conf_t.pub_conf_period_in_sec);
            vTaskDelay(pdMS_TO_TICKS(5));
        delete_Temperature_data_publish_timer();
        create_Temperature_data_publish_timer();
        }

        sensor_states[0].sensor_data.raw_value->data = &node_hearbeat_pub_conf_t;
        example_ble_mesh_send_sensor_status();
        break;

    case NODE_TEACHING_MODE_START_PACKET:
        ESP_LOGI(MESH_DEBUG_TAG, "NODE TEACHING MODE START PACKET");
        vendor_node_teaching_mode_t = BLE_recvd_data;
        node_teaching_mode_t = *vendor_node_teaching_mode_t;

        //Error Checks
        if(strcmp(node_teaching_mode_t.base_data.node_ser_no_str, NODE_SER_NO_IN_STRING) != 0) 
            node_teaching_mode_t.base_data.error_code = NODE_SER_NO_INVALID;

        if(node_teaching_mode_t.base_data.error_code == 0) {     
            teaching_mode = true;
            teachMode_size_done = true;
        }
        
        sensor_states[0].sensor_data.raw_value->data = &node_teaching_mode_t;
        example_ble_mesh_send_sensor_status();
        break;

    default:
        ESP_LOGE(MESH_ERROR_TAG, "Unknown JSON PACKET ID recvd from Gwy");
        break;
    }
}

static void example_ble_mesh_custom_model_cb(esp_ble_mesh_model_cb_event_t event,
                                             esp_ble_mesh_model_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BLE_MESH_MODEL_OPERATION_EVT:
        if (param->model_operation.opcode == ESP_BLE_MESH_VND_MODEL_OP_SEND)
        {
            uint16_t tid = *(uint16_t *)param->model_operation.msg;
            uint16_t *dats = param->model_operation.msg;
            BLE_recvd_data = param->model_operation.msg;
            store_data_to_node_structures();
            vTaskDelay(10);
            ESP_LOGI(MESH_DEBUG_TAG, "Recv 0x%06" PRIx32 ", tid 0x%04x", param->model_operation.opcode, tid);
            esp_err_t err = esp_ble_mesh_server_model_send_msg(&vnd_models[0],
                                                               param->model_operation.ctx, ESP_BLE_MESH_VND_MODEL_OP_STATUS,
                                                               sizeof(tid), (uint8_t *)&tid);
            if (err)
                ESP_LOGE(MESH_DEBUG_TAG, "Failed to send message 0x%06x", ESP_BLE_MESH_VND_MODEL_OP_STATUS);
        }
        break;

    case ESP_BLE_MESH_MODEL_SEND_COMP_EVT:
        if (param->model_send_comp.err_code)
        {
            ESP_LOGE(MESH_DEBUG_TAG, "Failed to send message 0x%06" PRIx32, param->model_send_comp.opcode);
            break;
        }
        ESP_LOGI(MESH_DEBUG_TAG, "Send 0x%06" PRIx32, param->model_send_comp.opcode);
        break;
    case ESP_BLE_MESH_MODEL_PUBLISH_UPDATE_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_MODEL_PUBLISH_UPDATE_EVT");
        ESP_LOGI(MESH_DEBUG_TAG, "TEMPERATURE PERIODIC PUBLISHING");
        sensor_states[0].sensor_data.raw_value->data = &node_heartbeat_t;
        example_ble_mesh_send_sensor_status();
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
    esp_ble_mesh_register_sensor_server_callback(example_ble_mesh_sensor_server_cb);

    err = esp_ble_mesh_init(&provision, &composition);
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "Failed to initialize mesh stack");
        return err;
    }
    for (int i = 0; i < 10; i++)
    {
        ESP_LOGI(MESH_DEBUG_TAG, "%0x :", dev_uuid[i]);
    }

    err = esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
    if (esp_ble_mesh_node_is_provisioned())
    {
        provisioned = true;
    }
    else
    {
        provisioned = false;
    }
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "Failed to enable mesh node");
        return err;
    }

    // board_led_operation(LED_G, LED_ON);

    ESP_LOGI(MESH_DEBUG_TAG, "BLE Mesh Node initialized BY AK");

    return ESP_OK;
}

/**
 * @brief Main entry point for BLE mesh initialization
 * @param none
 * @retval none
 */
void node_mesh_main_init(void)
{
    esp_err_t err;

    ESP_LOGI(MESH_DEBUG_TAG, "Initializing...");

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // board_init();

    err = bluetooth_init();
    if (err)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "esp32_bluetooth_init failed (err %d)", err);
        return;
    }

    ble_mesh_get_dev_uuid(dev_uuid);
    for (int i = 0; i < 10; i++)
    {

        ESP_LOGI(MESH_DEBUG_TAG, "%0x :", dev_uuid[i]);
    }
    /* Initialize the Bluetooth Mesh Subsystem */
    err = ble_mesh_init();
    if (err)
    {
        ESP_LOGE(MESH_DEBUG_TAG, "Bluetooth mesh init failed (err %d)", err);
    }
    ESP_LOGE(MESH_DEBUG_TAG, "Gpio detect");
    esp_ble_mesh_cfg_server_cb_param_t paramss;
}

/**
 * @brief Function to send AC remote configuration ack to gateway. When the device is
 * in configuration mode and Any currently supported AC remote button is pressed in-front
 * of the device's IR receiver, the device will configure itself to act as that AC remote
 * from then. This needs to be sent to Gwy and then to cloud for user to know.
 * @param none
 * @retval none
 */
void send_AC_configuration_ack_to_gwy()
{
    ESP_LOGI(MESH_DEBUG_TAG, "Sending Conf ack to Gwy");
    sensor_states[0].sensor_data.raw_value->data = &node_conf_t;
    example_ble_mesh_send_sensor_status();
}

/**
 * @brief Function to send Manual AC control ack to Gwy
 * @param none
 * @retval none
 */
void send_manual_ac_control_ack_to_gwy()
{
    ESP_LOGI(MESH_DEBUG_TAG, "Sending manual AC control ack to Gwy");
    sensor_states[0].sensor_data.raw_value->data = &node_manual_ac_control_t;
    example_ble_mesh_send_sensor_status();
}

/**
 * @brief Function to send provisioning ack to gwy
 * @param none
 * @retval none
 */
void send_provisioned_ack_to_gwy()
{
    ESP_LOGI(MESH_DEBUG_TAG, "Sending Provisioning ack to Gwy");
    sensor_states[0].sensor_data.raw_value->data = &provision_t;
    example_ble_mesh_send_sensor_status();
}

/**
 * @brief Function that sends Unprovisioning ack to Gwy
 * @param none
 * @retval none
 */
void send_unprovisioned_ack_to_gwy()
{
    ESP_LOGI(MESH_DEBUG_TAG, "Sending Unprovisioning ack to Gwy");
    sensor_states[0].sensor_data.raw_value->data = &unprovision_t;
    example_ble_mesh_send_sensor_status();
}

/**
 * @brief Function that periodically sends the measured temperature data ack to Gwy
 * @param none
 * @retval none
 */
void send_temperature_ack_to_gwy()
{
    ESP_LOGI(MESH_DEBUG_TAG, "Sending Temperature data ack to Gwy");
    sensor_states[0].sensor_data.raw_value->data = &node_heartbeat_t;
    example_ble_mesh_send_sensor_status();
}

#endif