#include "nvs_flash.h"
#include "nvs.h"

/*Global Variables*/
extern nvs_handle_t ble_nvs_handle;
extern nvs_handle_t ir_nvs_handle;
extern nvs_handle_t general_nvs_handle;

extern const char *NVS_NEW_DEVICE_KEY;
extern const char *NVS_SERIAL_NO_KEY;
extern const char *NVS_DEVICE_LOCATION_KEY;
extern const char *NVS_REGISTERED_KEY;
extern const char *NVS_CONFIGURED_KEY;
extern const char *NVS_PROVISIONED_KEY;
extern const char *NVS_RAWLEN_KEY;
extern const char *NVS_IR_PROTOCOL_KEY;
extern const char *NVS_POWER_KEY;
extern const char *NVS_TEMPERATURE_KEY;
extern const char *NVS_FANSPEED_KEY;
extern const char *NVS_MODE_KEY;
extern const char *NVS_SWINGH_KEY;
extern const char *NVS_SWINGV_KEY;
extern const char *NVS_LOCKING_KEY;
extern const char *NVS_ONTIMER_KEY;
extern const char *NVS_OFFTIMER_KEY;
extern const char *NVS_UPPER_TEMPERATURE_LIMIT_KEY;
extern const char *NVS_LOWER_TEMPERATURE_LIMIT_KEY;
extern const char *NVS_PUBPERIOD_KEY;

typedef enum
{
    BLE_HANDLE,
    IR_HANDLE,
    GENERAL_HANDLE,
}handle_enum_t;

/*Function declarations*/
void nvs_init();
void set_number_in_nvs_flash(handle_enum_t nvshandle, const char *key, int value, sizes_t size);
void set_str_in_nvs_flash(handle_enum_t nvshandle, const char *key, char *value);
int32_t get_number_from_nvs_flash(handle_enum_t nvshandle, const char *key, sizes_t size);
char *get_str_from_nvs_flash(handle_enum_t nvshandle, const char *key);