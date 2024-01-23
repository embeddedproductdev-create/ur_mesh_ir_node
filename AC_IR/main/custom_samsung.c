#include "include/main.h"

SamsungProtocol_t data_Samsung;

void setPower_Samsung(const bool on)
{

}

void setTemp_Samsung(const uint8_t temp)
{

}

void setFan_Samsung(const uint8_t fan)
{

}

void setMode_Samsung(const uint8_t mode)
{

}

void setSwingH_Samsung(const bool on)
{

}

void setSwingV_Samsung(const bool on)
{

}

void setOnTimer_Samsung()
{

}

void setOffTimer_Samsung()
{

}

void checksum_Samsung()
{

}

void send_Samsung()
{
	enableIROut(38, 65);
	// Header
	mark(kSamsungAcHdrMark);
	space(kSamsungAcHdrSpace);
	// Send in 7 byte sections.
	for (uint16_t offset = 0; offset < 14; offset += kSamsungAcSectionLength) {
	  sendGeneric(kSamsungAcSectionMark, kSamsungAcSectionSpace,
				  kSamsungAcBitMark, kSamsungAcOneSpace, kSamsungAcBitMark,
				  kSamsungAcZeroSpace, kSamsungAcBitMark, kSamsungAcSectionGap,
				  data_Samsung.raw + offset, kSamsungAcSectionLength,  // 7 bytes == 56 bits
				  38000, false, 0, 50);                    // Send in LSBF order
	}
	// Complete made up guess at inter-message gap.
	space(kDefaultMessageGap - kSamsungAcSectionGap);
}

void send_extended_Samsung()
{
	  static const uint8_t extended_middle_section[kSamsungAcSectionLength] = {
	      0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00};
	  // Copy/convert the internal state to an extended state by
	  // copying the second section to the third section, and inserting the extended
	  // middle (second) section.
	  memcpy(data_Samsung.raw + 2 * kSamsungAcSectionLength,
			  data_Samsung.raw + kSamsungAcSectionLength,
	              kSamsungAcSectionLength);
	  memcpy(data_Samsung.raw + kSamsungAcSectionLength, extended_middle_section,
	              kSamsungAcSectionLength);
	  send_Samsung();
	  // Now revert it by copying the third section over the second section.
	  memcpy(data_Samsung.raw + kSamsungAcSectionLength,
	              data_Samsung.raw + 2 * kSamsungAcSectionLength,
	              kSamsungAcSectionLength);
}

void init_dataSamsung()
{
	static const uint8_t kReset[kSamsungAcExtendedStateLength] = {
	      0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,
	      0x01, 0x02, 0xAE, 0x71, 0x00, 0x15, 0xF0};
	memcpy(data_Samsung.raw, kReset, kSamsungAcExtendedStateLength);
	setPower(true);
	data_Samsung.OnTimerEnable = false;
	data_Samsung.OffTimerEnable = false;
}
