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
uint32_t HBFreqInSecs = 5;
uint32_t lastSentTime = 0;

/**
 * @brief Thread that takes care of sending HB messages periodically
 * to cloud to let it know if the device is alive or dead
 * @param args
 * @return void*
 */
void *HeartBeat_task(void *args)
{
    uint32_t timediff = 0;
    char pubmessage[PUBMESG_LEN];
    while(1)
    {
        //Converting time diff from Usec to sec
        timediff = (esp_timer_get_time() - lastSentTime)/1000000;
        if(timediff > HBFreqInSecs)
        {
            lastSentTime = esp_timer_get_time();
            sprintf(pubmessage, "%s : %d, %s : %d",
            JSON_PACKET_ID, GWY_HB_PACKET,
            GWYSERNO_STR, GWY_SER_NO);
            add_to_pubmesg_queue(pubmessage, publish_topic);
        }
    }
}