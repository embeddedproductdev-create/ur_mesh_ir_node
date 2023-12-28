#ifndef MAIN_INCLUDE_CUSTOM_DAIKIN_H_
#define MAIN_INCLUDE_CUSTOM_DAIKIN_H_

#include <main.h>

/*DAIKIN CONSTANTS*/
#define kNoRepeat 0
#define kMarkExcess 50
#define kSingleRepeat 1
#define kDaikinStateLength 35
#define kDaikinBits kDaikinStateLength * 8
#define kDaikinStateLengthShort kDaikinStateLength - 8
#define kDaikinBitsShort kDaikinStateLengthShort * 8
#define kDaikinDefaultRepeat kNoRepeat
#define kDaikinAuto 0b000
#define kDaikinDry 0b010
#define kDaikinCool 0b011
#define kDaikinHeat 0b100
#define kDaikinFan 0b110
#define kDaikinModeOffset 4
#define kDaikinModeSize 3
#define kDaikinMinTemp 10 // Celsius
#define kDaikinMaxTemp 32 // Celsius
#define kDaikinFanMin 1
#define kDaikinFanMed 3
#define kDaikinFanMax 5
#define kDaikinFanAuto 0b1010  // 10 / 0xA
#define kDaikinFanQuiet 0b1011 // 11 / 0xB
#define kDaikinFanOffset 4
#define kDaikinFanSize 4
#define kDaikinSwingOffset 0
#define kDaikinSwingSize 4
#define kDaikinSwingOn 0b1111
#define kDaikinSwingOff 0b0000
#define kDaikinHeaderLength 5
#define kDaikinSections 3
#define kDaikinSection1Length 8
#define kDaikinSection2Length 8
#define kDaikinSection3Length kDaikinStateLength - kDaikinSection1Length - kDaikinSection2Length
#define kDaikinByteComfort 6
#define kDaikinByteChecksum1 7
#define kDaikinBitComfortOffset 4
#define kDaikinBitComfort 1 << kDaikinBitComfortOffset
#define kDaikinByteClockMinsLow 13
#define kDaikinByteClockMinsHigh 14
#define kDaikinClockMinsHighOffset 0
#define kDaikinClockMinsHighSize 3
#define kDaikinDoWOffset 3
#define kDaikinDoWSize 3
#define kDaikinByteChecksum2 15
#define kDaikinBytePower 21
#define kDaikinBitPowerOffset 0
#define kDaikinBitPower 1 << kDaikinBitPowerOffset
#define kDaikinTempOffset 1
#define kDaikinTempSize 6
#define kDaikinByteTemp 22
#define kDaikinByteFan 24
#define kDaikinByteSwingH 25
#define kDaikinByteOnTimerMinsLow 26
#define kDaikinByteOnTimerMinsHigh 27
#define kDaikinOnTimerMinsHighOffset 0
#define kDaikinOnTimerMinsHighSize 4
#define kDaikinByteOffTimerMinsLow kDaikinByteOnTimerMinsHigh
#define kDaikinByteOffTimerMinsHigh 28
#define kDaikinBytePowerful 29
#define kDaikinBitPowerfulOffset 0
#define kDaikinBitPowerful 1 << kDaikinBitPowerfulOffset
#define kDaikinByteSilent kDaikinBytePowerful
#define kDaikinBitSilentOffset 5
#define kDaikinBitSilent 1 << kDaikinBitSilentOffset
#define kDaikinByteSensor 32
#define kDaikinBitSensorOffset 1
#define kDaikinBitSensor 1 << kDaikinBitSensorOffset
#define kDaikinByteEcono kDaikinByteSensor
#define kDaikinBitEconoOffset 2
#define kDaikinBitEcono 1 << kDaikinBitEconoOffset
#define kDaikinByteEye kDaikinByteSensor
#define kDaikinBitEye 0b10000000
#define kDaikinByteWeeklyTimer kDaikinByteSensor
#define kDaikinBitWeeklyTimerOffset 7
#define kDaikinBitWeeklyTimer 1 << kDaikinBitWeeklyTimerOffset
#define kDaikinByteMold 33
#define kDaikinBitMoldOffset 1
#define kDaikinBitMold 1 << kDaikinBitMoldOffset
#define kDaikinByteOffTimer kDaikinBytePower
#define kDaikinBitOffTimerOffset 2
#define kDaikinBitOffTimer 1 << kDaikinBitOffTimerOffset
#define kDaikinByteOnTimer kDaikinByteOffTimer
#define kDaikinBitOnTimerOffset 1
#define kDaikinBitOnTimer 1 << kDaikinBitOnTimerOffset
#define kDaikinByteChecksum3 kDaikinStateLength - 1
#define kDaikinUnusedTime 0x600
#define kDaikinBeepQuiet 1
#define kDaikinBeepLoud 2
#define kDaikinBeepOff 3
#define kDaikinLightBright 1
#define kDaikinLightDim 2
#define kDaikinLightOff 3
#define kDaikinCurBit kDaikinStateLength
#define kDaikinCurIndex kDaikinStateLength + 1
#define kDaikinTolerance 35
#define kDaikinMarkExcess kMarkExcess
#define kDaikinHdrMark 3650  // kDaikinBitMark * 8
#define kDaikinHdrSpace 1623 // kDaikinBitMark * 4
#define kDaikinBitMark 428
#define kDaikinZeroSpace 428
#define kDaikinOneSpace 1280
#define kDaikinGap 29000
#define kDaikinFirstHeader64 0b1101011100000000000000001100010100000000001001111101101000010001


/*DAIKIN 280 CONSTANTS*/
#define DaikinAuto 0
#define DaikinDry 2
#define DaikinCool 3
#define DaikinHeat 4
#define DaikinFan 6
#define DaikinFanMin 1
#define DaikinFanMed 3
#define DaikinFanMax 5
#define DaikinFanAuto 10
#define DaikinFanQuiet 11

/*DAIKIN 216 CONSTANTS*/
#define kDaikin216Freq 38000
#define kDaikin216HdrMark 3440
#define kDaikin216HdrSpace 1750
#define kDaikin216BitMark 420
#define kDaikin216OneSpace 1300
#define kDaikin216ZeroSpace 450
#define kDaikin216Gap 29650
#define kDaikin216Sections 2
#define kDaikin216Section1Length 8
#define kDaikin216StateLength 27
#define kDaikin216Section2Length (kDaikin216StateLength - kDaikin216Section1Length)
#define kDaikin216SwingOn 0b1111
#define kDaikin216SwingOff 0b0000


typedef union Daikin280
{
  uint8_t raw[35]; ///< The state of the IR remote.
  struct
  {
    // Byte 0~5
    uint64_t : 48;
    // Byte 6
    uint64_t : 4;
    uint64_t Comfort : 1;
    uint64_t : 3;
    // Byte 7
    uint64_t Sum1 : 8; // checksum of the first part

    // Byte 8~12
    uint64_t : 40;
    // Byte 13~14
    uint64_t CurrentTime : 11; // Current time, mins past midnight
    uint64_t CurrentDay : 3;   // Day of the week (SUN=1, MON=2, ..., SAT=7)
    uint64_t : 2;
    // Byte 15
    uint64_t Sum2 : 8; // checksum of the second part

    // Byte 16~20
    uint64_t : 40;
    // Byte 21
    uint64_t Power : 1;
    uint64_t OnTimer : 1;
    uint64_t OffTimer : 1;
    uint64_t : 1; // always 1
    uint64_t Mode : 3;
    uint64_t : 1;
    // Byte 22
    uint64_t Temp : 8; // Temp should be between 20 - 64 (10 C - 32 C)
    // Byte 23
    uint64_t : 8;

    // Byte 24
    uint64_t SwingV : 4; // 0000       off, 1111      on
    uint64_t Fan : 4;
    // Byte 25
    uint64_t SwingH : 4; // 0000       off, 1111      on
    uint64_t : 4;
    // Byte 26~28
    uint64_t OnTime : 12;  // timer mins past midnight
    uint64_t OffTime : 12; // timer mins past midnight
    // Byte 29
    uint64_t Powerful : 1;
    uint64_t : 4;
    uint64_t Quiet : 1;
    uint64_t : 2;
    // Byte 30~31
    uint64_t : 0;

    // Byte 32
    uint8_t : 1;
    uint8_t Sensor : 1;
    uint8_t Econo : 1;
    uint8_t : 4;
    uint8_t WeeklyTimer : 1;
    // Byte 33
    uint8_t : 1;
    uint8_t Mold : 1;
    uint8_t : 6;
    // Byte 34
    uint8_t Sum3 : 8; // checksum of the third part
  };
} Daikin280_t;


typedef union Daikin216{
  uint8_t raw[27];  ///< The state of the IR remote.
  struct {
    // Byte 0~6
    uint8_t pad0[7];
    // Byte 7
    uint8_t Sum1  :8;
    // Byte 8~12
    uint8_t pad1[5];
    // Byte 13
    uint8_t Power :1;
    uint8_t       :3;
    uint8_t Mode  :3;
    uint8_t       :1;
    // Byte 14
    uint8_t         :1;
    uint8_t Temp    :6;
    uint8_t         :1;
    // Byte 15
    uint8_t         :8;
    // Byte 16
    uint8_t SwingV  :4;
    uint8_t Fan     :4;
    // Byte 17
    uint8_t SwingH  :4;
    uint8_t         :4;
    // Byte 18~20
    uint8_t pad2[3];
    // Byte 21
    uint8_t Powerful  :1;
    uint8_t           :0;
    // Byte 22~25
    uint8_t pad3[4];
    // Byte 26
    uint8_t Sum2      :8;
  };
}Daikin216_t;


/*DAIKIN 280 FUNCTIONS*/
void setTemp_Daikin280(const uint8_t temp);
void setPower_Daikin280(const bool on);
void setBit_Daikin280(uint8_t *const data, const uint8_t position, const bool on);
void setFan_Daikin280(const uint8_t fan);
void setBits_Daikin280(uint8_t *const dst, const uint8_t offset, const uint8_t nbits, const uint8_t data);
void setMode_Daikin280(const uint8_t mode);
void setSwingVertical_Daikin280(const bool on);
void setSwingHorizontal_Daikin280(const bool on);
void setQuiet_Daikin280(const bool on);
void setPowerful_Daikin280(const bool on);
void setEcono_Daikin280(const bool on);
void setSensor_Daikin280(const bool on);
void setMold_Daikin280(const bool on);
void setComfort_Daikin280(const bool on);
void disableOnTimer_Daikin280(void);
void checksum_Daikin280();
void data_init_Daikin280();
void send_Daikin280();

/*DAIKIN 216 FUNCTIONS*/
void checksum_Daikin216(void);
void data_init_Daikin216(void);
void setPower_Daikin216(const bool on);
void setTemp_Daikin216(const uint8_t temp);
void setMode_Daikin216(const uint8_t mode);
void setFan_Daikin216(const uint8_t fan);
void setSwingVertical_Daikin216(const bool on);
void setSwingHorizontal(const bool on);
void send_Daikin216();

/*DAIKIN 280 GLOBAL VARIABLES*/
extern Daikin280_t data_Daikin280;
extern uint8_t curr_temp_Daikin280;
extern uint8_t curr_fan_Daikin280;
extern uint8_t curr_mode_Daikin280;
extern uint8_t curr_power_Daikin280;

/*DAIKIN 216 GLOBAL VARIABLES*/
extern Daikin216_t data_Daikin216;
extern uint8_t curr_temp_Daikin216;
extern uint8_t curr_fan_Daikin216;
extern uint8_t curr_mode_Daikin216;
extern uint8_t curr_power_Daikin216;

#endif
