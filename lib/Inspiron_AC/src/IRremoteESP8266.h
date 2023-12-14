#ifndef IRREMOTEESP8266_H_
#define IRREMOTEESP8266_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#ifdef UNIT_TEST
#include <iostream>
#include <string>
#endif  // UNIT_TEST

// Library Version
#define _IRREMOTEESP8266_VERSION_ "2.6.4"
// Supported IR protocols
// Each protocol you include costs memory and, during decode, costs time
// Disable (set to false) all the protocols you do not need/want!
// The Air Conditioner protocols are the most expensive memory-wise.
//
#define SEND_RAW               true

#define DECODE_PANASONIC       true
#define SEND_PANASONIC         true

#define DECODE_SAMSUNG         true
#define SEND_SAMSUNG           true

#define DECODE_SAMSUNG36       true
#define SEND_SAMSUNG36         true

#define DECODE_SAMSUNG_AC      true
#define SEND_SAMSUNG_AC        true

#define DECODE_COOLIX          true
#define SEND_COOLIX            true

#define DECODE_TOSHIBA_AC      true
#define SEND_TOSHIBA_AC        true

#define DECODE_HAIER_AC        true
#define SEND_HAIER_AC          true

#define DECODE_HAIER_AC_YRW02  true
#define SEND_HAIER_AC_YRW02    true

#define DECODE_WHIRLPOOL_AC    true
#define SEND_WHIRLPOOL_AC      true

#define DECODE_MITSUBISHI112   true
#define SEND_MITSUBISHI112     true

#define DECODE_PANASONIC_AC    true
#define SEND_PANASONIC_AC      true

#define DECODE_TCL112AC        true
#define SEND_TCL112AC          true

#if  ( DECODE_TOSHIBA_AC || DECODE_HAIER_AC || DECODE_HAIER_AC_YRW02 || \
     DECODE_WHIRLPOOL_AC || DECODE_SAMSUNG_AC ||DECODE_PANASONIC_AC || \
     DECODE_TCL112AC || DECODE_MITSUBISHI112 )
#define DECODE_AC true  // We need some common infrastructure for decoding A/Cs.
#else
#define DECODE_AC false   // We don't need that infrastructure.
#endif

// Use millisecond 'delay()' calls where we can to avoid tripping the WDT.

#define ALLOW_DELAY_CALLS true


 /* Always add to the end of the list and should never remove entries
 * or change order. Projects may save the type number for later usage
 * so numbering should always stay the same.*/

enum decode_type_t {
  UNKNOWN = -1,
  UNUSED = 0,
  PANASONIC,  
  SAMSUNG,
  COOLIX,  
  TOSHIBA_AC,
  HAIER_AC,
  HAIER_AC_YRW02,
  WHIRLPOOL_AC,  
  SAMSUNG_AC,
  PANASONIC_AC,
  RAW,
  SAMSUNG36,
  TCL112AC,
  MITSUBISHI112,
};
 
// Message lengths & required repeat values
const uint16_t kNoRepeat = 0;
const uint16_t kSingleRepeat = 1;
const uint16_t kCoolixBits = 24;
const uint16_t kCoolixDefaultRepeat = kSingleRepeat;
const uint16_t kCarrierAcBits = 32;
const uint16_t kCarrierAcMinRepeat = kNoRepeat;
const uint16_t kHaierACStateLength = 9;
const uint16_t kHaierACBits = kHaierACStateLength * 8;
const uint16_t kHaierAcDefaultRepeat = kNoRepeat;
const uint16_t kHaierACYRW02StateLength = 14;
const uint16_t kHaierACYRW02Bits = kHaierACYRW02StateLength * 8;
const uint16_t kHaierAcYrw02DefaultRepeat = kNoRepeat;
const uint16_t kPanasonicBits = 48;
const uint32_t kPanasonicManufacturer = 0x4004;
const uint16_t kPanasonicAcStateLength = 27;
const uint16_t kPanasonicAcStateShortLength = 16;
const uint16_t kPanasonicAcBits = kPanasonicAcStateLength * 8;
const uint16_t kPanasonicAcShortBits = kPanasonicAcStateShortLength * 8;
const uint16_t kPanasonicAcDefaultRepeat = kNoRepeat;
const uint16_t kSamsungBits = 32;
const uint16_t kSamsung36Bits = 36;
const uint16_t kSamsungAcStateLength = 14;
const uint16_t kSamsungAcBits = kSamsungAcStateLength * 8;
const uint16_t kSamsungAcExtendedStateLength = 21;
const uint16_t kSamsungAcExtendedBits = kSamsungAcExtendedStateLength * 8;
const uint16_t kSamsungAcDefaultRepeat = kNoRepeat;
const uint16_t kTcl112AcStateLength = 14;
const uint16_t kTcl112AcBits = kTcl112AcStateLength * 8;
const uint16_t kTcl112AcDefaultRepeat = kNoRepeat;
const uint16_t kTecoBits = 35;
const uint16_t kTecoDefaultRepeat = kNoRepeat;
const uint16_t kToshibaACStateLength = 9;
const uint16_t kToshibaACBits = kToshibaACStateLength * 8;
const uint16_t kToshibaACMinRepeat = kSingleRepeat;
const uint16_t kWhirlpoolAcStateLength = 21;
const uint16_t kWhirlpoolAcBits = kWhirlpoolAcStateLength * 8;
const uint16_t kWhirlpoolAcDefaultRepeat = kNoRepeat;
const uint16_t kMitsubishi112StateLength = 14;
const uint16_t kMitsubishi112Bits = kMitsubishi112StateLength * 8;
const uint16_t kMitsubishi112MinRepeat = kNoRepeat;

// Legacy defines. (Deprecated)
#define COOLIX_BITS                   kCoolixBits
#define HAIER_AC_STATE_LENGTH         kHaierACStateLength
#define HAIER_AC_YRW02_STATE_LENGTH   kHaierACYRW02StateLength
#define PANASONIC_BITS                kPanasonicBits
#define SAMSUNG_BITS                  kSamsungBits

// Turn on Debugging information by uncommenting the following line.
// #define DEBUG 1

#ifdef DEBUG
#ifdef UNIT_TEST
#define DPRINT(x) do { std::cout << x; } while (0)
#define DPRINTLN(x) do { std::cout << x << std::endl; } while (0)
#endif  // UNIT_TEST
#ifdef ARDUINO
#define DPRINT(x) do { Serial.print(x); } while (0)
#define DPRINTLN(x) do { Serial.println(x); } while (0)
#endif  // ARDUINO
#else  // DEBUG
#define DPRINT(x)
#define DPRINTLN(x)
#endif  // DEBUG

#ifdef UNIT_TEST
#ifndef F
// Create a no-op F() macro so the code base still compiles outside of the
// Arduino framework. Thus we can safely use the Arduino 'F()' macro through-out
#define F(x) x
#endif  // F
typedef std::string String;
#endif  // UNIT_TEST

#endif  // IRREMOTEESP8266_H_