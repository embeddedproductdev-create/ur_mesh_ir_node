#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "include/custom_hitachi.h"
#include "main.h"

/*HITACHI296 GLOBAL VARIABLE INITIALIZATION*/
uint8_t curr_temp_Hitachi296 = 25;
uint8_t curr_power_Hitachi296 = false;
uint8_t curr_mode_Hitachi296 = kHitachiAc296Cool;
uint8_t curr_fan_Hitachi296 = kHitachiAc296FanSilent;
HitachiAC296_t data_Hitachi296;

void send_Hitachi296(){
	sendGeneric(kHitachiAcHdrMark, kHitachiAcHdrSpace, kHitachiAcBitMark,
	              kHitachiAcOneSpace, kHitachiAcBitMark, kHitachiAcZeroSpace,
	              kHitachiAcBitMark, kHitachiAcMinGap, data_Hitachi296.raw, HITACHI296_NBYTES, 38, false,
	              0, 50);
}

void data_init_Hitachi296()
{
	for(uint8_t i = 0; i < 37; i++)
		data_Hitachi296.raw[i] = 0x00;
	data_Hitachi296.raw[0] = 0x01;
	data_Hitachi296.raw[1] = 0x10;
	data_Hitachi296.raw[2] = 0x00;
	  // Every next byte is a parity byte
	data_Hitachi296.raw[3] = 0x40;
	data_Hitachi296.raw[5] = 0xFF;
	data_Hitachi296.raw[7] = 0xCC;
	data_Hitachi296.raw[9] = 0x92;
	data_Hitachi296.raw[11] = 0x43;
	  // 13-14 is Temperature and parity
	data_Hitachi296.raw[15] = 0x00;
	data_Hitachi296.raw[17] = 0x00;  // Off timer LSB
	data_Hitachi296.raw[19] = 0x00;  // Off timer cont
	data_Hitachi296.raw[21] = 0x00;  // On timer LSB
	data_Hitachi296.raw[23] = 0x00;  // On timer cont
	  // 25-26 is Mode and fan
	data_Hitachi296.raw[27] = 0xF1;  // Power on
	data_Hitachi296.raw[29] = 0x00;
	data_Hitachi296.raw[31] = 0x00;
	data_Hitachi296.raw[33] = 0x00;
	data_Hitachi296.raw[35] = 0x03;  // Humidity

	setInvertedStates_Hitachi296();
}

void setSwingV_Hitachi296(const bool on)
{
	;
}

void setSwingH_Hitachi296(const bool on)
{
	;
}

void setPower_Hitachi296(const bool on)
{
	data_Hitachi296.Power = on;
}

void setTemp_Hitachi296(uint8_t temp) {
	temp = min(temp, kHitachiAc296MaxTemp);
	temp = max(temp, kHitachiAc296MinTemp);
	data_Hitachi296.Power = temp;
}

void setMode_Hitachi296(const uint8_t mode)
{
	switch (mode) {
	    case kHitachiAc296Heat:
	    case kHitachiAc296Cool:
	    case kHitachiAc296Dehumidify:
	    case kHitachiAc296AutoDehumidifying:
	    case kHitachiAc296Auto:
	    	data_Hitachi296.Mode = mode;
	    	setTemp_Hitachi296(data_Hitachi296.Temp);
	    	break;
	    default:
	      setMode_Hitachi296(kHitachiAc296Auto);
	      printf("default case in setMode_Hitachi296\r\n");
	  }
}

void setFan_Hitachi296(const uint8_t speed)
{
	uint8_t newSpeed = max(speed, kHitachiAc296FanSilent);
	data_Hitachi296.Fan = min(newSpeed, kHitachiAc296FanAuto);
}

void setInvertedStates_Hitachi296()
{
	invertBytePairs(data_Hitachi296.raw + 3, HITACHI296_NBYTES - 3);
}

uint8_t * invertBytePairs(uint8_t *ptr, const uint16_t length) {
   for (uint16_t i = 1; i < length; i += 2) {
     // Code done this way to avoid a compiler warning bug.
     uint8_t inv = ~*(ptr + i - 1);
     *(ptr + i) = inv;
   }
   return ptr;
}



