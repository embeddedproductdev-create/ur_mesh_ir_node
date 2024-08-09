#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "string.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_insights.h"
#include "esp_diagnostics_system_metrics.h"
#include "esp_rmaker_utils.h"
#include "esp_system.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "freertos/event_groups.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../inc/Custom/esp_insights.h"

const char insights_auth_key_start[] = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyIjoiYWZhNjgzZDctMTk5Yi00MWI2LTk2OTMtNTIwMWY0OWQxZGZiIiwiaXNzIjoiZTMyMmI1OWMtNjNjYy00ZTQwLThlYTItNGU3NzY2NTQ1Y2NhIiwic3ViIjoiZGVlMTk4MmUtZWM5Yi00NDhjLWJjZTQtYjNjZTRkMjIwNjZjIiwiZXhwIjoyMDM4NTQ5MzY5LCJpYXQiOjE3MjMxODkzNjl9.hRL982aFJ79ft77oikVZkzOxaFmmI3uaJP9TS8KWli-eSFLISBqf9xKIe7x5pwqHM2FFJqekROvwRAQ7NrN6rkDYmNSd09JX5BqQZNRyTnJN269D9oiK6gzBhEyPgCNvjuDRWx-3Kcq_2USy3PBvryX-PkehNacSS2u2N4Bs0Ny9b3m3bFfpnn3mJFX4e8yq5TKisw7bNAsywQjRMQ02_Hlasq6AxJ__qAwKyM9gHhbKiic2ZEbwVGHqCfJ1xgVI_2tXXrRkBYlUCZefBfWoLT-3LgkNtv8du0nZ7plGDTH45dEqjM0FeHxNCIRCfYCEkWPcchLIcvNIfhtTG6RZvg";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static const char *TAG = "wifi station";
static int s_retry_num = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void connect_to_wifi()
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            // .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            // .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            // .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", WIFI_SSID, WIFI_PASSWORD);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", WIFI_SSID, WIFI_PASSWORD);
    }
    else ESP_LOGE(TAG, "UNEXPECTED EVENT");
}

/**
 * @brief Thread function to take care of ESP insights
 * @param args
 * @retval none
 */
void ESP_insights_task(void *args)
{
    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    connect_to_wifi();

    /* This initializes SNTP for time synchronization.
     * ESP Insights uses relative time since bootup if time is not synchronized and
     * epoch since 1970 if time is synsynchronized.
     */
    esp_rmaker_time_sync_init(NULL);

    esp_insights_config_t config = {
        .log_type = ESP_DIAG_LOG_TYPE_ERROR | ESP_DIAG_LOG_TYPE_WARNING | ESP_DIAG_LOG_TYPE_EVENT,
#ifdef CONFIG_ESP_INSIGHTS_TRANSPORT_HTTPS
        .auth_key = insights_auth_key_start,
#endif
    };
    ESP_LOGI(TAG, "ESP Insights Initializing ...");
    ret = esp_insights_init(&config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize ESP Insights, err:0x%x", ret);
    }
    else
    ESP_LOGI(TAG, "ESP Insights Initialized successfully");
    ESP_ERROR_CHECK(ret);

    /* Following code generates an example error and logs it */
    nvs_handle_t handle;
    ret = nvs_open("unknown", NVS_READONLY, &handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Test error: API nvs_open() failed, error:0x%x", ret);
    }

    /* Please make sure CONFIG_DIAG_ENABLE_METRICS, CONFIG_DIAG_ENABLE_WIFI_METRICS, and CONFIG_DIAG_ENABLE_HEAP_METRICS
     * config options are enabled in order to use esp_diag_heap_metrics_dump() and esp_diag_wifi_metrics_dump() APIs.
     *
     * Enabling the config options CONFIG_DIAG_ENABLE_HEAP_METRICS and CONFIG_DIAG_ENABLE_WIFI_METRICS are enough
     * to start reporting heap and wifi metrics respectively. Following is done to demostrate the use of
     * esp_diag_heap_metrics_dump() and esp_diag_wifi_metrics_dump() APIs and view good graphs on the dashboard.
     */
    while (1)
    {
        esp_diag_heap_metrics_dump();
        esp_diag_wifi_metrics_dump();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
