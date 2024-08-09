/**
 * @file mesh_main.c
 * @author Adhikesavan (Adhikesavan@qmaxsys.com)
 * @brief This file contains all ble-mesh related functions
 * @version 0.8.7
 * @date 2024-03-02
 * @copyright Copyright (c) 2024
 */

#include "../../inc/mesh/mesh_main.h"
#include "../../inc/Mesh/ble_mesh_example_init.h"

#include "esp_err.h"
// #include "btc_ble_mesh_prov.h"
#include "esp_ble_mesh_networking_api.h"
/*
 * SPDX-FileCopyrightText: 2017 Intel Corporation
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

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

#include "esp_ble_mesh_defs.h"

// #define TAG "EXAMPLE_INIT"

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

#if (IS_GWY)

#define BLE_BUF_SIZE 150

control_t *vendor_node_ac_control_t;
control_t *vendor_node_manual_ac_control_t;
prov_t *vendor_provision_t;
unprov_t *vendor_unprovision_t;
reconf_t *vendor_node_reconf_t;
reconf_t *vendor_node_config_t;
heartbeat_t *vendor_node_heartbeat_t;
pub_conf_t *vendor_node_heartbeat_pub_conf_t;
teaching_mode_t *vendor_node_teaching_mode_t;
debug_info_t *vendor_node_debug_info_t;

#define NVS_NAME "mesh_nvs"

esp_err_t ble_mesh_nvs_open(nvs_handle_t *handle)
{
    esp_err_t err = ESP_OK;

    if (handle == NULL)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Open, invalid nvs handle");
        return ESP_ERR_INVALID_ARG;
    }

    err = nvs_open(NVS_NAME, NVS_READWRITE, handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Open, nvs_open failed, err %d", err);
        return err;
    }

    ESP_LOGI(MESH_DEBUG_TAG, "Open namespace done, name \"%s\"", NVS_NAME);
    return err;
}

esp_err_t ble_mesh_nvs_store(nvs_handle_t handle, const char *key, const void *data, size_t length)
{
    esp_err_t err = ESP_OK;

    if (key == NULL || data == NULL || length == 0)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Store, invalid parameter");
        return ESP_ERR_INVALID_ARG;
    }

    err = nvs_set_blob(handle, key, data, length);
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Store, nvs_set_blob failed, err %d", err);
        return err;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Store, nvs_commit failed, err %d", err);
        return err;
    }

    // ESP_LOGI(MESH_DEBUG_TAG, "Store, key \"%s\", length %u", key, length);
    // ESP_LOG_BUFFER_HEX("EXAMPLE_NVS: Store, data", data, length);
    return err;
}

esp_err_t ble_mesh_nvs_get_length(nvs_handle_t handle, const char *key, size_t *length)
{
    esp_err_t err = ESP_OK;

    if (key == NULL || length == NULL)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Get length, invalid parameter");
        return ESP_ERR_INVALID_ARG;
    }

    err = nvs_get_blob(handle, key, NULL, length);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGI(MESH_DEBUG_TAG, "Get length, key \"%s\" not exists", key);
        *length = 0;
        return ESP_OK;
    }

    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Get length, nvs_get_blob failed, err %d", err);
    }
    else
    {
        ESP_LOGI(MESH_DEBUG_TAG, "Get length, key \"%s\", length %u", key, *length);
    }

    return err;
}

esp_err_t ble_mesh_nvs_restore(nvs_handle_t handle, const char *key, void *data, size_t length, bool *exist)
{
    esp_err_t err = ESP_OK;

    if (key == NULL || data == NULL || length == 0)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Restore, invalid parameter");
        return ESP_ERR_INVALID_ARG;
    }

    err = nvs_get_blob(handle, key, data, &length);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_LOGI(MESH_DEBUG_TAG, "Restore, key \"%s\" not exists", key);
        if (exist)
        {
            *exist = false;
        }
        return ESP_OK;
    }

    if (exist)
    {
        *exist = true;
    }

    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Restore, nvs_get_blob failed, err %d", err);
    }
    else
    {
        if (LOG_DATA)
        {
            ESP_LOGI(MESH_DEBUG_TAG, "Restore, key \"%s\", length %u", key, length);
            ESP_LOG_BUFFER_HEX("EXAMPLE_NVS: Restore, data", data, length);
        }
    }

    return err;
}

esp_err_t ble_mesh_nvs_erase(nvs_handle_t handle, const char *key)
{
    esp_err_t err = ESP_OK;

    if (key)
    {
        err = nvs_erase_key(handle, key);
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            ESP_LOGI(MESH_DEBUG_TAG, "Erase, key \"%s\" not exists", key);
            return ESP_OK;
        }
    }
    else
    {
        err = nvs_erase_all(handle);
    }
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Erase, nvs_erase_%s failed, err %d", key ? "key" : "all", err);
        return err;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Erase, nvs_commit failed, err %d", err);
        return err;
    }

    if (key)
    {
        ESP_LOGI(MESH_DEBUG_TAG, "Erase done, key \"%s\"", key);
    }
    else
    {
        ESP_LOGI(MESH_DEBUG_TAG, "Erase namespace done, name \"%s\"", NVS_NAME);
    }
    return err;
}

#define hide 0

#define CID_ESP 0x02E5

#define PROV_OWN_ADDR 0x0001

#define MSG_SEND_TTL 3
#define MSG_SEND_REL true
#define MSG_TIMEOUT 0
#define MSG_ROLE ROLE_PROVISIONER

#define COMP_DATA_PAGE_0 0x00

#define APP_KEY_IDX 0x0000
#define APP_KEY_OCTET GWY_SER_NO

#define COMP_DATA_1_OCTET(msg, offset) (msg[offset])
#define COMP_DATA_2_OCTET(msg, offset) (msg[offset + 1] << 8 | msg[offset])

/* Sensor Property ID */
#define SENSOR_PROPERTY_ID_0 0x0056 /* Present Indoor Ambient Temperature */
#define SENSOR_PROPERTY_ID_1 0x005B /* Present Outdoor Ambient Temperature */

#define SENSOR_POSITIVE_TOLERANCE ESP_BLE_MESH_SENSOR_UNSPECIFIED_POS_TOLERANCE
#define SENSOR_NEGATIVE_TOLERANCE ESP_BLE_MESH_SENSOR_UNSPECIFIED_NEG_TOLERANCE
#define SENSOR_SAMPLE_FUNCTION ESP_BLE_MESH_SAMPLE_FUNC_UNSPECIFIED
#define SENSOR_MEASURE_PERIOD ESP_BLE_MESH_SENSOR_NOT_APPL_MEASURE_PERIOD
#define SENSOR_UPDATE_INTERVAL ESP_BLE_MESH_SENSOR_NOT_APPL_UPDATE_INTERVAL

static uint8_t dev_uuid[ESP_BLE_MESH_OCTET16_LEN] = {0xdd, 0xdd};
static uint16_t server_address = ESP_BLE_MESH_ADDR_UNASSIGNED;
static uint16_t sensor_prop_id;

esp_ble_mesh_client_common_param_t common = {0};
esp_ble_mesh_cfg_client_set_state_t set = {0};
esp_ble_mesh_node_t *node = NULL;
esp_err_t err;

uint8_t binded = 0, prov = 0;
static struct esp_ble_mesh_key
{
    uint16_t net_idx;
    uint16_t app_idx;
    uint8_t app_key[ESP_BLE_MESH_OCTET16_LEN];
    uint8_t net_key[ESP_BLE_MESH_OCTET16_LEN];
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

static esp_ble_mesh_client_t config_client;
static esp_ble_mesh_client_t sensor_client;

NET_BUF_SIMPLE_DEFINE_STATIC(sensor_data_0, BLE_BUF_SIZE);
NET_BUF_SIMPLE_DEFINE_STATIC(sensor_data_1, BLE_BUF_SIZE);

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
ESP_BLE_MESH_MODEL_PUB_DEFINE(sensor_pub, BLE_BUF_SIZE * 2, ROLE_PROVISIONER);
static esp_ble_mesh_sensor_srv_t sensor_server = {
    .rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .state_count = ARRAY_SIZE(sensor_states),
    .states = sensor_states,
};

ESP_BLE_MESH_MODEL_PUB_DEFINE(sensor_setup_pub, BLE_BUF_SIZE * 2, ROLE_PROVISIONER);
static esp_ble_mesh_sensor_setup_srv_t sensor_setup_server = {
    .rsp_ctrl.get_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .rsp_ctrl.set_auto_rsp = ESP_BLE_MESH_SERVER_RSP_BY_APP,
    .state_count = ARRAY_SIZE(sensor_states),
    .states = sensor_states,
};

ESP_BLE_MESH_MODEL_PUB_DEFINE(client_pub, BLE_BUF_SIZE * 2, ROLE_PROVISIONER);

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
        ESP_LOGE(MESH_ERROR_TAG, "No memory for sensor descriptor status!");
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
        ESP_LOGE(MESH_ERROR_TAG, "Failed to send Sensor Descriptor Status");
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
        ESP_LOGE(MESH_ERROR_TAG, "Failed to send Sensor Cadence Status");
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
        ESP_LOGE(MESH_ERROR_TAG, "Failed to send Sensor Settings Status");
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
        ESP_LOGE(MESH_ERROR_TAG, "Failed to send Sensor Setting Status");
    }
}

static uint16_t example_ble_mesh_get_sensor_data(esp_ble_mesh_sensor_state_t *state, uint8_t *data)
{
    uint8_t mpid_len = 0, data_len = 0;
    uint32_t mpid = 0;

    if (state == NULL || data == NULL)
    {
        ESP_LOGE(MESH_ERROR_TAG, "%s, Invalid parameter", __func__);
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
    mpid = 0xe00e;
    mpid_len = 2;
    // net_buf_simple_add_u8(&sensor_data_0, 11);
    mpid = 0xe00e;

    data_len = 150;
    memcpy(data, &mpid, mpid_len);
    memcpy(data + mpid_len, state->sensor_data.raw_value->data, data_len);

    return (mpid_len + data_len);
}

static void example_ble_mesh_send_sensor_status(/*int a*/ esp_ble_mesh_sensor_server_cb_param_t *param)
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

    status = calloc(1, 70);
    if (!status)
    {
        ESP_LOGE(MESH_ERROR_TAG, "No memory for sensor status!");
        return;
    }

    for (i = 0; i < ARRAY_SIZE(sensor_states); i++)
    {
        length += example_ble_mesh_get_sensor_data(&sensor_states[i], status + length);
    }
    goto send;

send:
    if (LOG_DATA)
        ESP_LOG_BUFFER_HEX("Sensor Data", status, length);

    ESP_LOGE(MESH_ERROR_TAG, "Node pub addr 0x%04x ", sensor_server.model->pub->publish_addr);
    ESP_LOGE(MESH_ERROR_TAG, "Node ap idx addr 0x%04x ", sensor_server.model->pub->app_idx);
    err = esp_ble_mesh_model_publish(sensor_server.model, ESP_BLE_MESH_MODEL_OP_SENSOR_STATUS, length, status, ROLE_NODE);
    esp_ble_mesh_msg_ctx_t cntx;
    cntx.net_idx = 0;
    cntx.app_idx = 0;
    cntx.addr = 0001;
    cntx.send_rel = 1;
    cntx.send_ttl = 2;
    cntx.srv_send = true;

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
        ESP_LOGE(MESH_ERROR_TAG, "No memory for sensor column status!");
        return;
    }

    memcpy(status, &param->value.get.sensor_column.property_id, ESP_BLE_MESH_SENSOR_PROPERTY_ID_LEN);
    memcpy(status + ESP_BLE_MESH_SENSOR_PROPERTY_ID_LEN, param->value.get.sensor_column.raw_value_x->data,
           param->value.get.sensor_column.raw_value_x->len);

    err = esp_ble_mesh_server_model_send_msg(param->model, &param->ctx,
                                             ESP_BLE_MESH_MODEL_OP_SENSOR_COLUMN_STATUS, length, status);
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Failed to send Sensor Column Status");
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
        ESP_LOGE(MESH_ERROR_TAG, "Failed to send Sensor Column Status");
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
        ESP_LOGE(MESH_ERROR_TAG, "Node 0x%04x not exists", server_address);
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
        ESP_LOGE(MESH_ERROR_TAG, "Failed to send sensor message 0x%04" PRIx32, opcode);
    }
}

static void example_ble_mesh_sensor_timeout(uint32_t opcode)
{
    switch (opcode)
    {
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
        ESP_LOGE(MESH_ERROR_TAG, "Unknown Sensor Get/Set opcode 0x%04" PRIx32, opcode);
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
    /*  ESP_LOGI(MESH_DEBUG_TAG, "Sensor server, event %d, src 0x%04x, dst 0x%04x, model_id 0x%04x",
              event, param->ctx.addr, param->ctx.recv_dst, param->model->model_id);
      ESP_LOGI(MESH_DEBUG_TAG, "Sensor server, event %d, src 0x%04x, dst 0x%04x, model_id 0x%04x",
              event, param->ctx.addr, param->ctx.recv_dst, param->model->model_id);*/

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
            ESP_LOGE(MESH_ERROR_TAG, "Unknown Sensor Get opcode 0x%04" PRIx32, param->ctx.recv_op);
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
            ESP_LOGE(MESH_ERROR_TAG, "Unknown Sensor Set opcode 0x%04" PRIx32, param->ctx.recv_op);
            break;
        }
        break;
    default:
        ESP_LOGE(MESH_ERROR_TAG, "Unknown Sensor Server event %d", event);
        break;
    }
}

/**
 * @brief This function takes care of sending the Node acknowledgements back to cloud
 * @param param
 * @retval none
 */
static void store_data_to_node_structures(esp_ble_mesh_sensor_client_cb_param_t *param)
{
    char pubmessage[PUBMESG_LEN];
    jwOpen(&jwc, pubmessage, PUBMESG_LEN, JW_OBJECT, 1);
    uint8_t recvd_json_id = param->status_cb.sensor_status.marshalled_sensor_data->data[0];
    if (registered)
    {
        switch (recvd_json_id)
        {
        case NODE_HEARTBEAT_ACK:
            vendor_node_heartbeat_t = param->status_cb.sensor_status.marshalled_sensor_data->data;
            ESP_LOGI(MESH_DEBUG_TAG, "NODE HEARTBEAT ACK | FROM ELEMADDR : %d", vendor_node_heartbeat_t->base_data.elementAddr);
            jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_HEARTBEAT_ACK);
            jwObj_string(&jwc, GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING);
            jwObj_string(&jwc, NODE_SER_NO_KEY, vendor_node_heartbeat_t->base_data.node_ser_no_str);
            jwObj_int(&jwc, POWER_KEY, vendor_node_heartbeat_t->control.power);
            jwObj_string(&jwc, MODE_KEY, vendor_node_heartbeat_t->control.mode_str);
            jwObj_int(&jwc, FAN_SPEED_KEY, vendor_node_heartbeat_t->control.fanSpeed);
            jwObj_int(&jwc, TEMPERATURE_KEY, vendor_node_heartbeat_t->control.temp);
            jwObj_int(&jwc, AMBIENT_TEMPERATURE_DATA_KEY, vendor_node_heartbeat_t->measured_temperature);
            jwObj_int(&jwc, SWING_H_KEY, vendor_node_heartbeat_t->control.swingH);
            jwObj_int(&jwc, SWING_V_KEY, vendor_node_heartbeat_t->control.swingV);
            jwObj_int(&jwc, ONTIMER_KEY, vendor_node_heartbeat_t->control.OnTimer);
            jwObj_int(&jwc, OFFTIMER_KEY, vendor_node_heartbeat_t->control.OffTimer);
            jwObj_int(&jwc, AC_LOCKING_KEY, vendor_node_heartbeat_t->control.Locking);
            jwObj_int(&jwc, TEMP_LOCK_UP_LIMIT_KEY, vendor_node_heartbeat_t->control.TempLockUpLimit);
            jwObj_int(&jwc, TEMP_LOCK_LOW_LIMIT_KEY, vendor_node_heartbeat_t->control.TempLockLowLimit);
            break;

        case NODE_AC_CONTROL_PACKET:
            remove_from_ac_control_queue();
            vendor_node_ac_control_t = param->status_cb.sensor_status.marshalled_sensor_data->data;
            ESP_LOGI(MESH_DEBUG_TAG, "NODE AC CONTROL ACK | FROM ELEMADDR : %d", vendor_node_ac_control_t->base_data.elementAddr);
            jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_AC_CONTROL_PACKET);
            jwObj_int(&jwc, MSG_SEQ_NO_KEY, vendor_node_ac_control_t->base_data.msg_seq_no);
            jwObj_string(&jwc, GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING);
            jwObj_string(&jwc, NODE_SER_NO_KEY, vendor_node_ac_control_t->base_data.node_ser_no_str);
            jwObj_int(&jwc, ERROR_CODE_KEY, vendor_node_ac_control_t->base_data.error_code);
            break;

        case NODE_TEACHING_MODE_START_PACKET:
            remove_from_teaching_mode_queue();
            vendor_node_teaching_mode_t = param->status_cb.sensor_status.marshalled_sensor_data->data;
            ESP_LOGI(MESH_DEBUG_TAG, "NODE TEACHING MODE START ACK | FROM ELEMADDR : %d", vendor_node_teaching_mode_t->base_data.elementAddr);
            jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_TEACHING_MODE_START_PACKET);
            jwObj_int(&jwc, MSG_SEQ_NO_KEY, vendor_node_teaching_mode_t->base_data.msg_seq_no);
            jwObj_string(&jwc, GWY_SER_NO_KEY, vendor_node_teaching_mode_t->base_data.gwy_ser_no_str);
            jwObj_string(&jwc, NODE_SER_NO_KEY, vendor_node_teaching_mode_t->base_data.node_ser_no_str);
            jwObj_int(&jwc, ERROR_CODE_KEY, vendor_node_teaching_mode_t->base_data.error_code);
            break;

        case NODE_TEACHING_MODE_END_ACK:
            vendor_node_teaching_mode_t = param->status_cb.sensor_status.marshalled_sensor_data->data;
            ESP_LOGI(MESH_DEBUG_TAG, "NODE TEACHING MODE END ACK | FROM ELEMADDR : %d", vendor_node_teaching_mode_t->base_data.elementAddr);
            jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_TEACHING_MODE_END_ACK);
            jwObj_string(&jwc, GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING);
            jwObj_string(&jwc, NODE_SER_NO_KEY, vendor_node_teaching_mode_t->base_data.node_ser_no_str);
            jwObj_int(&jwc, ERROR_CODE_KEY, vendor_node_teaching_mode_t->base_data.error_code);
            break;

        case NODE_DEBUG_INFO_PACKET:
            remove_from_debug_info_queue();
            vendor_node_debug_info_t = param->status_cb.sensor_status.marshalled_sensor_data->data;
            ESP_LOGI(MESH_DEBUG_TAG, "NODE DEBUG INFO ACK | FROM ELEMADDR : %d", vendor_node_debug_info_t->base_data.elementAddr);
            jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_DEBUG_INFO_PACKET);
            jwObj_int(&jwc, MSG_SEQ_NO_KEY, vendor_node_debug_info_t->base_data.msg_seq_no);
            jwObj_string(&jwc, GWY_SER_NO_KEY, vendor_node_debug_info_t->base_data.gwy_ser_no_str);
            jwObj_string(&jwc, NODE_SER_NO_KEY, vendor_node_debug_info_t->base_data.node_ser_no_str);
            jwObj_int(&jwc, ELEMENT_ADDR_KEY, vendor_node_debug_info_t->base_data.elementAddr);
            jwObj_string(&jwc, FIRMWARE_VERSION_KEY, vendor_node_debug_info_t->firmware);
            jwObj_string(&jwc, PROTOCOL_SEL_NUM_KEY, get_protocol_string(vendor_node_debug_info_t->protocol));
            jwObj_string(&jwc, DEVICE_UPTIME_KEY, vendor_node_debug_info_t->uptimestr);
            jwObj_int(&jwc, LOGGING_KEY, vendor_node_debug_info_t->logging);
            jwObj_int(&jwc, RESET_DEVICE_KEY, vendor_node_debug_info_t->resetDevice);
            jwObj_int(&jwc, ERROR_CODE_KEY, vendor_node_debug_info_t->base_data.error_code);
            break;

        case NODE_RECONF_PACKET:
            remove_from_reconf_queue();
            vendor_node_reconf_t = (reconf_t *)param->status_cb.sensor_status.marshalled_sensor_data->data;
            ESP_LOGI(MESH_DEBUG_TAG, "NODE RECONF ACK | FROM ELEMADDR : %d", vendor_node_reconf_t->base_data.elementAddr);
            jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_RECONF_PACKET);
            jwObj_int(&jwc, MSG_SEQ_NO_KEY, vendor_node_reconf_t->base_data.msg_seq_no);
            jwObj_string(&jwc, GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING);
            jwObj_string(&jwc, NODE_SER_NO_KEY, vendor_node_reconf_t->base_data.node_ser_no_str);
            jwObj_int(&jwc, ERROR_CODE_KEY, vendor_node_reconf_t->base_data.error_code);
            break;

        case NODE_HEARTBEAT_PUB_CONF_PACKET:
            remove_from_heartbeat_pub_conf_queue();
            vendor_node_heartbeat_pub_conf_t = (heartbeat_t *)param->status_cb.sensor_status.marshalled_sensor_data->data;
            ESP_LOGI(MESH_DEBUG_TAG, "NODE PUB CONF ACK | FROM ELEMADDR : %d", vendor_node_heartbeat_pub_conf_t->base_data.elementAddr);
            jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_HEARTBEAT_PUB_CONF_PACKET);
            jwObj_int(&jwc, MSG_SEQ_NO_KEY, vendor_node_heartbeat_pub_conf_t->base_data.msg_seq_no);
            jwObj_string(&jwc, GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING);
            jwObj_string(&jwc, NODE_SER_NO_KEY, vendor_node_heartbeat_pub_conf_t->base_data.node_ser_no_str);
            jwObj_int(&jwc, ERROR_CODE_KEY, vendor_node_heartbeat_pub_conf_t->base_data.error_code);
            break;

        case NODE_MANUAL_AC_CONTROL_ACK:
            vendor_node_manual_ac_control_t = (control_t *)param->status_cb.sensor_status.marshalled_sensor_data->data;
            ESP_LOGI(MESH_DEBUG_TAG, "NODE MANUAL AC CONTROL ACK | FROM ELEMADDR : %d", vendor_node_manual_ac_control_t->base_data.elementAddr);
            jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_MANUAL_AC_CONTROL_ACK);
            jwObj_int(&jwc, MSG_SEQ_NO_KEY, vendor_node_manual_ac_control_t->base_data.msg_seq_no);
            jwObj_string(&jwc, GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING);
            jwObj_string(&jwc, NODE_SER_NO_KEY, vendor_node_manual_ac_control_t->base_data.node_ser_no_str);
            jwObj_int(&jwc, POWER_KEY, vendor_node_manual_ac_control_t->control.power);
            jwObj_string(&jwc, MODE_KEY, vendor_node_manual_ac_control_t->control.mode_str);
            jwObj_int(&jwc, FAN_SPEED_KEY, vendor_node_manual_ac_control_t->control.fanSpeed);
            jwObj_int(&jwc, TEMPERATURE_KEY, vendor_node_manual_ac_control_t->control.temp);
            break;

        case NODE_PROV_PACKET:
            remove_from_prov_queue();
            vendor_provision_t = (prov_t *)param->status_cb.sensor_status.marshalled_sensor_data->data;
            ESP_LOGI(MESH_DEBUG_TAG, "NODE PROV ACK | FROM ELEMADDR : %d", vendor_provision_t->base_data.elementAddr);
            jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_PROV_PACKET);
            jwObj_int(&jwc, MSG_SEQ_NO_KEY, provision_t.base_data.msg_seq_no);
            jwObj_string(&jwc, GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING);
            jwObj_string(&jwc, NODE_SER_NO_KEY, provision_t.base_data.node_ser_no_str);
            jwObj_int(&jwc, ELEMENT_ADDR_KEY, vendor_provision_t->base_data.elementAddr);
            jwObj_int(&jwc, ERROR_CODE_KEY, vendor_provision_t->base_data.error_code);
            break;

        case NODE_UNPROV_PACKET:
            // this is a special case handled by a the prov/unprov callback.
            break;

        case NODE_CONF_PACKET:
            vendor_node_config_t = (reconf_t *) param->status_cb.sensor_status.marshalled_sensor_data->data;
            ESP_LOGI(MESH_DEBUG_TAG, "NODE CONF ACK | FROM ELEMADDR : %d", vendor_node_config_t->base_data.elementAddr);
            jwObj_int(&jwc, JSON_PACKET_ID_KEY, NODE_CONF_PACKET);
            jwObj_string(&jwc, GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING);
            jwObj_string(&jwc, NODE_SER_NO_KEY, vendor_node_config_t->base_data.node_ser_no_str);
            jwObj_int(&jwc, ERROR_CODE_KEY, vendor_node_config_t->base_data.error_code);
            break;

        default:
            custom_printf(MESH_ERROR_TAG, "Unknown JSON PACKET ID recvd from Node", RED);
            jwClose(&jwc);
            return;
        }
        jwClose(&jwc);
        add_to_pubmesg_queue(pubmessage, publish_topic);
    }
    else custom_printf(MESH_ERROR_TAG, "Ack came in when Gwy in Unregistered state", RED);
}
static void example_ble_mesh_sensor_client_cb(esp_ble_mesh_sensor_client_cb_event_t event,
                                              esp_ble_mesh_sensor_client_cb_param_t *param)
{
    if (LOG_DATA)
        ESP_LOGI(MESH_DEBUG_TAG, "Sensor client data, event %u, addr 0x%04x", event, param->params->ctx.addr);
    store_data_to_node_structures(param);
    if (param->error_code)
    {
        if (LOG_DATA)
            ESP_LOGE(MESH_ERROR_TAG, "Send sensor client message failed (err %d)", param->error_code);
        return;
    }
    if (LOG_DATA)
        ESP_LOG_BUFFER_HEX("Sensor Data", param->status_cb.sensor_status.marshalled_sensor_data->data,
                           param->status_cb.sensor_status.marshalled_sensor_data->len);
}

#ifdef CONFIG_BT_BLUEDROID_ENABLED

uint16_t node_address;

void ble_mesh_get_dev_uuid(uint8_t *dev_uuid)
{
    if (dev_uuid == NULL)
    {
        ESP_LOGE(MESH_ERROR_TAG, "%s, Invalid device uuid", __func__);
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
        ESP_LOGE(MESH_ERROR_TAG, "%s initialize controller failed", __func__);
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        ESP_LOGE(MESH_ERROR_TAG, "%s enable controller failed", __func__);
        return ret;
    }
    ret = esp_bluedroid_init();
    if (ret)
    {
        ESP_LOGE(MESH_ERROR_TAG, "%s init bluetooth failed", __func__);
        return ret;
    }
    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(MESH_ERROR_TAG, "%s enable bluetooth failed", __func__);
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

static struct example_info_store
{
    uint16_t server_addr; /* Vendor server unicast address */
    uint16_t vnd_tid;
    control_t vendor_node_ac_control; /* TID contained in the vendor message */
    prov_t vendor_provision_t;
    unprov_t vendor_unprovision_t;
    reconf_t vendor_node_reconf_t;
    teaching_mode_t vendor_node_teaching_mode_t;
    debug_info_t vendor_node_debug_info_t;
    pub_conf_t vendor_node_heartbeat_pub_conf_t;
} store = {
    .server_addr = ESP_BLE_MESH_ADDR_UNASSIGNED,
    .vnd_tid = 0,
};

static nvs_handle_t NVS_HANDLE;
static const char *NVS_KEY = "vendor_client";

static esp_ble_mesh_client_t config_client;

static const esp_ble_mesh_client_op_pair_t vnd_op_pair[] = {
    {ESP_BLE_MESH_VND_MODEL_OP_SEND, ESP_BLE_MESH_VND_MODEL_OP_STATUS},
};

static esp_ble_mesh_client_t vendor_client = {
    .op_pair_size = ARRAY_SIZE(vnd_op_pair),
    .op_pair = vnd_op_pair,
};

static esp_ble_mesh_model_op_t vnd_op[] = {
    ESP_BLE_MESH_MODEL_OP(ESP_BLE_MESH_VND_MODEL_OP_STATUS, 2),
    ESP_BLE_MESH_MODEL_OP_END,
};

static esp_ble_mesh_model_t vnd_models[] = {
    ESP_BLE_MESH_VENDOR_MODEL(CID_ESP, ESP_BLE_MESH_VND_MODEL_ID_CLIENT,
                              vnd_op, NULL, &vendor_client),
};

static esp_ble_mesh_model_t root_models[] = {

    ESP_BLE_MESH_MODEL_CFG_CLI(&config_client),
    ESP_BLE_MESH_MODEL_SENSOR_CLI(&client_pub, &sensor_client),
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
    .prov_uuid = dev_uuid,
    .prov_unicast_addr = PROV_OWN_ADDR,
    .prov_start_address = 0x0005,
};

static void mesh_example_info_store(void)
{
    ble_mesh_nvs_store(NVS_HANDLE, NVS_KEY, &store, sizeof(store));
}

static void mesh_example_info_restore(void)
{
    esp_err_t err = ESP_OK;
    bool exist = false;

    err = ble_mesh_nvs_restore(NVS_HANDLE, NVS_KEY, &store, sizeof(store), &exist);
    if (err != ESP_OK)
    {
        return;
    }

    if (exist)
    {
        ESP_LOGI(MESH_DEBUG_TAG, "Restore, server_addr 0x%04x, vnd_tid 0x%04x", store.server_addr, store.vnd_tid);
    }
}

static esp_err_t prov_complete(uint16_t node_index, const esp_ble_mesh_octet16_t uuid,
                               uint16_t primary_addr, uint8_t element_num, uint16_t net_idx)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_cfg_client_get_state_t get = {0};
    esp_ble_mesh_node_t *node = NULL;
    char name[10] = {'\0'};
    esp_err_t err;

    ESP_LOGI(MESH_DEBUG_TAG, "node_index %u, primary_addr 0x%04x, element_num %u, net_idx 0x%03x",
             node_index, primary_addr, element_num, net_idx);
    ESP_LOG_BUFFER_HEX("uuid", uuid, ESP_BLE_MESH_OCTET16_LEN);

    store.server_addr = primary_addr;
    mesh_example_info_store(); /* Store proper mesh example info */

    sprintf(name, "%s%02x", "NODE-", node_index);
    err = esp_ble_mesh_provisioner_set_node_name(node_index, name);
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Failed to set node name");
        return ESP_FAIL;
    }

    node = esp_ble_mesh_provisioner_get_node_with_addr(primary_addr);
    if (node == NULL)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Failed to get node 0x%04x info", primary_addr);
        return ESP_FAIL;
    }

    example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET);
    get.comp_data_get.page = COMP_DATA_PAGE_0;
    err = esp_ble_mesh_config_client_get_state(&common, &get);
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Failed to send Config Composition Data Get");
        return ESP_FAIL;
    }

    return ESP_OK;
}

static void recv_unprov_adv_pkt(uint8_t dev_uuid[ESP_BLE_MESH_OCTET16_LEN], uint8_t addr[BD_ADDR_LEN],
                                esp_ble_mesh_addr_type_t addr_type, uint16_t oob_info,
                                uint8_t adv_type, esp_ble_mesh_prov_bearer_t bearer)
{
    esp_ble_mesh_unprov_dev_add_t add_dev = {0};
    esp_err_t err;

    /* Due to the API esp_ble_mesh_provisioner_set_dev_uuid_match, Provisioner will only
     * use this callback to report the devices, whose device UUID starts with 0xdd & 0xdd,
     * to the application layer.
     */

    ESP_LOG_BUFFER_HEX("Device address", addr, BD_ADDR_LEN);
    ESP_LOGI(MESH_DEBUG_TAG, "Address type 0x%02x, adv type 0x%02x", addr_type, adv_type);
    ESP_LOG_BUFFER_HEX("Device UUID", dev_uuid, ESP_BLE_MESH_OCTET16_LEN);
    ESP_LOGI(MESH_DEBUG_TAG, "oob info 0x%04x, bearer %s", oob_info, (bearer & ESP_BLE_MESH_PROV_ADV) ? "PB-ADV" : "PB-GATT");

    memcpy(add_dev.addr, addr, BD_ADDR_LEN);
    add_dev.addr_type = (uint8_t)addr_type;
    memcpy(add_dev.uuid, dev_uuid, ESP_BLE_MESH_OCTET16_LEN);
    add_dev.oob_info = oob_info;
    add_dev.bearer = (uint8_t)bearer;
    /* Note: If unprovisioned device adv packets have not been received, we should not add
             device with ADD_DEV_START_PROV_NOW_FLAG set. */
    err = esp_ble_mesh_provisioner_add_unprov_dev(&add_dev,
                                                  ADD_DEV_RM_AFTER_PROV_FLAG | ADD_DEV_START_PROV_NOW_FLAG | ADD_DEV_FLUSHABLE_DEV_FLAG);
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Failed to start provisioning device");
    }
}

static void example_ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                                             esp_ble_mesh_prov_cb_param_t *param)
{
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_cfg_client_set_state_t set = {0};
    esp_ble_mesh_node_t *node = NULL;
    esp_err_t err;

    switch (event)
    {
    case ESP_BLE_MESH_PROV_REGISTER_COMP_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_PROV_REGISTER_COMP_EVT, err_code %d", param->prov_register_comp.err_code);
        mesh_example_info_restore(); /* Restore proper mesh example info */
        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_ENABLE_COMP_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_PROVISIONER_PROV_ENABLE_COMP_EVT, err_code %d", param->provisioner_prov_enable_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_DISABLE_COMP_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_PROVISIONER_PROV_DISABLE_COMP_EVT, err_code %d", param->provisioner_prov_disable_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_RECV_UNPROV_ADV_PKT_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_PROVISIONER_RECV_UNPROV_ADV_PKT_EVT");
        recv_unprov_adv_pkt(param->provisioner_recv_unprov_adv_pkt.dev_uuid, param->provisioner_recv_unprov_adv_pkt.addr,
                            param->provisioner_recv_unprov_adv_pkt.addr_type, param->provisioner_recv_unprov_adv_pkt.oob_info,
                            param->provisioner_recv_unprov_adv_pkt.adv_type, param->provisioner_recv_unprov_adv_pkt.bearer);
        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_LINK_OPEN_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_PROVISIONER_PROV_LINK_OPEN_EVT, bearer %s",
                 param->provisioner_prov_link_open.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT");
        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_LINK_CLOSE_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_PROVISIONER_PROV_LINK_CLOSE_EVT, bearer %s, reason 0x%02x",
                 param->provisioner_prov_link_close.bearer == ESP_BLE_MESH_PROV_ADV ? "PB-ADV" : "PB-GATT", param->provisioner_prov_link_close.reason);

        break;
    case ESP_BLE_MESH_PROVISIONER_PROV_COMPLETE_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_PROVISIONER_PROV_COMPLETE_EVT");
        prov_complete(param->provisioner_prov_complete.node_idx, param->provisioner_prov_complete.device_uuid,
                      param->provisioner_prov_complete.unicast_addr, param->provisioner_prov_complete.element_num,
                      param->provisioner_prov_complete.netkey_idx);
        break;
    case ESP_BLE_MESH_PROVISIONER_ADD_UNPROV_DEV_COMP_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_PROVISIONER_ADD_UNPROV_DEV_COMP_EVT, err_code %d", param->provisioner_add_unprov_dev_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_SET_DEV_UUID_MATCH_COMP_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_PROVISIONER_SET_DEV_UUID_MATCH_COMP_EVT, err_code %d", param->provisioner_set_dev_uuid_match_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_SET_NODE_NAME_COMP_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_PROVISIONER_SET_NODE_NAME_COMP_EVT, err_code %d", param->provisioner_set_node_name_comp.err_code);
        if (param->provisioner_set_node_name_comp.err_code == 0)
        {
            const char *name = esp_ble_mesh_provisioner_get_node_name(param->provisioner_set_node_name_comp.node_index);
            if (name)
            {
                ESP_LOGI(MESH_DEBUG_TAG, "Node %d name %s", param->provisioner_set_node_name_comp.node_index, name);
            }
        }
        break;
    case ESP_BLE_MESH_PROVISIONER_ADD_LOCAL_APP_KEY_COMP_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_PROVISIONER_ADD_LOCAL_APP_KEY_COMP_EVT, err_code %d", param->provisioner_add_app_key_comp.err_code);
        if (param->provisioner_add_app_key_comp.err_code == 0)
        {
            prov_key.app_idx = param->provisioner_add_app_key_comp.app_idx;
            esp_err_t err = esp_ble_mesh_provisioner_bind_app_key_to_local_model(PROV_OWN_ADDR, prov_key.app_idx,
                                                                                 ESP_BLE_MESH_VND_MODEL_ID_CLIENT, CID_ESP); // CID_ESP
            // vTaskDelay(pdMS_TO_TICKS(200));
            err = esp_ble_mesh_provisioner_bind_app_key_to_local_model(PROV_OWN_ADDR, prov_key.app_idx,
                                                                       ESP_BLE_MESH_MODEL_ID_SENSOR_CLI, ESP_BLE_MESH_CID_NVAL);
            if (err != ESP_OK)
            {
                ESP_LOGE(MESH_ERROR_TAG, "Failed to bind AppKey to vendor client");
            }
        }
        break;
    case ESP_BLE_MESH_PROVISIONER_BIND_APP_KEY_TO_MODEL_COMP_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_PROVISIONER_BIND_APP_KEY_TO_MODEL_COMP_EVT, err_code %d", param->provisioner_bind_app_key_to_model_comp.err_code);
        break;
    case ESP_BLE_MESH_PROVISIONER_STORE_NODE_COMP_DATA_COMP_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_PROVISIONER_STORE_NODE_COMP_DATA_COMP_EVT, err_code %d", param->provisioner_store_node_comp_data_comp.err_code);
        break;
    case ESP_BLE_MESH_NODE_ADD_LOCAL_NET_KEY_COMP_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_NODE_ADD_LOCAL_NET_KEY_COMP_EVT, err_code %d ,net_idx %d", param->node_add_net_key_comp.err_code, param->node_add_net_key_comp.net_idx);
        prov_key.net_idx = param->node_add_net_key_comp.net_idx;
        break;
    case ESP_BLE_MESH_NODE_ADD_LOCAL_APP_KEY_COMP_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "ESP_BLE_MESH_NODE_ADD_LOCAL_NET_KEY_COMP_EVT, err_code %d ,net_idx %d", param->node_add_app_key_comp.err_code, param->node_add_app_key_comp.app_idx);
        prov_key.app_idx = param->node_add_app_key_comp.app_idx;
        break;
    case ESP_BLE_MESH_NODE_BIND_APP_KEY_TO_MODEL_COMP_EVT:
        ESP_LOGE(MESH_ERROR_TAG, "ESP_BLE_MESH_NODE_BIND_APP_KEY_TO_MODEL_COMP_EVT");
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
uint8_t set_conf = 0;

static void example_ble_mesh_config_client_cb(esp_ble_mesh_cfg_client_cb_event_t event,
                                              esp_ble_mesh_cfg_client_cb_param_t *param)
{
    ESP_LOGI(MESH_DEBUG_TAG, "Config client, err_code %d, event %u, addr 0x%04x, opcode 0x%04" PRIx32,
             param->error_code, event, param->params->ctx.addr, param->params->opcode);

    if (param->error_code)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Send config client message failed, opcode 0x%04" PRIx32, param->params->opcode);
        return;
    }

    node = esp_ble_mesh_provisioner_get_node_with_addr(param->params->ctx.addr);
    if (!node)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Failed to get node 0x%04x info", param->params->ctx.addr);
        return;
    }
    // ESP_LOGW(TAG, " config client event %u ", event);
    switch (event)
    {
    case ESP_BLE_MESH_CFG_CLIENT_GET_STATE_EVT:
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET)
        {
            ESP_LOG_BUFFER_HEX("Composition data", param->status_cb.comp_data_status.composition_data->data,
                               param->status_cb.comp_data_status.composition_data->len);
            example_ble_mesh_parse_node_comp_data(param->status_cb.comp_data_status.composition_data->data,
                                                  param->status_cb.comp_data_status.composition_data->len);
            err = esp_ble_mesh_provisioner_store_node_comp_data(param->params->ctx.addr,
                                                                param->status_cb.comp_data_status.composition_data->data,
                                                                param->status_cb.comp_data_status.composition_data->len);
            if (err != ESP_OK)
            {
                ESP_LOGE(MESH_ERROR_TAG, "Failed to store node composition data");
                break;
            }

            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
            set.app_key_add.net_idx = prov_key.net_idx;
            set.app_key_add.app_idx = prov_key.app_idx;
            memcpy(set.app_key_add.app_key, prov_key.app_key, ESP_BLE_MESH_OCTET16_LEN);
            err = esp_ble_mesh_config_client_set_state(&common, &set);
            if (err != ESP_OK)
            {
                ESP_LOGE(MESH_ERROR_TAG, "Failed to send Config AppKey Add");
            }
        }
        break;
    case ESP_BLE_MESH_CFG_CLIENT_SET_STATE_EVT:
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD)
        {
            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
            set.model_app_bind.element_addr = node->unicast_addr;
            set.model_app_bind.model_app_idx = prov_key.app_idx;
            set.model_app_bind.model_id = ESP_BLE_MESH_VND_MODEL_ID_SERVER;
            set.model_app_bind.company_id = CID_ESP;
            set_conf = 1;
            err = esp_ble_mesh_config_client_set_state(&common, &set);

            if (err != ESP_OK)
            {
                ESP_LOGE(MESH_ERROR_TAG, "Failed to send Config Model App Bind");
            }
        }
        else if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND)
        {
            uint8_t match[8] = {0xcd, 0xdc};
            ESP_LOGI(MESH_DEBUG_TAG, "%s, Provision and config successfully", __func__);
            for (uint8_t i = 2; i < 8; i++)
            {
                match[i] = 0xff;
            }
            if (set_conf)
            {
                example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
                set.model_app_bind.element_addr = node->unicast_addr;
                set.model_app_bind.model_app_idx = prov_key.app_idx;
                set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SRV;
                set.model_app_bind.company_id = 0xffff;
                err = esp_ble_mesh_config_client_set_state(&common, &set);
            }
            set_conf = 0;

            err = esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(provision_t.macid), 0x0, true);
            vTaskDelay(20);
        }
        else if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_NODE_RESET)
        {
            esp_ble_mesh_sensor_client_cb_param_t params;
            struct net_buf_simple marshmell;
            uint8_t data[130];
            uint16_t element_addr = param->params->ctx.addr;
            data[0] = NODE_UNPROV_PACKET;
            params.status_cb.sensor_status.marshalled_sensor_data = &marshmell;
            params.status_cb.sensor_status.marshalled_sensor_data->data = data;
            ESP_LOGI(MESH_DEBUG_TAG, "NODE UNPROV ACK | FROM ELEMADDR : %d", element_addr);
            vTaskDelay(20);
            remove_from_unprov_queue_based_on_elemaddr(element_addr);
        }
        else
            break;
    case ESP_BLE_MESH_CFG_CLIENT_PUBLISH_EVT:
        if (param->params->opcode == ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_STATUS)
        {
            ESP_LOG_BUFFER_HEX("Composition data", param->status_cb.comp_data_status.composition_data->data,
                               param->status_cb.comp_data_status.composition_data->len);
        }
        break;
    case ESP_BLE_MESH_CFG_CLIENT_TIMEOUT_EVT:
        switch (param->params->opcode)
        {
        case ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET:
        {
            esp_ble_mesh_cfg_client_get_state_t get = {0};
            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET);
            get.comp_data_get.page = COMP_DATA_PAGE_0;
            err = esp_ble_mesh_config_client_get_state(&common, &get);
            if (err != ESP_OK)
            {
                ESP_LOGE(MESH_ERROR_TAG, "Failed to send Config Composition Data Get");
            }
            break;
        }
        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
            set.app_key_add.net_idx = prov_key.net_idx;
            set.app_key_add.app_idx = prov_key.app_idx;
            memcpy(set.app_key_add.app_key, prov_key.app_key, ESP_BLE_MESH_OCTET16_LEN);
            err = esp_ble_mesh_config_client_set_state(&common, &set);
            if (err != ESP_OK)
            {
                ESP_LOGE(MESH_ERROR_TAG, "Failed to send Config AppKey Add");
            }
            break;
        case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND:
            example_ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
            set.model_app_bind.element_addr = node->unicast_addr;
            set.model_app_bind.model_app_idx = prov_key.app_idx;
            set.model_app_bind.model_id = ESP_BLE_MESH_VND_MODEL_ID_SERVER;
            set.model_app_bind.company_id = CID_ESP;
            err = esp_ble_mesh_config_client_set_state(&common, &set);
            set.model_app_bind.model_id = ESP_BLE_MESH_MODEL_ID_SENSOR_SRV;
            set.model_app_bind.company_id = 0xffff;
            err = esp_ble_mesh_config_client_set_state(&common, &set);
            if (err != ESP_OK)
            {
                ESP_LOGE(MESH_ERROR_TAG, "Failed to send Config Model App Bind");
            }
            ESP_LOGE(MESH_ERROR_TAG, "Failed to send Config Model App Bind");
            break;
        default:
            break;
        }
        break;
    default:
        ESP_LOGE(MESH_ERROR_TAG, "Invalid config client event %u", event);
        break;
    }
}

void example_ble_mesh_send_vendor_message(bool resend)
{
    esp_ble_mesh_msg_ctx_t ctx = {0};
    ctx.net_idx = prov_key.net_idx;
    ctx.app_idx = prov_key.app_idx;
    ctx.addr = store.server_addr;
    ctx.send_ttl = MSG_SEND_TTL;
    ctx.send_rel = MSG_SEND_REL;
    uint32_t opcode = ESP_BLE_MESH_VND_MODEL_OP_SEND;

    if (resend == false)
    {
        store.vnd_tid++;
    }

    /*  err = esp_ble_mesh_client_model_send_msg(vendor_client.model, &ctx, opcode,
              sizeof(store.ac), (uint8_t *)&store.ac, MSG_TIMEOUT, true, MSG_ROLE);*/
    /* if (err != ESP_OK) {
         ESP_LOGE(MESH_ERROR_TAG, "Failed to send vendor message 0x%06" PRIx32, opcode);
         return;
     }*/

    mesh_example_info_store(); /* Store proper mesh example info */
}

static void example_ble_mesh_custom_model_cb(esp_ble_mesh_model_cb_event_t event,
                                             esp_ble_mesh_model_cb_param_t *param)
{
    static int64_t start_time;

    switch (event)
    {
    case ESP_BLE_MESH_MODEL_OPERATION_EVT:
        if (param->model_operation.opcode == ESP_BLE_MESH_VND_MODEL_OP_STATUS)
        {
            int64_t end_time = esp_timer_get_time();
            ESP_LOGI(MESH_DEBUG_TAG, "Recved 0x06%" PRIx32 ", tid 0x%04x, time %lldus",
                     param->model_operation.opcode, store.vnd_tid, end_time - start_time);
        }
        break;
    case ESP_BLE_MESH_MODEL_SEND_COMP_EVT:
        if (param->model_send_comp.err_code)
        {
            ESP_LOGE(MESH_ERROR_TAG, "Failed to send message 0x%06" PRIx32, param->model_send_comp.opcode);
            break;
        }
        start_time = esp_timer_get_time();
        ESP_LOGI(MESH_DEBUG_TAG, "Send 0x%06" PRIx32, param->model_send_comp.opcode);
        break;
    case ESP_BLE_MESH_CLIENT_MODEL_RECV_PUBLISH_MSG_EVT:
        ESP_LOGI(MESH_DEBUG_TAG, "Receive publish message 0x%06" PRIx32, param->client_recv_publish_msg.opcode);
        break;
    case ESP_BLE_MESH_CLIENT_MODEL_SEND_TIMEOUT_EVT:
        // int a=param->model_operation.model->user_data;
        ESP_LOGW(TAG, "Client message 0x%06" PRIx32 "   timeout  tid 0x%04x", param->client_send_timeout.opcode, store.vnd_tid);
        example_ble_mesh_send_vendor_message(true);
        break;
    default:
        break;
    }
}
void net_keys_handler()
{
    uint8_t *net_key_local;
    esp_ble_mesh_provisioner_add_local_net_key(NULL, 0xFFFF);
    net_key_local = esp_ble_mesh_provisioner_get_local_net_key(prov_key.net_idx);
    for (uint8_t i = 0; i < 16; i++)
    {
        prov_key.net_key[i] = net_key_local[i];
    }
}

void app_keys_handler()
{
    uint8_t *app_key_local;
    // err = esp_ble_mesh_provisioner_add_local_app_key(NULL, prov_key.net_idx, 0xFFFF);
    // app_key_local = esp_ble_mesh_provisioner_get_local_app_key(prov_key.net_idx,prov_key.app_idx);
    // for(uint8_t i=0; i<16 ; i++)
    // {
    //     prov_key.app_key[i] = app_key_local[i];

    // }
    err = esp_ble_mesh_provisioner_add_local_app_key(prov_key.app_key, prov_key.net_idx, prov_key.app_idx);
}
static esp_err_t ble_mesh_init(void)
{
    uint8_t match[8] = {0xcd, 0xdc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    // uint8_t match[2] = { 0xcd, 0xdc};
    esp_err_t err;

    prov_key.net_idx = ESP_BLE_MESH_KEY_PRIMARY;
    prov_key.app_idx = APP_KEY_IDX;
    memset(prov_key.app_key, APP_KEY_OCTET, sizeof(prov_key.app_key));

    esp_ble_mesh_register_prov_callback(example_ble_mesh_provisioning_cb);
    esp_ble_mesh_register_config_client_callback(example_ble_mesh_config_client_cb);
    esp_ble_mesh_register_custom_model_callback(example_ble_mesh_custom_model_cb);
    esp_ble_mesh_register_sensor_client_callback(example_ble_mesh_sensor_client_cb);
    err = esp_ble_mesh_init(&provision, &composition);
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Failed to initialize mesh stack");
        return err;
    }

    err = esp_ble_mesh_client_model_init(&vnd_models[0]);
    if (err)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Failed to initialize vendor client");
        return err;
    }

    err = esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match), 0x0, true);
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Failed to set matching device uuid");
        return err;
    }

    err = esp_ble_mesh_provisioner_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Failed to enable mesh provisioner");
        return err;
    }

    app_keys_handler();

    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Failed to add local AppKey");
        return err;
    }
    return ESP_OK;
}

void gwy_mesh_main_init(void)
{
    esp_err_t err;
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // board_init();
    err = bluetooth_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "esp32_bluetooth_init failed (err %d)", err);
        return;
    }

    /* Open nvs namespace for storing/restoring mesh example info */
    err = ble_mesh_nvs_open(&NVS_HANDLE);
    if (err)
    {
        return;
    }

    ble_mesh_get_dev_uuid(dev_uuid);
    // net_keys_handler();

    /* Initialize the Bluetooth Mesh Subsystem */
    err = ble_mesh_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(MESH_ERROR_TAG, "Bluetooth mesh init failed (err %d)", err);
    }
}

esp_err_t err;
uint32_t opcode;

void update_the_provisioner_net_key(uint8_t *netkey)
{
    esp_ble_mesh_provisioner_update_local_net_key(netkey, prov_key.net_idx);
}

void update_the_provisioner_app_key(uint8_t *appkey)
{
    esp_ble_mesh_provisioner_update_local_app_key(appkey, prov_key.net_idx, prov_key.app_idx);
}

/**
 * @brief Function that takes care resetting the match array that is used for provisioning
 * a node using its mac id. This array gets set with a mac id when we recv a prov request and stays
 * set even after that. Sometimes, this causes autonomous provisioning of the node when it is brough up
 * To prevent that, after a NODE COMM TIMEOUT occurs, we better clear off this match array
 * @param none
 * @retval
 *
 */
void zero_out_match_arr_in_mesh()
{
    uint8_t match[8] = {0xcd, 0xdc};
    for (uint8_t i = 2; i < 8; i++)
    {
        match[i] = 0xff;
    }
    err = esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match), 0x0, true);
}

void send_prov_packet_to_node(prov_t *prov_packet)
{
    uint8_t match[8] = {0xcd, 0xdc};
    for (uint8_t i = 2; i < 8; i++)
    {
        match[i] = provision_t.macid[i - 2];
    }
    err = esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match), 0x0, true);
}

void send_unprov_packet_to_node(unprov_t *unprov_packet)
{
    esp_ble_mesh_cfg_client_set_state_t set_rst = {0}, set_hb = {0}, set_pub_conf = {0};
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_node_t node;
    node.unicast_addr = unprov_packet->base_data.elementAddr;
    example_ble_mesh_set_msg_common(&common, &node, config_client.model, ESP_BLE_MESH_MODEL_OP_NODE_RESET);
    set_rst.model_app_bind.element_addr = unprov_packet->base_data.elementAddr;
    set_rst.model_app_bind.model_app_idx = prov_key.app_idx;
    set_rst.model_app_bind.company_id = CID_ESP;
    err = esp_ble_mesh_config_client_set_state(&common, &set_rst);
}

void send_reconf_packet_to_node(reconf_t *reconf_packet)
{
    opcode = ESP_BLE_MESH_VND_MODEL_OP_SEND;
    esp_ble_mesh_msg_ctx_t ctx = {0};
    ctx.net_idx = prov_key.net_idx;
    ctx.app_idx = prov_key.app_idx;
    ctx.send_ttl = MSG_SEND_TTL;
    ctx.send_rel = MSG_SEND_REL;
    store.vendor_node_reconf_t = node_reconf_t;
    store.server_addr = node_reconf_t.base_data.elementAddr;
    ctx.addr = store.server_addr;
    err = esp_ble_mesh_client_model_send_msg(vendor_client.model, &ctx, opcode,
                                             sizeof(store.vendor_node_reconf_t), (uint8_t *)&store.vendor_node_reconf_t, MSG_TIMEOUT, true, MSG_ROLE);
    mesh_example_info_store();
}

void send_ac_control_packet_to_node(control_t *control_packet)
{
    opcode = ESP_BLE_MESH_VND_MODEL_OP_SEND;
    esp_ble_mesh_msg_ctx_t ctx = {0};
    ctx.net_idx = prov_key.net_idx;
    ctx.app_idx = prov_key.app_idx;
    ctx.send_ttl = MSG_SEND_TTL;
    ctx.send_rel = MSG_SEND_REL;
    store.vendor_node_ac_control = node_ac_control_t;
    store.server_addr = node_ac_control_t.base_data.elementAddr;
    ctx.addr = store.server_addr;
    err = esp_ble_mesh_client_model_send_msg(vendor_client.model, &ctx, opcode,
                                             sizeof(store.vendor_node_ac_control), (uint8_t *)&store.vendor_node_ac_control, MSG_TIMEOUT, true, MSG_ROLE);
    mesh_example_info_store();
}

void send_pub_conf_packet_to_node(pub_conf_t *pub_conf_packet)
{
    opcode = ESP_BLE_MESH_VND_MODEL_OP_SEND;
    esp_ble_mesh_msg_ctx_t ctx = {0};
    ctx.net_idx = prov_key.net_idx;
    ctx.app_idx = prov_key.app_idx;
    ctx.send_ttl = MSG_SEND_TTL;
    ctx.send_rel = MSG_SEND_REL;
    store.vendor_node_heartbeat_pub_conf_t = node_heartbeat_pub_conf_t;
    store.server_addr = node_heartbeat_pub_conf_t.base_data.elementAddr;
    ctx.addr = store.server_addr;
    err = esp_ble_mesh_client_model_send_msg(vendor_client.model, &ctx, opcode,
                                             sizeof(store.vendor_node_heartbeat_pub_conf_t), (uint8_t *)&store.vendor_node_heartbeat_pub_conf_t, MSG_TIMEOUT, true, MSG_ROLE);
    mesh_example_info_store();
}

/**
 * @brief Function that takes care of sending teaching mode packet to the node
 * @param node_teaching_mode_packet Pointer to the packet
 * @retval none
 */
void send_teaching_mode_packet_to_node(teaching_mode_t *node_teaching_mode_packet)
{
    opcode = ESP_BLE_MESH_VND_MODEL_OP_SEND;
    esp_ble_mesh_msg_ctx_t ctx = {0};
    ctx.net_idx = prov_key.net_idx;
    ctx.app_idx = prov_key.app_idx;
    ctx.send_ttl = MSG_SEND_TTL;
    ctx.send_rel = MSG_SEND_REL;
    store.vendor_node_teaching_mode_t = node_teaching_mode_t;
    store.server_addr = node_teaching_mode_t.base_data.elementAddr;
    ctx.addr = store.server_addr;
    err = esp_ble_mesh_client_model_send_msg(vendor_client.model, &ctx, opcode,
                                             sizeof(store.vendor_node_teaching_mode_t), (uint8_t *)&store.vendor_node_teaching_mode_t, MSG_TIMEOUT, true, MSG_ROLE);
    mesh_example_info_store();
}

/**
 * @brief Function that takes care of sending debug info packet to node
 * @param none
 * @retval none
 */
void send_debug_info_packet_to_node(debug_info_t *debug_info_packet)
{
    opcode = ESP_BLE_MESH_VND_MODEL_OP_SEND;
    esp_ble_mesh_msg_ctx_t ctx = {0};
    ctx.net_idx = prov_key.net_idx;
    ctx.app_idx = prov_key.app_idx;
    ctx.send_ttl = MSG_SEND_TTL;
    ctx.send_rel = MSG_SEND_REL;
    store.vendor_node_debug_info_t = node_debug_info_t;
    store.server_addr = node_debug_info_t.base_data.elementAddr;
    ctx.addr = store.server_addr;
    err = esp_ble_mesh_client_model_send_msg(vendor_client.model, &ctx, opcode,
                                             sizeof(store.vendor_node_debug_info_t), (uint8_t *)&store.vendor_node_debug_info_t, MSG_TIMEOUT, true, MSG_ROLE);
    mesh_example_info_store();
}

#endif