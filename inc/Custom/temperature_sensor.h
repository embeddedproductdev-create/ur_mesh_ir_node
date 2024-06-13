/**
 * @file temperature_sensor.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions and definitions related to Temperature sensor
 * @version 0.1
 * @date 2024-02-29
 * @link https://www.ti.com/lit/ds/symlink/tmp1075.pdf <-- Digital Temperature sensor Datasheet link
 * @copyright Copyright (c) 2024
 */

#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include "../Custom/main.h"

/* DIGITAL TEMPERATURE SENSOR DEFINITIONS */

/**
 * @brief At hardware the A2, A1 and A0 pins are grounded and as per
 * the datasheet, the slave address should be 0x48
 */
#define DIGITAL_TEMPERATURE_SENSOR_SLAVE_ADDR 0x48

#define I2C_MASTER_SCL_IO 6         /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO 5         /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM 1            /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 1000000  /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 2 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 2 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

#define SENSOR_SLAVE_ADDR 0x48 /*!< Slave address of the MPU9250 sensor */
#define POINTER_REGISTER 0x00  /*!< Register addresses of the temperature sensor*/
#define SLOPE_FOR_DIGI 0.0021
#define INTERCEPT_DIGI 9.9974 // y = 0.0021x + 9.9974

/* ANALOG TEMPERATURE SENSOR DEFINITIONS */
#define ANALOG_TEMPERATURE_SENSOR_SAMPLE_COUNT 50
#define VOLTAGE_FAC 0.000824926      /**< Voltage conversion factor */
#define TEMPERATURE_SLOPE -0.0196    /**< Slope for temperature conversion */
#define TEMPERATURE_INTERCEPT 89.362 /**< Intercept for temperature conversion y = -0.0196x + 89.362*/

#define DEFAULT_TEMPERATURE_DATA_PUBLISH_PERIOD_SEC 5

/* GLOBAL VARIABLES */
extern uint8_t measured_temperature;

#ifdef __cplusplus
extern "C"
{
#endif

/* COMMON FUNCTION DECLARATIONS */
void create_Temperature_data_publish_timer();
void delete_Temperature_data_publish_timer();
void temperature_sensing_init();

/* ANALOG TEMPERATURE SENSOR FUNCTION DECLARATIONS */
void init_analog_temperature_sensor();
void initialize_adc(void);
void read_adc(int *rawdata);
void Read_Analog_Temperature(uint8_t *measured_temperature);

/* DIGITAL TEMPERATURE SENSOR FUNCTION DECLARATIONS */
void init_digital_temperature_sensor();
esp_err_t initialize_i2c(void);
void initialize_temp_sensor(void);
esp_err_t i2c_read(uint8_t reg_addr, uint8_t *data, size_t len);
esp_err_t i2c_write(uint8_t reg_addr);
void Read_Digital_Temperature(uint8_t *measured_temperature);
void init_temperature_sensor();
void create_Temperature_data_publish_timer();
static void publish_temperature_cb(void *arg);
#ifdef __cplusplus
}
#endif

#endif
