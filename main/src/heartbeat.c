
#include "esp_timer.h"
#include "esp_log.h"

#include <main.h>
#include <lte.h>
#include <ble_new.h>
#include <cJSON.h>
#include <flash.h>
#include <json_maker.h>

#define HB_TAG "HEARTBEAT"

#define ONE_SEC_IN_MU_SEC 1000000

static esp_timer_handle_t hb_timer_handle;

static void hb_callback(void *arg)
{
#if (IS_GWY)
    generate_ack(GWY_HEARTBEAT_ACK, NULL);
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
    if(!esp_timer_is_active(hb_timer_handle)){
        ESP_LOGE(HB_TAG, "Can't stop timer that is not running");
        return;
    }
    ESP_LOGI(HB_TAG, "Stopping HB Publishing");
    ESP_ERROR_CHECK(esp_timer_stop(hb_timer_handle));
}

/**
 * @brief Function that starts the hb timer
 *
 */
void hb_timer_start()
{
    if(esp_timer_is_active(hb_timer_handle)) {
        ESP_LOGE(HB_TAG, "Can't start already running timer");
        return;
    }
    ESP_LOGI(HB_TAG, "Starting HB Publishing");
    ESP_ERROR_CHECK(esp_timer_start_periodic(hb_timer_handle, publishPeriod * ONE_SEC_IN_MU_SEC));
}

/**
 * @brief Function that restarts the hb timer
 *
 */
void hb_timer_restart()
{
    if(esp_timer_is_active(hb_timer_handle));
    else {
        ESP_LOGE(HB_TAG, "HB Timer is not active. Can't restart. Starting instead ... ");
        hb_timer_start();
        return;
    }
    ESP_LOGI(HB_TAG, "Restarting HB Publishing");
    ESP_ERROR_CHECK(esp_timer_restart(hb_timer_handle, publishPeriod * ONE_SEC_IN_MU_SEC));
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
    if(registered || provisioned) hb_timer_start();
}

/**
 * @brief Function that modifies the HB Publish Configuration frequency
 * Common function to both provisioner and node.
 *
 */
void handle_setting_hb_publish_configuration(uint16_t newPublishPeriodSec)
{
    if (publishPeriod == newPublishPeriodSec); //If we receive the same publishperiod, do nothing.
    else
    {
        publishPeriod = newPublishPeriodSec;
        set_number_in_nvs_flash(general_nvs_handle, NVS_PUBPERIOD_KEY, publishPeriod, UINT16_SIZE);
        hb_timer_restart();
    }
}