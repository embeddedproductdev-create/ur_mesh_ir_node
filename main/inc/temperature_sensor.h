#ifndef TEMPERATURE_SENSORS_H
#define TEMPERATURE_SENSOR_H

#define I2C_MASTER_SCL_IO           6      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           5      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define DIGITAL_TEMPERATURE_SENSOR_SLAVE_ADDR 0x48
#define POINTER_REGISTER 0x00  /*!< Register addresses of the temperature sensor*/
#define SLOPE_FOR_DIGI 0.0021
#define INTERCEPT_DIGI 9.9974

/* ANALOG TEMPERATURE SENSOR DEFINITIONS */
#define ANALOG_TEMPERATURE_SENSOR_SAMPLE_COUNT 50
#define VOLTAGE_FAC 0.000824926      /**< Voltage conversion factor */
#define TEMPERATURE_SLOPE -0.0196    /**< Slope for temperature conversion */
#define TEMPERATURE_INTERCEPT 89.362 /**< Intercept for temperature conversion y = -0.0196x + 89.362*/

/*Function declarations*/
void temperature_sensor_init();
int16_t read_analog_temperature_sensor();
int16_t read_digital_temperature_sensor();

#endif