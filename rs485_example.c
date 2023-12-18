/* Uart Events Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "string.h" // for strerror()
#include "errno.h"  // for errno
#include "esp_err.h"
#include "nvs.h"
#include "nvs_flash.h"

// #include "mbcontroller.h"
#include "common.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_timer.h"
#include "soc/soc.h"
#include "esp_sntp.h"
#include "driver/rtc_io.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "soc/soc_caps.h"
#include "cJSON.h"
#include "time.h"
// #include "xlsxwriter.h"

/**
 * This is a example which echos any data it receives on UART back to the sender using RS485 interface in half duplex mode.
 */
#define TAG ":"
#define DEBUG_LOG 0
// Note: Some pins on target chip cannot be assigned for UART communication.
// Please refer to documentation for selected board and target to configure pins using Kconfig.
#define ECHO_TEST_TXD 7
#define ECHO_TEST_RXD 8
#define UART_DIR_RS485 3
#define ECHO_UART_PORT_NUM 2
// RTS for RS485 Half-Duplex Mode manages DE/~RE
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)

// CTS is not used in RS485 Half-Duplex Mode
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define BUF_SIZE (127)
#define BAUD_RATE 9600

#define RS232_TXD_PIN 43
#define RS232_RXD_PIN 44
#define RS232_CTS_PIN UART_PIN_NO_CHANGE
#define RS232_RTS_PIN UART_PIN_NO_CHANGE

#define EXAMPLE_ADC1_CHAN0 ADC_CHANNEL_0
#define EXAMPLE_ADC1_CHAN1 ADC_CHANNEL_1
#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_11

// Read packet timeout
#define PACKET_READ_TICS (100 / portTICK_PERIOD_MS)
#define ECHO_TASK_STACK_SIZE (2048)
#define ECHO_TASK_PRIO (10)
#define ECHO_UART_PORT (CONFIG_ECHO_UART_PORT_NUM)

// Timeout threshold for UART = number of symbols (~10 tics) with unchanged state on receive pin
#define ECHO_READ_TOUT (3) // 3.5T * 8 = 28 ticks, TOUT=3 -> ~24..33 ticks

#define MB_DEV_SPEED (CONFIG_MB_UART_BAUD_RATE) // The communication speed of the UART
#define EM_MAX 100
#define EM_WATTS_TOTAL 0
#define LED_PIN 46
#define MAX_DATA_SIZE 128
#define BLE_LED 46
#define PWR_LED 48
#define MCU_WDI_PIN 9

#define BAT_MAX_LIMIT 11.63
int exp1_port0_dir = 0x00;

int exp1_port_dir_00 = 0;
int exp1_port_dir_01 = 0;

int exp1_port_status_00 = 0;
int exp1_port_status_01 = 0;

#define MCU_WAKEUP BIT0
#define STATUS_LTE BIT1
#define SET1_WDT BIT2
#define BUZZER_BIT BIT3
#define TEMP_ALERT_BIT BIT5
#define RELAY_CTRL_BIT BIT7

#define LA_PROT_CTL BIT0
#define MCU_RESET BIT1
#define UART_DTR BIT4
#define MCU_NET_MODE BIT5
#define MCU_ON_OFF BIT6
#define REG_CTRL_5V BIT7

#define PROMPT ">"
#define PRODUCT_INFO "ATI\r\n"
#define AT_CMD "AT\r\n"
#define GET_MANUFACTURER "AT+CGMI\r\n"
#define GET_IMEI_NUM "AT+GSN=1\r\n"
#define FACTORY_RESET "AT&F0\r\n"
#define GET_CURRENT_CONFIG "AT&V\r\n"
#define RESULT_CODE_ECHO_MODE "ATQ0\r\n"
#define ERROR_FORMAT "AT+CMEE=2\r\n"
#define ERROR_REPORT "AT+CEER\r\n"
#define INDICATOR_STATE "AT+CIND=?\r\n"
#define GET_TIME "AT+QLTS=2\r\n"

// UART configuration
#define SET_DCD "AT&C=0\r\n"
#define DTR_FUNCTION_MODE "AT&D2\r\n"
#define HW_FLOW_CTRL "AT+IFC=2,2\r\n"
#define SET_FRAMING_FORMAT "AT+ICF=3\r\n"
#define SET_BAUD_RATE "AT+IPR="

// Status Control
#define ME_ACTIVITY_STATUS "AT+CPAS=0\r\n"
#define NETWORK_SEARCH "AT+QCFG=\"nwscanmode\",3,1\r\n"
#define SET_FREQUENCY_BAND "AT+QCFG=\"band\,\r\n"
#define AIRPLANE_MODE "AT+QCFG=\"airplanecontrol\",0\r\n"

// SIM Card Status
#define SIM_INSERT_STATUS "AT+QSIMSTAT=1"

// Hardware commands
#define POWER_OFF "AT+QPOWD=0\r\n"
#define READ_CLOCK "AT+CCLK\r\n"
#define READ_BATTERY "AT+CBC\r\n"
#define SLEEP_MODE "AT+QSCLK=2\r\n"
#define SYNC_SCAN_WIFI "AT+QWIFISCAN?\r\n"

// MQTT commands
#define MQTT_VERSION "AT+QMTCFG=\"version\","
#define MQTT_PDP "AT+QMTCFG=\"pdpcid\","
#define ENABLE_SSL "AT+QMTCFG=\"ssl\","
#define MQTT_KEEP_ALIVE "AT+QMTCFG=\"keepalive\","
#define CLEAN_SESSION "AT+QMTCFG=\"session\","
#define MQTT_WILL_CONFIG "AT+QMTCFG=\"will\","
#define MQTT_RECV_MODE "AT+QMTCFG=\"recv/mode\","
#define MQTT_SEND_MODE "AT+QMTCFG=\"send/mode\",1,0\r\n"
#define MQTT_HEARTBEAT "AT+QMTCFG=\"qmtping\",1\r\n"
#define MQTT_DATA_FORMAT "AT+QMTCFG=\"dataformat\",1,0,0\r\n"

#define MQTT_NETWORK_OPEN "AT+QMTOPEN="
#define MQTT_NETWORK_CLOSE "AT+QMTCLOSE="
#define MQTT_CLIENT_CONN "AT+QMTCONN="
#define MQTT_CLIENT_DISCONN "AT+QMTDISC="

// subscribe & publish
#define SUB_TO_TOPIC "AT+QMTSUB="
#define UNSUB_TO_TOPIC "AT+QMTUNS="
#define PUB_MSG "AT+QMTPUBEX="
#define READ_MSG_BUFFER "AT+QMTRECV="

// MQTT configuration Response
#define OK_RESPONSE "OK\r\n"
#define CONNECT_RESPONSE "CONNECT\r\n"
#define INDICATOR_STATE_RESPONSE "+CIND: "
#define MQTT_VERSION_RESPONSE "+QMTCFG:\"version\",3"
#define MQTT_SSL_RESPONSE "+QMTCFG:\"ssl\",1"
#define MQTT_KEEP_ALIVE_RESPONSE "+QMTCFG:\"keepalive\",120"
#define MQTT_RECV_MODE_RESPONSE "+QMTCFG:\"recv/mode\",0,0"
#define MQTT_HEARTBEAT_RESPONSE "+QMTCFG:\"qmtping\",5"
#define MQTT_SEND_MODE_RESPONSE "+QMTCFG:\"send/mode\",0"
#define MQTT_DATA_FORMAT_RESPONSE "+QMTCFG:\"dataformat\",0,0"

// MQTT responses
#define MQTT_NETWORK_OPEN_RESPONSE "+QMTOPEN: "
#define MQTT_NETWORK_CLOSE_RESPONSE "+QMTCLOSE: "
#define MQTT_CLIENT_CONN_RESPONSE "+QMTCONN: "
#define MQTT_CLIENT_DISCONN_RESPONSE "+QMTDISC: "
#define MQTT_MSG_RECV_STATUS "+QMTRECV: "
#define STATUS_MQTT_NETWORK "+QMTSTAT:1,1"
#define MQTT_SUB_RESPONSE "+QMTSUB: "
#define MQTT_UNSUB_RESPONSE "+QMTUNS: "
#define MQTT_PUB_MSG_RESPONSE "+QMTPUBEX: "

// File commands
#define FILE_UPLOAD "AT+QFUPL="
#define FILE_LIST "AT+QFLST=\"*\"\r\n"
#define FILE_DELETE "AT+QFDEL="

// File response
#define FILE_UPLOAD_RESPONSE "+QFUPL: "

// SSL commands
#define SSL_SEC_WRITE "AT+QSECWRITE="
#define SSL_SEC_READ "AT+QSECREAD="
#define SSL_SEC_DELETE "AT+QSECDEL="
#define SSL_CTX_INDEX "AT+QSSLCFG=\"ctxindex\","
#define SSL_CONFIG_CACERT "AT+QSSLCFG=\"cacert\","
#define SSL_CONFIG_CCCERT "AT+QSSLCFG=\"clientcert\","
#define SSL_CONFIG_CLIKEY "AT+QSSLCFG=\"clientkey\","
#define SSL_CONFIG_AUTHMODE "AT+QSSLCFG=\"seclevel\","
#define SSL_CONFIG_AUTHVER "AT+QSSLCFG=\"sslversion\","
#define SSL_CONFIG_CIPHERSUITE "AT+QSSLCFG=\"ciphersuite\","
#define SSL_CONFIG_AUTHTIME "AT+QSSLCFG=\"ignorelocaltime\","

// SSL response
#define SSL_SEC_WRITE_RESPONSE "+QSECWRITE= "
#define SSL_SEC_READ_RESPONSE "+QSECREAD= 1"

#define SEND_EVETS_TOPIC "QSAFE_00001/send_events"
#define SUBS_TOPIC "QSAFE_00001/receive_events"
#define NODE_NAME "QSAFE_00001"
char *device_id = "QSAFE_00001";
#define TARGET_MAC_ADDRESS "11:22:33:44:55:66"
#define TARGET_BEACON_NAME "Amazfit Bip Wtach"

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80
#define BIT8 0x100
#define BIT9 0x200
#define BIT10 0x400
#define BIT11 0x800
#define BIT12 0x1000
#define BIT16 0xFFFF

#define GATTC_TAG ":"
#define REMOTE_SERVICE_UUID 0x00FF
#define REMOTE_NOTIFY_CHAR_UUID 0xFF01
#define PROFILE_NUM 1
#define PROFILE_A_APP_ID 0
#define INVALID_HANDLE 0
#define UART_NUM_1 1

static const char remote_device_name[] = "InConn";
static bool connect = false;
static bool get_server = false;
static esp_gattc_char_elem_t *char_elem_result = NULL;
static esp_gattc_descr_elem_t *descr_elem_result = NULL;

/* Declare static functions */
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

static esp_bt_uuid_t remote_filter_service_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {
        .uuid16 = REMOTE_SERVICE_UUID,
    },
};

static esp_bt_uuid_t remote_filter_char_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {
        .uuid16 = REMOTE_NOTIFY_CHAR_UUID,
    },
};

static esp_bt_uuid_t notify_descr_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {
        .uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG,
    },
};

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x50,
    .scan_window = 0x30,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE};

struct gattc_profile_inst
{
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_start_handle;
    uint16_t service_end_handle;
    uint16_t char_handle;
    esp_bd_addr_t remote_bda;
};

/* One gatt-based profile one app_id and one gattc_if, this array will store the gattc_if returned by ESP_GATTS_REG_EVT */
static struct gattc_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gattc_cb = gattc_profile_event_handler,
        .gattc_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

typedef struct
{
    uint8_t uuid[16];
    int8_t rssi;
} BeaconData;

// Define the MAC address or beacon name you want to filter

uint8_t network_flag = 0;
uint8_t client_flag = 0;
uint8_t subscribe_flag = 0;
enum stored_packet_counts
{
    file1 = 0,
    file2,
    total
};
int32_t to_be_send_Packet_Count[4];
uint32_t current_Position[3];
uint32_t sent_Packet_Count[4];
uint32_t stored_total_packets_count[4];
uint8_t current_File_Flag;
bool file1_first_flag = true, file2_first_flag = true;

char *heart_beat_id = "heart_beat_packet";
char *config_error_msg_id = "config_error_packet";
double temp_sensor = 0, noise = 0, humid = 0, pm2p5 = 0,
       pm10 = 0, CO_lvl = 0, CO2_lvl = 0, SO2_lvl = 0,
       NO2_lvl = 0, O3_lvl = 0, wind_speed = 0,
       wind_dir = 0, atm_pres = 0, rainfall = 0, iluminance = 0,
       solar_rad = 0, uv_idx = 0, lead_acid_bat = 0, lipo_bat = 0, x_axis = 0, y_axis = 0, z_axis = 0, voc_level = 0, pressure = 0, formaldehyde = 0;
double HD2010_sound = 0;

double humid_buf[10], pm2p5_buf[10],
    pm10_buf[10], CO_lvl_buf[10], CO2_lvl_buf[10], SO2_lvl_buf[10],
    NO2_lvl_buf[10], O3_lvl_buf[10], wind_speed_buf[10],
    wind_dir_buf[10], atm_pres_buf[10], rainfall_buf[10], iluminance_buf[10],
    solar_rad_buf[10], uv_idx_buf[10],
    voc_level_buf[10], pressure_buf[10], formaldehyde_buf[10],
    dust_pm1_buf[10], dust_pm2p5_buf[10], dust_pm10_buf[10], WBGT_data_01_buf[10],
    WBGT_data_02_buf[10], WBGT_data_03_buf[10], WBGT_data_04_buf[10], WBGT_data_05_buf[10],
    WBGT_data_06_buf[10], sound_level_3033_buf[10], sound_level_3036_buf[10];

char *sim_num = "9876543210";
float FM_version = 0.1;
char *time_stamp = NULL;
char *utcTime = NULL;
char *latitude = NULL;
char *longitude = NULL;
char *gps_day = NULL;
int numberOfSatellites = 0;
int satellites;
int wind_sen_state, dust_conc_sen_state,
    CO2_sen_state, air_quality_sr_state,
    VOC_sr_state, formaldehyde_sr_state,
    UV_rad_sr_state, solar_illum_sr_state,
    wind_dir_sr_state, solar_rad_sr_state,
    SO2_conc_sr_state, Atm_hum_pres_sr_state,
    NO2_conc_sr_state, CO_conc_sr_state,
    O3_conc_sr_state, rain_sr_state,
    HD2010UC_class_1_state, SL_4033SD_state,
    SL_4036SD_state, VM_6380_state,
    WBGT_2010SD_state = 0;
int GNSS_flag = 0;
double dust_pm1, dust_pm2p5, dust_pm10;
char sound_level_data_01[40];
char sound_level_data_02[40];
char sound_level_data_03[40];
char sound_level_data_04[40];
char WBGT_param_01[20];
char WBGT_param_02[20];
char WBGT_param_03[20];
char WBGT_param_04[20];
char WBGT_param_05[20];
char WBGT_param_06[20];
double sound_level_3033 = 0;
double sound_level_3036 = 0;
double WBGT_data_01, WBGT_data_02, WBGT_data_03, WBGT_data_04, WBGT_data_05, WBGT_data_06;

int average_flag = 0;
int calibration_flag = 0;

int GSM_check_flag = 0;
int hours, minutes, seconds;
int day, month, year;
char global_time[100];

void sendAT_Data(const char *data);
uint8_t check_response(char *response, uint32_t timeout);
uint8_t MQTT_Config(uint8_t client_idx,
                    uint8_t enable_ssl, uint8_t SSL_ctx_idx,
                    uint16_t keep_alive,
                    uint8_t clean_session,
                    uint8_t msg_recv_mode, uint8_t msg_len_enable,
                    uint8_t will_fg, uint8_t will_qos, uint8_t will_retain, char *will_topic, char *will_message);
uint8_t SSL_config(uint8_t ssl_context_index, char *ca_cert, char *client_cert, char *client_key);
uint8_t SubscribeTopic(int client_idx, int msgid, char *topic, int qos);
uint8_t UnsubscribeTopic(int client_idx, int msgid, char *topic);
int MQTT_NetworkOpen(int client_idx, char *hostname, uint16_t port);
uint8_t MQTT_NetworkClose(int client_idx);
uint8_t MQTT_ClientConnect(int client_idx, char *username, char *passwd, char *clientID);
uint8_t MQTT_ClientDisconnect(int client_idx);
uint8_t PublishMessage(uint8_t client_idx, uint8_t msgid, uint8_t qos, uint8_t retain, char *topic);
uint8_t ReadMessage(int client_idx);
uint8_t Error_Report();
void UART_Initilaization_of_LTE(void);
uint8_t OT_command(char *cmd);
void timer_config();
void check_gsm_connection();
void init_LTE();
void Gsm_Power_Off();
void extractGPSData(char *inputString);
static int adc_raw[2][10];
static int voltage[2][10];
static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void example_adc_calibration_deinit(adc_cali_handle_t handle);
void process_sensor_status(const char *json_string);
char *JSON_Types(int type);
void JSON_Parse(const cJSON *const root);
uint8_t Publish_Error_Message(uint8_t client_idx, uint8_t msgid, uint8_t qos, uint8_t retain, char *topic, int count_error);
uint8_t Publish_RS232_Error_Message(uint8_t client_idx, uint8_t msgid, uint8_t qos, uint8_t retain, char *topic, int count_error);
uint8_t Publish_config_reply_message(uint8_t client_idx, uint8_t msgid, uint8_t qos, uint8_t retain, char *topic);
void Initialization_of_LTE();
void send_GNSS_CMD();

#define SUCCESS 1
#define FAILURE 0
#define MAX_WAIT_MS 100
#define CLIENT_IDX 0
// #define TAG "UART"
#define BUF_SIZE 4096

#define TXD_PIN 17
#define RXD_PIN 18
#define CTS_PIN 21
#define RTS_PIN 26
// #define LTE_DTR

#define I2C_MASTER_SCL_IO 6         /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO 5         /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 0            /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

#define MPU9250_SENSOR_ADDR 0x74       // 0x48//        /*!< Slave address of the MPU9250 sensor */
#define MPU9250_WHO_AM_I_REG_ADDR 0x02 // 0x00//0x02        /*!< Register addresses of the "who am I" register */

#define TEMP_SENSOR_ADDR 0x48 // 0x48//        /*!< Slave address of the MPU9250 sensor */
#define TEMP_REG_ADDR 0x00    // 0x00//0x02

#define MPU9250_PWR_MGMT_1_REG_ADDR 0x6B /*!< Register addresses of the power managment register */
#define MPU9250_RESET_BIT 7

#define INPUT_PORT0 0x00
#define INPUT_PORT1 0x01
#define OUTPUT_PORT0 0x02
#define OUTPUT_PORT1 0x03
#define POL_INVERT_PORT0 0x04
#define POL_INVERT_PORT1 0x05
#define CONFIG_PORT0 0x06
#define CONFIG_PORT1 0x07

adc_oneshot_unit_handle_t adc1_handle;
adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
};
adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_BITWIDTH_DEFAULT,
    .atten = EXAMPLE_ADC_ATTEN,
};
adc_cali_handle_t adc1_cali_chan0_handle = NULL;
adc_cali_handle_t adc1_cali_chan1_handle = NULL;
bool do_calibration1_chan0, do_calibration1_chan1;

void extractBeaconData(const uint8_t *adv_data, size_t adv_data_len, int8_t rssi, const char *dev_name)
{
    // Check if the advertising packet is a beacon packet and matches the filter criteria
    // In a typical scenario, beacon data is included in Manufacturer Specific Data (Type 0xFF)
    if (adv_data_len > 2 && adv_data[0] == 0xFF && adv_data[1] == 0x4C)
    {
        // Assuming the beacon data starts from byte 2
        BeaconData beacon;
        memcpy(beacon.uuid, &adv_data[2], sizeof(beacon.uuid));
        beacon.rssi = rssi;

        // Now you can use the extracted beacon data as needed
        printf("Beacon UUID: ");
        for (int i = 0; i < sizeof(beacon.uuid); ++i)
        {
            printf("%02X", beacon.uuid[i]);
        }
        printf("\n");
        printf("RSSI: %d\n", beacon.rssi);

        // Check if the MAC address or beacon name matches the filter
        if (strcmp(dev_name, TARGET_BEACON_NAME) == 0)
        {
            printf("Found target beacon!\n");
            // Add your desired actions for the target beacon here
        }
    }
}

static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;

    switch (event)
    {
    case ESP_GATTC_REG_EVT:
        ESP_LOGI(GATTC_TAG, "REG_EVT");
        esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
        if (scan_ret)
        {
#if DEBUG_LOG
            ESP_LOGE(GATTC_TAG, "set scan params error, error code = %x", scan_ret);
#endif
        }
        break;
    case ESP_GATTC_CONNECT_EVT:
    {
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_CONNECT_EVT conn_id %d, if %d", p_data->connect.conn_id, gattc_if);
        gl_profile_tab[PROFILE_A_APP_ID].conn_id = p_data->connect.conn_id;
        memcpy(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, p_data->connect.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(GATTC_TAG, "REMOTE BDA:");
        esp_log_buffer_hex(GATTC_TAG, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, sizeof(esp_bd_addr_t));
        esp_err_t mtu_ret = esp_ble_gattc_send_mtu_req(gattc_if, p_data->connect.conn_id);
        if (mtu_ret)
        {
            ESP_LOGE(GATTC_TAG, "config MTU error, error code = %x", mtu_ret);
        }
        break;
    }
    case ESP_GATTC_OPEN_EVT:
        if (param->open.status != ESP_GATT_OK)
        {
            ESP_LOGE(GATTC_TAG, "open failed, status %d", p_data->open.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "open success");
        break;
    case ESP_GATTC_DIS_SRVC_CMPL_EVT:
        if (param->dis_srvc_cmpl.status != ESP_GATT_OK)
        {
            ESP_LOGE(GATTC_TAG, "discover service failed, status %d", param->dis_srvc_cmpl.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "discover service complete conn_id %d", param->dis_srvc_cmpl.conn_id);
        esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
        break;
    case ESP_GATTC_CFG_MTU_EVT:
        if (param->cfg_mtu.status != ESP_GATT_OK)
        {
            ESP_LOGE(GATTC_TAG, "config mtu failed, error status = %x", param->cfg_mtu.status);
        }
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu, param->cfg_mtu.conn_id);
        break;
    case ESP_GATTC_SEARCH_RES_EVT:
    {
        ESP_LOGI(GATTC_TAG, "SEARCH RES: conn_id = %x is primary service %d", p_data->search_res.conn_id, p_data->search_res.is_primary);
        ESP_LOGI(GATTC_TAG, "start handle %d end handle %d current handle value %d", p_data->search_res.start_handle, p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);
        //        if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16 && p_data->search_res.srvc_id.uuid.uuid.uuid16 == REMOTE_SERVICE_UUID) {
        //            ESP_LOGI(GATTC_TAG, "service found");
        get_server = true;
        //            gl_profile_tab[PROFILE_A_APP_ID].service_start_handle = p_data->search_res.start_handle;
        //            gl_profile_tab[PROFILE_A_APP_ID].service_end_handle = p_data->search_res.end_handle;
        //            ESP_LOGI(GATTC_TAG, "UUID16: %x", p_data->search_res.srvc_id.uuid.uuid.uuid16);
        //        }
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
        if (p_data->search_cmpl.status != ESP_GATT_OK)
        {
            ESP_LOGE(GATTC_TAG, "search service failed, error status = %x", p_data->search_cmpl.status);
            break;
        }
        if (p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_REMOTE_DEVICE)
        {
            ESP_LOGI(GATTC_TAG, "Get service information from remote device");
        }
        else if (p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_NVS_FLASH)
        {
            ESP_LOGI(GATTC_TAG, "Get service information from flash");
        }
        else
        {
            ESP_LOGI(GATTC_TAG, "unknown service source");
        }
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_SEARCH_CMPL_EVT");
        if (get_server)
        {
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count(gattc_if,
                                                                    p_data->search_cmpl.conn_id,
                                                                    ESP_GATT_DB_CHARACTERISTIC,
                                                                    gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                    gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                                    INVALID_HANDLE,
                                                                    &count);
            if (status != ESP_GATT_OK)
            {
                ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_attr_count error");
            }

            if (count > 0)
            {
                char_elem_result = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result)
                {
                    ESP_LOGE(GATTC_TAG, "gattc no mem");
                }
                else
                {
                    status = esp_ble_gattc_get_char_by_uuid(gattc_if,
                                                            p_data->search_cmpl.conn_id,
                                                            gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                            gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                            remote_filter_char_uuid,
                                                            char_elem_result,
                                                            &count);
                    if (status != ESP_GATT_OK)
                    {
                        ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_char_by_uuid error");
                    }

                    /*  Every service have only one char in our 'ESP_GATTS_DEMO' demo, so we used first 'char_elem_result' */
                    if (count > 0 && (char_elem_result[0].properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY))
                    {
                        gl_profile_tab[PROFILE_A_APP_ID].char_handle = char_elem_result[0].char_handle;
                        esp_ble_gattc_register_for_notify(gattc_if, gl_profile_tab[PROFILE_A_APP_ID].remote_bda, char_elem_result[0].char_handle);
                    }
                }
                /* free char_elem_result */
                free(char_elem_result);
            }
            else
            {
                ESP_LOGE(GATTC_TAG, "no char found");
            }
        }
        break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT:
    {
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_REG_FOR_NOTIFY_EVT");
        if (p_data->reg_for_notify.status != ESP_GATT_OK)
        {
            ESP_LOGE(GATTC_TAG, "REG FOR NOTIFY failed: error status = %d", p_data->reg_for_notify.status);
        }
        else
        {
            uint16_t count = 0;
            uint16_t notify_en = 1;
            esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count(gattc_if,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                        ESP_GATT_DB_DESCRIPTOR,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                                                        &count);
            if (ret_status != ESP_GATT_OK)
            {
                ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_attr_count error");
            }
            if (count > 0)
            {
                descr_elem_result = malloc(sizeof(esp_gattc_descr_elem_t) * count);
                if (!descr_elem_result)
                {
                    ESP_LOGE(GATTC_TAG, "malloc error, gattc no mem");
                }
                else
                {
                    ret_status = esp_ble_gattc_get_descr_by_char_handle(gattc_if,
                                                                        gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                        p_data->reg_for_notify.handle,
                                                                        notify_descr_uuid,
                                                                        descr_elem_result,
                                                                        &count);
                    if (ret_status != ESP_GATT_OK)
                    {
                        ESP_LOGE(GATTC_TAG, "esp_ble_gattc_get_descr_by_char_handle error");
                    }
                    /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                    if (count > 0 && descr_elem_result[0].uuid.len == ESP_UUID_LEN_16 && descr_elem_result[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG)
                    {
                        ret_status = esp_ble_gattc_write_char_descr(gattc_if,
                                                                    gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                    descr_elem_result[0].handle,
                                                                    sizeof(notify_en),
                                                                    (uint8_t *)&notify_en,
                                                                    ESP_GATT_WRITE_TYPE_RSP,
                                                                    ESP_GATT_AUTH_REQ_NONE);
                    }

                    if (ret_status != ESP_GATT_OK)
                    {
                        ESP_LOGE(GATTC_TAG, "esp_ble_gattc_write_char_descr error");
                    }

                    /* free descr_elem_result */
                    free(descr_elem_result);
                }
            }
            else
            {
                ESP_LOGE(GATTC_TAG, "decsr not found");
            }
        }
        break;
    }
    case ESP_GATTC_NOTIFY_EVT:
        if (p_data->notify.is_notify)
        {
            ESP_LOGI(GATTC_TAG, "ESP_GATTC_NOTIFY_EVT, receive notify value:");
        }
        else
        {
            ESP_LOGI(GATTC_TAG, "ESP_GATTC_NOTIFY_EVT, receive indicate value:");
        }
        esp_log_buffer_hex(GATTC_TAG, p_data->notify.value, p_data->notify.value_len);
        for (int i = 0; i < p_data->notify.value_len; ++i)
        {
            ESP_LOGI(GATTC_TAG, "Byte %d: 0x%02X", i, p_data->notify.value[i]);
        }
        break;
    case ESP_GATTC_WRITE_DESCR_EVT:
        if (p_data->write.status != ESP_GATT_OK)
        {
            ESP_LOGE(GATTC_TAG, "write descr failed, error status = %x", p_data->write.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "write descr success");
        uint8_t write_char_data[35];
        for (int i = 0; i < sizeof(write_char_data); ++i)
        {
            write_char_data[i] = i % 256;
        }
        esp_ble_gattc_write_char(gattc_if,
                                 gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                 gl_profile_tab[PROFILE_A_APP_ID].char_handle,
                                 sizeof(write_char_data),
                                 write_char_data,
                                 ESP_GATT_WRITE_TYPE_RSP,
                                 ESP_GATT_AUTH_REQ_NONE);
        break;
    case ESP_GATTC_SRVC_CHG_EVT:
    {
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:");
        esp_log_buffer_hex(GATTC_TAG, bda, sizeof(esp_bd_addr_t));
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:
        if (p_data->write.status != ESP_GATT_OK)
        {
            ESP_LOGE(GATTC_TAG, "write char failed, error status = %x", p_data->write.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "write char success ");
        break;
    case ESP_GATTC_DISCONNECT_EVT:
        connect = false;
        get_server = false;
        ESP_LOGI(GATTC_TAG, "ESP_GATTC_DISCONNECT_EVT, reason = %d", p_data->disconnect.reason);
        break;
    default:
        break;
    }
}

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    uint8_t *adv_name = NULL;
    uint8_t adv_name_len = 0;
    switch (event)
    {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
    {
        // the unit of the duration is second
        uint32_t duration = 15;
        esp_ble_gap_start_scanning(duration);
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        // scan start complete event to indicate scan start successfully or failed
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTC_TAG, "scan start failed, error status = %x", param->scan_start_cmpl.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "scan start success");

        break;
    case ESP_GAP_BLE_SCAN_RESULT_EVT:
    {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        char mac_str[18];
        sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X",
                scan_result->scan_rst.bda[0], scan_result->scan_rst.bda[1],
                scan_result->scan_rst.bda[2], scan_result->scan_rst.bda[3],
                scan_result->scan_rst.bda[4], scan_result->scan_rst.bda[5]);

        // Extract the device name if available
        //                    char dev_name[ESP_BLE_GAP_MAX_BDNAME_LEN + 1];
        //                    if (esp_ble_gap_get_device_name(scan_result->scan_rst.bda, dev_name) == ESP_OK) {
        //                        printf("Device Name: %s\n", dev_name);
        //                    }
        //
        //                    // Check if the device matches the MAC address or beacon name filter
        //                    if (strcmp(mac_str, TARGET_MAC_ADDRESS) == 0 || strcmp(dev_name, TARGET_BEACON_NAME) == 0) {
        //                        extractBeaconData(scan_result->scan_rst.ble_adv, scan_result->scan_rst.adv_data_len, scan_result->scan_rst.rssi, dev_name);
        //                    }

        switch (scan_result->scan_rst.search_evt)
        {
        case ESP_GAP_SEARCH_INQ_RES_EVT:

            //            esp_log_buffer_hex(GATTC_TAG, scan_result->scan_rst.bda, 6);

            //            ESP_LOGI(GATTC_TAG, "searched Adv Data Len %d, Scan Response Len %d", scan_result->scan_rst.adv_data_len, scan_result->scan_rst.scan_rsp_len);
            adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
                                                ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
            //            ESP_LOGI(GATTC_TAG, "searched Device Name Len %d", adv_name_len);
            esp_log_buffer_char(GATTC_TAG, adv_name, adv_name_len);

            // #if CONFIG_EXAMPLE_DUMP_ADV_DATA_AND_SCAN_RESP
            if (scan_result->scan_rst.adv_data_len > 0)
            {
#if DEBUG_LOG
                ESP_LOGI(GATTC_TAG, "adv data:");

                esp_log_buffer_hex(GATTC_TAG, &scan_result->scan_rst.ble_adv[0], scan_result->scan_rst.adv_data_len);
#endif
            }
            //            if (scan_result->scan_rst.scan_rsp_len > 0) {
            //                ESP_LOGI(GATTC_TAG, "scan resp:");
            //                esp_log_buffer_hex(GATTC_TAG, &scan_result->scan_rst.ble_adv[scan_result->scan_rst.adv_data_len], scan_result->scan_rst.scan_rsp_len);
            //            }
            // #endif
            //            ESP_LOGI(GATTC_TAG, "\n");

            if (adv_name != NULL)
            {
                if (strlen(remote_device_name) == adv_name_len && strncmp((char *)adv_name, remote_device_name, adv_name_len) == 0)
                {
                    ESP_LOGI(GATTC_TAG, "============searched device========= %s\n", remote_device_name);
                    //                    vTaskDelay(100);
                    if (connect == false)
                    {
                        connect = true;
                        ESP_LOGI(GATTC_TAG, "connect to the remote device.");
                        esp_ble_gap_stop_scanning();
                        esp_ble_gattc_open(gl_profile_tab[PROFILE_A_APP_ID].gattc_if, scan_result->scan_rst.bda, scan_result->scan_rst.ble_addr_type, true);
                    }
                    //                    vTaskDelay(100);
                }
            }
            break;
        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
            break;
        default:
            break;
        }
        break;
    }

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTC_TAG, "scan stop failed, error status = %x", param->scan_stop_cmpl.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "stop scan successfully");
        break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTC_TAG, "adv stop failed, error status = %x", param->adv_stop_cmpl.status);
            break;
        }
        ESP_LOGI(GATTC_TAG, "stop adv successfully");
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(GATTC_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                 param->update_conn_params.status,
                 param->update_conn_params.min_int,
                 param->update_conn_params.max_int,
                 param->update_conn_params.conn_int,
                 param->update_conn_params.latency,
                 param->update_conn_params.timeout);
        break;
    default:
        break;
    }
}

static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    /* If event is register event, store the gattc_if for each profile */
    if (event == ESP_GATTC_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
        }
        else
        {
            ESP_LOGI(GATTC_TAG, "reg app failed, app_id %04x, status %d",
                     param->reg.app_id,
                     param->reg.status);
            return;
        }
    }

    /* If the gattc_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do
    {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++)
        {
            if (gattc_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                gattc_if == gl_profile_tab[idx].gattc_if)
            {
                if (gl_profile_tab[idx].gattc_cb)
                {
                    gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
                }
            }
        }
    } while (0);
}

void ADC_Initialization()
{

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Config---------------//

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN1, &config));

    //-------------ADC1 Calibration Init---------------//

    do_calibration1_chan0 = example_adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN0, EXAMPLE_ADC_ATTEN, &adc1_cali_chan0_handle);
    do_calibration1_chan1 = example_adc_calibration_init(ADC_UNIT_1, EXAMPLE_ADC1_CHAN1, EXAMPLE_ADC_ATTEN, &adc1_cali_chan1_handle);
}

void read_battery_voltages()
{
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN0, &adc_raw[0][0]));
    //        ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN0, adc_raw[0][0]);
    if (do_calibration1_chan0)
    {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, adc_raw[0][0], &voltage[0][0]));

        double chan_vol = voltage[0][0];
        double cal_vol = chan_vol * (4.4050);
        lead_acid_bat = cal_vol / 1000;

#if DEBUG_LOG
        ESP_LOGI(TAG, "channel 1 volatge is %f mV", cal_vol);
#endif

        //            ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN0, voltage[0][0]);
    }
    vTaskDelay(1);

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN1, &adc_raw[0][1]));
    //        ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN1, adc_raw[0][1]);
    if (do_calibration1_chan1)
    {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan1_handle, adc_raw[0][1], &voltage[0][1]));
        lipo_bat = voltage[0][1];
        lipo_bat = lipo_bat / 1000;

#if DEBUG_LOG
        ESP_LOGI(TAG, "channel 2 volatge is %d mV", voltage[0][1]);
#endif
    }
}

static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
#endif
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
#endif
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK)
        {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Calibration Success");
#endif
    }
    else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated)
    {
#if DEBUG_LOG
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
#endif
    }
    else
    {
#if DEBUG_LOG
        ESP_LOGE(TAG, "Invalid arg or no memory");
#endif
    }

    return calibrated;
}

static void example_adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
#if DEBUG_LOG
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
#endif
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
#if DEBUG_LOG
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
#endif
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}

static esp_err_t mpu9250_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, MPU9250_SENSOR_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

static esp_err_t temp_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(0, 0x48, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
static esp_err_t mpu9250_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    int ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(I2C_MASTER_NUM, MPU9250_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    return ret;
}

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

static esp_err_t i2c_temp_sensor_init(void)
{
    int i2c_master_port = 1;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

// Function to convert UTC time to Indian Standard Time (IST)
void convertUtcToIst(char *utcTime_format)
{
    // Parse UTC time string
    int utcHours, utcMinutes, utcSeconds;
    sscanf(utcTime_format, "%2d%2d%2d", &utcHours, &utcMinutes, &utcSeconds);

    // Calculate IST time zone offset (5 hours and 30 minutes ahead of UTC)
    const int istOffsetHours = 5;
    const int istOffsetMinutes = 30;

    // Calculate IST time
    int istHours = (utcHours + istOffsetHours) % 24;
    int istMinutes = (utcMinutes + istOffsetMinutes) % 60;
    int istSeconds = utcSeconds;

    // Print IST time in HH:MM:SS format
    ESP_LOGI(TAG, "UTC Time: %s\n", utcTime_format);
    ESP_LOGI(TAG, "IST Time: %02d:%02d:%02d\n", istHours, istMinutes, istSeconds);

    //    char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
    time_stamp = strdup(time_stamp);
    sprintf(time_stamp, "%d:%d:%d\r\n", istHours, istMinutes, istSeconds);
    ESP_LOGI(TAG, "Formatted Time: %s\n", time_stamp);
    //    free(transmit_buffer);

    // Convert IST time to string format
    //    char timeString[9];
    //        snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", hours, minutes, seconds);
    //
    //        // Print the formatted time string
    //        ESP_LOGI(TAG,"Formatted Time: %s\n", timeString);
}

void UART_Initilaization_of_LTE(void)
{
    const uart_config_t uart_config = {
        //        .baud_rate = 9600,
        //        .data_bits = UART_DATA_8_BITS,
        //        .parity = UART_PARITY_DISABLE,
        //        .stop_bits = UART_STOP_BITS_1,
        //        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        //        .source_clk = UART_SCLK_APB,
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, RTS_PIN, CTS_PIN);
}

time_t now;
char strftime_buf[64];
struct tm timeinfo;
int RTC_onetime_flag = 1;

void RTC_date_time()
{

    if (RTC_onetime_flag)
    {
        RTC_onetime_flag = 0;
        timeinfo.tm_year = year;
        timeinfo.tm_mon = month;
        timeinfo.tm_mday = day;
        timeinfo.tm_hour = hours;
        timeinfo.tm_min = minutes;
        timeinfo.tm_sec = seconds;
    }

    //	    char* transmit_buffer = (char*) calloc(BUF_SIZE,sizeof(char));
    //	        seconds =(seconds+8);
    sprintf(global_time, "%d-%d-%dt%d:%d:%dz", year, month, day, hours, minutes, seconds);

    //	    	ESP_LOGI(TAG,"global time %s",global_time);

    //	    	sprintf(global_time,"dd%d:%d:%dtt%d:%d:%d",timeinfo.tm_year,timeinfo.tm_mon,timeinfo.tm_mday,timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);

#if DEBUG_LOG
    ESP_LOGI(TAG, "Current time: %d-%02d-%02d %02d:%02d:%02d\n",
             timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
//	    ESP_LOGI(TAG,"Current time: %d-%02d-%02d %02d:%02d:%02d\n",
//	   	               timeinfo.tm_year + 1953, timeinfo.tm_mon + 11, timeinfo.tm_mday+26,
//	   	               timeinfo.tm_hour+12, timeinfo.tm_min+18, timeinfo.tm_sec);
#endif
}

void sendAT_Data(const char *data)
{
    //	 gpio_set_level(LTE_RTS_PIN, 1);
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    //    sprintf((char*)transmit_buffer,"%s%s\r\n",SET_BAUD_RATE,data);

    int err = uart_write_bytes(UART_NUM_1, data, strlen(data));
    //    ESP_LOGI(TAG, "Wrote %d bytes", err);
    //    ESP_LOGI(TAG, "Write string: %s\r\n",(char *) data);
    //    ESP_LOGI(TAG, "uart_wr_err_code : %d\r\n",err);
    if (err != -1)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "AT Command sent : %s", data);
#endif
    }

    else
    {
#if DEBUG_LOG
        ESP_LOGE(TAG, "Error in sending AT command to the EC200!!!");
#endif
    }
    //    vTaskDelay(100);
    //    gpio_set_level(LTE_RTS_PIN, 0);
}
// char *inputString ;//= "+QGPSLOC:113223.0,1302.9435N,08005.3979E,0.6,9.0,2,152.09,271123,09";
// char utcTime[7];
// char latitude_01[10];
// char longitude_01[11];
// char date[7];
// char numSatellites[3];
bool isGPSLocationString(char *inputStr)
{
    // Check if the input string starts with "+QGPSLOC:"
    return (strncmp(inputStr, "\r\n+QGPSLOC:", strlen("\r\n+QGPSLOC:")) == 0);
}

void extractGPSData(char *inputString)
{

    const char *start = strstr(inputString, "+QGPSLOC:"); // Find the start of the GPS data
    const char *end = strstr(inputString, "\r\n");        // Find the end of the GPS data

    if (start != NULL && end != NULL)
    {
        size_t dataSize = end - start - strlen("+QGPSLOC:"); // Calculate the size of GPS data excluding the identifier
        char extractedData[dataSize];
        strncpy(extractedData, start + strlen("+QGPSLOC:"), dataSize);
        extractedData[dataSize] = '\0'; // Null-terminate the extracted data

        // Now 'extractedData' contains the GPS data
        ESP_LOGI(TAG, "Extracted GPS Data: %s\n", extractedData);

        // Parse the specific fields (UTC time, latitude, longitude, satellites)
        char utcTime[7], latitude[10], longitude[10], satellites[3];
        sscanf(extractedData, "%6s,%9[^,],%9[^,],%*f,%*f,%*d,%*f,%*f,%*f,%*d,%2s", utcTime, latitude, longitude, satellites);

        // Print the extracted values
        ESP_LOGI(TAG, "UTC Time: %s\n", utcTime);
        ESP_LOGI(TAG, "Latitude: %s\n", latitude);
        ESP_LOGI(TAG, "Longitude: %s\n", longitude);
        ESP_LOGI(TAG, "Number of Satellites: %s\n", satellites);
    }
    else
    {
        ESP_LOGI(TAG, "Error: GPS Data not found in the input string\n");
    }
}

int json_flag = 0;

void check_json_received_data(char *buf, uint32_t length)
{
    const char *start = strchr(buf, '{'); // Find the start of the data
    const char *end = strchr(buf, '}');   // Find the end of the data
    int sensor_enable_count = 0;
    int rs232_sensor_count = 0;
    if (start != NULL && end != NULL)
    {
        size_t dataSize = end - start + 1;
        char extractedData[dataSize];
        strncpy(extractedData, start, dataSize);
        extractedData[dataSize] = '\0'; // Null-terminate the extracted data

        // Now 'extractedData' contains the data between '{' and '}'
#if DEBUG_LOG
        ESP_LOGI(TAG, "Extracted Data: %s\n", extractedData);
#endif

        cJSON *root2 = cJSON_Parse(extractedData);
        //                        json_flag=0;
        if (cJSON_GetObjectItem(root2, "device_id"))
        {
            char *dev_id_check = cJSON_GetObjectItem(root2, "device_id")->valuestring;
#if DEBUG_LOG
            ESP_LOGI(TAG, "device_id=%s", dev_id_check);
#endif
        }
        if (cJSON_GetObjectItem(root2, "msg_id"))
        {
            char *msg_id_check = cJSON_GetObjectItem(root2, "msg_id")->valuestring;
#if DEBUG_LOG
            ESP_LOGI(TAG, "msg_id=%s", msg_id_check);
#endif

            if (!strcmp(msg_id_check, "config_packet"))
            {
                Publish_config_reply_message(CLIENT_IDX, 1, 1, 0, SEND_EVETS_TOPIC); //
            }
        }
        if (cJSON_GetObjectItem(root2, "wind_sr"))
        {
            wind_sen_state = cJSON_GetObjectItem(root2, "wind_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "wind_sen_state=%d", wind_sen_state);
#endif
            if (wind_sen_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "dust_conc_sr"))
        {
            dust_conc_sen_state = cJSON_GetObjectItem(root2, "dust_conc_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "dust_con_sen_state=%d", dust_conc_sen_state);
#endif
            if (dust_conc_sen_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "CO2_sr"))
        {
            CO2_sen_state = cJSON_GetObjectItem(root2, "CO2_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "CO2_sen_state=%d", CO2_sen_state);
#endif
            if (CO2_sen_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "air_quality_sr"))
        {
            air_quality_sr_state = cJSON_GetObjectItem(root2, "air_quality_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "air_quality_sr_state=%d", air_quality_sr_state);
#endif
            if (air_quality_sr_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "VOC_sr"))
        {
            VOC_sr_state = cJSON_GetObjectItem(root2, "VOC_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "VOC_sr_state=%d", VOC_sr_state);
#endif
            if (VOC_sr_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "formaldehyde_sr"))
        {
            formaldehyde_sr_state = cJSON_GetObjectItem(root2, "formaldehyde_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "formaldehyde_sr_state=%d", formaldehyde_sr_state);
#endif
            if (formaldehyde_sr_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "UV_rad_sr"))
        {
            UV_rad_sr_state = cJSON_GetObjectItem(root2, "UV_rad_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "UV_rad_sr_state=%d", UV_rad_sr_state);
#endif
            if (UV_rad_sr_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "solar_illum_sr"))
        {
            solar_illum_sr_state = cJSON_GetObjectItem(root2, "solar_illum_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "solar_illum_sr_state=%d", solar_illum_sr_state);
#endif
            if (solar_illum_sr_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "wind_dir_sr"))
        {
            wind_dir_sr_state = cJSON_GetObjectItem(root2, "wind_dir_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "wind_dir_sr_state=%d", wind_dir_sr_state);
#endif
            if (wind_dir_sr_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "solar_rad_sr"))
        {
            solar_rad_sr_state = cJSON_GetObjectItem(root2, "solar_rad_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "solar_rad_sr_state=%d", solar_rad_sr_state);
#endif
            if (solar_rad_sr_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "SO2_conc_sr"))
        {
            SO2_conc_sr_state = cJSON_GetObjectItem(root2, "SO2_conc_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "SO2_conc_sr_state=%d", SO2_conc_sr_state);
#endif
            if (SO2_conc_sr_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "Atm_hum_pres_sr"))
        {
            Atm_hum_pres_sr_state = cJSON_GetObjectItem(root2, "Atm_hum_pres_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "Atm_hum_pres_sr_state=%d", Atm_hum_pres_sr_state);
#endif
            if (Atm_hum_pres_sr_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "NO2_conc_sr"))
        {
            NO2_conc_sr_state = cJSON_GetObjectItem(root2, "NO2_conc_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "NO2_conc_sr_state=%d", NO2_conc_sr_state);
#endif
            if (NO2_conc_sr_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "CO_conc_sr"))
        {
            CO_conc_sr_state = cJSON_GetObjectItem(root2, "CO_conc_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "CO_conc_sr_state=%d", CO_conc_sr_state);
#endif
            if (CO_conc_sr_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "O3_conc_sr"))
        {
            O3_conc_sr_state = cJSON_GetObjectItem(root2, "O3_conc_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "O3_conc_sr_state=%d", O3_conc_sr_state);
#endif
            if (O3_conc_sr_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "rain_sr"))
        {
            rain_sr_state = cJSON_GetObjectItem(root2, "rain_sr")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "rain_sr_state=%d", rain_sr_state);
#endif
            if (rain_sr_state)
            {
                sensor_enable_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "HD2010UC_class_1"))
        {
            HD2010UC_class_1_state = cJSON_GetObjectItem(root2, "HD2010UC_class_1")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "HD2010UC_class_1_state=%d", HD2010UC_class_1_state);
#endif
            if (HD2010UC_class_1_state)
            {
                rs232_sensor_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "SL_4033SD"))
        {
            SL_4033SD_state = cJSON_GetObjectItem(root2, "SL_4033SD")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "SL_4033SD_state=%d", SL_4033SD_state);
#endif
            if (SL_4033SD_state)
            {
                rs232_sensor_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "SL_4036SD"))
        {
            SL_4036SD_state = cJSON_GetObjectItem(root2, "SL_4036SD")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "SL_4036SD_state=%d", SL_4036SD_state);
#endif
            if (SL_4036SD_state)
            {
                rs232_sensor_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "VM_6380"))
        {
            VM_6380_state = cJSON_GetObjectItem(root2, "VM_6380")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "VM_6380_state=%d", VM_6380_state);
#endif
            if (VM_6380_state)
            {
                rs232_sensor_count++;
            }
        }
        if (cJSON_GetObjectItem(root2, "WBGT_2010SD"))
        {
            WBGT_2010SD_state = cJSON_GetObjectItem(root2, "WBGT_2010SD")->valueint;
#if DEBUG_LOG
            ESP_LOGI(TAG, "WBGT_state=%d", WBGT_2010SD_state);
#endif
            if (WBGT_2010SD_state)
            {
                rs232_sensor_count++;
            }
        }

        if (sensor_enable_count > 10)
        {
#if DEBUG_LOG
            ESP_LOGI(TAG, "sensor enable count exceed the max limit");
#endif
            Publish_Error_Message(CLIENT_IDX, 1, 1, 0, SEND_EVETS_TOPIC, sensor_enable_count);

            sensor_enable_count = 0;
        }

        if (rs232_sensor_count > 1)
        {
#if DEBUG_LOG
            ESP_LOGI(TAG, "rs232 sensor exceed the max limit");
#endif
            Publish_RS232_Error_Message(CLIENT_IDX, 1, 1, 0, SEND_EVETS_TOPIC, sensor_enable_count);
            rs232_sensor_count = 0;
        }
    }
    else
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Error: Data not found between '{' and '}'\n");
#endif
    }
}

uint8_t check_response(char *response, uint32_t timeout)
{
    char *data = (char *)calloc(BUF_SIZE, sizeof(char));
    //		char* GNSS_data = (char*) calloc(BUF_SIZE,sizeof(char));

    uint32_t time = esp_timer_get_time() / 1000ULL;
    while ((esp_timer_get_time() / 1000ULL) - time < timeout)
    {
        //			ESP_LOGI(TAG, "LLLLLLLLLLLLLLLl");
        uint32_t length = uart_read_bytes(UART_NUM_1, data, BUF_SIZE, 100);
#if DEBUG_LOG
        ESP_LOGI(TAG, "Received string : %s\r\n", (char *)data);
#endif
        if (length > 0)
        {
            check_json_received_data(data, length);
            if (isGPSLocationString(data))
            {
//			    	    extractGPSData(data);
#if DEBUG_LOG
                ESP_LOGI(TAG, "======GPS location received=======");
#endif

                const char *colonPos = strchr(data, ':'); // Find the start of the GPS data
                if (colonPos != NULL)
                {
                    // Move to the character after the colon
                    colonPos++;

                    // Tokenize the string based on comma delimiter
                    char *token = strtok(colonPos, ",");

                    // Variables to store extracted data

                    // Assign values to variables based on token index
                    int tokenIndex = 0;
                    while (token != NULL)
                    {
                        switch (tokenIndex)
                        {
                        case 0:
                            utcTime = token;
                            break;
                        case 1:
                            latitude = token;
                            break;
                        case 2:
                            longitude = token;
                            break;
                        case 9:
                            gps_day = token;
                            break;
                        case 10:
                            numberOfSatellites = atoi(token);
                            break;
                        default:
                            // Handle other fields if needed
                            break;
                        }

                        // Move to the next token
                        token = strtok(NULL, ",");
                        tokenIndex++;
                    }
                    utcTime = strdup(utcTime);
                    latitude = strdup(latitude);
                    longitude = strdup(longitude);
                    gps_day = strdup(gps_day);
                    //			    		         numberOfSatellites = strdup(numberOfSatellites);

                    // Print or use the extracted values
#if DEBUG_LOG
                    ESP_LOGI(TAG, "UTC Time: %s\n", utcTime);
                    ESP_LOGI(TAG, "UTC Day: %s\n", gps_day);

                    ESP_LOGI(TAG, "Latitude: %s\n", latitude);

                    ESP_LOGI(TAG, "Longitude: %s\n", longitude);

                    ESP_LOGI(TAG, "Number of Satellites: %d\n", numberOfSatellites);

#endif
                    sscanf(utcTime, "%2d%2d%2d", &hours, &minutes, &seconds);
                    sscanf(gps_day, "%2d%2d%2d", &day, &month, &year);
                    free(data);
                    return SUCCESS;
                }
                else
                {
#if DEBUG_LOG
                    ESP_LOGI(TAG, "Colon not found in the input string.\n");
                    utcTime = strdup(utcTime);
                    latitude = strdup(latitude);
                    longitude = strdup(longitude);
                    gps_day = strdup(gps_day);
#endif
                }
            }
            //			   if(isGPSLocationString(data)) {

            //			   }
        }
        if (length > 0)
        {
            if (strstr((const char *)data, (const char *)response))
            {
#if DEBUG_LOG
                ESP_LOGI(TAG, "Received string : %s\r\n", (char *)data);
#endif

                free(data);
                return SUCCESS;
            }
        }
    }
    free(data);
    return FAILURE;
}

uint8_t Set_BaudRate(int baud_rate)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    sprintf((char *)transmit_buffer, "%s%d\r\n", SET_BAUD_RATE, baud_rate);
    sendAT_Data(transmit_buffer);
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Baud Rate Set\r\n");
#endif
        free(transmit_buffer);
        return SUCCESS;
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Baud Rate configuration failed\r\n");
#endif
    free(transmit_buffer);
    return FAILURE;
}

uint8_t get_loacalTime()
{
    sendAT_Data(GET_TIME);
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
        return SUCCESS;
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Time synchronization failed\r\n");
#endif
    return FAILURE;
}

uint8_t MQTT_Config(uint8_t client_idx,
                    uint8_t enable_ssl, uint8_t SSL_ctx_idx,
                    uint16_t keep_alive,
                    uint8_t clean_session,
                    uint8_t msg_recv_mode, uint8_t msg_len_enable,
                    uint8_t will_fg, uint8_t will_qos, uint8_t will_retain, char *will_topic, char *will_message)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    //		sprintf((char*)transmit_buffer,"%s%d,%d,%d\r\n",ENABLE_SSL,client_idx,enable_ssl,SSL_ctx_idx);
    //	    sendAT_Data(transmit_buffer);
    //	    memset(transmit_buffer,'\0',strlen(transmit_buffer));
    //		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS) ==	SUCCESS ){
    //			ESP_LOGI(TAG,"SSL Enabled");
    //		}
    //		sprintf((char*)transmit_buffer,"%s%d,4\r\n",MQTT_VERSION,client_idx);
    //		sendAT_Data(transmit_buffer);
    //		memset(transmit_buffer,'\0',strlen(transmit_buffer));
    //		if(check_response(OK_RESPONSE,3*MAX_WAIT_MS) ==	SUCCESS ){
    //			ESP_LOGI(TAG,"SSL Version set");
    //		}
    vTaskDelay(100);
    sprintf((char *)transmit_buffer, "%s%d,%d\r\n", MQTT_KEEP_ALIVE, client_idx, keep_alive);
    sendAT_Data(transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Keep_alive time set to %d s", keep_alive);
#endif
    }
    else
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Keep_alive not set");
#endif
    }

    vTaskDelay(100);
    sprintf((char *)transmit_buffer, "%s%d,%d,%d\r\n", MQTT_RECV_MODE, client_idx, msg_recv_mode, msg_len_enable);
    sendAT_Data(transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Receive mode configured");
#endif
    }
    vTaskDelay(100);
    sprintf((char *)transmit_buffer, "%s%d,%d,%d\r\n", MQTT_SEND_MODE, client_idx, msg_recv_mode, msg_len_enable);
    sendAT_Data(transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Send mode configured");
#endif
    }
    else
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Send mode not configured");
#endif
    }

    vTaskDelay(100);
    sprintf((char *)transmit_buffer, "%s%d,%d,%d,%d,\"%s\",\"%s\"\r\n", MQTT_WILL_CONFIG, client_idx, will_fg, will_qos, will_retain, will_topic, will_message);
    sendAT_Data(transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Will Parameters configured");
#endif
    }
    vTaskDelay(100);
    sprintf((char *)transmit_buffer, "%s%d,%d\r\n", CLEAN_SESSION, client_idx, clean_session);
    sendAT_Data(transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Session Clean : %d", clean_session);
#endif
    }
    else
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Session not  Clean");
#endif
    }
    vTaskDelay(100);
    send_GNSS_CMD();
    free(transmit_buffer);
    return SUCCESS;
}

uint8_t SubscribeTopic(int client_idx, int msgid, char *topic, int qos)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    sprintf((char *)transmit_buffer, "%s%d,%d,\"%s\",%d\r\n", SUB_TO_TOPIC, client_idx, msgid, topic, qos);
    sendAT_Data((char *)transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    sprintf((char *)transmit_buffer, "%s%d,%d,0\r\n", MQTT_SUB_RESPONSE, client_idx, msgid);
    if (check_response(MQTT_SUB_RESPONSE, 5 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Subscribed to topic:%s\r\n", topic);
#endif
        free(transmit_buffer);
        subscribe_flag = 1;
        return SUCCESS;
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Could not Subscribe to Topic. \r\n");
#endif
    free(transmit_buffer);
    return FAILURE;
}

uint8_t UnsubscribeTopic(int client_idx, int msgid, char *topic)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    sprintf((char *)transmit_buffer, "%s%d,%d,\"%s\"\r\n", UNSUB_TO_TOPIC, client_idx, msgid, topic);
    sendAT_Data((char *)transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    sprintf((char *)transmit_buffer, "%s%d,%d,0\r\n", MQTT_UNSUB_RESPONSE, client_idx, msgid);
    if (check_response(OK_RESPONSE, 150 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Unsubscribed from topic:%s\r\n", topic);
#endif
        free(transmit_buffer);
        subscribe_flag = 0;
        return SUCCESS;
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Could not Unsubscribe from Topic. \r\n");
#endif
    free(transmit_buffer);
    return FAILURE;
}

int MQTT_NetworkOpen(int client_idx, char *hostname, uint16_t port)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    sprintf((char *)transmit_buffer, "%s%d,\"%s\",%d\r\n", MQTT_NETWORK_OPEN, client_idx, hostname, port);
    sendAT_Data((char *)transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    sprintf((char *)transmit_buffer, "%s%d,0\r\n", MQTT_NETWORK_OPEN_RESPONSE, client_idx);
    //	    int con_sts=check_response(OK_RESPONSE,10*MAX_WAIT_MS);
    //	    ESP_LOGI(TAG, "Client connection error code =%d",con_sts);
    if (check_response(OK_RESPONSE, 10 * MAX_WAIT_MS) == SUCCESS)
    {
        // #if DEBUG_LOG
        ESP_LOGI(TAG, "Connected to network at:%s\r\n", hostname);
        // #endif
        free(transmit_buffer);
        network_flag = 1;
        return SUCCESS;
    }
    else
    {
        memset(transmit_buffer, '\0', strlen(transmit_buffer));
        sprintf((char *)transmit_buffer, "%s%d,2\r\n", MQTT_NETWORK_OPEN_RESPONSE, client_idx);
        if (check_response(transmit_buffer, 10 * MAX_WAIT_MS) == SUCCESS)
        {
            return 2;
        }
    }
    // #if DEBUG_LOG
    ESP_LOGI(TAG, "Could not Connect to network. \r\n");
    // #endif
    network_flag = 0;
    GSM_check_flag = 1;
    free(transmit_buffer);
    return FAILURE;
}

uint8_t MQTT_NetworkClose(int client_idx)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    sprintf((char *)transmit_buffer, "%s%d\r\n", MQTT_NETWORK_CLOSE, client_idx);
    sendAT_Data((char *)transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    sprintf((char *)transmit_buffer, "%s%d,0\r\n", MQTT_NETWORK_CLOSE_RESPONSE, client_idx);
    if (check_response(transmit_buffer, 10 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Closed MQTT network");
#endif
        free(transmit_buffer);
        network_flag = 0;
        return SUCCESS;
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Could not close MQTT network. \r\n");
#endif
    free(transmit_buffer);
    return FAILURE;
}

uint8_t MQTT_ClientConnect(int client_idx, char *username, char *passwd, char *clientID)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    sprintf((char *)transmit_buffer, "%s%d,\"%s\",\"%s\",\"%s\"\r\n", MQTT_CLIENT_CONN, client_idx, clientID, username, passwd);
    sendAT_Data((char *)transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    sprintf((char *)transmit_buffer, "%s%d,0,0\r\n", MQTT_CLIENT_CONN_RESPONSE, client_idx);
    //	    int cl_sa=check_response(transmit_buffer,5*MAX_WAIT_MS);
    //	    ESP_LOGI(TAG,"broker connection error code= %d",cl_sa);
    if (check_response(transmit_buffer, 10 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Connected client to broker: %s\r\n", username);
#endif
        free(transmit_buffer);
        client_flag = 1;
        return SUCCESS;
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Could not Connect client to broker.\r\n");
#endif
    client_flag = 0;
    network_flag = 0;
    free(transmit_buffer);
    return FAILURE;
}

uint8_t MQTT_ClientDisconnect(int client_idx)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    sprintf((char *)transmit_buffer, "%s%d\r\n", MQTT_CLIENT_DISCONN, client_idx);
    sendAT_Data((char *)transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    sprintf((char *)transmit_buffer, "%s%d,0", MQTT_CLIENT_DISCONN_RESPONSE, client_idx);
    if (check_response(OK_RESPONSE, 10 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Disconnected client from broker");
#endif
        free(transmit_buffer);
        client_flag = 0;
        return SUCCESS;
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Could not Disconnect client from broker.\r\n");
#endif
    free(transmit_buffer);
    return FAILURE;
}

uint8_t Publish_battery_alert_msg(uint8_t client_idx, uint8_t msgid, uint8_t qos, uint8_t retain, char *topic)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    char *msg = (char *)calloc(BUF_SIZE, sizeof(char));

    //		    sprintf(msg,config_err_pkt , device_id, config_error_msg_id,count_error);
    if (lead_acid_bat < BAT_MAX_LIMIT)
    {
        sprintf(msg, "battery low");
    }
    else
    {
        //            	  sprintf(msg,"");
    }

    sprintf((char *)transmit_buffer, "%s%d,%d,%d,%d,\"%s\",%d\r\n", PUB_MSG, client_idx, msgid, qos, retain, topic, strlen(msg));
    sendAT_Data((char *)transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    sprintf((char *)transmit_buffer, "%s%d,%d,0\r\n", MQTT_PUB_MSG_RESPONSE, client_idx, msgid);
    if (check_response(PROMPT, 50 * MAX_WAIT_MS) == SUCCESS)
    {
        //				ESP_LOGI(TAG,"Publish msg");
        uart_write_bytes(UART_NUM_1, msg, strlen(msg));
        if (check_response(transmit_buffer, 60 * MAX_WAIT_MS) == SUCCESS)
        {
#if DEBUG_LOG
            ESP_LOGI(TAG, "Published battery alert packet:%s\r\n", msg);
#endif
            free(transmit_buffer);
            return SUCCESS;
        }
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Could not publish battery alert message.\r\n");
#endif
    free(transmit_buffer);
    //			network_fl
    client_flag = 0;
    return FAILURE;
}

uint8_t Publish_temp_alert_msg(uint8_t client_idx, uint8_t msgid, uint8_t qos, uint8_t retain, char *topic)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    char *msg = (char *)calloc(BUF_SIZE, sizeof(char));

    //		    sprintf(msg,config_err_pkt , device_id, config_error_msg_id,count_error);
    if (temp_sensor > 40)
    {
        sprintf(msg, "temperature high");
    }
    if (temp_sensor < 10)
    {
        sprintf(msg, "temperature low ");
    }

    sprintf((char *)transmit_buffer, "%s%d,%d,%d,%d,\"%s\",%d\r\n", PUB_MSG, client_idx, msgid, qos, retain, topic, strlen(msg));
    sendAT_Data((char *)transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    sprintf((char *)transmit_buffer, "%s%d,%d,0\r\n", MQTT_PUB_MSG_RESPONSE, client_idx, msgid);
    if (check_response(PROMPT, 50 * MAX_WAIT_MS) == SUCCESS)
    {
        //				ESP_LOGI(TAG,"Publish msg");
        uart_write_bytes(UART_NUM_1, msg, strlen(msg));
        if (check_response(transmit_buffer, 60 * MAX_WAIT_MS) == SUCCESS)
        {
#if DEBUG_LOG
            ESP_LOGI(TAG, "Published temp alert packet:%s\r\n", msg);
#endif
            free(transmit_buffer);
            return SUCCESS;
        }
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Could not publish temp alert message.\r\n");
#endif
    free(transmit_buffer);
    //			network_fl
    client_flag = 0;
    return FAILURE;
}

#define config_err_pkt "{\"device_id\": \"%s\", \"msg_Id\": \"%s\",\"Sensor_count\":%d}"
uint8_t Publish_Error_Message(uint8_t client_idx, uint8_t msgid, uint8_t qos, uint8_t retain, char *topic, int count_error)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    char *msg = (char *)calloc(BUF_SIZE, sizeof(char));

    //		    sprintf(msg,config_err_pkt , device_id, config_error_msg_id,count_error);
    sprintf(msg, "RS485 maximum sensor limit exceed");
    sprintf((char *)transmit_buffer, "%s%d,%d,%d,%d,\"%s\",%d\r\n", PUB_MSG, client_idx, msgid, qos, retain, topic, strlen(msg));
    sendAT_Data((char *)transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    sprintf((char *)transmit_buffer, "%s%d,%d,0\r\n", MQTT_PUB_MSG_RESPONSE, client_idx, msgid);
    if (check_response(PROMPT, 50 * MAX_WAIT_MS) == SUCCESS)
    {
        //				ESP_LOGI(TAG,"Publish msg");
        uart_write_bytes(UART_NUM_1, msg, strlen(msg));
        if (check_response(transmit_buffer, 60 * MAX_WAIT_MS) == SUCCESS)
        {
#if DEBUG_LOG
            ESP_LOGI(TAG, "Published error message:%s\r\n", msg);
#endif
            free(transmit_buffer);
            return SUCCESS;
        }
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Could not publish error message.\r\n");
#endif
    free(transmit_buffer);
    //			network_fl
    client_flag = 0;
    return FAILURE;
}

uint8_t Publish_RS232_Error_Message(uint8_t client_idx, uint8_t msgid, uint8_t qos, uint8_t retain, char *topic, int count_error)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    char *msg = (char *)calloc(BUF_SIZE, sizeof(char));

    //		    sprintf(msg,config_err_pkt , device_id, config_error_msg_id,count_error);
    sprintf(msg, "RS232 maximum sensor limit exceed");
    sprintf((char *)transmit_buffer, "%s%d,%d,%d,%d,\"%s\",%d\r\n", PUB_MSG, client_idx, msgid, qos, retain, topic, strlen(msg));
    sendAT_Data((char *)transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    sprintf((char *)transmit_buffer, "%s%d,%d,0\r\n", MQTT_PUB_MSG_RESPONSE, client_idx, msgid);
    if (check_response(PROMPT, 50 * MAX_WAIT_MS) == SUCCESS)
    {
        //				ESP_LOGI(TAG,"Publish msg");
        uart_write_bytes(UART_NUM_1, msg, strlen(msg));
        if (check_response(transmit_buffer, 60 * MAX_WAIT_MS) == SUCCESS)
        {
#if DEBUG_LOG
            ESP_LOGI(TAG, "Published error message:%s\r\n", msg);
#endif
            free(transmit_buffer);
            return SUCCESS;
        }
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Could not publish error message.\r\n");
#endif
    free(transmit_buffer);
    //			network_fl
    client_flag = 0;
    return FAILURE;
}

uint8_t Publish_config_reply_message(uint8_t client_idx, uint8_t msgid, uint8_t qos, uint8_t retain, char *topic)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    char *msg = (char *)calloc(BUF_SIZE, sizeof(char));

    //		    sprintf(msg,config_err_pkt , device_id, config_error_msg_id,count_error);
    sprintf(msg, "config packet received");
    sprintf((char *)transmit_buffer, "%s%d,%d,%d,%d,\"%s\",%d\r\n", PUB_MSG, client_idx, msgid, qos, retain, topic, strlen(msg));
    sendAT_Data((char *)transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    sprintf((char *)transmit_buffer, "%s%d,%d,0\r\n", MQTT_PUB_MSG_RESPONSE, client_idx, msgid);
    if (check_response(PROMPT, 50 * MAX_WAIT_MS) == SUCCESS)
    {
        //				ESP_LOGI(TAG,"Publish msg");
        uart_write_bytes(UART_NUM_1, msg, strlen(msg));
        if (check_response(transmit_buffer, 60 * MAX_WAIT_MS) == SUCCESS)
        {
#if DEBUG_LOG
            ESP_LOGI(TAG, "Published config reply packet:%s\r\n", msg);
#endif
            free(transmit_buffer);
            return SUCCESS;
        }
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Could not publish config reply message.\r\n");
#endif
    free(transmit_buffer);
    //			network_fl
    client_flag = 0;
    return FAILURE;
}

uint8_t Publish_mqtt_connection_ckeck(uint8_t client_idx, uint8_t msgid, uint8_t qos, uint8_t retain, char *topic)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    char *msg = (char *)calloc(BUF_SIZE, sizeof(char));

    //		    sprintf(msg,config_err_pkt , device_id, config_error_msg_id,count_error);
    sprintf(msg, "0");
    sprintf((char *)transmit_buffer, "%s%d,%d,%d,%d,\"%s\",%d\r\n", PUB_MSG, client_idx, msgid, qos, retain, topic, strlen(msg));
    sendAT_Data((char *)transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    sprintf((char *)transmit_buffer, "%s%d,%d,0\r\n", MQTT_PUB_MSG_RESPONSE, client_idx, msgid);
    if (check_response(PROMPT, 50 * MAX_WAIT_MS) == SUCCESS)
    {
        //				ESP_LOGI(TAG,"Publish msg");
        uart_write_bytes(UART_NUM_1, msg, strlen(msg));
        if (check_response(transmit_buffer, 60 * MAX_WAIT_MS) == SUCCESS)
        {
#if DEBUG_LOG
            ESP_LOGI(TAG, "Published mqtt check packet:%s\r\n", msg);
#endif
            free(transmit_buffer);
            return SUCCESS;
        }
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Could not publish mqtt check message.\r\n");
#endif
    free(transmit_buffer);
    //			network_fl
    client_flag = 0;
    return FAILURE;
}

#define heart_beat_packet "{\"device_id\": \"%s\", \"msg_Id\": \"%s\",\"Sensor_data\":{\"temp_sensor\": %.2f,\"VOC_lvl\": %.2f,\"humid\": %.2f,\"pressure\": %.2f,\"dust_PM1\": %.2f,\"dust_PM2P5\": %.2f,\"dust_PM10\": %.2f,\"CO_lvl\": %.2f,\"CO2_lvl\": %.2f,\"SO2_lvl\": %.2f,\"NO2_lvl\": %.2f,\"O3_lvl\": %.2f,\"wind_speed\": %.2f,\"wind_dir\": %.2f,\"Atm_press\": %.2f,\"rainfall\":%.2f,\"illuminance\": %.2f,\"solar_rad\": %.2f,\"UV_index\": %.2f,\"lead_acid_batt\": %.2f,\"lipo_batt_vol\": %.2f,\"sound_level_3033\": %.2f,\"sound_level_3036\": %.2f,\"HD2010_sound_lvl\": %.2f,\"WBGT\": %.2f,\"WBGT_hum\": %.2f,\"WBGT_TA\": %.2f,\"WBGT_TG\": %.2f,\"WBGT_WB\": %.2f,\"WBGT_DEW\": %.2f,\"x-axis\": %.2f,\"y-axis\": %.2f,\"z-axis\": %.2f},\"GPS\":{\"lattitude\":\"%s\",\"longitude\":\"%s\",\"satellites\":%d},\"time_stamp\": \"%s\",\"SIM_No\": \"%s\",\"current_version\": \"v%.1f\")}"

char publish_payload[1024];
uint8_t PublishMessage(uint8_t client_idx, uint8_t msgid, uint8_t qos, uint8_t retain, char *topic)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    char *msg = (char *)calloc(BUF_SIZE, sizeof(char));
    //	ESP_LOGI(TAG,"latitude:%s\r\n",latitude);
    //	ESP_LOGI(TAG,"longitude:%s\r\n",longitude);
    //	ESP_LOGI(TAG,"satellites:%d\r\n",numberOfSatellites);
    sprintf(msg, heart_beat_packet, device_id, heart_beat_id, temp_sensor, voc_level, humid, pressure, dust_pm1, dust_pm2p5, dust_pm10, CO_lvl, CO2_lvl, SO2_lvl, NO2_lvl, O3_lvl, wind_speed, wind_dir, atm_pres, rainfall, iluminance, solar_rad, uv_idx, lead_acid_bat, lipo_bat, sound_level_3033, sound_level_3036, HD2010_sound, WBGT_data_01, WBGT_data_02, WBGT_data_03, WBGT_data_04, WBGT_data_05, WBGT_data_06, x_axis, y_axis, z_axis, latitude, longitude, numberOfSatellites, global_time, sim_num, FM_version);
    //		    sprintf(msg,"qsafe string data");

    sprintf((char *)transmit_buffer, "%s%d,%d,%d,%d,\"%s\",%d\r\n", PUB_MSG, client_idx, msgid, qos, retain, topic, strlen(msg));

    sendAT_Data((char *)transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    sprintf((char *)transmit_buffer, "%s%d,%d,0\r\n", MQTT_PUB_MSG_RESPONSE, client_idx, msgid);
    if (check_response(PROMPT, 50 * MAX_WAIT_MS) == SUCCESS)
    {
        //				ESP_LOGI(TAG,"Publish msg");
        uart_write_bytes(UART_NUM_1, msg, strlen(msg));
        if (check_response(transmit_buffer, 60 * MAX_WAIT_MS) == SUCCESS)
        {
#if DEBUG_LOG
            ESP_LOGI(TAG, "Published message:%s\r\n", msg);
#endif
            free(transmit_buffer);
            free(utcTime);
            free(latitude);
            free(longitude);
            free(gps_day);
            //					    free(numberOfSatellites);
            return SUCCESS;
        }
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Could not publish message.\r\n");
#endif
    free(transmit_buffer);
    //			network_fl
    client_flag = 0;
    //			GSM_check_flag=0;
    //			    free(utcTime);
    free(utcTime);
    free(latitude);
    free(longitude);
    free(gps_day);
    //			    free(gps_day);
    return FAILURE;
}

uint8_t ReadMessage(int client_idx)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    sprintf((char *)transmit_buffer, "%s%d\r\n", READ_MSG_BUFFER, client_idx);
    sendAT_Data((char *)transmit_buffer);
    if (check_response(OK_RESPONSE, 10 * MAX_WAIT_MS) == SUCCESS)
    {
        free(transmit_buffer);
        return SUCCESS;
    }
#if DEBUG_LOG
    ESP_LOGI(TAG, "Could not receive message.\r\n");
#endif
    free(transmit_buffer);
    return FAILURE;
}

uint8_t SSL_config(uint8_t ssl_context_index, char *ca_cert, char *client_cert, char *client_key)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));

    sprintf((char *)transmit_buffer, "%s\"UFS:ca.pem\",1464,100,0\r\n", FILE_UPLOAD);
    sendAT_Data(transmit_buffer);
    // Error_Report();
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(CONNECT_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
        uart_write_bytes(UART_NUM_1, ca_cert, strlen(ca_cert));
        if (check_response(FILE_UPLOAD_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
        {
#if DEBUG_LOG
            ESP_LOGI(TAG, "CA cerificate sent");
#endif
        }
    }

    sprintf((char *)transmit_buffer, "%s\"UFS:client.pem\",1269,100,0\r\n", FILE_UPLOAD);
    sendAT_Data(transmit_buffer);
    // Error_Report();
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(CONNECT_RESPONSE, 50 * MAX_WAIT_MS) == SUCCESS)
    {
        uart_write_bytes(UART_NUM_1, client_cert, strlen(client_cert));
        if (check_response(FILE_UPLOAD_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
        {
#if DEBUG_LOG
            ESP_LOGI(TAG, "CC cerificate sent");
#endif
        }
    }

    sprintf((char *)transmit_buffer, "%s\"UFS:client_key.pem\",1679,100,0\r\n", FILE_UPLOAD);
    sendAT_Data(transmit_buffer);
    // Error_Report();
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(CONNECT_RESPONSE, 50 * MAX_WAIT_MS) == SUCCESS)
    {
        uart_write_bytes(UART_NUM_1, client_key, strlen(client_key));
        if (check_response(FILE_UPLOAD_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
        {
#if DEBUG_LOG
            ESP_LOGI(TAG, "CK cerificate sent");
#endif
        }
    }

    sprintf((char *)transmit_buffer, "%s%d,\"UFS:ca.pem\"\r\n", SSL_CONFIG_CACERT, ssl_context_index);
    sendAT_Data(transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "CA certificate configured");
#endif
    }

    sprintf((char *)transmit_buffer, "%s%d,\"UFS:client.pem\"\r\n", SSL_CONFIG_CCCERT, ssl_context_index);
    sendAT_Data(transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "CC certificate configured");
#endif
    }

    sprintf((char *)transmit_buffer, "%s%d,\"UFS:client_key.pem\"\r\n", SSL_CONFIG_CLIKEY, ssl_context_index);
    sendAT_Data(transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "CK certificate configured");
#endif
    }

    sprintf((char *)transmit_buffer, "%s%d,2\r\n", SSL_CONFIG_AUTHMODE, ssl_context_index);
    sendAT_Data(transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Server authentication mode");
#endif
    }

    sprintf((char *)transmit_buffer, "%s%d,4\r\n", SSL_CONFIG_AUTHVER, ssl_context_index);
    sendAT_Data(transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "SSL authentication version");
#endif
    }

    sprintf((char *)transmit_buffer, "%s%d,0xFFFF\r\n", SSL_CONFIG_CIPHERSUITE, ssl_context_index);
    sendAT_Data(transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Cipher suite set");
#endif
    }

    sprintf((char *)transmit_buffer, "%s%d,1\r\n", SSL_CONFIG_AUTHTIME, ssl_context_index);
    sendAT_Data(transmit_buffer);
    memset(transmit_buffer, '\0', strlen(transmit_buffer));
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS) == SUCCESS)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Authentication time ignored");
#endif
    }

    free(transmit_buffer);
    return SUCCESS;
}

uint8_t List_all_files()
{
    sendAT_Data(FILE_LIST);
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS))
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "All files listed");
#endif
        return SUCCESS;
    }
    return FAILURE;
}
uint8_t Delete_file(char *filename)
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    sprintf((char *)transmit_buffer, "%s\"%s\"\r\n", FILE_DELETE, filename);
    sendAT_Data(transmit_buffer);
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS))
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "File Deleted");
#endif
        return SUCCESS;
    }
    return FAILURE;
}

uint8_t Error_Report()
{
    sendAT_Data(ERROR_REPORT);
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS))
    {
        return SUCCESS;
    }
    return FAILURE;
}

uint8_t OT_command(char *cmd)
{
    sendAT_Data(cmd);
    if (check_response(OK_RESPONSE, 3 * MAX_WAIT_MS))
    {
        return SUCCESS;
    }
    return FAILURE;
}
#define GNSS_CONFIGURE_CMD ""
#define GNSS_ON_OFF_CMD "AT+QGPS=1"
#define GNSS_DATA_CMD ""
#define GNSS_LOC_CMD "AT+QGPSLOC=0"

void send_GNSS_CMD()
{
    char *transmit_buffer = (char *)calloc(BUF_SIZE, sizeof(char));
    sprintf((char *)transmit_buffer, "%s\r\n", GNSS_ON_OFF_CMD);
    sendAT_Data(transmit_buffer);
    if (check_response(OK_RESPONSE, 10 * MAX_WAIT_MS))
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "GPS ON");
#endif
    }
    else
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "GPS NOT ON");
#endif
    }

    char *transmit_buffer_01 = (char *)calloc(BUF_SIZE, sizeof(char));
    sprintf((char *)transmit_buffer, "%s\r\n", GNSS_LOC_CMD);
    sendAT_Data(transmit_buffer);
    if (check_response(OK_RESPONSE, 10 * MAX_WAIT_MS))
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "GPS LOCATION get");
#endif
    }
    else
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "GPS location not recived");
#endif
    }
}
int watch_dog_counter = 0;
int interval_of_heartbeat = 0;

static void timer_callback(void *arg)
{
    interval_of_heartbeat++;
    calibration_flag++;
    //	ESP_LOGI(TAG,"timer inc %d",interval_of_heartbeat);
    watch_dog_counter++;
    if (watch_dog_counter >= 30)
    {
        watch_dog_counter = 0;
        gpio_set_level(MCU_WDI_PIN, 1);
        vTaskDelay(2);
        gpio_set_level(MCU_WDI_PIN, 0);
    }
}

void timer_config()
{
    const esp_timer_create_args_t timer_args = {
        .callback = &timer_callback,
        .name = "timer"};

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 500000));
}

void init_LTE()
{
    //	      uart_init();

    OT_command("ATE0\r\n");
    MQTT_Config(CLIENT_IDX,
                0, 2,
                3000,
                1,
                0, 1,
                1, 0, 0, "will/topic", "Network Disconnected unexpectedly");
    //	       SSL_config(2, ca_cert, client_cert, client_key);
    //	       send_GNSS_CMD();
}

void check_gsm_connection()
{
    if (network_flag == 0)
    {
        ESP_LOGI(TAG, "Network connection");
        int GSM_iniit_flag = MQTT_NetworkOpen(CLIENT_IDX, "54.215.188.103", 1883);
        if (GSM_iniit_flag == 2)
        {
            MQTT_NetworkClose(CLIENT_IDX);
        }
        if (!GSM_iniit_flag)
        {
            Gsm_Power_Off();
            vTaskDelay(1300);
            mpu9250_register_write_byte(0x02, BIT0); // Wakeup pin
            vTaskDelay(100);
            init_LTE();
        }
    }
    if (client_flag == 0)
    {
        ESP_LOGI(TAG, "client connection");

        MQTT_ClientConnect(CLIENT_IDX, "QmaxSystems", "Qmax_mosquitto_!@#", NODE_NAME);
        SubscribeTopic(CLIENT_IDX, 2, SUBS_TOPIC, 0);
    }
    else
    {
        //				ESP_LOGI(TAG,"timer intervel_READ_REC %d ", interval_of_heartbeat);

        if (client_flag)
        {
            ReadMessage(CLIENT_IDX);
        }
        //				ESP_LOGI(TAG,"timer intervel_READ_REC %d ", interval_of_heartbeat);
    }
}

void Gsm_Power_Off()
{
    int i2c_er_cd = mpu9250_register_write_byte(0x03, 0x00);
    //	    	   ESP_LOGI(TAG, "i2c write err %d ", i2c_er_cd);
    vTaskDelay(50);
    /*    exp1_port_status_01 |= (BIT6);
        int i2c_er_cd=mpu9250_register_write_byte(0x03,exp1_port_status_01);
        ESP_LOGI(TAG, "i2c write err %d ", i2c_er_cd);
        vTaskDelay(50);
        exp1_port_status_01 &= ~(BIT6);
        i2c_er_cd=mpu9250_register_write_byte(0x03,exp1_port_status_01);
        ESP_LOGI(TAG, "i2c write err %d ", i2c_er_cd);
        vTaskDelay(100);*/
    i2c_er_cd = mpu9250_register_write_byte(0x03, BIT6);
    //	    	   ESP_LOGI(TAG, "i2c write err %d ", i2c_er_cd);
    vTaskDelay(50);
    i2c_er_cd = mpu9250_register_write_byte(0x03, 0x00);
    //	    	   ESP_LOGI(TAG, "i2c write err %d ", i2c_er_cd);
    vTaskDelay(50);
    //	    	   vTaskDelay(70);
    //	    	   i2c_er_cd=mpu9250_register_write_byte(0x03,0x00);
    //	    	   ESP_LOGI(TAG, "i2c write err %d ", i2c_er_cd);
    //	    	   vTaskDelay(pdMS_TO_TICKS(1000));
    //	    	   i2c_er_cd=mpu9250_register_write_byte(0x03,0x00);
    //	    	   ESP_LOGI(TAG, "i2c write err %d ", i2c_er_cd);

    //	    	   i2c_er_cd=mpu9250_register_write_byte(0x03,0x40);
    //	    	   ESP_LOGI(TAG, "i2c write err %d ", i2c_er_cd);

    //    set_exp2_port0_gpio_state(MC60_POWERKEY_BIT, HIGH); //Set powerkey low
    //    delay(10);
    //    set_exp2_port0_gpio_state(MC60_POWERKEY_BIT, LOW); //Set powerkey low
    //    delay(800);
    //    set_exp2_port0_gpio_state(MC60_POWERKEY_BIT, HIGH); //Set powerkey low
}

void configure_expander_pin_direction(void)
{
    // Expander1 GPIOs Direction setting
    vTaskDelay(10);
    exp1_port_dir_00 |= (STATUS_LTE | TEMP_ALERT_BIT);
    mpu9250_register_write_byte(0x06, exp1_port_dir_00); // Configure Expander1 Port0 pins as input
    vTaskDelay(10);
    exp1_port_dir_01 |= MCU_NET_MODE;
    mpu9250_register_write_byte(0x07, exp1_port_dir_01); // Configure Expander1 Port0 pins as input
    vTaskDelay(10);
    exp1_port_dir_00 &= ~(MCU_WAKEUP | SET1_WDT | BUZZER_BIT | RELAY_CTRL_BIT);
    mpu9250_register_write_byte(0x06, exp1_port_dir_00); // Configure Expander2 Port1 pins as output
    vTaskDelay(10);
    exp1_port_dir_01 &= ~(LA_PROT_CTL | MCU_RESET | UART_DTR | MCU_ON_OFF | REG_CTRL_5V);
    mpu9250_register_write_byte(0x07, exp1_port_dir_01); // Configure Expander2 Port1 pins as output
    vTaskDelay(10);
}
void set_expander_default_gpio_state(void)
{
    exp1_port_status_00 &= ~(MCU_WAKEUP | BUZZER_BIT);
    mpu9250_register_write_byte(0x02, exp1_port_status_00); // Configure Expander2 Port1 pins as output
    vTaskDelay(10);
    exp1_port_status_01 &= ~(MCU_RESET | UART_DTR | MCU_ON_OFF | LA_PROT_CTL | REG_CTRL_5V);
    mpu9250_register_write_byte(0x03, exp1_port_status_01); // Configure Expander2 Port1 pins as output
    vTaskDelay(10);
}

void read_temp_sensor()
{
    uint8_t Temp_data[3];
    int err_code = temp_register_read(0x00, Temp_data, 2);
#if DEBUG_LOG
    ESP_LOGI(TAG, "port0 read status %x ", Temp_data[0]);
#endif
#if DEBUG_LOG
    ESP_LOGI(TAG, "port0 read status %x ", Temp_data[1]);
#endif
    temp_sensor = Temp_data[0];
    //	    	  ESP_LOGI(TAG, "port0 read status %x ", Temp_data[2]);
}

static void echo_send(const int port, const char *str, uint8_t length)
{
    //	gpio_set_level(UART_DIR_RS485, 1);
    if (uart_write_bytes(port, str, length) != length)
    {
#if DEBUG_LOG
        ESP_LOGE(TAG, "Send data critical failure.");
#endif
        // add your code to handle sending failure here
        abort();
    }
    //    vTaskDelay(10);
}

void check_WDI_Reset()
{
}
void UART_RS232_Initialization(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(0, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(0, &uart_config);
    uart_set_pin(0, RS232_TXD_PIN, RS232_RXD_PIN, RS232_CTS_PIN, RS232_RTS_PIN);
    gpio_set_direction(45, GPIO_MODE_OUTPUT);
    gpio_set_level(45, 0);
}

// An example of echo test with hardware flow control on UART
static void UART_Initialization_of_RS485()
{
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));
    gpio_set_direction(3, GPIO_MODE_OUTPUT);
    gpio_set_level(3, 0);
    //	    vTaskDelay(200);
}

void read_RS232_sensor_data_SL_4033()
{
    char *data = (char *)malloc(BUF_SIZE + 1);
    //		sendData("STT:ACQUISITION:?\r\n");
    //		uart_write_bytes(0, "STT:ACQUISITION:?\r\n", 19);
    //	vTaskDelay(1);
    //	ESP_LOGI(TAG, "3333333333333333");
    int data_rs232_len = uart_read_bytes(0, data, 16, 1);
    //
    vTaskDelay(1);
    if (data_rs232_len > 0)
    {
//		 sound_level_data_01=data[1];
//		 sound_level_data_02=data[2];
//		 sound_level_data_03=data[3];
#if DEBUG_LOG
        for (int i = 0; i < data_rs232_len; i++)
        {
            ESP_LOGI(TAG, "RS232 data is [%d] =%c", i, data[i]);
        }
#endif
        sprintf(sound_level_data_01, "%c%c%c%c%c%c%c%c", data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14]);
        //		ESP_LOGI(TAG,"rs232 data check :%s",sound_level_data_01);
        sound_level_3033 = atoi(sound_level_data_01);
        sound_level_3033 = sound_level_3033 / 10;
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                sound_level_3033 += sound_level_3033_buf[average_flag];
            }
            sound_level_3033 = sound_level_3033 / average_flag;
            ESP_LOGI(TAG, "avg sound_level_3033=%f DB", sound_level_3033);
        }
        free(data);
    }
    else
    {
        ESP_LOGI(TAG, "NULL");
        free(data);
    }
}

void read_RS232_sensor_data_SL_4036()
{
    //	uint8_t* data = (uint8_t*) malloc(BUF_SIZE+1);
    char *data = (char *)malloc(BUF_SIZE + 1);
    //	    ESP_LOGI(TAG, "6666666666666");
    int data_rs232_len = uart_read_bytes(0, data, 16, 1);
    //
    vTaskDelay(1);
    if (data_rs232_len > 0)
    {

#if DEBUG_LOG
        for (int i = 0; i < data_rs232_len; i++)
        {
            ESP_LOGI(TAG, "RS232 data is [%d] =%c", i, data[i]);
        }
#endif

        sprintf(sound_level_data_02, "%c%c%c%c%c%c%c%c", data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14]);
        //		 ESP_LOGI(TAG,"rs232 data check :%s",sound_level_data_02);
        sound_level_3036 = atoi(sound_level_data_02);
        sound_level_3036 = sound_level_3036 / 10;
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                sound_level_3036 += sound_level_3036_buf[average_flag];
            }
            sound_level_3036 = sound_level_3036 / average_flag;
            ESP_LOGI(TAG, "avg sound_level_3036=%f DB", sound_level_3036);
        }
        free(data);
    }
    else
    {
        ESP_LOGI(TAG, "NULL");
        free(data);
    }
}

void read_RS232_sensor_data_HD2010UC()
{
    char *data = (char *)malloc(BUF_SIZE + 1);
    uart_write_bytes(0, "KEY:PRINT\r\n", 11);
    vTaskDelay(1);
    int data_rs232_len = uart_read_bytes(0, data, 1024, 100);
    if (data_rs232_len > 0)
    {

        vTaskDelay(1);

        //		 ESP_LOGI(TAG, "RS232 data is %s",data);
        vTaskDelay(1);
        char *leqStart = strstr(data, "Leq");
        if (leqStart != NULL)
        {
            // Move the pointer to the Leq value
            leqStart += strlen("Leq");
            while (*leqStart == ' ' || *leqStart == '=' || *leqStart == '\t')
            {
                leqStart++;
            }

            // Extract the Leq value
            double leqValue;
            sscanf(leqStart, "%lf", &leqValue);
            HD2010_sound = leqValue;
            //		         ESP_LOGI(TAG,"Extracted Leq value: %.1f\n", leqValue);
        }
        else
        {
            //		    	 ESP_LOGI(TAG,"Leq data not found in the RS232 data.\n");
        }
        free(data);
    }
    else
    {
        free(data);
    }
}
void read_RS232_sensor_data_WBGT()
{
    char *data = (char *)malloc(BUF_SIZE + 1);
    int data_rs232_len = uart_read_bytes(0, data, 16, 1);
    vTaskDelay(1);
    if (data_rs232_len > 0)
    {

#if DEBUG_LOG
        for (int i = 0; i < data_rs232_len; i++)
        {
            ESP_LOGI(TAG, "RS232 data is [%d] =%c", i, data[i]);
        }
#endif

        if (data[2] == '1')
        {
            //			 ESP_LOGI(TAG,"11111111111111111111");
            sprintf(WBGT_param_01, "%c%c%c%c%c%c%c%c", data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14]);
            //			  ESP_LOGI(TAG,"rs232 data check :%s",WBGT_param_01);
            WBGT_data_01 = atoi(WBGT_param_01);
            WBGT_data_01 = WBGT_data_01 / 10;
            if (average_flag >= 10)
            {
                for (int i = 0; i < average_flag; i++)
                {
                    WBGT_data_01 += WBGT_data_01_buf[average_flag];
                }
                WBGT_data_01 = WBGT_data_01 / average_flag;
                ESP_LOGI(TAG, "avg WBGT_data_01=%f ", WBGT_data_01);
            }
            //		 ESP_LOGI(TAG,"rs232 data check :%f",WBGT_data_01);
        }
        if (data[2] == '2')
        {
            //		 			 ESP_LOGI(TAG,"2222222222222222");
            sprintf(WBGT_param_02, "%c%c%c%c%c%c%c%c", data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14]);
            //		 			 ESP_LOGI(TAG,"rs232 data check :%s",WBGT_param_02);
            WBGT_data_02 = atoi(WBGT_param_02);
            WBGT_data_02 = WBGT_data_02 / 10;
            if (average_flag >= 10)
            {
                for (int i = 0; i < average_flag; i++)
                {
                    WBGT_data_02 += WBGT_data_02_buf[average_flag];
                }
                WBGT_data_02 = WBGT_data_02 / average_flag;
                ESP_LOGI(TAG, "avg WBGT_data_02=%f ", WBGT_data_02);
            }
        }
        if (data[2] == '3')
        {
            //		 			 ESP_LOGI(TAG,"33333333333333333");
            sprintf(WBGT_param_03, "%c%c%c%c%c%c%c%c", data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14]);
            //		 			 ESP_LOGI(TAG,"rs232 data check :%s",WBGT_param_03);
            WBGT_data_03 = atoi(WBGT_param_03);
            WBGT_data_03 = WBGT_data_03 / 10;
            if (average_flag >= 10)
            {
                for (int i = 0; i < average_flag; i++)
                {
                    WBGT_data_03 += WBGT_data_03_buf[average_flag];
                }
                WBGT_data_03 = WBGT_data_03 / average_flag;
                ESP_LOGI(TAG, "avg WBGT_data_03=%f ", WBGT_data_03);
            }
        }
        if (data[2] == '4')
        {
            //		 			 ESP_LOGI(TAG,"44444444444444444");
            sprintf(WBGT_param_04, "%c%c%c%c%c%c%c%c", data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14]);
            //		 			 ESP_LOGI(TAG,"rs232 data check :%s",WBGT_param_04);
            WBGT_data_04 = atoi(WBGT_param_04);
            WBGT_data_04 = WBGT_data_04 / 10;
            if (average_flag >= 10)
            {
                for (int i = 0; i < average_flag; i++)
                {
                    WBGT_data_04 += WBGT_data_04_buf[average_flag];
                }
                WBGT_data_04 = WBGT_data_04 / average_flag;
                ESP_LOGI(TAG, "avg WBGT_data_04=%f ", WBGT_data_04);
            }
        }
        if (data[2] == '5')
        {
            //		 			 ESP_LOGI(TAG,"55555555555555555");
            sprintf(WBGT_param_05, "%c%c%c%c%c%c%c%c", data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14]);
            //		 			 ESP_LOGI(TAG,"rs232 data check :%s",WBGT_param_05);
            WBGT_data_05 = atoi(WBGT_param_05);
            WBGT_data_05 = WBGT_data_05 / 10;
            if (average_flag >= 10)
            {
                for (int i = 0; i < average_flag; i++)
                {
                    WBGT_data_05 += WBGT_data_05_buf[average_flag];
                }
                WBGT_data_05 = WBGT_data_05 / average_flag;
                ESP_LOGI(TAG, "avg WBGT_data_05=%f ", WBGT_data_05);
            }
        }
        if (data[2] == '6')
        {
            //		 			 ESP_LOGI(TAG,"66666666666666666");
            sprintf(WBGT_param_06, "%c%c%c%c%c%c%c%c", data[7], data[8], data[9], data[10], data[11], data[12], data[13], data[14]);
            //		 			ESP_LOGI(TAG,"rs232 data check :%s",WBGT_param_06);
            WBGT_data_06 = atoi(WBGT_param_06);
            WBGT_data_06 = WBGT_data_06 / 10;
            if (average_flag >= 10)
            {
                for (int i = 0; i < average_flag; i++)
                {
                    WBGT_data_06 += WBGT_data_06_buf[average_flag];
                }
                WBGT_data_06 = WBGT_data_06 / average_flag;
                ESP_LOGI(TAG, "avg WBGT_data_06=%f DB", WBGT_data_06);
            }
        }

        free(data);
    }
    else
    {
        ESP_LOGI(TAG, "NULL");
        free(data);
    }
}

int modbus_reg_addr = 0;

void calibration_of_RS485_sensor_data(uint8_t *rec_buf, int buf_len)
{
    // #if DEBUG_LOG
    //	ESP_LOGI(TAG, "====================================================================");
    // #endif
    //	ESP_LOGI(TAG,"SO2_conc_sr_state=%d",SO2_conc_sr_state);
    //	ESP_LOGI(TAG,"So2 level=%f",SO2_lvl);

    for (int i = 0; i < buf_len; i++)
    {
        // #if DEBUG_LOG
        //		ESP_LOGI(TAG, "Received data[%d] =%x", i,rec_buf[i]);
        // #endif
    }
    switch (rec_buf[1])
    {
    case 10:
        //		ESP_LOGI(TAG,"The hex value is 10.\n");
        break;
    case 20:
        //		ESP_LOGI(TAG,"The hex value is 20.\n");
        break;
    case 30:
        //		ESP_LOGI(TAG,"The hex value is 30.\n");
        break;
    case 40:
        //		ESP_LOGI(TAG,"The hex value is 40.\n");
        break;
    case 50:
        //		ESP_LOGI(TAG,"The hex value is 50.\n");
        break;
    case 60:
        //		ESP_LOGI(TAG,"The hex value is 60.\n");
        break;
    case 70:
        //		ESP_LOGI(TAG,"The hex value is 70.\n");
        break;
    case 80:
        //		ESP_LOGI(TAG,"The hex value is 80.\n");
        break;
    case 90:
        //		ESP_LOGI(TAG,"The hex value is 90.\n");
        break;
    case 100:
        //		ESP_LOGI(TAG,"The hex value is 100.\n");
        break;
    case 110:
        //		ESP_LOGI(TAG,"The hex value is 110.\n");
        break;
    case 120:
        //		ESP_LOGI(TAG,"The hex value is 120.\n");
        break;
    case 130:
        //		ESP_LOGI(TAG,"The hex value is 130.\n");
        break;
    case 140:
        //		ESP_LOGI(TAG,"The hex value is 140.\n");
        break;
    case 150:
        //		ESP_LOGI(TAG,"The hex value is 150.\n");
        break;
    case 160:
        //		ESP_LOGI(TAG,"The hex value is 160.\n");
        break;
    }

    if (rec_buf[1] == 0x0A)
    {
        wind_speed = 0;
#if DEBUG_LOG
        ESP_LOGI(TAG, "1-Wind Speed sensor-RK100-01");
#endif
        uint8_t wind_speed_MSB = rec_buf[4];
        uint8_t wind_speed_LSB = rec_buf[5];
        uint16_t raw_wind_speed = ((wind_speed_MSB << 8) | (wind_speed_LSB));
        wind_speed = (float)raw_wind_speed;
        wind_speed = (wind_speed / 10);
        wind_speed_buf[average_flag] = wind_speed;
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                wind_speed += wind_speed_buf[average_flag];
            }
            wind_speed = wind_speed / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "avg wind speed=%f m/s", wind_speed);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "wind speed=%f m/s", wind_speed);
#endif

        if (wind_sen_state)
        {
        }
        else
        {
            wind_speed = 0;
        }
    }

    if (rec_buf[1] == 0x14)
    {
        dust_pm1 = 0;
        dust_pm2p5 = 0;
        dust_pm10 = 0;
#if DEBUG_LOG
        ESP_LOGI(TAG, "2-Dust concentration-RK300-02");
#endif
        uint8_t dust_PM1_MSB = rec_buf[4];
        uint8_t dust_PM1_LSB = rec_buf[5];
        uint8_t dust_PM2P5_MSB = rec_buf[6];
        uint8_t dust_PM2P5_LSB = rec_buf[7];
        uint8_t dust_PM10_MSB = rec_buf[8];
        uint8_t dust_PM10_LSB = rec_buf[9];
        uint16_t raw_dust_pm1 = ((dust_PM1_MSB << 8) | (dust_PM1_LSB));
        uint16_t raw_dust_pm2p5 = ((dust_PM2P5_MSB << 8) | (dust_PM2P5_LSB));
        uint16_t raw_dust_pm10 = ((dust_PM10_MSB << 8) | (dust_PM10_LSB));
        dust_pm1 = (float)raw_dust_pm1;
        dust_pm2p5 = (float)raw_dust_pm2p5;
        dust_pm10 = (float)raw_dust_pm10;
        dust_pm1_buf[average_flag] = dust_pm1;
        dust_pm2p5_buf[average_flag] = dust_pm2p5;
        dust_pm10_buf[average_flag] = dust_pm10;
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                dust_pm1 += dust_pm1_buf[average_flag];
                dust_pm2p5 += dust_pm2p5_buf[average_flag];
                dust_pm10 += dust_pm10_buf[average_flag];
            }
            dust_pm1 = dust_pm1 / average_flag;
            dust_pm2p5 = dust_pm2p5 / average_flag;
            dust_pm10 = dust_pm10 / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "AvgPM1 of  dust concen/.=%f ug/m3", dust_pm1);
            ESP_LOGI(TAG, "AvgPM2.5 of  dust concen/.=%f ug/m3", dust_pm2p5);
            ESP_LOGI(TAG, "AvgPM10 of  dust concen/.=%f ug/m3", dust_pm10);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "PM1 of  dust concen/.=%f ug/m3", dust_pm1);
        ESP_LOGI(TAG, "PM2.5 of  dust concen/.=%f ug/m3", dust_pm2p5);
        ESP_LOGI(TAG, "PM10 of  dust concen/.=%f ug/m3", dust_pm10);
#endif

        if (dust_conc_sen_state)
        {
        }
        else
        {
            dust_pm1 = 0;
            dust_pm2p5 = 0;
            dust_pm10 = 0;
        }
    }

    if (rec_buf[1] == 0x1E)
    {
        CO2_lvl = 0;
#if DEBUG_LOG
        ESP_LOGI(TAG, "3-CO2 concentration-RK300-03");
#endif
        uint8_t CO2_lvl_MSB = rec_buf[4];
        uint8_t CO2_lvl_LSB = rec_buf[5];
        uint16_t raw_co2_lvl = ((CO2_lvl_MSB << 8) | (CO2_lvl_LSB));
        CO2_lvl = (float)raw_co2_lvl;
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                CO2_lvl += CO2_lvl_buf[average_flag];
            }
            CO2_lvl = CO2_lvl / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "avg CO2_lvl=%f ppm", CO2_lvl);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "CO2 level =%f ppm", CO2_lvl);
#endif

        if (CO2_sen_state)
        {
        }
        else
        {
            CO2_lvl = 0;
        }
    }

    if (rec_buf[1] == 0x28)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "4-Air quality");
#endif
    }

    if (rec_buf[1] == 0x32)
    {
        voc_level = 0;
#if DEBUG_LOG
        ESP_LOGI(TAG, "5-TVOC gas detection-RK300-13");
#endif
        uint8_t Voc_MSB = rec_buf[4];
        uint8_t Voc_LSB = rec_buf[5];
        uint16_t raw_Voc_lvl = ((Voc_MSB << 8) | (Voc_LSB));
        voc_level = (float)raw_Voc_lvl;
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                voc_level += voc_level_buf[average_flag];
            }
            voc_level = voc_level / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "avg voc_level=%fppm", voc_level);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "Voc gas detection =%f ppm", voc_level);
#endif

        if (VOC_sr_state)
        {
        }
        else
        {
            voc_level = 0;
        }
    }

    if (rec_buf[1] == 0x3C)
    {
        formaldehyde = 0;
#if DEBUG_LOG
        ESP_LOGI(TAG, "6-Formaldehyde-RK300-24");
#endif
        uint8_t formaldehyd_MSB = rec_buf[4];
        uint8_t formaldehyd_LSB = rec_buf[5];
        uint16_t raw_formaldehyd = ((formaldehyd_MSB << 8) | (formaldehyd_LSB));
        formaldehyde = (float)raw_formaldehyd;
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                formaldehyde += formaldehyde_buf[average_flag];
            }
            formaldehyde = formaldehyde / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "avg formaldehyde=%f ppb", formaldehyde);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "formaldehyde =%f ppb", formaldehyde);
#endif

        if (formaldehyde_sr_state)
        {
        }
        else
        {
            formaldehyde = 0;
        }
    }

    if (rec_buf[1] == 0x46)
    {
        uv_idx = 0;
#if DEBUG_LOG
        ESP_LOGI(TAG, "7-UV radiation-RK200-07");
#endif
        uint8_t uv_rad_MSB = rec_buf[4];
        uint8_t uv_rad_LSB = rec_buf[5];
        uint16_t raw_uv_rad = ((uv_rad_MSB << 8) | (uv_rad_LSB));
        uv_idx = (float)raw_uv_rad;
        uv_idx = uv_idx / 10;
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                uv_idx += uv_idx_buf[average_flag];
            }
            uv_idx = uv_idx / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "avg uv_idxe=%fW/m2", uv_idx);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "uv radiation =%f W/m2", uv_idx);
#endif
        if (UV_rad_sr_state)
        {
        }
        else
        {
            uv_idx = 0;
        }
    }

    if (rec_buf[1] == 0x50)
    {
        iluminance = 0;
#if DEBUG_LOG
        ESP_LOGI(TAG, "8-Solar illuminance-RK210-01");
#endif
        uint8_t illum_MSB = rec_buf[4];
        uint8_t illum_LSB = rec_buf[5];
        uint16_t raw_illum = ((illum_MSB << 8) | (illum_LSB));
        iluminance = (float)raw_illum;
        iluminance = iluminance * 10;
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                iluminance += iluminance_buf[average_flag];
            }
            iluminance = iluminance / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "avg iluminance=%f lux", iluminance);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "illuminance =%f lux", iluminance);
#endif

        if (solar_illum_sr_state)
        {
        }
        else
        {
            iluminance = 0;
        }
    }

    if (rec_buf[1] == 0x5A)
    {
        wind_dir = 0;
#if DEBUG_LOG
        ESP_LOGI(TAG, "9.Wind direction-RK110-02");
#endif
        uint8_t wind_dir_MSB = rec_buf[4];
        uint8_t wind_dir_LSB = rec_buf[5];
        uint16_t raw_wind_dir = ((wind_dir_MSB << 8) | (wind_dir_LSB));
        wind_dir = (float)raw_wind_dir;
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                wind_dir += wind_dir_buf[average_flag];
            }
            wind_dir = wind_dir / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "avg wind_dir=%f degree", wind_dir);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "wind direction =%f degree", wind_dir);
#endif

        if (wind_dir_sr_state)
        {
        }
        else
        {
            wind_dir = 0;
        }
    }

    if (rec_buf[1] == 0x64)
    {
        solar_rad = 0;
#if DEBUG_LOG
        ESP_LOGI(TAG, "10-Solar radiation-RK200-04");
#endif
        //			for(int j=0;j<6;j++) {
        //				ESP_LOGI(TAG, "=============received_solar_data[%d]:%x",j,rec_buf[j]);
        //			}
        uint8_t solar_rad_MSB = rec_buf[4];
        uint8_t solar_rad_LSB = rec_buf[5];
        uint16_t raw_solar_rad = ((solar_rad_MSB << 8) | (solar_rad_LSB));
        solar_rad = (float)raw_solar_rad;
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                solar_rad += solar_rad_buf[average_flag];
            }
            solar_rad = solar_rad / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "avg solar_rad=%f W/m2", solar_rad);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "solar radiation =%f W/m2", solar_rad);
#endif

        if (solar_rad_sr_state)
        {
        }
        else
        {
            solar_rad = 0;
        }
    }

    if (rec_buf[1] == 0x78)
    {
        atm_pres = 0;
        humid = 0;
        pressure = 0;
#if DEBUG_LOG
        ESP_LOGI(TAG, "12-ATM press Humid Pressure-RK330-01");
#endif
        uint8_t Atm_pres_MSB = rec_buf[4];
        uint8_t Atm_pres_LSB = rec_buf[5];
        uint8_t humid_MSB = rec_buf[6];
        uint8_t humid_LSB = rec_buf[7];
        uint8_t pres_MSB = rec_buf[8];
        uint8_t pres_LSB = rec_buf[9];
        uint16_t raw_atm_pres = ((uint16_t)(Atm_pres_MSB << 8) | (Atm_pres_LSB));
        uint16_t raw_humid = ((uint16_t)(humid_MSB << 8) | (humid_LSB));
        uint16_t raw_pres = ((uint16_t)(pres_MSB << 8) | (pres_LSB));
        atm_pres = (double)raw_atm_pres;
        atm_pres = atm_pres / 10;
        humid = (double)raw_humid;
        humid = humid / 10;
        pressure = (double)raw_pres;
        pressure = pressure / 10;
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                atm_pres += atm_pres_buf[average_flag];
                humid += humid_buf[average_flag];
                pressure += pressure_buf[average_flag];
            }
            atm_pres = atm_pres / average_flag;
            humid = humid / average_flag;
            pressure = pressure / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "Avg Atmospheric pressure=%f degree", atm_pres);
            ESP_LOGI(TAG, "Avg humidity=%f percentage", humid);
            ESP_LOGI(TAG, "Avg pressure=%f mbar", pressure);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "Atmospheric pressure=%f degree", atm_pres);
        ESP_LOGI(TAG, "humidity=%f percentage", humid);
        ESP_LOGI(TAG, "pressure=%f mbar", pressure);
#endif

        if (Atm_hum_pres_sr_state)
        {
        }
        else
        {
            atm_pres = 0;
            humid = 0;
            pressure = 0;
        }
    }

    if (rec_buf[1] == 0x6E)
    {
        SO2_lvl = 0;

#if DEBUG_LOG
        ESP_LOGI(TAG, "11-SO2 concentration-RK300-09");
#endif

        uint8_t SO2_lvl_MSB = rec_buf[4];
        uint8_t SO2_lvl_LSB = rec_buf[5];
        uint16_t raw_SO2_lvl = ((SO2_lvl_MSB << 8) | (SO2_lvl_LSB));
        //	 uint32_t raw_SO2_lvl = ((SO2_lvl_byte1<<24)|(SO2_lvl_byte2<<16)|(SO2_lvl_byte3<<8)|(SO2_lvl_byte4));
        SO2_lvl = (double)raw_SO2_lvl;
        SO2_lvl = (SO2_lvl / 1000);
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                SO2_lvl += SO2_lvl_buf[average_flag];
            }
            SO2_lvl = SO2_lvl / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "avg SO2_lvl=%f ppm", SO2_lvl);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "SO2 concentration =%f ppm", SO2_lvl);
#endif

        if (SO2_conc_sr_state)
        {
        }
        else
        {
            SO2_lvl = 0;
        }
    }

    if (rec_buf[1] == 0x82)
    {
        NO2_lvl = 0;
#if DEBUG_LOG
        ESP_LOGI(TAG, "13-NO2 concentration-RK300-14");
#endif
        uint8_t NO2_lvl_byte1 = rec_buf[4];
        uint8_t NO2_lvl_byte2 = rec_buf[5];
        uint8_t NO2_lvl_byte3 = rec_buf[6];
        uint8_t NO2_lvl_byte4 = rec_buf[7];
        uint32_t raw_NO2_lvl = ((NO2_lvl_byte1 << 24) | (NO2_lvl_byte2 << 16) | (NO2_lvl_byte3 << 8) | (NO2_lvl_byte4));
        NO2_lvl = (double)raw_NO2_lvl;
        NO2_lvl = (NO2_lvl / 1000);
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                NO2_lvl += NO2_lvl_buf[average_flag];
            }
            NO2_lvl = NO2_lvl / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "avg NO2_lvl=%f ppm", NO2_lvl);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "NO2 concentration =%f ppm", NO2_lvl);
#endif
        if (NO2_conc_sr_state)
        {
        }
        else
        {
            NO2_lvl = 0;
        }
    }

    if (rec_buf[1] == 0x8C)
    {
        CO_lvl = 0;
#if DEBUG_LOG
        ESP_LOGI(TAG, "14-CO concentration-RK300-11");
#endif
        uint8_t CO_lvl_MSB = rec_buf[4];
        uint8_t CO_lvl_LSB = rec_buf[5];
        uint16_t raw_CO_lvl = ((CO_lvl_MSB << 8) | (CO_lvl_LSB));
        CO_lvl = (float)raw_CO_lvl;
        CO_lvl = (CO_lvl / 10);
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                CO_lvl += CO_lvl_buf[average_flag];
            }
            CO_lvl = CO_lvl / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "avg CO_lvl=%f ppm", CO_lvl);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "CO concentration =%f ppm", CO_lvl);
#endif
        if (CO_conc_sr_state)
        {
        }
        else
        {
            CO_lvl = 0;
        }
    }

    if (rec_buf[1] == 0x96)
    {
        O3_lvl = 0;
#if DEBUG_LOG
        ESP_LOGI(TAG, "15-O3 concentration-RK300-18");
#endif
        uint8_t O3_lvl_MSB = rec_buf[4];
        uint8_t O3_lvl_LSB = rec_buf[5];
        uint16_t raw_o3_level = ((O3_lvl_MSB << 8) | (O3_lvl_LSB));
        O3_lvl = (float)raw_o3_level;
        O3_lvl = O3_lvl / 10;
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                O3_lvl += O3_lvl_buf[average_flag];
            }
            O3_lvl = O3_lvl / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "avg O3_lvl=%f ppm", O3_lvl);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "O3 concentration =%f ppm", O3_lvl);
#endif
        if (O3_conc_sr_state)
        {
        }
        else
        {
            O3_lvl = 0;
        }
    }

    if (rec_buf[1] == 0xA0)
    {
        rainfall = 0;
#if DEBUG_LOG
        ESP_LOGI(TAG, "16-rain sensor");
#endif
        uint8_t rain_idx_MSB = rec_buf[4];
        uint8_t rain_idx_LSB = rec_buf[5];
        uint16_t raw_rain_idx = ((rain_idx_MSB << 8) | (rain_idx_LSB));
        rainfall = (float)raw_rain_idx;
        rainfall = rainfall / 10;
        if (average_flag >= 10)
        {
            for (int i = 0; i < average_flag; i++)
            {
                rainfall += rainfall_buf[average_flag];
            }
            rainfall = rainfall / average_flag;
#if DEBUG_LOG
            ESP_LOGI(TAG, "avg rainfall=%f ppm", rainfall);
#endif
        }
#if DEBUG_LOG
        ESP_LOGI(TAG, "rainfall index =%f mm", rainfall);
#endif
        if (rain_sr_state)
        {
        }
        else
        {
            rainfall = 0;
        }
    }
}

uint8_t Bit_test(uint32_t var, uint8_t var_bit)
{
    if ((var & ((uint32_t)1 << var_bit)))
        return 1;
    return 0;
}

uint16_t CalcCRC(char *Logbuffer, uint8_t MsgLen)
{
    uint16_t CRC;
    uint8_t i, j, *Ptr8, Val, CRCLSB;

    i = 0;
    CRC = 0xffff;
    Ptr8 = (uint8_t *)Logbuffer; // Cast to uint8_t pointer
    do
    {
        Val = *Ptr8;
        CRC = CRC ^ Val;
        Ptr8++;
        j = 0;
        do
        {
            if (Bit_test(CRC, 0))
                CRCLSB = 1;
            else
                CRCLSB = 0;
            CRC >>= 1; // Shift one bit to the right
            if (CRCLSB)
                CRC = CRC ^ 0xa001;
        } while (++j < 8);
        i++;
    } while (--MsgLen); // For all message bytes
    return CRC;
}

char *pass_string;

void convertToHexString(uint8_t *array, size_t length, char *hexString)
{
    size_t index = 0;
    for (size_t i = 0; i < length; i++)
    {
        index += sprintf(&hexString[index], "\\x%02X", array[i]);
    }
}

void read_RS485_sensor_data(int slave_id, int data_len, uint16_t reg_addr, int func_address)
{
    uint8_t data[20];
    uint8_t *sen_data = (uint8_t *)calloc(5, 4);
    char req_buf[8];

    req_buf[0] = (slave_id & 0xFF);
    req_buf[1] = (func_address & 0xFF);
    req_buf[2] = ((reg_addr >> 8) & 0xFF);
    req_buf[3] = (reg_addr & 0xFF);
    req_buf[4] = (0 & 0xFF);
    req_buf[5] = (data_len & 0xFF);
    uint16_t crc_val = CalcCRC(req_buf, 6);
    req_buf[7] = (crc_val >> 8) & 0xFF;
    ;
    req_buf[6] = (crc_val) & 0xFF;

    gpio_set_level(3, 1);
    //	                 echo_send(uart_num, "\x01\x03\x00\x00\x00\x01\x84\x0A\r\n", 8);
    //	    	     	 uart_write_bytes(2,"\x01\x03\x00\x00\x00\x01\x84\x0A\r\n", 8);
    uart_write_bytes(2, req_buf, 8);
    vTaskDelay(5);
    gpio_set_level(3, 0);
    vTaskDelay(1);
    //	    	     	ESP_LOGI(TAG, "555555555555555555");
    int chk_len = uart_read_bytes(2, sen_data, 12, 100);
    vTaskDelay(1);
    /*	    	         if (len > 0) {
    #if DEBUG_LOG
                             ESP_LOGI(TAG, "Received %u bytes:", len);
    #endif
    #if DEBUG_LOG
                             ESP_LOGI(TAG,"[ ");
    #endif
                             for (int i = 0; i < len; i++) {
    #if DEBUG_LOG
                                 ESP_LOGI(TAG,"0x%.2X", (uint8_t)data[i]);
    #endif
                             }
    #if DEBUG_LOG
                             ESP_LOGI(TAG,"] \n");
    #endif
                         } else {
    #if DEBUG_LOG
                             ESP_LOGI(TAG,"\r \n not received");
    #endif
                             uart_write_bytes(2,"\x01\x03\x00\x00\x00\x01\x84\x0A\r\n", 8);
                             ESP_ERROR_CHECK(uart_wait_tx_done(2, 10));
                         }*/
    //	    	         ESP_LOGI(TAG,"received ist value %d",data[1]);
    //	    	         if(data[1]==0) {
    ////	    	        	 free(data);
    //	    	         }
    if (chk_len > 0)
    {
        calibration_of_RS485_sensor_data(sen_data, chk_len);
        free(sen_data);
    }
    else
    {
        //            	 ESP_LOGI(TAG,"null");
        free(sen_data);
    }

    //	    	         ESP_LOGI(TAG,"data freed");
    //	                 free(data);
}

void Get_GNSS_Data()
{
    GNSS_flag = 1;
    char *transmit_buffer_01 = (char *)calloc(BUF_SIZE, sizeof(char));
    sprintf((char *)transmit_buffer_01, "%s\r\n", GNSS_LOC_CMD);
    sendAT_Data(transmit_buffer_01);
    if (check_response(OK_RESPONSE, 10 * MAX_WAIT_MS))
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "GPS LOCATION get");
#endif
        //		                ESP_LOGI(TAG,"GPS data copied:%s",GNSS_data);
    }
    else
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "GPS location not received");
        utcTime = "";
        latitude = "";
        longitude = "";
        gps_day = "";

        utcTime = strdup(utcTime);
        latitude = strdup(latitude);
        longitude = strdup(longitude);
        gps_day = strdup(gps_day);
#endif
    }
}

void Initialization_of_LTE()
{
    int i2c_wr_err = mpu9250_register_write_byte(0x07, 0x00);
    //	            ESP_LOGI(TAG, "i2c config err %d ", i2c_wr_err);
    i2c_wr_err = mpu9250_register_write_byte(0x06, 0x00);
    //	            ESP_LOGI(TAG, "i2c config err %d ", i2c_wr_err);
    mpu9250_register_write_byte(0x02, 0x00);
    //	            ESP_LOGI(TAG,"i2c config err %d ", i2c_wr_err);
    mpu9250_register_write_byte(0x03, 0x00);
    //	            ESP_LOGI(TAG,"i2c config err %d ", i2c_wr_err);
    vTaskDelay(100);
    Gsm_Power_Off();
    vTaskDelay(1300);
    i2c_wr_err = mpu9250_register_write_byte(0x02, BIT0); // Wakeup pin
                                                          //	            ESP_LOGI(TAG,"i2c write err %d ", i2c_wr_err);
    vTaskDelay(100);

    //		            vTaskDelay(100);
    init_LTE();
}

void process_sensor_status(const char *json_string)
{
    cJSON *root = cJSON_Parse(json_string);

    if (!root)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
#endif
        return; // Error handling
    }

    int status_01 = cJSON_GetObjectItem(root, "status");
    if (status_01)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "status recived");
#endif
    }
    else
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "status not recived");
#endif
    }

    if (status_01)
    {
        int wind_sr = cJSON_GetObjectItem(status_01, "wind_sr")->valueint;
        int dust_conc_sr = cJSON_GetObjectItem(status_01, "dust_conc_sr")->valueint;
        // Repeat for other RS485 sensors

        // Now you can use these values as needed
        if (wind_sr == 1)
        {
            ESP_LOGI(TAG, "wind sensor enabled");
        }
        else
        {
            ESP_LOGI(TAG, "wind sensor disavbled");
        }
        // Repeat for other RS485 sensors
    }

    //    cJSON *rs232_sensor_state = cJSON_GetObjectItem(root, "RS232_sensor_state");
    //
    //    if (rs232_sensor_state) {
    //        int hd2010uc_class_1 = cJSON_GetObjectItem(rs232_sensor_state, "HD2010UC_class_1")->valueint;
    //        int sl_4033sd = cJSON_GetObjectItem(rs232_sensor_state, "SL_4033SD")->valueint;
    //        // Repeat for other RS232 sensors
    //
    //        // Now you can use these values as needed
    //        if (hd2010uc_class_1 == 1) {
    //        	ESP_LOGI(TAG,"Sound level enabled");
    //        } else {
    //        	ESP_LOGI(TAG,"Sound level disabled");
    //        }
    //        // Repeat for other RS232 sensors
    //    }

    cJSON_Delete(root);
}
char *JSON_Types(int type)
{
    if (type == cJSON_Invalid)
        return ("cJSON_Invalid");
    if (type == cJSON_False)
        return ("cJSON_False");
    if (type == cJSON_True)
        return ("cJSON_True");
    if (type == cJSON_NULL)
        return ("cJSON_NULL");
    if (type == cJSON_Number)
        return ("cJSON_Number");
    if (type == cJSON_String)
        return ("cJSON_String");
    if (type == cJSON_Array)
        return ("cJSON_Array");
    if (type == cJSON_Object)
        return ("cJSON_Object");
    if (type == cJSON_Raw)
        return ("cJSON_Raw");
    return NULL;
}

void JSON_Parse(const cJSON *const root)
{
    ESP_LOGI(TAG, "root->type=%s", JSON_Types(root->type));
    cJSON *current_element = NULL;
    // ESP_LOGI(TAG, "roo->child=%p", root->child);
    // ESP_LOGI(TAG, "roo->next =%p", root->next);
    cJSON_ArrayForEach(current_element, root)
    {
        // ESP_LOGI(TAG, "type=%s", JSON_Types(current_element->type));
        // ESP_LOGI(TAG, "current_element->string=%p", current_element->string);
        if (current_element->string)
        {
            const char *string = current_element->string;
            ESP_LOGI(TAG, "[%s]", string);
        }
        if (cJSON_IsInvalid(current_element))
        {
            ESP_LOGI(TAG, "Invalid");
        }
        else if (cJSON_IsFalse(current_element))
        {
            ESP_LOGI(TAG, "False");
        }
        else if (cJSON_IsTrue(current_element))
        {
            ESP_LOGI(TAG, "True");
        }
        else if (cJSON_IsNull(current_element))
        {
            ESP_LOGI(TAG, "Null");
        }
        else if (cJSON_IsNumber(current_element))
        {
            int valueint = current_element->valueint;
            double valuedouble = current_element->valuedouble;
            ESP_LOGI(TAG, "int=%d double=%f", valueint, valuedouble);
        }
        else if (cJSON_IsString(current_element))
        {
            const char *valuestring = current_element->valuestring;
            ESP_LOGI(TAG, "%s", valuestring);
        }
        else if (cJSON_IsArray(current_element))
        {
            // ESP_LOGI(TAG, "Array");
            JSON_Parse(current_element);
        }
        else if (cJSON_IsObject(current_element))
        {
            // ESP_LOGI(TAG, "Object");
            JSON_Parse(current_element);
        }
        else if (cJSON_IsRaw(current_element))
        {
            ESP_LOGI(TAG, "Raw(Not support)");
        }
    }
}

void JSON_Print(cJSON *element)
{
    if (element->type == cJSON_Invalid)
        ESP_LOGI(TAG, "cJSON_Invalid");
    if (element->type == cJSON_False)
        ESP_LOGI(TAG, "cJSON_False");
    if (element->type == cJSON_True)
        ESP_LOGI(TAG, "cJSON_True");
    if (element->type == cJSON_NULL)
        ESP_LOGI(TAG, "cJSON_NULL");
    if (element->type == cJSON_Number)
        ESP_LOGI(TAG, "cJSON_Number int=%d double=%f", element->valueint, element->valuedouble);
    if (element->type == cJSON_String)
        ESP_LOGI(TAG, "cJSON_String string=%s", element->valuestring);
    if (element->type == cJSON_Array)
        ESP_LOGI(TAG, "cJSON_Array");
    if (element->type == cJSON_Object)
        ESP_LOGI(TAG, "cJSON_Object");
    if (element->type == cJSON_Raw)
        ESP_LOGI(TAG, "cJSON_Raw");
}

void BLE_initialization()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {

        ESP_LOGE(GATTC_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        ESP_LOGE(GATTC_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret)
    {
        ESP_LOGE(GATTC_TAG, "%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(GATTC_TAG, "%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_ble_gap_register_callback(esp_gap_cb);
    if (ret)
    {
        ESP_LOGE(GATTC_TAG, "%s gap register failed, error code = %x\n", __func__, ret);
        return;
    }

    // register the callback function to the gattc module
    ret = esp_ble_gattc_register_callback(esp_gattc_cb);
    if (ret)
    {
        ESP_LOGE(GATTC_TAG, "%s gattc register failed, error code = %x\n", __func__, ret);
        return;
    }

    ret = esp_ble_gattc_app_register(PROFILE_A_APP_ID);
    if (ret)
    {
        ESP_LOGE(GATTC_TAG, "%s gattc app register failed, error code = %x\n", __func__, ret);
    }
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret)
    {
        ESP_LOGE(GATTC_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }
}

void read_all_rs485_sensors()
{
    //	ESP_LOGI(TAG,"SO2_conc_sr_state=%d",SO2_conc_sr_state);
    //	ESP_LOGI(TAG,"So2 level=%f",SO2_lvl);

    if (wind_sen_state)
    {
        read_RS485_sensor_data(10, 1, 0, 3);
    }
    else
    {
        wind_speed = 0;
    }
    if (dust_conc_sen_state)
    {
        read_RS485_sensor_data(20, 3, 0, 3);
    }
    else
    {
        dust_pm10 = 0;
        dust_pm2p5 = 0;
        dust_pm1 = 0;
    }
    if (CO2_sen_state)
    {
        read_RS485_sensor_data(30, 1, 0, 3);
    }
    else
    {
        CO2_lvl = 0;
    }
    if (air_quality_sr_state)
    {
        read_RS485_sensor_data(40, 2, 7, 3);
    }
    if (VOC_sr_state)
    {
        read_RS485_sensor_data(50, 1, 0, 3);
    }
    else
    {
        voc_level = 0;
    }
    if (formaldehyde_sr_state)
    {
        read_RS485_sensor_data(60, 1, 0, 3);
    }
    else
    {
        formaldehyde = 0;
    }
    if (UV_rad_sr_state)
    {
        read_RS485_sensor_data(70, 1, 0, 4);
    }
    else
    {
        uv_idx = 0;
    }
    if (solar_illum_sr_state)
    {
        read_RS485_sensor_data(80, 1, 0, 3);
    }
    else
    {
        iluminance = 0;
    }
    if (wind_dir_sr_state)
    {
        read_RS485_sensor_data(90, 1, 0, 3);
    }
    else
    {
        wind_dir = 0;
    }
    if (solar_rad_sr_state)
    {
        read_RS485_sensor_data(100, 1, 0, 3);
    }
    else
    {
        solar_rad = 0;
    }
    if (SO2_conc_sr_state)
    {
        //		            	ESP_LOGI(TAG,"enter So2 state");
        read_RS485_sensor_data(110, 1, 0, 3);
    }
    else
    {
        SO2_lvl = 0;
    }
    if (Atm_hum_pres_sr_state)
    {
        read_RS485_sensor_data(120, 3, 0, 3);
    }
    else
    {
        atm_pres = 0;
        humid = 0;
        pressure = 0;
    }
    if (NO2_conc_sr_state)
    {
        read_RS485_sensor_data(130, 2, 40002, 3);
    }
    else
    {
        NO2_lvl = 0;
    }
    if (CO_conc_sr_state)
    {
        read_RS485_sensor_data(140, 1, 2, 3);
    }
    else
    {
        CO_lvl = 0;
    }
    if (O3_conc_sr_state)
    {
        read_RS485_sensor_data(150, 1, 0, 3);
    }
    else
    {
        O3_lvl = 0;
    }
    if (rain_sr_state)
    {
        read_RS485_sensor_data(160, 1, 0, 3);
    }
    else
    {
        rainfall = 0;
    }
}

void read_RS232_sensors()
{
    if (SL_4033SD_state)
    {
#if DEBUG_LOG
        ESP_LOGI(TAG, "RS232_sound_level");
#endif
        read_RS232_sensor_data_SL_4033();
    }
    else
    {
        sound_level_3033 = 0;
    }
    if (SL_4036SD_state)
    {
        read_RS232_sensor_data_SL_4036();
    }
    else
    {
        sound_level_3036 = 0;
    }
    if (HD2010UC_class_1_state)
    {
        read_RS232_sensor_data_HD2010UC();
    }
    else
    {
        //		                	 sound_level_data_03=0;
    }
    if (WBGT_2010SD_state)
    { //
        read_RS232_sensor_data_WBGT();
        read_RS232_sensor_data_WBGT();
        read_RS232_sensor_data_WBGT();
        read_RS232_sensor_data_WBGT();
        read_RS232_sensor_data_WBGT();
        read_RS232_sensor_data_WBGT();
    }
    else
    {
        WBGT_data_01 = 0;
        WBGT_data_02 = 0;
        WBGT_data_03 = 0;
        WBGT_data_04 = 0;
        WBGT_data_05 = 0;
        WBGT_data_06 = 0;
    }
}

void app_main(void)
{

    vTaskDelay(10);
    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << LED_PIN);
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);
    //	    gpio_set_direction(UART_DIR_RS485, GPIO_MODE_OUTPUT);
    gpio_set_direction(PWR_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(MCU_WDI_PIN, GPIO_MODE_OUTPUT);
    vTaskDelay(10);
    //    gpio_set_level(BLE_LED, 1);
    gpio_set_level(PWR_LED, 0);
    gpio_set_level(MCU_WDI_PIN, 0);
    //    vTaskDelay(1000);
//	    gpio_set_level(UART_DIR_RS485,0);
#if DEBUG_LOG
    ESP_LOGI(TAG, "Application started");
#endif
    uint8_t i2c_data[2];
    int i2c_st = i2c_master_init();
    //	  ESP_LOGI(TAG, "I2C IOEXpander code :%d",i2c_st);
    //	  int i2c_st_01=i2c_temp_sensor_init();
    //	  ESP_LOGI(TAG, "I2C temp sensor code :%d",i2c_st_01);
    UART_Initialization_of_RS485();
    UART_RS232_Initialization();
    UART_Initilaization_of_LTE();
    ADC_Initialization();
    timer_config();
    vTaskDelay(100);
    Initialization_of_LTE();
    vTaskDelay(100);
    //	               send_GNSS_CMD();
    vTaskDelay(100);
    time(&now);
    localtime_r(&now, &timeinfo);
    //	               ESP_LOGI(TAG, "Serialize.....");
    //	               BLE_initialization();
    vTaskDelay(10);
    check_gsm_connection();
    vTaskDelay(10);
    check_gsm_connection();
    vTaskDelay(10);
    if (network_flag)
    {
    }
    //	               while(1) {
    //	            	   read_RS232_sensor_data_HD2010UC();
    //	               }

    while (1)
    {
        check_gsm_connection();
        read_temp_sensor();
        read_battery_voltages();
        if (temp_sensor > 40)
        {
            gpio_set_level(LED_PIN, 0);
            Publish_temp_alert_msg(CLIENT_IDX, 1, 1, 0, SEND_EVETS_TOPIC);
        }
        else
        {
            gpio_set_level(LED_PIN, 1);
        }
        if (temp_sensor < 10)
        {
            Publish_temp_alert_msg(CLIENT_IDX, 1, 1, 0, SEND_EVETS_TOPIC);
        }
        if (lead_acid_bat < 11.63)
        {
            Publish_battery_alert_msg(CLIENT_IDX, 1, 1, 0, SEND_EVETS_TOPIC);
            ESP_LOGI(TAG, "turnoff");
            vTaskDelay(10);
            mpu9250_register_write_byte(0x03, BIT0);
        }

        if (calibration_flag >= 60)
        {
            //	            	   ESP_LOGI(TAG, "calibration start %d",calibration_flag);
            read_all_rs485_sensors();
            read_RS232_sensors();
            if (average_flag >= 10)
            {

                //	            		   ESP_LOGI(TAG, "avg flag entered");
                average_flag = 0;
            }
            //	            	   ESP_LOGI(TAG, "====================== %d",interval_of_heartbeat);

            if (client_flag)
            {
                Get_GNSS_Data();
            }
            hours = hours + 5;
            minutes = minutes + 30;
            if (minutes >= 60)
            {
                minutes = minutes - 60;
                hours = (hours + 1);
            }
            RTC_date_time();
            average_flag++;
            calibration_flag = 0;
            //	            	   ESP_LOGI(TAG, "calibration end %d",calibration_flag);
            //	            	   ESP_LOGI(TAG, "avg flag %d",average_flag);
            //	            	   ESP_LOGI(TAG, "========================%d",interval_of_heartbeat);
        }
        //	               if(interval_of_heartbeat >= 120) {
        //	              	 if(client_flag){
        //	              	    Publish_mqtt_connection_ckeck(CLIENT_IDX,1,1,0,"server/output");
        //	              	 }
        //	              }
        //	               if(interval_of_heartbeat >= 120) {
        //	            	   if(client_flag){
        //	            	   	   PublishMessage(CLIENT_IDX,1,1,0,"server/output");
        //	            	   	}
        //	               }

        if (interval_of_heartbeat >= 600)
        {
            //	            	   ESP_LOGI(TAG, "========================%d",interval_of_heartbeat);
            interval_of_heartbeat = 0;
            if (client_flag)
            {
                PublishMessage(CLIENT_IDX, 1, 1, 0, SEND_EVETS_TOPIC);
            }
        }

//	               ESP_LOGI(TAG, "===========end============%d=================",interval_of_heartbeat);
#if DEBUG_LOG
//			ESP_LOGI(TAG, "packet received from server : %s\r\n", (char *) data_uart1);
#endif
    }
}
