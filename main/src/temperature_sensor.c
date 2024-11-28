#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include <temperature_sensor.h>
#include "driver/adc.h"
#include "driver/adc_i2s_legacy.h"
#include "driver/adc_types_legacy.h"

#define TEMP_TAG "TEMP"

/**
 * @brief Function to read RAW ADC values from channel 2 and average them.
 * @param rawdata Pointer to store the averaged raw ADC value.
 */
void read_adc(int *rawdata)
{
    double calibrate = 0;
    int data_read = 0;
    for (uint8_t count = 0; count < ANALOG_TEMPERATURE_SENSOR_SAMPLE_COUNT; count++)
    {
        // Read ADC value from channel 0
        adc2_get_raw(ADC2_CHANNEL_0, ADC_WIDTH_BIT_12, &data_read); 
        calibrate += data_read;
    }
    *rawdata = calibrate / ANALOG_TEMPERATURE_SENSOR_SAMPLE_COUNT;
}


/**
 * @brief Function that gets temperature from Analog temperature sensor
 * on-board by performing communications with ESPs internal ADC
 * @param none
 * @retval none
 */
int16_t read_analog_temperature_sensor()
{
    int adc_data;
    read_adc(&adc_data);
    return (((TEMPERATURE_SLOPE * adc_data)) + (TEMPERATURE_INTERCEPT));
}

/**
 * @brief Read a sequence of bytes from the I2C device.
 * @param reg_addr The register address to start reading from.
 * @param data Pointer to a buffer where read data will be stored.
 * @param len Length of the data to be read.
 * @return 0 on success, non-zero on failure.
 */
esp_err_t i2c_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    i2c_master_write_read_device(I2C_MASTER_NUM, DIGITAL_TEMPERATURE_SENSOR_SLAVE_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / 10);
    return 0;
}

/**
 * @brief Function to write a byte to the I2C device.
 * @param reg_addr The register address to write to.
 * @retval Zero on success, NonZero on Failure
 */
esp_err_t i2c_write(uint8_t reg_addr)
{
    int ret;
    uint8_t write_buf[1] = {reg_addr};
    ret = i2c_master_write_to_device(I2C_MASTER_NUM, DIGITAL_TEMPERATURE_SENSOR_SLAVE_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / 10);
    return ret;
}

/**
 * @brief Function that gets temperature from Digital temperature sensor
 * on-board by performing I2C communications
 * @param none
 * @retval none
 */
int16_t read_digital_temperature_sensor()
{
    uint8_t data[2];
    uint16_t temp_val;
    i2c_read(POINTER_REGISTER, data, sizeof(data));
    temp_val = ((data[0] << 8) | (data[1]));
    return (temp_val * SLOPE_FOR_DIGI) + INTERCEPT_DIGI;
    
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

/**
 * @brief Function that initializes the temperature sensor
 */
void temperature_sensor_init(void)
{
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TEMP_TAG, "I2C initialized successfully");
    /*Setting attenuation to 11dB for maximum voltage range*/
    adc2_config_channel_atten(ADC2_CHANNEL_0, ADC_ATTEN_DB_11);
    /*Initializing Digital Temperature Sensor*/
    i2c_write(POINTER_REGISTER);
}
