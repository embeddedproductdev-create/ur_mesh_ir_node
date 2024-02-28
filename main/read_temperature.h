#ifdef __cplusplus
extern "C"
{
#endif

#ifndef READ_TEMPERATURE_H
#define READ_TEMPERATURE_H

#include "main.h"
// #include "Wire.h"

/**
 * @brief At hardware the A2, A1 and A0 pins are grounded and as per
 * the datasheet, the slave address should be 0x48
 * @link https://www.ti.com/lit/ds/symlink/tmp1075.pdf <-- Temperature sensor Datasheet link
 */
#define SLAVE_ADDR 0x48

/* FUNCTION DECLARATIONS */
void *temperature_read(void *args);
void I2C_inital_setup();


#endif

#ifdef __cplusplus
}
#endif