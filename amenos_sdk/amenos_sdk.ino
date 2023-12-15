/***UART Freertos***/
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_intr_alloc.h"
#include <LedControl.h>
#include <SI1133.h>
#include <Wire.h>
#include <stdlib.h>
#include <NTPClient.h>//for getting time from server
#include <WiFi.h>
#include<WiFiUdp.h>
/******For OTAP*********/
#include <WiFiClient.h>
#include "ESP32_FTPClient.h"
#include "FS.h"
#include "SPIFFS.h"
#include "Update.h"
#include<EEPROM.h>//for internal flash
#include<BLEDevice.h>
#include<BLEServer.h>
#include<BLEUtils.h>
#include<BLE2902.h>
#include<ArduinoJson.h>
/****For Beacon scan*****/
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
/***IR library****/
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Coolix.h>
#include <ir_Haier.h>
#include <ir_Panasonic.h>
#include <ir_Samsung.h>
#include <ir_Tcl.h>
#include <ir_Toshiba.h>
#include <ir_Whirlpool.h>
#include <IRac.h>
#include <IRutils.h>
/***Light sensor***/
//#include <LTR303.h>
/***Hmac sha-256***/
//#include "mbedtls/md.h"
/*****Free rtos defintions**************/
#define Fac_To_KILO_BYTES   1024
#define MIN_TASK_SIZE       1024 //1 kb
#define SW_TIMER_PERIOD_MS  (1000/ portTICK_PERIOD_MS)//1
#define TASK_PRIORITY       configMAX_PRIORITIES - 1

/*********Byte Definitions************/
#define BYTE0   0x00
#define BYTE1   0x01
#define BYTE2   0x02
#define BYTE3   0x03
#define BYTE4   0x04
#define BYTE5   0x05
#define BYTE6   0x06
#define BYTE7   0x07
#define BYTE8   0x08
#define BYTE9   0x09
#define BYTE10  0x0A
#define BYTE11  0x0B
#define BYTE12  0x0C
#define BYTE13  0x0D
#define BYTE14  0x0E
#define BYTE15  0x0F

#define BYTE16 0x10
#define BYTE17 0x11
#define BYTE18 0x12
#define BYTE19 0x13
#define BYTE20 0x14
#define BYTE21 0x15
#define BYTE22 0x16
#define BYTE23 0x17
#define BYTE24 0x18
#define BYTE25 0x19
#define BYTE26 0x1A
#define BYTE27 0x1B
#define BYTE28 0x1C
#define BYTE29 0x1D
#define BYTE30 0x1E
#define BYTE31 0x1F
/*********Firmware Revision definition*********/
#define MAJOR_NUMBER      "1"
#define MINOR_NUMBER      "0"
#define FIRMWARE_VERSION  ""MAJOR_NUMBER"."MINOR_NUMBER""
#define MANUFACTURE_DATE  "13-06-2020"
/*********General definition***********/
#define ENABLE  1
#define DISABLE 0
#define DEVELOPMENT DISABLE
#define APPLICATION ENABLE
#define CONFIGURATION_MODE  0x00
#define NORMAL_MODE         0x01
#define OTAP_MODE           0x02
#define NO_MODE             0xFF
#define EXIT_SUCCESS        0
#define EXIT_FAILURE        1
#define TASK_DELAY          1
#define MAX_VARIANT         2
#define ROW                 10
#define COLOMN              5
#define DECIMAL             10
#define HEX_VALUE           16
#define uS_TO_S_FACTOR      1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP       10        /* Time ESP32 will go to sleep (in seconds) */
#define OVER_TEMPERATURE    50
#define BUZZER_DRIVE_COUNT  5000
#define BEEP_DELAY          1
/******GPIO pin defintions********/
#define EXPANDER_INT      36
#define TEMP_ALERT        39
#define CAP_TOUCH_OUT1    34
#define CAP_TOUCH_OUT2    35

#define LED_R1            33
#define LED_G1            25
#define LED_R2            26
#define LED_G2            27
#define IR_CTRL           2
#define LED_B1            19
#define LIGHT_DEFAULT     0x00

#define LED_DRIVE_LOW           HIGH
#define LED_DRIVE_HIGH          LOW
#define TOGGLE                  2
#define TOUCH_BUTTON_LED_TIME   5
#define DEBOUNCING_TIME         20 //Debouncing Time in Milliseconds
/******AC control definitions*****/
#define MIN_TEMPERATURE     16
#define MAX_TEMPERATURE     28
#define DEFAULT_TEMPERATURE MIN_TEMPERATURE
#define LOW_ROOM_TEMP       25
#define HIGH_ROOM_TEMP      30
#define TEMP_HYSTERISIS     2
#define HITACHI             ENABLE
/*******DOT MATRIX definitions******/
#define NUMBER_OF_DEVICES 2 //number of led matrix connect in series
#define ROW_WIDTH         8
#define COLOUMN_WIDTH     8
#define DEGREE_C          1
#define MIN_INTENSITY     5

#define CS_PIN 5
#define CLK_PIN 18
#define MISO_PIN 19 //we do not use this pin just fill to match constructor
#define MOSI_PIN 23 //DIN

/*********I2C definitions**********/
#define SDA_PIN                                   21  //using GPIO21 as SDA
#define SCL_PIN                                   22  //using GPIO22 as SCL
#define MAX_I2C_DEVICE_COUNT                      0x05
//Temperature sensor
#define TEMP_SENSOR_SLAVE_ADDRESS                 0x48   //slave address is 0x48 using all 3 address pins are connected to ground
#define TEMP_SENSOR_CONFIGURATION_REG             0x01
#define TEMP_SENSOR_DATA_REG                      0x00
#define SET_12BIT_RESOLUTION                      (uint16_t)0x6000

//IO Expander
#define EXPANDER_SLAVE_ADDRESS        0x77
#define LIGHT_INTENSITY_SLAVE_ADDRESS 0x29

//Expander IO Pin Assignment
#define SWITCH_BIT                      0x01
#define TEMP_ALERT_BIT                  0X02
#define PWR_CTRL_BIT                    0x04
#define TOUCH_UP_LED_BIT3               0x08
#define TOUCH_DOWN_LED_BIT4             0x10
#define BUZZER_DRIVE_BIT5               0x20
#define LOW_SPEED_RELAY_DRIVE_BIT6      0x40
#define MEDIUM_SPEED_RELAY_DRIVE_BIT7   0x80
#define HIGH_SPEED_RELAY_DRIVE_BIT0     0x01
#define LED_B2_BIT1                     0x02
#define ZIGBEE_BOOT_CTRL_BIT2           0x04
#define ZIGBEE_NRESET_BIT3              0x08

#define TCA9539_INPUT_PORT0             0x00
#define TCA9539_INPUT_PORT1             0x01
#define TCA9539_OUTPUT_PORT0            0x02
#define TCA9539_OUTPUT_PORT1            0x03
#define TCA9539_POLARITY_INVERSION0     0x04
#define TCA9539_POLARITY_INVERSION1     0x05
#define TCA9539_CONFIG0                 0x06
#define TCA9539_CONFIG1                 0x07


/******Wifi definitiions**********/
#define REDMI                       DISABLE
#define MADDY                       ENABLE
#define SUMATHI                     DISABLE
#define NO_WIFI_AVAILABLE           0
#define WIFI_CONNECTION_TIME_OUT    6  //6 - means 3 sec
#define WIFI_CREDENTIAL_BUFFER_SIZE 15
#define WIFI_CREDENTIAL_LENGTH      6
#define CONNECTION_TRIAL_COUNT      3
/*******UDP definitions***********/
#define STATIC_CODE               "CXPL"
#define AIR_CONDITIONER           0
#define SMART_BULB                1
#define SPLIT_AC_TYPE             1
#define HVAC_TYPE                 2
#define DEVICE_TURN_ON            1
#define DEVICE_TURN_OFF           0
#define AUTO_MODE                 "010"
#define COOL_MODE                 "001"
#define DRY_MODE                  "100"
#define FAN_MODE                  "101"
#define EMPTY_IP                  "0.0.0.0"
#define DIM                       "dim"
#define BRIGHT                    "brighten"
#define COMMA_DELIMITER           ","
#define NIGHT_LAMP                1
#define LIVING_AREA               2
#define CORRIDOR                  3
#define YELLOW_COLOR              0xFFFF00
#define RED_COLOR                 0xFF0000
#define BLUE_COLOR                0x0000FF
#define PURPLE_COLOR              0x800080
#define ORANGE_COLOR              0xFFD700
#define GREEN_COLOR               0x008000
#define PINK_COLOR                0xFFC0CB
#define WHITE_COLOR               0xFFFFFF
#define ONE_SEC                   1
#define ONE_MINUTE_IN_SEC         60
#define HR_TO_SEC                 3600
#define HR_TO_MIN                 60
#define END_OF_DAY                24 * HR_TO_MIN
#define TCP_PORT                  1234//80
#define MULT_FACTOR               1000
#define HUB_ACK_TIMEOUT_SEC        10
#define HUB_ACK_TIMEOUT           (uint16_t)( HUB_ACK_TIMEOUT_SEC * MULT_FACTOR)
#define UDP_RECONNECT_TIME_IN_MS  5
#define UDP_RECONNECT_TIME        (uint16_t)( UDP_RECONNECT_TIME_IN_MS * MULT_FACTOR)
#define HEART_BEAT_TIME           ONE_SEC * MULT_FACTOR
#define SENSOR_DATA_TIME          ONE_MINUTE_IN_SEC * MULT_FACTOR
#define VARIANT_UPDATE_TIME       (uint32_t)(5 * ONE_MINUTE_IN_SEC * MULT_FACTOR )
#define AMENOS                    0x05
#define KINISI                    AMENOS+1
#define AIFI                      KINISI+1
#define LIGHT                     AIFI+1
#define DEVICE_TYPE               AMENOS

#define VCON_AC                   BYTE1
#define PANA_AC                   BYTE2
#define VOLT_AC                   BYTE3
#define HAIR_AC                   BYTE4
#define SAMG_AC                   BYTE5
#define AMENOS_DEVICE_TYPE        BYTE5
/*******Sensot to Hub***********/
#define DEVICE_REGISTRATION       1
#define DEVICE_HEALTH_STATUS      2
#define DEVICE_SENSOR_DATA        3
#define LIGHT_REG_ACK_PACKET      4
#define AC_FAIL_PACKET            5
#define MAITENANCE_ACK_PACKET     6
#define OTAP_ACK_PACKET           7
#define AC_SETTING_PACKET         8
#define LIGHT_FAIL_PACKET         9
#define SLAVE_TO_HUB              1
#define SLAVE_TO_AMENOS           3

#define AC_FAIL_ALERT             10
#define LIGHT_FAIL_ALERT          12

/*******Hub to Sensor***********/
#define DAY_MODE                        1
#define NIGHT_MODE                      2
#define AC_CMD_TYPE                     3
#define LIGHT_CMD_TYPE                  4
#define REG_PACKET_MAX_TRIAL_COUNT      3

#define MAINTENANCE_CMD                 6
#define FIRMWARE_UPDATE_CMD             7
#define MOBILE_IP_INFO_CMD              8
#define ENABLE_BLE_MESH                 9
#define SHUTDOWN_AND_CTRL_CMD           10
#define SHUTDOWN_CMD_TYPE               1
#define AC_CTRL_CMD_TYPE                3
#define ZONE_LIGHT_CTRL_CMD_TYPE        4
#define DEVICE_REG_ACK                  11
#define DECOMMISSION_CMD                12
#define REG_REQ_CMD                     14
#define DEFAULT_SETTING_CMD             15
#define AC_DEFAULT_TYPE                 1
#define LIGHT_DEFAULT_TYPE              2
#define LIGHT_CONFIG_CMD                16
#define LOCK_TOUCH_BUTTON_CMD           17
#define FORCE_SET_AC_TEMP_CMD           18
#define AMENITY_SLEEP_MODE_SETTING_CMD  19
#define OVER_TEMP_WAKE_AFTER_CMD        20
#define DEFAULT_BROADCAST_CMD           21


#define PRODUCT_MODEL             "AMENOS"
#define NORMAL_RECEIVE_MODE       0xFF

#define POWER_OFF             0x01
#define POWER_ON              0x00
#define MAINTENANCE_MODE_ON   0x01
#define MAINTENANCE_MODE_OFF  0x00
/*******FLASH definition*******/
//No of bytes allocated to each parameters in flash
#define SERIAL_NUM_ALLOCATED_BYTE                   0x14
#define AP_SSID_ALLOCATED_BYTE                      0x0A
#define AP_PASS_KEY_ALLOCATED_BYTE                  0x0C
#define NOT_FACTORY_DEVICE                          BYTE1

#define FLASH_SIZE                                  1024
#define MAX_KINISI_DEVICE                           5
#define FLASH_READ_WRITE_DELAY                      0x02
#define FLASH_BASE_ADDRESS                          0x00
#define AMENOS_ALLOCATED_ADDR                       200
#define OPERATING_MODE_BASE_ADDR                    FLASH_BASE_ADDRESS
#define SERIAL_NUM_BASE_ADDR                        (OPERATING_MODE_BASE_ADDR + BYTE1)
#define AP_SSID_BASE_ADDR                           (SERIAL_NUM_BASE_ADDR + SERIAL_NUM_ALLOCATED_BYTE)
#define AP_PASS_KEY_BASE_ADDR                       (AP_SSID_BASE_ADDR + AP_SSID_ALLOCATED_BYTE)
#define WIFI_CONNECTION_TRIAL_COUNT_BASE_ADDR       (AP_PASS_KEY_BASE_ADDR + AP_PASS_KEY_ALLOCATED_BYTE)
#define DEVICE_REG_STS_BASE_ADDR                    (WIFI_CONNECTION_TRIAL_COUNT_BASE_ADDR + BYTE1)
#define CONFIG_DATA_STATUS_BASE_ADDR                (DEVICE_REG_STS_BASE_ADDR + BYTE1)
#define AC_TYPE_BASE_ADDR                           (CONFIG_DATA_STATUS_BASE_ADDR + BYTE1)
#define AC_BRAND_BASE_ADDR                          (AC_TYPE_BASE_ADDR + BYTE1)
#define LIGHT_CTRL_BASE_ADDR                        (AC_BRAND_BASE_ADDR + BYTE1)
#define OCCUPANCY_BASE_ADDR                         (LIGHT_CTRL_BASE_ADDR + BYTE1)
#define DEVICE_TYPE_BASE_ADDR                       (OCCUPANCY_BASE_ADDR + BYTE1)
#define ORG_ID_BASE_ADDR                            (DEVICE_TYPE_BASE_ADDR + BYTE1)
#define FACILITY_ID_BASE_ADDR                       (ORG_ID_BASE_ADDR + BYTE3)
#define ROOM_NUM_BASE_ADDR                          (FACILITY_ID_BASE_ADDR + BYTE3)
#define DEVICE_NAME_BASE_ADDR                       (ROOM_NUM_BASE_ADDR + BYTE6)
#define KINISI_REG_STS_BASE_ADDR                    (DEVICE_NAME_BASE_ADDR + BYTE11)
#define SET_AC_TEMP_BASE_ADDR                       (KINISI_REG_STS_BASE_ADDR + BYTE1)
#define FACTORY_DEVICE_BASE_ADDR                    (SET_AC_TEMP_BASE_ADDR + BYTE1)
#define UP_BUTTON_PRESS_COUNT_BASE_ADDR             (FACTORY_DEVICE_BASE_ADDR + BYTE1)
#define DOWN_BUTTON_PRESS_COUNT_BASE_ADDR           (UP_BUTTON_PRESS_COUNT_BASE_ADDR + BYTE2)
#define AC_CUM_ON_TIME_BAS_ADDR                     (DOWN_BUTTON_PRESS_COUNT_BASE_ADDR + BYTE2)
#define LIGHT_CUM_ON_TIME_BAS_ADDR                  (AC_CUM_ON_TIME_BAS_ADDR + sizeof(uint32_t))
#define OVER_TEMP_STATUS_BASE_ADDR                  (LIGHT_CUM_ON_TIME_BAS_ADDR + sizeof(uint32_t))
#define HSR_COUNT_BASE_ADDR                         (OVER_TEMP_STATUS_BASE_ADDR + BYTE1)
#define MSR_COUNT_BASE_ADDR                         (HSR_COUNT_BASE_ADDR + sizeof(uint32_t))
#define LSR_COUNT_BASE_ADDR                         (MSR_COUNT_BASE_ADDR + sizeof(uint32_t))
#define SET_AC_MODE_BASE_ADDR                       (LSR_COUNT_BASE_ADDR + sizeof(uint32_t))
#define SET_AC_STATUS_BASE_ADDR                     (SET_AC_MODE_BASE_ADDR + BYTE3)
#define AUTH_CODE_BASE_ADDR                         (SET_AC_STATUS_BASE_ADDR + BYTE1)


#define KINSI_BASE_ADDR                             (FLASH_BASE_ADDRESS +  AMENOS_ALLOCATED_ADDR)
#define KINISI_DEVICE_COUNT                          KINSI_BASE_ADDR
#define KINISI_DEVICE_NAME_BASE_ADDR                (KINISI_DEVICE_COUNT + BYTE1)
#define KINISI_SERIAL_NUM_BASE_ADDR                 (KINISI_DEVICE_NAME_BASE_ADDR + (BYTE11 * MAX_KINISI_DEVICE))
#define KINISI_DECOM_DEVICE_COUNT                   (KINISI_SERIAL_NUM_BASE_ADDR + (BYTE16 * MAX_KINISI_DEVICE))
#define KINISI_REG_ADDR                             (KINISI_DECOM_DEVICE_COUNT  + BYTE1)
#define MAX_KINISI_COUNT_BASE_ADDR                  (KINISI_REG_ADDR  + MAX_KINISI_DEVICE)
#define KINISI_SERIAL_MACID_BASE_ADDR               (MAX_KINISI_COUNT_BASE_ADDR  + BYTE1)
/******BLE definition**********/
#define RESERVED                  0x00
#define CHARACTER_OFFSET          0x30
#define BLE_BUFFER_SIZE           25
#define BLE_PERIODIC_ADV_TIMEOUT  (30 * MULT_FACTOR) // sec * 1000
#define MAX_BLE_PACKET_SIZE       0x14
#define SERVICE_UUID              "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX    "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX    "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_PASSOWRD              "CXAMENOS"
#define KINISI_AMENOS_PASSWORD    "amenos#k"
#define BLE_PASSKEY_FUNC_CODE     0x05
#define BLE_FUNCTIONAL_CODE       0x07
#define AMENOS_BLE_COMMAND        0xF4
#define BLE_ACK_INSTANT_ID        0x01
#define AUTHENTICATION            0x00
#define AUTHENTICATION_LEN        0x08
#define ROOM_DETAILS              0x01
#define SERIAL_NUMBER             0x02
#define DEVICE_DETAILS            0x03
#define AUTH_CODE                 0x04
#define CONFIGURATION_FAILED      0xD1
//#define CONFIGURATION_SUCCESS   0xA1
#define WIFI_CONNECTION_FAIL      0xD2
#define CONFIGURATION_SUCCESS   0xAA
#define CONFIG_WRITE_FAIL       0xD1
#define CONNECTION_IN_PROGRESS  0xA1
#define AUTHENTICATION_SUCCESS  0xAA
#define AUTHENTICATION_FAIL     0xDD

/********Kinisi BLE defintions***********/
#define KINISI_FUNCTIONAL_CODE    0x0A
#define KINISI_BLE_COMMAND        0xF7
#define BLE_NACK                  0xDD
#define OCCUPANCY_SENSED          0x01
/***************Beacon definitions***********/
#define KINISI_ADV_NAME             "Kinisi_Amenos"
#define KINISI_BEACON_NAME          "KinisBeacon"
#define PRODUCT_MODEL_KINISI        "KINISI"
#define BEACON_MODE                 "beacon"
#define SCAN_TIME                   1
#define BEACON_RX_BUFFER_SIZE       300
#define BEACON_SERVICE_CLASS_FLAG   BYTE3
#define BEACON_SERVICE_UUID         0x3424
#define KINISI_SERIAL_NUM_LEN       0x10
#define KINSI_DEVICE_NAME_LEN       BYTE11
#define BEACON_AD0_LEN              BYTE2
#define BEACON_AD1_LEN              BYTE3
#define BEACON_AD2_LEN              0x2B
#define MAX_KINISI_COUNT            10
/****************Zigbee Defintions********************/
//Uart Pin Assignment
#define ZIGBEE_RX  16//pin #
#define ZIGBEE_TX  17//pin #
#define ZIGBEE_UART         UART_NUM_2
#define PATTERN_CHR_NUM     (3)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/
#define BUF_SIZE            (1024)
#define RD_BUF_SIZE         (BUF_SIZE)
#define BUFFER_SIZE         50
//AT Commands
#define AT                "AT\n"
#define SOFT_RESET        "AT&F\n"
#define ENABLE_MESH       "AT+ENMESH\n"
#define MESH_STATUS       "AT+ENMESH?\n"
#define LIST_BIND_TABLE   "AT+LBTABLE\n"

//AT command response
#define OK_RESPONSE "\r\nOK\r\n"
#define ERROR_CODE  "\r\nERROR:"

/******************OTAP definitions*************/
#define FTP_USER_NAME       "hubuser"
#define FTP_PASSWORD        "123456"
#define FTP_MAX_TRIAL_COUNT REG_PACKET_MAX_TRIAL_COUNT
/*****************Zigbee parameters*************/
bool device_available;
char CONFIG_LIGHT[BUFFER_SIZE];
char CTRL_LIGHT[BUFFER_SIZE];
char DECOMMISSION_LIGHT[BUFFER_SIZE];
/******************DOT MATRIX parameters***********/
/*
 pin 23 is connected to the DataIn
 pin 18 is connected to the CLK
 pin 5 is connected to CS
 We have 2 MAX72XX.
 */

//LedControl lc=LedControl(MOSI_PIN,CLK_PIN,CS_PIN,NUMBER_OF_DEVICES);

/* we always wait a bit between updates of the display */
unsigned long delaytime=1000;
byte matrix1_display[8], matrix2_display[8]; //byte is 8-bit unsigned (unsignede char)
byte word1[8];
unsigned int number_of_digits, input_temp;
char input_temperature[3], previous_temperature[3];
int data_on;
bool button_pressed;
uint16_t previous_lux,current_lux;
char list_of_characters[11] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ' '
};

unsigned char led_hex[13][8] = {
{0x00,0x00,0xe0,0xa0,0xa0,0xa0,0xe0,0x00},//0
{0x00,0x00,0x40,0xc0,0x40,0x40,0xe0,0x00},//1
{0x00,0x00,0xe0,0x20,0xe0,0x80,0xe0,0x00},//2
{0x00,0x00,0xe0,0x20,0xe0,0x20,0xe0,0x00},//3
{0x00,0x00,0xa0,0xa0,0xe0,0x20,0x20,0x00},//4
{0x00,0x00,0xe0,0x80,0xe0,0x20,0xe0,0x00},//5
{0x00,0x00,0xe0,0x80,0xe0,0xa0,0xe0,0x00},//6
{0x00,0x00,0xe0,0x20,0x20,0x20,0x20,0x00},//7
{0x00,0x00,0xe0,0xa0,0xe0,0xa0,0xe0,0x00},//8
{0x00,0x00,0xe0,0xa0,0xe0,0x20,0xe0,0x00},//9
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},//space
{0x00,0x08,0x00,0x07,0x04,0x04,0x07,0x00},//degree C
{0x00,0x10,0x07,0x04,0x07,0x04,0x04,0x00} //degree F
};
//{0x18,0x18,0x00,0x07,0x04,0x04,0x07,0x00},//degree C


/*******I2c device parameters****/
byte found_slave_address[MAX_I2C_DEVICE_COUNT];

uint16_t read_temp;
float room_temperature;
uint16_t default_cool_mode_AC_temperature;

uint8_t Exp_Port0,Exp_Port1,Exp_Port_Cfg0,Exp_Port_Cfg1;

enum i2cdevices
{
  LIGHT_SENSOR,
  TEMPERATURE_SENSOR,
  IO_EXPANDER,
};


LTR303 light_sensor;

// Global variables:

unsigned char gain;     // Gain setting, values = 0-7
unsigned char integrationTime;  // Integration ("shutter") time in milliseconds
unsigned char measurementRate;  // Interval between DATA_REGISTERS update
double lux;    // Resulting lux value

/*********GPIO parameters*********/
enum gpio_input
{
  TOUCH_1,
  TOUCH_2,
  TOUCH_1_PRESS_COUNT,
  TOUCH_2_PRESS_COUNT,
  GPIO_INPUT_SIZE,
};
uint16_t input_gpio[GPIO_INPUT_SIZE];
volatile uint16_t up_button_press_count = 0;
volatile uint16_t down_button_press_count = 0;
volatile uint32_t hsr_switching_count = 0;
volatile uint32_t msr_switching_count = 0;
volatile uint32_t lsr_switching_count = 0;
int numberOfInterrupts = 0;

volatile unsigned long last_micros;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
/********WiFi parameters**********/
enum wifi_connection_status
{
  WIFI_NOT_CONNECT,
  WIFI_CONNECTION_IN_PROGRESS,
  WIFI_CONNECTED,
  CONFIG_FAIL,
  PASSWORD_MATCHED,
  PASSWORD_MIS_MATCHED,
  WIFI_STATUS_BUFFER_SIZE,
};
#if REDMI
  char* ssid = "123456";
  const char* passkey = "#123456123#";
#endif
#if MADDY
  char* ssid = "123456";
  const char* passkey = "#123456123#";
#endif
bool write_data_to_flash;
bool send_device_reg_packet;
uint8_t ap_ssid[WIFI_CREDENTIAL_BUFFER_SIZE];
uint8_t ap_passkey[WIFI_CREDENTIAL_BUFFER_SIZE];
uint8_t configuration_status;
uint8_t trial_count = 0;
/********UDP parameters***********/
WiFiUDP Udp;
const char* udp_ip = "192.168.43.18"; // enter pc ip
unsigned int udp_port = 8080;      // server port
char  udp_rx_packets[20]; //udp_tx_packets to hold incoming packet
uint8_t  udp_tx_packets[10] = "hari";       // a string to send back

/**********TCP parameters*********/
WiFiClient TCP_Client;// Use WiFiClient class to create TCP connections
const char* tcp_host = "192.168.43.224";//"172.217.160.132"; //tcp_host
//const char* tcp_host = "www.google.com";
const int tcp_port = 8088;//TCP_PORT;
/* create a server and listen on port 8088 */
//WiFiServer TCP_server(80);
/********NTP parameters*********/
/*
 * Indian ntp server          : in.pool.ntp.org
 * GMT(Greenwhich Mean Time)  : 5.30 (19800 seconds)
 */
NTPClient timeClient(Udp, "in.pool.ntp.org", 19800, 60000);
char dayWeek [7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String formatted_date;
String timeStamp;
struct timeinfo
{
  uint8_t hour;
  uint8_t minute;
  uint8_t sec;
  uint32_t on_minute;
  uint32_t current_on_time;
  uint32_t previous_on_time;
};
enum Device_Variant{
  AC,
  BULB,
  KINISI_HS,
  KINISI_SCAN,
  DEVICE_VARIANT_SIZE,
};
struct timeinfo current;
struct timeinfo previous;
struct timeinfo ac;
struct timeinfo light;
uint32_t total_on_minute;
uint32_t previous_total_on_minute;
uint32_t total_bulb_on_time;
uint32_t total_ac_on_time;
uint32_t cum_on_time[DEVICE_VARIANT_SIZE];
uint8_t *seceret_key;
byte hmacResult[32];
uint16_t found_location[30] ={0};
char *hmac_payload;
/*******BLE parameters*********/
BLECharacteristic *pTxCharacteristic;
BLECharacteristic *pRxCharacteristic;
BLEServer *pServer;// = BLEDevice::createServer();//create ble server
BLEService *pService;// = pServer->createService(SERVICE_UUID);//create ble service
enum BLE_Rx_Data
{
  RXD_PACKET_SIZE,
  FUNCTION_CODE,
  COMMAND,
  INSTANT_ID,
};
enum ble_ack_packet
{
  PACKET_SIZE,
  FUNCTION_CODE_BYTE,
  COMMAND_BYTE,
  INSTANT_ID_BYTE,
  RESULT_BYTE,
  BLE_ACK_BYTE_SIZE,
};
bool ble_connection_status = false;
bool config_data_rcvd,allow_process_ble_data;
int ble_tx_value = 0;
//char ble_tx_data[10];
uint32_t volatile ble_rx_length;
bool volatile ble_rx_flag;
char ble_rx_buffer[BLE_BUFFER_SIZE];
char ble_authentication[BLE_BUFFER_SIZE];
uint8_t room_details[BLE_BUFFER_SIZE];
uint8_t wifi_details[BLE_BUFFER_SIZE];
uint8_t serial_number[BLE_BUFFER_SIZE];
uint8_t ble_ack_to_mobile[BLE_ACK_BYTE_SIZE];
uint8_t device_name[BLE_BUFFER_SIZE];
uint8_t room_no[BLE_BUFFER_SIZE/2];
uint8_t org_id[BLE_BUFFER_SIZE/2];
uint8_t facility_id[BLE_BUFFER_SIZE/2];
uint8_t copy_ap_passkey[BLE_BUFFER_SIZE/2];
uint8_t amenos_auth_code[BLE_BUFFER_SIZE/2];
char device_type;
char tirisi_ac_brand,tirisi_ac_type,max_kinisi_in_room;
bool light_control,occupancy_detection;
uint16_t device_variant;
String bulb_lookup_table[5][20];
/*************Kinisi parameters************/
bool connected_to_kinisi,kinisi_connection_lost;
bool allow_process_Kinisi_ble_data;
uint8_t occupancy_detected;
uint8_t kinisi_room_details[BLE_BUFFER_SIZE];
uint8_t kinisi_serial_number[BLE_BUFFER_SIZE];
uint8_t kinisi_device_name[BLE_BUFFER_SIZE];
char Kinisi_Mac_Id[BLE_BUFFER_SIZE];
uint8_t tx_data_to_kinisi[MAX_BLE_PACKET_SIZE];
uint8_t kinisi_auth_code[BLE_BUFFER_SIZE/2];
char occupancy_mac_id[MAX_BLE_PACKET_SIZE];
uint16_t Kinisi_ack_status,no_of_kinisi_device,kinisi_reg_location;
const char *kinisi_mac_id;
enum mfg
{
  DATE,
  MONTH,
  YEAR_MSB,
  YEAR_LSB,
  MFG_SIZE,
};
char kinisi_mfg[MFG_SIZE] ;
uint8_t *kinisi_serial_num_ptr[MAX_KINISI_COUNT];
uint8_t *kinisi_device_name_ptr[MAX_KINISI_COUNT];
char *kinisi_decommission_serial_num[MAX_KINISI_COUNT];
uint8_t *kinisi_macid_ptr[MAX_KINISI_COUNT];
String kinisi_lookup_table[MAX_KINISI_DEVICE][6];

/*
 * Colomn data for kinisi_lookup_table
 * 0 -no of kinisi
 * 1 -serial num
 * 2 -Device name
 * 3 -decommission status
 * 4 -Registration status
 */

enum kinisi_ble_packet
{
  PASSWORD,
  KINISI_REG_ACK,
};
enum kinisi_device_count
{
  KINISI_SERIAL_NUM,
  KINISI_DEVICE_NAME,
  KINISI_DECOMMISSION_COUNT,
  KINSI_DEVICE_COUNT_SIZE,
};
uint16_t device_count[KINSI_DEVICE_COUNT_SIZE];
/**************Beacon parameters***********/


// The remote service we wish to connect to.
static BLEUUID serviceUUID(SERVICE_UUID);
// The characteristic of the remote service we are interested in.
static BLEUUID    charUUID_RX(CHARACTERISTIC_UUID_RX); //server's Rx characteristics
static BLEUUID    charUUID_TX(CHARACTERISTIC_UUID_TX); //server's Tx characteristics
String My_BLE_Address = "a4:cf:12:9a:c2:c2";

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLEClient*  pClient;
static BLEScan* pBLEScan;
static BLERemoteCharacteristic* pRemoteRxCharacteristic;
static BLERemoteCharacteristic* pRemoteTxCharacteristic;
static BLEAdvertisedDevice* myDevice;
static BLEAddress *Server_BLE_Address;
String Scaned_BLE_Address;
uint8_t scan_data[BEACON_RX_BUFFER_SIZE];
uint8_t kinisi_org_id[BLE_BUFFER_SIZE/2];
uint8_t kinisi_Facility_id[BLE_BUFFER_SIZE/2];
uint8_t kinisi_Amenity_id[BLE_BUFFER_SIZE/2];
uint16_t kinisi_device_type;
char kinsi_fm_version[4];
int kinisi_battery_percentage;
bool beacon_rx_flag;
bool register_kinisi,kinisi_register_status,send_password_to_kinisi;
uint16_t beac_rxlen;
char adv_room_no[BLE_BUFFER_SIZE/2];
String scan_ble_list[MAX_KINISI_COUNT];
uint8_t beacon_count;
/***************Flash parameters**********/
enum len
{
  SERIAL_NUM,
  WIFI_SSID,
  WIFI_PASSWORD,
  ORG_ID,
  FACILITY_ID,
  ROOM_NUM,
  DEVICE_NAME,
  BEACON_FLAG,
  BEACON_UUID,
  BEACON_SCAN_DATA,
  STATIC_CODE_LEN,
  SENSOR_AUTH_CODE,
  LEN_BUFFER_SIZE,
};
uint16_t Length[LEN_BUFFER_SIZE];
uint8_t flash_read_data[100];
/***********Json parameters**********/
//wheather report link
const char* resource = "http://api.openweathermap.org/data/2.5/weather?q=chennai,IN&APPID=6f1ceb2d7c2764eeb3fb92607f68e7e6";
char json_post_buffer[512];

struct udp_server_response
{
    const char* hmac;
    const char* device_id;
    const char* devicename;
    const char* ipaddress;
    const char* amenity_name;
    const char* zone_light_epid;
    const char* individual_light_macid;
    const char* kinisi_serial_no;
    const char* ac_mode;
    const char* fm_version;
    const char* fm_path;
    const char* fm_crc;
    const char* fm_file_size;
    const char* from_time;
    const char* to_time;
    const char* light_mfg_date;
    const char* light_org_uid;
    const char* light_facility_uid;
    const char* light_device_name;
    const char* light_amenity_name;

    uint32_t action,time_type;
    uint32_t zone,command_type,default_type;
//    uint32_t fm_file_size;
    uint8_t light_contrast;
    uint8_t ac_status;
    bool ack_type;
    bool status;
    bool maintenance_mode;
    bool poweroff;
    bool reg_required;
    bool pairing_mode;
    bool light_status;
    bool ble_mesh;
    uint32_t light_color;
    uint32_t day_mode_temp,night_mode_temp,ac_temp,ac_type;
};
struct udp_server_response json_extract;
bool device_register_status,enable_maintenance_mode;
char *mobile_host;
/************General parameters*********/
struct relay_status
{
  bool on;
  bool off;
};
struct relay_status high_speed_relay;
struct relay_status medium_speed_relay;
struct relay_status low_speed_relay;
char str_buffer[20];
uint16_t registration_packet_send_count;
uint8_t operating_mode;
unsigned long current_time,last_time,hs_current_time,hs_last_time;
char char_ssid[5];
uint8_t txValue = 0;
bool device_powered;
uint32_t Current_Time[DEVICE_VARIANT_SIZE];
uint32_t Last_Time[DEVICE_VARIANT_SIZE];
uint16_t bulb_count;
enum UDP_Packet{
  AC_CTRL,
  LIGHT_CTRL,
  DEVICE_REG_REQUEST,
  LIGHT_CONFIG,
  LOCK_BUTTON,
  FORCE_SET_PACKET,
  AMENITY_SLEEP_MODE,
  OVER_TEMP,
  OTAP,
  LIGHT_DECOMMISSION,
  BLE_MESH,
  UDP_PACKET_SIZE,
};
bool UDP_Rx_Packet[UDP_PACKET_SIZE];
/***********AC Parameters******************/
enum ac_brand
{
  COOLIX_BRAND,
  HAIER_BRAND,
  PANASONIC_BRAND,
  SAMSUNG_BRAND,
  TCL_BRAND,
  TOSHIBA_BRAND,
  WHIRLPOOL_BRAND,
  HITACHI_BRAND,
};
IRSamsungAc samsung(IR_CTRL);  // Set the GPIO used for sending messages.
IRCoolixAC  coolix(IR_CTRL);
IRHaierAC   haier(IR_CTRL);
IRPanasonicAc panasonic(IR_CTRL);
IRTcl112Ac tcl(IR_CTRL);
IRToshibaAC tosibha(IR_CTRL);
IRWhirlpoolAc whirlpool(IR_CTRL);
IRsend irsend(IR_CTRL);

#if HITACHI
uint16_t turn_off[595] = {3324, 1774,  376, 1294,  378, 476,  374, 478,  338, 506,  376, 472,  340, 508,  314, 508,  374, 474,  374, 476,  342, 508,  374, 476,  340, 508,  376, 1292,  344, 508,  342, 506,  342, 508,  372, 476,  340, 508,  314, 508,  372, 474,  342, 508,  340, 508,  372, 476,  374, 476,  340, 506,  320, 500,  374, 476,  342, 508,  374, 472,  342, 508,  374, 1294,  344, 508,  342, 1352,  378, 1320,  344, 1328,  344, 1354,  344, 1354,  344, 1326,  378, 474,  374, 1320,  376, 1322,  376, 1296,  376, 1320,  346, 1354,  344, 1354,  318, 1354,  346, 1352,  344, 1354,  378, 450,  372, 476,  342, 506,  342, 508,  342, 508,  340, 508,  340, 508,  316, 508,  374, 472,  344, 536,  312, 1354,  346, 1352,  318, 508,  340, 508,  342, 1352,  344, 1354,  344, 1328,  376, 1322,  376, 476,  340, 508,  340, 1328,  344, 1354,  344, 508,  372, 478,  340, 1326,  344, 1354,  346, 508,  348, 500,  342, 508,  340, 1328,  344, 506,  342, 1352,  344, 506,  342, 508,  340, 1326,  344, 1354,  344, 1354,  376, 476,  340, 1328,  376, 476,  372, 1322,  376, 1322,  318, 508,  340, 508,  340, 1352,  346, 506,  342, 508,  372, 476,  314, 506,  376, 476,  374, 1320,  346, 1352,  344, 482,  372, 1320,  344, 1354,  346, 1352,  318, 510,  340, 508,  340, 1354,  344, 1354,  344, 482,  342, 1354,  344, 1354,  344, 508,  374, 1294,  344, 1354,  344, 508,  342, 506,  342, 1352,  318, 508,  342, 508,  342, 1352,  346, 508,  372, 474,  342, 482,  340, 508,  342, 508,  340, 508,  340, 508,  340, 508,  340, 1328,  344, 1348,  350, 1352,  346, 1354,  318, 1354,  344, 1352,  346, 1354,  344, 1326,  346, 506,  342, 506,  342, 506,  342, 508,  342, 508,  314, 508,  340, 508,  340, 508,  340, 1354,  344, 1326,  344, 1354,  344, 1354,  346, 1352,  346, 1326,  344, 1354,  344, 1354,  344, 508,  314, 508,  340, 508,  342, 508,  340, 508,  340, 508,  342, 508,  340, 482,  340, 1354,  344, 1354,  344, 1352,  318, 1354,  346, 1352,  346, 1354,  344, 1326,  346, 1354,  346, 506,  342, 508,  340, 508,  342, 482,  340, 506,  342, 508,  342, 508,  342, 508,  340, 1352,  318, 1354,  346, 1352,  344, 1354,  344, 1326,  346, 1352,  344, 1354,  344, 1326,  344, 508,  340, 508,  340, 508,  342, 508,  340, 508,  342, 508,  318, 504,  340, 508,  342, 1354,  344, 1354,  344, 1326,  344, 1354,  344, 1354,  346, 1326,  344, 1340,  358, 1354,  344, 1354,  318, 508,  342, 1354,  344, 508,  342, 508,  342, 1326,  344, 508,  340, 508,  340, 508,  342, 1352,  344, 508,  316, 1352,  344, 1354,  344, 506,  342, 1298,  372, 1354,  344, 1352,  346, 508,  342, 506,  342, 482,  340, 508,  340, 1354,  344, 1352,  344, 1328,  344, 508,  342, 1352,  344, 1354,  344, 1328,  346, 1352,  344, 508,  342, 508,  340, 508,  316, 506,  342, 538,  312, 508,  340, 508,  342, 508,  340, 508,  340, 508,  314, 508,  342, 1352,  344, 1354,  346, 1326,  344, 1354,  344, 1354,  346, 1354,  316, 1354,  346, 1352,  346, 506,  342, 508,  340, 482,  340, 508,  342, 506,  342, 510,  338, 508,  340, 510,  340, 1326,  344, 1354,  344, 1354,  344, 1354,  318, 1354,  346, 1352,  344, 1354,  344, 1326,  344, 508,  340, 508,  342, 508,  340, 1354,  318, 508,  342, 508,  340, 508,  340, 508,  340, 1352,  346, 1326,  344, 1354,  344, 508,  342, 1354,  344, 1328,  344, 1354,  344, 1354,  344, 1328,  344, 1352,  344, 508,  340, 508,  340, 506,  342, 480,  342, 508,  342, 508,  340, 508,  340, 508,  342, 1352,  344, 1326,  346, 1352,  344, 1354,  344, 1326,  346, 1354,  344};  // UNKNOWN D0B59572
uint16_t turn_on[595] = {3292, 1806,  344, 1328,  342, 510,  340, 510,  340, 508,  340, 510,  340, 510,  312, 508,  342, 508,  340, 508,  340, 508,  340, 536,  312, 506,  344, 1326,  344, 508,  342, 508,  340, 508,  342, 508,  340, 508,  314, 508,  342, 508,  340, 536,  314, 508,  340, 506,  342, 508,  340, 508,  314, 508,  340, 508,  342, 508,  342, 506,  340, 510,  340, 1326,  344, 508,  342, 1352,  346, 1352,  344, 1326,  346, 1354,  344, 1354,  344, 1326,  344, 510,  340, 1354,  344, 1354,  346, 1326,  344, 1354,  344, 1354,  346, 1352,  318, 1354,  346, 1354,  344, 1354,  344, 482,  340, 508,  340, 508,  340, 508,  340, 510,  340, 508,  342, 508,  314, 508,  340, 508,  340, 508,  340, 1354,  344, 1354,  316, 510,  340, 508,  342, 1354,  344, 1352,  346, 1326,  344, 1354,  346, 508,  340, 508,  340, 1328,  344, 1352,  346, 506,  342, 506,  342, 508,  340, 1328,  344, 510,  340, 508,  342, 1352,  346, 508,  340, 484,  338, 1354,  344, 1354,  344, 508,  340, 1326,  346, 1354,  344, 508,  342, 1302,  396, 1328,  344, 508,  342, 1354,  344, 1354,  318, 510,  342, 508,  340, 1354,  344, 536,  312, 508,  342, 508,  312, 508,  340, 508,  340, 1354,  344, 1352,  346, 482,  340, 1354,  344, 1354,  344, 1354,  316, 510,  340, 508,  340, 1352,  344, 1352,  346, 482,  340, 1320,  378, 1354,  346, 506,  342, 1326,  344, 1354,  344, 508,  342, 508,  340, 1354,  318, 506,  342, 508,  340, 1354,  344, 508,  340, 508,  342, 482,  340, 510,  340, 508,  340, 508,  340, 508,  340, 508,  340, 1326,  344, 1354,  344, 1354,  346, 1352,  318, 1354,  344, 1294,  404, 1354,  344, 1328,  344, 508,  342, 508,  340, 508,  340, 508,  340, 508,  314, 508,  340, 508,  340, 508,  342, 1352,  344, 1326,  344, 1352,  346, 1354,  344, 1354,  346, 1326,  344, 1352,  344, 1354,  344, 508,  314, 508,  342, 508,  340, 508,  340, 508,  340, 506,  342, 508,  340, 480,  342, 1354,  344, 1352,  346, 1352,  318, 1354,  344, 1354,  344, 1352,  346, 1326,  346, 1354,  344, 508,  340, 510,  338, 508,  342, 482,  340, 508,  340, 508,  340, 508,  340, 508,  342, 1352,  318, 1354,  344, 1354,  344, 1354,  344, 1326,  346, 1352,  346, 1352,  344, 1328,  344, 508,  342, 508,  340, 508,  342, 508,  340, 508,  340, 508,  314, 508,  340, 506,  342, 1352,  344, 1354,  344, 1326,  346, 1354,  344, 1354,  344, 1326,  344, 1354,  346, 1352,  344, 1354,  318, 508,  342, 1354,  344, 508,  342, 506,  342, 1328,  344, 508,  342, 508,  340, 508,  340, 1352,  344, 508,  314, 1354,  344, 1354,  344, 508,  342, 1326,  344, 1354,  344, 1354,  344, 508,  340, 508,  340, 484,  340, 1352,  344, 1352,  346, 1354,  318, 1354,  344, 508,  342, 1352,  346, 1326,  344, 1352,  346, 508,  340, 508,  340, 508,  340, 508,  314, 508,  340, 508,  340, 508,  340, 506,  344, 508,  340, 506,  342, 512,  310, 508,  342, 1354,  344, 1354,  344, 1328,  344, 1352,  344, 1332,  366, 1354,  318, 1354,  346, 1352,  344, 508,  340, 508,  340, 482,  340, 506,  342, 506,  342, 536,  312, 508,  342, 506,  342, 1326,  344, 1352,  346, 1354,  344, 1352,  318, 1354,  344, 1354,  344, 1354,  344, 1326,  344, 508,  342, 508,  340, 508,  340, 508,  342, 536,  286, 508,  342, 506,  342, 508,  340, 1354,  344, 1354,  318, 1354,  344, 1354,  344, 1352,  344, 1328,  344, 1352,  344, 1354,  344, 1328,  344, 1354,  344, 508,  340, 508,  340, 508,  340, 482,  340, 508,  340, 508,  342, 508,  342, 508,  340, 1354,  344, 1328,  344, 1354,  344, 1354,  344, 1326,  344, 1354,  344};  // UNKNOWN 16CF7DAE
uint16_t temp_down[595] = {3382, 1714,  436, 1236,  436, 416,  430, 418,  404, 444,  404, 446,  430, 418,  404, 418,  420, 428,  430, 420,  432, 416,  430, 420,  430, 418,  432, 1234,  436, 418,  432, 416,  432, 416,  432, 416,  404, 444,  404, 418,  430, 418,  430, 418,  430, 420,  432, 416,  432, 418,  430, 418,  380, 438,  436, 416,  432, 418,  404, 444,  430, 418,  408, 1260,  438, 416,  432, 1260,  438, 1262,  436, 1234,  438, 1262,  436, 1260,  436, 1236,  434, 420,  430, 1264,  434, 1264,  436, 1234,  436, 1262,  436, 1260,  438, 1262,  410, 1262,  438, 1258,  410, 1290,  412, 416,  404, 446,  430, 418,  430, 418,  434, 416,  434, 416,  432, 416,  406, 414,  432, 416,  432, 416,  434, 1262,  438, 1262,  408, 418,  430, 420,  430, 1262,  440, 1260,  434, 1236,  436, 1264,  436, 416,  432, 418,  430, 1236,  434, 1262,  408, 444,  432, 416,  402, 442,  436, 1236,  434, 420,  432, 416,  404, 1290,  438, 416,  430, 392,  404, 1290,  406, 1292,  406, 448,  404, 1262,  434, 1264,  434, 418,  404, 1290,  434, 1236,  408, 444,  438, 1256,  434, 1264,  410, 418,  404, 446,  402, 446,  404, 446,  430, 1264,  438, 416,  376, 444,  434, 412,  408, 1290,  434, 1264,  434, 1238,  436, 1262,  408, 446,  404, 1290,  484, 310,  466, 416,  404, 444,  404, 1290,  438, 416,  430, 1236,  436, 1262,  436, 418,  400, 1292,  436, 1234,  410, 1290,  434, 420,  430, 1262,  410, 418,  404, 444,  432, 1262,  436, 416,  404, 446,  432, 390,  432, 418,  430, 418,  432, 416,  432, 416,  432, 418,  432, 1232,  438, 1260,  438, 1260,  438, 1262,  410, 1260,  438, 1262,  438, 1262,  436, 1240,  426, 420,  432, 416,  430, 418,  432, 418,  430, 418,  406, 418,  432, 416,  432, 416,  432, 1260,  438, 1232,  438, 1262,  436, 1262,  436, 1262,  434, 1236,  438, 1260,  434, 1264,  436, 418,  378, 444,  432, 418,  430, 418,  432, 416,  432, 416,  432, 418,  432, 388,  434, 1260,  410, 1290,  408, 1290,  408, 1262,  436, 1262,  436, 1262,  438, 1234,  438, 1260,  436, 416,  404, 446,  432, 416,  430, 390,  432, 418,  404, 446,  430, 418,  430, 418,  404, 1290,  408, 1264,  434, 1264,  436, 1262,  434, 1238,  436, 1262,  406, 1292,  436, 1236,  434, 418,  430, 418,  432, 418,  430, 418,  404, 446,  402, 446,  404, 418,  402, 446,  432, 1262,  434, 1264,  406, 1264,  434, 1262,  434, 1264,  408, 1264,  410, 1288,  458, 1240,  404, 1292,  380, 446,  402, 1292,  406, 448,  402, 446,  400, 1266,  432, 420,  402, 444,  430, 418,  402, 1290,  434, 420,  376, 1290,  436, 1262,  410, 444,  404, 1262,  408, 1290,  406, 1294,  406, 446,  400, 448,  402, 420,  400, 1292,  432, 1264,  484, 1214,  378, 1294,  406, 448,  402, 1292,  408, 1264,  434, 1264,  406, 446,  402, 444,  406, 446,  400, 448,  376, 446,  402, 446,  402, 446,  404, 446,  402, 446,  402, 446,  402, 446,  376, 446,  404, 1288,  436, 1262,  408, 1264,  408, 1290,  432, 1264,  408, 1292,  378, 1294,  406, 1292,  408, 444,  404, 446,  400, 422,  402, 446,  400, 448,  402, 446,  402, 448,  402, 446,  406, 1262,  410, 1288,  408, 1290,  410, 1290,  406, 1264,  408, 1290,  408, 1290,  406, 1266,  406, 446,  402, 446,  402, 446,  402, 446,  402, 444,  378, 446,  402, 446,  402, 444,  404, 1290,  436, 1262,  410, 1262,  434, 1264,  406, 1292,  408, 1264,  408, 1290,  406, 1292,  434, 1236,  408, 1292,  434, 420,  400, 448,  404, 444,  404, 418,  400, 444,  406, 444,  402, 446,  432, 416,  404, 1288,  436, 1236,  410, 1290,  406, 1290,  410, 1262,  408, 1290,  408};  // UNKNOWN B527435E
uint16_t temp_up[595] = {3384, 1712,  438, 1232,  438, 416,  432, 416,  432, 418,  430, 418,  432, 416,  406, 416,  430, 418,  434, 414,  406, 444,  404, 444,  432, 418,  432, 1234,  438, 416,  432, 418,  432, 414,  434, 414,  434, 416,  380, 444,  404, 442,  434, 414,  432, 418,  432, 418,  432, 416,  430, 418,  406, 416,  432, 416,  432, 416,  432, 416,  432, 416,  432, 1234,  436, 418,  434, 1262,  438, 1258,  438, 1234,  438, 1260,  438, 1262,  438, 1234,  436, 416,  432, 1260,  438, 1260,  436, 1234,  438, 1262,  374, 1324,  436, 1262,  410, 1262,  436, 1262,  440, 1258,  438, 390,  434, 444,  404, 414,  406, 444,  432, 416,  432, 416,  434, 416,  404, 418,  432, 418,  432, 416,  434, 1260,  438, 1260,  408, 418,  432, 416,  432, 1262,  436, 1262,  438, 1234,  438, 1262,  434, 418,  432, 416,  430, 1236,  436, 1260,  438, 414,  432, 416,  432, 418,  434, 1234,  436, 416,  432, 420,  432, 1260,  438, 416,  432, 390,  434, 1260,  438, 1260,  438, 414,  432, 1236,  436, 1262,  438, 414,  434, 1260,  438, 1234,  436, 416,  432, 416,  432, 418,  432, 1260,  380, 446,  368, 480,  432, 416,  438, 1256,  438, 416,  432, 1234,  438, 1260,  438, 416,  432, 1260,  434, 1238,  438, 1260,  438, 416,  432, 1262,  382, 444,  432, 418,  404, 1288,  410, 1290,  436, 388,  432, 1260,  438, 1262,  410, 442,  406, 1260,  412, 1286,  410, 444,  402, 446,  402, 1290,  380, 446,  406, 442,  432, 1262,  434, 420,  406, 444,  406, 418,  404, 444,  404, 438,  408, 446,  402, 446,  404, 444,  404, 1262,  410, 1288,  412, 1288,  410, 1288,  378, 1292,  410, 1288,  410, 1290,  434, 1236,  410, 444,  404, 444,  404, 446,  404, 444,  400, 442,  380, 474,  376, 446,  402, 446,  404, 1288,  410, 1262,  410, 1288,  410, 1288,  408, 1290,  410, 1262,  408, 1290,  408, 1288,  410, 444,  376, 446,  404, 446,  404, 444,  402, 448,  402, 446,  404, 446,  404, 418,  404, 1288,  410, 1290,  410, 1288,  376, 1324,  344, 1324,  412, 1288,  410, 1262,  410, 1288,  410, 444,  404, 446,  402, 446,  404, 414,  408, 444,  402, 446,  404, 446,  402, 446,  404, 1290,  378, 1292,  408, 1290,  408, 1290,  408, 1262,  408, 1290,  410, 1288,  410, 1262,  410, 444,  402, 446,  404, 444,  370, 476,  402, 450,  402, 446,  376, 446,  406, 444,  402, 1290,  410, 1288,  410, 1260,  410, 1288,  410, 1288,  410, 1260,  410, 1288,  410, 1288,  410, 1288,  378, 448,  404, 1288,  408, 444,  406, 444,  404, 1262,  410, 444,  404, 444,  404, 444,  404, 1288,  408, 444,  376, 1292,  408, 1288,  410, 444,  404, 1262,  410, 1288,  408, 1288,  408, 442,  408, 444,  404, 418,  404, 1288,  410, 1290,  410, 1288,  382, 1288,  410, 444,  406, 1288,  408, 1264,  408, 1288,  410, 444,  404, 446,  404, 444,  404, 446,  378, 442,  404, 444,  402, 440,  406, 448,  402, 446,  404, 444,  404, 444,  378, 444,  402, 1290,  408, 1290,  410, 1262,  410, 1288,  410, 1288,  410, 1288,  380, 1292,  408, 1288,  410, 444,  402, 446,  402, 420,  402, 446,  404, 446,  402, 446,  402, 446,  402, 446,  402, 1264,  408, 1290,  408, 1292,  406, 1290,  376, 1296,  408, 1290,  408, 1290,  408, 1264,  408, 444,  402, 442,  404, 448,  402, 448,  400, 448,  344, 478,  402, 446,  370, 478,  370, 1322,  406, 1292,  348, 1324,  406, 1290,  406, 1294,  406, 1264,  404, 1294,  384, 1312,  406, 1226,  416, 1324,  376, 478,  370, 478,  370, 478,  370, 452,  370, 478,  370, 478,  370, 478,  370, 478,  370, 1322,  376, 1296,  374, 1322,  376, 1322,  376, 1296,  376, 1322,  374};  // UNKNOWN 531099DC
#endif

volatile int  set_temp;
enum AC_Mode{
  MODE001,
  MODE101,
  MODE100,
  AC_MODE_SIZE
};
uint32_t From_Time[AC_MODE_SIZE];
uint32_t To_Time[AC_MODE_SIZE];
/*************light parameters****************/
enum light_zone
{
  ZONE1,
  ZONE2,
  ZONE3,
  LIGHT_ZONE_SIZE,
};
String default_light_setting_table0[LIGHT_ZONE_SIZE][6];
String default_light_setting_table1[LIGHT_ZONE_SIZE][6];
String default_ac_setting_table[AC_MODE_SIZE][6];

/*
 * Colomn data for default light setting table
 * 0 -zone
 * 1 -color(RGB)
 * 2 -Status
 * 3 -contrast
 * 4 -From time
 * 5 -To time
 */
/**************Timer parameters*******************/
TimerHandle_t SwTimerHandle = NULL;
static void SwTimerHandle_Isr(TimerHandle_t xTimer);

/**********UART2 Parameters*****************/
static const char *TAG = "uart_events";
static QueueHandle_t UART2_queue;
static intr_handle_t handle_console;
uint8_t rxbuf[256];
uint16_t urxlen;
bool uart2_rx_flag;
volatile uint16_t Zigbee_msg_length;
#define NOTASK 0

/********switch parameters********************/
bool sw1_pressed = false, sw2_pressed = false, sw3_pressed = false, sw4_pressed = false;
long lastDebounceTime = 0;  //holds the last time when switch was pressed
long debounceDelay = 10;    // the debounce time (time between switch presses)
int value;
/**************OTAP parameters********************/
/*
 * FTP server =: ipv4 address  of FTP server running PC
 * default port FTP is 21
 */

unsigned int ftp_Port = 21;
char *download_file_path = "/binary/firmware.bin";
bool SPIFF_mounted;
char download_path[100];
char* FTP_server    = NULL;
char* download_url;
/**************Function declaration****************/
static void listNetworks();
static void connect_to_ap(uint8_t *APsssid,uint8_t *APpassword);
static void ble_stop_advertisement();
static void ble_start_advertisement();
static void udp_rx_data();
static void udp_tx_data();
static void setupudp();
static void setup_internal_flash();
static void setup_ble();
static void send_ble_ack_packets(uint8_t Status);
static void Json_post_to_AP(uint16_t tcp_packet_no,uint16_t sensor,uint8_t *Serial_Number,uint8_t *Device_Name);
static void receive_process_hub_data(uint16_t operation,uint16_t sensor);
static void process_ble_data();
static void update_flash(uint16_t address,uint8_t *flash_data,uint16_t len);
static void extract_device_registration_credential( uint16_t sensor);
static void Normal_mode_operation();
static void update_byte_to_flash(uint16_t address,uint8_t working_mode);
static void print_amenos_status();
static void bootup_configuartion();
static void init_MAX7219();
static void init_timer();
static uint32_t get_time();
static void initialize_ble_client();
static void start_scan();
static void transmit_over_tcp();
static void Receive_tcp_data();
static void initialize_zigbee_uart();
int16_t wait_for(const char* data, uint16_t timeout ,bool timout_alone) ;
static void IRAM_ATTR uart_intr_handle(void *arg);
static void sense_button_press();
static void smart_bulb( void *pvParameters );
static void peripheral_sensing(void *pvParameters);
static void kinisi_communication(void *pvParameters) ;
static void pin_configuration();
static void print_uint_data(uint8_t *print_data,uint16_t data_len);
static void displayByte_matrix1(byte character[]);
static void displayByte_matrix2(byte character[]);
static void update_display_content(char temperature[],int digit_count);
static void display_temperature(int temp);
static void send_ble_packets_to_kinisi( uint16_t data_type ,bool status );
static void process_Kinisi_ble_data();
static void scan_i2c_device_and_initialize();
static void i2c_write(byte address,uint16_t data,byte reg);
static int i2c_read(byte address,byte reg,byte read_byte_count);
static void read_temperature();
static void extract_beacon_data();
static void flush_out_Flash();
static void parse_macid(char *data , char *delimiter);
static void configuration_mode_operation();
static void hvac_relay_control(uint16_t Set_AC_Temperature);
static void cummulative_on_minute(uint8_t variant , uint32_t previous_on_time);
static uint8_t *get_hmac_key(uint8_t *Room_Num,uint8_t *Static_Code);
static uint16_t Generate_hmac(uint8_t *key,uint8_t *payload);
static char *delchar(char *data,int no_of_char, int position_to_delete);
static char *get_payload(uint8_t *Json_buffer,uint16_t length);
static void find_char_location(uint8_t *buffer,int length,char ch);
static uint16_t hmac_validation(uint8_t *copy_buffer);
static void update_uint32_t_to_flash(uint16_t address,uint32_t working_mode);
static void update_variant_data(uint8_t variant);
static void print_light_lookup_table(String array1[ROW][COLOMN]);
static void print_default_table( String array1[LIGHT_ZONE_SIZE][6] ,int variant );
static void print_Kinsi_lookup_table( String array1[MAX_KINISI_DEVICE][4] );
static void init_wakeup_timer();
static uint32_t get_ac_mode_change_time(String time_table);
static void maintain_ac_setting();
static void send_ir_ac_command(uint16_t set_temperature);
static void set_Led_B2_ON();
static void set_Led_B2_OFF();
static void set_Buzzer_OFF();
static void set_Buzzer_ON();
static void Relay_ON(byte pin);
static void set_Relays_OFF();
static void set_up_expander();
static void set_Capacitive_Down_Led_OFF();
static void set_Capacitive_Down_Led_ON();
static void io_expander_write_one_byte(int address,uint8_t data,byte reg);
static  uint8_t io_expander_read_one_byte(byte address,byte reg);
static void set_individual_Relays_OFF(byte pin);
static void printError(byte error);
static bool get_lux_value() ;
static void setup_light_sensor() ;
static void DL2_led_control(char pin,char control);
static void DL1_led_control(char pin,char control);
static void set_dot_matrix_intensity();
static void update_dot_matrix_intensity();
static void kinisi_related_process();
static void mount_spiffs();
static bool open_ftp_socket();
static void Download_Flash_image();
static void Get_ftp_details();
static void beep_buzzer(uint16_t buzzer_drive_count,uint16_t beep_delay);
/**********************************************************************
 * @brief  subroutine for gpio34
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
void IRAM_ATTR touch1handleInterrupt() {
   if((long)(micros() - last_micros) >= DEBOUNCING_TIME * 1000)
   {
    portENTER_CRITICAL_ISR(&mux);
    if( json_extract.ac_temp < MAX_TEMPERATURE )
    {
      if( json_extract.ac_temp >= DEFAULT_TEMPERATURE)
      {
        json_extract.ac_temp++;
      }
      else
      {
        json_extract.ac_temp = DEFAULT_TEMPERATURE;
      }
    }
    else
    {
      json_extract.ac_temp = json_extract.ac_temp;
    }
    //input_gpio[TOUCH_1_PRESS_COUNT] = up_button_press_count++;
    input_gpio[TOUCH_1] = true;
    portEXIT_CRITICAL_ISR(&mux);
    last_micros = micros();
  }

}
/**********************************************************************
 * @brief  subroutine for gpio35
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
void IRAM_ATTR touch2handleInterrupt()
{
   if((long)(micros() - last_micros) >= DEBOUNCING_TIME * 1000)
   {
    portENTER_CRITICAL_ISR(&mux);
    if( json_extract.ac_temp > DEFAULT_TEMPERATURE )
    {
      json_extract.ac_temp--;
    }
    else
    {
      json_extract.ac_temp = DEFAULT_TEMPERATURE;
    }
    //input_gpio[TOUCH_2_PRESS_COUNT] = down_button_press_count++;
    input_gpio[TOUCH_2] = true;
    portEXIT_CRITICAL_ISR(&mux);
    last_micros = micros();
  }
}
/**********************************************************************
 * @brief  UART2 interrupt subroutine to ackowledge interrupt
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void IRAM_ATTR uart_intr_handle(void *arg)
{
  uint16_t rx_fifo_len,status;
  uint16_t i=0;

  status = UART2.int_st.val; // read UART interrupt Status
  rx_fifo_len = UART2.status.rxfifo_cnt; // read number of bytes in UART buffer
  while(rx_fifo_len)
  {
   rxbuf[i++] = UART2.fifo.rw_byte; // read all bytes
   rx_fifo_len--;
   uart2_rx_flag = 1;
  }
 // after reading bytes from buffer clear UART interrupt status
 uart_clear_intr_status(ZIGBEE_UART, UART_RXFIFO_FULL_INT_CLR|UART_RXFIFO_TOUT_INT_CLR);

}
/**********************************************************************
 * @brief  Receive data from BLE server
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
//    Serial.print("Notify callback for characteristic ");
//    Serial.println(pBLERemoteCharacteristic->getUUID().toString().c_str());
    int rx_count = 0;
    ble_rx_length = length;
    if( !ble_rx_flag ) //check ble is ready to store received data
    {
      if ( ble_rx_length > 0 )
      {
        Serial.printf("Received data length: %d\n",ble_rx_length);
        //copy data from ble_isr_rx_buffer to ble_rx_buffer
        for(rx_count = 0;rx_count < ble_rx_length ;rx_count++)
        {
          ble_rx_buffer[rx_count] = pData[rx_count];
          Serial.printf("%x\t",ble_rx_buffer[rx_count]);
        }
        Serial.println();
        Serial.printf("BLE Rx print count : %d\n",rx_count);
        if ( ble_rx_length == rx_count )
        {
          ble_rx_flag = true ;
          Serial.println("BLE data received successfully");
        }
      }
    }
}
/**********************************************************************
 * @brief  BLE Client connection status
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    registration_packet_send_count = 0;
    register_kinisi = 0;
    //kinisi_connection_lost = true;
    Serial.println("onDisconnect");
  }
};
/**********************************************************************
 * @brief  used to connect to BLE server with specific name
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());

    pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our Rx service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      Serial.println("Disconnected from Kinisi");
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteRxCharacteristic = pRemoteService->getCharacteristic(charUUID_RX);
    if (pRemoteRxCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID_RX.toString().c_str());
      pClient->disconnect();
      Serial.println("Disconnected from Kinisi");
      return false;
    }
    Serial.println(" - Found our Rx characteristic");

    pRemoteTxCharacteristic = pRemoteService->getCharacteristic(charUUID_TX);
    if (pRemoteRxCharacteristic == nullptr) {
      Serial.print("Failed to find our Tx characteristic UUID: ");
      Serial.println(charUUID_RX.toString().c_str());
      pClient->disconnect();
      Serial.println("Disconnected from Kinisi");
      return false;
    }
    Serial.println(" - Found our Tx characteristic");

    // Read the value of the characteristic.
    if(pRemoteTxCharacteristic->canRead()) {
      std::string value = pRemoteTxCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteTxCharacteristic->canNotify())
      pRemoteTxCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    connected_to_kinisi = false;
    return true;
}
/**********************************************************************
 * @brief  list the available ble devices
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    uint16_t count,indice;
    indice     =  0;
    beac_rxlen =  0;
    count      =  0;

    scan_ble_list[beacon_count] = advertisedDevice.getAddress().toString().c_str();
    beacon_count++;

    if ( !beacon_rx_flag )
    {
//      Serial.print("BLE Advertised Device found: ");
//      Serial.println(advertisedDevice.toString().c_str());
//      Serial.println(advertisedDevice.getAddress().toString().c_str());

      /*********Connecting to Kinisi********/
      String kinisi_adv_name = advertisedDevice.getName().c_str();
      if ( kinisi_adv_name.length() > 1 )
      {
        if (kinisi_adv_name == KINISI_ADV_NAME )
        {
          Serial.print("\nname found :");
          BLEDevice::getScan()->stop();
          myDevice = new BLEAdvertisedDevice(advertisedDevice);
          doConnect = true;
          send_password_to_kinisi = true;
        }
        /******Beacon processing******/
        else if (kinisi_adv_name == KINISI_BEACON_NAME ) // check beacon name is matching
        {

          if( advertisedDevice.haveManufacturerData() )
          {
            //store Adv data in strAdvData string data type
            std::string  strAdvData = advertisedDevice.getManufacturerData();
//            Serial.print("Getting data from :");
            String  ble_mac_id = advertisedDevice.getAddress().toString().c_str();
//            Serial.println(ble_mac_id);
            for(count = 0;count<ble_mac_id.length();count++)
            {
              occupancy_mac_id[count] = ble_mac_id[count];
//              Serial.printf( "%c",occupancy_mac_id[count] );
            }
            occupancy_mac_id[count] = '\0';
            Serial.println();
//            Serial.printf( "stored data : %s\n",strAdvData);
            beac_rxlen = strAdvData.length();
            //make copy of beacon adv data
            if ( !beacon_rx_flag )
            {
              if( beac_rxlen > 0)
              {
                //first 2 bytes are reserved bytes for company.That data is not required
                for(count = 2;count < beac_rxlen;count++)
                {
                  scan_data[count-2] = strAdvData[count];
//                  Serial.printf("%c",scan_data[count-2]);
                }
                scan_data[count]  = '\0';//add delimiter
                for(indice = BYTE5 ;indice < beac_rxlen;indice++)
                {
                  adv_room_no[indice - BYTE5] = scan_data[indice];
//                  Serial.printf("%c",scan_data[indice]);
                }
                adv_room_no[indice - BYTE5] = '\0';
                beac_rxlen = 0;
                beacon_rx_flag    = true;
              }
            }
            else
            {
              Serial.println("\nExtracting Task in Progress");
            }
          }
          else
          {
            Serial.println("No manufacture data");
          }
        }
        else
        {
          Serial.println("Kinisi not found");
        }
      }
    }
  }
};
/**********************************************************************
 * @brief  Class to know the connection status of Client BLE device
 *         It is similar to isr
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
class MyServerCallbacks: public BLEServerCallbacks
{
  void onConnect(BLEServer* pServer)
  {
    ble_connection_status = true;
  }
  void onDisconnect(BLEServer* pServer)
  {
    ble_connection_status = false;
  }
};
/**********************************************************************
 * @brief  Class to receive data from connected BLE device
 *         It is similar to isr
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
class MyCallbacks: public BLECharacteristicCallbacks
{
  void onWrite (BLECharacteristic *pCharacteristic)
  {
    int rx_count = 0;
    std::string ble_isr_rx_buffer = pCharacteristic->getValue();
    //ble_rx_buffer = pCharacteristic->getValue();
    ble_rx_length = ble_isr_rx_buffer.length();
    if( !ble_rx_flag ) //check ble is ready to store received data
    {
      if ( ble_rx_length > 0 )
      {
        Serial.printf("Received data length: %d\n",ble_rx_length);
        //copy data from ble_isr_rx_buffer to ble_rx_buffer
        for(rx_count = 0;rx_count < ble_rx_length ;rx_count++)
        {
          ble_rx_buffer[rx_count] = ble_isr_rx_buffer[rx_count];
          Serial.printf("%x\t",ble_rx_buffer[rx_count]);
        }
        Serial.println();
        Serial.printf("BLE Rx print count : %d\n",rx_count);
        if ( ble_rx_length == rx_count )
        {
          ble_rx_flag = true ;
          Serial.println("BLE data received successfully");
        }
      }
    }
  }
};

/**********************************************************************
 * @brief  Initialization function
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
void setup()
{
  Serial.begin(115200);//for serial print
  device_powered = true;
  operating_mode =  NO_MODE;
  mount_spiffs();
  pin_configuration();
  initialize_zigbee_uart();
  init_MAX7219();
  scan_i2c_device_and_initialize();
  DL1_led_control(LIGHT_DEFAULT,LIGHT_DEFAULT);
  DL2_led_control(LIGHT_DEFAULT,LIGHT_DEFAULT);
  #if APPLICATION
  if (xTaskCreate(
  peripheral_sensing
  ,  "user_interface"   // A name just for humans
  ,  5*MIN_TASK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
  ,  NULL
  ,  TASK_PRIORITY  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
  ,  NULL
  ) != pdPASS)
  {
    Serial.println("peripheral_sensing Task creation failed");
  }
  setup_internal_flash();//initialize flash
//  flush_out_Flash();
  bootup_configuartion();
//  if (xTaskCreate(
//    smart_bulb
//    ,  "smart_bulb"   // A name just for humans
//    ,  MIN_TASK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
//    ,  NULL
//    ,  TASK_PRIORITY  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
//    ,  NULL
//    ) != pdPASS)
//  {
//    Serial.println("smart_bulb Task creation failed");
//  }
//
  // if (xTaskCreate(
  //   kinisi_communication
  //   ,  "kinisi_communication"   // A name just for humans
  //   ,  5*MIN_TASK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
  //   ,  NULL
  //   ,  TASK_PRIORITY  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
  //   ,  NULL
  //   ) != pdPASS)
  // {
  //   Serial.println("kinisi_communication Task creation failed");
  // }
  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.

  #endif
#if DEVELOPMENT
if (xTaskCreate(
    peripheral_sensing
    ,  "user_interface"   // A name just for humans
    ,  5*MIN_TASK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  TASK_PRIORITY  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL
    ) != pdPASS)
    {
      Serial.println("peripheral_sensing Task creation failed");
    }
    operating_mode =  NORMAL_MODE;
    setup_internal_flash();//initialize flash
    device_powered  = false;
    //read over temperature status
    UDP_Rx_Packet[OVER_TEMP] = EEPROM.readUChar(OVER_TEMP_STATUS_BASE_ADDR);
    if( UDP_Rx_Packet[OVER_TEMP] )
    {
      device_powered = false;
      DL2_led_control(LED_R2,LED_DRIVE_LOW);
      DL1_led_control(LED_R1,LED_DRIVE_HIGH); //over temperature error
      init_wakeup_timer();
      read_temperature();
//      room_temperature = 60;
      if( room_temperature >= OVER_TEMPERATURE )
      {
        beep_buzzer(BUZZER_DRIVE_COUNT,BEEP_DELAY);
        Serial.println("Going to sleep now");
        vTaskDelay(100);
        Serial.flush();
        esp_deep_sleep_start();
      }
      else
      {
        Serial.println("Recovered from over tempearture");
        UDP_Rx_Packet[OVER_TEMP] = 0;
        update_byte_to_flash(OVER_TEMP_STATUS_BASE_ADDR,UDP_Rx_Packet[OVER_TEMP]);
      }
    }
    else
    {
      Serial.println("\nOver temperature flag is not set");
    }
    DL1_led_control(LED_R1,LED_DRIVE_LOW); //over temperature error
    WiFi.begin(ssid, passkey);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      vTaskDelay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    setupudp();
    json_extract.fm_path = "ftp://192.168.43.18/binary/firmware.bin";
    json_extract.fm_file_size= "10";
    device_powered = false;
//    UDP_Rx_Packet[OVER_TEMP]  = true;
//    update_byte_to_flash(OVER_TEMP_STATUS_BASE_ADDR,UDP_Rx_Packet[OVER_TEMP]);

    tirisi_ac_type  = HVAC_TYPE;
    json_extract.ac_mode = AUTO_MODE;
//  beacon_rx_flag = 0;
#endif
  Serial.printf("\nFirmware version: %s\n",FIRMWARE_VERSION);

}
/**********************************************************************
 * @brief  main appliacation
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
void loop()
{
#if APPLICATION
    if ( operating_mode == CONFIGURATION_MODE )
    {
      configuration_mode_operation();
    }
    else if ( operating_mode == NORMAL_MODE )
    {
      Normal_mode_operation();
    }
    else if ( operating_mode ==  OTAP_MODE )
    {
      otap_mode_operation();
    }
    else
    {
      Serial.println("Error:check operating mode");
    }

#else if DEVELOPMENT
              otap_mode_operation();
//        Get_ftp_details();

//  if( UDP_Rx_Packet[OVER_TEMP] )
//  {
//    Serial.println("ESP restarting...Over temeperature detected");
//    vTaskDelay(TASK_DELAY);
//    ESP.restart();
//  }
//  udp_tx_data();
//  udp_rx_data();
//  set_Buzzer_ON();
//  set_Buzzer_OFF();

//    DL1_led_control(LED_G1,LED_DRIVE_HIGH);
//    vTaskDelay(5*TASK_DELAY);
//    DL1_led_control(LED_R1,LED_DRIVE_HIGH);
//    vTaskDelay(5*TASK_DELAY);
//    DL1_led_control(LED_B1,LED_DRIVE_HIGH);
//    vTaskDelay(5*TASK_DELAY);
//    DL2_led_control(LED_G2,LED_DRIVE_HIGH);
//    vTaskDelay(5*TASK_DELAY);
//    DL2_led_control(LED_R2,LED_DRIVE_HIGH);
//    vTaskDelay(5*TASK_DELAY);
//    DL2_led_control(LED_B2_BIT1,LED_DRIVE_LOW);
//    vTaskDelay(5*TASK_DELAY);

//    Relay_ON(LOW_SPEED_RELAY_DRIVE_BIT6);
//    vTaskDelay(2000);
//    Relay_ON(MEDIUM_SPEED_RELAY_DRIVE_BIT7);
//    vTaskDelay(2000);
//    Relay_ON(HIGH_SPEED_RELAY_DRIVE_BIT0);
//    vTaskDelay(2000);

//    hvac_relay_control(23);
#endif
    vTaskDelay(TASK_DELAY);  // one tick delay (15ms) in between reads for stability //TASK_DELAY
}
/**********************************************************************
 * @brief  Configure pin as Input/Output
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void pin_configuration()
{
  pinMode(EXPANDER_INT, INPUT);
  pinMode(TEMP_ALERT, INPUT);
  pinMode(CAP_TOUCH_OUT1, INPUT);//34
  attachInterrupt(digitalPinToInterrupt(CAP_TOUCH_OUT1), touch1handleInterrupt, FALLING);
  pinMode(CAP_TOUCH_OUT2, INPUT);//35
  attachInterrupt(digitalPinToInterrupt(CAP_TOUCH_OUT2), touch2handleInterrupt, FALLING);

  pinMode(LED_R1, OUTPUT);
  pinMode(LED_G1, OUTPUT);
  pinMode(LED_R2, OUTPUT);
  pinMode(LED_G2, OUTPUT);
  pinMode(IR_CTRL, OUTPUT);
  pinMode(LED_B1, OUTPUT);
}
/**********************************************************************
 * @brief  Timer task to increment count
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
void SwTimerHandle_Isr( TimerHandle_t xTimer )
{
  Serial.println("Timer");
}
/**********************************************************************
 * @brief  downloading file from FTP server and flash it to ESP32
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void otap_mode_operation()
{
  if( SPIFF_mounted )
  {//Check SPIFFS is mounted successfully
    Get_ftp_details();
    FS_rmFile(download_path); //delete the already existing binary
    if( open_ftp_socket() ) // open FTP socket
    {
      Download_Flash_image(); // download image from FTP server
    }
    else
    {//if socket opens fails try 3 times.
      for (int trial_count = 0;trial_count < 3;trial_count++)
      {
        if( open_ftp_socket() )
        {
          Download_Flash_image();
        }
        if( (trial_count == 2) && !open_ftp_socket() )
        {
          Serial.println("FTP server not found");
        }
        vTaskDelay(100);
      }
    }
  }
}
/**********************************************************************
 * @brief  This function will do following process
 *         1.check for AP connectivity.
 *         2.If connects successfully,send device health status and sensor data to Hub
 *         3.whenever it receives data from Hub,perform action based on received data
 *
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void Normal_mode_operation()
{
  if ( WiFi.status() == WL_CONNECTED )
  {//Do process only when wifi is connected
    if( device_register_status )
    {
      if( !UDP_Rx_Packet[OVER_TEMP] )
      {
        device_powered = false;
        if( low_speed_relay.on || medium_speed_relay.on || high_speed_relay.on )
        {
          DL1_led_control(LED_B1,LED_DRIVE_HIGH);
          //DL2_led_control(LED_B2_BIT1,LED_DRIVE_LOW);
        }
        else
        {
          DL1_led_control(LED_G1,LED_DRIVE_HIGH); // all well after boot
          DL2_led_control(LED_G2,LED_DRIVE_HIGH); //connection with hub
        }
        /*** Read temperature***/
        read_temperature();
        /***At every POR send device registration packet of
            1.Amenos
            2.Kinisi
            3.Light to Hub if successfully registered***/
        if( send_device_reg_packet )
        {
          //send Amenos registration packet
          Json_post_to_AP(DEVICE_REGISTRATION,AMENOS,NULL,NULL);
          Json_post_to_AP(AC_SETTING_PACKET,AMENOS,NULL,NULL);// send AC setting packet
          //send kinisi registration packet
          if ( no_of_kinisi_device >= BYTE1 )
          {
            print_Kinsi_lookup_table(kinisi_lookup_table);
            for (int reg_device = 0; reg_device < no_of_kinisi_device; reg_device++)
            {
              if (  (atoi(kinisi_lookup_table[reg_device][4].c_str()) == true) &&
                    (atoi(kinisi_lookup_table[reg_device][3].c_str()) == false)  )
              {

                kinisi_serial_num_ptr[reg_device]   = (uint8_t *)kinisi_lookup_table[reg_device][1].c_str();
                kinisi_device_name_ptr[reg_device]  = (uint8_t *)kinisi_lookup_table[reg_device][2].c_str();
                Serial.print((char *)kinisi_serial_num_ptr[reg_device]);
                Serial.print((char *)kinisi_device_name_ptr[reg_device]);
                Json_post_to_AP(DEVICE_REGISTRATION,KINISI,kinisi_serial_num_ptr[reg_device],kinisi_device_name_ptr[reg_device]);
              }
            }
          }
          send_device_reg_packet = false;
        }
        /*
         * If device registered to hub successfully, send
         * 1.heart beat(health status) every 1 sec
         * 2.sensor data every 1 min
         * and scan for beacon devices
         */
        current_time = millis();
        if(current_time - last_time >= HEART_BEAT_TIME)
        {
          //every 1 sec once send the data
          Json_post_to_AP(DEVICE_HEALTH_STATUS,AMENOS,NULL,NULL);//send heart beat
          last_time = millis();
        }
        hs_current_time = millis();
        if(hs_current_time - hs_last_time >= SENSOR_DATA_TIME)
        {
          //every 1 min once send the data
          Json_post_to_AP(DEVICE_SENSOR_DATA,AMENOS,NULL,NULL);//send sensor data
          hs_last_time = millis();
        }
        if( !register_kinisi && (!send_device_reg_packet))
        {
          receive_process_hub_data(NORMAL_RECEIVE_MODE,AMENOS);//waiting for receiving data from Hub
        }

        /*
         * 1.Maintain AC settings
         * 2.update total AC run time in flash
         */
        update_variant_data(AC);

        /***Update Dot matrix intensity based on lightt sensor data***/
        update_dot_matrix_intensity();

        /***Do kinisi related process like
         * 1.send device registration packet
         * 2.send health status
         * 3.send event data (Occupancy detection)
         */
         kinisi_related_process();

         /***Control AC based on switch press***/
        if( button_pressed )
        {
          button_pressed = 0;
          if ( (json_extract.ac_temp >= MIN_TEMPERATURE) && (json_extract.ac_temp <= MAX_TEMPERATURE) )
          {
            update_byte_to_flash(SET_AC_TEMP_BASE_ADDR,json_extract.ac_temp);
            if( json_extract.ac_status == DEVICE_TURN_ON )
            {
              if( tirisi_ac_type == HVAC_TYPE )
              {
                hvac_relay_control(json_extract.ac_temp);
              }
              else if(tirisi_ac_type == SPLIT_AC_TYPE )
              {
                send_ir_ac_command(json_extract.ac_temp);
              }
            }
          }
        }

        /***Switch to OTAP mode if OTAP packet is received from Hub***/
        if( UDP_Rx_Packet[OTAP] )
        {
          Json_post_to_AP(OTAP_ACK_PACKET,AMENOS,NULL,NULL);//send OTAP ACK
          operating_mode = OTAP_MODE;
          Serial.println("operating mode changed to OTAP_MODE");
        }

        if( UDP_Rx_Packet[FORCE_SET_PACKET] )
        {
          if ( (json_extract.ac_temp >= MIN_TEMPERATURE) && (json_extract.ac_temp <= MAX_TEMPERATURE) )
          {
            display_temperature(json_extract.ac_temp);
            update_byte_to_flash(SET_AC_TEMP_BASE_ADDR,json_extract.ac_temp);
            if( tirisi_ac_type == HVAC_TYPE )
            {
              hvac_relay_control(json_extract.ac_temp);
            }
            else if(tirisi_ac_type == SPLIT_AC_TYPE )
            {
              send_ir_ac_command(json_extract.ac_temp);
            }
          }
          UDP_Rx_Packet[FORCE_SET_PACKET] = false;
        }
        /*
         * 1.Turn OFF all lights
         * 2.Set AC in night mode setting
         * 3.Set Dot Matrix brightness to minimum level
         */
        if ( UDP_Rx_Packet[AMENITY_SLEEP_MODE] )
        {
          for (int DM_device_count = 0 ; DM_device_count < NUMBER_OF_DEVICES ; DM_device_count++)
          {
            /* Change Dot matrix brightness  */
            lc.setIntensity(DM_device_count,MIN_INTENSITY);
          }
        }

      }
      else if( UDP_Rx_Packet[OVER_TEMP] )
      {
        Serial.println("ESP restarting.....Over temeperature detected");
        vTaskDelay(TASK_DELAY);
        ESP.restart();
      }
    }
  }
  else
  {
    DL2_led_control(LED_R2,LED_DRIVE_HIGH); //connection lost with Hub
    DL1_led_control(LED_G1,LED_DRIVE_LOW);
    Serial.println("Error: WiFi Connection Lost");
    connect_to_ap(ap_ssid,ap_passkey);
  }
}
/**********************************************************************
 * @brief  This function will do following process
 *         1.Advertise BLE at every 30 sec once
 *         2.wait for BLE device to connect
 *         3.If connects it will receive the configuartion parameters and extract it
 *         4.Once extracted store configuration parameters in Flash,then restart
 *         the system for AP connection
 *         5.If device has already received config data and connects to AP send
 *         Device Registration packet to hub wait for get ACK from Hub.
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void configuration_mode_operation()
{
  /*
   * Advertise BLE at every 30 sec once
   */
  device_powered = false;
  if( ble_connection_status )
  {
    if ( configuration_status == CONFIG_FAIL )
    {
      DL1_led_control(LED_G1,TOGGLE);
      DL1_led_control(LED_G2,LED_DRIVE_LOW);
      DL1_led_control(LED_B1,LED_DRIVE_LOW);
    }
    else
    {
      DL1_led_control(LED_B1,LED_DRIVE_HIGH);
      DL2_led_control(LED_B2_BIT1,LED_DRIVE_HIGH);
    }
  }
  else
  {
    DL1_led_control(LED_G1,TOGGLE);
    DL2_led_control(LED_G2,TOGGLE);
  }
  current_time = millis();
  if(current_time - last_time >= BLE_PERIODIC_ADV_TIMEOUT)
  {
    if( !allow_process_ble_data )
    {
      //If UDP socket not opened,every 5 sec once ESP try to open the socket
      Serial.println("Advertise BLE");
      ble_start_advertisement();
      last_time = millis();
    }
  }
  /*
   * If received BLE data store it in respective buffer
   */
  if( ble_rx_flag )
  {
    process_ble_data();
    ble_rx_length = 0;//clear receive length
    ble_rx_flag = false;//clear the flag
  }
  /*
   * If received all config parameters extract required data and store all data in flash
   */
  if( write_data_to_flash )
  {
    write_data_to_flash = false;
    extract_device_registration_credential(AMENOS);//extract ssid,passkey from received data
    //print data length of each parameter that need to be stored in flash
    Serial.printf("\nssid length        = %d\n",Length[WIFI_SSID]);
    Serial.printf("password length    = %d\n",Length[WIFI_PASSWORD]);
    Serial.printf("serial num length  = %d\n",Length[SERIAL_NUM]);
    Serial.printf("org id length      = %d\n",Length[ORG_ID]);
    Serial.printf("facility id length = %d\n",Length[FACILITY_ID]);
    Serial.printf("room num length    = %d\n",Length[ROOM_NUM]);
    Serial.printf("Device name length = %d\n",Length[DEVICE_NAME]);
    Serial.printf("auth code length   = %d\n",Length[SENSOR_AUTH_CODE]);
    //store in flash
    update_flash(SERIAL_NUM_BASE_ADDR,serial_number,Length[SERIAL_NUM]);
    update_flash(AP_SSID_BASE_ADDR,ap_ssid,Length[WIFI_SSID]);
    update_flash(AP_PASS_KEY_BASE_ADDR,ap_passkey,Length[WIFI_PASSWORD]);
    update_flash(AUTH_CODE_BASE_ADDR,amenos_auth_code,Length[SENSOR_AUTH_CODE]);
    update_flash(ORG_ID_BASE_ADDR,org_id,Length[ORG_ID]);
    update_flash(FACILITY_ID_BASE_ADDR,facility_id,Length[FACILITY_ID]);
    update_flash(ROOM_NUM_BASE_ADDR,room_no,Length[ROOM_NUM]);
    update_flash(DEVICE_NAME_BASE_ADDR,device_name,Length[DEVICE_NAME]);

    update_byte_to_flash(AC_TYPE_BASE_ADDR,tirisi_ac_type);
    update_byte_to_flash(AC_BRAND_BASE_ADDR,tirisi_ac_brand);
    update_byte_to_flash(LIGHT_CTRL_BASE_ADDR,light_control);
    update_byte_to_flash(OCCUPANCY_BASE_ADDR,occupancy_detection);
    update_byte_to_flash(DEVICE_TYPE_BASE_ADDR,device_type);
    update_byte_to_flash(MAX_KINISI_COUNT_BASE_ADDR,max_kinisi_in_room);


    if ( configuration_status != CONFIG_FAIL )
    {
      config_data_rcvd = true ;
      //store config_data_rcvd status in flash
      update_byte_to_flash(CONFIG_DATA_STATUS_BASE_ADDR,config_data_rcvd);
      connect_to_ap(ap_ssid,ap_passkey);
      registration_packet_send_count = 0;
    }
    else
    {
      /*serial number,ssid,password failes to store in flash
       *operate in CONFIGURATION_MODE
       */
      operating_mode = CONFIGURATION_MODE ;
      //store operating mode in flash
      update_byte_to_flash(OPERATING_MODE_BASE_ADDR,operating_mode);
      print_amenos_status();
    }
  }
  if(config_data_rcvd)
  {
    if ( WiFi.status() == WL_CONNECTED )
    {//Do process only when wifi is connected
      //vTaskDelay(100);
      get_time();
      if(!device_register_status)
      {
        if( registration_packet_send_count < REG_PACKET_MAX_TRIAL_COUNT)
        {
          //if device is not registered send device registration packet
          Json_post_to_AP(DEVICE_REGISTRATION,AMENOS,NULL,NULL);
          registration_packet_send_count++;
        }
        else
        {
          Serial.println("Not get response from hub\nClearing all Tirisi configuration parameters");
          config_data_rcvd = false ;
          //store config_data_rcvd status in flash
          update_byte_to_flash(CONFIG_DATA_STATUS_BASE_ADDR,config_data_rcvd);
          //clear all buffer
          for ( int indice = 0;indice < BLE_BUFFER_SIZE;indice++ )
          {
             room_details[indice]  = '\0';
             serial_number[indice] = '\0';
             device_name[indice]   = '\0';
             if ( indice <= BLE_BUFFER_SIZE/2 )
             {
                amenos_auth_code[indice]  = '\0' ;
             }
          }
          flush_out_Flash();
          print_amenos_status();
        }
      }
    }
  }
  //vTaskDelay(500);
}
/**********************************************************************
 * @brief  This task is to process zigbee and beacon functinality
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void smart_bulb(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  for (;;) // A Task shall never return or exit.
  {
    /*
     * At startup send AT command 3 times to check whether Zigbee module is working fine
     */
    for(int cmd_cnt = 0;cmd_cnt < 3 ;cmd_cnt++)
    {
      if ( !device_available )
      {
        uart_write_bytes(ZIGBEE_UART,AT,strlen(AT)); //AT
        if ( wait_for(OK_RESPONSE,5000,0) == EXIT_SUCCESS )
        {
          device_available = true ;
          Serial.println("Get response for AT Command");
          break;
        }
        else
        {
          device_available = false ;
        }
      }
    }
    /*
     * If device_available status set,send further AT commands
     */
    if ( device_available )
    {
      if( UDP_Rx_Packet[LIGHT_CONFIG] )
      {
        uart_write_bytes(ZIGBEE_UART,CONFIG_LIGHT,strlen(CONFIG_LIGHT)); //Send light configuration command
        if ( wait_for(OK_RESPONSE,5000,0) == EXIT_SUCCESS )
        {
          Serial.println("Get response for AT+NODE Command");
        }
        UDP_Rx_Packet[LIGHT_CONFIG] = false;
      }
      if( UDP_Rx_Packet[LIGHT_CTRL] )
      {
        uart_write_bytes(ZIGBEE_UART,CTRL_LIGHT,strlen(CTRL_LIGHT)); //Send zone light control command
        if ( wait_for(OK_RESPONSE,5000,0) == EXIT_SUCCESS )
        {
          Serial.println("Get response for AT+ZLC Command");
        }
        UDP_Rx_Packet[LIGHT_CTRL] = false;
      }
      if( UDP_Rx_Packet[BLE_MESH] )
      {
        uart_write_bytes(ZIGBEE_UART,ENABLE_MESH,strlen(ENABLE_MESH)); //Send Enbale BLE mesh command
        if ( wait_for(OK_RESPONSE,5000,0) == EXIT_SUCCESS )
        {
          Serial.println("Get response for AT+ZLC Command");
        }
        UDP_Rx_Packet[BLE_MESH] = false;
      }
      if( UDP_Rx_Packet[LIGHT_DECOMMISSION] )
      {
        uart_write_bytes(ZIGBEE_UART,DECOMMISSION_LIGHT,strlen(DECOMMISSION_LIGHT)); //Send light decommission command
        if ( wait_for(OK_RESPONSE,5000,0) == EXIT_SUCCESS )
        {
          Serial.println("Get response for AT+LDCOM Command");
        }
        UDP_Rx_Packet[LIGHT_DECOMMISSION] = false;
      }
    }
    else
    {
      Serial.println("\nError:Please check Zigbee module is available");
    }
    vTaskDelay(TASK_DELAY);  // one tick delay (15ms) in between reads for stability
  }
}

/**********************************************************************
 * @brief  This task is for sensing peripherals
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void peripheral_sensing(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  for (;;) // A Task shall never return or exit.
  {
    if ( operating_mode == NORMAL_MODE )
    {
      sense_button_press();
    }
    if ( operating_mode == CONFIGURATION_MODE )
    {
      if ( device_powered )
      {
        DL1_led_control(LED_R1,TOGGLE);
        DL2_led_control(LED_R2,TOGGLE);
      }
    }
    vTaskDelay(TASK_DELAY);  // one tick delay (15ms) in between reads for stability
  }
}

/**********************************************************************
 * @brief  This task is for kinisi communication
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void kinisi_communication(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  for (;;) // A Task shall never return or exit.
  {
//    Serial.println("Kinisi communication");
    /*
     * Amenos - Kinisi commuincation begins only when Amenos operates in Normal Mode
     */
    if ( operating_mode == NORMAL_MODE )
    {
        // If the flag "doConnect" is true then we have scanned for and found the desired
        // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
        // connected we set the connected flag to be true.
        if (doConnect == true)
        {
          if (connectToServer())
          {
            Serial.println("We are now connected to the BLE Server.");
          }
          else
          {
            Serial.println("We have failed to connect to the server; there is nothin more we will do.");
            DL2_led_control(LED_B2_BIT1,LED_DRIVE_HIGH);
            Serial.println("kinisi connection lost");
          }
          doConnect = false;
        }

        // If we are connected to a peer BLE Server, send data to it
        if (connected)
        {
          connected_to_kinisi = true;
          if( send_password_to_kinisi )
          {
            send_ble_packets_to_kinisi(PASSWORD,0);
          }
        }
        else
        {
          /***scan for kinisi***/
          if( !beacon_rx_flag )
          {
            if ( (registration_packet_send_count == 0) && (register_kinisi == 0) && (send_device_reg_packet == 0) )
            {
              Current_Time[KINISI_SCAN] = millis();
              if(Current_Time[KINISI_SCAN] - Last_Time[KINISI_SCAN] >= (2*HEART_BEAT_TIME))
              {
                //every 2 sec once scan
                beacon_count = 0;
//                Serial.println("BLE scanning......");
                start_scan();
                Last_Time[KINISI_SCAN] = millis();
//                for(int i = 0;i < beacon_count ; i++)
//                {
//                  Serial.println(scan_ble_list[i]);
//                }
              }
            }
          }
        }
        if ( beacon_rx_flag )
        {
          extract_beacon_data();
          if( !occupancy_detected )
          {
            beacon_rx_flag = 0;
          }
        }


        if( ble_rx_flag )
        {
          process_Kinisi_ble_data();
          ble_rx_length = 0;//clear receive length
          ble_rx_flag = false;//clear the flag
        }
        if( write_data_to_flash )
        {
          write_data_to_flash = false;
          extract_device_registration_credential(KINISI);//extract device registration details
        }
    }
    vTaskDelay(TASK_DELAY);  // one tick delay (15ms) in between reads for stability
  }
}

/**********************************************************************
 * @brief  Send kinisi related packets to Hub
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void kinisi_related_process()
{
  uint8_t mac_id_found  = 0;
  char *Ser_num         = NULL;
  /*
   * Registre kinisi to Hub
   */
  if ( register_kinisi )
  {
    if( atoi(kinisi_lookup_table[kinisi_reg_location][4].c_str()) == false )
    {
      if ( registration_packet_send_count < REG_PACKET_MAX_TRIAL_COUNT )
      {
        registration_packet_send_count++;
        Serial.println("Registration packet trial count = " + String(registration_packet_send_count) );
        get_time();
        Json_post_to_AP(DEVICE_REGISTRATION,KINISI,kinisi_serial_num_ptr[kinisi_reg_location],kinisi_device_name_ptr[kinisi_reg_location]);
      }
      else
      {
        Serial.println("Kinisi not registered after 3 trial.....");
        registration_packet_send_count = 0;
        register_kinisi = 0;
        send_ble_packets_to_kinisi(DEVICE_REG_ACK,EXIT_FAILURE);//send NACK to Kinisi
        if( !kinisi_register_status )
        {
          //update kinisi count
          if( no_of_kinisi_device >= BYTE1)
          {
            no_of_kinisi_device -= 1;
            update_byte_to_flash(KINISI_DEVICE_COUNT,no_of_kinisi_device);
            Serial.printf("\nno of kinsi device available in this room = %d\n",no_of_kinisi_device);
          }
          pClient->disconnect();
          Serial.println("Disconnected from Kinisi");
          vTaskDelay(50);
          //store kinisi registration status in flash
//              update_byte_to_flash(KINISI_REG_STS_BASE_ADDR,kinisi_register_status);
          update_byte_to_flash((KINISI_REG_ADDR + no_of_kinisi_device),kinisi_register_status);

          kinisi_lookup_table[kinisi_reg_location][4] = itoa(kinisi_register_status,str_buffer,DECIMAL); //update kinisi registration status
          print_Kinsi_lookup_table(kinisi_lookup_table);

          //clear all buffer
          for ( int indice = 0;indice < BLE_BUFFER_SIZE;indice++ )
          {
             kinisi_room_details[indice]  = '\0';
             kinisi_serial_number[indice] = '\0';
             kinisi_device_name[indice]   = '\0';
             if ( indice <= BLE_BUFFER_SIZE/2 )
             {
                kinisi_auth_code[indice]  = '\0' ;
             }
          }
        }
      }
    }
    else
    {
      Serial.printf("searching location = %d",kinisi_reg_location);
      Serial.println("Registration status is set" + String(atoi(kinisi_lookup_table[kinisi_reg_location][4].c_str())));
    }
  }
  Current_Time[KINISI_HS] = millis();
  if(Current_Time[KINISI_HS] - Last_Time[KINISI_HS] >= HEART_BEAT_TIME)
  {
    //every 1 sec once send the data
    if ( no_of_kinisi_device >= BYTE1 )
    {
//      Serial.printf("beacon_count = %d\n",beacon_count);
      for( int list = 0;list < beacon_count ; list++ )
      {
        for (int reg_device = 0; reg_device < no_of_kinisi_device; reg_device++)
        {
          if ( scan_ble_list[list] == kinisi_lookup_table[reg_device][5] )
          {
            kinisi_serial_num_ptr[reg_device] = NULL;
            if (  (atoi(kinisi_lookup_table[reg_device][4].c_str()) == true) &&
                  (atoi(kinisi_lookup_table[reg_device][3].c_str()) == false)  )
            {
//              Serial.printf(" mactch found @ %d\n",reg_device);
              kinisi_serial_num_ptr[reg_device]   = (uint8_t *)kinisi_lookup_table[reg_device][1].c_str();
//              Serial.println((char *)kinisi_serial_num_ptr[reg_device]);
              //send heart beat
              Json_post_to_AP(DEVICE_HEALTH_STATUS,KINISI,kinisi_serial_num_ptr[reg_device],NULL);
            }
          }
        }
      }
    }
    Last_Time[KINISI_HS] = millis();
  }

  if( occupancy_detected )
  {
    //when ever occupancy is detected we need to send sensor data
//      print_Kinsi_lookup_table(kinisi_lookup_table);
      for (int i = 0; i < no_of_kinisi_device; i++)
      {
        if( kinisi_lookup_table[i][5]  == (String)(char *)occupancy_mac_id)
        {
          Serial.printf("match found @ %d\n",i);
          mac_id_found  = true;
          Ser_num       = (char *)kinisi_lookup_table[i][1].c_str();
          break;
        }
      }
      if( mac_id_found )
      {
        Json_post_to_AP(DEVICE_SENSOR_DATA,KINISI,(uint8_t *)Ser_num,NULL);//send event data
      }
      occupancy_detected  = 0;
      beacon_rx_flag      = 0;
  }
}
/**********************************************************************
 * @brief  Increment/Decrement AC Temperature
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void sense_button_press()
{
  if ( input_gpio[TOUCH_1] )
  {
    button_pressed      = 1;
    set_Capacitive_Up_Led_ON();
    vTaskDelay(TOUCH_BUTTON_LED_TIME);
    set_Capacitive_Up_Led_OFF();
    display_temperature(json_extract.ac_temp);
    Serial.printf("set temperature = %d\n",json_extract.ac_temp);
    input_gpio[TOUCH_1] = false;
    input_gpio[TOUCH_2] = false;
    #if HITACHI
      irsend.sendRaw(temp_up, 595, 38);  // Send a raw data capture at 38kHz.
    #endif
  }
  else if ( input_gpio[TOUCH_2] )
  {
    button_pressed      = 1;
    set_Capacitive_Down_Led_ON();
    vTaskDelay(TOUCH_BUTTON_LED_TIME);
    set_Capacitive_Down_Led_OFF();
    display_temperature(json_extract.ac_temp);
    Serial.printf("set temperature = %d\n",json_extract.ac_temp);
    input_gpio[TOUCH_1] = false;
    input_gpio[TOUCH_2] = false;
    #if HITACHI
      irsend.sendRaw(temp_down, 595, 38);  // Send a raw data capture at 38kHz.
    #endif
  }
}
/**********************************************************************
 * @brief  Intialize Zigbee uart @115200
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void initialize_zigbee_uart()
{
  int ret;
  esp_log_level_set(TAG, ESP_LOG_INFO);

  /* Configure parameters of an UART driver,
  * communication pins and install the driver */
  uart_config_t uart_config = {
    .baud_rate = 19200,//zigbee modules default baudrate
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };

  ESP_ERROR_CHECK(uart_param_config(ZIGBEE_UART, &uart_config));

  //Set UART log level
  esp_log_level_set(TAG, ESP_LOG_INFO);

  //Set UART pins (using UART2 default pins ie no changes.)
  ESP_ERROR_CHECK(uart_set_pin(ZIGBEE_UART, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
//  ESP_ERROR_CHECK(uart_set_pin(ZIGBEE_UART, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

  //Install UART driver, and get the queue.
  ESP_ERROR_CHECK(uart_driver_install(ZIGBEE_UART, BUF_SIZE * 2, 0, 0, NULL, 0));

  // release the pre registered UART handler/subroutine
  ESP_ERROR_CHECK(uart_isr_free(ZIGBEE_UART));

  // register new UART subroutine
  ESP_ERROR_CHECK(uart_isr_register(ZIGBEE_UART,uart_intr_handle, NULL, ESP_INTR_FLAG_IRAM, &handle_console));

  // enable RX interrupt
  ESP_ERROR_CHECK(uart_enable_rx_intr(ZIGBEE_UART));
}

/**********************************************************************
 * @brief  Extract ssid,passkey from array wifi_details
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void extract_device_registration_credential( uint16_t sensor)
{
  uint16_t ssid_byte_count=0,location = 0;
  uint8_t already_reagistered = 0;
  for( uint16_t misc_count = 0;misc_count < 15;misc_count++)
  {//clear buffer
    ap_passkey[misc_count] = '\0';
  }
  if( sensor == AMENOS )
  {
    /***********Get device type**************/
    device_type = device_name[11];
    if( device_type == AMENOS )
    {
      for(  location = 0;location < 12;location++)
      {
        /*********Get Org ID/Propert Name ***********/
        if( location < 3 )
        {
          if( room_details[location] != '#' )
          {
            org_id[location]= room_details[location];//CHARACTER_OFFSET
          }
          else
          {
            org_id[location] = '\0';// add delimiter
          }
          Serial.printf("org_id %d : %c\n",location,org_id[location]);
        }
        /*********Get Facility ID**************/
        else if( (location >= 3) && (location < 6))
        {
          if( location == 3)
          {
            org_id[location] = '\0';// add delimiter
          }
          if( room_details[location] != '#' )
          {
            facility_id[location-3]= room_details[location];
          }
          else
          {
            facility_id[location-3] = '\0';// add delimiter
          }
          Serial.printf("facility_id %d : %c\n",(location-3),facility_id[location-3]);
        }
        /**********Get Room No***************/
        else if( (location >= 6) && (location < 12))
        {
          if( location == 6)
          {
            facility_id[location-3] = '\0';// add delimiter
          }
          if( room_details[location] != '#' )
          {
            room_no[location-6]= room_details[location];
          }
          else
          {
            room_no[location-6] = '\0';// add delimiter
          }
          Serial.printf("room no %d : %c\n",(location-6),room_no[location-6]);
        }
      }
      room_no[location-6] = '\0';// add delimiter

      Length[ORG_ID]            = strlen((char*)org_id);//store array length of pass key
      Length[FACILITY_ID]       = strlen((char*)facility_id);//store array length of pass key
      Length[ROOM_NUM]          = strlen((char*)room_no);//store array length of pass key
      Length[SENSOR_AUTH_CODE]  = strlen((char*)amenos_auth_code);//store array length of auth code

      /*************Get AC mode*************/
      tirisi_ac_type      = room_details[BYTE12];
      /************Get Ac Brand***********/
      tirisi_ac_brand     = room_details[BYTE13];
      /************Get status of zigbee control,occupancy******/
      light_control       = room_details[BYTE14];
      occupancy_detection = room_details[BYTE15];

      if ( tirisi_ac_type == HVAC_TYPE )
      {
        device_variant = 0x5000;
        if ( light_control == ENABLE  )
        {
          device_variant = 0; //need to check
        }
        if ( occupancy_detection == ENABLE  )
        {
          device_variant = 0x5010;
        }
        if( (light_control == ENABLE) && (occupancy_detection == ENABLE) )
        {
          device_variant = 0x5011;
        }
      }
      else if ( tirisi_ac_type == SPLIT_AC_TYPE )
      {
        device_variant = 0x5100;
        if ( light_control == ENABLE  )
        {
          device_variant = 0; //need to check
        }
        if ( occupancy_detection == ENABLE  )
        {
          device_variant = 0x5110;
        }
        if( (light_control == ENABLE) && (occupancy_detection == ENABLE) )
        {
          device_variant = 0x5111;
        }
      }
      max_kinisi_in_room  = device_name[12];

      Serial.printf("No. of kinisi in room = %d\n",max_kinisi_in_room );
      Serial.printf("device variant   = %x\n",device_variant);
      device_name[11] = '\0'; //add delimiter
      Length[DEVICE_NAME] = strlen((char*)device_name);//store array length of pass key
      Serial.println("\nExtracting Ssid,Passkey");
       /*
       * Room no and ssid is same.so store room_no buffer data in ap_ssid buffer
       */
      for( ssid_byte_count = 0 ;ssid_byte_count < Length[ROOM_NUM] ; ssid_byte_count++ )
      {
        ap_ssid[ssid_byte_count] = room_no[ssid_byte_count];
        Serial.printf("ap_ssid %d : %c\n",ssid_byte_count,ap_ssid[ssid_byte_count]);
      }
      ap_ssid[ssid_byte_count] = '\0';//add terminating character
      Length[WIFI_SSID] = strlen((char*)ap_ssid);//store array length of ssid
      /*
       * AP password logic:
       * First character - '#'
       * Then pad        - <room_no> and <org_id>
       * Last character  - '#'
       * e.g. if ssid     = "010101"
       *         password = "#<org_id><room_no>#"
       */
      strcat((char*)ap_passkey, (const char*)room_no); //pad room_no
      strcat((char*)ap_passkey, (const char*)org_id); //pad org_id
      Serial.printf("AP Password : %s\n", ap_passkey);
      Serial.printf("AP password length: %d\n", strlen((const char*)ap_passkey));
      strcpy((char*)copy_ap_passkey,(const char*)ap_passkey);
      memset(ap_passkey, 0, ((BLE_BUFFER_SIZE/2)*sizeof(ap_passkey[BYTE0])));
      ap_passkey[BYTE0] = '#';
      for( uint16_t misc_count = 0 ;misc_count < strlen((const char*)copy_ap_passkey);misc_count++)
      {
        ap_passkey[misc_count+1] = copy_ap_passkey[misc_count] ;
      }
      ap_passkey[strlen((const char*)copy_ap_passkey) + BYTE1] = '#';
      ap_passkey[strlen((const char*)copy_ap_passkey) + BYTE2] = '\0';
      Serial.printf("password: %s\n", ap_passkey);
      Serial.printf("password length: %d\n", strlen((const char*)ap_passkey));
      Length[WIFI_PASSWORD] = strlen((char*)ap_passkey);//store array length of pass key

      Serial.print("Org id: ");
      print_uint_data(org_id,Length[ORG_ID]);
      Serial.print("\nFacility id:");
      print_uint_data(facility_id,Length[FACILITY_ID]);
      Serial.print("\nRoom no:");
      print_uint_data(room_no,Length[ROOM_NUM]);
      Serial.print("\nDevice name:");
      print_uint_data(device_name,Length[DEVICE_NAME]);
      Serial.print("\nssid:");
      print_uint_data(ap_ssid,Length[WIFI_SSID]);
      Serial.print("\npassword:");
      print_uint_data(ap_passkey,Length[WIFI_PASSWORD]);
    }
    else
    {
      Serial.printf("device type mis-match :  %d\n",device_type);
      Serial.println("Expected value 5");
    }
  }
  else if ( sensor == KINISI )
  {
    /***********Get device type**************/
    kinisi_device_type    = kinisi_device_name[11];
    /**********Get Kinisi manufacture date******/
    kinisi_mfg[DATE]      = kinisi_device_name[12];
    kinisi_mfg[MONTH]     = kinisi_device_name[13];
    kinisi_mfg[YEAR_MSB]  = kinisi_device_name[14];
    kinisi_mfg[YEAR_LSB]  = kinisi_device_name[15];
    kinisi_mfg[MFG_SIZE]  = '\0';
    Serial.println("\nKinisi mfg date :" + String(kinisi_mfg[DATE])  + "-" + String(kinisi_mfg[MONTH])  + "-" + String(kinisi_mfg[YEAR_MSB]) + String(kinisi_mfg[YEAR_LSB]));
    if( kinisi_device_type == KINISI)
    {
      /*
       * extract only when device type matches sensor
       */
      kinisi_device_name[11] = '\0'; //add delimiter
      Length[DEVICE_NAME] = strlen((char*)kinisi_device_name);//store array length of pass key

      for(  location = 0;location < 12;location++)
      {
        /*********Get Org ID/Propert Name ***********/
        if( location < 3 )
        {
          if( kinisi_room_details[location] != '#' )
          {
            kinisi_org_id[location]= kinisi_room_details[location];
          }
          else
          {
            kinisi_org_id[location] = '\0';// add delimiter
          }
          Serial.printf("kinisi_org_id %d : %c\n",location,kinisi_org_id[location]);
        }
        /*********Get Facility ID**************/
        else if( (location >= 3) && (location < 6))
        {
          if( location == 3)
          {
            kinisi_org_id[location] = '\0';// add delimiter
          }
          if( kinisi_room_details[location] != '#' )
          {
            kinisi_Facility_id[location-3]= kinisi_room_details[location];
          }
          else
          {
            kinisi_Facility_id[location-3] = '\0';// add delimiter
          }
          Serial.printf("kinisi_Facility_id %d : %c\n",(location-3),kinisi_Facility_id[location-3]);
        }
        /**********Get Room No***************/
        else if( (location >= 6) && (location < 12))
        {
          if( location == 6)
          {
            kinisi_Facility_id[location-3] = '\0';// add delimiter
          }
          if( kinisi_room_details[location] != '#' )
          {
            kinisi_Amenity_id[location-6]= kinisi_room_details[location];
          }
          else
          {
            kinisi_Amenity_id[location-6] = '\0';// add delimiter
          }
          Serial.printf("room no %d : %c\n",(location-6),kinisi_Amenity_id[location-6]);
        }
      }
      kinisi_Amenity_id[location-6] = '\0';// add delimiter

      Length[ORG_ID] = strlen((char*)kinisi_org_id);//store array length of pass key
      Length[FACILITY_ID] = strlen((char*)kinisi_Facility_id);//store array length of pass key
      Length[ROOM_NUM] = strlen((char*)kinisi_Amenity_id);//store array length of pass key
      //check whether Amenos and kinisi are in same room
      if ( strcmp((const char*)room_no,(const char*)kinisi_Amenity_id) == 0 )
      {
        //allow kinisi to register to Hub
//        if ( !kinisi_register_status )
//        {
          no_of_kinisi_device = EEPROM.readUChar(KINISI_DEVICE_COUNT);
          Serial.printf(" no of kinisi = %d\n",no_of_kinisi_device);
          if( no_of_kinisi_device < max_kinisi_in_room )
          {
            register_kinisi                   = true;
            registration_packet_send_count    = 0;

            device_count[KINISI_SERIAL_NUM]   = no_of_kinisi_device;
            device_count[KINISI_DEVICE_NAME]  = no_of_kinisi_device;

            kinisi_serial_num_ptr[device_count[KINISI_SERIAL_NUM]++]     = kinisi_serial_number;
            kinisi_device_name_ptr[device_count[KINISI_DEVICE_NAME]++]   = kinisi_device_name;


            if ( device_count[KINISI_SERIAL_NUM] - device_count[KINISI_DEVICE_NAME] == 0)
            {
              Serial.println("checking it is same serial # or not");
              if ( no_of_kinisi_device >= BYTE1)
              {
                for (int i = 0; i < no_of_kinisi_device; i++)
                {
                  if( kinisi_lookup_table[i][1]  == (String)(char *)kinisi_serial_number)
                  {
                    Serial.printf("match found @ %d\n",i);
                    kinisi_reg_location                                       = i;
                    kinisi_lookup_table[kinisi_reg_location][4]               = itoa(false,str_buffer,DECIMAL);
                    Serial.printf("updating in location : %d\n",i);
                    already_reagistered                                       = true;
                    kinisi_serial_num_ptr[device_count[KINISI_SERIAL_NUM]]    = NULL;
                    kinisi_device_name_ptr[device_count[KINISI_DEVICE_NAME]]  = NULL;
                    kinisi_serial_num_ptr[kinisi_reg_location]                = kinisi_serial_number;
                    kinisi_device_name_ptr[kinisi_reg_location]               = kinisi_device_name;
                    kinisi_lookup_table[kinisi_reg_location][5]               = myDevice->getAddress().toString().c_str();
                    kinisi_mac_id                                             = myDevice->getAddress().toString().c_str();
                    Serial.println(kinisi_lookup_table[kinisi_reg_location][5]);

//                    update_flash( ( KINISI_SERIAL_MACID_BASE_ADDR + ( ( no_of_kinisi_device - 1) * BYTE17) ),(uint8_t *)kinisi_lookup_table[kinisi_reg_location][5].c_str(),BYTE17 );

                    break;
                  }
                }
              }

              if( !already_reagistered )
              {
                no_of_kinisi_device = device_count[KINISI_SERIAL_NUM];
                kinisi_reg_location = no_of_kinisi_device - 1;

                kinisi_lookup_table[kinisi_reg_location][1] = (char *)kinisi_serial_number;//kinisi_serial_num_ptr[device_count[KINISI_SERIAL_NUM]];
                kinisi_lookup_table[kinisi_reg_location][2] = (char *)kinisi_device_name;//kinisi_device_name_ptr[device_count[KINISI_DEVICE_NAME]] ;

                kinisi_lookup_table[kinisi_reg_location][4] = itoa(kinisi_register_status,str_buffer,DECIMAL); //update kinisi registration status

                kinisi_lookup_table[kinisi_reg_location][5]               = myDevice->getAddress().toString().c_str();
                kinisi_mac_id                                             = myDevice->getAddress().toString().c_str();
                Serial.println(kinisi_mac_id);

                Serial.println(myDevice->getAddress().toString().c_str());
                Serial.println(kinisi_lookup_table[kinisi_reg_location][5]);
                Serial.println(kinisi_mac_id);

                Serial.println("kinisi location" + String(kinisi_reg_location));

                Serial.println("list of kinisi serial #:");
                for (int i = 0; i < no_of_kinisi_device; i++)
                {
                  kinisi_lookup_table[i][0] = itoa(i+1,str_buffer,DECIMAL);
                  Serial.print(kinisi_lookup_table[i][1]);
                  Serial.print("\t");
                  Serial.println(kinisi_lookup_table[i][1].length());
                }
                Serial.println("list of kinisi Device name:");
                for (int i = 0; i < no_of_kinisi_device; i++)
                {
                  Serial.print(kinisi_lookup_table[i][2]);
                  Serial.print("\t");
                  Serial.println(kinisi_lookup_table[i][2].length());
                }

                update_flash( ( KINISI_DEVICE_NAME_BASE_ADDR + ( ( no_of_kinisi_device - 1) * BYTE11) ),kinisi_device_name,strlen((char *)kinisi_device_name) );
                update_flash( ( KINISI_SERIAL_NUM_BASE_ADDR + ( ( no_of_kinisi_device - 1) * BYTE16) ),kinisi_serial_number,strlen((char *)kinisi_serial_number) );
                update_flash( ( KINISI_SERIAL_MACID_BASE_ADDR + ( ( no_of_kinisi_device - 1) * BYTE17) ),(uint8_t *)kinisi_lookup_table[kinisi_reg_location][5].c_str(),BYTE17 );
              }
              else
              {
                Serial.println("match found");
              }
            }
            print_Kinsi_lookup_table(kinisi_lookup_table);

          }
          else
          {
            if ( no_of_kinisi_device >= BYTE1)
            {
              for (int i = 0; i < no_of_kinisi_device; i++)
              {
                if( kinisi_lookup_table[i][1]  == (String)(char *)kinisi_serial_number)
                {
                  Serial.printf("match found @ %d\n",i);
                  kinisi_lookup_table[i][4] = itoa(false,str_buffer,DECIMAL);
                  kinisi_reg_location       = i;
                  Serial.printf("updating in location : %d\n",i);
                  already_reagistered       = true;
                  no_of_kinisi_device -=  1;
                  kinisi_serial_num_ptr[kinisi_reg_location]  = kinisi_serial_number;
                  kinisi_device_name_ptr[kinisi_reg_location] = kinisi_device_name;
                  kinisi_lookup_table[kinisi_reg_location][4] = itoa(kinisi_register_status,str_buffer,DECIMAL); //update kinisi registration status
                  register_kinisi                   = true;
                  registration_packet_send_count    = 0;
                  print_Kinsi_lookup_table(kinisi_lookup_table);
                  break;
                }
              }
            }
            if( !already_reagistered  )
            {
              Serial.printf("\nMax kinisi device allowed is %d\n", max_kinisi_in_room );
              Serial.println("Disconnect kinisi");
              pClient->disconnect();
              vTaskDelay(50);
            }
            else
            {
              Serial.println("match found");
            }
          }

          Serial.println("Allow to register Kinisi");
//        }
      }
      else
      {
        Serial.println("\nNot a same room number\n disconnect kinisi");
        pClient->disconnect();
        vTaskDelay(50);
      }
    }
    else
    {
      Serial.printf("Error: %d\n",kinisi_device_type);
      Serial.println("Sensor mis-matches\n Disconnect kinisi");
      pClient->disconnect();
      vTaskDelay(50);
    }

    Serial.print("Kinisi Org id: ");
    print_uint_data(kinisi_org_id,Length[ORG_ID]);
    Serial.print("\nKinisi Facility id:");
    print_uint_data(kinisi_Facility_id,Length[FACILITY_ID]);
    Serial.print("\nKinisi Room no:");
    print_uint_data(kinisi_Amenity_id,Length[ROOM_NUM]);
    Serial.print("\nKinisi Device name:");
    print_uint_data(kinisi_device_name,Length[DEVICE_NAME]);
  }
  else
  {

  }
}
/**********************************************************************
 * @brief  store the data in flash
 *
 * @param  address    - location of data need to be verify
 *         flash_data - array pointer which holds the data need to store/verify
 *         len        - length of data to be store in flash
 *
 * @return none
 *************************************************************************/
static void update_flash(uint16_t address,uint8_t *flash_data,uint16_t len)
{
  uint16_t verify_count = 0;
  uint8_t temp_write_buffer[len] = {'\0'};
  uint8_t temp_read_buffer[len] = {'\0'};
  //store data in temp buffer
  Serial.printf("write data len : %d\n",len);
  Serial.print("write data :");
  for (int misc_count = 0 ; misc_count < len ; misc_count++)
  {
    temp_write_buffer[misc_count] = flash_data[misc_count];
    Serial.printf("%c",temp_write_buffer[misc_count]);
  }
  //store data in flash
  EEPROM.writeString(address, (char*)flash_data);
  vTaskDelay(FLASH_READ_WRITE_DELAY);
  EEPROM.commit(); //call this function to update in flash
  vTaskDelay(FLASH_READ_WRITE_DELAY);
  //verify data is read/write successfully
  if( address == SERIAL_NUM_BASE_ADDR )
  {
    Serial.println("\nverifying serial number....");
  }
  else if( address == AP_SSID_BASE_ADDR )
  {
    Serial.println("\nverifying AP -> ssid....");
  }
  else if( address == AP_PASS_KEY_BASE_ADDR )
  {
    Serial.println("\nverifying AP -> password....");
  }
  else if( address == ORG_ID_BASE_ADDR )
  {
    Serial.println("\nverifying Org Id....");
  }
  else if( address == FACILITY_ID_BASE_ADDR )
  {
    Serial.println("\nverifying Facility Id....");
  }
  else if( address == ROOM_NUM_BASE_ADDR )
  {
    Serial.println("\nverifying Room no....");
  }
  else if( address == DEVICE_NAME_BASE_ADDR )
  {
    Serial.println("\nverifying device name....");
  }
  else if( address == SET_AC_MODE_BASE_ADDR )
  {
    Serial.println("\nverifying Set AC mode....");
  }
  else if( address == AUTH_CODE_BASE_ADDR )
  {
    Serial.println("\nverifying auth code....");
  }
  else if ( (address >= KINISI_SERIAL_MACID_BASE_ADDR) && ( address < (KINISI_SERIAL_MACID_BASE_ADDR * BYTE17) ) )
  {
    Serial.println("\nverifying Kinisi mac id....");
  }
  else if ( (address >= KINISI_SERIAL_NUM_BASE_ADDR) && ( address < (KINISI_SERIAL_NUM_BASE_ADDR * BYTE16) ) )
  {
    Serial.println("\nverifying Kinisi serial number....");
  }
  else if ( (address >= KINISI_DEVICE_NAME_BASE_ADDR) && ( address < (KINISI_DEVICE_NAME_BASE_ADDR * BYTE11) ) )
  {
    Serial.println("\nverifying Kinisi device name....");
  }

  Serial.println();
  Serial.print("read data :");
  if ( address == AP_SSID_BASE_ADDR )
  {
    for (int misc_count = address;misc_count < (len+address);misc_count++)
    {
      temp_read_buffer[misc_count - address] = EEPROM.readUChar(misc_count);
      Serial.printf("%c",temp_read_buffer[misc_count - address]);
    }
  }
  else
  {
    for (int misc_count = address;misc_count < (len+address);misc_count++)
    {
      temp_read_buffer[misc_count - address] = EEPROM.readUChar(misc_count);
      Serial.printf("%c",temp_read_buffer[misc_count - address]);
    }
  }
  for (verify_count = 0 ; verify_count < len ; verify_count++)
  {
    //String temp_flash_read_buffer = EEPROM.readString(address);
    //if( temp_flash_read_buffer.equals(String((char*)flash_data)) )
    if(temp_read_buffer[verify_count] == temp_write_buffer[verify_count])
    {
      //write and read data is same it will enter here
      if(verify_count <= (len-1))
      {
        Serial.print(".");
      }
    }
    else
    {
      Serial.println("\nError:Flash read/write issue");
      //update configuration status
      configuration_status = CONFIG_FAIL;
      send_ble_ack_packets(configuration_status);//send status to ble
      break;
    }
  }
  if( verify_count == len )
  {
    Serial.println("\nSuccessfully saved in flash");
  }
}
/**********************************************************************
 * @brief  store operating mode in flash
 *
 * @param  address      - location of data need to be stored
 *         working_mode - data need to be stored
 *
 * @return none
 *************************************************************************/
static void update_byte_to_flash(uint16_t address,uint8_t working_mode)
{
  uint8_t verify_mode = 0;
  vTaskDelay(FLASH_READ_WRITE_DELAY);
  EEPROM.writeUChar(address, working_mode);//store operating mode
  vTaskDelay(FLASH_READ_WRITE_DELAY);
  EEPROM.commit(); //call this function to update in flash
  vTaskDelay(FLASH_READ_WRITE_DELAY);
  if ( address == OVER_TEMP_STATUS_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating over temperature status");
  }
  else if ( address == KINISI_DECOM_DEVICE_COUNT )
  {
    verify_mode = working_mode;
    Serial.println("updating kinisi decommission device count");
  }
  else if ( address == KINISI_DEVICE_COUNT )
  {
    verify_mode = working_mode;
    Serial.println("updating device count");
  }
  else if ( address == SET_AC_TEMP_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating Set temperature");
  }
  else if ( address == KINISI_REG_STS_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating Kinisi registration status");
  }
  else if( address == WIFI_CONNECTION_TRIAL_COUNT_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating trial count");
  }
  else if( address == DEVICE_REG_STS_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating device registration status");
  }
  else if( address == CONFIG_DATA_STATUS_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating BLE configuration status");
  }
  else if( address == FACTORY_DEVICE_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating factory device status");
  }
  /****BLE extracted data *****/
  else if( address == AC_TYPE_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating AC mode");
  }
  else if( address == AC_BRAND_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating AC type");
  }
  else if( address == LIGHT_CTRL_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating Light control status");
  }
  else if( address == OCCUPANCY_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating Occupancy detection status");
  }
  else if( address == DEVICE_TYPE_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating Device type");
  }
  else if( address == SET_AC_STATUS_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating set AC status");
  }
  else if ( (address >= KINISI_REG_ADDR) && (address < (KINISI_REG_ADDR + MAX_KINISI_DEVICE ) ) )
  {
    verify_mode = working_mode;
    Serial.println("updating Kinisi registration  status");
  }
  else if( address == MAX_KINISI_COUNT_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating max kinisi in room");
  }

  /***Operating mode***/
  else
  {
    switch(working_mode)
    {
      case CONFIGURATION_MODE:
            verify_mode = CONFIGURATION_MODE;
            Serial.println("\nUpdating mode: CONFIGURATION_MODE");
            break;
      case NORMAL_MODE:
            verify_mode = NORMAL_MODE;
            Serial.println("\nUpdating mode: NORMAL_MODE");
            break;
      case OTAP_MODE:
            verify_mode = OTAP_MODE;
            Serial.println("\nUpdating mode: OTAP_MODE");
            break;
      default:
            break;
    }
  }
  if( EEPROM.readUChar(address) == verify_mode)
  {
    if ( address == OVER_TEMP_STATUS_BASE_ADDR )
    {
      Serial.println("over temperature status updated successfully");
    }
    else if ( address == KINISI_DECOM_DEVICE_COUNT )
    {
      Serial.println("kinisi decommission device count updated successfully");
    }
    else if ( address == KINISI_DEVICE_COUNT )
    {
      Serial.println("device count updated successfully");
    }
    else if( address == SET_AC_TEMP_BASE_ADDR )
    {
      Serial.println("Set temperature updated successfully");
    }
    else if( address == KINISI_REG_STS_BASE_ADDR )
    {
      Serial.println("Kinisi registartion status updated successfully");
    }
    else if( address == WIFI_CONNECTION_TRIAL_COUNT_BASE_ADDR )
    {
      Serial.println("trial count updated successfully");
    }
    else if ( address == DEVICE_REG_STS_BASE_ADDR )
    {
      Serial.println("Device reg status updated successfully");
    }
    /***BLE extracted data***/
    else if( address == AC_TYPE_BASE_ADDR )
    {
      Serial.println("AC mode updated successfully");
    }
    else if( address == AC_BRAND_BASE_ADDR )
    {
      Serial.println("AC type updated successfully");
    }
    else if( address == LIGHT_CTRL_BASE_ADDR )
    {
      Serial.println("Light control status ");
    }
    else if( address == OCCUPANCY_BASE_ADDR )
    {
      Serial.println("Occupancy detection status updated successfully");
    }
    else if( address == DEVICE_TYPE_BASE_ADDR )
    {
      Serial.println("Device type updated successfully");
    }
    else if( address == SET_AC_STATUS_BASE_ADDR )
    {
      Serial.println("AC status updated successfully");
    }
    else if( address == FACTORY_DEVICE_BASE_ADDR )
    {
      Serial.println("Factory device status updated successfully");
    }
    else if ( (address >= KINISI_REG_ADDR) && (address < (KINISI_REG_ADDR + MAX_KINISI_DEVICE ) ) )
    {
      Serial.println("Kinisi registration  status updated successfully");
    }
    else if( address == MAX_KINISI_COUNT_BASE_ADDR )
    {
      Serial.println("max kinisi in room count updated successfully");
    }
    /***Operating mode***/
    else
    {
      Serial.println("Operating mode updated in flash successfully.....");
    }
  }
  else
  {
    Serial.print("Error:");
    Serial.print(EEPROM.readUChar(address));
    Serial.println("\tFlash read fail");
  }
}
/**********************************************************************
 * @brief  store uint32_t bytes in flash
 *
 * @param  address      - location of data need to be stored
 *         working_mode - data need to be stored
 *
 * @return none
 *************************************************************************/
static void update_uint32_t_to_flash(uint16_t address,uint32_t working_mode)
{
  uint32_t verify_mode = 0;
  vTaskDelay(FLASH_READ_WRITE_DELAY);
  EEPROM.writeULong(address, working_mode);//store operating mode
  vTaskDelay(FLASH_READ_WRITE_DELAY);
  EEPROM.commit(); //call this function to update in flash
  vTaskDelay(FLASH_READ_WRITE_DELAY);
  if ( address == AC_CUM_ON_TIME_BAS_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating AC on time");
  }
  else if ( address == LIGHT_CUM_ON_TIME_BAS_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating light on time");
  }
  else if ( address == HSR_COUNT_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating HSR count");
  }
  else if ( address == MSR_COUNT_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating MSR count");
  }
  else if ( address == LSR_COUNT_BASE_ADDR )
  {
    verify_mode = working_mode;
    Serial.println("updating LSR count");
  }
  /*
   * check in flash data is write successfully
   */
  if( EEPROM.readLong(address) == verify_mode)
  {
    if ( address == AC_CUM_ON_TIME_BAS_ADDR )
    {
      Serial.println("AC on time updated successfully");
    }
    else if ( address == LIGHT_CUM_ON_TIME_BAS_ADDR )
    {
      Serial.println("light on time updated successfully");
    }
    else if ( address == HSR_COUNT_BASE_ADDR )
    {
      verify_mode = working_mode;
      Serial.println("HSR count updated successfully");
    }
    else if ( address == MSR_COUNT_BASE_ADDR )
    {
      verify_mode = working_mode;
      Serial.println("MSR count updated successfully");
    }
    else if ( address == LSR_COUNT_BASE_ADDR )
    {
      verify_mode = working_mode;
      Serial.println("LSR count updated successfully");
    }
  }
  else
  {
    Serial.print("Error:");
    Serial.print(EEPROM.readLong(address));
    Serial.println("\tFlash read fail");
  }
}
/**********************************************************************
 * @brief  Process received ble data
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void process_ble_data()
{
  uint16_t update_count=0;

  if( (ble_rx_buffer[FUNCTION_CODE] == BLE_PASSKEY_FUNC_CODE ) && (ble_rx_buffer[COMMAND] == AMENOS_BLE_COMMAND) )
  {
    if ( ble_rx_buffer[INSTANT_ID] == AUTHENTICATION )
    {
      if ( ble_rx_length == ble_rx_buffer[RXD_PACKET_SIZE] )
      {
        Serial.printf("Authentication Password Received : %x\n",ble_rx_buffer[INSTANT_ID]);
        for ( update_count = INSTANT_ID +1 ;update_count < (MAX_BLE_PACKET_SIZE - AUTHENTICATION_LEN) ; update_count++)
        {
          ble_authentication[update_count - (INSTANT_ID +1)] = ble_rx_buffer[update_count];
          Serial.printf("%c",ble_authentication[update_count - (INSTANT_ID +1)]);
        }
        ble_authentication[update_count - (INSTANT_ID +1)] = '\0';
        if( strcmp((const char*)ble_authentication,BLE_PASSOWRD) == 0 )
        {
          Serial.println("BLE password matched");
          configuration_status = PASSWORD_MATCHED;
          send_ble_ack_packets(configuration_status);//send status to ble
          allow_process_ble_data = 1;
        }
        else
        {
          Serial.println("BLE password mis-matched");
          configuration_status = PASSWORD_MIS_MATCHED;
          send_ble_ack_packets(configuration_status);//send status to ble
          allow_process_ble_data = 0;
        }

      }
      else
      {
        Serial.println("\nError: BLE packet size mismatch\nSend 20 bytes");
        //update configuration status
        configuration_status = CONFIG_FAIL;
        send_ble_ack_packets(configuration_status);//send status to ble
      }
      memset(ble_rx_buffer,NULL,sizeof(ble_rx_buffer));//clear array
    }
  }
  if ( allow_process_ble_data  )
  {
    if( (ble_rx_buffer[FUNCTION_CODE] == BLE_FUNCTIONAL_CODE ) && (ble_rx_buffer[COMMAND] == AMENOS_BLE_COMMAND) )
    {
      if ( ble_rx_buffer[INSTANT_ID] == ROOM_DETAILS )
      {
        if ( ble_rx_length == ble_rx_buffer[RXD_PACKET_SIZE] )
        {
          Serial.printf("Room Details Received : %x\n",ble_rx_buffer[INSTANT_ID]);
          for ( update_count = INSTANT_ID +1 ;update_count < MAX_BLE_PACKET_SIZE ; update_count++)
          {
            room_details[update_count - (INSTANT_ID +1)] = ble_rx_buffer[update_count];
            Serial.printf("%c",room_details[update_count - (INSTANT_ID +1)]);
          }
          room_details[update_count - (INSTANT_ID +1)] = '\0';
        }
        else
        {
          Serial.println("\nError: BLE packet size mismatch\nSend 20 bytes");
          //update configuration status
          configuration_status = CONFIG_FAIL;
          send_ble_ack_packets(configuration_status);//send status to ble
        }
      }
      else if ( ble_rx_buffer[INSTANT_ID] == SERIAL_NUMBER )
      {
        if ( ble_rx_length == ble_rx_buffer[RXD_PACKET_SIZE] )
        {
          Serial.printf("Serial Number Received : %x\n",ble_rx_buffer[INSTANT_ID]);
          for ( update_count = INSTANT_ID +1 ;update_count < MAX_BLE_PACKET_SIZE ; update_count++)
          {
            if( ble_rx_buffer[update_count] != '#' )
            {
              serial_number[update_count - (INSTANT_ID +1)] = ble_rx_buffer[update_count];
              Serial.printf("%c",serial_number[update_count - (INSTANT_ID +1)]);
            }
            else
            {
              serial_number[update_count - (INSTANT_ID +1)] = '\0';
            }
          }
          serial_number[update_count - (INSTANT_ID +1)] = '\0';//add delimiter
          Length[SERIAL_NUM] = strlen((char*)serial_number);//store array length of serial number
        }
        else
        {
          Serial.println("\nError: BLE packet size mismatch\nSend 20 bytes");
          //update configuration status
          configuration_status = CONFIG_FAIL;
          send_ble_ack_packets(configuration_status);//send status to ble
        }
      }
      else if ( ble_rx_buffer[INSTANT_ID] == DEVICE_DETAILS )
      {
        if ( ble_rx_length == ble_rx_buffer[RXD_PACKET_SIZE] )
        {
          Serial.printf("Device details Received : %x\n",ble_rx_buffer[INSTANT_ID]);
          for ( update_count = INSTANT_ID +1 ;update_count < MAX_BLE_PACKET_SIZE ; update_count++)
          {
            if( ble_rx_buffer[update_count] != '#' )
            {
              device_name[update_count - (INSTANT_ID +1)] = ble_rx_buffer[update_count];
              Serial.printf("%c",device_name[update_count - (INSTANT_ID +1)]);
            }
            else
            {
              device_name[update_count - (INSTANT_ID +1)] = '\0';
            }
          }
        }
        else
        {
          Serial.println("\nError: BLE packet size mismatch\nSend 20 bytes");
          //update configuration status
          configuration_status = CONFIG_FAIL;
          send_ble_ack_packets(configuration_status);//send status to ble
        }
      }
      else if ( ble_rx_buffer[INSTANT_ID] == AUTH_CODE )
      {
        if ( ble_rx_length == ble_rx_buffer[RXD_PACKET_SIZE] )
        {
          Serial.printf("Auth code details Received : %x\n",ble_rx_buffer[INSTANT_ID]);
          for ( update_count = INSTANT_ID +1 ;update_count < MAX_BLE_PACKET_SIZE ; update_count++)
          {
            if( ble_rx_buffer[update_count] != '#' )
            {
              amenos_auth_code[update_count - (INSTANT_ID +1)] = ble_rx_buffer[update_count];
              Serial.printf("%c",amenos_auth_code[update_count - (INSTANT_ID +1)]);
            }
            else
            {
              amenos_auth_code[update_count - (INSTANT_ID +1)] = '\0';
            }
          }
          write_data_to_flash = true;
          allow_process_ble_data = 0;
        }
        else
        {
          Serial.println("\nError: BLE packet size mismatch\nSend 20 bytes");
          //update configuration status
          configuration_status = CONFIG_FAIL;
          send_ble_ack_packets(configuration_status);//send status to ble
        }
      }
      else
      {
        Serial.print("Error: Data lost send again");
        //update configuration status
        configuration_status = CONFIG_FAIL;
        send_ble_ack_packets(configuration_status);//send status to ble
      }
      memset(ble_rx_buffer,NULL,sizeof(ble_rx_buffer));//clear array

    }
  }
}
/**********************************************************************
 * @brief  scan the available Hotspot/AP and list it out with ssid,signal strength
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void listNetworks()
{
  Serial.println("Scan nearby networks....");

  // WiFi.scanNetworks will return the number of networks found
  int available_wifi = WiFi.scanNetworks();
  Serial.println("scan done");
  if ( available_wifi == NO_WIFI_AVAILABLE )
  {
    Serial.println("no networks found");
  }
  else
  {
    Serial.print(available_wifi);
    Serial.println(" networks found");
    for (int network = 0; network < available_wifi; ++network)
    {
      // Print SSID and RSSI for each network found
      Serial.print(network + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(network));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(network));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(network) == WIFI_AUTH_OPEN)?" ":"*"); //if condition ? true : false
      vTaskDelay(10);
    }
  }
}

/**********************************************************************//**
 * @brief  Trying to connect to ssid,passkey mentioned.It will restart the ESP
 *         if not connects to AP 3 sec.
 *
 * @param  APsssid    - pointer holds the ssid of accesspoint
 *         APpassword - pointer holds the password of accesspoint
 *
 * @return none
 *************************************************************************/

static void connect_to_ap(uint8_t *APsssid,uint8_t *APpassword)
{
  if(EEPROM.readUChar(OPERATING_MODE_BASE_ADDR) != CONFIGURATION_MODE)
  {
    send_device_reg_packet  = true;
    Serial.println(" send device reg packet Flag set");
  }
  DL2_led_control(LED_R2,LED_DRIVE_HIGH);
  uint16_t connection_intervel = 0;
  trial_count = EEPROM.readUChar(WIFI_CONNECTION_TRIAL_COUNT_BASE_ADDR);
  Serial.printf("\nRead trial count = %d",trial_count);
  vTaskDelay(2);
  Serial.println("Disconnecting previous WiFi connection...");
  WiFi.disconnect();//disconnect previous AP connection
  Serial.println("Disconnected successfully");
  WiFi.mode(WIFI_STA);//set WiFi to station mode
  listNetworks();//scan for avilable networks
  for(int i = 0;i<strlen((char*)ap_ssid);i++)
  {
    Serial.printf("%x",APsssid[i]);
  }
  Serial.println();
  for(int i = 0;i<strlen((char*)ap_passkey);i++)
  {
    Serial.printf("%x",APpassword[i]);
  }
  Serial.println();
  WiFi.begin((char*)APsssid,(const char*)APpassword);//connect to mentioned wifi network
  Serial.print("connecting to wifi");
  while( WiFi.status() != WL_CONNECTED )
  {
    //wait untill board connects to mentioned network
    connection_intervel++;
    //update configuration status
    configuration_status = WIFI_CONNECTION_IN_PROGRESS;
    /*
     * Send Acknowledgement to TIRISI App only on configuration mode
     */
    if( operating_mode == CONFIGURATION_MODE )
    {
      Serial.println("send ble ack");
      send_ble_ack_packets(configuration_status);//send status to ble
    }
    Serial.print(".");
    if(  connection_intervel >= WIFI_CONNECTION_TIME_OUT )
    {
      trial_count++;
      Serial.printf("\nAP connection trial count = %d",trial_count);
      update_byte_to_flash(WIFI_CONNECTION_TRIAL_COUNT_BASE_ADDR,trial_count);
      /*if ESP doesn't connect to AP within WIFI_CONNECTION_TIME_OUT
        restart ESP*/
      if(trial_count <= CONNECTION_TRIAL_COUNT)
      {
        Serial.println("ESP restarting for Wifi connection.....");
        vTaskDelay(500);
        ESP.restart();
      }
      else
      {
        /*reset the trial count and update in flash*/
        trial_count = 0;
        update_byte_to_flash(WIFI_CONNECTION_TRIAL_COUNT_BASE_ADDR,trial_count);
        /*In 3 trial if ESP doesn't connect to ssid,password
         *change mode to CONFIGURATION_MODE and restart the ESP if device is already not registered
         */
         if( !device_register_status )
         {
          operating_mode = CONFIGURATION_MODE;
          update_byte_to_flash(OPERATING_MODE_BASE_ADDR,operating_mode);
          //Change configuration received status to false
          config_data_rcvd = false;
          update_byte_to_flash(CONFIG_DATA_STATUS_BASE_ADDR,config_data_rcvd);
          //send acknowledgment to connected ble device
          configuration_status = WIFI_NOT_CONNECT;
          send_ble_ack_packets(configuration_status);//send status to ble
          //restart ESP to change mode to CONFIGURATION_MODE
          Serial.println("ESP restarting for mode change");
          vTaskDelay(500);
          ESP.restart();
         }
         else
         {
            Serial.println("Device already registered\nHub not available...\nESP restarting");
            vTaskDelay(500);
            ESP.restart();
         }
      }
    }
    vTaskDelay(500);
  }
  DL2_led_control(LED_R2,LED_DRIVE_LOW);
  DL2_led_control(LED_G2,LED_DRIVE_HIGH);
  if( operating_mode == CONFIGURATION_MODE )
  {
    configuration_status = WIFI_CONNECTED;
    send_ble_ack_packets(configuration_status);//send status to ble
  }
  trial_count = 0;
  update_byte_to_flash(WIFI_CONNECTION_TRIAL_COUNT_BASE_ADDR,trial_count);
  /*
   * print Wifi connection status
   */
  Serial.println("ESP32 WiFi connection status =>");
  Serial.print("\nconnected to WiFi Network : ");
  for ( int misc_count = 0;misc_count < strlen((char*)APsssid); misc_count++)
  {
    Serial.printf("%c",APsssid[misc_count]);
  }
  Serial.println();
  Serial.print("IP address: ");
  Serial.println( WiFi.localIP() ); /*once board gets successfully conneced,
                                      AP will assign ip to board*/
  Serial.print("signal strength : ");
  Serial.print( WiFi.RSSI() );
  Serial.println(" dbm");
  Serial.println("Starting NTP client...");
  timeClient.begin();
  setupudp();
}
/**********************************************************************//**
 * @brief  whenever data is received fro UDP server it will receive and
 *         store in udp_rx_packets buffer
 *         Print the server IP,PORT
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void udp_rx_data()
{
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {//
    Serial.print("\nReceived packet size : ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(Udp.remotePort());
    size_t len = 0;
    StaticJsonDocument<2048> parsing_buffer;
    DeserializationError error;
    error = deserializeJson(parsing_buffer, Udp);
    if( error )
    {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(error.c_str());
    }
    else
    {
      Serial.println("parsing success");
      len = measureJsonPretty(parsing_buffer);
      if ( len > 0)
      {
        Serial.printf("\nparsed buffer length =%d\n",len);
        /*
         * print received data in serial terminal
         */
        Serial.println("Received data");
        serializeJsonPretty(parsing_buffer, Serial);//print in serial terminal
      }
    }
  }
  else
  {
//    Serial.print("Server respone: ");
//    Serial.println(packetSize);
  }
}
/**********************************************************************//**
 * @brief  Open the socket mentioned in udp_ip,udp_port
 *         Transmit data in the udp_tx_packets buffer to opened socket.
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void udp_tx_data()
{
  if (!Udp.beginPacket(udp_ip,udp_port)) //open the socket
  {
    Serial.println("\nError:UDP connection failed");
  }
  else
  {
    Serial.println("Success:connected to server");
  }
  Serial.println("UDP data to server");

  StaticJsonDocument<1024> json_buffer;//allocate 1kb memory in stack
  json_buffer["hmac"] = "";
  JsonObject payload = json_buffer.createNestedObject("payload");//add for nested json document
  payload["device_uid"] = "123456";//add serial number

  payload["action"] = DEVICE_REG_ACK;
  JsonArray data = payload.createNestedArray("device_data");
  JsonObject device_data = data.createNestedObject();//add for nested json document
  device_data["ack_type"] = 1;
  device_data["status"] = 1;
  Serial.println("Sensor to Hub:");
  serializeJsonPretty(json_buffer, Serial);//print in serial terminal
  serializeJsonPretty(json_buffer,Udp);//send data to idp server
  //Udp.write(udp_tx_packets,4);
  Udp.endPacket();
}
/**********************************************************************//**
 * @brief  listening at UDP PORT mentioned udp_port.
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void setupudp()
{//open UDP socket
  if ( Udp.begin(udp_port) )
  {
    Serial.println("Successfully opened UDP socket");
    Serial.println("Listening at :");
    Serial.print("Local port: ");
    Serial.println(udp_port);
    Serial.print("Local IP: ");
    Serial.println( WiFi.localIP() );

  }
  else
  {
    Serial.println("Error:No Sockets available to use");
  }
}

/**********************************************************************//**
 * @brief  Enable the ineternal flash for user
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void setup_internal_flash()
{
  Serial.println("Initializing Internal Flash....");
  if (!EEPROM.begin(FLASH_SIZE))
  {
    Serial.println("Failed to initialise Flash");
    Serial.println("Restarting...");
    vTaskDelay(1000);
    ESP.restart();
  }
  Serial.println("Initialize Internal Flash successfully");
}
/**********************************************************************//**
 * @brief  Initialize BLE with name AMENOS
 *         added Tx,Rx characteristics
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void setup_ble()
{
  BLEDevice::init("CaleidoXenia");//create ble device
  //BLEServer *pServer = BLEDevice::createServer();//create ble server
  pServer = BLEDevice::createServer();//create ble server
  pServer->setCallbacks(new MyServerCallbacks());
  //BLEService *pService = pServer->createService(SERVICE_UUID);//create ble service
  pService = pServer->createService(SERVICE_UUID);//create ble service
  pTxCharacteristic = pService->createCharacteristic
                    (
                        //create ble Tx characteristics
                        CHARACTERISTIC_UUID_TX,//add tx characteristics
                        BLECharacteristic::PROPERTY_NOTIFY//add notification characteristics
//                       BLECharacteristic::PROPERTY_READ |
//                       BLECharacteristic::PROPERTY_WRITE
                     );
  pTxCharacteristic->addDescriptor(new BLE2902());//ble2902 need to notify
  pRxCharacteristic = pService->createCharacteristic
                    (
                      //create ble rx characteristics
                      CHARACTERISTIC_UUID_RX,//add rx characteristics
                      BLECharacteristic::PROPERTY_WRITE //add write permission
                    );
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  ble_start_advertisement();
  Serial.print("Waiting for BLE device to connect");
}
/**********************************************************************//**
 * @brief  starts the BLE advertisement
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void ble_start_advertisement()
{
  pService->start();//start the service
  pServer->getAdvertising()->start();//start advertising
}
/**********************************************************************//**
 * @brief  stops the BLE advertisement
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void ble_stop_advertisement()
{
  pService->stop();//stop the service
  pServer->getAdvertising()->stop();//stop advertising
}
/**********************************************************************//**
 * @brief  Transmit data to connected BLE device
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void send_ble_ack_packets(uint8_t Status)
{
  if( ble_connection_status )
  {
    ble_ack_to_mobile[PACKET_SIZE] = BLE_ACK_BYTE_SIZE;
    ble_ack_to_mobile[FUNCTION_CODE_BYTE] = BLE_FUNCTIONAL_CODE;
    ble_ack_to_mobile[COMMAND_BYTE] = AMENOS_BLE_COMMAND;
    ble_ack_to_mobile[INSTANT_ID_BYTE] = BLE_ACK_INSTANT_ID;
    if( configuration_status == WIFI_NOT_CONNECT)
    {
      ble_ack_to_mobile[RESULT_BYTE] = WIFI_CONNECTION_FAIL;
      Serial.println("Wifi connection fail");
    }
    else if( configuration_status == WIFI_CONNECTION_IN_PROGRESS)
    {
      ble_ack_to_mobile[RESULT_BYTE] = CONNECTION_IN_PROGRESS;
      Serial.println("Wifi connection in progress");
    }
    else if( configuration_status == WIFI_CONNECTED)
    {
      ble_ack_to_mobile[RESULT_BYTE] = CONFIGURATION_SUCCESS;
      Serial.println("Configuration success");
    }
    else if( configuration_status == CONFIG_FAIL)
    {
      ble_ack_to_mobile[RESULT_BYTE] = CONFIG_WRITE_FAIL;
      Serial.println("Configuration write fail");
    }
    else  if( configuration_status == PASSWORD_MATCHED )
    {
      ble_ack_to_mobile[INSTANT_ID_BYTE] = AUTHENTICATION;
      ble_ack_to_mobile[RESULT_BYTE] = AUTHENTICATION_SUCCESS;
      Serial.println("BLE Authentication success");
    }
    else if( configuration_status == PASSWORD_MIS_MATCHED  )
    {
      ble_ack_to_mobile[INSTANT_ID_BYTE] = AUTHENTICATION;
      ble_ack_to_mobile[RESULT_BYTE] = AUTHENTICATION_FAIL;
      Serial.println("BLE Authentication failed");
    }

//    String ble_tx_data = (char*)ble_ack_to_mobile;
//    String ble = "hello";
//    ble_tx_value = random(1,10);
    //dtostrf(ble_tx_value,1,2,ble_tx_data);
    pTxCharacteristic->setValue(ble_ack_to_mobile,sizeof(ble_ack_to_mobile));//set value to characteristic
    pTxCharacteristic->notify();//notifying the connected client
    Serial.print("sent value: ");
    Serial.println((char*)ble_ack_to_mobile);
    memset(ble_ack_to_mobile,'\0',sizeof(ble_ack_to_mobile));
    //vTaskDelay(1000);
  }
}
/**********************************************************************//**
 * @brief  Post data in Json format to TCP server
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void Json_post_to_AP(uint16_t tcp_packet_no,uint16_t sensor,uint8_t *Serial_Number,uint8_t *Device_Name)
{
     /*
     * Create JSON documnet.
     * Don't use more than 1 Kb size,else it will create issue
     */
    StaticJsonDocument<1024> json_buffer;//allocate 1kb memory in stack
    uint8_t copy_buffer[1024]={0};
    /*
     *      Device Registration packet
     */
    if(  tcp_packet_no ==  DEVICE_REGISTRATION )
    {
      /*  sample device registration JOSN packet
           * {
        "hmac": "",
        "payload": {
        "device_uid": "1234566778",
        "data_type": 1,
        "device_data": [
          {
          "device_type": 4,
          "product_model": "AMENOS",
          "firmware_version": "1.0",
          "mfg_date": "2-3-2020",
          "relay_cutoff": 2,
          "device_operational_mode": 1,
          "device_variant":16,
          "org_uid": "012",
          "fraciliti_uid": "k12",
          "amenity_name": "SPA010",
          "device_name": "AME1",
          "timestamp": "2020-12-24 05:15:09"
          }
        ]
        }
      }
          */
      Serial.println("\nsending Device registrartion packet");
      if ( sensor == AMENOS )
      {
        json_buffer["hmac"]                     = "";
        JsonObject payload                      = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]                   = serial_number;//add serial number
#if 0   //ack testing
        payload["action"] = DEVICE_REG_ACK;
        JsonArray data = payload.createNestedArray("device_data");
        JsonObject device_data = data.createNestedObject();//add for nested json document
        device_data["ack_type"] = 1;
        device_data["status"] = 1;
#endif
#if 0   //OTAP testing
        payload["action"] = 7;
        JsonArray data = payload.createNestedArray("device_data");
        JsonObject device_data = data.createNestedObject();//add for nested json document
        device_data["firmware_version"] = "1.1";
        device_data["firmware_path"] = "ftp://192.168.43.18/binary/firmware.bin";
        device_data["crc"] = "2345";
        device_data["file_size"] = "10";
#endif
#if 1
        payload["data_type"]                    = DEVICE_REGISTRATION;
        JsonArray data                          = payload.createNestedArray("device_data");
        JsonObject device_data                  = data.createNestedObject();//add for nested json document
        device_data["device_type"]              = device_type;
        device_data["auth_code"]                = amenos_auth_code;
        device_data["product_model"]            = PRODUCT_MODEL;
        device_data["firmware_version"]         = FIRMWARE_VERSION;
        device_data["mfg_date"]                 = MANUFACTURE_DATE;
        device_data["device_operational_mode"]  = SLAVE_TO_HUB;
        device_data["device_variant"]           = device_variant;
        device_data["org_uid"]                  = org_id;
        device_data["faciliti_uid"]             = facility_id;
        device_data["amenity_name"]             = room_no;
        device_data["device_name"]              = device_name;
        device_data["timestamp"]                = formatted_date;
#endif
      }
      else if ( sensor == KINISI )
      {
        json_buffer["hmac"]                      = "";
        JsonObject payload                       = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]                    = Serial_Number;//add serial number
#if 1
        payload["action"] = DEVICE_REG_ACK;
        JsonArray data = payload.createNestedArray("device_data");
        JsonObject device_data = data.createNestedObject();//add for nested json document
        device_data["ack_type"] = 1;
        device_data["status"] = 1;
#endif
#if 0   //OTAP testing
        payload["action"] = 7;
        JsonArray data = payload.createNestedArray("device_data");
        JsonObject device_data = data.createNestedObject();//add for nested json document
        device_data["firmware_version"] = "1.1";
        device_data["firmware_path"] = "ftp://192.168.43.18/binary/firmware.bin";
        device_data["crc"] = "2345";
        device_data["file_size"] = "10";
#endif
#if 0
        payload["data_type"]                    = DEVICE_REGISTRATION;
        JsonArray data                          = payload.createNestedArray("device_data");
        JsonObject device_data                  = data.createNestedObject();//add for nested json document
        device_data["device_type"]              = kinisi_device_type;
        device_data["auth_code"]                = kinisi_auth_code;
        device_data["product_model"]            = PRODUCT_MODEL_KINISI;
        device_data["firmware_version"]         = kinsi_fm_version;
        device_data["mfg_date"]                 = kinisi_mfg;
        device_data["device_operational_mode"]  = SLAVE_TO_AMENOS;
        device_data["org_uid"]                  = kinisi_org_id;
        device_data["faciliti_uid"]             = kinisi_Facility_id;
        device_data["amenity_name"]             = kinisi_Amenity_id;
        device_data["device_name"]              = Device_Name;
        device_data["timestamp"]                = formatted_date;
#endif
      }
      else if ( sensor == LIGHT )
      {
        json_buffer["hmac"]                     = "";
        JsonObject payload                      = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]                   = json_extract.zone_light_epid; //light mac id
#if 1
        payload["data_type"]                    = DEVICE_REGISTRATION;
        JsonArray data                          = payload.createNestedArray("device_data");
        JsonObject device_data                  = data.createNestedObject();//add for nested json document
        device_data["device_type"]              = 8; // for light
        device_data["mfg_date"]                 = json_extract.light_mfg_date;
        device_data["device_operational_mode"]  = SLAVE_TO_AMENOS;
        device_data["org_uid"]                  = json_extract.light_org_uid;
        device_data["faciliti_uid"]             = json_extract.light_facility_uid;
        device_data["amenity_name"]             = json_extract.light_amenity_name;
        device_data["device_name"]              = json_extract.light_device_name;
        device_data["zone"]                     = json_extract.zone;
        device_data["timestamp"]                = formatted_date;
#endif
      }
    }
    /*
     *           sensor health packet
     */
    else if (tcp_packet_no == DEVICE_HEALTH_STATUS )
    {
      /* sample device registration JSON packet
       * {
      "hmac": "",
      "payload": {
      "device_uid": "1234566778",
      "data_type": 1,
      "device_data": [
        {
          "device_health_status": 1
        }
      ]
      }
    }
        */
      Serial.println("\nSending device health status");
      if ( sensor == AMENOS )
      {
        JsonObject key                      = json_buffer.to<JsonObject>();//create json object
        json_buffer["hmac"]                 = "";
        JsonObject payload                  = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]               = serial_number;//add serial number
        payload["data_type"]                = DEVICE_HEALTH_STATUS;
        JsonArray data                      = payload.createNestedArray("device_data");
        JsonObject device_data              = data.createNestedObject();//add for nested json documentdevice_data["device_type"] = DEVICE_TYPE;
        device_data["device_health_status"] = 0;
        device_data["device_temperature"]   = room_temperature;
      }
      else if ( sensor == KINISI )
      {
        JsonObject key                      = json_buffer.to<JsonObject>();//create json object
        json_buffer["hmac"]                 = "";
        JsonObject payload                  = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]               = Serial_Number;
        payload["data_type"]                = DEVICE_HEALTH_STATUS;
        JsonArray data                      = payload.createNestedArray("device_data");
        JsonObject device_data              = data.createNestedObject();//add for nested json documentdevice_data["device_type"] = DEVICE_TYPE;
        device_data["device_health_status"] = 0;
        device_data["device_temperature"]   = 0; //kinisi don't have tempearture sensor
      }
      else if ( sensor == LIGHT )
      {
        JsonObject key                      = json_buffer.to<JsonObject>();//create json object
        json_buffer["hmac"]                 = "";
        JsonObject payload                  = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]               = json_extract.zone_light_epid ;//bulb mac id
        payload["data_type"]                = DEVICE_HEALTH_STATUS;
        JsonArray data                      = payload.createNestedArray("device_data");
        JsonObject device_data              = data.createNestedObject();//add for nested json documentdevice_data["device_type"] = DEVICE_TYPE;
        device_data["device_health_status"] = 0;
        device_data["device_temperature"]   = 0; //Light don't have tempearture sensor
      }
    }
    /*
     * Sensor data packet
     */
    else if (tcp_packet_no == DEVICE_SENSOR_DATA)
    {
      /*  sample device sensor data JSON packet
       * {
      "hmac": "",
      "payload": {
      "device_uid": "1234566778",
      "data_type": 1,
      "device_data": [
        {
        "sender_uid": "AMENOS",
        "packet_type": 3,
        "temperature": 34.5,
        "light_intensity": 23
        }
      ]
      }
    }

       */
      Serial.println("\nSending sensor data");
      if ( sensor == AMENOS )
      {
        JsonObject key                    = json_buffer.to<JsonObject>();//create json object
        json_buffer["hmac"]               = "";
        JsonObject payload                = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]             = serial_number;//add serial number
        payload["data_type"]              = DEVICE_SENSOR_DATA;
        JsonArray data                    = payload.createNestedArray("device_data");
        JsonObject device_data            = data.createNestedObject();//add for nested json document
        device_data["sender_uid"]         = PRODUCT_MODEL;
        device_data["packet_type"]        = DEVICE_SENSOR_DATA;
        device_data["device_temperature"] = room_temperature;
        device_data["set_ac_temperature"] = json_extract.ac_temp;
        device_data["relay1_count"]       = lsr_switching_count;
        device_data["relay2_count"]       = msr_switching_count;
        device_data["relay3_count"]       = hsr_switching_count;
        device_data["ac_status"]          = json_extract.ac_status;//need to check
        device_data["mode"]               = json_extract.ac_mode; //need to check
      }
      else if ( sensor == KINISI )
      {
        JsonObject key = json_buffer.to<JsonObject>();//create json object
        json_buffer["hmac"]                 = "";
        JsonObject payload                  = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]               = Serial_Number;
        payload["data_type"]                = DEVICE_SENSOR_DATA;
        JsonArray data                      = payload.createNestedArray("device_data");
        JsonObject device_data              = data.createNestedObject();//add for nested json document
        device_data["sender_uid"]           = PRODUCT_MODEL_KINISI;
        device_data["packet_type"]          = DEVICE_SENSOR_DATA;
        device_data["battery_percentage"]   = kinisi_battery_percentage;
        device_data["occupancy_detection"]  = occupancy_detected;
      }
      else if ( sensor == LIGHT )
      {
        JsonObject key                = json_buffer.to<JsonObject>();//create json object
        json_buffer["hmac"]           = "";
        JsonObject payload            = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]         = serial_number;//add serial number
        payload["data_type"]          = DEVICE_SENSOR_DATA;
        JsonArray data                = payload.createNestedArray("device_data");
        JsonObject device_data        = data.createNestedObject();//add for nested json document
        device_data["sender_uid"]     = "light";
        device_data["packet_type"]    = DEVICE_SENSOR_DATA;
        device_data["address_id"]     = "12434ed455345";//need to change to light mac id
        device_data["zone"]           = json_extract.zone;
        device_data["total_on_time"]  = "2345";//need to change to minutes
        device_data["color"]          = json_extract.light_color;
        device_data["light_status"]   = json_extract.light_status;
        device_data["contrast"]       = json_extract.light_contrast;
      }
    }
    else if (tcp_packet_no == LIGHT_REG_ACK_PACKET)
    {
      Serial.println("\nSending Light registration ACK packet");
      if ( sensor == AMENOS )
      {
        JsonObject key            = json_buffer.to<JsonObject>();//create json object
        json_buffer["hmac"]       = "";
        JsonObject payload        = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]     = serial_number;//add serial number
        payload["data_type"]      = LIGHT_REG_ACK_PACKET;
        JsonArray data            = payload.createNestedArray("device_data");
        JsonObject device_data    = data.createNestedObject();//add for nested json document
        device_data["address_id"] = "23e3r3443tgrg5";//need to change
        device_data["ack_type"]   = 1;//need to change
        device_data["status"]     = 1;//need to change
      }
    }
    else if (tcp_packet_no == AC_FAIL_PACKET)
    {
      Serial.println("\nSending AC Fail packet");
      if ( sensor == AMENOS )
      {
        JsonObject key                    = json_buffer.to<JsonObject>();//create json object
        json_buffer["hmac"]               = "";
        JsonObject payload                = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]             = serial_number;//add serial number
        payload["data_type"]              = AC_FAIL_PACKET;
        JsonArray data                    = payload.createNestedArray("device_data");
        JsonObject device_data            = data.createNestedObject();//add for nested json document
        device_data["alert_type"]         = AC_FAIL_ALERT;
        device_data["time_ac_run"]        = cum_on_time[AC];
        device_data["ac_set_temperature"] = json_extract.ac_temp;
        device_data["room_temperature"]   = room_temperature;
      }
    }
    else if (tcp_packet_no == MAITENANCE_ACK_PACKET)
    {
      Serial.println("\nSending Maintenance ACK packet");
      if ( sensor == AMENOS )
      {
        JsonObject key          = json_buffer.to<JsonObject>();//create json object
        json_buffer["hmac"]     = "";
        JsonObject payload      = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]   = serial_number;//add serial number
        payload["data_type"]    = MAITENANCE_ACK_PACKET;
        JsonArray data          = payload.createNestedArray("device_data");
        JsonObject device_data  = data.createNestedObject();//add for nested json document
        device_data["ack"]      = 1;
      }
    }
    else if (tcp_packet_no == OTAP_ACK_PACKET)
    {
      Serial.println("\nSending OTAP ACK packet");
      if ( sensor == AMENOS )
      {
        JsonObject key          = json_buffer.to<JsonObject>();//create json object
        json_buffer["hmac"]     = "";
        JsonObject payload      = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]   = serial_number;//add serial number
        payload["data_type"]    = OTAP_ACK_PACKET;
        JsonArray data          = payload.createNestedArray("device_data");
        JsonObject device_data  = data.createNestedObject();//add for nested json document
        device_data["ack"]      = 1;
      }
    }
    else if (tcp_packet_no == AC_SETTING_PACKET)
    {
      Serial.println("\nSending  AC Setting packet");
      if ( sensor == AMENOS )
      {
        JsonObject key                    = json_buffer.to<JsonObject>();//create json object
        json_buffer["hmac"]               = "";
        JsonObject payload                = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]             = serial_number;//add serial number
        payload["data_type"]              = AC_SETTING_PACKET;
        JsonArray data                    = payload.createNestedArray("device_data");
        JsonObject device_data            = data.createNestedObject();//add for nested json document
        device_data["ac_status"]          = json_extract.ac_status;
        device_data["mode"]               = json_extract.ac_mode;
        device_data["ac_set_temperature"] = json_extract.ac_temp;
        device_data["time_ac_run"]        = cum_on_time[AC];
      }
    }
    else if (tcp_packet_no == LIGHT_FAIL_PACKET)
    {
      Serial.println("\nSending Light Fail packet");
      if ( sensor == AMENOS )
      {
        JsonObject key            = json_buffer.to<JsonObject>();//create json object
        json_buffer["hmac"]       = "";
        JsonObject payload        = json_buffer.createNestedObject("payload");//add for nested json document
        payload["device_uid"]     = serial_number;//add serial number
        payload["data_type"]      = LIGHT_FAIL_PACKET;
        JsonArray data            = payload.createNestedArray("device_data");
        JsonObject device_data    = data.createNestedObject();//add for nested json document
        device_data["alert_type"] = LIGHT_FAIL_ALERT;
        device_data["address_id"] = "234567345353535";//need to change
        device_data["zone"]       = json_extract.zone;//need to change to variable
      }
    }
    else
    {
      Serial.printf("\nTCP packet mis-match : %d\n",tcp_packet_no);
    }
    /*
     * send json data to udp socket
     */
    Serial.print("connecting to UDP server :");
    Serial.println((char *)udp_ip);
    if (!Udp.beginPacket(udp_ip,udp_port)) //open the socket
    {
      Serial.println("\nError:UDP connection failed");
      if ( sensor  == AMENOS )
      {
        device_register_status = false;
        Serial.println("Amenos not registered");
      }
      else if (  sensor  == KINISI )
      {
        kinisi_register_status = false ;
        Serial.println("Kinisi registration status : Fail");
        registration_packet_send_count = 0;
        register_kinisi = 0;
        send_ble_packets_to_kinisi(DEVICE_REG_ACK,EXIT_FAILURE);//send NACK to Kinisi
        //update kinisi count
        if( no_of_kinisi_device >= BYTE1)
        {
          no_of_kinisi_device -= 1;
          update_byte_to_flash(KINISI_DEVICE_COUNT,no_of_kinisi_device);
          Serial.printf("\nno of kinsi device available in this room = %d\n",no_of_kinisi_device);
        }
        pClient->disconnect();
        Serial.println("Disconnected from Kinisi");
        vTaskDelay(50);
        //clear all buffer
        for ( int indice = 0;indice < BLE_BUFFER_SIZE;indice++ )
        {
           kinisi_room_details[indice]  = '\0';
           kinisi_serial_number[indice] = '\0';
           kinisi_device_name[indice]   = '\0';
           if ( indice <= BLE_BUFFER_SIZE/2 )
           {
              kinisi_auth_code[indice]  = '\0' ;
           }
        }
        //store kinisi registration status in flash
        kinisi_lookup_table[kinisi_reg_location][4] = itoa(kinisi_register_status,str_buffer,DECIMAL); //update kinisi registration status
        print_Kinsi_lookup_table(kinisi_lookup_table);
//        update_byte_to_flash(KINISI_REG_STS_BASE_ADDR,kinisi_register_status);
        update_byte_to_flash((KINISI_REG_ADDR + no_of_kinisi_device),kinisi_register_status);
      }
    }
    else
    {
      Serial.println("Sensor to Hub:");
      //uint8_t Room[7] = "11012"; // for testing
      seceret_key = get_hmac_key(room_no,(uint8_t *)STATIC_CODE) ;
      if( seceret_key != NULL )
      {
        serializeJson(json_buffer,copy_buffer);//make copy
        hmac_payload = get_payload(copy_buffer,strlen((char *)copy_buffer));
        if( hmac_payload != NULL )
        {
          if( Generate_hmac((uint8_t *)seceret_key,(uint8_t *)hmac_payload) != EXIT_FAILURE )
          {
//            Serial.println("adding hmac to data");
            json_buffer["hmac"] = hmacResult;
            serializeJsonPretty(json_buffer, Serial);//print in serial terminal

            serializeJsonPretty(json_buffer,Udp);//send data to idp server
            if(  tcp_packet_no ==  DEVICE_REGISTRATION )
            {
              if( !json_extract.reg_required && !send_device_reg_packet)
              {
                Serial.println("\nwaiting for device registration ACK");
                if ( sensor == AMENOS )
                {
                  Serial.println("*****************************************************");
                  receive_process_hub_data(DEVICE_REGISTRATION,AMENOS);
                }
                else if (  sensor == KINISI )
                {
                  Serial.println("_____________________________________________________");
//                  vTaskDelay(100);
                  receive_process_hub_data(DEVICE_REGISTRATION,KINISI);
                }
              }
              else
              {
                json_extract.reg_required = 0;
                Serial.println("\nDevice already registered.Do not wait for Acknowledgement");
              }
            }
            else
            {

            }
          }
          else
          {
//            hmacResult = (byte)NULL;
          }
        }
        else
        {
          hmac_payload = NULL;
        }
      }
      else
      {
        seceret_key = NULL;
      }
    }
    //Udp.endPacket();//close the udp server
    Serial.println("Closing connection.");
    Udp.endPacket();
}
/**********************************************************************//**
 * @brief  Get Json data from API and print it
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void receive_process_hub_data(uint16_t operation,uint16_t sensor)
{
  size_t len = 0;
  StaticJsonDocument<2048> parsing_buffer;
  uint8_t parsing_buffer_copy[1024]={0};
  DeserializationError error;
  int packetSize=0;
  if( operation == DEVICE_REGISTRATION )
  {
    current_time = 0;
    last_time = 0;
    //waits here to receive and extract device registration ack packet
//    current_time = millis();
//    while(millis() < (current_time+HUB_ACK_TIMEOUT) )
//    {
      packetSize = Udp.parsePacket();
      if (packetSize)
      {//
        Serial.print("\nReceived packet size : ");
        Serial.println(packetSize);
        Serial.print("From ");
        IPAddress remoteIp = Udp.remoteIP();
        Serial.print(remoteIp);
        Serial.print(", port ");
        Serial.println(Udp.remotePort());
        error = deserializeJson(parsing_buffer, Udp);
        if( error )
        {
          Serial.print(F("deserializeJson() failed with code "));
          Serial.println(error.c_str());
        }
        else
        {
          Serial.println("parsing success");
          len = measureJson(parsing_buffer);
          if ( len > 0)
          {
          Serial.printf("\nparsed buffer length =%d\n",len);
          /*
           * print received data in serial terminal
           */
          Serial.println("Received data");
          serializeJsonPretty(parsing_buffer, Serial);//print in serial terminal
          serializeJson(parsing_buffer, parsing_buffer_copy);//make copy
          json_extract.hmac = parsing_buffer["hmac"];
          if( hmac_validation(parsing_buffer_copy) != EXIT_FAILURE) // validate hmac for received data
          {
            json_extract.device_id = parsing_buffer["payload"]["device_uid"];
            json_extract.action = parsing_buffer["payload"]["action"];
            Serial.printf("action num = %d\n",json_extract.action );
            json_extract.ack_type = parsing_buffer["payload"]["device_data"][0]["ack_type"];
            json_extract.status = parsing_buffer["payload"]["device_data"][0]["status"];
            Serial.printf("ack type = %d\n",json_extract.ack_type);
            Serial.printf("status = %d\n",json_extract.status);


            if(strcmp(json_extract.device_id,(const char*)kinisi_serial_number) == 0 )
            {//check it is kinisi serial number or not
              if( json_extract.action == DEVICE_REG_ACK )
              {
                /***********Device ack packet extraction**********/
                Serial.println("\nKinisi Device registartion ack received");
                if ( json_extract.status && json_extract.ack_type )
                {
                  if (  sensor  == KINISI )
                  {
                    kinisi_register_status = true ;
                    Serial.println("Kinisi registration status : Success");
                    send_ble_packets_to_kinisi(DEVICE_REG_ACK,EXIT_SUCCESS);//send ACK to Kinisi
                    //update register kinisi count
                    update_byte_to_flash(KINISI_DEVICE_COUNT,no_of_kinisi_device);
                    Serial.printf("\nno of kinsi registered in this room = %d\n",no_of_kinisi_device);
                    pClient->disconnect(); //don't forget to check device disconnection after sending ack to kinisi
                    Serial.println("Disconnected from Kinisi");
                    vTaskDelay(50);
                    //store kinisi registration status in flash
//                    update_byte_to_flash(KINISI_REG_STS_BASE_ADDR,kinisi_register_status);
                    register_kinisi = 0;
                    if ( no_of_kinisi_device >= BYTE1 )
                    {
                      kinisi_lookup_table[kinisi_reg_location][4] = itoa(kinisi_register_status,str_buffer,DECIMAL); //update kinisi registration status
                      print_Kinsi_lookup_table(kinisi_lookup_table);
                      update_byte_to_flash((KINISI_REG_ADDR + (no_of_kinisi_device - 1)),kinisi_register_status);
                    }
                    kinisi_register_status = false ;
                    //clear all buffer
                    for ( int indice = 0;indice < BLE_BUFFER_SIZE;indice++ )
                    {
                       kinisi_room_details[indice]  = '\0';
                       kinisi_serial_number[indice] = '\0';
                       kinisi_device_name[indice]   = '\0';
                       if ( indice <= BLE_BUFFER_SIZE/2 )
                       {
                          kinisi_auth_code[indice]  = '\0' ;
                       }
                    }
                  }
                }
                else
                {
                  if (  sensor  == KINISI )
                  {
                    kinisi_register_status = false ;
                    Serial.println("Kinisi registration status : Fail");
                    registration_packet_send_count = 0;
                    register_kinisi = 0;
                    send_ble_packets_to_kinisi(DEVICE_REG_ACK,EXIT_FAILURE);//send NACK to Kinisi
                    //update kinisi count
                    if( no_of_kinisi_device >= BYTE1)
                    {
                      no_of_kinisi_device -= 1;
                      update_byte_to_flash(KINISI_DEVICE_COUNT,no_of_kinisi_device);
                      Serial.printf("\nno of kinsi device available in this room = %d\n",no_of_kinisi_device);
                    }
                    pClient->disconnect();
                    Serial.println("Disconnected from Kinisi");
                    vTaskDelay(50);
                    //store kinisi registration status in flash
      //              update_byte_to_flash(KINISI_REG_STS_BASE_ADDR,kinisi_register_status);
                    update_byte_to_flash((KINISI_REG_ADDR + no_of_kinisi_device),kinisi_register_status);

                    kinisi_lookup_table[kinisi_reg_location][4] = itoa(kinisi_register_status,str_buffer,DECIMAL); //update kinisi registration status
                    print_Kinsi_lookup_table(kinisi_lookup_table);


                    //clear all buffer
                    for ( int indice = 0;indice < BLE_BUFFER_SIZE;indice++ )
                    {
                       kinisi_room_details[indice]  = '\0';
                       kinisi_serial_number[indice] = '\0';
                       kinisi_device_name[indice]   = '\0';
                       if ( indice <= BLE_BUFFER_SIZE/2 )
                       {
                          kinisi_auth_code[indice]  = '\0' ;
                       }
                    }
                  }
                }
              }
            }
            else if( strcmp(json_extract.device_id,(const char*)serial_number) == 0 )
            {//Check it is Amenos serial number or not
              if( json_extract.action == DEVICE_REG_ACK )
              {
                /***********Device ack packet extraction**********/
                Serial.println("\nAmenos Device registartion ack received");
                if ( json_extract.status && json_extract.ack_type )
                {
                  if ( sensor  == AMENOS )
                  {
                    device_register_status = true;
                    Serial.println("Amenos registered successfully");
                  }
                }
                else
                {
                  if ( sensor  == AMENOS )
                  {
                    device_register_status = false;
                    Serial.println("Amenos not registered");
                  }
                }
              }
            }
            else
            {
              Serial.println("Serial number mis match");
            }
          }
          else
          {
            Serial.println("Error : Hmac validation failed");
          }
          }
        }
      }
      else
      {
        if (  sensor  == KINISI )
        {
          kinisi_register_status = false ;
          Serial.println("Device registration ACK not received from Hub......");
          Serial.println("Kinisi registration status : Fail");
        }
      }
//    }
    if ( sensor == AMENOS )
    {
      if( device_register_status )
      {
        //store device register status in flash
        update_byte_to_flash(DEVICE_REG_STS_BASE_ADDR,device_register_status);
        //change operating mode to NORMAL_MODE
        operating_mode = NORMAL_MODE;
        update_byte_to_flash(OPERATING_MODE_BASE_ADDR,operating_mode);
        Serial.println("Changing mode to NORMAL MODE - Device registered successfully...");
        vTaskDelay(100);
        ESP.restart();
      }
      else
      {
        Serial.println("Device registration ACK not received from Hub......");
      }
    }

  }
  else if ( operation == NORMAL_RECEIVE_MODE )
  {
    // if there's data available, read a packet
    packetSize = Udp.parsePacket();
    if (packetSize)
    {//
      Serial.print("\nReceived packet size : ");
      Serial.println(packetSize);
      Serial.print("From ");
      IPAddress remoteIp = Udp.remoteIP();
      Serial.print(remoteIp);
      Serial.print(", port ");
      Serial.println(Udp.remotePort());
      error = deserializeJson(parsing_buffer, Udp);
      if( error )
      {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
      }
      else
      {
        Serial.println("parsing success");
        len = measureJsonPretty(parsing_buffer);
        if ( len > 0)
        {
          Serial.printf("\nparsed buffer length =%d\n",len);
          /*
           * print received data in serial terminal
           */
          Serial.println("Received data");
          serializeJsonPretty(parsing_buffer, Serial);//print in serial terminal
          #if 1
          serializeJson(parsing_buffer, parsing_buffer_copy);//make copy
          json_extract.hmac = parsing_buffer["hmac"];
          if( hmac_validation(parsing_buffer_copy) != EXIT_FAILURE) // validate hmac for received data
          {
            json_extract.device_id = parsing_buffer["payload"]["device_uid"];
            json_extract.action = parsing_buffer["payload"]["action"];
            Serial.printf("action num = %d\n",json_extract.action );
            if( strcmp(json_extract.device_id,(const char*)serial_number) == 0 )
            {//Check it is Amenos serial number or not
                //comes here any data received from hub other than device registration ack
                if( json_extract.action == MAINTENANCE_CMD )
                {
                  /**********maintenance packet extraction*****************/
                  json_extract.maintenance_mode = parsing_buffer["payload"]["device_data"][0]["maintenance_mode"];
                  /*
                   * Do action for maintenance packet
                   */
                  if ( json_extract.maintenance_mode == MAINTENANCE_MODE_ON )
                  {
                    Serial.println("Maintenance mode ON");
                    //send data to destination of ip address received via MOBILE_IP_INFO_CMD
                    enable_maintenance_mode = true;
                  }
                  else if ( json_extract.maintenance_mode == MAINTENANCE_MODE_OFF )
                  {
                    Serial.println("Maintenance mode OFF");
                    enable_maintenance_mode = false;
                  }
                  json_extract.maintenance_mode = 0;
                }
                else if( json_extract.action == FIRMWARE_UPDATE_CMD  )
                {
                  Serial.println("\nFirmware update command received");
                  /*******Firmware update data extraction*********/
                  json_extract.fm_version   = parsing_buffer["payload"]["device_data"][0]["firmware_version"];
                  json_extract.fm_path      = parsing_buffer["payload"]["device_data"][0]["firmware_path"];
                  json_extract.fm_crc       = parsing_buffer["payload"]["device_data"][0]["crc"];
                  json_extract.fm_file_size = parsing_buffer["payload"]["device_data"][0]["file_size"];
                  /************print OTAP data*********/
                  Serial.printf("\nFM Version : %s\n",json_extract.fm_version);
                  Serial.printf("FM Path      : %s\n",json_extract.fm_path);
                  Serial.printf("FM Crc       : %s\n",json_extract.fm_crc);
                  Serial.printf("FM Size      : %s\n",json_extract.fm_file_size);
                  UDP_Rx_Packet[OTAP]  = true;
                }
                else if( json_extract.action == MOBILE_IP_INFO_CMD  )
                {
                  Serial.println("\nMobile ip addres command received");
                  /*******mobile ip address extraction*********/
                  json_extract.ipaddress = parsing_buffer["payload"]["device_data"][0]["ipaddr"];
                  if( strcmp(json_extract.ipaddress,EMPTY_IP) == 0 )
                  {
                    enable_maintenance_mode = 0;
                    if ( WiFi.status() == WL_CONNECTED )
                    {
                      udp_ip = WiFi.gatewayIP().toString().c_str();
                    }
                    else
                    {
                      udp_ip = EMPTY_IP;
                    }
                    Serial.print("Server changed to Hub ip ");
                    Serial.println(udp_ip);
                  }
                  else
                  {
                    if(enable_maintenance_mode)
                    {
                      udp_ip = (char*)json_extract.ipaddress;
                      Serial.print("Server changed to mobile ip");
                      Serial.println(udp_ip);
                    }
                    else
                    {
                      Serial.println("Maintenance mode not enabled");
                    }
                  }
                }
                else if(json_extract.action == ENABLE_BLE_MESH)
                {
                  json_extract.ble_mesh = parsing_buffer["payload"]["device_data"][0]["enable_mesh"];
                  if( json_extract.ble_mesh )
                  {
                    UDP_Rx_Packet[BLE_MESH] = true;
                    Serial.println("Enbale BLE mesh");
                  }
                  else
                  {
                    UDP_Rx_Packet[BLE_MESH] = false;
                  }

                }
                else if( json_extract.action == SHUTDOWN_AND_CTRL_CMD  )
                {
                  json_extract.command_type = parsing_buffer["payload"]["command_type"];
                  if( json_extract.command_type == SHUTDOWN_CMD_TYPE)
                  {
                    /*******senor on/off extraction***********/
                    Serial.println("\nSensor shutdown command received");
                    json_extract.amenity_name = parsing_buffer["payload"]["device_data"][0]["amenity-name"];
                    json_extract.poweroff     = parsing_buffer["payload"]["device_data"][0]["turn_off_power"];
                    Serial.printf("amenity name   : %s\n",json_extract.amenity_name);
                    /*
                     * do action for sensor ON/OFF packet
                     */
                    if( strcmp(json_extract.device_id,(const char*)room_no) == 0 )//Need clarification
                    {
                      Serial.println("\ndevice_uid is HUB SSID");
                      if ( json_extract.poweroff == POWER_OFF )
                      {
                        //enter sleep mode
                        Serial.println("ESP entering into sleep mode");
                        vTaskDelay(5000);
                        esp_deep_sleep_start(); //deep sleep not tested
                      }
                      else if (json_extract.poweroff == POWER_ON )
                      {
                        //don't enter sleep mode
                        Serial.println("Operate in normal mode");
                      }
                    }
                    else
                    {
                      Serial.println("\ndevice_uid is not HUB SSID");
                    }
                    json_extract.poweroff = POWER_ON;
                  }
                  else if( json_extract.command_type == AC_CTRL_CMD_TYPE  )
                  {
                    Serial.println("\nAC control command received");
                    /*******AC control data extraction*********/
                    json_extract.ac_status  = parsing_buffer["payload"]["device_data"][0]["ac_status"];
                    json_extract.ac_mode    = parsing_buffer["payload"]["device_data"][0]["mode"];
                    json_extract.ac_temp    = parsing_buffer["payload"]["device_data"][0]["ac_temperature"];
                    UDP_Rx_Packet[AC_CTRL]  = true;
                    /****Store AC setting in Flash****/
                    update_flash(SET_AC_MODE_BASE_ADDR,(uint8_t *)json_extract.ac_mode,strlen(json_extract.ac_mode));
                    update_byte_to_flash(SET_AC_STATUS_BASE_ADDR,json_extract.ac_status);
                    update_byte_to_flash(SET_AC_TEMP_BASE_ADDR,json_extract.ac_temp);
                    display_temperature(json_extract.ac_temp);
                    maintain_ac_setting();//update the AC setting

      //                  /******print AC type**********/
      //                  if( json_extract.ac_type == SPLIT_AC_TYPE )
      //                  {
      //                    Serial.println("Selected AC type -  SPLIT AC");
      //                  }
      //                  else if( json_extract.ac_type == HVAC_TYPE )
      //                  {
      //                    Serial.println("Selected AC type -  HVAC");
      //                  }
      //                  else
      //                  {
      //                    Serial.printf("AC type mis - match : %d\n",json_extract.ac_type);
      //                  }

                  }
                  else if( json_extract.command_type == ZONE_LIGHT_CTRL_CMD_TYPE  )
                  {
                    Serial.println("\nZone light control command received");
                    /*******Zone light control data extraction*********/
                    json_extract.light_color    = parsing_buffer["payload"]["device_data"][0]["color"];
                    json_extract.light_status   = parsing_buffer["payload"]["device_data"][0]["light_status"];
                    json_extract.light_contrast = parsing_buffer["payload"]["device_data"][0]["contrast"];
                    json_extract.zone           = parsing_buffer["payload"]["device_data"][0]["zone"];
                    UDP_Rx_Packet[LIGHT_CTRL] = true;
                    sprintf(CTRL_LIGHT,"AT+ZLC:%d,0x%x,%d,%d\n",json_extract.zone,json_extract.light_color,json_extract.light_status,json_extract.light_contrast);
                    Serial.println(CTRL_LIGHT);
                    /******print light color**********/
                    if( json_extract.light_color > 0 )
                    {
                      Serial.printf("Light color : %x\n",json_extract.light_color);
                    }
                    else
                    {
                      Serial.printf("Light color mis - match : %x\n",json_extract.light_color);
                    }

                    /******print light status**********/
                    if( json_extract.light_status == DEVICE_TURN_ON )
                    {
                      Serial.println("Light status : ON");
                    }
                    else if( json_extract.light_status == DEVICE_TURN_OFF )
                    {
                      Serial.println("Light status : OFF");
                    }
                    else
                    {
                      Serial.printf("Light status mis - match : %x\n",json_extract.light_status);
                    }
                    /******print light contrast**********/
                    if( json_extract.light_contrast >= 0 )
                    {
                      Serial.printf("Light contrast : %ld\n",json_extract.light_contrast);
                    }
                    else
                    {
                      json_extract.light_contrast = 15;//minimum light contrast
                    }

                     /******print light zone**********/
                    if( json_extract.zone == NIGHT_LAMP )
                    {
                      Serial.println("zone : Night Lamp");
                    }
                    else if( json_extract.zone == LIVING_AREA )
                    {
                      Serial.println("zone : Living Area");
                    }
                    else if( json_extract.zone == CORRIDOR )
                    {
                      Serial.println("zone : Corridor");
                    }
                    else
                    {
                      Serial.printf("Light zone mis - match : %d\n",json_extract.zone);
                    }
                  }
                  else
                  {
                    Serial.println("\n received command type + String(json_extract.command_type)");
                  }
                }
                else if (json_extract.action == DECOMMISSION_CMD  )
                {
                  /******** Decommissioning data extraction********/
                  Serial.println("\nDecommission command received");
                  /*
                   * Do action for Decommissioning packet
                   */
                   json_extract.individual_light_macid  = parsing_buffer["payload"]["device_data"][0]["address_id"];
                   json_extract.kinisi_serial_no        = parsing_buffer["payload"]["device_data"][0]["Kinisi_uid"];
                   if( (json_extract.individual_light_macid == false) && (json_extract.kinisi_serial_no == false))
                   {
                      Serial.println("Amenos Decommission command");
                      //clear flash data
                      flush_out_Flash();
                      //Go back to CONFIGURATION_MODE
                      //change operating mode to CONFIGURATION_MODE
                      operating_mode = CONFIGURATION_MODE ;
                      //store operating mode in flash
                      update_byte_to_flash(OPERATING_MODE_BASE_ADDR,operating_mode);
                      //restart ESP to change the operating mode
                      Serial.println("Changing mode to CONFIGURATION_MODE");
                      vTaskDelay(1000);
                      ESP.restart(); //this is to initialize configuration mode parameters
                   }
                   else if( (json_extract.kinisi_serial_no != false) && (json_extract.individual_light_macid == false))
                   {
                      Serial.println("Kinisi decommission command");
                      kinisi_decommission_serial_num[device_count[KINISI_DECOMMISSION_COUNT]++] = (char *)json_extract.kinisi_serial_no;
                      /************updtae Kinisi decommission device count in flash*****************/
                      update_byte_to_flash(KINISI_DECOM_DEVICE_COUNT,device_count[KINISI_DECOMMISSION_COUNT]);
                      Serial.println("\nList of decommissioned Kinisi Serial Number");
                      for (int i = 0; i < device_count[KINISI_DECOMMISSION_COUNT]; i++)
                      {
                        Serial.println((char *)kinisi_decommission_serial_num[i]);
                      }
                      print_Kinsi_lookup_table(kinisi_lookup_table);
                      for(int indice = 0;indice < device_count[KINISI_DECOMMISSION_COUNT]; indice++)
                      {
                        for (int i = 0; i < no_of_kinisi_device; i++)
                        {
                          if( kinisi_lookup_table[i][1]  == (String)kinisi_decommission_serial_num[i])
                          {
                            Serial.printf("match found @ %d\n",i);
                            kinisi_lookup_table[i][3] = itoa(1,str_buffer,10) ;
                          }
                        }
                      }
                      print_Kinsi_lookup_table(kinisi_lookup_table);
                   }
                   else
                   {
                       Serial.println("Light decommission command");
                       UDP_Rx_Packet[LIGHT_DECOMMISSION] = true;
                       sprintf(DECOMMISSION_LIGHT,"AT+LDCOM:%s\n",json_extract.individual_light_macid);
                       Serial.println(DECOMMISSION_LIGHT);
                   }
                }
                else if( json_extract.action == REG_REQ_CMD  )
                {
                  Serial.println("\nRegistration request command received");
                  /*******Registration request data extraction*********/
                  json_extract.reg_required = parsing_buffer["payload"]["device_data"][0]["reg_required"];
                  if( json_extract.reg_required )
                  {
                     Json_post_to_AP(DEVICE_REGISTRATION,AMENOS,NULL,NULL);
                  }
                  else
                  {
                    Serial.printf("\nreg status rcvd : %d\n",json_extract.reg_required);
                  }
                }
                else if( json_extract.action == DEFAULT_SETTING_CMD  )
                {
                  json_extract.default_type = parsing_buffer["payload"]["default_type"];
                  if( json_extract.default_type == AC_DEFAULT_TYPE  )
                  {
                    Serial.println("\nAC default setting command received");
                    /*******AC default setting data extraction*********/
                    json_extract.from_time                = parsing_buffer["payload"]["device_data"][0]["schedule"][0]["from_time"];
                    json_extract.to_time                  = parsing_buffer["payload"]["device_data"][0]["schedule"][0]["to_time"];
                    json_extract.ac_mode                  = parsing_buffer["payload"]["device_data"][0]["schedule"][0]["mode"];
                    default_ac_setting_table[MODE001][0]  = json_extract.ac_mode;
                    default_ac_setting_table[MODE001][1]  = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["temperature"],str_buffer,DECIMAL);
                    default_ac_setting_table[MODE001][2]  = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["ac_status"],str_buffer,DECIMAL);
                    default_ac_setting_table[MODE001][3]  = json_extract.from_time;
                    default_ac_setting_table[MODE001][4]  = json_extract.to_time;
                    json_extract.from_time = NULL;
                    json_extract.to_time = NULL;
                    default_cool_mode_AC_temperature = atoi(default_ac_setting_table[MODE001][1].c_str());

                    json_extract.from_time = parsing_buffer["payload"]["device_data"][0]["schedule"][1]["from_time"];
                    json_extract.to_time = parsing_buffer["payload"]["device_data"][0]["schedule"][1]["to_time"];
                    json_extract.ac_mode = parsing_buffer["payload"]["device_data"][0]["schedule"][1]["mode"];
                    default_ac_setting_table[MODE101][0] = json_extract.ac_mode;
                    default_ac_setting_table[MODE101][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["temperature"],str_buffer,DECIMAL);
                    default_ac_setting_table[MODE101][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["ac_status"],str_buffer,DECIMAL);
                    default_ac_setting_table[MODE101][3] = json_extract.from_time;
                    default_ac_setting_table[MODE101][4] = json_extract.to_time;
                    json_extract.from_time = NULL;
                    json_extract.to_time = NULL;

                    json_extract.from_time = parsing_buffer["payload"]["device_data"][0]["schedule"][2]["from_time"];
                    json_extract.to_time = parsing_buffer["payload"]["device_data"][0]["schedule"][2]["to_time"];
                    json_extract.ac_mode = parsing_buffer["payload"]["device_data"][0]["schedule"][2]["mode"];
                    default_ac_setting_table[MODE100][0] = json_extract.ac_mode;
                    default_ac_setting_table[MODE100][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][2]["temperature"],str_buffer,DECIMAL);
                    default_ac_setting_table[MODE100][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][2]["ac_status"],str_buffer,DECIMAL);
                    default_ac_setting_table[MODE100][3] = json_extract.from_time;
                    default_ac_setting_table[MODE100][4] = json_extract.to_time;
                    json_extract.from_time = NULL;
                    json_extract.to_time = NULL;

                    /***********Cool Mode From Time & To Time*************/
                    From_Time[MODE001]  = get_ac_mode_change_time(default_ac_setting_table[MODE001][3]);
                    To_Time[MODE001]    = get_ac_mode_change_time(default_ac_setting_table[MODE001][4]);

                    /***********Fan Mode From Time & To Time*************/
                    From_Time[MODE101]  = get_ac_mode_change_time(default_ac_setting_table[MODE101][3]);
                    To_Time[MODE101]    = get_ac_mode_change_time(default_ac_setting_table[MODE101][4]);

                    /***********Dry Mode From Time & To Time*************/
                    From_Time[MODE100]  = get_ac_mode_change_time(default_ac_setting_table[MODE100][3]);
                    To_Time[MODE100]    = get_ac_mode_change_time(default_ac_setting_table[MODE100][4]);

                    maintain_ac_setting();

                    print_default_table(default_ac_setting_table,AIR_CONDITIONER);
                  }
                  else if( json_extract.default_type == LIGHT_DEFAULT_TYPE  )
                  {
                    Serial.println("\nLight default setting command received");
                    /*******Light default setting data extraction*********/
                    json_extract.from_time = parsing_buffer["payload"]["device_data"][0]["schedule"][0]["from_time"];
                    json_extract.to_time = parsing_buffer["payload"]["device_data"][0]["schedule"][0]["to_time"];

                    default_light_setting_table0[ZONE1][0] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][0]["zone"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE1][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][0]["color"],str_buffer,HEX_VALUE);
                    default_light_setting_table0[ZONE1][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][0]["light_status"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE1][3] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][0]["contrast"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE1][4] = json_extract.from_time;
                    default_light_setting_table0[ZONE1][5] = json_extract.to_time;

                    default_light_setting_table0[ZONE2][0] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][1]["zone"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE2][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][1]["color"],str_buffer,HEX_VALUE);
                    default_light_setting_table0[ZONE2][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][1]["light_status"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE2][3] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][1]["contrast"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE2][4] = json_extract.from_time;
                    default_light_setting_table0[ZONE2][5] = json_extract.to_time;

                    default_light_setting_table0[ZONE3][0] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][2]["zone"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE3][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][2]["color"],str_buffer,HEX_VALUE);
                    default_light_setting_table0[ZONE3][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][2]["light_status"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE3][3] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][2]["contrast"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE3][4] = json_extract.from_time;
                    default_light_setting_table0[ZONE3][5] = json_extract.to_time;

                    json_extract.from_time = NULL;
                    json_extract.to_time = NULL;
                    json_extract.from_time = parsing_buffer["payload"]["device_data"][0]["schedule"][1]["from_time"];
                    json_extract.to_time = parsing_buffer["payload"]["device_data"][0]["schedule"][1]["to_time"];

                    default_light_setting_table1[ZONE1][0] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][0]["zone"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE1][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][0]["color"],str_buffer,HEX_VALUE);
                    default_light_setting_table1[ZONE1][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][0]["light_status"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE1][3] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][0]["contrast"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE1][4] = json_extract.from_time;
                    default_light_setting_table1[ZONE1][5] = json_extract.to_time;

                    default_light_setting_table1[ZONE2][0] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][1]["zone"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE2][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][1]["color"],str_buffer,HEX_VALUE);
                    default_light_setting_table1[ZONE2][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][1]["light_status"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE2][3] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][1]["contrast"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE2][4] = json_extract.from_time;
                    default_light_setting_table1[ZONE2][5] = json_extract.to_time;

                    default_light_setting_table1[ZONE3][0] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][2]["zone"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE3][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][2]["color"],str_buffer,HEX_VALUE);
                    default_light_setting_table1[ZONE3][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][2]["light_status"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE3][3] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][2]["contrast"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE3][4] = json_extract.from_time;
                    default_light_setting_table1[ZONE3][5] = json_extract.to_time;

                    json_extract.from_time = NULL;
                    json_extract.to_time = NULL;

                    print_default_table(default_light_setting_table0,SMART_BULB);
                    print_default_table(default_light_setting_table1,SMART_BULB);
                  }
                  json_extract.default_type = 0;
                }
                else if( json_extract.action == LIGHT_CONFIG_CMD  )
                {
                  Serial.println("\nEnable  light  pairing mode command received");
                  /*******Enable light pairing mode data extraction*********/
                  json_extract.zone               = parsing_buffer["payload"]["device_data"][0]["zone"];
                  json_extract.zone_light_epid    = parsing_buffer["payload"]["device_data"][0]["address_id"];
                  json_extract.light_mfg_date     = parsing_buffer["payload"]["device_data"][0]["mfg_date"];
                  json_extract.light_org_uid      = parsing_buffer["payload"]["device_data"][0]["org_uid"];
                  json_extract.light_facility_uid = parsing_buffer["payload"]["device_data"][0]["facility_uid"];
                  json_extract.light_device_name  = parsing_buffer["payload"]["device_data"][0]["device_name"];
                  json_extract.light_amenity_name = parsing_buffer["payload"]["device_data"][0]["amenity_name"];
                  /******print light zone**********/
                  if( json_extract.zone == NIGHT_LAMP )
                  {
                    Serial.println("zone : Night Lamp");
                  }
                  else if( json_extract.zone == LIVING_AREA )
                  {
                    Serial.println("zone : Living Area");
                  }
                  else if( json_extract.zone == CORRIDOR )
                  {
                    Serial.println("zone : Corridor");
                  }
                  else
                  {
                    Serial.printf("Light zone mis - match : %d\n",json_extract.zone);
                  }
                  Serial.printf("\nzone_light_epid  :%s\n",json_extract.zone_light_epid);
                  Serial.printf("light_mfg_date     :%s\n",json_extract.light_mfg_date);
                  Serial.printf("light_org_uid      :%s\n",json_extract.light_org_uid);
                  Serial.printf("light_facility_uid :%s\n",json_extract.light_facility_uid);
                  Serial.printf("light_device_name  :%s\n",json_extract.light_device_name);
                  Serial.printf("light_amenity_name :%s\n",json_extract.light_amenity_name);

                  if(strcmp(json_extract.light_amenity_name,(const char*)room_no) == 0)
                  {
                    if(strcmp(json_extract.light_org_uid,(const char*)org_id) == 0)
                    {
                      if(strcmp(json_extract.light_facility_uid,(const char*)facility_id) == 0)
                      {
                        if(strcmp(json_extract.light_device_name,(const char*)device_name) == 0)
                        {
                          UDP_Rx_Packet[LIGHT_CONFIG] = true;
                          sprintf(CONFIG_LIGHT,"AT+NODE:%s,%d\n",json_extract.zone_light_epid,json_extract.zone);
                          Serial.println(CONFIG_LIGHT);
                          Serial.println("Room details matched");
                        }
                        else
                        {
                          UDP_Rx_Packet[LIGHT_CONFIG] = false;
                          Serial.println("\nDevice name mis match");
                        }
                      }
                      else
                      {
                        UDP_Rx_Packet[LIGHT_CONFIG] = false;
                        Serial.println("\nFacility Id mis match");
                      }
                    }
                    else
                    {
                      UDP_Rx_Packet[LIGHT_CONFIG] = false;
                      Serial.println("\nOrg Id mis match");
                    }
                  }
                  else
                  {
                    UDP_Rx_Packet[LIGHT_CONFIG] = false;
                    Serial.println("\nRoom number mis match");
                  }
                  /*******print individual light epid*******/
                  //parse_macid((char*)json_extract.zone_light_epid,COMMA_DELIMITER);
                }
                else if( json_extract.action == LOCK_TOUCH_BUTTON_CMD  )
                {
                  Serial.println("\nLock touch button command received");
                  /*******Lock touch button data extraction*********/
                  json_extract.status = parsing_buffer["payload"]["device_data"][0]["status"];
                  if( json_extract.status )
                  {
                    Serial.println("Lock touch button");
                    UDP_Rx_Packet[LOCK_BUTTON] = true;
                  }
                  else
                  {
                    Serial.println("Unlock touch button");
                    UDP_Rx_Packet[LOCK_BUTTON] = false;
                  }
                }
                else if( json_extract.action == FORCE_SET_AC_TEMP_CMD  )
                {
                  Serial.println("\nForce set temperature command received");
                  /*******Force set temperature data extraction*********/
                  json_extract.status = parsing_buffer["payload"]["device_data"][0]["status"];
                  json_extract.ac_type = parsing_buffer["payload"]["device_data"][0]["ac_type"];
                  json_extract.ac_status = parsing_buffer["payload"]["device_data"][0]["ac_status"];
                  json_extract.ac_temp = parsing_buffer["payload"]["device_data"][0]["temperature"];
                  if( json_extract.status )
                  {
                    json_extract.status = 0;
                    Serial.println("Enable force set AC temperature");
                    UDP_Rx_Packet[FORCE_SET_PACKET] = true;
                  }
                  else
                  {
                    Serial.println("Disable force set AC temperature");
                    UDP_Rx_Packet[FORCE_SET_PACKET] = false;
                  }

                   /********print AC status*******/
                  if( json_extract.ac_status == DEVICE_TURN_ON )
                  {
                    Serial.println("Turning ON AC");
                  }
                  else if( json_extract.ac_status == DEVICE_TURN_OFF )
                  {
                    Serial.println("Turning OFF AC");
                  }
                  else
                  {
                    Serial.printf("AC status mis - match : %d\n",json_extract.ac_status);
                  }
                  if ( (json_extract.ac_temp >= MIN_TEMPERATURE) && (json_extract.ac_temp <= MAX_TEMPERATURE) )
                  {
                    Serial.printf("AC temperature :%d\n",json_extract.ac_temp);
                  }
                  else
                  {
                    Serial.printf("AC temperature not in range (16 - 28):%d\n",json_extract.ac_temp);
                  }
                }
                else if( json_extract.action == AMENITY_SLEEP_MODE_SETTING_CMD  )
                {
                  Serial.println("\nAmenity sleep mode command received");
                  /*******Amenity sleep mode data extraction*********/
                  json_extract.status = parsing_buffer["payload"]["device_data"][0]["status"];
                  if ( json_extract.status )
                  {
                      Serial.println("Enable sleep mode setting");
                      UDP_Rx_Packet[AMENITY_SLEEP_MODE] = true;
                  }
                  else
                  {
                    UDP_Rx_Packet[AMENITY_SLEEP_MODE] = false;
                    Serial.println("Disable sleep mode setting");
                  }
                }
                else if( json_extract.action == OVER_TEMP_WAKE_AFTER_CMD  )
                {
                  Serial.println("\nOver temperature wake after command received");
                  /*******Over temperature wake after data extraction*********/
                  json_extract.status = parsing_buffer["payload"]["device_data"][0]["status"];
                  if( json_extract.status )
                  {
                    json_extract.status = 0;
                    Serial.println("Over temperature detected");
                    UDP_Rx_Packet[OVER_TEMP] = true;
                  }
                  else
                  {
                    Serial.println("No over temperature");
                    UDP_Rx_Packet[OVER_TEMP] = false;
                  }
                  update_byte_to_flash(OVER_TEMP_STATUS_BASE_ADDR,UDP_Rx_Packet[OVER_TEMP]);
                }
                else if( json_extract.action == DEFAULT_BROADCAST_CMD  )
                {
                  json_extract.default_type = parsing_buffer["payload"]["default_type"];
                  if( json_extract.default_type == AC_DEFAULT_TYPE  )
                  {
                    Serial.println("\nAC default setting command received");
                    /*******AC default setting data extraction*********/
                    json_extract.from_time                = parsing_buffer["payload"]["device_data"][0]["schedule"][0]["from_time"];
                    json_extract.to_time                  = parsing_buffer["payload"]["device_data"][0]["schedule"][0]["to_time"];
                    json_extract.ac_mode                  = parsing_buffer["payload"]["device_data"][0]["schedule"][0]["mode"];
                    default_ac_setting_table[MODE001][0]  = json_extract.ac_mode;
                    default_ac_setting_table[MODE001][1]  = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["temperature"],str_buffer,DECIMAL);
                    default_ac_setting_table[MODE001][2]  = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["ac_status"],str_buffer,DECIMAL);
                    default_ac_setting_table[MODE001][3]  = json_extract.from_time;
                    default_ac_setting_table[MODE001][4]  = json_extract.to_time;
                    json_extract.from_time = NULL;
                    json_extract.to_time = NULL;
                    default_cool_mode_AC_temperature = atoi(default_ac_setting_table[MODE001][1].c_str());

                    json_extract.from_time = parsing_buffer["payload"]["device_data"][0]["schedule"][1]["from_time"];
                    json_extract.to_time = parsing_buffer["payload"]["device_data"][0]["schedule"][1]["to_time"];
                    json_extract.ac_mode = parsing_buffer["payload"]["device_data"][0]["schedule"][1]["mode"];
                    default_ac_setting_table[MODE101][0] = json_extract.ac_mode;
                    default_ac_setting_table[MODE101][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["temperature"],str_buffer,DECIMAL);
                    default_ac_setting_table[MODE101][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["ac_status"],str_buffer,DECIMAL);
                    default_ac_setting_table[MODE101][3] = json_extract.from_time;
                    default_ac_setting_table[MODE101][4] = json_extract.to_time;
                    json_extract.from_time = NULL;
                    json_extract.to_time = NULL;

                    json_extract.from_time = parsing_buffer["payload"]["device_data"][0]["schedule"][2]["from_time"];
                    json_extract.to_time = parsing_buffer["payload"]["device_data"][0]["schedule"][2]["to_time"];
                    json_extract.ac_mode = parsing_buffer["payload"]["device_data"][0]["schedule"][2]["mode"];
                    default_ac_setting_table[MODE100][0] = json_extract.ac_mode;
                    default_ac_setting_table[MODE100][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][2]["temperature"],str_buffer,DECIMAL);
                    default_ac_setting_table[MODE100][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][2]["ac_status"],str_buffer,DECIMAL);
                    default_ac_setting_table[MODE100][3] = json_extract.from_time;
                    default_ac_setting_table[MODE100][4] = json_extract.to_time;
                    json_extract.from_time = NULL;
                    json_extract.to_time = NULL;

                    /***********Cool Mode From Time & To Time*************/
                    From_Time[MODE001]  = get_ac_mode_change_time(default_ac_setting_table[MODE001][3]);
                    To_Time[MODE001]    = get_ac_mode_change_time(default_ac_setting_table[MODE001][4]);

                    /***********Fan Mode From Time & To Time*************/
                    From_Time[MODE101]  = get_ac_mode_change_time(default_ac_setting_table[MODE101][3]);
                    To_Time[MODE101]    = get_ac_mode_change_time(default_ac_setting_table[MODE101][4]);

                    /***********Dry Mode From Time & To Time*************/
                    From_Time[MODE100]  = get_ac_mode_change_time(default_ac_setting_table[MODE100][3]);
                    To_Time[MODE100]    = get_ac_mode_change_time(default_ac_setting_table[MODE100][4]);

                    maintain_ac_setting();

                    print_default_table(default_ac_setting_table,AIR_CONDITIONER);
                  }
                  else if( json_extract.default_type == LIGHT_DEFAULT_TYPE  )
                  {
                    Serial.println("\nLight default setting command received");
                    /*******Light default setting data extraction*********/
                    json_extract.from_time = parsing_buffer["payload"]["device_data"][0]["schedule"][0]["from_time"];
                    json_extract.to_time = parsing_buffer["payload"]["device_data"][0]["schedule"][0]["to_time"];

                    default_light_setting_table0[ZONE1][0] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][0]["zone"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE1][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][0]["color"],str_buffer,HEX_VALUE);
                    default_light_setting_table0[ZONE1][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][0]["light_status"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE1][3] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][0]["contrast"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE1][4] = json_extract.from_time;
                    default_light_setting_table0[ZONE1][5] = json_extract.to_time;

                    default_light_setting_table0[ZONE2][0] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][1]["zone"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE2][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][1]["color"],str_buffer,HEX_VALUE);
                    default_light_setting_table0[ZONE2][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][1]["light_status"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE2][3] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][1]["contrast"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE2][4] = json_extract.from_time;
                    default_light_setting_table0[ZONE2][5] = json_extract.to_time;

                    default_light_setting_table0[ZONE3][0] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][2]["zone"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE3][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][2]["color"],str_buffer,HEX_VALUE);
                    default_light_setting_table0[ZONE3][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][2]["light_status"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE3][3] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][0]["zone_settings"][2]["contrast"],str_buffer,DECIMAL);
                    default_light_setting_table0[ZONE3][4] = json_extract.from_time;
                    default_light_setting_table0[ZONE3][5] = json_extract.to_time;

                    json_extract.from_time = NULL;
                    json_extract.to_time = NULL;
                    json_extract.from_time = parsing_buffer["payload"]["device_data"][0]["schedule"][1]["from_time"];
                    json_extract.to_time = parsing_buffer["payload"]["device_data"][0]["schedule"][1]["to_time"];

                    default_light_setting_table1[ZONE1][0] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][0]["zone"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE1][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][0]["color"],str_buffer,HEX_VALUE);
                    default_light_setting_table1[ZONE1][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][0]["light_status"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE1][3] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][0]["contrast"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE1][4] = json_extract.from_time;
                    default_light_setting_table1[ZONE1][5] = json_extract.to_time;

                    default_light_setting_table1[ZONE2][0] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][1]["zone"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE2][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][1]["color"],str_buffer,HEX_VALUE);
                    default_light_setting_table1[ZONE2][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][1]["light_status"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE2][3] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][1]["contrast"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE2][4] = json_extract.from_time;
                    default_light_setting_table1[ZONE2][5] = json_extract.to_time;

                    default_light_setting_table1[ZONE3][0] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][2]["zone"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE3][1] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][2]["color"],str_buffer,HEX_VALUE);
                    default_light_setting_table1[ZONE3][2] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][2]["light_status"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE3][3] = itoa(parsing_buffer["payload"]["device_data"][0]["schedule"][1]["zone_settings"][2]["contrast"],str_buffer,DECIMAL);
                    default_light_setting_table1[ZONE3][4] = json_extract.from_time;
                    default_light_setting_table1[ZONE3][5] = json_extract.to_time;

                    json_extract.from_time = NULL;
                    json_extract.to_time = NULL;

                    print_default_table(default_light_setting_table0,SMART_BULB);
                    print_default_table(default_light_setting_table1,SMART_BULB);
                  }
                  json_extract.default_type = 0;
                }
                else
                {
                  Serial.println("check for ack frame");
                }
                json_extract.action = 0;
            }
            else
            {
              Serial.println("Error : Amenos device uid mis-match");
            }
          }
          else
          {
            Serial.println("Error : Hmac validation failed");
          }
          #endif
        }
      }
    }
    else
    {
//      Serial.print("Server respone: ");
//      Serial.println(packetSize);
    }
  }
}
/**********************************************************************//**
 * @brief  Print status of amenos
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void print_amenos_status()
{
  Serial.println("\nAMENOS status");
  if( operating_mode == CONFIGURATION_MODE )
  {
    Serial.printf("Operating mode : CONFIGURATION MODE\n");
  }
  else if ( operating_mode == NORMAL_MODE )
  {
    Serial.printf("Operating mode : NORMAL MODE\n");
  }
  else if ( operating_mode == OTAP_MODE)
  {
    Serial.printf("Operating mode : OTAP MODE\n");
  }
  if( operating_mode != CONFIGURATION_MODE )
  {
    Serial.printf("Serial Number  : %s\n",serial_number);
    Serial.printf("AP -> ssid     : %s\n",ap_ssid);
    Serial.printf("AP -> Password : %s\n",ap_passkey);
  }
  else
  {
    Serial.println("Not yet configured");
  }
}

/**********************************************************************//**
 * @brief  read data from flash and update in respective buffer
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void read_flash()
{
  uint16_t read_kinisi_count = 0;
  //read data from flash and update in respective buffer
    for (uint16_t location = FLASH_BASE_ADDRESS;location < (AP_PASS_KEY_BASE_ADDR + AP_PASS_KEY_ALLOCATED_BYTE);location++)
    {
      if(location == FLASH_BASE_ADDRESS)
      {
        //read opearting mode and update in variable operating_mode
        operating_mode = EEPROM.readUChar(OPERATING_MODE_BASE_ADDR);
      }
      else if ((location >= SERIAL_NUM_BASE_ADDR) && location < (SERIAL_NUM_BASE_ADDR + SERIAL_NUM_ALLOCATED_BYTE) )
      {
        //read serial number and update in serial_number buffer
        serial_number[location-SERIAL_NUM_BASE_ADDR] = EEPROM.readUChar(location);
      }
      else if ( (location >= AP_SSID_BASE_ADDR)  && location < (AP_SSID_BASE_ADDR + AP_SSID_ALLOCATED_BYTE) )
      {
        //read access point ssid and update in ap_ssid buffer
         ap_ssid[location-AP_SSID_BASE_ADDR] = EEPROM.readUChar(location);
      }
      else if ((location >= AP_PASS_KEY_BASE_ADDR) && location < (AP_PASS_KEY_BASE_ADDR + AP_PASS_KEY_ALLOCATED_BYTE) )
      {
        //read access point password and update in ap_passkey buffer
        ap_passkey[location-AP_PASS_KEY_BASE_ADDR] = EEPROM.readUChar(location);
      }
    }
    for (uint16_t location = ORG_ID_BASE_ADDR;location < (DEVICE_NAME_BASE_ADDR + BYTE11);location++)
    {
      flash_read_data[location - ORG_ID_BASE_ADDR] = EEPROM.readUChar(location);
    }
    for (uint16_t location = AUTH_CODE_BASE_ADDR;location < (AUTH_CODE_BASE_ADDR + BYTE8);location++)
    {
      amenos_auth_code[location - AUTH_CODE_BASE_ADDR] = EEPROM.readUChar(location);
    }
    amenos_auth_code[BYTE8]  = '\0';
    Length[SENSOR_AUTH_CODE]    = strlen((char*)amenos_auth_code);//store array length of auth code
    //store read data in respective buffer
    /*********Get Org ID/Propert Name ***********/
     org_id[BYTE0]= flash_read_data[BYTE0];
     org_id[BYTE1]= flash_read_data[BYTE1];
     org_id[BYTE2]= flash_read_data[BYTE2];
     org_id[BYTE3]= '\0'; // add delimiter
     Length[ORG_ID] = strlen((char*)org_id);//store array length of pass key
    /*********Get Facility ID**************/
     facility_id[BYTE0]= flash_read_data[BYTE3];
     facility_id[BYTE1]= flash_read_data[BYTE4];
     facility_id[BYTE2]= flash_read_data[BYTE5];
     facility_id[BYTE3]= '\0'; // add delimiter
     Length[FACILITY_ID] = strlen((char*)facility_id);//store array length of pass key
     /**********Get Room No***************/
     room_no[BYTE0]= flash_read_data[BYTE6];
     room_no[BYTE1]= flash_read_data[BYTE7];
     room_no[BYTE2]= flash_read_data[BYTE8];
     room_no[BYTE3]= flash_read_data[BYTE9];
     room_no[BYTE4]= flash_read_data[BYTE10];
     room_no[BYTE5]= flash_read_data[BYTE11];
     room_no[BYTE6]= '\0'; // add delimiter
     Length[ROOM_NUM] = strlen((char*)room_no);//store array length of pass key
     device_name[BYTE0]= flash_read_data[BYTE12];
     device_name[BYTE1]= flash_read_data[BYTE13];
     device_name[BYTE2]= flash_read_data[BYTE14];
     device_name[BYTE3]= flash_read_data[BYTE15];
     device_name[BYTE4]= flash_read_data[BYTE16];
     device_name[BYTE5]= flash_read_data[BYTE17];
     device_name[BYTE6]= flash_read_data[BYTE18];
     device_name[BYTE7]= flash_read_data[BYTE19];
     device_name[BYTE8]= flash_read_data[BYTE20];
     device_name[BYTE9]= flash_read_data[BYTE21];
     device_name[BYTE10]= flash_read_data[BYTE22];
     device_name[BYTE11]= '\0'; // add delimiter


    Serial.print("Org id: ");
    print_uint_data(org_id,Length[ORG_ID]);
    Serial.print("\nFacility id:");
    print_uint_data(facility_id,Length[FACILITY_ID]);
    Serial.print("\nRoom no:");
    print_uint_data(room_no,Length[ROOM_NUM]);
    Serial.print("\nDevice name:");
    print_uint_data(device_name,Length[DEVICE_NAME]);
    Serial.print("\nssid:");
    print_uint_data(ap_ssid,Length[WIFI_SSID]);
    Serial.print("\npassword:");
    print_uint_data(ap_passkey,Length[WIFI_PASSWORD]);
    Serial.print("\nAuth code:");
    print_uint_data(amenos_auth_code,Length[SENSOR_AUTH_CODE]);

    //read 1 byte data
    device_register_status  = EEPROM.readUChar(DEVICE_REG_STS_BASE_ADDR);
    tirisi_ac_type          = EEPROM.readUChar(AC_TYPE_BASE_ADDR);
    tirisi_ac_brand         = EEPROM.readUChar(AC_BRAND_BASE_ADDR);
    light_control           = EEPROM.readUChar(LIGHT_CTRL_BASE_ADDR);
    occupancy_detection     = EEPROM.readUChar(OCCUPANCY_BASE_ADDR);
    device_type             = EEPROM.readUChar(DEVICE_TYPE_BASE_ADDR);
    max_kinisi_in_room      = EEPROM.readUChar(MAX_KINISI_COUNT_BASE_ADDR);
    //read uint32_t data
    cum_on_time[AC]         = EEPROM.readLong(AC_CUM_ON_TIME_BAS_ADDR);
    cum_on_time[BULB]       = EEPROM.readLong(LIGHT_CUM_ON_TIME_BAS_ADDR);
    hsr_switching_count     = EEPROM.readLong(HSR_COUNT_BASE_ADDR);
    msr_switching_count     = EEPROM.readLong(MSR_COUNT_BASE_ADDR);
    lsr_switching_count     = EEPROM.readLong(LSR_COUNT_BASE_ADDR);
    set_Relays_OFF();//set all relay OFF
    read_temperature();
    //read over temperature status
    UDP_Rx_Packet[OVER_TEMP] = EEPROM.readUChar(OVER_TEMP_STATUS_BASE_ADDR);
    if( UDP_Rx_Packet[OVER_TEMP] )
    {
      device_powered = false;
      DL2_led_control(LED_R2,LED_DRIVE_LOW);
      DL1_led_control(LED_R1,LED_DRIVE_HIGH); //over temperature error
      init_wakeup_timer();
      if( room_temperature >= OVER_TEMPERATURE )
      {
        Serial.println("Going to sleep now");
        vTaskDelay(100);
        Serial.flush();
        esp_deep_sleep_start();
      }
      else
      {
        Serial.println("Recovered from over tempearture");
        UDP_Rx_Packet[OVER_TEMP] = 0;
        update_byte_to_flash(OVER_TEMP_STATUS_BASE_ADDR,UDP_Rx_Packet[OVER_TEMP]);
      }
    }
    else
    {
      Serial.println("\nOver temperature flag is not set");
    }
    DL1_led_control(LED_R1,LED_DRIVE_LOW);
    Serial.printf("ssid length        = %d\n",strlen((char*)ap_ssid));
    Serial.printf("password length    = %d\n",strlen((char*)ap_passkey));
    Serial.printf("serial num length  = %d\n",strlen((char*)serial_number));
    Serial.printf("mode = %d",operating_mode);
    if(device_register_status)
    {
        Serial.println("Device successfully registered");
        /*
         * print ac_mode selected
         */
        if( tirisi_ac_type == HVAC_TYPE )
        {
            Serial.println("ac type            : HVAC Type ");
            hvac_relay_control(json_extract.ac_temp);
        }
        else if ( tirisi_ac_type == SPLIT_AC_TYPE)
        {
            Serial.println("ac type            : SPLIT AC Type ");
            /* If Split AC is selected send relay switching count to 0 */
            hsr_switching_count     = 0;
            msr_switching_count     = 0;
            lsr_switching_count     = 0;
            send_ir_ac_command(json_extract.ac_temp);
        }
        else
        {
            Serial.println("Error:Check ac mode read location");
        }
        /*
         * print AC type selected
         */
         if ( tirisi_ac_brand == VCON_AC )
         {
            Serial.println("Selected AC type    : VCON_AC ");
         }
         else if ( tirisi_ac_brand == PANA_AC )
         {
            Serial.println("Selected AC type    : PANA_AC ");
         }
         else if ( tirisi_ac_brand == VOLT_AC )
         {
            Serial.println("Selected AC type    : VOLT_AC ");
         }
         else if ( tirisi_ac_brand == HAIR_AC )
         {
            Serial.println("Selected AC type    : HAIR_AC ");
         }
         else if ( tirisi_ac_brand == SAMG_AC )
         {
            Serial.println("Selected AC type    : SAMG_AC ");
         }
         else
         {
            Serial.println("Error:Check tirisi_ac_brand read location");
         }
         /*
          * print light_control status
          */
         if ( light_control == ENABLE )
         {
            Serial.println("Light control       : Enabled");
         }
         else if ( light_control == DISABLE )
         {
            Serial.println("Light control       : Disabled");
         }
         else
         {
            Serial.println("Error:Check light_control read location");
         }
         /*
          * print occupancy_detection status
          */
         if ( occupancy_detection == ENABLE )
         {
            Serial.println("Occupancy Detection : Enabled");
         }
         else if ( occupancy_detection == DISABLE )
         {
            Serial.println("Occupancy Detection : Disabled");
         }
         else
         {
            Serial.println("Error:Check occupancy_detection read location");
         }
         /*
          * print device type
          */
         if ( device_type == DEVICE_TYPE )
         {
            Serial.println("Device Type         : AMENOS");
         }
         else
         {
            Serial.println("Error:Check device_type read location");
         }
         Serial.printf("No. of kinisi need to be registered in room : %d\n",max_kinisi_in_room);
        if ( tirisi_ac_type == HVAC_TYPE )
        {
          device_variant = 0x5000;
          if ( light_control == ENABLE  )
          {
            device_variant = 0; //need to check
          }
          if ( occupancy_detection == ENABLE  )
          {
            device_variant = 0x5010;
          }
          if( (light_control == ENABLE) && (occupancy_detection == ENABLE) )
          {
            device_variant = 0x5011;
          }
        }
        else if ( tirisi_ac_type == SPLIT_AC_TYPE )
        {
          device_variant = 0x5100;
          if ( light_control == ENABLE  )
          {
            device_variant = 0; //need to check
          }
          if ( occupancy_detection == ENABLE  )
          {
            device_variant = 0x5110;
          }
          if( (light_control == ENABLE) && (occupancy_detection == ENABLE) )
          {
            device_variant = 0x5111;
          }
        }
        Serial.printf("device variant   = %x\n",device_variant);

        /*
         * Read kinisi related data from flash
         */
         Serial.printf("\nno of kinisi registered = %d\n",no_of_kinisi_device);

        if( no_of_kinisi_device >=BYTE1)
        {
          for( int i = 0;i<max_kinisi_in_room;i++)
          {
            kinisi_serial_num_ptr[i]  = NULL;
            kinisi_device_name_ptr[i] = NULL;
            kinisi_macid_ptr[i]       = NULL;
          }
          if( read_kinisi_count < no_of_kinisi_device )
          {
            for(int i = KINISI_SERIAL_NUM_BASE_ADDR ;i < (KINISI_SERIAL_NUM_BASE_ADDR + BYTE16);i++)
            {
              kinisi_serial_number[i - KINISI_SERIAL_NUM_BASE_ADDR] = EEPROM.readUChar(i);
            }
            kinisi_serial_number[BYTE16] = '\0';
            kinisi_serial_num_ptr[0] = (uint8_t *)kinisi_serial_number;
            kinisi_lookup_table[0][1] = (char *)kinisi_serial_num_ptr[0];

            for(int i = KINISI_DEVICE_NAME_BASE_ADDR ;i < (KINISI_DEVICE_NAME_BASE_ADDR + BYTE11);i++)
            {
              kinisi_device_name[i - KINISI_DEVICE_NAME_BASE_ADDR] = EEPROM.readUChar(i);
            }
            kinisi_device_name[BYTE11] = '\0';
            kinisi_device_name_ptr[0] = (uint8_t *)kinisi_device_name;
            kinisi_lookup_table[0][2] = (char *)kinisi_device_name_ptr[0];

            for(int i = KINISI_SERIAL_MACID_BASE_ADDR ;i < (KINISI_SERIAL_MACID_BASE_ADDR + BYTE17);i++)
            {
              Kinisi_Mac_Id[i - KINISI_SERIAL_MACID_BASE_ADDR] = EEPROM.readUChar(i);
//              Serial.printf("%d - %c\n",(i - KINISI_SERIAL_MACID_BASE_ADDR),Kinisi_Mac_Id[i - KINISI_SERIAL_MACID_BASE_ADDR]);
            }
            Kinisi_Mac_Id[BYTE17] = '\0';
            kinisi_lookup_table[0][5] = (char *)Kinisi_Mac_Id;
//            Serial.println(kinisi_lookup_table[0][5]);
            memset(Kinisi_Mac_Id,'\0',BYTE17);
            read_kinisi_count++;
          }

          if( read_kinisi_count < no_of_kinisi_device )
          {
            for(int i = KINISI_SERIAL_NUM_BASE_ADDR + BYTE16 ;i < (KINISI_SERIAL_NUM_BASE_ADDR + ( BYTE16 * BYTE2));i++)
            {
              kinisi_serial_number[i - (KINISI_SERIAL_NUM_BASE_ADDR + BYTE16 )] = EEPROM.readUChar(i);
            }
            kinisi_serial_number[BYTE16] = '\0';
            kinisi_serial_num_ptr[1] = (uint8_t *)kinisi_serial_number;
            kinisi_lookup_table[1][1] = (char *)kinisi_serial_num_ptr[1];

            for(int i = KINISI_DEVICE_NAME_BASE_ADDR + BYTE11 ;i < (KINISI_DEVICE_NAME_BASE_ADDR + ( BYTE11 * BYTE2));i++)
            {
              kinisi_device_name[i - (KINISI_DEVICE_NAME_BASE_ADDR + BYTE11 )] = EEPROM.readUChar(i);
            }
            kinisi_device_name[BYTE11] = '\0';
            kinisi_device_name_ptr[1] = (uint8_t *)kinisi_device_name;
            kinisi_lookup_table[1][2] = (char *)kinisi_device_name_ptr[1];

            for(int i = KINISI_SERIAL_MACID_BASE_ADDR + BYTE17 ;i < (KINISI_SERIAL_MACID_BASE_ADDR + ( BYTE17 * BYTE2));i++)
            {
              Kinisi_Mac_Id[i - (KINISI_SERIAL_MACID_BASE_ADDR + BYTE17)] = EEPROM.readUChar(i);
//              Serial.printf("%d - %c\n",(i - (KINISI_DEVICE_NAME_BASE_ADDR + BYTE17 )),Kinisi_Mac_Id[i - (KINISI_DEVICE_NAME_BASE_ADDR + BYTE17 )]);
            }
            Kinisi_Mac_Id[BYTE17] = '\0';
//            Serial.printf(" lenghth = %d\n",strlen((char *)Kinisi_Mac_Id));

            kinisi_lookup_table[1][5] = (char *)Kinisi_Mac_Id;
//            Serial.println(kinisi_lookup_table[1][5]);
            memset(Kinisi_Mac_Id,'\0',BYTE17);
            read_kinisi_count++;
          }

          if( read_kinisi_count < no_of_kinisi_device )
          {
            for(int i = (KINISI_SERIAL_NUM_BASE_ADDR + (BYTE16 * BYTE2));i < (KINISI_SERIAL_NUM_BASE_ADDR + ( BYTE16 * BYTE3));i++)
            {
              kinisi_serial_number[i - (KINISI_SERIAL_NUM_BASE_ADDR + (BYTE16 * BYTE2))] = EEPROM.readUChar(i);
            }
            kinisi_serial_number[BYTE16] = '\0';
            kinisi_serial_num_ptr[2] = (uint8_t *)kinisi_serial_number;
            kinisi_lookup_table[2][1] = (char *)kinisi_serial_num_ptr[2] ;

            for(int i = (KINISI_DEVICE_NAME_BASE_ADDR + (BYTE11 * BYTE2));i < (KINISI_DEVICE_NAME_BASE_ADDR + ( BYTE11 * BYTE3));i++)
            {
              kinisi_device_name[i - (KINISI_DEVICE_NAME_BASE_ADDR + (BYTE11 * BYTE2))] = EEPROM.readUChar(i);
            }
            kinisi_device_name[BYTE11] = '\0';
            kinisi_device_name_ptr[2] = (uint8_t *)kinisi_device_name;
            kinisi_lookup_table[2][2] = (char *)kinisi_device_name_ptr[2];

            for(int i = (KINISI_SERIAL_MACID_BASE_ADDR + (BYTE17 * BYTE2));i < (KINISI_SERIAL_MACID_BASE_ADDR + ( BYTE17 * BYTE3));i++)
            {
              Kinisi_Mac_Id[i - (KINISI_SERIAL_MACID_BASE_ADDR + (BYTE17 * BYTE2))] = EEPROM.readUChar(i);
            }
            Kinisi_Mac_Id[BYTE17] = '\0';
            kinisi_lookup_table[2][5] = (char *)Kinisi_Mac_Id;
            memset(Kinisi_Mac_Id,'\0',BYTE17);

            read_kinisi_count++;
          }

          if( read_kinisi_count < no_of_kinisi_device )
          {
            for(int i = (KINISI_SERIAL_NUM_BASE_ADDR + (BYTE16 * BYTE3)) ;i < (KINISI_SERIAL_NUM_BASE_ADDR + ( BYTE16 * BYTE4));i++)
            {
              kinisi_serial_number[i - (KINISI_SERIAL_NUM_BASE_ADDR + (BYTE16 * BYTE3)) ] = EEPROM.readUChar(i);
            }
            kinisi_serial_number[BYTE16] = '\0';
            kinisi_serial_num_ptr[3] = (uint8_t *)kinisi_serial_number;
            kinisi_lookup_table[3][1] = (char *)kinisi_serial_num_ptr[3] ;

            for(int i = (KINISI_DEVICE_NAME_BASE_ADDR + (BYTE11 * BYTE3)) ;i < (KINISI_DEVICE_NAME_BASE_ADDR + ( BYTE11 * BYTE4));i++)
            {
              kinisi_device_name[i - (KINISI_DEVICE_NAME_BASE_ADDR + (BYTE11 * BYTE3)) ] = EEPROM.readUChar(i);
            }
            kinisi_device_name[BYTE11] = '\0';
            kinisi_device_name_ptr[3] = (uint8_t *)kinisi_device_name;
            kinisi_lookup_table[3][2] = (char *)kinisi_device_name_ptr[3];

            for(int i = (KINISI_SERIAL_MACID_BASE_ADDR + (BYTE17 * BYTE3));i < (KINISI_SERIAL_MACID_BASE_ADDR + ( BYTE17 * BYTE4));i++)
            {
              Kinisi_Mac_Id[i - (KINISI_SERIAL_MACID_BASE_ADDR + (BYTE17 * BYTE3))] = EEPROM.readUChar(i);
            }
            Kinisi_Mac_Id[BYTE17] = '\0';
            kinisi_lookup_table[3][5] = (char *)Kinisi_Mac_Id;
            memset(Kinisi_Mac_Id,'\0',BYTE17);

            read_kinisi_count++;
          }

          if( read_kinisi_count < no_of_kinisi_device )
          {
            for(int i = (KINISI_SERIAL_NUM_BASE_ADDR + (BYTE16 * BYTE4)) ;i < (KINISI_SERIAL_NUM_BASE_ADDR + ( BYTE16 * BYTE5));i++)
            {
              kinisi_serial_number[i - (KINISI_SERIAL_NUM_BASE_ADDR + (BYTE16 * BYTE4))] = EEPROM.readUChar(i);
            }
            kinisi_serial_number[BYTE16] = '\0';
            kinisi_serial_num_ptr[4] = (uint8_t *)kinisi_serial_number;
            kinisi_lookup_table[4][1] = (char *)kinisi_serial_num_ptr[4] ;

            for(int i = (KINISI_DEVICE_NAME_BASE_ADDR + (BYTE11 * BYTE4)) ;i < (KINISI_DEVICE_NAME_BASE_ADDR + ( BYTE11 * BYTE5));i++)
            {
              kinisi_device_name[i - (KINISI_DEVICE_NAME_BASE_ADDR + (BYTE11 * BYTE4))] = EEPROM.readUChar(i);
            }
            kinisi_device_name[BYTE11] = '\0';
            kinisi_device_name_ptr[4] = (uint8_t *)kinisi_device_name;
            kinisi_lookup_table[4][2] = (char *)kinisi_device_name_ptr[4];

            for(int i = (KINISI_SERIAL_MACID_BASE_ADDR + (BYTE17 * BYTE4));i < (KINISI_SERIAL_MACID_BASE_ADDR + ( BYTE17 * BYTE4));i++)
            {
              Kinisi_Mac_Id[i - (KINISI_SERIAL_MACID_BASE_ADDR + (BYTE17 * BYTE4))] = EEPROM.readUChar(i);
            }
            Kinisi_Mac_Id[BYTE17] = '\0';
            kinisi_lookup_table[4][5] = (char *)Kinisi_Mac_Id;
            memset(Kinisi_Mac_Id,'\0',BYTE17);

            read_kinisi_count++;
          }
          read_kinisi_count = 0;

          Serial.println();
          for(int i = 0 ;i < no_of_kinisi_device;i++)
          {
            kinisi_lookup_table[i][0] = itoa(i+1,str_buffer,DECIMAL);
            kinisi_lookup_table[i][4] = itoa(EEPROM.readUChar(KINISI_REG_ADDR + i),str_buffer,DECIMAL);
          }
          print_Kinsi_lookup_table(kinisi_lookup_table);
        }
    }
    else
    {
        Serial.println("Device not registered");
    }
    Serial.printf(" Total AC ON time = %ld\n",cum_on_time[AC]);
    Serial.printf(" Total Light ON time = %ld\n",cum_on_time[BULB]);
}
/**********************************************************************//**
 * @brief  read data from flash and update in respective buffer
 *
 * @param  none
 *
 * @return none
 *************************************************************************/

static void bootup_configuartion()
{
  char ac_mode[4]={0};
  int address = 0;
  if(EEPROM.readUChar(FACTORY_DEVICE_BASE_ADDR) == NOT_FACTORY_DEVICE)
  {
    Serial.println("Not Factory device");
    for(address = SET_AC_MODE_BASE_ADDR;address<(SET_AC_MODE_BASE_ADDR + BYTE3);address++)
    {
      ac_mode[ address - SET_AC_MODE_BASE_ADDR ]= EEPROM.readUChar(address);
    }
    ac_mode[address]                        = '\0';
    json_extract.ac_mode                    = ac_mode;
    json_extract.ac_status                  = EEPROM.readUChar(SET_AC_STATUS_BASE_ADDR);
    json_extract.ac_temp                    = EEPROM.readUChar(SET_AC_TEMP_BASE_ADDR);
    no_of_kinisi_device                     = EEPROM.readUChar(KINISI_DEVICE_COUNT);
    device_count[KINISI_DECOMMISSION_COUNT] = EEPROM.readUChar(KINISI_DECOM_DEVICE_COUNT);
    Serial.println("set ac temperature =" + String(json_extract.ac_temp ));
    Serial.println("set ac Mode        =" + String(json_extract.ac_mode));
    Serial.println("set ac status      =" + String(json_extract.ac_status));
  }
  else
  {
    Serial.println("Factory device");
    //Default AC setting
    json_extract.ac_temp    = DEFAULT_TEMPERATURE;
    json_extract.ac_mode    = AUTO_MODE;
    json_extract.ac_status  = DEVICE_TURN_ON;
    //update default AC setting flash
    update_flash(SET_AC_MODE_BASE_ADDR,(uint8_t *)json_extract.ac_mode,strlen(json_extract.ac_mode));
    update_byte_to_flash(SET_AC_STATUS_BASE_ADDR,json_extract.ac_status);
    update_byte_to_flash(SET_AC_TEMP_BASE_ADDR,json_extract.ac_temp);

    no_of_kinisi_device = 0;
    update_byte_to_flash(KINISI_DEVICE_COUNT,no_of_kinisi_device);
    device_count[KINISI_DECOMMISSION_COUNT] = 0;
    update_byte_to_flash(KINISI_DECOM_DEVICE_COUNT,device_count[KINISI_DECOMMISSION_COUNT]);
    update_byte_to_flash(FACTORY_DEVICE_BASE_ADDR,NOT_FACTORY_DEVICE);
  }
  display_temperature(json_extract.ac_temp);
  if(EEPROM.readUChar(OPERATING_MODE_BASE_ADDR) != CONFIGURATION_MODE)
  {
    Serial.println("\nConfigured device");
    /*allow to store in flash only when all configuration parametrs
    are received from mobile app*/
    write_data_to_flash = false;
    send_device_reg_packet = true;
    read_flash();
    connect_to_ap(ap_ssid,ap_passkey);
    Serial.println("\nInitialize OTAP");
//    Serial.println("BLE Advertisement stopped");
//    ble_stop_advertisement();
    initialize_ble_client();

    /***Turn AC ON******/
//    maintain_ac_setting();
#if HITACHI
    irsend.begin();
    Serial.println("Turn on");
    irsend.sendRaw(turn_on, 595, 38);  // Send a raw data capture at 38kHz.
#endif
    ac.on_minute = get_time();
    Serial.printf("AC turn ON time =  %ld\n",ac.on_minute);
  }
  else
  {
    /*allow to store in flash only when all configuration parametrs
    are received from mobile app*/
    write_data_to_flash = false;
//    for(uint16_t misc_count = 0;misc_count < FLASH_SIZE ; misc_count++)
//    {
//      EEPROM.writeUChar(misc_count,'\0');
//    }
    config_data_rcvd = EEPROM.readUChar(CONFIG_DATA_STATUS_BASE_ADDR);
    Serial.printf("device config data status = %d\n",config_data_rcvd);
    if(config_data_rcvd)
    {
      Serial.println("confiuration parameters received successfully\nReading data from flash");
      read_flash();
      connect_to_ap(ap_ssid,ap_passkey);
      device_register_status = false;
    }
    else
    {
      Serial.println("unconfigured device");
      Serial.println("Initialize BLE");
      setup_ble();
      //For factory device/unconfigured device set opearting mode to CONFIGURATION_MODE
      operating_mode = CONFIGURATION_MODE;
      //store operating mode in flash
      update_byte_to_flash(OPERATING_MODE_BASE_ADDR,operating_mode);
      device_register_status = false;
    }
  }
  print_amenos_status();
}

/**********************************************************************//**
 * @brief  wrap the display data to matrix1_display,matrix2_display buffer
 *
 * @param  temperature[]  - array that holds the value to be displayed
 *
 * @param  digit_count    - No of digits need to display
 *
 * @return none
 *************************************************************************/
static void update_display_content(char temperature[],int digit_count)
{
  unsigned int i, j, digit, count = 0;
  for (digit = 0; digit < 3; digit++)
  {
    for(i = 0; i < 11 ; i++)
    {
      if (temperature[digit] == list_of_characters[i])
      {
        previous_temperature[digit] = temperature[digit];
        //Serial.print("character is found in : ");
        //Serial.println(i);
        count++;
        if ( digit_count == 1 )
        {
          for (j = 1; j < 9; j++)
          {
            //Serial.println(led_hex[i][j-1]);
            matrix2_display[j-1] = led_hex[i][j-1] >> 5; //displaying 1st digit in matrix1
            if (DEGREE_C)
            {
              matrix1_display[j-1] = led_hex[11][j-1] << 2; //2//displaying 2nd digit and degree C in matrix2
            }
            else
            {
              matrix1_display[j-1] = led_hex[12][j-1] << 2; //displaying 2nd digit and degree F in matrix2
            }
            vTaskDelay(2);
          }
        }
        else if ( digit_count == 2 )
        {
          for (j = 1; j < 9; j++)
          {
            //Serial.println(led_hex[i][j-1]);
            if (count == 1)
            {
              word1[j-1] = led_hex[i][j-1];
            }
            if (count == 2)
            {
              matrix2_display[j-1] = ((word1[j-1] >> 1 ) + (led_hex[i][j-1] >> 5)); //displaying 1st digit in matrix1
              if (DEGREE_C)
              {
                matrix1_display[j-1] = led_hex[11][j-1] << 2; //displaying 2nd digit and degree C in matrix2
              }
              else
              {
                matrix1_display[j-1] = led_hex[12][j-1] << 2; //displaying 2nd digit and degree F in matrix2
              }
            }
            vTaskDelay(2);
          }
        }
        else if ( digit_count == 3 )
        {
          for (j = 1; j < 9; j++)
          {
            //Serial.println(led_hex[i][j-1]);
            if (count == 1)
            {
              word1[j-1] = led_hex[i][j-1];
            }
            if (count == 2)
            {
              matrix2_display[j-1] = word1[j-1] + (led_hex[i][j-1] >> 4); //displaying 1st and 2nd digit in matrix1
            }
            if (count == 3)
            {
              if (DEGREE_C)
              {
                matrix1_display[j-1] = led_hex[i][j-1] + led_hex[11][j-1];  //displaying 3rd digit and degree C in matrix2
              } else
              {
                matrix1_display[j-1] = led_hex[i][j-1] + led_hex[12][j-1];  //displaying 3rd digit and degree F in matrix2
              }
            }
            vTaskDelay(2);
          }
        }
      }
      vTaskDelay(1);
    }
    vTaskDelay(2);
  }
}
/**********************************************************************//**
 * @brief  Display byte in Dot matrix-1
 *
 * @param  temperature[] - array that holds the value to be displayed
 *
 * @return none
 *************************************************************************/
static void displayByte_matrix1(byte character[])
{
  for(int row=0;row<ROW_WIDTH;row++)
  {
    lc.setRow(0,row,character[row]);
  }
}

/**********************************************************************//**
 * @brief  Display two digits in Dot matrix-2
 *
 * @param  temperature[] - array that holds the value to be displayed
 *
 * @return none
 *************************************************************************/
static void displayByte_matrix2(byte character[])
{
  for(int row=0;row<ROW_WIDTH;row++)
  {
    lc.setRow(1,row,character[row]);
  }
}

/**********************************************************************//**
 * @brief  display temperature
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void display_temperature(int temp)
{
    itoa(temp,input_temperature, 10);  //convert interger to string
    //Serial.printf("Hexadecimal value = %s\n", input_temperature);
    number_of_digits = strlen(input_temperature);  //finding length of the string
    for (int i = 0; i < 3; i++)
    {
      if (input_temperature[i] != previous_temperature[i])
      {
        update_display_content(input_temperature,number_of_digits);
        lc.clearDisplay(0);
        lc.clearDisplay(1);
        displayByte_matrix2(matrix1_display);
        displayByte_matrix1(matrix2_display);
      }
    }
}
/**********************************************************************//**
 * @brief  Initialize MAX7219
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void init_MAX7219()
{
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
   for (int DM_device_count = 0 ; DM_device_count < NUMBER_OF_DEVICES ; DM_device_count++)
   {
      lc.shutdown(DM_device_count,false);
      /* Set the brightness to a medium values */
      lc.setIntensity(DM_device_count,MIN_INTENSITY);
      /* and clear the display */
      lc.clearDisplay(DM_device_count);
  }

}

/**********************************************************************//**
 * @brief  Get time from server
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static uint32_t get_time()
{
  //get time from server
  while(!timeClient.update())
  {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formatted_date = timeClient.getFormattedDate();
//  Serial.println(formatted_date);

  int splitT = formatted_date.indexOf("T");
  // Extract time
  timeStamp = formatted_date.substring(splitT+1, formatted_date.length()-1);
//  Serial.print("HOUR: ");
//  Serial.println(timeStamp);
//
  current.hour    = (((timeStamp[BYTE0] - CHARACTER_OFFSET) * 10) + (timeStamp[BYTE1] - CHARACTER_OFFSET));
  current.minute  = (((timeStamp[BYTE3] - CHARACTER_OFFSET) * 10) + (timeStamp[BYTE4] - CHARACTER_OFFSET));
  current.on_minute = ( ( current.hour * ONE_MINUTE_IN_SEC ) + current.minute );
  Serial.printf(" current on time = %d\n",current.on_minute);
  return current.on_minute;
}
/**********************************************************************//**
 * @brief  calculate cummulative on time of variant
 *
 * @param  variant - refers for which we need to calculate cummulative time
 *
 * @return cum_on_time - total on time of variant
 *************************************************************************/
static void cummulative_on_minute(uint8_t variant , uint32_t previous_on_time)
{
  if( get_time() > previous_on_time )
  {
    total_on_minute = get_time() - previous_on_time;
  }
  else
  {
    /*** At end of the day clear the ac & light start time of previous day***/
    total_on_minute         = get_time();
    ac.on_minute            = 0;
    ac.previous_on_time     = 0;
    light.on_minute         = 0;
    light.previous_on_time  = 0;
    Serial.println("\nClear time at End of day");
  }

//  Serial.printf("total on time = %d\n",total_on_minute);
  if(variant == AIR_CONDITIONER)
  {
    if( total_on_minute > 0 )
    {
      ac.current_on_time = total_on_minute;
      if( ac.current_on_time > ac.previous_on_time)
      {
        cum_on_time[AC] += (ac.current_on_time - ac.previous_on_time);
        ac.previous_on_time = ac.current_on_time;
      }
      Serial.printf("AC cum on time = %ld\n",cum_on_time[AC]);
    }
  }
  else if(variant == SMART_BULB)
  {
    if( total_on_minute > 0 )
    {
      light.current_on_time = total_on_minute;
      if( light.current_on_time > light.previous_on_time)
      {
        cum_on_time[BULB] += (light.current_on_time - light.previous_on_time);
        light.previous_on_time = light.current_on_time;
      }
      Serial.printf("Light cum on time = %ld\n",cum_on_time[BULB]);
    }
  }
}
/**********************************************************************//**
 * @brief  Initialize BLE-Beacon scanner
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void initialize_ble_client()
{
  Serial.println("Starting ESP BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  //pBLEScan->start(SCAN_TIME, false);
}
/**********************************************************************//**
 * @brief  Start the BLE-Scan service and list out the devices found,
 *         then clear the scan buffer
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void start_scan()
{
  BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME, false);
//  Serial.print("Devices found: ");
//  Serial.println(foundDevices.getCount());
//  Serial.println("Scan done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
}
/**********************************************************************//**
 * @brief  send data to destination over TCP protocol
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void transmit_over_tcp()
{
  Serial.print("connecting to ");
  Serial.println(tcp_host);
  if (!TCP_Client.connect(tcp_host, tcp_port)) //open the socket
  {
    Serial.println("\nError:TCP connection failed");
  }
  else
  {
    Serial.println("Success:Connection ok");
    /* This will send the data to the server */
    Serial.println("Sending packets");
    StaticJsonDocument<1024> json_buffer;//allocate 1kb memory in stack
    JsonObject key = json_buffer.to<JsonObject>();//create json object
    json_buffer["hmac"] = "";
    JsonObject payload = json_buffer.createNestedObject("payload");//add for nested json document
    payload["device_uid"] = serial_number;//add serial number
    payload["data_type"] = DEVICE_SENSOR_DATA;
    JsonArray data = payload.createNestedArray("device_data");
    JsonObject device_data = data.createNestedObject();//add for nested json document
    device_data["sender_uid"] = PRODUCT_MODEL_KINISI;
    device_data["packet_type"] = DEVICE_SENSOR_DATA;
    device_data["battery_percentage"] = kinisi_battery_percentage;
    device_data["Occupancy_detection"] = 1;

    serializeJsonPretty(json_buffer, Serial);//print in serial terminal

    serializeJsonPretty(json_buffer,TCP_Client);//send data to idp server


    //TCP_Client.print("hello world\n\n");
    #if 0
    int maxloops = 0;

    //wait for the server's reply to become available
    while (!TCP_Client.available() && maxloops < 1000)
    {
      maxloops++;
      vTaskDelay(1); //delay 1 msec
    }
    if (TCP_Client.available() > 0)
    {
      //read back one line from the server
      String line = TCP_Client.readStringUntil('\r');
      Serial.println("Data rx: ");
      Serial.println(line);
    }
    else
    {
      Serial.println("client.available() timed out ");
    }

    Serial.println("Closing connection.");
    TCP_Client.stop();

    Serial.println("Waiting 5 seconds before restarting...");
    vTaskDelay(5000);
    #endif
    size_t len = 0;
  StaticJsonDocument<1024> parsing_buffer;
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(parsing_buffer, TCP_Client);

  // Test if parsing succeeds.
  if( error )
  {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
  }
  else
  {
    Serial.println("parsing success");
    len = measureJsonPretty(parsing_buffer);
    if ( len > 0)
    {
      Serial.printf("\nparsed buffer length =%d\n",len);
      /*
       * print received data in serial terminal
       */
      Serial.println("Received data");
      serializeJsonPretty(parsing_buffer, Serial);//print in serial terminal
      json_extract.hmac = parsing_buffer["hmac"];
      json_extract.device_id = parsing_buffer["payload"]["device_uid"];
      json_extract.action = parsing_buffer["payload"]["action"];
   }
  }
    TCP_Client.stop(); //close the socket
    Serial.print("TCP Socket closed");
  }
}
/**********************************************************************//**
 * @brief  Receive data transmitted over TCP protocol
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
 #if 0
static void Receive_tcp_data()
{

  TCP_Client = TCP_server.available();
  uint8_t data[30];
  if (TCP_Client)
  {
    Serial.println("new client");
    /* check client is connected */
    while (TCP_Client.connected())
    {
      if (TCP_Client.available())
      {
        int len = TCP_Client.read(data, 30);
        if(len < 30)
        {
          data[len] = '\0';
        }
        else
        {
          data[30] = '\0';
        }
        Serial.print("client sent: ");
        Serial.println((char *)data);
      }
    }
  }
}
#endif
/**********************************************************************//**
 * @brief  Extract required data from scanned beacon data
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void extract_beacon_data()
{

  /*
   *  kinisi Beacon advertising packet format
   *  |--------------------|------------------|---------------------|-----------------|---------------|--------------------|
   *  | kinisi device type | firmware version | PIR sensor status   | Battery voltage |  Room number  | Advertisement name |
      |--------------------|------------------|---------------------|-----------------|---------------|--------------------| */

  if( scan_data[BYTE0] == KINISI )
  {
    sprintf(kinsi_fm_version,"%d.%d\0",scan_data[BYTE1],scan_data[BYTE2]);
//    Serial.print("FW version :");
//    Serial.println(kinsi_fm_version);

    if( strcmp(adv_room_no,(const char*)room_no) == 0 )
    {
      if( scan_data[BYTE3] ==  OCCUPANCY_SENSED )
      {
        occupancy_detected = true;
      }
      else
      {
        occupancy_detected = false;
//        Serial.println("occupancy not detecetd");
      }
      kinisi_battery_percentage = scan_data[BYTE4];
//      Serial.println("battery percentage = " + String(kinisi_battery_percentage) + "%");
    }
    else
    {
      Serial.println("Packet discarded reason : Room number not matched");
    }
  }
  else
  {
    Serial.println("Packet discarded reason : Kinisi Device Type not matched");
  }
}
/**********************************************************************//**
 * @brief  Extract required data from scanned beacon data
 *
 * @param  data         - pointer that holds the data need to check whether it is present in
 *                        rxbuf
*          timeout      - duration to wait in this function in milli seconds
*          timout_alone - Enable/Disable that just to wait or wait and check response
 *
 * @return none
 *************************************************************************/
int16_t wait_for(const char* data, uint16_t timeout ,bool timout_alone)
{
  uint64_t timer = millis();
  char c;

  while (millis() - timer < timeout)
  {
    if ( timout_alone )
    {
      if( uart2_rx_flag )
      {
        Zigbee_msg_length =  strlen((const char*)rxbuf) ;
        uart_write_bytes(ZIGBEE_UART, (const char*)rxbuf,Zigbee_msg_length);
        Serial.printf("zigbee data received len = %d\n",Zigbee_msg_length);
        uart2_rx_flag = 0 ;
        Zigbee_msg_length = 0;
        return EXIT_SUCCESS;
      }

    }
    else
    {
      if( uart2_rx_flag )
      {
        Zigbee_msg_length =  strlen((const char*)rxbuf) ;
        uart_write_bytes(ZIGBEE_UART, (const char*)rxbuf,Zigbee_msg_length);
        Serial.printf("zigbee data received len = %d\n",Zigbee_msg_length);
        if ( rxbuf[Zigbee_msg_length-2] == 0x0D && rxbuf[Zigbee_msg_length-1] == 0x0A )
        {
          if ( strstr((const char*)rxbuf,data) )
          {
            Serial.printf("%x\n",rxbuf[Zigbee_msg_length]);
            Serial.printf("%x\n",rxbuf[Zigbee_msg_length-1]);
            Serial.println("success");
          }
          if ( strstr((const char*)rxbuf,ERROR_CODE) )
          {
            Serial.print("Error: ");
            Serial.println((char*)rxbuf);
          }

          //Serial.println("super");
        }
        for ( int misc_count = 0; misc_count < Zigbee_msg_length ; misc_count++ )
        {
          Serial.printf("%x ",rxbuf[misc_count]);
          rxbuf[misc_count] = '\0';//clear buffer
        }
        uart_flush_input(ZIGBEE_UART);
        uart2_rx_flag = 0;
        Zigbee_msg_length = 0;
        return EXIT_SUCCESS;
      }
    }
    vTaskDelay(1);
  }
  return EXIT_FAILURE;
}
/**********************************************************************//**
 * @brief  print uint8_t data in hex
 *
 * @param  print_data    - array holds the data to be print
*          data_len      - length of data to be print

 *
 * @return none
 *************************************************************************/
static void print_uint_data(uint8_t *print_data,uint16_t data_len)
{
  //Serial.println();
  for(int misc_count = 0;misc_count < data_len ;misc_count++)
  {
    Serial.printf("%c",print_data[misc_count]);
  }
}

/**********************************************************************
 * @brief  Process received ble data from Kinisi
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void process_Kinisi_ble_data()
{
  uint16_t update_count=0;
  if ( !allow_process_Kinisi_ble_data )
  {
    if( (ble_rx_buffer[FUNCTION_CODE] == BLE_PASSKEY_FUNC_CODE ) && (ble_rx_buffer[COMMAND] == KINISI_BLE_COMMAND) )
    {
      if ( ble_rx_buffer[INSTANT_ID] == AUTHENTICATION )
      {
        if ( ble_rx_length == ble_rx_buffer[RXD_PACKET_SIZE] )
        {
          Serial.printf("Kinisi Password ACK status received : %x\n",ble_rx_buffer[INSTANT_ID]);
          Kinisi_ack_status = ble_rx_buffer[INSTANT_ID + 1];
          Serial.printf("kinisi password ack status = %x\n",Kinisi_ack_status);
          if( Kinisi_ack_status == AUTHENTICATION_SUCCESS )
          {
            Serial.println("Amenos-Kinisi password matched");
            allow_process_Kinisi_ble_data = 1;
          }
          else
          {
            Serial.println("Amenos-Kinisi password mis-matched");
            pClient->disconnect();
            Serial.println("Disconnected from Kinisi");
            vTaskDelay(50);
            allow_process_Kinisi_ble_data = 0;
          }
        }
        else
        {
          Serial.println("\nError: BLE packet size mismatch\nSend 5 bytes");
          send_ble_packets_to_kinisi(DEVICE_REG_ACK,EXIT_FAILURE);//send NACK to Kinisi
          pClient->disconnect();
          Serial.println("Disconnected from Kinisi");
          vTaskDelay(50);
        }
        memset(ble_rx_buffer,NULL,sizeof(ble_rx_buffer));//clear array
      }
      else
      {
        Serial.println("\nError: Authentication field mis-match\n");
        pClient->disconnect();
        Serial.println("Disconnected from Kinisi");
        vTaskDelay(50);
        memset(ble_rx_buffer,NULL,sizeof(ble_rx_buffer));//clear array
      }
    }
    else
    {
      pClient->disconnect();
      Serial.println("Error:Fuction code & command - Mismatch\nDisconnected from BLE Kinisi");
      vTaskDelay(50);
    }
  }
  if ( ble_rx_buffer[FUNCTION_CODE] == KINISI_FUNCTIONAL_CODE )
  {
    if( (ble_rx_buffer[FUNCTION_CODE] == KINISI_FUNCTIONAL_CODE ) && (ble_rx_buffer[COMMAND] == KINISI_BLE_COMMAND) )
    {
      if ( ble_rx_buffer[INSTANT_ID] == ROOM_DETAILS )
      {
        if ( ble_rx_length == ble_rx_buffer[RXD_PACKET_SIZE] )
        {
          Serial.printf("Room Details Received : %x\n",ble_rx_buffer[INSTANT_ID]);
          for ( update_count = INSTANT_ID +1 ;update_count < MAX_BLE_PACKET_SIZE ; update_count++)
          {
            kinisi_room_details[update_count - (INSTANT_ID +1)] = ble_rx_buffer[update_count];
            Serial.printf("%c",kinisi_room_details[update_count - (INSTANT_ID +1)]);
          }
          kinisi_room_details[update_count - (INSTANT_ID +1)] = '\0';
        }
        else
        {
          Serial.println("\nError: BLE packet size mismatch\nSend 20 bytes");
          send_ble_packets_to_kinisi(DEVICE_REG_ACK,EXIT_FAILURE);//send NACK to Kinisi
        }
      }
      else if ( ble_rx_buffer[INSTANT_ID] == SERIAL_NUMBER )
      {
        if ( ble_rx_length == ble_rx_buffer[RXD_PACKET_SIZE] )
        {
          Serial.printf("Serial Number Received : %x\n",ble_rx_buffer[INSTANT_ID]);
          for ( update_count = INSTANT_ID +1 ;update_count < MAX_BLE_PACKET_SIZE ; update_count++)
          {
            kinisi_serial_number[update_count - (INSTANT_ID +1)] = ble_rx_buffer[update_count];
            Serial.printf("%c",kinisi_serial_number[update_count - (INSTANT_ID +1)]);
          }
          kinisi_serial_number[update_count - (INSTANT_ID +1)] = '\0';//add delimiter
          Length[SERIAL_NUM] = strlen((char*)kinisi_serial_number);//store array length of serial number
       }
        else
        {
          Serial.println("\nError: BLE packet size mismatch\nSend 20 bytes");
          send_ble_packets_to_kinisi(DEVICE_REG_ACK,EXIT_FAILURE);//send NACK to Kinisi
        }
      }
      else if ( ble_rx_buffer[INSTANT_ID] == DEVICE_DETAILS )
      {
        if ( ble_rx_length == ble_rx_buffer[RXD_PACKET_SIZE] )
        {
          Serial.printf("Device details Received : %x\n",ble_rx_buffer[INSTANT_ID]);
          for ( update_count = INSTANT_ID +1 ;update_count < MAX_BLE_PACKET_SIZE ; update_count++)
          {
            if( ble_rx_buffer[update_count] != '#' )
            {
              kinisi_device_name[update_count - (INSTANT_ID +1)] = ble_rx_buffer[update_count];
              Serial.printf("%c",kinisi_device_name[update_count - (INSTANT_ID +1)]);
            }
            else
            {
              kinisi_device_name[update_count - (INSTANT_ID +1)] = '\0';
            }
          }
          kinisi_device_name[update_count - (INSTANT_ID +1)] = '\0';
        }
        else
        {
          Serial.println("\nError: BLE packet size mismatch\nSend 20 bytes");
          send_ble_packets_to_kinisi(DEVICE_REG_ACK,EXIT_FAILURE);//send NACK to Kinisi
        }
      }
      else if ( ble_rx_buffer[INSTANT_ID] == AUTH_CODE )
      {
        if ( ble_rx_length == ble_rx_buffer[RXD_PACKET_SIZE] )
        {
          Serial.printf("Auth code details Received : %x\n",ble_rx_buffer[INSTANT_ID]);
          for ( update_count = INSTANT_ID +1 ;update_count < MAX_BLE_PACKET_SIZE ; update_count++)
          {
            if( ble_rx_buffer[update_count] != '#' )
            {
              kinisi_auth_code[update_count - (INSTANT_ID +1)] = ble_rx_buffer[update_count];
              Serial.printf("%c",kinisi_auth_code[update_count - (INSTANT_ID +1)]);
            }
            else
            {
              kinisi_auth_code[update_count - (INSTANT_ID +1)] = '\0';
            }
          }
          kinisi_auth_code[update_count - (INSTANT_ID +1)] = '\0';
          write_data_to_flash = true;
          allow_process_Kinisi_ble_data = 0;
        }
        else
        {
          Serial.println("\nError: BLE packet size mismatch\nSend 20 bytes");
          //update configuration status
          configuration_status = CONFIG_FAIL;
          send_ble_ack_packets(configuration_status);//send status to ble
          pClient->disconnect();
          Serial.println("Disconnected from Kinisi");
          vTaskDelay(50);
        }
      }
      else
      {
        Serial.print("Error: Data lost send again");
        send_ble_packets_to_kinisi(DEVICE_REG_ACK,EXIT_FAILURE);//send NACK to Kinisi
        pClient->disconnect();
        Serial.println("Disconnected from Kinisi");
        vTaskDelay(50);
      }
      memset(ble_rx_buffer,NULL,sizeof(ble_rx_buffer));//clear array
    }
    else
    {
      Serial.println("Error:Fuction code & command - Mismatch");
      pClient->disconnect();
      Serial.println("Disconnected from Kinisi");
      vTaskDelay(50);
    }
  }
}

/**********************************************************************//**
 * @brief  Transmit data to connected BLE device
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void send_ble_packets_to_kinisi( uint16_t data_type ,bool status )
{
  if( connected )
  {
    if( data_type == PASSWORD )
    {
      //Header
      tx_data_to_kinisi[PACKET_SIZE] = MAX_BLE_PACKET_SIZE;
      tx_data_to_kinisi[FUNCTION_CODE_BYTE] = BLE_PASSKEY_FUNC_CODE;
      tx_data_to_kinisi[COMMAND_BYTE] = KINISI_BLE_COMMAND;
      tx_data_to_kinisi[INSTANT_ID_BYTE] = AUTHENTICATION;
      //data-password
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE1]  = KINISI_AMENOS_PASSWORD[BYTE0];
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE2]  = KINISI_AMENOS_PASSWORD[BYTE1];
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE3]  = KINISI_AMENOS_PASSWORD[BYTE2];
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE4]  = KINISI_AMENOS_PASSWORD[BYTE3];
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE5]  = KINISI_AMENOS_PASSWORD[BYTE4];
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE6]  = KINISI_AMENOS_PASSWORD[BYTE5];
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE7]  = KINISI_AMENOS_PASSWORD[BYTE6];
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE8]  = KINISI_AMENOS_PASSWORD[BYTE7];
      //reserved bytes
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE9]  = RESERVED;
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE10] = RESERVED;
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE11] = RESERVED;
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE12] = RESERVED;
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE13] = RESERVED;
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE14] = RESERVED;
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE15] = RESERVED;
      tx_data_to_kinisi[INSTANT_ID_BYTE+BYTE16] = RESERVED;
      tx_data_to_kinisi[MAX_BLE_PACKET_SIZE]    = '\0';
      //set the flag
      send_password_to_kinisi = 0;
      pRemoteRxCharacteristic->writeValue(tx_data_to_kinisi,sizeof(tx_data_to_kinisi));//set value to characteristic
    }
    else if( data_type == DEVICE_REG_ACK )
    {
      tx_data_to_kinisi[PACKET_SIZE]        = BLE_ACK_BYTE_SIZE;
      tx_data_to_kinisi[FUNCTION_CODE_BYTE] = KINISI_FUNCTIONAL_CODE;
      tx_data_to_kinisi[COMMAND_BYTE]       = KINISI_BLE_COMMAND;
      tx_data_to_kinisi[INSTANT_ID_BYTE]    = BLE_ACK_INSTANT_ID;
      if( status )
      {
        tx_data_to_kinisi[RESULT_BYTE] = AUTHENTICATION_FAIL;
        Serial.println("Error:Fail");
      }
      else
      {
        tx_data_to_kinisi[RESULT_BYTE] = AUTHENTICATION_SUCCESS;
        Serial.println("registered to Hub successfully");
      }
      for(int count = RESULT_BYTE+1 ; count < MAX_BLE_PACKET_SIZE;count++)
      {
        tx_data_to_kinisi[count] = '\0';
      }
      pRemoteRxCharacteristic->writeValue(tx_data_to_kinisi, strlen((char*)tx_data_to_kinisi));
    }
    Serial.print("sent value: ");
    Serial.println((char*)tx_data_to_kinisi);
  }
}
/**********************************************************************//**
 * @brief  scan the available i2c devices in the default i2c bus
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void scan_i2c_device_and_initialize()
{
  Wire.begin();
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
      {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      found_slave_address[nDevices] = address;
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknow error at address 0x");
      if (address<16)
      {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
  {
    Serial.println("No I2C devices found\n");
  }
  else
  {
    Serial.println("done\n");
    if( found_slave_address[TEMPERATURE_SENSOR] == TEMP_SENSOR_SLAVE_ADDRESS )
    {
      Serial.println("Initialize Temperature sensor");
      i2c_write(TEMP_SENSOR_SLAVE_ADDRESS,SET_12BIT_RESOLUTION,TEMP_SENSOR_CONFIGURATION_REG);
      vTaskDelay(200);
    }
    if( found_slave_address[IO_EXPANDER] == EXPANDER_SLAVE_ADDRESS )
    {
      Serial.println("IO-Expander found");
      set_up_expander();   //configure port pins as input/output
      set_Capacitive_Up_Led_OFF();    //set capacitive led off by default, led should be glown only when button pressed
      set_Capacitive_Down_Led_OFF();  //set capacitive led off by default, led should be glown only when button pressed
      vTaskDelay(200);
    }
    if( found_slave_address[LIGHT_SENSOR] == LIGHT_INTENSITY_SLAVE_ADDRESS )
    {
      Serial.println("Light sensor found");
      setup_light_sensor();
    }
  }
}
/**********************************************************************//**
 * @brief  write data to i2c devices
 *
 * @param  address  - i2c slave address
 * @param  data     - data to be write in i2c devices
 * @param  reg      - i2c register address
 *
 * @return none
 *************************************************************************/
static void i2c_write(byte address,uint16_t data,byte reg)
{
  Wire.beginTransmission(address); // Start I2C Transmission
  Wire.write(reg);  // Select configuration register
  // Set resolution = 12-bits, Normal operations, Comparator mode
  if( data > 0 )
  {
    Serial.printf("msb = %x\t lsb = %x\n",data >> 8,data & 0x00FF);
    Wire.write(data >> 8);//first write msb
    Wire.write(data & 0x00FF);//then write lsb
  }
  Wire.endTransmission(); // Stop I2C Transmission
}

/**********************************************************************//**
 * @brief  read data from i2c devices
 *
 * @param  address          - i2c slave address
 * @param  read_byte_count  - no of bytes to be read
 * @param  reg              - i2c register address
 *
 * @return none
 *************************************************************************/
int i2c_read(byte address,byte reg,byte read_byte_count)
{
  unsigned char read_data[2];
  int Data;
  Wire.beginTransmission(address); // Start I2C Transmission
  Wire.write(reg);  // send Temperature register address
  Wire.endTransmission(); // Stop I2C Transmission
  vTaskDelay(300);//convertion time

  Wire.requestFrom(address, read_byte_count); // Request 2 bytes of data
  // Read  bytes of data
  if (Wire.available() == read_byte_count)
  {
    read_data[0] = Wire.read(); // temp msb
    read_data[1] = Wire.read(); //  temp lsb
    Data = (((unsigned int) read_data[0] << 8) | read_data[1] );  //converting 8bit to 16bit
  }
  return Data;
}
/**********************************************************************//**
 * @brief  read temperature from sensor and convert into celsiuis or fareheat
 *
 * @param   none
 *
 * @return  none
 *************************************************************************/
static void read_temperature()
{
  read_temp = i2c_read(TEMP_SENSOR_SLAVE_ADDRESS,TEMP_SENSOR_DATA_REG,2);
  if(DEGREE_C)
  {
    room_temperature = (read_temp >> 4) * 0.0625;  //temperature in celsius .Taken from data sheet
  }
  else
  {
    /*
     * Formula : (32°C × 9/5) + 32 = 89.6°F

     */
    room_temperature = ((read_temp >> 4)  * 0.1125) + 32; //temperature in fahrenheit. 0.0625*1.8 = 0.1125
  }
//  Serial.printf("16 bit value = %d room_temperature = %0.2f\n",read_temp,room_temperature);
}

/**********************************************************************//**
 * @brief  write data to IO EXPANDER
 *
 * @param  address  - IO EXPANDER  slave address
 * @param  data     - data to be write in IO EXPANDER
 * @param  reg      - IO EXPANDER register address
 *
 * @return none
 *************************************************************************/
static void io_expander_write_one_byte(int address,uint8_t data,byte reg)
{
  Wire.beginTransmission(address); // Start I2C Transmission
  Wire.write(reg);  // Select configuration register
  Wire.write(data);//first write msb
  Wire.endTransmission(); // Stop I2C Transmission
}

/**********************************************************************//**
 * @brief  read data from IO EXPANDER
 *
 * @param  address          - IO EXPANDER slave address
 * @param  read_byte_count  - no of bytes to be read
 * @param  reg              - IO EXPANDER register address
 *
 * @return none
 *************************************************************************/
static uint8_t io_expander_read_one_byte(byte address,byte reg)
{
  unsigned char read_data;
  Wire.beginTransmission(address); // Start I2C Transmission
  Wire.write(reg);  // send Temperature register address
  Wire.endTransmission(); // Stop I2C Transmission
  Wire.requestFrom(address, (byte)1); // Request 1 bytes of data
  // Read  bytes of data
  if (Wire.available() == 1)
  {
    read_data = Wire.read();
  }
  return read_data;
}
/**********************************************************************//**
 * @brief  Turn LED B2 ON
 *
 * @param   none
 *
 * @return  none
 *************************************************************************/
static void set_Led_B2_ON()
{
  Exp_Port1 &= (~LED_B2_BIT1);
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port1,TCA9539_OUTPUT_PORT1);
}
/**********************************************************************//**
 * @brief  Turn LED B2 OFF
 *
 * @param   none
 *
 * @return  none
 *************************************************************************/
static void set_Led_B2_OFF()
{
  Exp_Port1 |= LED_B2_BIT1;
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port1,TCA9539_OUTPUT_PORT1);
}
/**********************************************************************//**
 * @brief  Turn ON up capacitive Touch Button Led
 *
 * @param   none
 *
 * @return  none
 *************************************************************************/
static void set_Capacitive_Up_Led_ON()
{
  Exp_Port0 &= (~TOUCH_UP_LED_BIT3);
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port0,TCA9539_OUTPUT_PORT0);
}
/**********************************************************************//**
 * @brief  Turn OFF up capacitive Touch Button Led
 *
 * @param   none
 *
 * @return  none
 *************************************************************************/
static void set_Capacitive_Up_Led_OFF()
{
  Exp_Port0 |= TOUCH_UP_LED_BIT3;
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port0,TCA9539_OUTPUT_PORT0);
}
/**********************************************************************//**
 * @brief  Turn ON down capacitive Touch Button Led
 *
 * @param   none
 *
 * @return  none
 *************************************************************************/
static void set_Capacitive_Down_Led_ON()
{
  Exp_Port0 &= (~TOUCH_DOWN_LED_BIT4);
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port0,TCA9539_OUTPUT_PORT0);
}
/**********************************************************************//**
 * @brief  Turn OFF down capacitive Touch Button Led
 *
 * @param   none
 *
 * @return  none
 *************************************************************************/
static void set_Capacitive_Down_Led_OFF()
{
  Exp_Port0 |= TOUCH_DOWN_LED_BIT4;
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port0,TCA9539_OUTPUT_PORT0);
}
/**********************************************************************//**
 * @brief  Turn OFF Buzzer
 *
 * @param   none
 *
 * @return  none
 *************************************************************************/
static void set_Buzzer_OFF()
{
  Exp_Port0 &= (~BUZZER_DRIVE_BIT5);
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port0,TCA9539_OUTPUT_PORT0);
}
/**********************************************************************//**
 * @brief  Turn ON Buzzer
 *
 * @param   none
 *
 * @return  none
 *************************************************************************/
static void set_Buzzer_ON()
{
  Exp_Port0 |= BUZZER_DRIVE_BIT5;
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port0,TCA9539_OUTPUT_PORT0);
}
/**********************************************************************//**
 * @brief  Turn Relay ON
 *
 * @param   pin IO EXPANDER pin number to be set High
 *
 * @return  none
 *************************************************************************/
static void Relay_ON(byte pin)
{
  /***change pin status***/
  if( pin == LOW_SPEED_RELAY_DRIVE_BIT6 )
  {
    Exp_Port0 |=  LOW_SPEED_RELAY_DRIVE_BIT6;
    Exp_Port0 &= ~MEDIUM_SPEED_RELAY_DRIVE_BIT7;
    Exp_Port1 &= ~HIGH_SPEED_RELAY_DRIVE_BIT0;
  }
  else if( pin == MEDIUM_SPEED_RELAY_DRIVE_BIT7 )
  {
    Exp_Port0 |=  MEDIUM_SPEED_RELAY_DRIVE_BIT7;
    Exp_Port0 &= ~LOW_SPEED_RELAY_DRIVE_BIT6;
    Exp_Port1 &= ~HIGH_SPEED_RELAY_DRIVE_BIT0;
  }
  else if( pin == HIGH_SPEED_RELAY_DRIVE_BIT0 )
  {
    Exp_Port0 &= ~MEDIUM_SPEED_RELAY_DRIVE_BIT7;
    Exp_Port0 &= ~LOW_SPEED_RELAY_DRIVE_BIT6;
    Exp_Port1 |=  HIGH_SPEED_RELAY_DRIVE_BIT0;
  }
  /***increment relay switching count***/
  hsr_switching_count++;
  msr_switching_count++;
  lsr_switching_count++;
  /***set value of pin in IO EXPANDER***/
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port0,TCA9539_OUTPUT_PORT0);
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port1,TCA9539_OUTPUT_PORT1);
  /***store value in Flash***/
  update_uint32_t_to_flash(HSR_COUNT_BASE_ADDR,hsr_switching_count);
  update_uint32_t_to_flash(MSR_COUNT_BASE_ADDR,msr_switching_count);
  update_uint32_t_to_flash(LSR_COUNT_BASE_ADDR,lsr_switching_count);
  /***Print relay switching count***/
  Serial.println("hsr_switching_count" + String(hsr_switching_count));
  Serial.println("msr_switching_count" + String(msr_switching_count));
  Serial.println("lsr_switching_count" + String(lsr_switching_count));
}
/**********************************************************************//**
 * @brief  Turn OFF all relay
 *
 * @param   none
 *
 * @return  none
 *************************************************************************/
static void set_Relays_OFF()
{
  /***change pin status***/
  Exp_Port0 &= (~(LOW_SPEED_RELAY_DRIVE_BIT6 | MEDIUM_SPEED_RELAY_DRIVE_BIT7));
  Exp_Port1 &= (~HIGH_SPEED_RELAY_DRIVE_BIT0);
  /***set value of pin in IO EXPANDER***/
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port0,TCA9539_OUTPUT_PORT0);
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port1,TCA9539_OUTPUT_PORT1);
}
/**********************************************************************//**
 * @brief  Turn OFF individual relay
 *
 * @param   none
 *
 * @return  none
 *************************************************************************/
static void set_individual_Relays_OFF(byte pin)
{
  /***change pin status & increment relay switching count***/
  if( pin == LOW_SPEED_RELAY_DRIVE_BIT6 )
  {
    Exp_Port0 &= ~LOW_SPEED_RELAY_DRIVE_BIT6;
    lsr_switching_count++;
  }
  else if( pin == MEDIUM_SPEED_RELAY_DRIVE_BIT7 )
  {
    Exp_Port0 &= ~MEDIUM_SPEED_RELAY_DRIVE_BIT7;
    msr_switching_count++;
  }
  else if( pin == HIGH_SPEED_RELAY_DRIVE_BIT0 )
  {
    Exp_Port1 &= (~HIGH_SPEED_RELAY_DRIVE_BIT0);
    hsr_switching_count++;
  }
  /***set value of pin in IO EXPANDER***/
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port0,TCA9539_OUTPUT_PORT0);
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port1,TCA9539_OUTPUT_PORT1);
  /***store value in Flash***/
  update_uint32_t_to_flash(HSR_COUNT_BASE_ADDR,hsr_switching_count);
  update_uint32_t_to_flash(MSR_COUNT_BASE_ADDR,msr_switching_count);
  update_uint32_t_to_flash(LSR_COUNT_BASE_ADDR,lsr_switching_count);
  /***Print relay switching count***/
  Serial.println("hsr_switching_count" + String(hsr_switching_count));
  Serial.println("msr_switching_count" + String(msr_switching_count));
  Serial.println("lsr_switching_count" + String(lsr_switching_count));
}
/**********************************************************************//**
 * @brief  configure the GPIO in IO EXPANDER
 *
 * @param   none
 *
 * @return  none
 *************************************************************************/
static void set_up_expander()
{
  Exp_Port0 = Exp_Port1 = Exp_Port_Cfg0 = Exp_Port_Cfg1 = 0x00;
  Exp_Port_Cfg0 &= ~(TOUCH_UP_LED_BIT3 | TOUCH_DOWN_LED_BIT4 | BUZZER_DRIVE_BIT5 | LOW_SPEED_RELAY_DRIVE_BIT6 | MEDIUM_SPEED_RELAY_DRIVE_BIT7);//configure pins as output
  Exp_Port_Cfg0 |= (TEMP_ALERT_BIT | SWITCH_BIT);                                                     //configure pins as input
  Exp_Port_Cfg1 &= ~(HIGH_SPEED_RELAY_DRIVE_BIT0 |LED_B2_BIT1 | ZIGBEE_BOOT_CTRL_BIT2 | ZIGBEE_NRESET_BIT3);    //configure pins as output
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port_Cfg0,TCA9539_CONFIG0);
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port_Cfg1,TCA9539_CONFIG1);
  Exp_Port1 = 0x00;
  Exp_Port1 |= ZIGBEE_BOOT_CTRL_BIT2; //Zigbee module PA5 should be set high at boot
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port1,TCA9539_OUTPUT_PORT1);
  delay(5);
  Exp_Port1 |= ZIGBEE_NRESET_BIT3;  //set zigbee reset pin high
  io_expander_write_one_byte(EXPANDER_SLAVE_ADDRESS,Exp_Port1,TCA9539_OUTPUT_PORT1);
}
/**********************************************************************//**
 * @brief  Used to clear the flash data
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void flush_out_Flash()
{
  for(uint16_t misc_count = 0;misc_count < FLASH_SIZE ; misc_count++)
  {
    EEPROM.writeUChar(misc_count,'\0');
  }
}
/**********************************************************************//**
 * @brief  Extract data by using delimiter
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void parse_macid(char *data , char *delimiter)
{
    // Returns first token
    char* token = strtok(data, delimiter);
    char* bulb_epid_list[5];
    int count=0;

    // Keep printing tokens while one of the
    // delimiters present in str[].
    while (token != NULL)
    {
        bulb_epid_list[count++] = token;
        token = strtok(NULL, delimiter);
        if( count == 1)
        {
          break;
        }
    }
    Serial.println("\nextracted data");
    for (int i = 0; i < count; i++)
    {
        Serial.println(bulb_epid_list[i]);
    }
}
/**********************************************************************//**
 * @brief  control relay for HVAC type
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static void hvac_relay_control(uint16_t Set_AC_Temperature)
{
  if( tirisi_ac_type == HVAC_TYPE )
  {
    Serial.println("\nHVAC Relay control");
    if( strcmp(json_extract.ac_mode,AUTO_MODE) == 0 )
    {
      Serial.println("AC mode : Auto");
      if( low_speed_relay.on || medium_speed_relay.on || high_speed_relay.on )
      {
        if( room_temperature <= LOW_ROOM_TEMP )
        {
          /*
           * High speed relay - OFF
           * Low speed relay  - ON
           * Medium speed relay - OFF
           */
          Relay_ON(LOW_SPEED_RELAY_DRIVE_BIT6);
          low_speed_relay.on = true;
          Serial.println("Turn ON low speed relay");
        }
        else if( (room_temperature >= (LOW_ROOM_TEMP + TEMP_HYSTERISIS)) && (room_temperature < (HIGH_ROOM_TEMP + TEMP_HYSTERISIS)) )
        {
          /*
           * High speed relay - OFF
           * Low speed relay  - OFF
           * Medium speed relay - ON
           */
          Relay_ON(MEDIUM_SPEED_RELAY_DRIVE_BIT7);
          medium_speed_relay.on = true;
          Serial.println("Turn ON medium speed relay");
        }
        else if( room_temperature >= HIGH_ROOM_TEMP )
        {
          /*
           * High speed relay - ON
           * Low speed relay  - OFF
           * Medium speed relay - OFF
           */
          Relay_ON(HIGH_SPEED_RELAY_DRIVE_BIT0);
          high_speed_relay.on = true;
          Serial.println("Turn ON high speed relay");
        }
      }
      else
      {
        if( room_temperature < LOW_ROOM_TEMP )
        {
          /*
           * High speed relay - OFF
           * Low speed relay  - ON
           * Medium speed relay - OFF
           */
          Relay_ON(LOW_SPEED_RELAY_DRIVE_BIT6);
          low_speed_relay.on = true;
          Serial.println("Turn ON low speed relay");
        }
        else if( (room_temperature >= LOW_ROOM_TEMP) && (room_temperature < HIGH_ROOM_TEMP) )
        {
          /*
           * High speed relay - OFF
           * Low speed relay  - OFF
           * Medium speed relay - ON
           */
          Relay_ON(MEDIUM_SPEED_RELAY_DRIVE_BIT7);
          medium_speed_relay.on = true;
          Serial.println("Turn ON medium speed relay");
        }
        else if( room_temperature >= HIGH_ROOM_TEMP )
        {
          /*
           * High speed relay - ON
           * Low speed relay  - OFF
           * Medium speed relay - OFF
           */
          Relay_ON(HIGH_SPEED_RELAY_DRIVE_BIT0);
          high_speed_relay.on = true;
          Serial.println("Turn ON high speed relay");
        }
      }
    }
    else if( strcmp(json_extract.ac_mode,COOL_MODE) == 0 )
    {
      Serial.println("AC mode : Cool");
      if( (Set_AC_Temperature >= MIN_TEMPERATURE) && (Set_AC_Temperature <= MAX_TEMPERATURE))
      {
        if( high_speed_relay.off )
        {
          if( room_temperature >= (Set_AC_Temperature + TEMP_HYSTERISIS) )
          {
            Relay_ON(HIGH_SPEED_RELAY_DRIVE_BIT0);
            Serial.println("Turn ON High Speed relay");
          }
          else
          {
            high_speed_relay.off = true;
            set_individual_Relays_OFF(HIGH_SPEED_RELAY_DRIVE_BIT0);
            Serial.println("Turn OFF High speed relay");
          }
        }
        else
        {
          if( room_temperature >= Set_AC_Temperature )
          {
            Serial.println("Turn ON High Speed relay");
            Relay_ON(HIGH_SPEED_RELAY_DRIVE_BIT0);
          }
          else
          {
            high_speed_relay.off = true;
            set_individual_Relays_OFF(HIGH_SPEED_RELAY_DRIVE_BIT0);
            Serial.println("Turn OFF High speed relay");
          }
        }
      }
      else
      {
        Serial.println("Process discarded\nReason: Temperature not in range (16 -28)");
      }
    }
    else
    {
      Serial.println("only Auto & Cool mode is allowed");
    }
  }
}

/**********************************************************************//**
 * @brief  shift array right side to make all data to same length and
 *         calculate secrete key
 *
 * @param  Room_Num
 *
 * @param  Static_Code
 *
 * @return copy_buffer
 *************************************************************************/
static uint8_t *get_hmac_key(uint8_t *Room_Num,uint8_t *Static_Code)
{
  static uint8_t return_buf[8]={0}; //need to us static else we we will loose the data
  uint8_t copy_buffer[8]={0};
  uint8_t copy_room_no[8]={0};
  uint8_t copy_static_code[8]={0};
  int shift=0,max_len=0,trialing_zero=0;

  /*
   *  secrete key generation  logic
   *  1.convert room number and static code to same length
   *  2.Do bit wise AND operation
   *
   *  case : 1 room number length == static code length
   *  31 31  30  31  room number

      43  58  50  4c  static code ( CXPL)

      -----------------------------------------
      1 16  16  0     secrete key
      -----------------------------------------

      case : 2 room number length < static code length
       0  31  30  31  room number

      43  58  50  4c  static code ( CXPL)

      -----------------------------------------
      0 16  16  0     secrete key
      -----------------------------------------

      case : 3 room number length > static code length
      31  30  31  31  32  room number

      0   43  58  50  4c  static code ( CXPL)

      -----------------------------------------
      0   0   16  16   0  secrete key
      -----------------------------------------
   */
  Length[ROOM_NUM] = strlen((const char*)Room_Num);
  Length[STATIC_CODE_LEN] = strlen((const char*)Static_Code);//store array length of static code

//  Serial.printf(" room len        = %d\n",Length[ROOM_NUM]);
//  Serial.printf(" static code len = %d\n",Length[STATIC_CODE_LEN]);

  max_len = Length[STATIC_CODE_LEN] > Length[ROOM_NUM] ? Length[STATIC_CODE_LEN]:Length[ROOM_NUM];
//  Serial.printf(" Max len         = %d\n",max_len);

  if(Length[ROOM_NUM] < Length[STATIC_CODE_LEN])
  {
//    Serial.println("shiting room number right side");
    for( shift = 0;shift < Length[STATIC_CODE_LEN] - Length[ROOM_NUM];shift++)
    {
      copy_buffer[shift] = '\0';
    }
    for( int index = 0;index < Length[ROOM_NUM];index++)
    {
      copy_buffer[shift] = Room_Num[index];  // need to change to room_no
      shift++;
    }
    copy_buffer[shift] = '\0';
    memcpy ((char*)copy_room_no, (const char*)copy_buffer, max_len);
    memcpy ((char*)copy_static_code, (const char*)Static_Code, max_len);
  }
  else if(Length[ROOM_NUM] > Length[STATIC_CODE_LEN])
  {
//    Serial.println("shiting static code right side");
    for( shift = 0;shift < Length[ROOM_NUM] - Length[STATIC_CODE_LEN];shift++)
    {
      copy_buffer[shift] = '\0';
    }
    for( int index = 0;index < Length[STATIC_CODE_LEN];index++)
    {
      copy_buffer[shift] = Static_Code[index];  // need to change to room_no
      shift++;
    }
    copy_buffer[shift] = '\0';
    memcpy((char*)copy_static_code, (const char*)copy_buffer, max_len);
    memcpy((char*)copy_room_no, (const char*)Room_Num, max_len);
  }
  else
  {
//    Serial.println("Keep as it");
    memcpy((char*)copy_room_no, (const char*)Room_Num, max_len);
    memcpy((char*)copy_static_code, (const char*)Static_Code, max_len);
  }

  Serial.println("calculating secrete key");
//  for( int index = 0;index < max_len;index++)
//  {
//    Serial.printf("%x\t",copy_room_no[index] );
//  }
//  Serial.println("room number");
//  Serial.println();
//  for( int index = 0;index < max_len;index++)
//  {
//    Serial.printf("%02x\t",copy_static_code[index] );
//  }
//  Serial.println("static code ( CXPL)");
//  Serial.println();
//  Serial.println("-----------------------------------------------------");
  for (int misc_count = 0;misc_count< max_len; misc_count++)
  {
    return_buf[misc_count] = copy_room_no[misc_count] & copy_static_code[misc_count];
//    Serial.printf("%02d\t",return_buf[misc_count]);
  }
  return_buf[max_len+1] = '\0';//add delimiter
//  Serial.println();
//  Serial.println("-----------------------------------------------------");
  trialing_zero = 0;
  for (int misc_count = 0;misc_count< max_len; misc_count++)
  {
    if( return_buf[misc_count] != '\0' )
    {
      return_buf[trialing_zero++] = return_buf[misc_count];
//      Serial.printf("%02d\t",return_buf[misc_count]);
    }
  }
  return_buf[trialing_zero++] = '\0';//add delimiter
//  Serial.println();
//  Serial.printf( "secrete key len = %d\n",strlen((const char*)return_buf));
  if( return_buf !=  NULL )
  {
    return return_buf;
  }
  else
  {
    return NULL;
  }

}
/**********************************************************************//**
 * @brief  Generate 32 byte Hmac code uisng SHA-256
 *
 * @param  key  - secret key
 *
 * @param  payload  - data to calculate hmac
 *
 * @return none
 *************************************************************************/
static uint16_t Generate_hmac(uint8_t *key,uint8_t *payload)
{

  mbedtls_md_context_t ctx; //create context
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256; //define sha-256 hmac we are using

  const size_t payloadLength = strlen((char*)payload);
  const size_t keyLength = strlen((char*)key);

  mbedtls_md_init(&ctx); //intialize the context
  if( mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1) != 0 ) //select hmac
  {
    Serial.println("Error : Hmac setup fail\nPossible reason:\n1.parameter failure\n2.memory allocation failure");
    return EXIT_FAILURE;
  }
  if( mbedtls_md_hmac_starts(&ctx, (const unsigned char *) key, keyLength) != 0 )//start the hmac process by passing secret key
  {
    Serial.println(" Error : Fail to start hmac");
    return EXIT_FAILURE;
  }
  if( mbedtls_md_hmac_update(&ctx, (const unsigned char *) payload, payloadLength) != 0 ) // generate hmac for payload
  {
    Serial.println(" Error : Fail to update hmac");
    return EXIT_FAILURE;
  }
  if( mbedtls_md_hmac_finish(&ctx, hmacResult) != 0 )//result
  {
    Serial.println("Error : Fails to generate Hmac");
    return EXIT_FAILURE;
  }
  mbedtls_md_free(&ctx);//call this function to complete the process

//  Serial.print("Hash: ");
//
//  for(int i= 0; i< sizeof(hmacResult); i++)
//  {
//    char str[3];
//
//    sprintf(str, "%02x\t", (int)hmacResult[i]);
//    if( (i % 10) == 0 )
//    {
//      Serial.println();
//    }
//    Serial.print(str);
//  }
//  Serial.println();
  return EXIT_SUCCESS;
}
/**********************************************************************//**
 * @brief  Delete character in the given string
 *
 * @param  data               - data where character to be deleted
 *
 * @param  no_of_char         - No of character to be deleted
 *
 *@param  position_to_delete  - From which location character to be deleted
 *
 *@param  length              - data length
 *
 * @return return_data        - buffer which has final deleted data
 *************************************************************************/
static char *delchar(char *data,int no_of_char, int position_to_delete)
{
  static char return_data[1024]={0}; //need to us static else we we will loose the data
  Serial.println();
  if ((no_of_char+position_to_delete-1) <= strlen(data))
  {
    strcpy(&data[position_to_delete-1],&data[no_of_char+position_to_delete-1]);
//    Serial.print(data);
  }
  strcpy(return_data,data);
  return return_data;
}
/**********************************************************************//**
 * @brief  Separate payload for hmac calculation
 *
 * @param  none
 *
 * @return none
 *************************************************************************/
static char *get_payload(uint8_t *Json_buffer,uint16_t length)
{
  /*
   * Note : use static type if we are going to pass local variable
   *        else will face issue
   */
  uint16_t buf_len[5] = {0};
  static uint8_t copy_json_buffer[1024]={0};
  static char *del_data;
  static char *del_hmac;
  static char *Payload;
  char *find_string = "payload";
  char* ptr;
  char* buffer_ptr;

  memcpy (copy_json_buffer, (const char*)Json_buffer, length);
//  Serial.println("\ncopied data");
//  Serial.println((char *)copy_json_buffer);
  buffer_ptr =  (char *)copy_json_buffer;



    // Find first occurrence of find_string in Json_buffer
    ptr = strstr(buffer_ptr, find_string);

    // Prints the result
    if (ptr)
    {
//        Serial.println("String found");
//        Serial.printf("loctaion = %d\n",ptr - buffer_ptr);

        del_hmac = delchar((char *)copy_json_buffer, (ptr - buffer_ptr - 1),2);
        buf_len[BYTE0] = strlen((char *)del_hmac);
//        Serial.printf("\nbuffer length = %d\n",buf_len[BYTE0]);
        memset(copy_json_buffer,'\0',sizeof(copy_json_buffer));

        memcpy (copy_json_buffer, (const char*)del_hmac, buf_len[BYTE0]);
//        Serial.println("\ncopied data");
//        Serial.println((char *)copy_json_buffer);
//        Serial.println("_______________________________________");
        if( buf_len[BYTE0] == strlen((char *)copy_json_buffer) )
        {
          find_char_location(copy_json_buffer,buf_len[BYTE0],'{');
          del_data = delchar((char *)copy_json_buffer, found_location[BYTE1] - found_location[BYTE0],found_location[BYTE0]);
          memset(found_location,'\0',sizeof(found_location));
          buf_len[BYTE1] = strlen(del_data);
//          Serial.printf("\ndel data payload length = %d\n",buf_len[BYTE1]);
          find_char_location((uint8_t *)del_data,buf_len[BYTE1],'}');
          if( buf_len[BYTE1] <= (buf_len[BYTE0] - 10) )
          {
//            Serial.println("\ngetting payload");
            Payload = delchar((char *)del_data, 1,found_location[BYTE1]);
          }
          buf_len[BYTE2] = strlen(Payload);
//          Serial.printf("\nhmac payload length = %d\n",buf_len[BYTE2]);
          if( buf_len[BYTE2] == (buf_len[BYTE1] - 1) )
          {
            Serial.println("\nHmac payload parsing success");
            return Payload;
          }
          else
          {
            Serial.println("\nHmac payload parsing failed");
            return NULL;
          }
        }
        else
        {
          Serial.println("\ncopy length mis match");
          return NULL;
        }
    }
    else
    {
        printf("String not found\n");
        return NULL;
    }




}
/**********************************************************************//**
 * @brief  Find location of character in the string
 *
 * @param  buffer - Data buffer in which we need to find character
 *
 * @param  length - Data buffer length
 *
 * @param  buffer - Character to be found
 *
 * @return none
 *************************************************************************/
static void find_char_location(uint8_t *buffer,int length,char ch)
{
  uint8_t location;
  location = 0;
  for( int index = 0;index < length;index++)
  {
    if( buffer[index] == ch)
    {
      found_location[location] = index+1;
//      Serial.printf("found at %d\n",found_location[location]);
      location++;
    }
  }
  found_location[location] = '\0';
}
/**********************************************************************//**
 * @brief  validate hmac for received data
 *
 * @param  copy_buffer  - Holds server response
 *
 * @return EXIT_FAILURE - on failure this will be return
 *         EXIT_SUCCESS - on success this will be return
 *
 *************************************************************************/
static uint16_t hmac_validation(uint8_t *copy_buffer)
{
  //uint8_t Room[7] = "11012"; //for testing
//  Serial.println();
//  Serial.println((char *)copy_buffer);
//  Serial.println("\nlength = " + String(strlen((char *)copy_buffer)));
  seceret_key = get_hmac_key(room_no,(uint8_t *)STATIC_CODE) ;
  if( seceret_key != NULL )
  {
    hmac_payload = get_payload(copy_buffer,strlen((char *)copy_buffer));
    if( hmac_payload != NULL )
    {
      if( Generate_hmac((uint8_t *)seceret_key,(uint8_t *)hmac_payload) != EXIT_FAILURE )
      {
        if( strcmp((const char*)hmacResult,json_extract.hmac) == 0 )
        {
          return EXIT_SUCCESS;
        }
        else
        {
          return EXIT_FAILURE;
        }
      }
      else
      {
        return EXIT_FAILURE;
      }
    }
    else
    {
      return EXIT_FAILURE;
    }
  }
  else
  {
    return EXIT_FAILURE;
  }
}
/**********************************************************************//**
 * @brief  update time in flash
 *
 * @param  variant  - select light or AC
 *
 * @return
 *
 *************************************************************************/
static void update_variant_data(uint8_t variant)
{
  if( variant == AC )
  {
    Current_Time[AC] = millis();
    if(Current_Time[AC] - Last_Time[AC] >= VARIANT_UPDATE_TIME)
    {
      //every 5 minutes once update data
      /*** Send AC fail packet by following steps below
       *  1.Check AC is ON
       *  2.Check room temperature is in range of +-1 AC set temperature.If so send AC fail packet
       */
      cummulative_on_minute(AIR_CONDITIONER,ac.on_minute);
      update_uint32_t_to_flash(AC_CUM_ON_TIME_BAS_ADDR,cum_on_time[AC]);
      if( json_extract.ac_status == DEVICE_TURN_ON )
      {
        if( (room_temperature >= (json_extract.ac_temp - 1)) && (room_temperature <= (json_extract.ac_temp + 1)) )
        {
          Serial.println("\nAC working Fine");
        }
        else
        {
          Serial.println("\nError : AC Fails");
          read_temperature();
          Json_post_to_AP(AC_FAIL_PACKET,AMENOS,NULL,NULL);//send sensor data
        }
      }
      maintain_ac_setting();
      Last_Time[AC] = millis();
    }
  }
  else if( variant == BULB )
  {
    Current_Time[BULB] = millis();
    if(Current_Time[BULB] - Last_Time[BULB] >= VARIANT_UPDATE_TIME)
    {
      //every 5 minutes once update data
      update_uint32_t_to_flash(LIGHT_CUM_ON_TIME_BAS_ADDR,cum_on_time[BULB]);
      Last_Time[BULB] = millis();
    }
  }
}
/**********************************************************************//**
 * @brief  print Light look up table
 *
 * @param  array1[ROW][COLOMN]  - 2D array buffer
 *
 * @return
 *
 *************************************************************************/
static void print_light_lookup_table( String array1[ROW][COLOMN] )
{
  Serial.println("Count\tZone\t   Mac id\tTotal ON time(min)");
  Serial.println("----------------------------------------------------");
  for (int i = 0; i < bulb_count; ++i)
  {
    for (int j = 0; j < COLOMN; ++j)
    {
        Serial.print("\t");
        Serial.print(array1[i][j]);
    }
    Serial.println();
  }
  Serial.println("----------------------------------------------------");
}
/**********************************************************************//**
 * @brief  print light default table
 *
 * @param  array1[LIGHT_ZONE_SIZE][6]  - 2D array buffer
 *
 * @return
 *
 *************************************************************************/
static void print_default_table( String array1[LIGHT_ZONE_SIZE][6] ,int variant )
{
  if( variant == SMART_BULB )
  {
    Serial.println("Zone\tcolor\tLight status\tContrast\tFrom Time\tTo Time");
    Serial.println("----------------------------------------------------");
    for (int i = 0; i < LIGHT_ZONE_SIZE; ++i)
    {
      for (int j = 0; j < 6; ++j)
      {
          Serial.print("\t");
          Serial.print(array1[i][j]);
      }
      Serial.println();
    }
    Serial.println("----------------------------------------------------");
  }
  else if (variant == AIR_CONDITIONER)
  {
    Serial.println("Mode\tTemp\tstatus\tFrom Time\tTo Time");
    Serial.println("----------------------------------------------------");
    for (int i = 0; i < LIGHT_ZONE_SIZE; ++i)
    {
      for (int j = 0; j < 6; ++j)
      {
          Serial.print(array1[i][j]);
          Serial.print("\t");
      }
      Serial.println();
    }
    Serial.println("----------------------------------------------------");
  }
}
/**********************************************************************//**
 * @brief  print light default table
 *
 * @param  array1[LIGHT_ZONE_SIZE][6]  - 2D array buffer
 *
 * @return
 *
 *************************************************************************/
static void print_Kinsi_lookup_table( String array1[MAX_KINISI_DEVICE][6] )
{
  Serial.println("Count\tSerial_number\tDevice_name\tDecom\tRegister\t\tmac_id");
  Serial.println("------------------------------------------------------------------------------------------------------------");
  for (int i = 0; i < MAX_KINISI_DEVICE; ++i)
  {
    for (int j = 0; j < 6; ++j)
    {
      Serial.print("\t");
      Serial.print(array1[i][j]);
    }
    Serial.println();
  }
  Serial.println("------------------------------------------------------------------------------------------------------------");
}
/**********************************************************************//**
 * @brief  Initialize timer as sleep mode wake up source
 *
 * @param  array1[LIGHT_ZONE_SIZE][6]  - 2D array buffer
 *
 * @return
 *
 *************************************************************************/
static void init_wakeup_timer()
{
  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 10 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}
/**********************************************************************//**
 * @brief  Get default AC time
 *
 * @param  array1[LIGHT_ZONE_SIZE][6]  - 2D array buffer
 *
 * @return Time in minutes
 *
 *************************************************************************/
static uint32_t get_ac_mode_change_time(String time_table)
{
  static uint32_t time_in_sec = 0;
  current.hour    = (((time_table[0] - CHARACTER_OFFSET) * 10) + (time_table[1] - CHARACTER_OFFSET));
  current.minute  = (((time_table[3] - CHARACTER_OFFSET) * 10) + (time_table[4] - CHARACTER_OFFSET));
  current.sec     = (((time_table[6] - CHARACTER_OFFSET) * 10) + (time_table[7] - CHARACTER_OFFSET));
  time_in_sec = ( ( current.hour * HR_TO_MIN ) + current.minute);
  Serial.println("\ntime in sec " + String(time_in_sec));
  return time_in_sec ;
}
/**********************************************************************//**
 * @brief  maintaining default AC setting
 *
 * @param
 *
 * @return
 *
 *************************************************************************/
static void maintain_ac_setting()
{
  print_default_table(default_ac_setting_table,AIR_CONDITIONER);
  for(int i =0 ;i<3;i++)
  {
    Serial.println("From Time : " + String(From_Time[i]) + "\tTo Time :" + String(To_Time[i])) ;
  }
  uint32_t curent_time = get_time();
  if( (curent_time >= From_Time[MODE001]) && (curent_time <= To_Time[MODE001]) )
  {
    //Serial.println( "nSet temperature to 26");
    if( UDP_Rx_Packet[AC_CTRL] )
    {
      if( tirisi_ac_type == HVAC_TYPE )
      {
        hvac_relay_control(json_extract.ac_temp);
      }
      else if(tirisi_ac_type == SPLIT_AC_TYPE )
      {
        send_ir_ac_command(json_extract.ac_temp);
      }
      display_temperature(json_extract.ac_temp);
    }
    else
    {
      json_extract.ac_temp  = atoi(default_ac_setting_table[MODE001][1].c_str());
      if( tirisi_ac_type == HVAC_TYPE )
      {
        hvac_relay_control(json_extract.ac_temp);
      }
      else if(tirisi_ac_type == SPLIT_AC_TYPE )
      {
        json_extract.ac_status  = atoi(default_ac_setting_table[MODE001][2].c_str());
        json_extract.ac_mode    = default_ac_setting_table[MODE001][0].c_str();
        send_ir_ac_command(json_extract.ac_temp);
      }
      display_temperature(json_extract.ac_temp);
    }
  }
  else if( (curent_time >= From_Time[MODE101]) && (curent_time <= To_Time[MODE101]) )
  {
    //Serial.println( "\nSet temperature to 18");
    if( UDP_Rx_Packet[AC_CTRL] )
    {
      if( tirisi_ac_type == HVAC_TYPE )
      {
        hvac_relay_control(json_extract.ac_temp);
      }
      else if(tirisi_ac_type == SPLIT_AC_TYPE )
      {
        send_ir_ac_command(json_extract.ac_temp);
      }
      display_temperature(json_extract.ac_temp);
    }
    else
    {
      json_extract.ac_temp  = atoi(default_ac_setting_table[MODE101][1].c_str());
      if( tirisi_ac_type == HVAC_TYPE )
      {
        hvac_relay_control(json_extract.ac_temp);
      }
      else if(tirisi_ac_type == SPLIT_AC_TYPE )
      {
        json_extract.ac_status  = atoi(default_ac_setting_table[MODE101][2].c_str());
        json_extract.ac_mode    = default_ac_setting_table[MODE101][0].c_str();
        send_ir_ac_command(json_extract.ac_temp);
      }
      display_temperature(json_extract.ac_temp);
    }
  }
  else if( (curent_time >= From_Time[MODE100]) && (curent_time <= To_Time[MODE100]) )
  {
    //Serial.println( "\nSet temperature to 28");
    if( UDP_Rx_Packet[AC_CTRL] )
    {
      if( tirisi_ac_type == HVAC_TYPE )
      {
        hvac_relay_control(json_extract.ac_temp);
      }
      else if(tirisi_ac_type == SPLIT_AC_TYPE )
      {
        send_ir_ac_command(json_extract.ac_temp);
      }
      display_temperature(json_extract.ac_temp);
    }
    else
    {
      json_extract.ac_temp  = atoi(default_ac_setting_table[MODE100][1].c_str());
      if( tirisi_ac_type == HVAC_TYPE )
      {
        hvac_relay_control(json_extract.ac_temp);
      }
      else if(tirisi_ac_type == SPLIT_AC_TYPE )
      {
        json_extract.ac_status  = atoi(default_ac_setting_table[MODE100][2].c_str());
        json_extract.ac_mode    = default_ac_setting_table[MODE100][0].c_str();
        send_ir_ac_command(json_extract.ac_temp);
      }
      display_temperature(json_extract.ac_temp);
    }
  }
  else
  {
    Serial.println("Not Handled");
  }
}
/**********************************************************************//**
 * @brief  Split AC control command
 *
 * @param
 *
 * @return
 *
 *************************************************************************/
static void send_ir_ac_command(uint16_t set_temperature)
{
  unsigned long end = 0;
  float delta = 0;
  unsigned long start = 0;
  start = millis();
  for(int ac_brand = COOLIX_BRAND; ac_brand <= WHIRLPOOL_BRAND; ac_brand++)
  {
    switch(ac_brand)
    {
      case COOLIX_BRAND:
            Serial.println("\nsending command to COOLIX brand");
            coolix.begin();
            irsend.begin();
            if( json_extract.ac_status == DEVICE_TURN_ON )
            {
              /***Turn ON AC & set Fab speed to Auto***/
              coolix.on();
              coolix.setFan(kCoolixFanAuto);
              /***Set AC functional Mode***/
              if( strcmp(json_extract.ac_mode,AUTO_MODE) == 0 )
              {
                coolix.setMode(kCoolixAuto);
              }
              else if( strcmp(json_extract.ac_mode,COOL_MODE) == 0 )
              {
                coolix.setMode(kCoolixCool);
              }
              else if( strcmp(json_extract.ac_mode,DRY_MODE) == 0 )
              {
                coolix.setMode(kCoolixDry);
              }
              else if( strcmp(json_extract.ac_mode,FAN_MODE) == 0 )
              {
                coolix.setMode(kCoolixFan);
              }
              else
              {
                //set to default mode
                coolix.setMode(kCoolixAuto);
              }
              /***Set Temperature***/
              if ( (set_temperature >= MIN_TEMPERATURE) && (set_temperature <= MAX_TEMPERATURE) )
              {
                coolix.setTemp(set_temperature);
              }

            }
            else if( json_extract.ac_status == DEVICE_TURN_OFF )
            {
              /***Turn OFF AC***/
              coolix.off();
            }
            //send all above configuration
            coolix.send();
            break;
      case HAIER_BRAND:
            Serial.println("sending command to HAIER brand");
            haier.begin();
            irsend.begin();
            if( json_extract.ac_status == DEVICE_TURN_ON )
            {
              /***Turn ON AC & set Fab speed to Auto***/
              haier.setCommand(kHaierAcCmdOn);
              haier.setFan(kCoolixFanAuto);
              /***Set AC functional Mode***/
              if( strcmp(json_extract.ac_mode,AUTO_MODE) == 0 )
              {
                haier.setMode(kHaierAcAuto);
              }
              else if( strcmp(json_extract.ac_mode,COOL_MODE) == 0 )
              {
                haier.setMode(kHaierAcCool);
              }
              else if( strcmp(json_extract.ac_mode,DRY_MODE) == 0 )
              {
                haier.setMode(kHaierAcDry);
              }
              else if( strcmp(json_extract.ac_mode,FAN_MODE) == 0 )
              {
                haier.setMode(kHaierAcFan);
              }
              else
              {
                //set to default mode
                haier.setMode(kHaierAcAuto);
              }
              /***Set Temperature***/
              if ( (set_temperature >= MIN_TEMPERATURE) && (set_temperature <= MAX_TEMPERATURE) )
              {
                haier.setTemp(set_temperature);
              }
            }
            else if( json_extract.ac_status == DEVICE_TURN_OFF )
            {
              /***Turn OFF AC***/
              haier.setCommand(kHaierAcCmdOff);
            }
            //send all above configuration
            haier.send();
            break;
      case PANASONIC_BRAND:
            Serial.println("sending command to PANASONIC brand");
            panasonic.begin();
            irsend.begin();
            if( json_extract.ac_status == DEVICE_TURN_ON )
            {
              /***Turn ON AC & set Fab speed to Auto***/
              panasonic.on();
              panasonic.setFan(kPanasonicAcFanAuto);
              /***Set AC functional Mode***/
              if( strcmp(json_extract.ac_mode,AUTO_MODE) == 0 )
              {
                panasonic.setMode(kPanasonicAcAuto);
              }
              else if( strcmp(json_extract.ac_mode,COOL_MODE) == 0 )
              {
                panasonic.setMode(kPanasonicAcCool);
              }
              else if( strcmp(json_extract.ac_mode,DRY_MODE) == 0 )
              {
                panasonic.setMode(kPanasonicAcDry);
              }
              else if( strcmp(json_extract.ac_mode,FAN_MODE) == 0 )
              {
                panasonic.setMode(kPanasonicAcFan);
              }
              else
              {
                //set to default mode
                panasonic.setMode(kPanasonicAcAuto);
              }
              /***Set Temperature***/
              if ( (set_temperature >= MIN_TEMPERATURE) && (set_temperature <= MAX_TEMPERATURE) )
              {
                panasonic.setTemp(set_temperature);
              }
            }
            else if( json_extract.ac_status == DEVICE_TURN_OFF )
            {
              /***Turn OFF AC***/
              panasonic.off();
            }
            //send all above configuration
            panasonic.send();
            break;
      case SAMSUNG_BRAND:
            Serial.println("sending command to SAMSUNG brand");
            samsung.begin();
            irsend.begin();
            if( json_extract.ac_status == DEVICE_TURN_ON )
            {
              /***Turn ON AC & set Fab speed to Auto***/
              samsung.on();
              samsung.setFan(kSamsungAcFanAuto);
              /***Set AC functional Mode***/
              if( strcmp(json_extract.ac_mode,AUTO_MODE) == 0 )
              {
                samsung.setMode(kSamsungAcAuto);
              }
              else if( strcmp(json_extract.ac_mode,COOL_MODE) == 0 )
              {
                samsung.setMode(kSamsungAcCool);
              }
              else if( strcmp(json_extract.ac_mode,DRY_MODE) == 0 )
              {
                samsung.setMode(kSamsungAcDry);
              }
              else if( strcmp(json_extract.ac_mode,FAN_MODE) == 0 )
              {
                samsung.setMode(kSamsungAcFan);
              }
              else
              {
                //set to default mode
                samsung.setMode(kSamsungAcAuto);
              }
              /***Set Temperature***/
              if ( (set_temperature >= MIN_TEMPERATURE) && (set_temperature <= MAX_TEMPERATURE) )
              {
                samsung.setTemp(set_temperature);
              }
            }
            else if( json_extract.ac_status == DEVICE_TURN_OFF )
            {
              /***Turn OFF AC***/
              samsung.off();
            }
            //send all above configuration
            samsung.send();
            break;
      case TCL_BRAND:
            Serial.println("sending command to TCL brand");
            tcl.begin();
            irsend.begin();
            if( json_extract.ac_status == DEVICE_TURN_ON )
            {
              /***Turn ON AC & set Fab speed to Auto***/
              tcl.on();
              tcl.setFan(kTcl112AcFanAuto);
              /***Set AC functional Mode***/
              if( strcmp(json_extract.ac_mode,AUTO_MODE) == 0 )
              {
                tcl.setMode(kTcl112AcAuto);
              }
              else if( strcmp(json_extract.ac_mode,COOL_MODE) == 0 )
              {
                tcl.setMode(kTcl112AcCool);
              }
              else if( strcmp(json_extract.ac_mode,DRY_MODE) == 0 )
              {
                tcl.setMode(kTcl112AcDry);
              }
              else if( strcmp(json_extract.ac_mode,FAN_MODE) == 0 )
              {
                tcl.setMode(kTcl112AcFan);
              }
              else
              {
                //set to default mode
                tcl.setMode(kTcl112AcAuto);
              }
              /***Set Temperature***/
              if ( (set_temperature >= MIN_TEMPERATURE) && (set_temperature <= MAX_TEMPERATURE) )
              {
                tcl.setTemp(set_temperature);
              }
            }
            else if( json_extract.ac_status == DEVICE_TURN_OFF )
            {
              /***Turn OFF AC***/
              tcl.off();
            }
            tcl.send();
            break;
      case TOSHIBA_BRAND:
            Serial.println("sending command to TOSHIBA brand");
            tosibha.begin();
            irsend.begin();
            if( json_extract.ac_status == DEVICE_TURN_ON )
            {
              /***Turn ON AC & set Fab speed to Auto***/
              tosibha.on();
              tosibha.setFan(kToshibaAcFanAuto);
              /***Set AC functional Mode***/
              if( strcmp(json_extract.ac_mode,AUTO_MODE) == 0 )
              {
                tosibha.setMode(kToshibaAcAuto);
              }
              else if( strcmp(json_extract.ac_mode,COOL_MODE) == 0 )
              {
                tosibha.setMode(kToshibaAcCool);
              }
              else if( strcmp(json_extract.ac_mode,DRY_MODE) == 0 )
              {
                tosibha.setMode(kToshibaAcDry);
              }
              else if( strcmp(json_extract.ac_mode,FAN_MODE) == 0 )
              {
                Serial.println("No Fan Mode");
              }
              else
              {
                //set to default mode
                tosibha.setMode(kToshibaAcAuto);
              }
              /***Set Temperature***/
              if ( (set_temperature >= MIN_TEMPERATURE) && (set_temperature <= MAX_TEMPERATURE) )
              {
                tosibha.setTemp(set_temperature);
              }
            }
            else if( json_extract.ac_status == DEVICE_TURN_OFF )
            {
              /***Turn OFF AC***/
              tosibha.off();
            }
            tosibha.send();
            break;
      case WHIRLPOOL_BRAND:
            Serial.println("sending command to WHIRLPOOL brand");
            whirlpool.begin();
            irsend.begin();
            if( json_extract.ac_status == DEVICE_TURN_ON )
            {
              /***Turn ON AC & set Fab speed to Auto***/
              whirlpool.setCommand(kWhirlpoolAcCommandPower);
              whirlpool.setFan(kWhirlpoolAcFanAuto);
              /***Set AC functional Mode***/
              if( strcmp(json_extract.ac_mode,AUTO_MODE) == 0 )
              {
                whirlpool.setMode(kWhirlpoolAcAuto);
              }
              else if( strcmp(json_extract.ac_mode,COOL_MODE) == 0 )
              {
                whirlpool.setMode(kWhirlpoolAcCool);
              }
              else if( strcmp(json_extract.ac_mode,DRY_MODE) == 0 )
              {
                whirlpool.setMode(kWhirlpoolAcDry);
              }
              else if( strcmp(json_extract.ac_mode,FAN_MODE) == 0 )
              {
                whirlpool.setMode(kWhirlpoolAcFan);
              }
              else
              {
                //set to default mode
                whirlpool.setMode(kWhirlpoolAcAuto);
              }
              /***Set Temperature***/
              if ( (set_temperature >= MIN_TEMPERATURE) && (set_temperature <= MAX_TEMPERATURE) )
              {
                whirlpool.setTemp(set_temperature);
              }
            }
            else if( json_extract.ac_status == DEVICE_TURN_OFF )
            {
              /***Turn OFF AC***/
              whirlpool.setCommand(kWhirlpoolAcCommandSleep);
            }
            //send all above configuration
            whirlpool.send();
            break;
      default:
        Serial.println("deafult break");
        break;
    }
  }
  end = millis();
  delta = (end - start)/1000;
  Serial.println("Execution Time :" + String(delta) + "sec");
}

/**********************************************************************//**
 * @brief  Initialize light sensor
 *
 * @param
 *
 * @return
 *
 *************************************************************************/
static void setup_light_sensor()
{
  // You can pass nothing to light.begin() for the default I2C address (0x29)
  light_sensor.begin();

  // Get factory ID from sensor:
  // (Just for fun, you don't need to do this to operate the sensor)

  unsigned char ID;

  if (light_sensor.getPartID(ID))
  {
    Serial.print("Got Sensor Part ID: 0X");
    Serial.print(ID,HEX);
  }
  // Most library commands will return true if communications was successful,
  // and false if there was a problem. You can ignore this returned value,
  // or check whether a command worked correctly and retrieve an error code:
  else
  {
    byte error = light_sensor.getError();
    printError(error);
  }

  // The light sensor has a default integration time of 100ms,
  // and a default gain of low (1X).

  // If you would like to change either of these, you can
  // do so using the setControl() and setMeasurementRate() command.

  // Gain can take any value from 0-7, except 4 & 5
  // If gain = 0, device is set to 1X gain (default)
  // If gain = 1, device is set to 2X gain
  // If gain = 2, device is set to 4X gain
  // If gain = 3, device is set to 8X gain
  // If gain = 4, invalid
  // If gain = 5, invalid
  // If gain = 6, device is set to 48X gain
  // If gain = 7, device is set to 96X gain
  gain = 0;
  Serial.println("Setting Gain...");
  light_sensor.setControl(gain, false, false);

  // If integrationTime = 0, integrationTime will be 100ms (default)
  // If integrationTime = 1, integrationTime will be 50ms
  // If integrationTime = 2, integrationTime will be 200ms
  // If integrationTime = 3, integrationTime will be 400ms
  // If integrationTime = 4, integrationTime will be 150ms
  // If integrationTime = 5, integrationTime will be 250ms
  // If integrationTime = 6, integrationTime will be 300ms
  // If integrationTime = 7, integrationTime will be 350ms

  unsigned char time = 5;

  // If integrationTime = 0, integrationTime will be 100ms (default)
  // If integrationTime = 1, integrationTime will be 50ms
  // If integrationTime = 2, integrationTime will be 200ms
  // If integrationTime = 3, integrationTime will be 400ms
  // If integrationTime = 4, integrationTime will be 150ms
  // If integrationTime = 5, integrationTime will be 250ms
  // If integrationTime = 6, integrationTime will be 300ms
  // If integrationTime = 7, integrationTime will be 350ms

  Serial.println("\nSet timing...");
  light_sensor.setMeasurementRate(time,3);

  // To start taking measurements, power up the sensor:

  Serial.println("Powerup...");
  light_sensor.setPowerUp();

  // The sensor will now gather light_sensor during the integration time.
  // After the specified time, you can retrieve the result from the sensor.
  // Once a measurement occurs, another integration period will start.
}
/**********************************************************************//**
 * @brief  Get room light intensity
 *
 * @param
 *
 * @return
 *
 *************************************************************************/
static bool get_lux_value()
{
  // Wait between measurements before retrieving the result
  // You can also configure the sensor to issue an interrupt
  // when measurements are complete)

  // This sketch uses the LTR303's built-in integration timer.

//  int ms = 1000;
//
//  delay(ms);


  // Once integration is complete, we'll retrieve the data.

  // There are two light sensors on the device, one for visible light
  // and one for infrared. Both sensors are needed for lux calculations.

  // Retrieve the data from the device:

  unsigned int data0, data1;

  if (light_sensor.getData(data0,data1))
  {
    // getData() returned true, communication was successful
//
//    Serial.print("data0: ");
//    Serial.println(data0);
//    Serial.print("data1: ");
//    Serial.println(data1);

    // To calculate lux, pass all your settings and readings
    // to the getLux() function.

    // The getLux() function will return 1 if the calculation
    // was successful, or 0 if one or both of the sensors was
    // saturated (too much light). If this happens, you can
    // reduce the integration time and/or gain.

    boolean good;  // True if neither sensor is saturated

    // Perform lux calculation:
    integrationTime = 100; //In millisec
    good = light_sensor.getLux(gain,integrationTime,data0,data1,lux);

    // Print out the results:

    if (good)
    {
//      Serial.println(" (good)");
//      Serial.print(" lux: ");
//      Serial.println(lux);
      return EXIT_SUCCESS;
    }
    else
    {
      Serial.println(" (BAD)");
      return EXIT_FAILURE;
    }
  }
  else
  {
    // getData() returned false because of an I2C error, inform the user.

    byte error = light_sensor.getError();
    printError(error);
    return EXIT_FAILURE;
  }
}
/**********************************************************************//**
 * @brief  print light sensor error  code
 *
 * @param
 *
 * @return
 *
 *************************************************************************/
static void printError(byte error)
{
  // If there's an I2C error, this function will
  // print out an explanation.

  Serial.print("I2C error: ");
  Serial.print(error,DEC);
  Serial.print(", ");

  switch(error) {
    case 0:
      Serial.println("success");
      break;
    case 1:
      Serial.println("data too long for transmit buffer");
      break;
    case 2:
      Serial.println("received NACK on address (disconnected?)");
      break;
    case 3:
      Serial.println("received NACK on data");
      break;
    case 4:
      Serial.println("other error");
      break;
    default:
      Serial.println("unknown error");
  }
}

/**********************************************************************//**
 * @brief  control DL1 RGB led
 *
 * @param  pin        - led pin to control
 *
 * @param  control    - set or reset
 *
 * @return
 *
 *************************************************************************/
 static void DL1_led_control(char pin,char control)
 {
  switch(pin)
  {
    case LED_R1:
          if( control == LED_DRIVE_HIGH )
          {
            digitalWrite(LED_R1, LED_DRIVE_HIGH);
            digitalWrite(LED_G1, LED_DRIVE_LOW);
            digitalWrite(LED_B1, LED_DRIVE_LOW);
          }
          else if( control == LED_DRIVE_LOW )
          {
            digitalWrite(LED_R1, LED_DRIVE_LOW);
          }
          else if ( control == TOGGLE )
          {
            digitalWrite(LED_R1, !digitalRead(LED_R1));
            digitalWrite(LED_G1, LED_DRIVE_LOW);
            digitalWrite(LED_B1, LED_DRIVE_LOW);
          }
          break;
    case LED_G1:
          if( control == LED_DRIVE_HIGH )
          {
            digitalWrite(LED_R1, LED_DRIVE_LOW);
            digitalWrite(LED_G1, LED_DRIVE_HIGH);
            digitalWrite(LED_B1, LED_DRIVE_LOW);
          }
          else if( control == LED_DRIVE_LOW )
          {
            digitalWrite(LED_G1, LED_DRIVE_LOW);
          }
          else if ( control == TOGGLE )
          {
            digitalWrite(LED_G1, !digitalRead(LED_G1));
            digitalWrite(LED_R1, LED_DRIVE_LOW);
            digitalWrite(LED_B1, LED_DRIVE_LOW);
          }
          break;
    case LED_B1:
          if( control == LED_DRIVE_HIGH )
          {
            digitalWrite(LED_R1, LED_DRIVE_LOW);
            digitalWrite(LED_G1, LED_DRIVE_LOW);
            digitalWrite(LED_B1, LED_DRIVE_HIGH);
          }
          else if( control == LED_DRIVE_LOW )
          {
            digitalWrite(LED_B1, LED_DRIVE_LOW);
          }
          else if ( control == TOGGLE )
          {
            digitalWrite(LED_B1, !digitalRead(LED_B1));
            digitalWrite(LED_R1, LED_DRIVE_LOW);
            digitalWrite(LED_G1, LED_DRIVE_LOW);
          }
          break;
    default:
          digitalWrite(LED_R1, LED_DRIVE_LOW);
          digitalWrite(LED_G1, LED_DRIVE_LOW);
          digitalWrite(LED_B1, LED_DRIVE_LOW);
          Serial.println("DL1 led turn off");
          break;

  }
 }
 /**********************************************************************//**
 * @brief  control DL2 RGB led
 *
 * @param  pin        - led pin to control
 *
 * @param  control    - set or reset
 *
 * @return
 *
 *************************************************************************/
 static void DL2_led_control(char pin,char control)
 {
  switch(pin)
  {
    case LED_R2:
          if( control == LED_DRIVE_HIGH )
          {
            digitalWrite(LED_R2, LED_DRIVE_HIGH);
            digitalWrite(LED_G2, LED_DRIVE_LOW);
            set_Led_B2_OFF();
          }
          else if( control == LED_DRIVE_LOW )
          {
            digitalWrite(LED_R2, LED_DRIVE_LOW);
          }
          else if ( control == TOGGLE )
          {
            digitalWrite(LED_R2, !digitalRead(LED_R2));
            digitalWrite(LED_G2, LED_DRIVE_LOW);
            set_Led_B2_OFF();
          }
          break;
    case LED_G2:
          if( control == LED_DRIVE_HIGH )
          {
            digitalWrite(LED_R2, LED_DRIVE_LOW);
            digitalWrite(LED_G2, LED_DRIVE_HIGH);
            set_Led_B2_OFF();
          }
          else if( control == LED_DRIVE_LOW )
          {
            digitalWrite(LED_G2, LED_DRIVE_LOW);
          }
          else if ( control == TOGGLE )
          {
            digitalWrite(LED_G2, !digitalRead(LED_G2));
            digitalWrite(LED_R2, LED_DRIVE_LOW);
            set_Led_B2_OFF();
          }
          break;
    case LED_B2_BIT1:
          if( control == LED_DRIVE_HIGH )
          {
            digitalWrite(LED_R2, LED_DRIVE_LOW);
            digitalWrite(LED_G2, LED_DRIVE_LOW);
            set_Led_B2_ON();
          }
          else if( control == LED_DRIVE_LOW )
          {
            set_Led_B2_OFF();
          }
          else if ( control == TOGGLE )
          {
//            digitalWrite(LED_B2_BIT1, !digitalRead(LED_B2_BIT1)); //have to check .don't forget
          }
          break;
    default:
          digitalWrite(LED_R2, LED_DRIVE_LOW);
          digitalWrite(LED_G2, LED_DRIVE_LOW);
          set_Led_B2_OFF();
          Serial.println("DL2 led turn off");
          break;

  }
 }
/**********************************************************************//**
 * @brief  control DL2 RGB led
 *
 * @param  none
 *
 * @return
 *
 *************************************************************************/
 static void set_dot_matrix_intensity()
 {
  if ( lux >= 500 )
  {
    for (int DM_device_count = 0 ; DM_device_count < NUMBER_OF_DEVICES ; DM_device_count++)
    {
      /* Change Dot matrix brightness  */
      lc.setIntensity(DM_device_count,(MIN_INTENSITY + 20));
    }
    Serial.println("set intensity = " + String(MIN_INTENSITY + 20));
  }
  else if ( lux >= 450 )
  {
    for (int DM_device_count = 0 ; DM_device_count < NUMBER_OF_DEVICES ; DM_device_count++)
    {
      /* Change Dot matrix brightness  */
      lc.setIntensity(DM_device_count,(MIN_INTENSITY + 18));
    }
    Serial.println("set intensity = " + String(MIN_INTENSITY + 18));
  }
  else if ( lux >= 400 )
  {
    for (int DM_device_count = 0 ; DM_device_count < NUMBER_OF_DEVICES ; DM_device_count++)
    {
      /* Change Dot matrix brightness  */
      lc.setIntensity(DM_device_count,(MIN_INTENSITY + 16));
    }
    Serial.println("set intensity = " + String(MIN_INTENSITY + 16));
  }
  else if ( lux >= 350 )
  {
    for (int DM_device_count = 0 ; DM_device_count < NUMBER_OF_DEVICES ; DM_device_count++)
    {
      /* Change Dot matrix brightness  */
      lc.setIntensity(DM_device_count,(MIN_INTENSITY + 14));
    }
    Serial.println("set intensity = " + String(MIN_INTENSITY + 14));
  }
  else if ( lux >= 300 )
  {
    for (int DM_device_count = 0 ; DM_device_count < NUMBER_OF_DEVICES ; DM_device_count++)
    {
      /* Change Dot matrix brightness  */
      lc.setIntensity(DM_device_count,(MIN_INTENSITY + 12));
    }
    Serial.println("set intensity = " + String(MIN_INTENSITY));
  }
  if ( lux >= 250 )
  {
    for (int DM_device_count = 0 ; DM_device_count < NUMBER_OF_DEVICES ; DM_device_count++)
    {
      /* Change Dot matrix brightness  */
      lc.setIntensity(DM_device_count,(MIN_INTENSITY + 10));
    }
    Serial.println("set intensity = " + String(MIN_INTENSITY + 12));
  }
  else if ( lux >= 200 )
  {
    for (int DM_device_count = 0 ; DM_device_count < NUMBER_OF_DEVICES ; DM_device_count++)
    {
      /* Change Dot matrix brightness  */
      lc.setIntensity(DM_device_count,(MIN_INTENSITY + 8));
    }
    Serial.println("set intensity = " + String(MIN_INTENSITY + 8));
  }
  if ( lux >= 150 )
  {
    for (int DM_device_count = 0 ; DM_device_count < NUMBER_OF_DEVICES ; DM_device_count++)
    {
      /* Change Dot matrix brightness  */
      lc.setIntensity(DM_device_count,(MIN_INTENSITY + 6));
    }
    Serial.println("set intensity = " + String(MIN_INTENSITY + 6));
  }
  else if ( lux >= 100 )
  {
    for (int DM_device_count = 0 ; DM_device_count < NUMBER_OF_DEVICES ; DM_device_count++)
    {
      /* Change Dot matrix brightness  */
      lc.setIntensity(DM_device_count,(MIN_INTENSITY + 4));
    }
    Serial.println("set intensity = " + String(MIN_INTENSITY + 4));
  }
  else if ( lux >= 50 )
  {
    for (int DM_device_count = 0 ; DM_device_count < NUMBER_OF_DEVICES ; DM_device_count++)
    {
      /* Change Dot matrix brightness  */
      lc.setIntensity(DM_device_count,(MIN_INTENSITY + 2));
    }
    Serial.println("set intensity = " + String(MIN_INTENSITY + 2));
  }
  else
  {
    for (int DM_device_count = 0 ; DM_device_count < NUMBER_OF_DEVICES ; DM_device_count++)
    {
      /* Change Dot matrix brightness  */
      lc.setIntensity(DM_device_count,MIN_INTENSITY);
    }
    Serial.println("set intensity = " + String(MIN_INTENSITY));
  }
}
/**********************************************************************//**
 * @brief  control the brightness of DOT Matrix based on light sensor data
 *
 * @param  none
 *
 * @return
 *
 *************************************************************************/
static void update_dot_matrix_intensity()
{
  if( get_lux_value() == EXIT_SUCCESS )
  {
    current_lux = lux;
    if ( current_lux - previous_lux >= 15 )
    {
      set_dot_matrix_intensity();
      previous_lux = current_lux;
      Serial.println("current lux is greater");
    }
    else if ( previous_lux - current_lux >= 15 )
    {
      set_dot_matrix_intensity();
      previous_lux = current_lux;
      Serial.println("previous lux is greater");
    }
  }
}

/**********************************************************************//**
 * @brief  Mount SPIIFS to ESP
 *
 * @param  none
 *
 * @return
 *
 *************************************************************************/
static void mount_spiffs()
{
  if(!SPIFFS.begin(true)) //mount SPIFFS to ESP32
  {
      Serial.println("An Error has occurred while mounting SPIFFS");
      SPIFF_mounted = false;
  }
  else
  {
    SPIFF_mounted = true;
  }

  float totalBytes = (float)SPIFFS.totalBytes()/(1024 * 1024 ); //Get the available SPIFFS space
  float usedBytes = (float)SPIFFS.usedBytes()/(1024 * 1024 ); //Get the used space in SPIFFS

  Serial.println("File sistem info.");

  Serial.print("Total space:      ");
  Serial.print(totalBytes);
  Serial.println("  Mb");

  Serial.print("Total space used: ");
  Serial.print(usedBytes);
  Serial.println("  Mb");

  Serial.print("Total free space: ");
  Serial.print(totalBytes - usedBytes); // Get Free space
  Serial.println("  Mb");

  Serial.println();
}

/**********************************************************************//**
 * @brief  login to FTP server
 *
 * @param  none
 *
 * @return 0 - Failed to login to server
 *         1 - login success
 *
 *************************************************************************/
static bool open_ftp_socket()
{
  if ( connectToFTPServer(FTP_server,ftp_Port) )
  {
    if( loginFTPServer(FTP_USER_NAME,FTP_PASSWORD) )
    {
      Serial.println(">Loged iN");
      sendFTPCommand("NOOP");
      sendFTPCommand("TYPE I");
      sendFTPCommand("TYPE I");
      vTaskDelay(200);
      sendFTPCommand("EPSV");
    }
    else
    {
      Serial.println("Login Failed");
      return false;
    }
  }
  else
  {
    return false;
  }
}
/**********************************************************************//**
 * @brief  download binary image from FTP server and flash to esp32
 *
 * @param  none
 *
 * @return 0 - Failed to login to server
 *         1 - login success
 *
 *************************************************************************/
static void Download_Flash_image()
{
  uint32_t download_image_size = 0;
  if( !parseFTPDataPort() )
  {
    /*
     * Check parshed FTP data port is greater than 0
     * If parshed FTP data port is 0 try to connect to server 3 times
     */
    for (int trial_count = 0;trial_count < FTP_MAX_TRIAL_COUNT;trial_count++)
    {
      if( open_ftp_socket() )
      {
       if( parseFTPDataPort() )
       {
        /*
         * If parshed FTP data port is greater than 0
         * 1. Download image in the path json_extract.fm_path
         * 2. update the downloaded image if File size matched
         */
        downloadFileFromFTP(download_path);
        listDir("/", 0);
        update_firmware(download_path);
//        /*
//         * Check downloaded image size and received image size is approx equal
//         * If matched update the image else delete the image and don't do Update process
//         */
//        download_image_size = Get_File_Size(download_path)/Fac_To_KILO_BYTES;
//        if( (download_image_size -2) <= json_extract.fm_file_size )//have to change.Don't forget
//        {
//          update_firmware(download_path);
//        }
//        else
//        {
//          FS_rmFile(download_path);
//        }
       }
       if( (trial_count == 2) && !parseFTPDataPort() )
       {
          Serial.println("FTP Data port is 0");
       }
       vTaskDelay(100);
      }
    }
  }
  else
  {
    /*
     * If parshed FTP data port is greater than 0
     * 1. Download image in the path json_extract.fm_path
     * 2. update the downloaded image if File size matched
     */
    downloadFileFromFTP(download_path);
    listDir("/", 0);
    update_firmware(download_path);
//    /*
//     * Check downloaded image size and received image size is approx equal
//     * If matched update the image else delete the image and don't do Update process
//     */
//    download_image_size = Get_File_Size(download_path)/Fac_To_KILO_BYTES;
//    if( (download_image_size - 2) <= json_extract.fm_file_size )//have to change.Don't forget
//    {
//      update_firmware(download_path);
//    }
//    else
//    {
//      FS_rmFile(download_path);
//    }
  }
}
/**********************************************************************//**
 * @brief  Get download path and FTP server ip from FTP url
 *
 * @param  none
 *
 * @return
 *
 *************************************************************************/
static void Get_ftp_details()
{
  char* Get_path = NULL;
  FTP_server    = NULL;
  char get_path[100]  = {0};
  char get_ip[100]    = {0};
  memset(download_path,'\0',sizeof(download_path));
//  Serial.printf("download url : %s\n",json_extract.fm_path);
  memcpy (get_path,json_extract.fm_path, strlen(json_extract.fm_path));
  memcpy (get_ip,json_extract.fm_path, strlen(json_extract.fm_path));
  find_char_location((uint8_t*)get_path,strlen(get_path),'/');
  Get_path = delchar(get_path,found_location[BYTE2]-1,BYTE1);
  memcpy (download_path,Get_path,strlen(Get_path));
  Serial.print("Download path :");
  Serial.println(download_path);
  FTP_server = delchar(get_ip,strlen(get_ip) - found_location[BYTE2] +1,found_location[BYTE2]);
  FTP_server = delchar(FTP_server,found_location[BYTE1],BYTE1);
  Serial.print("FTP server IP :");
  Serial.println(FTP_server);
  memset(found_location,'\0',sizeof(found_location));
}
/**********************************************************************//**
 * @brief  make beep sound using buzzer
 *
 * @param  buzzer_drive_count - beep duration
 *
 *         beep_delay         - Buzzer switching speed
 *
 * @return none
 *************************************************************************/
static void beep_buzzer(uint16_t buzzer_drive_count,uint16_t beep_delay)
{
  for(uint16_t drive_count = 0;drive_count < buzzer_drive_count;drive_count++)
  {
     set_Buzzer_ON();
     vTaskDelay(beep_delay);
     set_Buzzer_OFF();
     vTaskDelay(beep_delay);
  }
}
