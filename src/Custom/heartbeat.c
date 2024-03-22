/**
 * @file heartbeat.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to sending
 * heartbeat packets to cloud to let it know if a device is
 * alive in the network or not
 * @version 0.1
 * @date 2024-03-19
 * @copyright Copyright (c) 2024
 */

#include "../../inc/Custom/heartbeat.h"

//Initialization
uint32_t HBFreqInSec = 5;

static void publish_HB_cb(void* arg)
{
    ESP_LOGI(DEBUG_TAG, "Sending Gwy Heartbeat Ackr\r\n");
    char pubmessage[PUBMESG_LEN];
    sprintf(pubmessage, "%s : %d, %s : %s, %s : %d",
    JSON_PACKET_ID, GWY_HB_PACKET,
    JSON_ACK_NAME, GWY_HB_ACK,
    GWYSERNO_STR, GWY_SER_NO);
    add_to_pubmesg_queue(pubmessage, publish_topic);
}

/**
 * @brief Thread that takes care of sending HB messages periodically
 * to cloud to let it know if the device is alive or dead
 * @param args
 * @return void*
 */
void *HB_task(void *args)
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &publish_HB_cb,
        .name = "HB_Timer"
    };
    esp_timer_handle_t hb_publish_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &hb_publish_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(hb_publish_timer, HBFreqInSec*1000000));
    while(1)
    {
        vTaskDelay(1);
        ;
    }
    ESP_ERROR_CHECK(esp_timer_delete(hb_publish_timer));
    ESP_ERROR_CHECK(esp_timer_delete(hb_publish_timer));
}