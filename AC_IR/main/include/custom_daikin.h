/*
 * custom_daikin.h
 *
 *  Created on: 21-Dec-2023
 *      Author: EmbeddedDevelopment
 */

#ifndef MAIN_INCLUDE_CUSTOM_DAIKIN_H_
#define MAIN_INCLUDE_CUSTOM_DAIKIN_H_

#include <custom_irsend.h>

/*Function Declarations*/

void setTemp(const uint8_t temp);
void setPower(const bool on);
void setBit(uint8_t * const data, const uint8_t position, const bool on);
void setFan(const uint8_t fan);
void setBits(uint8_t * const dst, const uint8_t offset, const uint8_t nbits, const uint8_t data);
void setMode(const uint8_t mode);
void setSwingVertical(const bool on);
void setSwingHorizontal(const bool on);
void setQuiet(const bool on);
void setPowerful(const bool on);
void setEcono(const bool on);
void setSensor(const bool on);
void setMold(const bool on);
void setComfort(const bool on);
void enableOnTimer(const uint16_t starttime);
void disableOnTimer(void);
void daikin_280_checksum();
uint8_t daikin280_sumBytes(const uint8_t * const start, const uint16_t length);

#endif /* MAIN_INCLUDE_CUSTOM_DAIKIN_H_ */
