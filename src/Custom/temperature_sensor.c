/**
 * @file read_temperature.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions related to the I2C read of the temperature sensor
 * @version 0.1
 * @date 2024-02-28
 * @link https://www.ti.com/lit/ds/symlink/tmp1075.pdf <-- Temperature sensor Datasheet link
 * @copyright Copyright (c) 2024
 *
 */

#include "../../inc/Custom/temperature_sensor.h"
// #include <Wire.h>



//Initialization
uint8_t measured_temperature = 0;
uint32_t TempDataFreqSec = 10;

/**
 * @brief Function that performs initial I2C setup
 * @param none
 * @retval none
 */
void I2C_inital_setup()
{
//   Wire.begin(); // Initiate the Wire library
//   delay(100);
    ;
}

void get_temperature(uint8_t *temp)
{
    *temp = 25;
}

static void publish_temperature_cb(void *arg)
{
    if(registered)
    {
        ESP_LOGI(DEBUG_TAG, "Sending Gwy Temperature Ack\r\n");
        get_temperature(&measured_temperature);
        char pubmessage[PUBMESG_LEN];
        sprintf(pubmessage, "%s : %d, %s : %s, %s : %d, %s : %d",
        JSON_PACKET_ID, GWY_TEMPERATURE_DATA_PACKET,
        JSON_ACK_NAME, GWY_TEMPERATURE_DATA_ACK,
        GWYSERNO_STR, GWY_SER_NO,
        TEMPERATURE_DATA_STR, measured_temperature);
        add_to_pubmesg_queue(pubmessage, publish_topic);
    }
}

/**
 * @brief Thread that performs the I2C temperature read communications
 * @param args
 * @return void*
 */
void *temperature_read(void *args)
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &publish_temperature_cb,
        .name = "Temperature_data_timer"
    };
    esp_timer_handle_t temp_publish_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &temp_publish_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(temp_publish_timer, TempDataFreqSec*1000000));
    while(1)
    {
        ;
        vTaskDelay(1);
    }
    ESP_ERROR_CHECK(esp_timer_delete(temp_publish_timer));
    ESP_ERROR_CHECK(esp_timer_delete(temp_publish_timer));
}