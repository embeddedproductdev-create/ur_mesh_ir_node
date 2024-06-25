/**
 * @file main.cpp
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file is the starting point for the whole program
 * @version 0.8
 * @date 2024-06-19
 * @copyright Copyright (c) 2024
 */

#include "../../inc/IR/main_IR.h"
#include "../../inc/Custom/main.h"
#include "../../inc/Custom/accesspoint.h"
#include "../../inc/Custom/button.h"

/* INITIALIZATION */
bool show_boot_indication = true;
bool esp_restart_flag = false;
#if (CLIENT_RELEASE)
uint32_t GWY_SER_NO = 100;
uint32_t NODE_SER_NO = 100;
#endif
#if (!CLIENT_RELEASE)
uint32_t GWY_SER_NO = 2;
uint32_t NODE_SER_NO = 1;
#endif
char GWY_SER_NO_IN_STRING[15];
char NODE_SER_NO_IN_STRING[15];

/*Global Structures*/
gwy_reg_t gwy_registration_t;
gwy_unreg_t gwy_unregistration_t;
reconf_t gwy_conf_t;
reconf_t gwy_reconf_t;
control_t gwy_ac_control_t;
manual_ac_control_t gwy_manual_ac_control_t;
pub_conf_t gwy_pub_conf_t;
heartbeat_t gwy_heartbeat_t;
teaching_mode_t gwy_teaching_mode_t;
debug_info_t gwy_debug_info_t;

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

manual_ac_control_t node_manual_ac_control_t;

pub_conf_t node_hearbeat_pub_conf_t;
pub_conf_t *node_pub_conf_queue_head;
pub_conf_t *node_pub_conf_queue_tail;

heartbeat_t node_heartbeat_t;

teaching_mode_t node_teaching_mode_t;
teaching_mode_t *node_teaching_mode_queue_head;
teaching_mode_t *node_teaching_mode_queue_tail;

debug_info_t node_debug_info_t;
debug_info_t *node_debug_info_queue_head;
debug_info_t *node_debug_info_queue_tail;

#if (IS_GWY)
void fill_gwy_ser_no_str()
{
    strcpy(GWY_SER_NO_IN_STRING, "GWY");
    char serialNo[20];
    char zerostr[20];
    strcpy(serialNo, "");
    strcpy(zerostr, "");
    sprintf(serialNo, "%ld", GWY_SER_NO);
    uint8_t len = strlen(GWY_SER_NO_IN_STRING) + strlen(serialNo);
    for (uint8_t i = 0; i < (8 - len); i++)
    {
        strcat(zerostr, "0");
    }
    strcat(GWY_SER_NO_IN_STRING, zerostr);
    strcat(GWY_SER_NO_IN_STRING, serialNo);
}
#endif

#if (!IS_GWY)
void fill_node_ser_no_str()
{
    strcpy(NODE_SER_NO_IN_STRING, "N");
    char serialNo[20];
    char zerostr[20];
    strcpy(serialNo, "");
    strcpy(zerostr, "");
    sprintf(serialNo, "%ld", NODE_SER_NO);
    uint8_t len = strlen(NODE_SER_NO_IN_STRING) + strlen(serialNo);
    for (uint8_t i = 0; i < (6 - len); i++)
    {
        strcat(zerostr, "0");
    }
    strcat(NODE_SER_NO_IN_STRING, zerostr);
    strcat(NODE_SER_NO_IN_STRING, serialNo);
}
#endif

/**
 * @brief Function that takes care of fetching data from flash reg. registration and configuration status
 * Last AC control packet settings
 * @param none
 * @retval none
 */
void fetch_from_flash()
{
/*Registered/Provisioned*/
#if (IS_GWY)
    registered = eeprom_read_byte(EEPROM_SLAVE_ADDR, REGISTERED_FLAG_FLASH_ADDR);
    if (registered)
        registered = false;
    else
        registered = true;
#endif
#if (!IS_GWY)
    provisioned = eeprom_read_byte(EEPROM_SLAVE_ADDR, PROVISIONED_FLAG_FLASH_ADDR);
#endif

    /*Configured*/
    configured = eeprom_read_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR);
    if (configured)
        configured = false;
    else
        configured = true;

    /*ProtocolSelectedNum*/
    protocol_selected_num = eeprom_read_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR_HI);
    protocol_selected_num <<= 8;
    protocol_selected_num |= eeprom_read_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR_LO);

    /*Heartbeat Publish Period*/
    #if (IS_GWY)
    gwy_pub_conf_t.pub_conf_period_in_sec = eeprom_read_byte(EEPROM_SLAVE_ADDR, HB_PUB_CONF_PERIOD_ADDR);
    #endif

    #if (!IS_GWY)
    node_pub_conf_t.pub_conf_period_in_sec = eeprom_read_byte(EEPROM_SLAVE_ADDR, HB_PUB_CONF_PERIOD_ADDR);
    #endif
    
    /*AC Settings*/
#if (IS_GWY)
    gwy_ac_control_t.control.power = eeprom_read_byte(EEPROM_SLAVE_ADDR, POWER_FLASH_ADDR);
    gwy_ac_control_t.control.mode_val = eeprom_read_byte(EEPROM_SLAVE_ADDR, MODE_FLASH_ADDR);
    strcpy(gwy_ac_control_t.control.mode_str, get_mode_string(gwy_ac_control_t.control.mode_val));
    gwy_ac_control_t.control.fan = eeprom_read_byte(EEPROM_SLAVE_ADDR, FAN_FLASH_ADDR);
    gwy_ac_control_t.control.temp = eeprom_read_byte(EEPROM_SLAVE_ADDR, TEMPERATURE_FLASH_ADDR);
    gwy_ac_control_t.control.swingH = eeprom_read_byte(EEPROM_SLAVE_ADDR, SWINGH_FLASH_ADDR);
    gwy_ac_control_t.control.swingV = eeprom_read_byte(EEPROM_SLAVE_ADDR, SWINGV_FLASH_ADDR);
    gwy_ac_control_t.control.Locking = eeprom_read_byte(EEPROM_SLAVE_ADDR, LOCKING_FLASH_ADDR);
    gwy_ac_control_t.control.TempLockLowLimit = eeprom_read_byte(EEPROM_SLAVE_ADDR, TEMPLOCKLOWLIMIT_FLASH_ADDR);
    gwy_ac_control_t.control.TempLockUpLimit = eeprom_read_byte(EEPROM_SLAVE_ADDR, TEMPLOCKUPLIMIT_FLASH_ADDR);
#endif

#if (!IS_GWY)
    node_ac_control_t.control.power = eeprom_read_byte(EEPROM_SLAVE_ADDR, POWER_FLASH_ADDR);
    node_ac_control_t.control.mode_val = eeprom_read_byte(EEPROM_SLAVE_ADDR, MODE_FLASH_ADDR);
    strcpy(node_ac_control_t.control.mode_str, get_mode_string(node_ac_control_t.control.mode_val));
    node_ac_control_t.control.fan = eeprom_read_byte(EEPROM_SLAVE_ADDR, FAN_FLASH_ADDR);
    node_ac_control_t.control.temp = eeprom_read_byte(EEPROM_SLAVE_ADDR, TEMPERATURE_FLASH_ADDR);
    node_ac_control_t.control.swingH = eeprom_read_byte(EEPROM_SLAVE_ADDR, SWINGH_FLASH_ADDR);
    node_ac_control_t.control.swingV = eeprom_read_byte(EEPROM_SLAVE_ADDR, SWINGV_FLASH_ADDR);
    node_ac_control_t.control.Locking = eeprom_read_byte(EEPROM_SLAVE_ADDR, LOCKING_FLASH_ADDR);
    node_ac_control_t.control.TempLockLowLimit = eeprom_read_byte(EEPROM_SLAVE_ADDR, TEMPLOCKLOWLIMIT_FLASH_ADDR);
    node_ac_control_t.control.TempLockUpLimit = eeprom_read_byte(EEPROM_SLAVE_ADDR, TEMPLOCKUPLIMIT_FLASH_ADDR);
#endif
}

/**
 * @brief Starting point for the whole program
 * @param none
 * @retval none
 */
void app_main()
{

    TaskHandle_t LED_task_handle;
    TaskHandle_t IR_task_handle;
    TaskHandle_t LTE_task_handle;
    TaskHandle_t queue_task_handle;
    TaskHandle_t button_task_handle;

    // First step we need to do is to fetch registered, configured, provisioned, protocol_Sel_num details from flash
    initialize_i2c();
    fetch_from_flash();

    // If the device is Not registered / Not provisioned, set default pub conf value to flash. This is req for factory new devices.
    if(!registered || !configured)
    {
        eeprom_write_byte(EEPROM_SLAVE_ADDR, HB_PUB_CONF_PERIOD_ADDR, DEFAULT_HEARTBEAT_PUB_CONF_PERIOD_SEC);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    
    // Needed by freeRTOS
    BaseType_t xReturned;

#if (IS_GWY)
    fill_gwy_ser_no_str();
    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(MAIN_DEBUG_TAG, "%s APPLICATION STARTED : %d.%d", GWY_SER_NO_IN_STRING, MAJ_VERSION, MIN_VERSION);
    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(MAIN_DEBUG_TAG, "General:");
    ESP_LOGI(MAIN_DEBUG_TAG, "\tRegistered          : %d", registered);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tConfigured          : %d", configured);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tProtocol            : %d", protocol_selected_num);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tPublishPeriodSec    : %d", gwy_pub_conf_t.pub_conf_period_in_sec);
    ESP_LOGI(MAIN_DEBUG_TAG, "AC Settings:");
    ESP_LOGI(MAIN_DEBUG_TAG, "\tPower               : %d", gwy_ac_control_t.control.power);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tMode                : %s", gwy_ac_control_t.control.mode_str);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tFanSpeed            : %d", gwy_ac_control_t.control.fan);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tTemperature         : %d", gwy_ac_control_t.control.temp);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tSwingH              : %d", gwy_ac_control_t.control.swingH);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tSwingV              : %d", gwy_ac_control_t.control.swingV);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tLocking             : %d", gwy_ac_control_t.control.Locking);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tTempLockUpLimit     : %d", gwy_ac_control_t.control.TempLockUpLimit);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tTempLockLowLimit    : %d", gwy_ac_control_t.control.TempLockLowLimit);
#endif

#if (!IS_GWY)
    fill_node_ser_no_str();
    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(MAIN_DEBUG_TAG, "%s APPLICATION STARTED : %d.%d", NODE_SER_NO_IN_STRING, MAJ_VERSION, MIN_VERSION);
    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(MAIN_DEBUG_TAG, "General:");
    ESP_LOGI(MAIN_DEBUG_TAG, "\tRegistered          : %d", registered);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tConfigured          : %d", configured);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tProtocol            : %d", protocol_selected_num);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tPublishPeriodSec    : %d", node_pub_conf_t.pub_conf_period_in_sec);
    ESP_LOGI(MAIN_DEBUG_TAG, "AC Settings:");
    ESP_LOGI(MAIN_DEBUG_TAG, "\tPower               : %d", node_ac_control_t.control.power);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tMode                : %s", node_ac_control_t.control.mode_str);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tFanSpeed            : %d", node_ac_control_t.control.fan);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tTemperature         : %d", node_ac_control_t.control.temp);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tSwingH              : %d", node_ac_control_t.control.swingH);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tSwingV              : %d", node_ac_control_t.control.swingV);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tLocking             : %d", node_ac_control_t.control.Locking);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tTempLockUpLimit     : %d", node_ac_control_t.control.TempLockUpLimit);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tTempLockLowLimit    : %d", node_ac_control_t.control.TempLockLowLimit);
#endif

    init_structures();

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

#if (LED_PART_ENABLED)
    xReturned = xTaskCreatePinnedToCore(LED_task, "LED task",
                                        4096, (void *)1, tskIDLE_PRIORITY, &LED_task_handle, CORE0);
    if (xReturned != pdPASS)
    {
        perror("Error in taskCreate for LED task : ");
        exit(FAILURE);
    }
#endif

#if (IR_RECV_PART_ENABLED)
    
    xReturned = xTaskCreatePinnedToCore(IR_receiver_task, "IR recv task",
                                        8192, (void *)1, 2, &IR_task_handle, CORE1);
    if (xReturned != pdPASS)
    {
        perror("Error in taskCreate for IR recv task : ");
        exit(FAILURE);
    }
#endif

#if (LTE_PART_ENABLED)
    
    xReturned = xTaskCreatePinnedToCore(LTE_task, "LTE Task",
                                        4096, (void *)1, 1, &LTE_task_handle, CORE0);
    if (xReturned != pdPASS)
    {
        perror("Error in taskCreate for LTE task : ");
        exit(FAILURE);
    }
#endif

#if (QUEUE_PART_ENABLED)
    xReturned = xTaskCreatePinnedToCore(queue_handler, "Queue Task",
                                        8192, (void *)1, tskIDLE_PRIORITY, &queue_task_handle, CORE0);
    if (xReturned != pdPASS)
    {
        perror("Error in taskCreate for Queue task : ");
        exit(FAILURE);
    }
#endif

#if (BUTTON_PART_ENABLED)
    xReturned = xTaskCreatePinnedToCore(button_task, "button task",
                                        4096, (void *)1, tskIDLE_PRIORITY, &button_task_handle, CORE0);
    if (xReturned != pdPASS)
    {
        perror("Error in taskCreate for button task : ");
        exit(FAILURE);
    }
#endif
    // /**
    //  * @brief Create the threads and stay here until we have to control AC
    //  * When we have to control AC, before controlling, all other threads will get deleted.
    //  * AC control function works out and after that. Let's recreate the threads and wait here again.
    //  * This plan is stupid what this is what we have now.
    //  */
    // while(!needToSendIRComamnd) vTaskDelay(1);
    // }
}
