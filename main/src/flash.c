#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#include "../inc/main.h"
#include "../inc/lte.h"

#define NVS_TAG "NVS"

#define BLE_NVS_PARTITION_NAME "ble"
#define IR_NVS_PARTITION_NAME "ir"
#define GENERAL_NVS_PARTITION_NAME "general"

#define BLE_NVS_NAMESPACE "BLE"
#define IR_NVS_NAMESPACE "IR"
#define GENERAL_NVS_NAMESPACE "GENERAL"

nvs_handle_t ble_nvs_handle;
nvs_handle_t ir_nvs_handle;
nvs_handle_t general_nvs_handle;

const char *NVS_NEW_DEVICE_KEY = "NewDevice";
const char *NVS_SERIAL_NO_KEY = "SerialNo";
const char *NVS_DEVICE_LOCATION_KEY = "Location";
const char *NVS_REGISTERED_KEY = "Registered";
const char *NVS_CONFIGURED_KEY = "Configured";
const char *NVS_PROVISIONED_KEY = "Provisioned";
const char *NVS_RAWLEN_KEY = "RawLen";
const char *NVS_IR_PROTOCOL_KEY = "IrProtocol";
const char *NVS_POWER_KEY = "Power";
const char *NVS_TEMPERATURE_KEY = "Temperature";
const char *NVS_FANSPEED_KEY = "FanSpeed";
const char *NVS_MODE_KEY = "Mode";
const char *NVS_SWINGH_KEY = "SwingH";
const char *NVS_SWINGV_KEY = "SwingV";
const char *NVS_LOCKING_KEY = "Locking";
const char *NVS_ONTIMER_KEY = "OnTimer";
const char *NVS_OFFTIMER_KEY = "OffTimer";
const char *NVS_UPPER_TEMPERATURE_LIMIT_KEY = "UTL";
const char *NVS_LOWER_TEMPERATURE_LIMIT_KEY = "LTL";
const char *NVS_PUBPERIOD_KEY = "PubPeriod";

/**
 * @brief Set the number in nvs flash
 * @param handle 
 * @param key 
 * @param value 
 */
void set_number_in_nvs_flash(nvs_handle_t handle, const char *key, int value, sizes_t size)
{
    nvs_open_from_partition(GENERAL_NVS_PARTITION_NAME, GENERAL_NVS_NAMESPACE, NVS_READWRITE, &general_nvs_handle);

    switch(size)
    {
        case INT8:
            nvs_set_i8(handle, key, (int8_t)value);
            break;

        case UINT8:
            nvs_set_u8(handle, key, (uint8_t)value);
            break;

        case INT16:
            nvs_set_i16(handle, key, (int16_t)value);
            break;

        case UINT16:
            nvs_set_u16(handle, key, (uint16_t)value);
            break;

        case INT32:
            nvs_set_i32(handle, key, (int32_t)value);
            break;

        case UINT32:
            nvs_set_u32(handle, key, (uint32_t)value);
            break;

        case INT64:
            break;

        case UINT64:
            break;

        default:
            break;
    }
    
    nvs_commit(general_nvs_handle);
    nvs_close(general_nvs_handle);
}

/**
 * @brief Set the str in nvs flash
 * @param handle 
 * @param key 
 * @param value 
 */
void set_str_in_nvs_flash(nvs_handle_t *handle, const char *key, char *value)
{
    nvs_open_from_partition(GENERAL_NVS_PARTITION_NAME, GENERAL_NVS_NAMESPACE, NVS_READWRITE, &general_nvs_handle);
    
    nvs_set_str(*handle, key, value);
    
    nvs_commit(general_nvs_handle);
    nvs_close(general_nvs_handle);
}

/**
 * @brief Get the number from nvs flash
 * @param handle 
 * @param key 
 * @return int32_t 
 */
int32_t get_number_from_nvs_flash(nvs_handle_t *handle, const char *key, sizes_t size)
{
    nvs_open_from_partition(GENERAL_NVS_PARTITION_NAME, GENERAL_NVS_NAMESPACE, NVS_READWRITE, &general_nvs_handle);
    
    switch(size)
    {
        case INT8:
            break;

        case UINT8:
            break;

        case INT16:
            break;

        case UINT16:
            break;

        case INT32:
            break;

        case UINT32:
            break;

        case INT64:
            break;

        case UINT64:
            break;

        default:
            break;
    }
    
    nvs_close(general_nvs_handle);
    return 0;
}

/**
 * @brief Get the str from nvs flash
 * @param handle 
 * @param key 
 * @return char* 
 */
char *get_str_from_nvs_flash(nvs_handle_t *handle, const char *key)
{
    nvs_open_from_partition(GENERAL_NVS_PARTITION_NAME, GENERAL_NVS_NAMESPACE, NVS_READWRITE, &general_nvs_handle);
    
    nvs_close(general_nvs_handle);
    return "";
}

/**
 * @brief Function that initializes the nvs flash with defaults
 * @return esp_err_t 
 */
esp_err_t init_data_in_nvs(void)
{
    nvs_open_from_partition(BLE_NVS_PARTITION_NAME, BLE_NVS_NAMESPACE, NVS_READWRITE, &ble_nvs_handle);
    nvs_open_from_partition(IR_NVS_PARTITION_NAME, IR_NVS_NAMESPACE, NVS_READWRITE, &ir_nvs_handle);
    nvs_open_from_partition(GENERAL_NVS_PARTITION_NAME, GENERAL_NVS_NAMESPACE, NVS_READWRITE, &general_nvs_handle);

    nvs_set_u16(ir_nvs_handle, NVS_RAWLEN_KEY, teaching_mode_raw_len);
    nvs_set_i16(ir_nvs_handle, NVS_IR_PROTOCOL_KEY, ir_protocol_num);

    nvs_set_str(general_nvs_handle, NVS_SERIAL_NO_KEY, DEFAULT_DEVICE_SER_NO);
    nvs_set_u8(general_nvs_handle, NVS_REGISTERED_KEY, registered);
    nvs_set_u8(general_nvs_handle, NVS_CONFIGURED_KEY, configured);
    nvs_set_str(general_nvs_handle, NVS_DEVICE_LOCATION_KEY, DEFAULT_DEVICE_LOCATION_STR);
    nvs_set_u16(general_nvs_handle, NVS_PUBPERIOD_KEY, publishPeriod);

    nvs_set_u8(general_nvs_handle, NVS_POWER_KEY, last_command.power);
    nvs_set_u8(general_nvs_handle, NVS_TEMPERATURE_KEY, last_command.temperature);
    nvs_set_u8(general_nvs_handle, NVS_FANSPEED_KEY, last_command.fanspeed);
    nvs_set_str(general_nvs_handle, NVS_MODE_KEY, COOL_MODE_STR);
    nvs_set_u8(general_nvs_handle, NVS_SWINGH_KEY, last_command.swingh);
    nvs_set_u8(general_nvs_handle, NVS_SWINGV_KEY, last_command.swingv);
    nvs_set_u16(general_nvs_handle, NVS_ONTIMER_KEY, last_command.ontimer);
    nvs_set_u16(general_nvs_handle, NVS_OFFTIMER_KEY, last_command.offtimer);
    nvs_set_u8(general_nvs_handle, NVS_LOCKING_KEY, last_command.locking);
    nvs_set_u8(general_nvs_handle, NVS_UPPER_TEMPERATURE_LIMIT_KEY, last_command.upperTemperatureLimit);
    nvs_set_u8(general_nvs_handle, NVS_LOWER_TEMPERATURE_LIMIT_KEY, last_command.lowerTemperatureLimit);

    nvs_commit(ble_nvs_handle);
    nvs_commit(ir_nvs_handle);
    nvs_commit(general_nvs_handle);

    nvs_close(ble_nvs_handle);
    nvs_close(ir_nvs_handle);
    nvs_close(general_nvs_handle);
    return ESP_OK;
}

/**
 * @brief Function that pulls data from NVS flash. If no data present, 
 * meaning its a fresh device, then initializes the defaults to NVS
 * @return esp_err_t 
 */
esp_err_t pull_data_from_nvs(void)
{
    nvs_open_from_partition(BLE_NVS_PARTITION_NAME, BLE_NVS_NAMESPACE, NVS_READWRITE, &ble_nvs_handle);
    nvs_open_from_partition(IR_NVS_PARTITION_NAME, IR_NVS_NAMESPACE, NVS_READWRITE, &ir_nvs_handle);
    nvs_open_from_partition(GENERAL_NVS_PARTITION_NAME, GENERAL_NVS_NAMESPACE, NVS_READWRITE, &general_nvs_handle);
    
    nvs_get_u16(ir_nvs_handle, NVS_RAWLEN_KEY, &teaching_mode_raw_len);
    nvs_get_i16(ir_nvs_handle, NVS_IR_PROTOCOL_KEY, &ir_protocol_num);

    size_t serialNoReqSize = 10;
    nvs_get_str(general_nvs_handle, NVS_SERIAL_NO_KEY, serialNoStr, &serialNoReqSize);
    nvs_get_u8(general_nvs_handle, NVS_REGISTERED_KEY, &registered);
    nvs_get_u8(general_nvs_handle, NVS_CONFIGURED_KEY, &configured);
    size_t locationReqSize = LOCATION_STR_LEN;
    nvs_get_str(general_nvs_handle, NVS_DEVICE_LOCATION_KEY, device_location_str, &locationReqSize);
    nvs_get_u16(general_nvs_handle, NVS_PUBPERIOD_KEY, &publishPeriod);

    nvs_get_u8(general_nvs_handle, NVS_POWER_KEY, &last_command.power);
    nvs_get_u8(general_nvs_handle, NVS_TEMPERATURE_KEY, &last_command.temperature);
    nvs_get_u8(general_nvs_handle, NVS_FANSPEED_KEY, &last_command.fanspeed);
    size_t modeReqSize = MAX_MODE_STR_LEN;
    nvs_get_str(general_nvs_handle, NVS_MODE_KEY, last_command.mode_str, &modeReqSize);
    nvs_get_u8(general_nvs_handle, NVS_SWINGH_KEY, &last_command.swingh);
    nvs_get_u8(general_nvs_handle, NVS_SWINGV_KEY, &last_command.swingv);
    nvs_get_u16(general_nvs_handle, NVS_ONTIMER_KEY, &last_command.ontimer);
    nvs_get_u16(general_nvs_handle, NVS_OFFTIMER_KEY, &last_command.offtimer);
    nvs_get_u8(general_nvs_handle, NVS_LOCKING_KEY, &last_command.locking);
    nvs_get_u8(general_nvs_handle, NVS_UPPER_TEMPERATURE_LIMIT_KEY, &last_command.upperTemperatureLimit);
    nvs_get_u8(general_nvs_handle, NVS_LOWER_TEMPERATURE_LIMIT_KEY, &last_command.lowerTemperatureLimit);
    
    // Close
    nvs_close(ble_nvs_handle);
    nvs_close(ir_nvs_handle);
    nvs_close(general_nvs_handle);
    return ESP_OK;
}

/**
 * @brief Function that checks if a device is brand new
 * @return true 
 * @return false 
 */
bool isNewDevice()
{
    nvs_open_from_partition(GENERAL_NVS_PARTITION_NAME, GENERAL_NVS_NAMESPACE, NVS_READWRITE, &general_nvs_handle);
    nvs_get_u8(general_nvs_handle, NVS_REGISTERED_KEY, &newDevice);
    nvs_close(general_nvs_handle);
    if(newDevice) return true;
    else return false;
}

/**
 * @brief Function that initializes nvs flash partitions.
 * 
 */
void nvs_init()
{
    esp_err_t err;

    err = nvs_flash_init_partition(BLE_NVS_PARTITION_NAME);
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_LOGE(NVS_TAG, "Err in %s NVS partition. Erasing data", BLE_NVS_PARTITION_NAME);
        ESP_ERROR_CHECK(nvs_flash_erase_partition(BLE_NVS_PARTITION_NAME));
        err = nvs_flash_init_partition(BLE_NVS_PARTITION_NAME);
    }

    err = nvs_flash_init_partition(IR_NVS_PARTITION_NAME);
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_LOGE(NVS_TAG, "Err in %s NVS partition. Erasing data", IR_NVS_PARTITION_NAME);
        ESP_ERROR_CHECK(nvs_flash_erase_partition(IR_NVS_PARTITION_NAME));
        err = nvs_flash_init_partition(IR_NVS_PARTITION_NAME);
    }

    err = nvs_flash_init_partition(GENERAL_NVS_PARTITION_NAME);
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_LOGE(NVS_TAG, "Err in %s NVS partition. Erasing data", GENERAL_NVS_PARTITION_NAME);
        ESP_ERROR_CHECK(nvs_flash_erase_partition(GENERAL_NVS_PARTITION_NAME));
        err = nvs_flash_init_partition(GENERAL_NVS_PARTITION_NAME);
    }

    if(isNewDevice()) ESP_LOGI(NVS_TAG, "Defaulting data in NVS flash : %s", esp_err_to_name(init_data_in_nvs()));
    ESP_LOGI(NVS_TAG, "Pulling data from NVS flash : %s", esp_err_to_name(pull_data_from_nvs()));          

    // Example of nvs_get_stats() to get overview of actual statistics of data entries :
    nvs_stats_t nvs_stats;
    
    nvs_get_stats(GENERAL_NVS_PARTITION_NAME, &nvs_stats);
    ESP_LOGI(NVS_TAG, "=-=-=-=-=-=-=-=-=-=- GENERAL_PARTITION_STATS -=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(NVS_TAG, "UsedEntries : (%d)", nvs_stats.used_entries); 
    ESP_LOGI(NVS_TAG, "FreeEntries : (%d)", nvs_stats.free_entries);
    // ESP_LOGI(NVS_TAG, "AvailableEntries : (%lu)", nvs_stats.available_entries);
    ESP_LOGI(NVS_TAG, "AllEntries : (%d)", nvs_stats.total_entries);

    nvs_get_stats(BLE_NVS_PARTITION_NAME, &nvs_stats);
    ESP_LOGI(NVS_TAG, "=-=-=-=-=-=-=-=-=-=- BLE_PARTITION_STATS -=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(NVS_TAG, "UsedEntries : (%d)", nvs_stats.used_entries); 
    ESP_LOGI(NVS_TAG, "FreeEntries : (%d)", nvs_stats.free_entries);
    // ESP_LOGI(NVS_TAG, "AvailableEntries : (%lu)", nvs_stats.available_entries);
    ESP_LOGI(NVS_TAG, "AllEntries : (%d)", nvs_stats.total_entries);

    nvs_get_stats(IR_NVS_PARTITION_NAME, &nvs_stats);
    ESP_LOGI(NVS_TAG, "=-=-=-=-=-=-=-=-=-=- IR_PARTITION_STATS -=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(NVS_TAG, "UsedEntries : (%d)", nvs_stats.used_entries); 
    ESP_LOGI(NVS_TAG, "FreeEntries : (%d)", nvs_stats.free_entries);
    // ESP_LOGI(NVS_TAG, "AvailableEntries : (%lu)", nvs_stats.available_entries);
    ESP_LOGI(NVS_TAG, "AllEntries : (%d)", nvs_stats.total_entries);
}