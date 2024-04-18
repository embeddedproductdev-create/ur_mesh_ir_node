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
bool esp_restart_flag = false;
uint16_t GWY_SER_NO = 3;
char GWY_SER_NO_IN_STRING[8];

//Initializing Global Structures
gwy_reg_t gwy_registration_t;
gwy_unreg_t gwy_unregistration_t;
reconf_t gwy_conf_t;
reconf_t gwy_reconf_t;
control_t gwy_ac_control_t;
control_t gwy_locking_t;
mqtt_reset_t gwy_reset_mqtt_t;
pub_conf_t gwy_pub_conf_t;
temperature_data_t gwy_temperature_data_t;
teaching_mode_t gwy_teaching_mode_t;

prov_t provision_t;
prov_t *prov_queue_head;
prov_t *prov_queue_tail;

unprov_t unprovision_t;
unprov_t *unprov_queue_head;
unprov_t *unprov_queue_tail;

reconf_t node_conf_t;

reconf_t node_reconf_t;
reconf_t *node_reconf_queue_head;
reconf_t *node_reconf_queue_tail;

control_t node_ac_control_t;
control_t *node_ac_control_queue_head;
control_t *node_ac_control_queue_tail;

control_t node_locking_t;

pub_conf_t node_pub_conf_t;
pub_conf_t *node_pub_conf_queue_head;
pub_conf_t *node_pub_conf_queue_tail;

temperature_data_t node_temperature_data_t;

void fill_gwy_ser_no_str()
{
    strcpy(GWY_SER_NO_IN_STRING, "GWY");
    char serialNo[20];
    char zerostr[20];
    strcpy(serialNo,"");
    strcpy(zerostr,"");
    sprintf(serialNo, "%d", GWY_SER_NO);
    uint8_t len = strlen(GWY_SER_NO_IN_STRING) + strlen(serialNo);
    for(uint8_t i=0; i<(8-len); i++)
    {
        strcat(zerostr, "0");
    }
    strcat(GWY_SER_NO_IN_STRING, zerostr);
    strcat(GWY_SER_NO_IN_STRING, serialNo);
}

/**
 * @brief Function to create the AP task
 * @param none
 * @retval none
 */
void create_AP_task()
{
ESP_LOGI(MAIN_DEBUG_TAG, "Creating AP task\n");
BaseType_t xReturned;
TaskHandle_t xHandle = NULL;
xReturned = xTaskCreate(AP_task, "AccessPoint Task",
                        4096, (void *)1, tskIDLE_PRIORITY, &xHandle);
if (xReturned != pdPASS)
    perror("Error in taskCreate for AP mode : ");
}

/**
 * @brief Starting point for the whole program
 * @param none
 * @retval none
 */
void app_main()
{
    fill_gwy_ser_no_str();

    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(MAIN_DEBUG_TAG, "%s APPLICATION STARTED : %d.%d", GWY_SER_NO_IN_STRING, MAJ_VERSION, MIN_VERSION);
    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    sleep(2);
    init_structures();

    #if(LED_PART_ENABLED)
    pthread_t LED_tid;
    if(pthread_create(&LED_tid, NULL, LED_task, NULL)!=0){
        perror("Error in creating recv_task : ");
        exit(FAILURE);
    }
    #endif

    // Queue part
    pthread_t queue_tid;
    if(pthread_create(&queue_tid, NULL, queue_handler, NULL)!=0){
        perror("Error in creating queue_handler_task : ");
        exit(FAILURE);
    }

    #if(AP_PART_ENABLED)
    create_AP_task();
    #endif

    while(!mqtt_params_fetched_flag)
    {
        ;//Do nothing until we fetch the mqtt params through the AP Mode (for the very first setup alone)
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    #if(MESH_PART_ENABLED)
    mesh_main_init();
    #endif

    #if(IR_RECV_PART_ENABLED)
        BaseType_t xReturned;
        TaskHandle_t xHandle = NULL;
        xReturned = xTaskCreate(IR_receiver_task, "IR recv task",
                                4096, (void *)1, tskIDLE_PRIORITY, &xHandle);
        if (xReturned != pdPASS)
        {
            perror("Error in taskCreate for IR recv task : ");
            exit(FAILURE);
        }
    #endif

    #if(LTE_PART_ENABLED)
    pthread_t LTE_tid;
    if(pthread_create(&LTE_tid, NULL, LTE_task, NULL)!=0){
        perror("Error in creating LTE_task : ");
        exit(FAILURE);
    }
    #endif

    #if(TEMPERATURE_SENSOR_PART_ENABLED)
    init_temperature_sensor();
    #endif

    #if(BUTTON_PART_ENABLED)
    pthread_t button_tid;
    if(pthread_create(&button_tid, NULL, button_task, NULL)!=0){
        perror("Error in creating button_thread : ");
        exit(FAILURE);
    }
    #endif

    #if(LED_PART_ENABLED)
    pthread_join(LED_tid, NULL);
    #endif

    #if(LTE_PART_ENABLED)
    pthread_join(LTE_tid, NULL);
    #endif

    #if(BUTTON_PART_ENABLED)
    pthread_join(button_tid, NULL);
    #endif

    pthread_join(queue_tid, NULL);
}
