#include "include/custom_daikin.h"
#include "include/custom_irsend.h"
#include <stdint.h>
#include <stdio.h>

const uint16_t kNoRepeat = 0;
const uint16_t kMarkExcess = 50;
const uint16_t kSingleRepeat = 1;
const uint16_t kDaikinStateLength = 35;
const uint16_t kDaikinBits = kDaikinStateLength * 8;
const uint16_t kDaikinStateLengthShort = kDaikinStateLength - 8;
const uint16_t kDaikinBitsShort = kDaikinStateLengthShort * 8;
const uint16_t kDaikinDefaultRepeat = kNoRepeat;
const uint16_t kDaikin2StateLength = 39;
const uint16_t kDaikin2Bits = kDaikin2StateLength * 8;
const uint16_t kDaikin2DefaultRepeat = kNoRepeat;
const uint16_t kDaikin64Bits = 64;
const uint16_t kDaikin64DefaultRepeat = kNoRepeat;
const uint16_t kDaikin160StateLength = 20;
const uint16_t kDaikin160Bits = kDaikin160StateLength * 8;
const uint16_t kDaikin160DefaultRepeat = kNoRepeat;
const uint16_t kDaikin128StateLength = 16;
const uint16_t kDaikin128Bits = kDaikin128StateLength * 8;
const uint16_t kDaikin128DefaultRepeat = kNoRepeat;
const uint16_t kDaikin152StateLength = 19;
const uint16_t kDaikin152Bits = kDaikin152StateLength * 8;
const uint16_t kDaikin152DefaultRepeat = kNoRepeat;
const uint16_t kDaikin176StateLength = 22;
const uint16_t kDaikin176Bits = kDaikin176StateLength * 8;
const uint16_t kDaikin176DefaultRepeat = kNoRepeat;
const uint16_t kDaikin216StateLength = 27;
const uint16_t kDaikin216Bits = kDaikin216StateLength * 8;
const uint16_t kDaikin216DefaultRepeat = kNoRepeat;
const uint8_t kDaikinAuto = 0b000;
const uint8_t kDaikinDry =  0b010;
const uint8_t kDaikinCool = 0b011;
const uint8_t kDaikinHeat = 0b100;
const uint8_t kDaikinFan =  0b110;
const uint8_t kDaikinModeOffset = 4;
const uint8_t kDaikinModeSize = 3;
const uint8_t kDaikinMinTemp = 10;  // Celsius
const uint8_t kDaikinMaxTemp = 32;  // Celsius
const uint8_t kDaikinFanMin = 1;
const uint8_t kDaikinFanMed = 3;
const uint8_t kDaikinFanMax = 5;
const uint8_t kDaikinFanAuto = 0b1010;  // 10 / 0xA
const uint8_t kDaikinFanQuiet = 0b1011;  // 11 / 0xB
const uint8_t kDaikinFanOffset = 4;
const uint8_t kDaikinFanSize = 4;
const uint8_t kDaikinSwingOffset = 0;
const uint8_t kDaikinSwingSize = 4;
const uint8_t kDaikinSwingOn =  0b1111;
const uint8_t kDaikinSwingOff = 0b0000;
const uint16_t kDaikinHeaderLength = 5;
const uint8_t kDaikinSections = 3;
const uint8_t kDaikinSection1Length = 8;
const uint8_t kDaikinSection2Length = 8;
const uint8_t kDaikinSection3Length = kDaikinStateLength - kDaikinSection1Length - kDaikinSection2Length;
const uint8_t kDaikinByteComfort = 6;
const uint8_t kDaikinByteChecksum1 = 7;
const uint8_t kDaikinBitComfortOffset = 4;
const uint8_t kDaikinBitComfort = 1 << kDaikinBitComfortOffset;
const uint8_t kDaikinByteClockMinsLow = 13;
const uint8_t kDaikinByteClockMinsHigh = 14;
const uint8_t kDaikinClockMinsHighOffset = 0;
const uint8_t kDaikinClockMinsHighSize = 3;
const uint8_t kDaikinDoWOffset = 3;
const uint8_t kDaikinDoWSize = 3;
const uint8_t kDaikinByteChecksum2 = 15;
const uint8_t kDaikinBytePower = 21;
const uint8_t kDaikinBitPowerOffset = 0;
const uint8_t kDaikinBitPower = 1 << kDaikinBitPowerOffset;
const uint8_t kDaikinTempOffset = 1;
const uint8_t kDaikinTempSize = 6;
const uint8_t kDaikinByteTemp = 22;
const uint8_t kDaikinByteFan = 24;
const uint8_t kDaikinByteSwingH = 25;
const uint8_t kDaikinByteOnTimerMinsLow = 26;
const uint8_t kDaikinByteOnTimerMinsHigh = 27;
const uint8_t kDaikinOnTimerMinsHighOffset = 0;
const uint8_t kDaikinOnTimerMinsHighSize = 4;
const uint8_t kDaikinByteOffTimerMinsLow = kDaikinByteOnTimerMinsHigh;
const uint8_t kDaikinByteOffTimerMinsHigh = 28;
const uint8_t kDaikinBytePowerful = 29;
const uint8_t kDaikinBitPowerfulOffset = 0;
const uint8_t kDaikinBitPowerful = 1 << kDaikinBitPowerfulOffset;
const uint8_t kDaikinByteSilent = kDaikinBytePowerful;
const uint8_t kDaikinBitSilentOffset = 5;
const uint8_t kDaikinBitSilent = 1 << kDaikinBitSilentOffset;
const uint8_t kDaikinByteSensor = 32;
const uint8_t kDaikinBitSensorOffset = 1;
const uint8_t kDaikinBitSensor = 1 << kDaikinBitSensorOffset;
const uint8_t kDaikinByteEcono = kDaikinByteSensor;
const uint8_t kDaikinBitEconoOffset = 2;
const uint8_t kDaikinBitEcono = 1 << kDaikinBitEconoOffset;
const uint8_t kDaikinByteEye = kDaikinByteSensor;
const uint8_t kDaikinBitEye = 0b10000000;
const uint8_t kDaikinByteWeeklyTimer = kDaikinByteSensor;
const uint8_t kDaikinBitWeeklyTimerOffset = 7;
const uint8_t kDaikinBitWeeklyTimer = 1 << kDaikinBitWeeklyTimerOffset;
const uint8_t kDaikinByteMold = 33;
const uint8_t kDaikinBitMoldOffset = 1;
const uint8_t kDaikinBitMold = 1 << kDaikinBitMoldOffset;
const uint8_t kDaikinByteOffTimer = kDaikinBytePower;
const uint8_t kDaikinBitOffTimerOffset = 2;
const uint8_t kDaikinBitOffTimer = 1 << kDaikinBitOffTimerOffset;
const uint8_t kDaikinByteOnTimer = kDaikinByteOffTimer;
const uint8_t kDaikinBitOnTimerOffset = 1;
const uint8_t kDaikinBitOnTimer = 1 << kDaikinBitOnTimerOffset;
const uint8_t kDaikinByteChecksum3 = kDaikinStateLength - 1;
const uint16_t kDaikinUnusedTime = 0x600;
const uint8_t kDaikinBeepQuiet = 1;
const uint8_t kDaikinBeepLoud = 2;
const uint8_t kDaikinBeepOff = 3;
const uint8_t kDaikinLightBright = 1;
const uint8_t kDaikinLightDim = 2;
const uint8_t kDaikinLightOff = 3;
const uint8_t kDaikinCurBit = kDaikinStateLength;
const uint8_t kDaikinCurIndex = kDaikinStateLength + 1;
const uint8_t kDaikinTolerance = 35;
const uint16_t kDaikinMarkExcess = kMarkExcess;
const uint16_t kDaikinHdrMark = 3650;   // kDaikinBitMark * 8
const uint16_t kDaikinHdrSpace = 1623;  // kDaikinBitMark * 4
const uint16_t kDaikinBitMark = 428;
const uint16_t kDaikinZeroSpace = 428;
const uint16_t kDaikinOneSpace = 1280;
const uint16_t kDaikinGap = 29000;
// Note bits in each octet swapped so can be sent as a single value
const uint64_t kDaikinFirstHeader64 =
    0b1101011100000000000000001100010100000000001001111101101000010001;


const uint16_t kDaikin2Freq = 36700;  // Modulation Frequency in Hz.
const uint16_t kDaikin2LeaderMark = 10024;
const uint16_t kDaikin2LeaderSpace = 25180;
const uint16_t kDaikin2Gap = kDaikin2LeaderMark + kDaikin2LeaderSpace;
const uint16_t kDaikin2HdrMark = 3500;
const uint16_t kDaikin2HdrSpace = 1728;
const uint16_t kDaikin2BitMark = 460;
const uint16_t kDaikin2OneSpace = 1270;
const uint16_t kDaikin2ZeroSpace = 420;
const uint16_t kDaikin2Sections = 2;
const uint16_t kDaikin2Section1Length = 20;
const uint16_t kDaikin2Section2Length = 19;
const uint8_t kDaikin2Tolerance = 5;  // Extra percentage tolerance
const uint8_t kDaikin2BitSleepTimerOffset = 5;
const uint8_t kDaikin2BitSleepTimer = 1 << kDaikin2BitSleepTimerOffset;
const uint8_t kDaikin2BitPurifyOffset = 4;
const uint8_t kDaikin2BitPurify = 1 << kDaikin2BitPurifyOffset;  // 0b00010000
const uint8_t kDaikin2BitEyeOffset = 1;
const uint8_t kDaikin2BitEye = 1 << kDaikin2BitEyeOffset;  // 0b00000010
const uint8_t kDaikin2BitEyeAutoOffset = 7;
const uint8_t kDaikin2BitEyeAuto = 1 << kDaikin2BitEyeAutoOffset;  // 0b10000000
const uint8_t kDaikin2BitMoldOffset = 3;
const uint8_t kDaikin2BitMold = 1 << kDaikin2BitMoldOffset;    // 0b00001000
const uint8_t kDaikin2BitCleanOffset = 5;  // Byte[8]
const uint8_t kDaikin2BitClean = 1 << kDaikin2BitCleanOffset;  // 0b00100000
const uint8_t kDaikin2BitFreshAirOffset = 0;
const uint8_t kDaikin2BitFreshAir = 1 << kDaikin2BitFreshAirOffset;
const uint8_t kDaikin2BitFreshAirHighOffset = 7;
const uint8_t kDaikin2BitFreshAirHigh = 1 << kDaikin2BitFreshAirHighOffset;
const uint8_t kDaikin2BitPowerOffset = 7;
const uint8_t kDaikin2BitPower = 1 << kDaikin2BitPowerOffset;  // 0b10000000
// const uint8_t kDaikin2LightMask =    0b00110000;  // Byte[7]
const uint8_t kDaikin2LightOffset = 4;  // Byte[7]
const uint8_t kDaikin2LightSize = 2;
// const uint8_t kDaikin2BeepMask =     0b11000000;  // Byte[7]
const uint8_t kDaikin2BeepOffset = 6;  // Byte[7]
const uint8_t kDaikin2BeepSize = 2;
const uint8_t kDaikin2SwingVHigh = 0x1;
const uint8_t kDaikin2SwingVLow = 0x6;
const uint8_t kDaikin2SwingVSwing = 0xF;
const uint8_t kDaikin2SwingVAuto = 0xE;
const uint8_t kDaikin2SwingVBreeze = 0xC;
const uint8_t kDaikin2SwingVCirculate = 0xD;
const uint8_t kDaikin2FanByte = 28;

const uint8_t kDaikin2SwingHWide =     0xA3;
const uint8_t kDaikin2SwingHLeftMax =  0xA8;
const uint8_t kDaikin2SwingHLeft =     0xA9;
const uint8_t kDaikin2SwingHMiddle =   0xAA;
const uint8_t kDaikin2SwingHRight =    0xAB;
const uint8_t kDaikin2SwingHRightMax = 0xAC;
const uint8_t kDaikin2SwingHAuto =     0xBE;
const uint8_t kDaikin2SwingHSwing =    0xBF;

const uint8_t kDaikin2MinCoolTemp = 18;  // Min temp (in C) when in Cool mode.


const uint16_t kDaikin216Freq = 38000;  // Modulation Frequency in Hz.
const uint16_t kDaikin216HdrMark = 3440;
const uint16_t kDaikin216HdrSpace = 1750;
const uint16_t kDaikin216BitMark = 420;
const uint16_t kDaikin216OneSpace = 1300;
const uint16_t kDaikin216ZeroSpace = 450;
const uint16_t kDaikin216Gap = 29650;
const uint16_t kDaikin216Sections = 2;
const uint16_t kDaikin216Section1Length = 8;
const uint16_t kDaikin216Section2Length = kDaikin216StateLength -
                                          kDaikin216Section1Length;
const uint8_t kDaikin216BytePower = 13;
const uint8_t kDaikin216ByteMode = kDaikin216BytePower;
// const uint8_t kDaikin216MaskMode = 0b01110000;
const uint8_t kDaikin216ByteTemp = 14;
// const uint8_t kDaikin216MaskTemp = 0b01111110;
const uint8_t kDaikin216TempOffset = 1;
const uint8_t kDaikin216TempSize = 6;

const uint8_t kDaikin216ByteFan = 16;
const uint8_t kDaikin216MaskFan = 0b11110000;
const uint8_t kDaikin216ByteSwingV = 16;
// const uint8_t kDaikin216MaskSwingV = 0b00001111;
const uint8_t kDaikin216SwingSize = 4;
const uint8_t kDaikin216SwingOn = 0b1111;
const uint8_t kDaikin216SwingOff = 0b0000;
const uint8_t kDaikin216ByteSwingH = 17;
const uint8_t kDaikin216BytePowerful = 21;


const uint16_t kDaikin160Freq = 38000;  // Modulation Frequency in Hz.
const uint16_t kDaikin160HdrMark = 5000;
const uint16_t kDaikin160HdrSpace = 2145;
const uint16_t kDaikin160BitMark = 342;
const uint16_t kDaikin160OneSpace = 1786;
const uint16_t kDaikin160ZeroSpace = 700;
const uint16_t kDaikin160Gap = 29650;
const uint16_t kDaikin160Sections = 2;
const uint16_t kDaikin160Section1Length = 7;
const uint16_t kDaikin160Section2Length = kDaikin160StateLength -
                                          kDaikin160Section1Length;
const uint8_t kDaikin160BytePower = 12;
const uint8_t kDaikin160ByteMode = kDaikin160BytePower;
// const uint8_t kDaikin160MaskMode = 0b01110000;
const uint8_t kDaikin160ByteTemp = 16;
// const uint8_t kDaikin160MaskTemp = 0b01111110;
const uint8_t kDaikin160TempOffset = 1;
const uint8_t kDaikin160TempSize = 6;
const uint8_t kDaikin160ByteFan = 17;
const uint8_t kDaikin160MaskFan = 0b00001111;
const uint8_t kDaikin160ByteSwingV = 13;
const uint8_t kDaikin160MaskSwingV = 0b11110000;
const uint8_t kDaikin160SwingVLowest =  0x1;
const uint8_t kDaikin160SwingVLow =     0x2;
const uint8_t kDaikin160SwingVMiddle =  0x3;
const uint8_t kDaikin160SwingVHigh =    0x4;
const uint8_t kDaikin160SwingVHighest = 0x5;
const uint8_t kDaikin160SwingVAuto =    0xF;


const uint16_t kDaikin176Freq = 38000;  // Modulation Frequency in Hz.
const uint16_t kDaikin176HdrMark = 5070;
const uint16_t kDaikin176HdrSpace = 2140;
const uint16_t kDaikin176BitMark = 370;
const uint16_t kDaikin176OneSpace = 1780;
const uint16_t kDaikin176ZeroSpace = 710;
const uint16_t kDaikin176Gap = 29410;
const uint16_t kDaikin176Sections = 2;
const uint16_t kDaikin176Section1Length = 7;
const uint16_t kDaikin176Section2Length = kDaikin176StateLength -
                                          kDaikin176Section1Length;
const uint8_t kDaikin176ByteAltMode = 12;
const uint8_t kDaikin176ByteModePower = 14;
const uint8_t kDaikin176Fan =  0b000;  // 0
const uint8_t kDaikin176Heat = 0b001;  // 1
const uint8_t kDaikin176Cool = 0b010;  // 2
const uint8_t kDaikin176Auto = 0b011;  // 3
const uint8_t kDaikin176Dry =  0b111;  // 7
const uint8_t kDaikin176MaskMode = 0b01110000;
const uint8_t kDaikin176ByteModeButton = 13;
const uint8_t kDaikin176ModeButton = 0b00000100;
const uint8_t kDaikin176ByteTemp = 17;
// const uint8_t kDaikin176MaskTemp = 0b01111110;
const uint8_t kDaikin176TempOffset = 1;
const uint8_t kDaikin176TempSize = 6;
const uint8_t kDaikin176DryFanTemp = 17;  // Dry/Fan mode is always 17 Celsius.
const uint8_t kDaikin176ByteFan = 18;
const uint8_t kDaikin176MaskFan = 0b11110000;
const uint8_t kDaikin176FanMax = 3;
const uint8_t kDaikin176ByteSwingH = 18;
// const uint8_t kDaikin176MaskSwingH = 0b00001111;
const uint8_t kDaikin176SwingHAuto =  0x5;
const uint8_t kDaikin176SwingHOff = 0x6;


const uint16_t kDaikin128Freq = 38000;  // Modulation Frequency in Hz.
const uint16_t kDaikin128LeaderMark = 9800;
const uint16_t kDaikin128LeaderSpace = 9800;
const uint16_t kDaikin128HdrMark = 4600;
const uint16_t kDaikin128HdrSpace = 2500;
const uint16_t kDaikin128BitMark = 350;
const uint16_t kDaikin128OneSpace = 954;
const uint16_t kDaikin128ZeroSpace = 382;
const uint16_t kDaikin128Gap = 20300;
const uint16_t kDaikin128FooterMark = kDaikin128HdrMark;
const uint16_t kDaikin128Sections = 2;
const uint16_t kDaikin128SectionLength = 8;
const uint8_t kDaikin128ByteModeFan = 1;
// const uint8_t kDaikin128MaskMode =     0b00001111;
const uint8_t kDaikin128ModeSize = 4;
const uint8_t kDaikin128Dry =             0b00000001;
const uint8_t kDaikin128Cool =            0b00000010;
const uint8_t kDaikin128Fan =             0b00000100;
const uint8_t kDaikin128Heat =            0b00001000;
const uint8_t kDaikin128Auto =            0b00001010;
const uint8_t kDaikin128MaskFan =         0b11110000;
const uint8_t kDaikin128FanAuto =         0b0001;
const uint8_t kDaikin128FanHigh =         0b0010;
const uint8_t kDaikin128FanMed =          0b0100;
const uint8_t kDaikin128FanLow =          0b1000;
const uint8_t kDaikin128FanPowerful =     0b0011;
const uint8_t kDaikin128FanQuiet =        0b1001;
const uint8_t kDaikin128ByteClockMins = 2;
const uint8_t kDaikin128ByteClockHours = 3;
const uint8_t kDaikin128ByteOnTimer = 4;
const uint8_t kDaikin128ByteOffTimer = 5;
const uint8_t kDaikin128BitTimerEnabledOffset = 7;
const uint8_t kDaikin128BitTimerEnabled = 1 << kDaikin128BitTimerEnabledOffset;
const uint8_t kDaikin128TimerOffset = 0;
const uint8_t kDaikin128TimerSize = 7;
const uint8_t kDaikin128HalfHourOffset = 6;
const uint8_t kDaikin128BitHalfHour = 1 << kDaikin128HalfHourOffset;
// const uint8_t kDaikin128MaskHours =       0b00111111;
const uint8_t kDaikin128HoursOffset = 0;
const uint8_t kDaikin128HoursSize = 6;
const uint8_t kDaikin128ByteTemp = 6;
const uint8_t kDaikin128MinTemp = 16;  // C
const uint8_t kDaikin128MaxTemp = 30;  // C
const uint8_t kDaikin128BytePowerSwingSleep = 7;
const uint8_t kDaikin128BitSwingOffset = 0;
const uint8_t kDaikin128BitSwing = 1 << kDaikin128BitSwingOffset;  // 0b00000001
const uint8_t kDaikin128BitSleepOffset = 1;
const uint8_t kDaikin128BitSleep = 1 << kDaikin128BitSleepOffset;  // 0b00000010
const uint8_t kDaikin128BitPowerToggleOffset = 3;
const uint8_t kDaikin128BitPowerToggle = 1 << kDaikin128BitPowerToggleOffset;
const uint8_t kDaikin128ByteEconoLight = 9;
const uint8_t kDaikin128BitEconoOffset = 2;
const uint8_t kDaikin128BitEcono = 1 << kDaikin128BitEconoOffset;  // 0b00000100
const uint8_t kDaikin128BitWall =         0b00001000;
const uint8_t kDaikin128BitCeiling =      0b00000001;
const uint8_t kDaikin128MaskLight = kDaikin128BitWall | kDaikin128BitCeiling;


const uint16_t kDaikin152Freq = 38000;  // Modulation Frequency in Hz.
const uint8_t  kDaikin152LeaderBits = 5;
const uint16_t kDaikin152HdrMark = 3492;
const uint16_t kDaikin152HdrSpace = 1718;
const uint16_t kDaikin152BitMark = 433;
const uint16_t kDaikin152OneSpace = 1529;
const uint16_t kDaikin152ZeroSpace = kDaikin152BitMark;
const uint16_t kDaikin152Gap = 25182;

// Byte[5]
const uint8_t kDaikin152ModeByte = 5;                        // Mask 0b01110000
const uint8_t kDaikin152PowerByte = kDaikin152ModeByte;      // Mask 0b00000001
// Byte[6]
const uint8_t kDaikin152TempByte = 6;                        // Mask 0b11111110
const uint8_t kDaikin152TempSize = 7;
const uint8_t kDaikin152DryTemp = kDaikin2MinCoolTemp;  // Celsius
const uint8_t kDaikin152FanTemp = 0x60;  // 96 Celsius
// Byte[8]
const uint8_t kDaikin152FanByte = 8;
const uint8_t kDaikin152SwingVByte = kDaikin152FanByte;
// Byte[13]
const uint8_t kDaikin152QuietByte = 13;                      // Mask 0b00100000
const uint8_t kDaikin152PowerfulByte = kDaikin152QuietByte;  // Mask 0b00000001
// Byte[16]
const uint8_t kDaikin152EconoByte = 16;                      // Mask 0b00000100
const uint8_t kDaikin152ComfortByte = kDaikin152EconoByte;   // Mask 0b00000010
const uint8_t kDaikin152ComfortOffset = 1;                   // Mask 0b00000010
const uint8_t kDaikin152SensorByte = kDaikin152EconoByte;    // Mask 0b00001000
const uint8_t kDaikin152SensorOffset = 3;                    // Mask 0b00001000


const uint16_t kDaikin64HdrMark = kDaikin128HdrMark;
const uint16_t kDaikin64BitMark = kDaikin128BitMark;
const uint16_t kDaikin64HdrSpace = kDaikin128HdrSpace;
const uint16_t kDaikin64OneSpace = kDaikin128OneSpace;
const uint16_t kDaikin64ZeroSpace = kDaikin128ZeroSpace;
const uint16_t kDaikin64LdrMark = kDaikin128LeaderMark;
const uint16_t kDaikin64Gap = kDaikin128Gap;
const uint16_t kDaikin64LdrSpace = kDaikin128LeaderSpace;
const uint16_t kDaikin64Freq = kDaikin128Freq;  // Hz.
const uint8_t kDaikin64Overhead = 9;
const int8_t  kDaikin64ToleranceDelta = 5;  // +5%

const uint8_t kDaikin64ModeOffset = 8;
const uint8_t kDaikin64ModeSize = 4;  // Mask 0b111100000000
const uint8_t kDaikin64Dry =  0b001;
const uint8_t kDaikin64Cool = 0b010;
const uint8_t kDaikin64Fan =  0b100;
const uint8_t kDaikin64FanOffset = kDaikin64ModeOffset + kDaikin64ModeSize;
const uint8_t kDaikin64FanSize = 4;  // Mask 0b1111000000000000
const uint8_t kDaikin64FanAuto =  0b0001;
const uint8_t kDaikin64FanLow =   0b1000;
const uint8_t kDaikin64FanMed =   0b0100;
const uint8_t kDaikin64FanHigh =  0b0010;
const uint8_t kDaikin64FanQuiet = 0b1001;
const uint8_t kDaikin64FanTurbo = 0b0011;
const uint8_t kDaikin64ClockOffset = kDaikin64FanOffset + kDaikin64FanSize;
const uint8_t kDaikin64ClockMinsSize = 8;
const uint8_t kDaikin64ClockHoursSize = 8;
const uint8_t kDaikin64ClockSize = kDaikin64ClockMinsSize +
    kDaikin64ClockHoursSize;  // Mask 0b1111111111111111 << 15
const uint8_t kDaikin64OnTimeOffset = kDaikin64ClockOffset +
                                      kDaikin64ClockSize;
const uint8_t kDaikin64OnTimeSize = 6;
const uint8_t kDaikin64OnTimeHalfHourBit = kDaikin64OnTimeOffset +
                                           kDaikin64OnTimeSize;
const uint8_t kDaikin64OnTimeEnableBit = kDaikin64OnTimeHalfHourBit + 1;
const uint8_t kDaikin64OffTimeOffset = kDaikin64OnTimeEnableBit + 1;
const uint8_t kDaikin64OffTimeSize = 6;
const uint8_t kDaikin64OffTimeHalfHourBit = kDaikin64OffTimeOffset +
                                            kDaikin64OffTimeSize;
const uint8_t kDaikin64OffTimeEnableBit = kDaikin64OffTimeHalfHourBit + 1;
const uint8_t kDaikin64TempOffset = 48;
const uint8_t kDaikin64TempSize = 8;  // Mask 0b11111111 << 47
const uint8_t kDaikin64MinTemp = 16;  // Celsius
const uint8_t kDaikin64MaxTemp = 30;  // Celsius
const uint8_t kDaikin64SwingVBit = 56;
const uint8_t kDaikin64SleepBit = kDaikin64SwingVBit + 1;
const uint8_t kDaikin64PowerToggleBit = 59;
const uint8_t kDaikin64ChecksumOffset = 60;
const uint8_t kDaikin64ChecksumSize = 4;  // Mask 0b1111 << 59

void setTemp_Daikin280(const uint8_t temp)
{
  uint8_t degrees = (uint8_t)max(temp, kDaikinMinTemp);
  degrees = (uint8_t)min(degrees, kDaikinMaxTemp);
  data[kDaikinByteTemp] = degrees << 1;
}

void setPower_Daikin280(const bool on)
{
  setBit_Daikin280(&data[kDaikinBytePower], kDaikinBitPowerOffset, on);
}

void setBit_Daikin280(uint8_t * const data, const uint8_t position, const bool on)
{
	uint8_t mask = 1 << position;
	if (on)
	  *data |= mask;
	else
	  *data &= ~mask;
}

void setBits_Daikin280(uint8_t * const dst, const uint8_t offset, const uint8_t nbits,
               const uint8_t data) {
    if (offset >= 8 || !nbits) return;  // Short circuit as it won't change.
    // Calculate the mask for the supplied value.
    uint8_t mask = UINT8_MAX >> (8 - ((nbits > 8) ? 8 : nbits));
    // Calculate the mask & clear the space for the data.
    // Clear the destination bits.
    *dst &= ~(uint8_t)(mask << offset);
    // Merge in the data.
    *dst |= ((data & mask) << offset);
  }

void setFan_Daikin280(const uint8_t fan) {
  // Set the fan speed bits, leave low 4 bits alone
  uint8_t fanset;
  if (fan == kDaikinFanQuiet || fan == kDaikinFanAuto)
    fanset = fan;
  else if (fan < kDaikinFanMin || fan > kDaikinFanMax)
    fanset = kDaikinFanAuto;
  else
    fanset = 2 + fan;
  setBits_Daikin280(&data[kDaikinByteFan], kDaikinFanOffset, kDaikinFanSize, fanset);
}

void setMode_Daikin280(const uint8_t mode) {
  switch (mode) {
	case kDaikinAuto:
	case kDaikinCool:
	case kDaikinHeat:
	case kDaikinFan:
	case kDaikinDry:
	  setBits_Daikin280(&data[kDaikinBytePower], kDaikinModeOffset, kDaikinModeSize,
			  mode);
	  break;
	default:
	  setMode_Daikin280(kDaikinAuto);
  }
}

void setSwingVertical_Daikin280(const bool on) {
  setBits_Daikin280(&data[kDaikinByteFan], kDaikinSwingOffset, kDaikinSwingSize,
          on ? kDaikinSwingOn : kDaikinSwingOff);
}

void setSwingHorizontal_Daikin280(const bool on) {
  setBits_Daikin280(&data[kDaikinByteSwingH], kDaikinSwingOffset, kDaikinSwingSize,
          on ? kDaikinSwingOn : kDaikinSwingOff);
}

void setQuiet_Daikin280(const bool on) {
  setBit_Daikin280(&data[kDaikinByteSilent], kDaikinBitSilentOffset, on);
  // Powerful & Quiet mode being on are mutually exclusive.
  if (on) setPowerful_Daikin280(false);
}

void setPowerful_Daikin280(const bool on) {
  setBit_Daikin280(&data[kDaikinBytePowerful], kDaikinBitPowerfulOffset, on);
  if (on) {
	setQuiet_Daikin280(false);
	setEcono_Daikin280(false);
  }
}

void setEcono_Daikin280(const bool on) {
  setBit_Daikin280(&data[kDaikinByteEcono], kDaikinBitEconoOffset, on);
  if (on) setPowerful_Daikin280(false);
}

void setSensor_Daikin280(const bool on) {
  setBit_Daikin280(&data[kDaikinByteSensor], kDaikinBitSensorOffset, on);
}

void setMold_Daikin280(const bool on) {
  setBit_Daikin280(&data[kDaikinByteMold], kDaikinBitMoldOffset, on);
}

void setComfort_Daikin280(const bool on) {
  setBit_Daikin280(&data[kDaikinByteComfort], kDaikinBitComfortOffset, on);
}

//void enableOnTimer(const uint16_t starttime) {
//  setBit(&data[kDaikinByteOnTimer], kDaikinBitOnTimerOffset);
//  data[kDaikinByteOnTimerMinsLow] = starttime;
//  // only keep 4 bits
//  setBits(&data[kDaikinByteOnTimerMinsHigh], kDaikinOnTimerMinsHighOffset,
//		  kDaikinOnTimerMinsHighSize, starttime >> 8);
//}

void disableOnTimer_Daikin280(void) {
  enableOnTimer_Daikin280(kDaikinUnusedTime);
  setBit_Daikin280(&data[kDaikinByteOnTimer], kDaikinBitOnTimerOffset, false);
}

void sendDaikin280IRCommand(void *arg)
{
//	printf("IRObject.onTimePeriod : %d\r\n",IRObject.onTimePeriod);
//	printf("IRObject.dutycycle    : %d\r\n",IRObject._dutycycle);
//	configASSERT( ( ( uint32_t ) arg ) == 1 );
	uint16_t offset = 0;
//	for(uint8_t index=0; index<35; index++)
//	{
//		printf("data[%d] : %x\r\n",index,data[index]);
//	}
//	printf("\r\n");
	sendGenericmsgtime(0,0, kDaikinBitMark, kDaikinOneSpace, kDaikinBitMark, kDaikinZeroSpace,
			kDaikinBitMark, kDaikinZeroSpace + kDaikinGap, 0U, (uint64_t)0U, kDaikinHeaderLength,
			41, false, 0, IRObject._dutycycle);
	sendGeneric(kDaikinHdrMark, kDaikinHdrSpace, kDaikinBitMark,
				  kDaikinOneSpace, kDaikinBitMark, kDaikinZeroSpace,
				  kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
				  data, kDaikinSection1Length, 41, false, 0, IRObject._dutycycle);
	offset += kDaikinSection1Length;
	sendGeneric(kDaikinHdrMark, kDaikinHdrSpace, kDaikinBitMark,
				kDaikinOneSpace, kDaikinBitMark, kDaikinZeroSpace,
				kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
				data+offset, kDaikinSection2Length, 41, false, 0, IRObject._dutycycle);
	offset += kDaikinSection2Length;
	sendGeneric(kDaikinHdrMark, kDaikinHdrSpace, kDaikinBitMark,
				kDaikinOneSpace, kDaikinBitMark, kDaikinZeroSpace,
				kDaikinBitMark, kDaikinZeroSpace + kDaikinGap,
				data+offset, 35 - offset, 41, false, 0, IRObject._dutycycle);
}

uint8_t daikin280_sumBytes(const uint8_t * const start, const uint16_t length) {
  uint8_t checksum = 0;
  const uint8_t *ptr;
  for (ptr = start; ptr - start < length; ptr++) checksum += *ptr;
  return checksum;
}

void daikin_280_checksum()
{
	data[kDaikinByteChecksum1] = daikin280_sumBytes(data, kDaikinSection1Length - 1);
	data[kDaikinByteChecksum2] = daikin280_sumBytes(data + kDaikinSection1Length,
										  kDaikinSection2Length - 1);
    data[kDaikinByteChecksum3] = daikin280_sumBytes(data + kDaikinSection1Length +
										  kDaikinSection2Length,
										  kDaikinSection3Length - 1);
}

void data_init_Daikin280()
{
	for(uint8_t index=0; index<35; index++)
	{
		data[index] = 0x00;
	}
	data[0] = 0x11;
	data[1] = 0xda;
	data[2] = 0x27;
	data[4] = 0xc5;
	data[8] = 0x11;
	data[9] = 0xda;
	data[10] = 0x27;
	data[12] = 0x42;
	data[16] = 0x11;
	data[17] = 0xDA;
	data[18] = 0x27;
	data[21] = 0x49;
	data[22] = 0x1E;
	data[24] = 0xB0;
	data[27] = 0x06;
    data[28] = 0x60;
    data[31] = 0xC0;
    daikin_280_checksum();
}

void toggle_daikin()
{

	vTaskDelay(pdMS_TO_TICKS(1000));
	vTaskDelay(pdMS_TO_TICKS(1000));
}
