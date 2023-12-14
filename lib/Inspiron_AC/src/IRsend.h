#ifndef IRSEND_H_
#define IRSEND_H_

#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "IRremoteESP8266.h"

// sending IR code on ESP8266

#if TEST || UNIT_TEST
#define VIRTUAL virtual
#else
#define VIRTUAL
#endif

// Constants
// Offset (in microseconds) to use in Period time calculations to account for
// code excution time in producing the software PWM signal.
#if defined(ESP32)
// Calculated on a generic ESP-WROOM-32 board with v3.2-18 SDK @ 240MHz
const int8_t kPeriodOffset = -2;
#elif (defined(ESP8266) && F_CPU == 160000000L)  // NOLINT(whitespace/parens)
// Calculated on an ESP8266 NodeMCU v2 board using:
// v2.6.0 with v2.5.2 ESP core @ 160MHz
const int8_t kPeriodOffset = -2;
#else  // (defined(ESP8266) && F_CPU == 160000000L)
// Calculated on ESP8266 Wemos D1 mini using v2.4.1 with v2.4.0 ESP core @ 40MHz
const int8_t kPeriodOffset = -5;
#endif  // (defined(ESP8266) && F_CPU == 160000000L)
const uint8_t kDutyDefault = 50;  // Percentage
const uint8_t kDutyMax = 100;     // Percentage
// delayMicroseconds() is only accurate to 16383us.
const uint16_t kMaxAccurateUsecDelay = 16383;
//  Usecs to wait between messages we don't know the proper gap time.
const uint32_t kDefaultMessageGap = 100000;

namespace stdAc {
  enum class opmode_t {
    kOff  = -1,
    kAuto =  0,
    kCool =  1,
    kHeat =  2,
    kDry  =  3,
    kFan  =  4,
  };

  enum class fanspeed_t {
    kAuto =   0,
    kMin =    1,
    kLow =    2,
    kMedium = 3,
    kHigh =   4,
    kMax =    5,
  };

  enum class swingv_t {
    kOff =    -1,
    kAuto =    0,
    kHighest = 1,
    kHigh =    2,
    kMiddle =  3,
    kLow =     4,
    kLowest =  5,
  };

  enum class swingh_t {
    kOff =     -1,
    kAuto =     0,  // a.k.a. On.
    kLeftMax =  1,
    kLeft =     2,
    kMiddle =   3,
    kRight =    4,
    kRightMax = 5,
    kWide =     6,
  };

  // Structure to hold a common A/C state.
  typedef struct {
    decode_type_t protocol;
    int16_t model;
    bool power;
    stdAc::opmode_t mode;
    float degrees;
    bool celsius;
    stdAc::fanspeed_t fanspeed;
    stdAc::swingv_t swingv;
    stdAc::swingh_t swingh;
    bool quiet;
    bool turbo;
    bool econo;
    bool light;
    bool filter;
    bool clean;
    bool beep;
    int16_t sleep;
    int16_t clock;
  } state_t;
};  // namespace stdAc

// Classes
class IRsend {
 public:
  explicit IRsend(uint16_t IRsendPin, bool inverted = false,
                  bool use_modulation = true);
  void begin();
  void enableIROut(uint32_t freq, uint8_t duty = kDutyDefault);
  VIRTUAL void _delayMicroseconds(uint32_t usec);
  VIRTUAL uint16_t mark(uint16_t usec);
  VIRTUAL void space(uint32_t usec);
  int8_t calibrate(uint16_t hz = 38000U);
  void sendRaw(uint16_t buf[], uint16_t len, uint16_t hz);
  void sendData(uint16_t onemark, uint32_t onespace, uint16_t zeromark,
                uint32_t zerospace, uint64_t data, uint16_t nbits,
                bool MSBfirst = true);
  void sendGeneric(const uint16_t headermark, const uint32_t headerspace,
                   const uint16_t onemark, const uint32_t onespace,
                   const uint16_t zeromark, const uint32_t zerospace,
                   const uint16_t footermark, const uint32_t gap,
                   const uint64_t data, const uint16_t nbits,
                   const uint16_t frequency, const bool MSBfirst,
                   const uint16_t repeat, const uint8_t dutycycle);
  void sendGeneric(const uint16_t headermark, const uint32_t headerspace,
                   const uint16_t onemark, const uint32_t onespace,
                   const uint16_t zeromark, const uint32_t zerospace,
                   const uint16_t footermark, const uint32_t gap,
                   const uint32_t mesgtime, const uint64_t data,
                   const uint16_t nbits, const uint16_t frequency,
                   const bool MSBfirst, const uint16_t repeat,
                   const uint8_t dutycycle);
  void sendGeneric(const uint16_t headermark, const uint32_t headerspace,
                   const uint16_t onemark, const uint32_t onespace,
                   const uint16_t zeromark, const uint32_t zerospace,
                   const uint16_t footermark, const uint32_t gap,
                   const uint8_t *dataptr, const uint16_t nbytes,
                   const uint16_t frequency, const bool MSBfirst,
                   const uint16_t repeat, const uint8_t dutycycle);
  static uint16_t minRepeats(const decode_type_t protocol);
  static uint16_t defaultBits(const decode_type_t protocol);
  bool send(const decode_type_t type, const uint64_t data,
            const uint16_t nbits, const uint16_t repeat = kNoRepeat);
  bool send(const decode_type_t type, const uint8_t state[],
            const uint16_t nbytes);
#if SEND_SAMSUNG
  void sendSAMSUNG(const uint64_t data, const uint16_t nbits = kSamsungBits,
                   const uint16_t repeat = kNoRepeat);
  uint32_t encodeSAMSUNG(const uint8_t customer, const uint8_t command);
#endif
#if SEND_SAMSUNG36
  void sendSamsung36(const uint64_t data, const uint16_t nbits = kSamsung36Bits,
                     const uint16_t repeat = kNoRepeat);
#endif
#if SEND_SAMSUNG_AC
  void sendSamsungAC(const unsigned char data[],
                     const uint16_t nbytes = kSamsungAcStateLength,
                     const uint16_t repeat = kSamsungAcDefaultRepeat);
#endif
#if (SEND_PANASONIC )
  void sendPanasonic64(const uint64_t data,
                       const uint16_t nbits = kPanasonicBits,
                       const uint16_t repeat = kNoRepeat);
  void sendPanasonic(const uint16_t address, const uint32_t data,
                     const uint16_t nbits = kPanasonicBits,
                     const uint16_t repeat = kNoRepeat);
  uint64_t encodePanasonic(const uint16_t manufacturer, const uint8_t device,
                           const uint8_t subdevice, const uint8_t function);
#endif
#if SEND_COOLIX
  void sendCOOLIX(uint64_t data, uint16_t nbits = kCoolixBits,
                  uint16_t repeat = kCoolixDefaultRepeat);
#endif

#if SEND_TOSHIBA_AC
  void sendToshibaAC(const unsigned char data[],
                     const uint16_t nbytes = kToshibaACStateLength,
                     const uint16_t repeat = kToshibaACMinRepeat);
#endif
#if (SEND_HAIER_AC || SEND_HAIER_AC_YRW02)
  void sendHaierAC(const unsigned char data[],
                   const uint16_t nbytes = kHaierACStateLength,
                   const uint16_t repeat = kHaierAcDefaultRepeat);
#endif
#if SEND_HAIER_AC_YRW02
  void sendHaierACYRW02(const unsigned char data[],
                        const uint16_t nbytes = kHaierACYRW02StateLength,
                        const uint16_t repeat = kHaierAcYrw02DefaultRepeat);
#endif
#if SEND_WHIRLPOOL_AC
  void sendWhirlpoolAC(const unsigned char data[],
                       const uint16_t nbytes = kWhirlpoolAcStateLength,
                       const uint16_t repeat = kWhirlpoolAcDefaultRepeat);
#endif
#if SEND_PANASONIC_AC
  void sendPanasonicAC(const unsigned char data[],
                       const uint16_t nbytes = kPanasonicAcStateLength,
                       const uint16_t repeat = kPanasonicAcDefaultRepeat);
#endif
#if SEND_TCL112AC
  void sendTcl112Ac(const unsigned char data[],
                    const uint16_t nbytes = kTcl112AcStateLength,
                    const uint16_t repeat = kTcl112AcDefaultRepeat);
#endif
 protected:
#ifdef UNIT_TEST
#ifndef HIGH
#define HIGH 0x1
#endif
#ifndef LOW
#define LOW 0x0
#endif
#endif  // UNIT_TEST
  uint8_t outputOn;
  uint8_t outputOff;
  VIRTUAL void ledOff();
  VIRTUAL void ledOn();
#ifndef UNIT_TEST

 private:
#else
  uint32_t _freq_unittest;
#endif  // UNIT_TEST
  uint16_t onTimePeriod;
  uint16_t offTimePeriod;
  uint16_t IRpin;
  int8_t periodOffset;
  uint8_t _dutycycle;
  bool modulation;
  uint32_t calcUSecPeriod(uint32_t hz, bool use_offset = true);
};

#endif  // IRSEND_H_
