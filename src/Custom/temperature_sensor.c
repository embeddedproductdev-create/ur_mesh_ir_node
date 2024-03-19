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
uint32_t lastSentTime = 0;
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

/**
 * @brief Thread that performs the I2C temperature read communications
 * @param args
 * @return void*
 */
void *temperature_read(void *args)
{
    uint32_t timediff = 0;
    char pubmessage[PUBMESG_LEN];
    while(1)
    {
        //Converting time diff from Usec to Mins
        timediff = (esp_timer_get_time()-lastSentTime)/60*1000000; 
        if(timediff > gwy_pub_conf_t.pub_conf_period_in_mins)
        {
            lastSentTime = esp_timer_get_time();
            get_temperature(&measured_temperature);
            sprintf(pubmessage, "%s : %d, %s : %d, %s : %d",
            JSON_PACKET_ID, GWY_TEMPERATURE_DATA_PACKET,
            GWYSERNO_STR, GWY_SER_NO,
            TEMPERATURE_DATA_STR, measured_temperature);
            add_to_pubmesg_queue(pubmessage, publish_topic);
        }
    }
}