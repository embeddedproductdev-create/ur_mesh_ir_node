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
#include "../../inc/Mesh/ble_mesh_example_init.h"

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

    ESP_LOGI(DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\r\n");
    ESP_LOGI(DEBUG_TAG, "APPLICATION STARTED : %d.%d",MAJ_VERSION, MIN_VERSION);
    ESP_LOGI(DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\r\n");

    #if(MESH_PART_ENABLED)
    mesh_main_init();
    #endif

    #if(IR_RECV_PART_ENABLED)
    pthread_t IR_Receiver_tid;
    if(pthread_create(&IR_Receiver_tid, NULL, IR_receiver_task, NULL)!=0){
        perror("Error in creating recv_task : ");
    }
    #endif

    #if(LED_PART_ENABLED)
    pthread_t LED_tid;
    if(pthread_create(&LED_tid, NULL, LED_task, NULL)!=0){
        perror("Error in creating recv_task : ");
    }
    #endif

    #if(LTE_PART_ENABLED)
    pthread_t LTE_tid;
    if(pthread_create(&LTE_tid, NULL, LTE_task, NULL)!=0){
        perror("Error in creating LTE_task : ");
    }
    #endif

    #if(TEMPERATURE_SENSOR_PART_ENABLED)
    pthread_t temperature_sensor_tid;
    if(pthread_create(&temperature_sensor_tid, NULL, temperature_read, NULL)!=0){
        perror("Error in creating temperature_read_thread : ");
    }
    #endif

    #if(BUTTON_PART_ENABLED)
    pthread_t button_tid;
    if(pthread_create(&button_tid, NULL, button_task, NULL)!=0){
        perror("Error in creating button_thread : ");
    }
    #endif

    #if(IR_RECV_PART_ENABLED)
    pthread_join(IR_Receiver_tid, NULL);
    #endif

    #if(LED_PART_ENABLED)
    pthread_join(LED_tid, NULL);
    #endif

    #if(LTE_PART_ENABLED)
    pthread_join(LTE_tid, NULL);
    #endif

    #if(TEMPERATURE_SENSOR_PART_ENABLED)
    pthread_join(temperature_sensor_tid, NULL);
    #endif

    #if(BUTTON_PART_ENABLED)
    pthread_join(button_tid, NULL);
    #endif
}
