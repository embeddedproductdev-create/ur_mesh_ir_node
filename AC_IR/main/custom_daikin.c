#include "include/main.h"

/*DAIKIN 280 GLOBAL VARIABLE INITIALIZAITON*/
uint8_t curr_temp_Daikin280 = 25;
uint8_t curr_power_Daikin280 = false;
uint8_t curr_mode_Daikin280 = DaikinAuto;
uint8_t curr_fan_Daikin280 = DaikinFanMin;
Daikin280_t data_Daikin280;

/*DAIKIN 216 GLOBAL VARIABLE INITIALIZATION*/
uint8_t curr_temp_Daikin216 = 25;
uint8_t curr_power_Daikin216 = false;
uint8_t curr_mode_Daikin216 = DaikinAuto;
uint8_t curr_fan_Daikin216 = DaikinFanMin;
Daikin216_t data_Daikin216;

void setTemp_Daikin280(const uint8_t temp)
{
	uint8_t degrees = (uint8_t)max(temp, kDaikinMinTemp);
	degrees = (uint8_t)min(degrees, kDaikinMaxTemp);
	data_Daikin280.raw[kDaikinByteTemp] = degrees << 1;
}

void setTemp_Daikin216(const uint8_t temp) {
  uint8_t degrees = max(temp, kDaikinMinTemp);
  degrees = min(degrees, kDaikinMaxTemp);
  data_Daikin216.Temp = degrees;
}

void setPower_Daikin280(const bool on) { data_Daikin280.Power = on; }

void setPower_Daikin216(const bool on) { data_Daikin216.Power = on; }

void setBit_Daikin280(uint8_t *const data_Daikin280, const uint8_t position, const bool on)
{
	uint8_t mask = 1 << position;
	if (on)
		*data_Daikin280 |= mask;
	else
		*data_Daikin280 &= ~mask;
}

void setBits_Daikin280(uint8_t *const dst, const uint8_t offset, const uint8_t nbits,
					   const uint8_t data_Daikin280)
{
	if (offset >= 8 || !nbits)
		return; // Short circuit as it won't change.
	// Calculate the mask for the supplied value.
	uint8_t mask = UINT8_MAX >> (8 - ((nbits > 8) ? 8 : nbits));
	// Calculate the mask & clear the space for the data_Daikin280.
	// Clear the destination bits.
	*dst &= ~(uint8_t)(mask << offset);
	// Merge in the data_Daikin280.
	*dst |= ((data_Daikin280 & mask) << offset);
}

void setFan_Daikin280(const uint8_t fan)
{
	// Set the fan speed bits, leave low 4 bits alone
	uint8_t fanset;
	if (fan == kDaikinFanQuiet || fan == kDaikinFanAuto)
		fanset = fan;
	else if (fan < kDaikinFanMin || fan > kDaikinFanMax)
		fanset = kDaikinFanAuto;
	else
		fanset = 2 + fan;
	setBits_Daikin280(&data_Daikin280.raw[kDaikinByteFan], kDaikinFanOffset, kDaikinFanSize, fanset);
}

void setFan_Daikin216(const uint8_t fan) {
  // Set the fan speed bits, leave low 4 bits alone
  uint8_t fanset;
  if (fan == kDaikinFanQuiet || fan == kDaikinFanAuto)
    fanset = fan;
  else if (fan < kDaikinFanMin || fan > kDaikinFanMax)
    fanset = kDaikinFanAuto;
  else
    fanset = 2 + fan;
  data_Daikin216.Fan = fanset;
}

void setMode_Daikin280(const uint8_t mode)
{
	switch (mode)
	{
	case kDaikinAuto:
	case kDaikinCool:
	case kDaikinHeat:
	case kDaikinFan:
	case kDaikinDry:
		setBits_Daikin280(&data_Daikin280.raw[kDaikinBytePower], kDaikinModeOffset, kDaikinModeSize,
						  mode);
		break;
	default:
		setMode_Daikin280(kDaikinAuto);
	}
}

void setMode_Daikin216(const uint8_t mode)
{
	switch (mode)
	{
	case kDaikinAuto:
	case kDaikinCool:
	case kDaikinHeat:
	case kDaikinFan:
	case kDaikinDry:
		data_Daikin216.Mode = mode;
		break;
	default:
		data_Daikin216.Mode = kDaikinAuto;
	}
}

void setSwingVertical_Daikin280(const bool on)
{
	setBits_Daikin280(&data_Daikin280.raw[kDaikinByteFan], kDaikinSwingOffset, kDaikinSwingSize,
					  on ? kDaikinSwingOn : kDaikinSwingOff);
}

void setSwingVertical_Daikin216(const bool on) {
  data_Daikin216.SwingV = (on ? kDaikin216SwingOn : kDaikin216SwingOff);
}

void setSwingHorizontal_Daikin280(const bool on)
{
	setBits_Daikin280(&data_Daikin280.raw[kDaikinByteSwingH], kDaikinSwingOffset, kDaikinSwingSize,
					  on ? kDaikinSwingOn : kDaikinSwingOff);
}

void setSwingHorizontal(const bool on) {
  data_Daikin216.SwingH = (on ? kDaikin216SwingOn : kDaikin216SwingOff);
}

void setQuiet_Daikin280(const bool on)
{
	setBit_Daikin280(&data_Daikin280.raw[kDaikinByteSilent], kDaikinBitSilentOffset, on);
	// Powerful & Quiet mode being on are mutually exclusive.
	if (on)
		setPowerful_Daikin280(false);
}

void setPowerful_Daikin280(const bool on)
{
	setBit_Daikin280(&data_Daikin280.raw[kDaikinBytePowerful], kDaikinBitPowerfulOffset, on);
	if (on)
	{
		setQuiet_Daikin280(false);
		setEcono_Daikin280(false);
	}
}

void setEcono_Daikin280(const bool on)
{
	setBit_Daikin280(&data_Daikin280.raw[kDaikinByteEcono], kDaikinBitEconoOffset, on);
	if (on)
		setPowerful_Daikin280(false);
}

void setSensor_Daikin280(const bool on)
{
	setBit_Daikin280(&data_Daikin280.raw[kDaikinByteSensor], kDaikinBitSensorOffset, on);
}

void setMold_Daikin280(const bool on)
{
	setBit_Daikin280(&data_Daikin280.raw[kDaikinByteMold], kDaikinBitMoldOffset, on);
}

void setComfort_Daikin280(const bool on)
{
	setBit_Daikin280(&data_Daikin280.raw[kDaikinByteComfort], kDaikinBitComfortOffset, on);
}

void send_Daikin216()
{
	// Section #1
	sendGeneric(kDaikin216HdrMark, kDaikin216HdrSpace, kDaikin216BitMark,
				kDaikin216OneSpace, kDaikin216BitMark, kDaikin216ZeroSpace,
				kDaikin216BitMark, kDaikin216Gap, data_Daikin216.raw,
				kDaikin216Section1Length,
				kDaikin216Freq, false, 0, kDutyDefault);
	// Section #2
	sendGeneric(kDaikin216HdrMark, kDaikin216HdrSpace, kDaikin216BitMark,
				kDaikin216OneSpace, kDaikin216BitMark, kDaikin216ZeroSpace,
				kDaikin216BitMark, kDaikin216Gap,
				data_Daikin216.raw + kDaikin216Section1Length,
				27 - kDaikin216Section1Length,
				kDaikin216Freq, false, 0, kDutyDefault);
}

void send_Daikin280()
{
	uint16_t offset = 0;
	sendGenericmsgtime(0, 0, kDaikinBitMark, kDaikinOneSpace, kDaikinBitMark, kDaikinZeroSpace,
					   kDaikinBitMark, kDaikinZeroSpace + kDaikinGap, 0U, (uint64_t)0U, kDaikinHeaderLength,
					   41, false, 0, IRObject._dutycycle);
	sendGeneric(kDaikinHdrMark, kDaikinHdrSpace, kDaikinBitMark,
				kDaikinOneSpace, kDaikinBitMark, kDaikinZeroSpace,
				kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
				data_Daikin280.raw, kDaikinSection1Length, 41, false, 0, IRObject._dutycycle);
	offset += kDaikinSection1Length;
	sendGeneric(kDaikinHdrMark, kDaikinHdrSpace, kDaikinBitMark,
				kDaikinOneSpace, kDaikinBitMark, kDaikinZeroSpace,
				kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
				data_Daikin280.raw + offset, kDaikinSection2Length, 41, false, 0, IRObject._dutycycle);
	offset += kDaikinSection2Length;
	sendGeneric(kDaikinHdrMark, kDaikinHdrSpace, kDaikinBitMark,
				kDaikinOneSpace, kDaikinBitMark, kDaikinZeroSpace,
				kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
				data_Daikin280.raw + offset, 35 - offset, 41, false, 0, IRObject._dutycycle);
}

void checksum_Daikin280()
{
	data_Daikin280.raw[kDaikinByteChecksum1] = sumBytes(data_Daikin280.raw, kDaikinSection1Length - 1);
	data_Daikin280.raw[kDaikinByteChecksum2] = sumBytes(data_Daikin280.raw + kDaikinSection1Length,
														kDaikinSection2Length - 1);
	data_Daikin280.raw[kDaikinByteChecksum3] = sumBytes(data_Daikin280.raw + kDaikinSection1Length +
															kDaikinSection2Length,
														kDaikinSection3Length - 1);
}

void checksum_Daikin216(void)
{
	data_Daikin216.Sum1 = sumBytes(data_Daikin216.raw, kDaikin216Section1Length - 1);
	data_Daikin216.Sum2 = sumBytes(data_Daikin216.raw + kDaikin216Section1Length, kDaikin216Section2Length - 1);
}

void data_init_Daikin280()
{
	for (uint8_t index = 0; index < 35; index++)
	{
		data_Daikin280.raw[index] = 0x00;
	}
	data_Daikin280.raw[0] = 0x11;
	data_Daikin280.raw[1] = 0xda;
	data_Daikin280.raw[2] = 0x27;
	data_Daikin280.raw[4] = 0xc5;
	data_Daikin280.raw[8] = 0x11;
	data_Daikin280.raw[9] = 0xda;
	data_Daikin280.raw[10] = 0x27;
	data_Daikin280.raw[12] = 0x42;
	data_Daikin280.raw[16] = 0x11;
	data_Daikin280.raw[17] = 0xDA;
	data_Daikin280.raw[18] = 0x27;
	data_Daikin280.raw[21] = 0x49;
	data_Daikin280.raw[22] = 0x1E;
	data_Daikin280.raw[24] = 0xB0;
	data_Daikin280.raw[27] = 0x06;
	data_Daikin280.raw[28] = 0x60;
	data_Daikin280.raw[31] = 0xC0;
	setTemp_Daikin280(curr_temp_Daikin280);
	setMode_Daikin280(curr_mode_Daikin280);
	setFan_Daikin280(curr_fan_Daikin280);
	setPower_Daikin280(curr_power_Daikin280);
	checksum_Daikin280();
}

void data_init_Daikin216()
{
	for (uint8_t i = 0; i < 27; i++)
		data_Daikin216.raw[i] = 0x00;
	data_Daikin216.raw[0] = 0x11;
	data_Daikin216.raw[1] = 0xDA;
	data_Daikin216.raw[2] = 0x27;
	data_Daikin216.raw[3] = 0xF0;
	// _.raw[7] is a checksum byte, it will be set by checksum().
	data_Daikin216.raw[8] = 0x11;
	data_Daikin216.raw[9] = 0xDA;
	data_Daikin216.raw[10] = 0x27;
	data_Daikin216.raw[23] = 0xC0;
	// _.raw[26] is a checksum byte, it will be set by checksum().
}
