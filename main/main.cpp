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
#include "../../inc/Custom/accesspoint.h"
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

    printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
    printf("APPLICATION STARTED : %d.%d\n",MAJ_VERSION, MIN_VERSION);
    printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");

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

    #if(MESH_PART_ENABLED)
    pthread_t send_data_tid;
    if(pthread_create(&send_data_tid, NULL, send_data_task, NULL)!=0){
        perror("Error in creating button_thread : ");
    }
    #endif

    #if(AP_PART_ENABLED)
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;
    xReturned = xTaskCreate(AP_task, "AccessPoint Task",
    4096, (void *)1, tskIDLE_PRIORITY, &xHandle);
    if(xReturned != pdPASS)
        perror("Error in taskCreate for AP mode : ");
    #endif

    #if (PUBLISHING_ENABLED)
    pthread_t mqtt_pub_tid;
    if(pthread_create(&mqtt_pub_tid, NULL, publish_task, NULL) != 0){
        perror("Error in creating mqtt_publish_thread : ");
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

    #if(MESH_PART_ENABLED)
    pthread_join(send_data_tid, NULL);
    #endif

    #if(PUBLISHING_ENABLED)
    pthread_join(mqtt_pub_tid, NULL);
    #endif
}
