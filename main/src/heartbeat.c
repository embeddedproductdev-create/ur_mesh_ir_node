
#include "esp_timer.h"
#include "esp_log.h"

#include <main.h>
#include <lte.h>
#include <ble.h>
#include <cJSON.h>
#include <flash.h>
#include <json_maker.h>

#define HB_TAG "HEARTBEAT"

static esp_timer_handle_t hb_timer_handle;

static void hb_callback(void *arg)
{
#if (IS_GWY)
    char *buffer = (char *)malloc(1024);
    if (!buffer)
    {
        ESP_LOGE(HB_TAG, "Memory Allocation failed for buffer | Can't send HB");
        return;
    }
    struct jWriteControl *jwc = (struct jWriteControl *)malloc(sizeof(struct jWriteControl));
    if (!jwc)
    {
        ESP_LOGE(HB_TAG, "Memory Allocation failed for jwc | Can't send HB");
        free(buffer);
        return;
    }

    jwOpen(jwc, buffer, 1024, JW_OBJECT, 1);
    jwObj_int(jwc, JSON_PACKET_ID_KEY, GWY_HEARTBEAT_ACK);
    jwObj_int(jwc, POWER_KEY, last_command.power);
    jwObj_string(jwc, MODE_KEY, last_command.mode_str);
    jwObj_int(jwc, FAN_SPEED_KEY, last_command.fanspeed);
    jwObj_int(jwc, TEMPERATURE_KEY, last_command.temperature);
    jwObj_int(jwc, AMBIENT_TEMPERATURE_DATA_KEY, last_command.ambientTemperature);
    jwObj_int(jwc, SWING_H_KEY, last_command.swingh);
    jwObj_int(jwc, SWING_V_KEY, last_command.swingv);
    jwObj_int(jwc, ONTIMER_KEY, last_command.ontimer);
    jwObj_int(jwc, OFFTIMER_KEY, last_command.offtimer);
    jwObj_int(jwc, AC_LOCKING_KEY, last_command.locking);
    jwObj_int(jwc, UPPER_TEMPERATURE_LIMIT_KEY, last_command.upperTemperatureLimit);
    jwObj_int(jwc, LOWER_TEMPERATURE_LIMIT_KEY, last_command.lowerTemperatureLimit);
    jwClose(jwc);
    enqueue_for_publish(buffer);
    free(jwc);
#endif

#if (!IS_GWY)
    send_ack_to_provisioner(NODE_HEARTBEAT_ACK, NULL);
#endif
}

/**
 * @brief Function that stops the hb timer
 *
 */
void hb_timer_stop()
{
    ESP_LOGI(HB_TAG, "Stopping HB Publishing");
    ESP_ERROR_CHECK(esp_timer_stop(hb_timer_handle));
}

/**
 * @brief Function that starts the hb timer
 *
 */
void hb_timer_start()
{
    ESP_LOGI(HB_TAG, "Starting HB Publishing");
    ESP_ERROR_CHECK(esp_timer_start_periodic(hb_timer_handle, publishPeriod * 1000000));
}

/**
 * @brief Function that restarts the hb timer
 *
 */
void hb_timer_restart()
{
    ESP_LOGI(HB_TAG, "Restarting HB Publishing");
    ESP_ERROR_CHECK(esp_timer_restart(hb_timer_handle, publishPeriod * 1000000));
}

esp_timer_create_args_t hb_timer_args = {
    .callback = hb_callback,
    .arg = NULL,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "LED Blink Timer"};

/**
 * @brief Function that initializes the heartbeat timer (but doesn't start)
 *
 */
void hb_init()
{
    ESP_ERROR_CHECK(esp_timer_create(&hb_timer_args, &hb_timer_handle));
    ESP_LOGI(HB_TAG, "HB Timer configuration successful");
}

/**
 * @brief Function that modifies the HB Publish Configuration frequency
 * Common function to both provisioner and node.
 *
 */
void handle_setting_hb_publish_configuration(CommandStruct *cmd_struct)
{
    if (publishPeriod == cmd_struct->publishPeriodSec)
        ;
    else
    {
        publishPeriod = cmd_struct->publishPeriodSec;
        set_number_in_nvs_flash(general_nvs_handle, NVS_PUBPERIOD_KEY, publishPeriod, UINT16_SIZE);
        hb_timer_restart();
    }
#if (!IS_GWY)
    send_ack_to_provisioner(cmd_struct->packetid, cmd_struct);
#endif
}