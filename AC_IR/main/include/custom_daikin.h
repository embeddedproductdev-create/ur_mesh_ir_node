#ifndef MAIN_INCLUDE_CUSTOM_DAIKIN_H_
#define MAIN_INCLUDE_CUSTOM_DAIKIN_H_

#include <custom_irsend.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define DaikinAuto		0
#define DaikinDry		2
#define DaikinCool		3
#define DaikinHeat		4
#define DaikinFan		6

#define DaikinFanMin 	1
#define DaikinFanMed 	3
#define DaikinFanMax 	5
#define DaikinFanAuto 	10
#define DaikinFanQuiet	11

/*Function Declarations*/

void setTemp_Daikin280(const uint8_t temp);
void setPower_Daikin280(const bool on);
void setBit_Daikin280(uint8_t * const data, const uint8_t position, const bool on);
void setFan_Daikin280(const uint8_t fan);
void setBits_Daikin280(uint8_t * const dst, const uint8_t offset, const uint8_t nbits, const uint8_t data);
void setMode_Daikin280(const uint8_t mode);
void setSwingVertical_Daikin280(const bool on);
void setSwingHorizontal_Daikin280(const bool on);
void setQuiet_Daikin280(const bool on);
void setPowerful_Daikin280(const bool on);
void setEcono_Daikin280(const bool on);
void setSensor_Daikin280(const bool on);
void setMold_Daikin280(const bool on);
void setComfort_Daikin280(const bool on);
void enableOnTimer_Daikin280(const uint16_t starttime);
void disableOnTimer_Daikin280(void);
void daikin_280_checksum();
void data_init_Daikin280();
uint8_t daikin280_sumBytes(const uint8_t * const start, const uint16_t length);
void toggle_daikin();

extern uint8_t data[35];

#endif /* MAIN_INCLUDE_CUSTOM_DAIKIN_H_ */
