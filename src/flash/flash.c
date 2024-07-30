/**
 * @file flash.c
 * @author Umamaheswari (uma@qmaxsys.com)
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains all functions related to Storing/Retrieving contents to/from
 * EEPROM flash on-board.
 * @version 0.8.7
 * @date 2024-07-02
 * @copyright Copyright (c) 2024
 *
 */
#include "../../inc/flash/flash.h"

void get_new_serial_no()
{
    uint32_t serial_no = 0;
    char input[10];
    while (serial_no == 0)
    {
        vTaskDelay(pdMS_TO_TICKS(5));
        ESP_LOGI(MAIN_DEBUG_TAG, "Enter the Serial Number : ");
        if (fgets(input, sizeof(input), stdin) != NULL)
        {
            serial_no = atoi(input);
            ESP_LOGI(MAIN_DEBUG_TAG, "input : %s | serial_no : %ld", input, serial_no);
            if (serial_no == 0)
                ESP_LOGE(MAIN_ERROR_TAG, "Invalid Serial Number value entered");
        }
    }
    eeprom_write_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_HI, serial_no >> 16);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_MID, serial_no >> 8);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, SER_NO_IN_FLASH_ADDR_LO, serial_no);
}

/**
 * @brief Function that takes care of erasing the data in device and set it up as factory device
 * Maybe we can implement some security check before going on to clear data. Also, resetting Device
 * for now doesn't erase the MQTT settings. Need to discuss on this later.
 * @param none
 * @retval none
 */
void factory_reset_device()
{
    ESP_LOGI(MAIN_DEBUG_TAG, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= FACTORY RESETTING DEVICE =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");

    registered = false;
    provisioned = false;
    configured = false;
    protocol_selected_num = -1;

#if (IS_GWY)
    // Registered
    eeprom_write_byte(EEPROM_SLAVE_ADDR, REGISTERED_FLAG_FLASH_ADDR, 0);
#endif

    // Configured
    eeprom_write_byte(EEPROM_SLAVE_ADDR, CONFIGURED_FLAG_FLASH_ADDR, 0);

    // Protocol Selected Number
    eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR_HI, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, PROTOCOL_SEL_FLASH_ADDR_LO, 0);

    eeprom_write_byte(EEPROM_SLAVE_ADDR, RAWLEN_ADDR_HI, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, RAWLEN_ADDR_LO, 0);

    // Publish Period
    eeprom_write_byte(EEPROM_SLAVE_ADDR, HB_PUB_CONF_PERIOD_ADDR_LO, DEFAULT_HEARTBEAT_PUB_CONF_PERIOD_SEC);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, HB_PUB_CONF_PERIOD_ADDR_HI, DEFAULT_HEARTBEAT_PUB_CONF_PERIOD_SEC>>8);
    gwy_heartbeat_pub_conf_t.pub_conf_period_in_sec = DEFAULT_HEARTBEAT_PUB_CONF_PERIOD_SEC;
    node_heartbeat_pub_conf_t.pub_conf_period_in_sec = DEFAULT_HEARTBEAT_PUB_CONF_PERIOD_SEC;

    // AC Control Settings
    eeprom_write_byte(EEPROM_SLAVE_ADDR, POWER_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, MODE_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, FAN_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, TEMPERATURE_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, SWINGH_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, SWINGV_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, LOCKING_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, TEMPLOCKUPLIMIT_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, TEMPLOCKLOWLIMIT_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, ONTIMER_FLASH_ADDR, 0);
    eeprom_write_byte(EEPROM_SLAVE_ADDR, OFFTIMER_FLASH_ADDR, 0);

#if (!IS_GWY)
    unprovision_node();
#endif

    delete_Temperature_data_publish_timer();
}

/**
 * @brief Writes a single byte to the EEPROM.
 * @param deviceaddress The I2C address of the EEPROM.
 * @param eeaddress The address in the EEPROM to write to.
 * @param byte The byte to write.
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t eeprom_write_byte(uint8_t deviceaddress, uint16_t eeaddress, uint8_t byte)
{
    esp_err_t ret = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (deviceaddress << 1) | EEPROM_WRITE, 1);
    i2c_master_write_byte(cmd, eeaddress >> 8, 1);
    i2c_master_write_byte(cmd, eeaddress & 0xFF, 1);
    i2c_master_write_byte(cmd, byte, 1);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(pdMS_TO_TICKS(5));
    return ret;
}
/**
 * @brief Writes array of data to the EEPROM.
 * @param deviceaddress The I2C address of the EEPROM.
 * @param eeaddress The starting address in the EEPROM to write to.
 * @param data Pointer to the data array to write.
 * @param size Number of bytes to write.
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t eeprom_write(uint8_t deviceaddress, uint16_t eeaddress, uint8_t *data, size_t size)
{
    int bytes_remaining = size;
    int current_address = eeaddress;
    int first_write_size = 0;
    esp_err_t ret = ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (deviceaddress << 1) | EEPROM_WRITE, 1);
    i2c_master_write_byte(cmd, eeaddress >> 8, 1);
    i2c_master_write_byte(cmd, eeaddress & 0xFF, 1);
    // printf("first_write_size:%d\n",first_write_size);
    if (bytes_remaining > EEPROM_PAGE_SIZE)
        first_write_size = EEPROM_PAGE_SIZE;
    if (bytes_remaining <= first_write_size)
    {
        i2c_master_write(cmd, data, bytes_remaining, 1);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
    }
    else
    {
        i2c_master_write(cmd, data, first_write_size, 1);
        bytes_remaining -= first_write_size;
        current_address += first_write_size;
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
        if (ret != ESP_OK)
            return ret;
        while (bytes_remaining > 0)
        {
            vTaskDelay(20 / portTICK_PERIOD_MS);
            cmd = i2c_cmd_link_create();
            // 2ms delay period to allow EEPROM to write the page
            // buffer to memory.
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (deviceaddress << 1) | EEPROM_WRITE, 1);
            i2c_master_write_byte(cmd, current_address >> 8, 1);
            i2c_master_write_byte(cmd, current_address & 0xFF, 1);
            if (bytes_remaining <= EEPROM_PAGE_SIZE)
            {
                i2c_master_write(cmd, data + (size - bytes_remaining), bytes_remaining, 1);
                bytes_remaining = 0;
            }
            else
            {
                i2c_master_write(cmd, data + (size - bytes_remaining), EEPROM_PAGE_SIZE, 1);
                bytes_remaining -= EEPROM_PAGE_SIZE;
                current_address += EEPROM_PAGE_SIZE;
            }
            i2c_master_stop(cmd);
            ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_PERIOD_MS);
            i2c_cmd_link_delete(cmd);
            if (ret != ESP_OK)
                return ret;
        }
    }
    return ret;
}

/**
 * @brief Reads a single byte from the EEPROM.
 * @param deviceaddress The I2C address of the EEPROM.
 * @param eeaddress The address in the EEPROM to read from.
 * @return The byte read from the EEPROM.
 */
uint8_t eeprom_read_byte(uint8_t deviceaddress, uint16_t eeaddress)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (deviceaddress << 1) | EEPROM_WRITE, 1);
    i2c_master_write_byte(cmd, eeaddress >> 8, 1);
    i2c_master_write_byte(cmd, eeaddress & 0xFF, 1);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (deviceaddress << 1) | EEPROM_READ, 1);

    uint8_t data;
    i2c_master_read_byte(cmd, &data, 1);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return data;
}
/**
 * @brief Reads array data from the EEPROM.
 * @param deviceaddress The I2C address of the EEPROM.
 * @param eeaddress The starting address in the EEPROM to read from.
 * @param data Pointer to the data array to store read data.
 * @param size Number of bytes to read.
 * @return ESP_OK on success, error code otherwise.
 */
esp_err_t eeprom_read(uint8_t deviceaddress, uint16_t eeaddress, uint8_t *data, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (deviceaddress << 1) | EEPROM_WRITE, 1);
    i2c_master_write_byte(cmd, eeaddress >> 8, 1);
    i2c_master_write_byte(cmd, eeaddress & 0xFF, 1);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (deviceaddress << 1) | EEPROM_READ, 1);

    if (size > 1)
    {
        i2c_master_read(cmd, data, size - 1, 0);
    }
    i2c_master_read_byte(cmd, data + size - 1, 1);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

/**
 * @brief Task to demonstrate EEPROM operations just for testing purpose.
 */
void eeprom_task()
{
    const uint8_t eeprom_address = 0x50;
    const uint16_t starting_address = 0x0400;
    uint8_t random_read_byte = 0;
    // EEPROM single byte write example
    uint8_t single_write_byte = 0x27;
    eeprom_write_byte(eeprom_address, starting_address, single_write_byte);
    printf("Wrote byte 0x%02X to address 0x%04X\n", single_write_byte, starting_address);
    // EEPROM random read example
    vTaskDelay(20 / portTICK_PERIOD_MS);
    random_read_byte = eeprom_read_byte(eeprom_address, starting_address);
    printf("Read byte 0x%02X at address 0x%04X\n", random_read_byte, starting_address);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    // EEPROM page write example
    uint8_t character_arary[5] = {198, 34, 43, 5, 5};
    eeprom_write(eeprom_address, starting_address, character_arary, 5);
    printf("Wrote the following string to EEPROM: %d %d\n", character_arary[0], character_arary[1]);

    vTaskDelay(20 / portTICK_PERIOD_MS);

    // EEPROM sequential read example and error checking
    uint8_t received_array[5];
    esp_err_t ret = eeprom_read(eeprom_address, starting_address, received_array, 5);

    if (ret == ESP_ERR_TIMEOUT)
        printf("I2C timeout...\n");
    if (ret == ESP_OK)
        printf("The read operation was successful!\n");
    else
        printf("The read operation was not successful, no ACK recieved.  Is the device connected properly?\n");

    printf("Read the following string from EEPROM: %d %d \n", received_array[0], received_array[1]);
}
/**
 * @brief Converts a 16-bit value to two 8-bit values.
 * @param value The 16-bit value to convert.
 * @param converted_data Array to store the converted 8-bit values.
 */
void convert_16bit_to_8bit(uint16_t value, uint8_t *converted_data)
{
    // Extract MSB and LSB
    converted_data[0] = (uint8_t)(value & 0xFF);
    converted_data[1] = (uint8_t)(value >> 0x08);
}
/**
 * @brief Converts two 8-bit values to a 16-bit value.
 * @param convert_buffer Array containing two 8-bit values.
 * @return The 16-bit value.
 */
uint16_t convert_8bit_to_16bit(uint8_t *convert_buffer)
{
    // Combine MSB and LSB into a 16-bit value
    return ((uint16_t)convert_buffer[1] << 8) | convert_buffer[0];
}
/**
 * @brief Converts an array of 16-bit values to an array of 8-bit values.
 * @param input_array Array of 16-bit values.
 * @param input_size Size of the input array.
 * @param output_array Array to store converted 8-bit values.
 */
void convert_16_to_8(volatile uint16_t *input_array, size_t input_size, uint8_t *output_array)
{
    for (size_t i = 1; i <= input_size; i++)
    {
        // Convert the 16-bit value to two separate 8-bit values
        input_array[i] = input_array[i] * 2;
        uint8_t first_8_bits = input_array[i] & 0xFF;         // Get the first 8 bits lsb
        uint8_t second_8_bits = (input_array[i] >> 8) & 0xFF; // Get the next 8 bits

        // Store the 8-bit values in the output array
        output_array[((i - 1) * 2)] = first_8_bits;
        output_array[((i - 1) * 2) + 1] = second_8_bits;
    }
}
/**
 * @brief Writes data to EEPROM after converting it to 8-bit format.
 * @param input_array Array of 16-bit values to write.
 * @param input_size Size of the input array.
 * @param eeprom_address Starting address in EEPROM to write to.
 * @return 0 on success, non-zero error code otherwise.
 * @note here I ignore buffer0 because of IR receiver will have 1 as their initial value to indicate data reception
 * actual data only come from buffer1
 */
int write_to_memory(volatile uint16_t *input_array, size_t input_size, uint16_t eeprom_address)
{
    // Allocate memory for the 8-bit array
    uint8_t *bit8_array = (uint8_t *)malloc(input_size * 2 * sizeof(uint8_t));
    if (bit8_array == NULL)
    {
        printf("Memory allocation failed");
        return 1;
    }
#if (IR_RECV_LOG_ENABLED)
    printf("\n16 bit data :\n");
    for (size_t i = 1; i <= input_size; i++)
    {
        printf("0x%X ", input_array[i] * 2);
    }
#endif
    // Call the function to perform the conversion
    convert_16_to_8(input_array, input_size, bit8_array);
#if (IR_RECV_LOG_ENABLED)
    // Print the 8-bit array
    printf("\n8 bit array :\n");
    for (size_t i = 0; i <= input_size * 2; i++)
    {
        printf("0x%X ", bit8_array[i]);
    }
#endif
    eeprom_write(EEPROM_SLAVE_ADDR, eeprom_address, bit8_array, input_size * 2);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    // Free the dynamically allocated memory
    free(bit8_array);
    return 0;
}
/**
 * @brief Converts an array of 8-bit values to an array of 16-bit values.
 * @param input_array Array of 8-bit values.
 * @param input_size Size of the input array.
 * @param output_array Array to store converted 16-bit values.
 */
void convert_8_to_16(volatile uint8_t *input_array, size_t input_size, volatile uint16_t *output_array)
{
    for (size_t i = 0; i <= (input_size / 2); i++)
    {
        // Combine two 8-bit values into a 16-bit value
        uint16_t value = ((uint16_t)input_array[i * 2 + 1] << 8) | (uint16_t)input_array[i * 2];
        // Store the 16-bit value in the output array
        output_array[i] = value;
    }
}
/**
 * @brief Reads data from EEPROM and converts it to 16-bit format.
 * @param output_array Array to store read 16-bit values.
 * @param input_size Size of the output array.
 * @param eeprom_address Starting address in EEPROM to read from.
 * @return 0 on success, non-zero error code otherwise.
 */
int read_from_memory(volatile uint16_t *output_array, size_t input_size, uint16_t eeprom_address)
{
    // Allocate memory for the 8-bit array
    uint8_t *bit8_array = (uint8_t *)malloc(input_size * sizeof(uint8_t));
    if (bit8_array == NULL)
    {
        printf("Memory allocation failed");
        return 1;
    }
    eeprom_read(EEPROM_SLAVE_ADDR, eeprom_address, bit8_array, input_size);
// Call the function to perform the conversion
#if (IR_RECV_LOG_ENABLED)
    printf("\n8 bit array :\n");
    for (size_t i = 0; i <= input_size; i++)
    {
        printf("0x%X ", bit8_array[i]);
    }
#endif
    convert_8_to_16(bit8_array, input_size, output_array);
#if (IR_RECV_LOG_ENABLED)
    // Print the 16-bit array
    printf("\n16 bit array :\n");
    for (size_t i = 0; i <= (input_size / 2); i++)
    {
        printf("0x%X ", output_array[i]);
    }
#endif
    // Free the dynamically allocated memory
    free(bit8_array);
    return 0;
}
