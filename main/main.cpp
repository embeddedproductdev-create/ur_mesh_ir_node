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
#include "../../inc/Custom/button.h"

//Initialization
uint16_t GWY_SER_NO = 1;

//Initializing Global Structures
control_t gwy_ac_control_t;
control_t gwy_locking_t;
control_t node_ac_control_t;
control_t node_locking_t;
gwy_reg_t gwy_registration_t;
gwy_unreg_t gwy_unregistration_t;
prov_t provision_t;
unprov_t unprovision_t;
reconf_t gwy_reconf_t;
reconf_t node_reconf_t;
mqtt_reset_t gwy_reset_mqtt_t;
pub_conf_t gwy_pub_conf_t;
pub_conf_t node_pub_conf_t;

/**
 * @brief Function that initializes the members of global strucutres with
 * values that will probably never changes
 * For examples, GWY SER NO is never going to change
 * For example, the JSON PACKET ID is never going to change
 * So we need to assign these members, values during the runtime
 * @param none
 * @retval none
 */
void init_structures()
{
    /* GWY_SER_NO */
    gwy_registration_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_unregistration_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_conf_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_reconf_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_ac_control_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_locking_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_reset_mqtt_t.base_data.gwy_ser_no = GWY_SER_NO;
    gwy_pub_conf_t.base_data.gwy_ser_no = GWY_SER_NO;

    /* JSON PACKET IDs */
    gwy_registration_t.base_data.json_packet_id = GWY_REG_PACKET;
    gwy_unregistration_t.base_data.json_packet_id = GWY_UNREG_PACKET;
    gwy_conf_t.base_data.json_packet_id = GWY_CONF_PACKET;
    gwy_reconf_t.base_data.json_packet_id = GWY_RECONF_PACKET;
    gwy_ac_control_t.base_data.json_packet_id = GWY_AC_CONTROL_PACKET;
    gwy_locking_t.base_data.json_packet_id = GWY_AC_LOCKING_PACKET;
    gwy_reset_mqtt_t.base_data.json_packet_id = RESET_MQTT;
    gwy_pub_conf_t.base_data.json_packet_id = GWY_PUB_CONF_PACKET;
}

/**
 * @brief Starting point for the whole program
 * @param none
 * @retval none
 */
void app_main()
{
    ESP_LOGI(DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(DEBUG_TAG, "APPLICATION STARTED : %d.%d",MAJ_VERSION, MIN_VERSION);
    ESP_LOGI(DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");

    #if(LED_PART_ENABLED)
    pthread_t LED_tid;
    if(pthread_create(&LED_tid, NULL, LED_task, NULL)!=0){
        perror("Error in creating recv_task : ");
    }
    #endif

    init_structures();

    #if(AP_PART_ENABLED)
    create_AP_task();
    #endif

    while(!mqtt_params_fetched_flag)
    {
        ;//Do nothing until we fetch the mqtt params through the AP Mode (for the very first setup alone)
        vTaskDelay(1);
    }

    #if(MESH_PART_ENABLED)
    mesh_main_init();
    #endif

    #if(HEARTBEAT_PART_ENABLED)
    pthread_t HB_tid;
    if(pthread_create(&HB_tid, NULL, HeartBeat_task, NULL)!=0){
        perror("Error in creating HeartBeat_task : ");
    }
    #endif

    #if(IR_RECV_PART_ENABLED)
        ESP_LOGI(DEBUG_TAG, "Creating IR recv task\n");
        BaseType_t xReturned;
        TaskHandle_t xHandle = NULL;
        xReturned = xTaskCreate(IR_receiver_task, "IR recv task",
                                4096, (void *)1, tskIDLE_PRIORITY, &xHandle);
        if (xReturned != pdPASS)
        perror("Error in taskCreate for IR recv task : ");
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

    #if (PUBLISHING_ENABLED)
    pthread_t mqtt_pub_tid;
    if(pthread_create(&mqtt_pub_tid, NULL, publish_task, NULL) != 0){
        perror("Error in creating mqtt_publish_thread : ");
    }
    #endif

    #if(HEARTBEAT_PART_ENABLED)
    pthread_join(HB_tid, NULL);
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

/**
 * @brief Function to create the AP task
 * @param none
 * @retval none
 */
void create_AP_task()
{
  ESP_LOGI(DEBUG_TAG, "Creating AP task\n");
  BaseType_t xReturned;
  TaskHandle_t xHandle = NULL;
  xReturned = xTaskCreate(AP_task, "AccessPoint Task",
                          4096, (void *)1, tskIDLE_PRIORITY, &xHandle);
  if (xReturned != pdPASS)
    perror("Error in taskCreate for AP mode : ");
}
