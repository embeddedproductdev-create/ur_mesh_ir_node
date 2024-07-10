/**
 * @file main.cpp
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file is the starting point for the whole program
 * @version 0.8
 * @date 2024-06-29
 * @copyright Copyright (c) 2024
 */

#include "../../inc/IR/main_IR.h"
#include "../../inc/Custom/main.h"
#include "../../inc/Custom/accesspoint.h"
#include "../../inc/Custom/button.h"

/* INITIALIZATION */
bool show_boot_indication = true;
bool esp_restart_flag = false;
bool provisioned = false;

#if (CLIENT_RELEASE)
uint32_t GWY_SER_NO=100;
uint32_t NODE_SER_NO=100;
#endif
#if (!CLIENT_RELEASE)
uint32_t GWY_SER_NO=1;
uint32_t NODE_SER_NO=2;
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

pub_conf_t node_heartbeat_pub_conf_t;
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
/*Serial No*/
#if (IS_GWY)
    // GWY_SER_NO = (GWY_SER_NO | eeprom_read_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_HI)) << 8;
    // GWY_SER_NO = (GWY_SER_NO | eeprom_read_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_MID)) << 8;
    // GWY_SER_NO = (GWY_SER_NO | eeprom_read_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_LO));
#endif

#if (!IS_GWY)
    // NODE_SER_NO = (NODE_SER_NO | eeprom_read_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_HI)) << 8;
    // NODE_SER_NO = (NODE_SER_NO | eeprom_read_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_MID)) << 8;
    // NODE_SER_NO = (NODE_SER_NO | eeprom_read_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_LO));
#endif

/*Registered/Provisioned*/
#if (IS_GWY)
    registered = eeprom_read_byte(EEPROM_SLAVE_ADDR, REGISTERED_FLAG_FLASH_ADDR);
#endif
#if (!IS_GWY)
    //Check for provisioned is being taken care on the Node mesh side.
#endif

/*Configured*/
    configured = eeprom_read_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR);

/*ProtocolSelectedNum*/
    protocol_selected_num = eeprom_read_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR_HI);
    protocol_selected_num <<= 8;
    protocol_selected_num |= eeprom_read_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR_LO);

    if(protocol_selected_num==RAW) {
        teaching_mode_rawlen = ((teaching_mode_rawlen | eeprom_read_byte(EEPROM_SLAVE_ADDR, RAWLEN_ADDR_HI)) << 8) | eeprom_read_byte(EEPROM_SLAVE_ADDR, RAWLEN_ADDR_LO);
    }

/*Heartbeat Publish Period*/
#if (IS_GWY)
    gwy_pub_conf_t.pub_conf_period_in_sec = eeprom_read_byte(EEPROM_SLAVE_ADDR, HB_PUB_CONF_PERIOD_ADDR);
#endif

#if (!IS_GWY)
    node_heartbeat_pub_conf_t.pub_conf_period_in_sec = eeprom_read_byte(EEPROM_SLAVE_ADDR, HB_PUB_CONF_PERIOD_ADDR);
#endif

/*AC Settings*/
#if (IS_GWY)
    gwy_ac_control_t.control.power = eeprom_read_byte(EEPROM_SLAVE_ADDR, POWER_FLASH_ADDR);
    gwy_ac_control_t.control.mode_val = eeprom_read_byte(EEPROM_SLAVE_ADDR, MODE_FLASH_ADDR);
    strcpy(gwy_ac_control_t.control.mode_str, get_mode_string(gwy_ac_control_t.control.mode_val));
    gwy_ac_control_t.control.fanSpeed = eeprom_read_byte(EEPROM_SLAVE_ADDR, FAN_FLASH_ADDR);
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
    node_ac_control_t.control.fanSpeed = eeprom_read_byte(EEPROM_SLAVE_ADDR, FAN_FLASH_ADDR);
    node_ac_control_t.control.temp = eeprom_read_byte(EEPROM_SLAVE_ADDR, TEMPERATURE_FLASH_ADDR);
    node_ac_control_t.control.swingH = eeprom_read_byte(EEPROM_SLAVE_ADDR, SWINGH_FLASH_ADDR);
    node_ac_control_t.control.swingV = eeprom_read_byte(EEPROM_SLAVE_ADDR, SWINGV_FLASH_ADDR);
    node_ac_control_t.control.Locking = eeprom_read_byte(EEPROM_SLAVE_ADDR, LOCKING_FLASH_ADDR);
    node_ac_control_t.control.TempLockLowLimit = eeprom_read_byte(EEPROM_SLAVE_ADDR, TEMPLOCKLOWLIMIT_FLASH_ADDR);
    node_ac_control_t.control.TempLockUpLimit = eeprom_read_byte(EEPROM_SLAVE_ADDR, TEMPLOCKUPLIMIT_FLASH_ADDR);

    //Doing this unnecessary thing because, gwy_ac_control_t is being used at sending IR command
    //Later we can fix this.
    gwy_ac_control_t.control.power = node_ac_control_t.control.power;
    strcpy(gwy_ac_control_t.control.mode_str,node_ac_control_t.control.mode_str);
    gwy_ac_control_t.control.fanSpeed = node_ac_control_t.control.fanSpeed;
    gwy_ac_control_t.control.temp = node_ac_control_t.control.temp;
    gwy_ac_control_t.control.swingH = node_ac_control_t.control.swingH;
    gwy_ac_control_t.control.swingV = node_ac_control_t.control.swingV;
    gwy_ac_control_t.control.OnTimer = node_ac_control_t.control.OnTimer;
    gwy_ac_control_t.control.OffTimer = node_ac_control_t.control.OffTimer;
#endif
}

/**
 * @brief Function that printsout the size of currently used MQTT structures
 * @param none
 * @retval none
 */
void printout_struct_sizes()
{
    sprintf(log_buffer, "Size of base_data_t : %d", sizeof(base_data_t));
    custom_printf(MAIN_DEBUG_TAG, log_buffer, WHITE);
    sprintf(log_buffer, "Size of ac_control_params_t : %d", sizeof(ac_control_params_t));
    custom_printf(MAIN_DEBUG_TAG, log_buffer, WHITE);
    sprintf(log_buffer, "Size of gwy_reg_struct : %d", sizeof(gwy_reg_struct));
    custom_printf(MAIN_DEBUG_TAG, log_buffer, WHITE);
    sprintf(log_buffer, "Size of reconf_struct : %d", sizeof(reconf_struct));
    custom_printf(MAIN_DEBUG_TAG, log_buffer, WHITE);
    sprintf(log_buffer, "Size of control_struct : %d", sizeof(control_struct));
    custom_printf(MAIN_DEBUG_TAG, log_buffer, WHITE);
    sprintf(log_buffer, "Size of teaching_mode_struct : %d", sizeof(teaching_mode_struct));
    custom_printf(MAIN_DEBUG_TAG, log_buffer, WHITE);
    sprintf(log_buffer, "Size of prov_struct : %d", sizeof(prov_struct));
    custom_printf(MAIN_DEBUG_TAG, log_buffer, WHITE);
    sprintf(log_buffer, "Size of unprov_struct : %d", sizeof(unprov_struct));
    custom_printf(MAIN_DEBUG_TAG, log_buffer, WHITE);
    sprintf(log_buffer, "Size of pub_conf_struct : %d", sizeof(pub_conf_struct));
    custom_printf(MAIN_DEBUG_TAG, log_buffer, WHITE);
    sprintf(log_buffer, "Size of heartbeat_struct_t : %d", sizeof(heartbeat_struct_t));
    custom_printf(MAIN_DEBUG_TAG, log_buffer, WHITE);
    sprintf(log_buffer, "Size of manual_ac_control_ack_t : %d", sizeof(manual_ac_control_ack_t));
    custom_printf(MAIN_DEBUG_TAG, log_buffer, WHITE);
    sprintf(log_buffer, "Size of debug_info_struct : %d", sizeof(debug_info_struct));
    custom_printf(MAIN_DEBUG_TAG, log_buffer, WHITE);
} 

/**
 * @brief Starting point for the whole program
 * @param none
 * @retval none
 */
void app_main()
{

    // First step we need to do is to fetch registered, configured, provisioned, protocol_Sel_num details from flash
    initialize_i2c();
  
    /**
     * @brief Very first step for us to check if the device is a factory new device.
     * Factory new devices will have their serial number as zero. If that's the case, then we
     * need to get input from user for serial number through UART and store it to EEPROM flash.
     * Upon the next reboot, we can fetch it from flash and fill it in RAM.
     */
    if(eeprom_read_byte(EEPROM_SLAVE_ADDR, FACTORY_DEVICE_CHECK_FLASH_ADDR))
    {
        factory_reset_device();
    }
    else fetch_from_flash();

    TaskHandle_t LED_task_handle;
    TaskHandle_t IR_task_handle;
    TaskHandle_t LTE_task_handle;
    TaskHandle_t queue_task_handle;
    TaskHandle_t button_task_handle;

#if (!IS_GWY)
    // fill element address into structures
    if (provisioned)
        fill_element_addr_to_all_structures();
#endif

    //Let's printout the size of structures that we're using, as we need to know this for BLE MESH's sake
    printout_struct_sizes();

    // Needed by freeRTOS
    BaseType_t xReturned;

#if (IS_GWY)
    fill_gwy_ser_no_str();
    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(MAIN_DEBUG_TAG, "%s APPLICATION STARTED : %d.%d.%d", GWY_SER_NO_IN_STRING, MAJ_VERSION, MIN_VERSION, INTERNAL_MIN_VERSION);
    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(MAIN_DEBUG_TAG, "General:");
    ESP_LOGI(MAIN_DEBUG_TAG, "\tRegistered          : %d", registered);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tConfigured          : %d", configured);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tProtocol            : %s", get_protocol_string(protocol_selected_num));
    ESP_LOGI(MAIN_DEBUG_TAG, "\tPublishPeriodSec    : %d", gwy_pub_conf_t.pub_conf_period_in_sec);
    ESP_LOGI(MAIN_DEBUG_TAG, "AC Settings:");
    ESP_LOGI(MAIN_DEBUG_TAG, "\tPower               : %d", gwy_ac_control_t.control.power);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tMode                : %s", gwy_ac_control_t.control.mode_str);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tFanSpeed            : %d", gwy_ac_control_t.control.fanSpeed);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tTemperature         : %d", gwy_ac_control_t.control.temp);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tSwingH              : %d", gwy_ac_control_t.control.swingH);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tSwingV              : %d", gwy_ac_control_t.control.swingV);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tLocking             : %d", gwy_ac_control_t.control.Locking);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tTempLockUpLimit     : %d", gwy_ac_control_t.control.TempLockUpLimit);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tTempLockLowLimit    : %d", gwy_ac_control_t.control.TempLockLowLimit);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tRawLen              : %d", teaching_mode_rawlen);

#endif

#if (MESH_PART_ENABLED)
#if (IS_GWY)
    gwy_mesh_main_init();

#endif
#if (!IS_GWY)
    node_mesh_main_init();
#endif
#endif

#if (!IS_GWY)
    fill_node_ser_no_str();
    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(MAIN_DEBUG_TAG, "%s APPLICATION STARTED : %d.%d.%d", NODE_SER_NO_IN_STRING, MAJ_VERSION, MIN_VERSION, INTERNAL_MIN_VERSION);
    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
    ESP_LOGI(MAIN_DEBUG_TAG, "General:");
    ESP_LOGI(MAIN_DEBUG_TAG, "\tProvisioned         : %d", provisioned);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tConfigured          : %d", configured);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tProtocol            : %s", get_protocol_string(protocol_selected_num));
    ESP_LOGI(MAIN_DEBUG_TAG, "\tElementAddr         : %d", node_ac_control_t.base_data.elementAddr);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tPublishPeriodSec    : %d", node_heartbeat_pub_conf_t.pub_conf_period_in_sec);
    ESP_LOGI(MAIN_DEBUG_TAG, "AC Settings:");
    ESP_LOGI(MAIN_DEBUG_TAG, "\tPower               : %d", node_ac_control_t.control.power);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tMode                : %s", node_ac_control_t.control.mode_str);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tFanSpeed            : %d", node_ac_control_t.control.fanSpeed);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tTemperature         : %d", node_ac_control_t.control.temp);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tSwingH              : %d", node_ac_control_t.control.swingH);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tSwingV              : %d", node_ac_control_t.control.swingV);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tLocking             : %d", node_ac_control_t.control.Locking);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tTempLockUpLimit     : %d", node_ac_control_t.control.TempLockUpLimit);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tTempLockLowLimit    : %d", node_ac_control_t.control.TempLockLowLimit);
    ESP_LOGI(MAIN_DEBUG_TAG, "\tRawLen              : %d", teaching_mode_rawlen);
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
    else ESP_LOGI(MAIN_DEBUG_TAG, "LED task creation successful");
#endif

#if (IR_RECV_PART_ENABLED)

    xReturned = xTaskCreatePinnedToCore(IR_receiver_task, "IR recv task",
                                        8192, (void *)1, 2, &IR_task_handle, CORE1);
    if (xReturned != pdPASS)
    {
        perror("Error in taskCreate for IR recv task : ");
        exit(FAILURE);
    }
    else ESP_LOGI(MAIN_DEBUG_TAG, "IR task creation successful");
#endif

#if (LTE_PART_ENABLED)

    xReturned = xTaskCreatePinnedToCore(LTE_task, "LTE Task",
                                        8192, (void *)1, 1, &LTE_task_handle, CORE0);
    if (xReturned != pdPASS)
    {
        perror("Error in taskCreate for LTE task : ");
        exit(FAILURE);
    }
    else ESP_LOGI(MAIN_DEBUG_TAG, "LTE task creation successful");
#endif

#if (QUEUE_PART_ENABLED)
    xReturned = xTaskCreatePinnedToCore(queue_handler, "Queue Task",
                                        8192, (void *)1, tskIDLE_PRIORITY, &queue_task_handle, CORE0);
    if (xReturned != pdPASS)
    {
        perror("Error in taskCreate for Queue task : ");
        exit(FAILURE);
    }
    else ESP_LOGI(MAIN_DEBUG_TAG, "Queue task creation successful");
#endif

#if (BUTTON_PART_ENABLED)
    xReturned = xTaskCreatePinnedToCore(button_task, "button task",
                                        4096, (void *)1, tskIDLE_PRIORITY, &button_task_handle, CORE0);
    if (xReturned != pdPASS)
    {
        perror("Error in taskCreate for button task : ");
        exit(FAILURE);
    }
    else ESP_LOGI(MAIN_DEBUG_TAG, "Button task creation successful");
#endif

ESP_LOGI(MAIN_DEBUG_TAG, "Successfully Created all tasks");
}
