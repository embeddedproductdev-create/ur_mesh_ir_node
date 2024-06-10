/**
 * @file main.cpp
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This is the starting point for the whole program
 * @version 0.6
 * @date 2024-02-29
 * @copyright Copyright (c) 2024
 */

#include "../../inc/IR/main_IR.h"
#include "../../inc/Custom/main.h"
#include "../../inc/Custom/accesspoint.h"
#include "../../inc/Custom/button.h"

// Initialization
bool esp_restart_flag = false;
#if (CLIENT_RELEASE)
uint16_t GWY_SER_NO = 100;
uint16_t NODE_SER_NO = 100;
#endif
#if (!CLIENT_RELEASE)
uint16_t GWY_SER_NO = 15;
uint16_t NODE_SER_NO = 2;
#endif
char GWY_SER_NO_IN_STRING[15];
char NODE_SER_NO_IN_STRING[15];

// Initializing Global Structures
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
    strcpy(serialNo, "");
    strcpy(zerostr, "");
    sprintf(serialNo, "%d", GWY_SER_NO);
    uint8_t len = strlen(GWY_SER_NO_IN_STRING) + strlen(serialNo);
    for (uint8_t i = 0; i < (8 - len); i++)
    {
        strcat(zerostr, "0");
    }
    strcat(GWY_SER_NO_IN_STRING, zerostr);
    strcat(GWY_SER_NO_IN_STRING, serialNo);
}

void fill_node_ser_no_str()
{
    strcpy(NODE_SER_NO_IN_STRING, "N");
    char serialNo[20];
    char zerostr[20];
    strcpy(serialNo, "");
    strcpy(zerostr, "");
    sprintf(serialNo, "%d", NODE_SER_NO);
    uint8_t len = strlen(NODE_SER_NO_IN_STRING) + strlen(serialNo);
    for (uint8_t i = 0; i < (6 - len); i++)
    {
        strcat(zerostr, "0");
    }
    strcat(NODE_SER_NO_IN_STRING, zerostr);
    strcat(NODE_SER_NO_IN_STRING, serialNo);
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
    xReturned = xTaskCreatePinnedToCore(AP_task, "AccessPoint Task",
                            4096, (void *)1, tskIDLE_PRIORITY, &xHandle, CORE0);
    if (xReturned != pdPASS)
        perror("Error in taskCreate for AP mode : ");
}

/**
 * @brief Function that takes care of fetching data from flash reg. registration and configuration status
 * @param none
 * @retval none
 */
void fetch_from_flash()
{
    uint8_t temp = 0;
    #if (IS_GWY)
        registered = eeprom_read_byte(EEPROM_SLAVE_ADDR, REGISTERED_FLAG_FLASH_ADDR);
        if(registered) registered = false;
        else registered = true;
    #endif
    #if (!IS_GWY)
        provisioned = eeprom_read_byte(EEPROM_SLAVE_ADDR, PROVISIONED_FLAG_FLASH_ADDR);
    #endif
    configured = eeprom_read_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR);
    if(configured) configured = false;
    else configured = true;
    protocol_selected_num = eeprom_read_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR);
    protocol_selected_num <<= 8;
    protocol_selected_num |= eeprom_read_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR+1);
    #if (IS_GWY)
    ESP_LOGI(MAIN_DEBUG_TAG, "REG : %d | CONF : %d | PROTOCOL : %d", registered, configured, protocol_selected_num);
    #endif
    #if (!IS_GWY)
    ESP_LOGI(MAIN_DEBUG_TAG, "PROV : %d | CONF : %d | PROTOCOL : %d",provisioned, configured, protocol_selected_num);
    #endif
}

/**
 * @brief Starting point for the whole program
 * @param none
 * @retval none
 */
void app_main()
{
    initialize_i2c();
    // eeprom_write_byte(EEPROM_SLAVE_ADDR, REGISTERED_FLAG_FLASH_ADDR, 0XFF);
    // vTaskDelay(pdMS_TO_TICKS(5));
    // eeprom_write_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR, 0XFF);
    // vTaskDelay(pdMS_TO_TICKS(5));
    fetch_from_flash();

    // these two are needed incase if we're creating tasks using RTOS
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;

#if (IS_GWY)
    fill_gwy_ser_no_str();
    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(MAIN_DEBUG_TAG, "%s APPLICATION STARTED : %d.%d", GWY_SER_NO_IN_STRING, MAJ_VERSION, MIN_VERSION);
    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    init_structures();
#endif

#if (!IS_GWY)
    fill_node_ser_no_str();
    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(MAIN_DEBUG_TAG, "%s APPLICATION STARTED : %d.%d", NODE_SER_NO_IN_STRING, MAJ_VERSION, MIN_VERSION);
    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    init_structures();
#endif

#if (LED_PART_ENABLED)
    xReturned = xTaskCreatePinnedToCore(LED_task, "LED task",
                            4096, (void *)1, tskIDLE_PRIORITY, &xHandle, CORE0);
    if (xReturned != pdPASS)
    {
        perror("Error in taskCreate for LED task : ");
        exit(FAILURE);
    }
#endif

#if (AP_PART_ENABLED)
    create_AP_task();
#endif

    while (!mqtt_params_fetched_flag)
    {
        /**
         * It's better if we don't enable other threads until
         * we get the paramters for MQTT from AP mode. Cause,
         * without it, it's meaningless to run other thread.
         * NOTE: If AP mode disabled and MQTT params hardcoded,
         * then the mqtt_params_fetch_flag is be default set to
         * true.
         */
        ;
        vTaskDelay(pdMS_TO_TICKS(50));
    }

#if (MESH_PART_ENABLED)
#if (IS_GWY)
    gwy_mesh_main_init();
#endif
#if (!IS_GWY)
    node_mesh_main_init();
#endif
#endif

#if (TEMPERATURE_SENSOR_PART_ENABLED)
    init_temperature_sensor();
#endif

#if (IR_RECV_PART_ENABLED)
    TaskHandle_t IR_task_handle;
    xReturned = xTaskCreatePinnedToCore(IR_receiver_task, "IR recv task",
                            4096, (void *)1, 10, &IR_task_handle, CORE1);
    if (xReturned != pdPASS) 
    {
        perror("Error in taskCreate for IR recv task : ");
        exit(FAILURE);
    }
#endif

#if (LTE_PART_ENABLED)
    xReturned = xTaskCreatePinnedToCore(LTE_task, "LTE Task",
                            4096, (void *)1, tskIDLE_PRIORITY, &xHandle, CORE0);
    if (xReturned != pdPASS)
    {
        perror("Error in taskCreate for LTE task : ");
        exit(FAILURE);
    }
#endif

#if (QUEUE_PART_ENABLED)
    xReturned = xTaskCreatePinnedToCore(queue_handler, "Queue Task",
                            8192, (void *)1, tskIDLE_PRIORITY, &xHandle, CORE0);
    if (xReturned != pdPASS)
    {
        perror("Error in taskCreate for Queue task : ");
        exit(FAILURE);
    }
#endif

#if (BUTTON_PART_ENABLED)
    xReturned = xTaskCreatePinnedToCore(button_task, "button task",
                            4096, (void *)1, tskIDLE_PRIORITY, &xHandle, CORE0);
    if (xReturned != pdPASS)
    {
        perror("Error in taskCreate for button task : ");
        exit(FAILURE);
    }
#endif
}
