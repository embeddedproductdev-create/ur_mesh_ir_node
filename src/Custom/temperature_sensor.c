/**
 * @file read_temperature.c
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions related to the I2C read of the temperature sensor
 * @version 0.1
 * @date 2024-02-28
 * @link https://www.ti.com/lit/ds/symlink/tmp1075.pdf <-- Temperature sensor Datasheet link
 * @copyright Copyright (c) 2024
 *
 */

#include "../../inc/Custom/temperature_sensor.h"

// Initialization
uint8_t measured_temperature = 0;
uint32_t TempDataFreqSec = 10;

const esp_timer_create_args_t periodic_timer_args = {
    .callback = &publish_temperature_cb,
    .name = "heartbeat_timer"};
esp_timer_handle_t temp_publish_timer;

esp_err_t initialize_i2c(void)
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
 * @brief Read a sequence of bytes from the I2C device.
 * @param reg_addr The register address to start reading from.
 * @param data Pointer to a buffer where read data will be stored.
 * @param len Length of the data to be read.
 * @return 0 on success, non-zero on failure.
 */
esp_err_t i2c_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    i2c_master_write_read_device(I2C_MASTER_NUM, SENSOR_SLAVE_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / 10);
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
    ret = i2c_master_write_to_device(I2C_MASTER_NUM, SENSOR_SLAVE_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / 10);
    return ret;
}

/**
 * @brief Function that gets temperature from Digital temperature sensor
 * on-board by performing I2C communications
 * @param none
 * @retval none
 */
void read_digital_temperature(uint8_t *measured_temperature)
{
    uint8_t data[2];
    uint16_t temp_val;
    float temperature_val;
    i2c_read(POINTER_REGISTER, data, sizeof(data));
    temp_val = ((data[0] << 8) | (data[1]));
    temperature_val = (temp_val * SLOPE_FOR_DIGI) + INTERCEPT_DIGI;
    *measured_temperature = (uint8_t)temperature_val;
}

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
        adc2_get_raw(ADC2_CHANNEL_0, ADC_WIDTH_BIT_12, &data_read); // Read ADC value from channel 0
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
void read_analog_temperature(uint8_t *measured_temperature)
{
    int adc_data = 0;
    float voltage = 0;
    int temperature;
    read_adc(&adc_data);
    //(146.222969-(45.46104901*voltage));
    temperature = (((TEMPERATURE_SLOPE * adc_data)) + (TEMPERATURE_INTERCEPT));
    *measured_temperature = (uint8_t)temperature;
}

/**
 * @brief Function that takes care of setting up
 * of Analog Temperature sensor
 * @param none
 * @retval none
 */
void init_analog_temperature_sensor()
{
    // Set attenuation to 11dB for maximum voltage range
    adc2_config_channel_atten(ADC2_CHANNEL_0, ADC_ATTEN_DB_11);
}

/**
 * @brief Function that takes care of setting up
 * of Digital Temperature sensor
 * @param none
 * @retval none
 */
void init_digital_temperature_sensor()
{
    i2c_write(POINTER_REGISTER);
}

static void publish_temperature_cb(void *arg)
{
    char pubmessage[PUBMESG_LEN];
    read_analog_temperature(&measured_temperature);
#if (IS_GWY)
    if (registered && mqtt_connected)
    {
        if (LOG_DATA)
        {
            sprintf(temperature_log_buffer, "Sending Gwy Temperature Ack");
            magenta_printf(TEMPERATURE_DEBUG_TAG, temperature_log_buffer);
        }
        sprintf(pubmessage, "{\"%s\" : %d, \"%s\" : \"%s\", \"%s\" : \"%s\", \"%s\" : %d,  \"%s\" : \"%s\", \"%s\" : %d,  \"%s\" : %d,  \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d, \"%s\" : %d}",
                JSON_PACKET_ID_KEY, GWY_HEARTBEAT_ACK,
                JSON_ACK_NAME_KEY, GWY_HEARTBEAT_ACK_NAME,
                GWY_SER_NO_KEY, GWY_SER_NO_IN_STRING,
                POWER_KEY, gwy_ac_control_t.control.power,
                MODE_KEY, gwy_ac_control_t.control.mode_str,
                FAN_SPEED_KEY, gwy_ac_control_t.control.fan,
                TEMPERATURE_KEY, gwy_ac_control_t.control.temp,
                AMBIENT_TEMPERATURE_DATA_KEY, measured_temperature,
                SWING_H_KEY, gwy_ac_control_t.control.swingH,
                SWING_V_KEY, gwy_ac_control_t.control.swingV,
                ONTIMER_KEY, gwy_ac_control_t.control.OnTimer,
                OFFTIMER_KEY, gwy_ac_control_t.control.OffTimer,
                AC_LOCKING_KEY, gwy_ac_control_t.control.Locking,
                TEMP_LOCK_UP_LIMIT_KEY, gwy_ac_control_t.control.TempLockUpLimit,
                TEMP_LOCK_LOW_LIMIT_KEY, gwy_ac_control_t.control.TempLockUpLimit);
        add_to_pubmesg_queue(pubmessage, publish_topic);
    }
#endif

#if (!IS_GWY)
    if (provisioned)
        send_heartbeat_ack_to_gwy();
#endif
}

/**
 * @brief Function that initalizes the Analog and Digital temperature sensor
 * at board startup. This also takes care of setting up a repeated timer that takes
 * care of publishing the measured temperature data to the cloud
 * @param none
 * @retval none
 */
void init_temperature_sensor()
{
    init_digital_temperature_sensor();
    init_analog_temperature_sensor();
    create_Temperature_data_publish_timer();
}

/**
 * @brief Function that deletes the already configured repeated timer
 * whenever we receive a GWY_PUB_CONF packet
 * @param none
 * @retval none
 */
void delete_Temperature_data_publish_timer()
{
    ESP_ERROR_CHECK(esp_timer_stop(temp_publish_timer));
    ESP_ERROR_CHECK(esp_timer_delete(temp_publish_timer));
}

/**
 * @brief Function that creates a repeating timer that measures temperature data with
 * on-board temperature sensor and publishes it as per desired frequency to the cloud
 * @param none
 * @return none
 */
void create_Temperature_data_publish_timer()
{
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &temp_publish_timer));
#if (IS_GWY)
    ESP_ERROR_CHECK(esp_timer_start_periodic(temp_publish_timer, gwy_pub_conf_t.pub_conf_period_in_sec * 1000000));
#endif
#if (!IS_GWY)
    ESP_ERROR_CHECK(esp_timer_start_periodic(temp_publish_timer, node_heartbeat_pub_conf_t.pub_conf_period_in_sec * 1000000));
#endif
}