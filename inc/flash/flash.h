/**
 * @file flash.h
 * @author Umamaheswari (uma@qmaxsys.com)
 * @brief This file contains all MACROS and functions related to storing/retrieving
 * data to/from the EEPROM flash on-board
 * @version 0.3
 * @date 2024-04-10
 * @copyright Copyright (c) 2024
 */

#ifndef EEPROM_H
#define EEPROM_H

#include "../Custom/main.h"

#define TEACHING_FAC 0x01
#define EEPROM_WRITE 0x00
#define EEPROM_READ 0x01
#define EEPROM_SLAVE_ADDR 0x50
#define EEPROM_PAGE_SIZE 64

#define MAX_OFFSET 0X0540
#define EEPROM_CONF_FAC 0x0440
#define TEACH_DATA_LEN EEPROM_CONF_FAC + EEPROM_PAGE_SIZE
#define FETCH_ADDR_LOW 15

#define TEACH_DATA_POFF 0X0800
#define TEACH_DATA_PON_16C TEACH_DATA_POFF + MAX_OFFSET
#define TEACH_DATA_PON_17C TEACH_DATA_PON_16C + MAX_OFFSET
#define TEACH_DATA_PON_18C TEACH_DATA_PON_17C + MAX_OFFSET
#define TEACH_DATA_PON_19C TEACH_DATA_PON_18C + MAX_OFFSET
#define TEACH_DATA_PON_20C TEACH_DATA_PON_19C + MAX_OFFSET
#define TEACH_DATA_PON_21C TEACH_DATA_PON_20C + MAX_OFFSET
#define TEACH_DATA_PON_22C TEACH_DATA_PON_21C + MAX_OFFSET
#define TEACH_DATA_PON_23C TEACH_DATA_PON_22C + MAX_OFFSET
#define TEACH_DATA_PON_24C TEACH_DATA_PON_23C + MAX_OFFSET
#define TEACH_DATA_PON_25C TEACH_DATA_PON_24C + MAX_OFFSET
#define TEACH_DATA_PON_26C TEACH_DATA_PON_25C + MAX_OFFSET
#define TEACH_DATA_PON_27C TEACH_DATA_PON_26C + MAX_OFFSET
#define TEACH_DATA_PON_28C TEACH_DATA_PON_27C + MAX_OFFSET
#define TEACH_DATA_PON_29C TEACH_DATA_PON_28C + MAX_OFFSET
#define TEACH_DATA_PON_30C TEACH_DATA_PON_29C + MAX_OFFSET
#define TEACH_DATA_PON_31C TEACH_DATA_PON_30C + MAX_OFFSET
#define TEACH_DATA_PON_32C TEACH_DATA_PON_31C + MAX_OFFSET
#define NEXT_ADDR TEACH_DATA_PON_32C + MAX_OFFSET

/* GLOBAL VARIABLES */
extern bool storing_IR_data_to_flash;

#ifdef __cplusplus
extern "C" {
#endif
esp_err_t eeprom_write_byte(uint8_t deviceaddress, uint16_t eeaddress, uint8_t byte);
esp_err_t eeprom_write(uint8_t deviceaddress, uint16_t eeaddress, uint8_t* data, size_t size);

uint8_t eeprom_read_byte(uint8_t deviceaddress, uint16_t eeaddress);
esp_err_t eeprom_read(uint8_t deviceaddress, uint16_t eeaddress, uint8_t* data, size_t size);

void eeprom_task();

uint16_t convert_8bit_to_16bit(uint8_t *convert_buffer);
void convert_16bit_to_8bit(uint16_t value,uint8_t *converted_data);
void convert_16_to_8(volatile uint16_t *input_array, size_t input_size, uint8_t *output_array);
int write_to_memory(volatile uint16_t *input_array, size_t input_size,uint16_t eeprom_address);
void convert_8_to_16(volatile uint8_t *input_array, size_t input_size,volatile uint16_t *output_array);
int read_from_memory(volatile uint16_t* output_array,size_t input_size,uint16_t eeprom_address);

#ifdef __cplusplus
}
#endif

#endif