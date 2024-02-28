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

#include "main.h"

/**
 * @brief Function that performs initial I2C setup
 * @param none
 * @retval none
 */
void I2C_inital_setup()
{
//   Wire.begin(); // Initiate the Wire library
//   delay(100);
    ;
}

/**
 * @brief Thread that performs the I2C temperature read communications
 * @param args
 * @return void*
 */
void *temperature_read(void *args)
{
    // uint8_t data_hi = 0, data_lo = 0;
    // uint16_t temperature_value = 0;
    // float temperature_in_float = 0.0;
    // I2C_inital_setup();
    while(1)
    {
    //     Wire.beginTransmission(SLAVE_ADDR); // Begin transmission to the Sensor
    //     //Ask the particular registers for data
    //     Wire.write(0X00); //Selecting the Temperature register
    //     Wire.endTransmission(); // Ends the transmission and transmits the data from the two registers

    //     Wire.requestFrom(SLAVE_ADDR,2); // Request the transmitted two bytes from the two registers

    //     if(Wire.available()<=2) {  //
    //         data_hi = Wire.read(); // Reads the data from the register
    //         data_lo = Wire.read();
    //     }
    //     printf("Data_hi = %x", data_hi);
    //     printf("Data_lo = %x", data_lo);

    //     temperature_value |= data_hi;
    //     temperature_value <<= 8;
    //     temperature_value |= data_lo;
    //     printf("Temperature value in int : %d", temperature_value);
    //     temperature_in_float = *(float *)&temperature_value;
    //     printf("Temperature value in float : %f", temperature_in_float);
    vTaskDelay((1));
    ;
    }
}