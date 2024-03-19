/**
 * @file temperature_sensor.h
 * @author Kulasekaran (kulasekaran@qmaxsys.com)
 * @brief This file contains functions and definitions related to Temperature sensor
 * @version 0.1
 * @date 2024-02-29
 * @copyright Copyright (c) 2024
 */

#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include "../Custom/main.h"

/**
 * @brief At hardware the A2, A1 and A0 pins are grounded and as per
 * the datasheet, the slave address should be 0x48
 * @link https://www.ti.com/lit/ds/symlink/tmp1075.pdf <-- Temperature sensor Datasheet link
 */
#define SLAVE_ADDR 0x48

/* GLOBAL VARIABLES */
extern uint8_t measured_temperature;

#ifdef __cplusplus
extern "C"
{
#endif

/* FUNCTION DECLARATIONS */
void *temperature_read(void *args);
void I2C_inital_setup();

#ifdef __cplusplus
}
#endif

#endif

