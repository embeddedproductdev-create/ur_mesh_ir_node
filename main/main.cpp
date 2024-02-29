/**
 * @file main.cpp
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This is the starting point for the whole program
 * @version 0.1
 * @date 2024-02-29
 * @copyright Copyright (c) 2024
 */

#include "../../inc/IR/main_IR.h"
#include "../../inc/Custom/main.h"

/**
 * @brief Starting point for the whole program
 * @param none
 * @retval none
 */
void app_main()
{
    Serial.begin(BAUD_RATE);
    while(!Serial)
        delay(50);

    /* Thread IDs initialization */
    pthread_t IR_Receiver_tid;
    pthread_t LTE_tid;
    pthread_t LED_tid;
    pthread_t temperature_sensor_tid;
    pthread_t button_tid;

    if(pthread_create(&IR_Receiver_tid, NULL, IR_receiver_task, NULL)!=0){
        perror("Error in creating recv_task : ");
    }
    if(pthread_create(&LED_tid, NULL, LED_task, NULL)!=0){
        perror("Error in creating recv_task : ");
    }
    if(pthread_create(&LTE_tid, NULL, LTE_task, NULL)!=0){
        perror("Error in creating LTE_task : ");
    }
    if(pthread_create(&temperature_sensor_tid, NULL, temperature_read, NULL)!=0){
        perror("Error in creating temperature_read_thread : ");
    }
    if(pthread_create(&button_tid, NULL, button_task, NULL)!=0){
        perror("Error in creating button_thread : ");
    }

    pthread_join(IR_Receiver_tid, NULL);
    pthread_join(LED_tid, NULL);
    pthread_join(LTE_tid, NULL);
    pthread_join(temperature_sensor_tid, NULL);
    pthread_join(button_tid, NULL);
}