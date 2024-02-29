#include "mesh_main.h"

prov prov_t = {};

static uint8_t  dev_uuid[ESP_BLE_MESH_OCTET16_LEN];
static uint16_t server_address = ESP_BLE_MESH_ADDR_UNASSIGNED;
static uint16_t sensor_prop_id;
esp_timer_handle_t reset_timer;
static esp_ble_mesh_client_t config_client;
static esp_ble_mesh_client_t sensor_client;
static esp_ble_mesh_client_t onoff_client;

static esp_ble_mesh_cfg_srv_t config_server = {
    .beacon = ESP_BLE_MESH_BEACON_DISABLED,
#if defined(CONFIG_BLE_MESH_FRIEND)
    .friend_state = ESP_BLE_MESH_FRIEND_ENABLED,
#else
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
#endif
    .default_ttl = 7,
    /* 3 transmissions with 20ms interval */
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
};

ESP_BLE_MESH_MODEL_PUB_DEFINE(sensorCliPub, 10, ROLE_PROVISIONER);
ESP_BLE_MESH_MODEL_PUB_DEFINE(onoffCliPub, 10, ROLE_PROVISIONER);
static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_CFG_CLI(&config_client),
    ESP_BLE_MESH_MODEL_SENSOR_CLI(NULL, &sensor_client),
	ESP_BLE_MESH_MODEL_GEN_ONOFF_CLI(&onoffCliPub, &onoff_client),
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
    .prov_uuid          = dev_uuid,
    .prov_unicast_addr  = PROV_OWN_ADDR,
    .prov_start_address = 0x0005,
};

static nvs_handle_t NVS_PROV_HANDLE;
const char nvsProvName[] = "nvs_prov";
static const char * NVS_PROV_KEY = "IR_nvs_prov";
static const char* NVS_PROV_POI = "provNvs";

static struct {
	uint16_t lastAssignedUniAddr;
	uint8_t tid;
} __attribute__((packed)) nvsProv_t = {
	.lastAssignedUniAddr = 0x0005,
	.tid = 0,
};

esp_err_t nvsPartitionRestore(nvs_handle_t handle, const char *key, void *data, size_t length, bool *exist)
{
    esp_err_t err = ESP_OK;

    if (key == NULL || data == NULL || length == 0) {
        ESP_LOGE(TAG, "Restore, invalid parameter");
        return ESP_ERR_INVALID_ARG;
    }

    err = nvs_get_blob(handle, key, data, &length);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Restore, key \"%s\" not exists", key);
        if (exist) {
            *exist = false;
        }
        return ESP_OK;
    }

    if (exist) {
        *exist = true;
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Restore, nvs_get_blob failed, err %d", err);
    } else {
        ESP_LOGI(TAG, "nvs_prov key \"%s\", nvs_prov length %u", key, length);
        ESP_LOG_BUFFER_HEX("nvs_prov: Restore, data", data, length);
    }

    return err;
}

static void nvsProvInit(void)
{
	esp_err_t err;

	ESP_LOGI(TAG, "\n\nINITIALISING nvs_prov PARTITION!!!");
	err = nvs_flash_init_partition(nvsProvName);
    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
		ESP_LOGE(TAG, "OUT OF MEMORY IN nvs_prov!! ERASING FLASH!!");
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init_partition(nvsProvName);
    }
    ESP_ERROR_CHECK(err);
    esp_err_to_name(err);

    err = nvs_open_from_partition(nvsProvName, NVS_PROV_POI, NVS_READWRITE, &NVS_PROV_HANDLE);
    if (err) {
    	ESP_LOGE(TAG, "ERROR IN OPENING BLE MESH NVS HANDLE");
        ESP_ERROR_CHECK(err);
        esp_err_to_name(err);
        return;
    }
}

/*
 * @brief - Restores user related data to the nvs section - Function that is called
 * 		    @nvsExtInfo nvsExtStore struct is restored
 */
static void nvsPartitionRestore_prov()
{
    esp_err_t err = ESP_OK;
    bool exist = false;

    ESP_LOGI(TAG, "\n\n");
    ESP_LOGI(TAG, "RESTORING NVS_PROV PARTITION!!");
    err = nvsPartitionRestore(NVS_PROV_HANDLE, NVS_PROV_KEY, &nvsProv_t, sizeof(nvsProv_t), &exist);
    if (err != ESP_OK) {
    	ESP_LOGE(TAG, "Not able to restore nvs_prov partition!!");
        return;
    }
    ESP_ERROR_CHECK(err);
    esp_err_to_name(err);

    if(exist) {
    	ESP_LOGI(TAG, "nvs_prov data exists!!");
    	ESP_LOGI(TAG, "Last Unicast address assigned is - 0x%04x, tid - %d", nvsProv_t.lastAssignedUniAddr, nvsProv_t.tid);
    	ESP_LOGI(TAG, "Provision start address before - 0x%04x", provision.prov_start_address);
    	provision.prov_start_address = nvsProv_t.lastAssignedUniAddr;
    	ESP_LOGI(TAG, "Provision start address after - 0x%04x", provision.prov_start_address);
    } else {
    	ESP_LOGE(TAG, "nvs_prov does not exist after opening!!");
    }
    //ESP_LOGI(TAG, "\n\n");
}

static void reset_timer_cb()
{
	prov_t.resetCount++;
}

void gpioInit()
{
	gpio_config_t gpio;
	gpio.mode = GPIO_MODE_INPUT;
	gpio.intr_type = GPIO_INTR_DISABLE;
	gpio.pin_bit_mask = MSK_GPIO_RESET;
	gpio.pull_down_en = 0;
	gpio.pull_up_en = 1;
	gpio_config(&gpio);
}

void timerInit()
{
	const esp_timer_create_args_t reset_timer_args = {
            .callback = &reset_timer_cb,
            .name = "reset_timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&reset_timer_args, &reset_timer));
}

void handle_cloud_packets(enum json_packet_enum json_packet_id)
{
    switch(json_packet_id)
    {
        case GWY_REG_PACKET:
            break;
        case GWY_CONF_PACKET:
            break;
        case GWY_UNREG_PACKET:
            break;
        case GWY_AC_CONTROL_PACKET:
            break;
        case GWY_AC_LOCKING_PACKET:
            break;
        case GWY_RECONF_PACKET:
            break;
        case NODE_PROV_PACKET:
            provision_with_mac();
            break;
        case NODE_CONF_PACKET:
            break;
        case NODE_UNPROV_PACKET:
            unprovision_with_elemaddr();
            break;
        case NODE_AC_CONTROL_PACKET:
            break;
        case NODE_AC_LOCKING_PACKET:
            break;
        case NODE_RECONF_PACKET:
            break;
        default:
            ESP_LOGE(TAG, "Unknown cloud packet\r\n");
    }
}

static esp_err_t ble_mesh_init(void)
{
    uint8_t match[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_err_t err = ESP_OK;
    uint8_t macAddr[6] = {};

    esp_read_mac(macAddr, ESP_MAC_BT);
    ESP_LOGI(TAG, "MAC_ADDR: %02x %02x %02x %02x %02x %02x", macAddr[0], macAddr[1], macAddr[2], \
    															macAddr[3], macAddr[4], macAddr[5]);
    uint8_t appKeyToBeSet[16] = {macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5], \
    								0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12};
    prov_key.net_idx = ESP_BLE_MESH_KEY_PRIMARY;
    prov_key.app_idx = APP_KEY_IDX;
    memcpy(prov_key.app_key, appKeyToBeSet, 16);

    esp_ble_mesh_register_prov_callback(example_ble_mesh_provisioning_cb);
    esp_ble_mesh_register_config_client_callback(example_ble_mesh_config_client_cb);
    esp_ble_mesh_register_config_server_callback(example_ble_mesh_config_server_cb);
    esp_ble_mesh_register_sensor_client_callback(example_ble_mesh_sensor_client_cb);
    esp_ble_mesh_register_generic_client_callback(example_ble_mesh_generic_client_cb);


    err = esp_ble_mesh_init(&provision, &composition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize mesh stack");
        return err;
    }

    err = esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match), 0x6, false);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set matching device uuid");
        return err;
    }

    err = esp_ble_mesh_provisioner_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable mesh provisioner");
        return err;
    }

    err = esp_ble_mesh_provisioner_add_local_app_key(prov_key.app_key, prov_key.net_idx, prov_key.app_idx);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add local AppKey");
        return err;
    }

    ESP_LOG_BUFFER_HEX(TAG, prov_key.app_key, 16);

    ESP_LOGI(TAG, "BLE Mesh sensor client initialized");

    return ESP_OK;
}

void mesh_init()
{
    esp_err_t err = ESP_OK;

    gpioInit();
    ESP_LOGI(TAG, "Initializing...");

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    nvsProvInit();
    nvsPartitionRestore_prov();

    timerInit();
    board_init();

    err = bluetooth_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp32_bluetooth_init failed (err %d)", err);
        return;
    }

    ble_mesh_get_dev_uuid(dev_uuid);

    /* Initialize the Bluetooth Mesh Subsystem */
    err = ble_mesh_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth mesh init failed (err %d)", err);
    }
}