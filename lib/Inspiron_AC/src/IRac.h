#ifndef IRAC_H_
#define IRAC_H_

#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include "IRremoteESP8266.h"
#include "ir_Coolix.h"
#include "ir_Haier.h"
#include "ir_Panasonic.h"
#include "ir_Samsung.h"
#include "ir_Tcl.h"
#include "ir_Toshiba.h"
#include "ir_Whirlpool.h"

// Constants
const int8_t kGpioUnused = -1;

// Class
class IRac {
 public:
  explicit IRac(const uint16_t pin, const bool inverted = false,
                const bool use_modulation = true);
  static bool isProtocolSupported(const decode_type_t protocol);
  bool sendAc(const decode_type_t vendor, const int16_t model,
              const bool power, const stdAc::opmode_t mode, const float degrees,
              const bool celsius, const stdAc::fanspeed_t fan,
              const stdAc::swingv_t swingv, const stdAc::swingh_t swingh,
              const bool quiet, const bool turbo, const bool econo,
              const bool light, const bool filter, const bool clean,
              const bool beep, const int16_t sleep = -1,
              const int16_t clock = -1);
  bool sendAc(const stdAc::state_t desired, const stdAc::state_t *prev = NULL);
  static bool cmpStates(const stdAc::state_t a, const stdAc::state_t b);
  static bool strToBool(const char *str, const bool def = false);
  static int16_t strToModel(const char *str, const int16_t def = -1);
  static stdAc::opmode_t strToOpmode(
      const char *str, const stdAc::opmode_t def = stdAc::opmode_t::kAuto);
  static stdAc::fanspeed_t strToFanspeed(
      const char *str,
      const stdAc::fanspeed_t def = stdAc::fanspeed_t::kAuto);
  static stdAc::swingv_t strToSwingV(
      const char *str, const stdAc::swingv_t def = stdAc::swingv_t::kOff);
  static stdAc::swingh_t strToSwingH(
      const char *str, const stdAc::swingh_t def = stdAc::swingh_t::kOff);
  static String boolToString(const bool value);
  static String opmodeToString(const stdAc::opmode_t mode);
  static String fanspeedToString(const stdAc::fanspeed_t speed);
  static String swingvToString(const stdAc::swingv_t swingv);
  static String swinghToString(const stdAc::swingh_t swingh);
#ifndef UNIT_TEST

 private:
#endif
  uint16_t _pin;
  bool _inverted;
  bool _modulation;
#if SEND_COOLIX
  void coolix(IRCoolixAC *ac,
              const bool on, const stdAc::opmode_t mode, const float degrees,
              const stdAc::fanspeed_t fan,
              const stdAc::swingv_t swingv, const stdAc::swingh_t swingh,
              const bool turbo, const bool light, const bool clean,
              const int16_t sleep = -1);
#endif  // SEND_COOLIX

#if SEND_HAIER_AC
  void haier(IRHaierAC *ac,
             const bool on, const stdAc::opmode_t mode, const float degrees,
             const stdAc::fanspeed_t fan, const stdAc::swingv_t swingv,
             const bool filter, const int16_t sleep = -1,
             const int16_t clock = -1);
#endif  // SEND_HAIER_AC
#if SEND_HAIER_AC_YRW02
  void haierYrwo2(IRHaierACYRW02 *ac,
                  const bool on, const stdAc::opmode_t mode,
                  const float degrees, const stdAc::fanspeed_t fan,
                  const stdAc::swingv_t swingv,
                  const bool turbo, const bool filter,
                  const int16_t sleep = -1);
#endif  // SEND_HAIER_AC_YRW02
#if SEND_PANASONIC_AC
  void panasonic(IRPanasonicAc *ac, const panasonic_ac_remote_model_t model,
                 const bool on, const stdAc::opmode_t mode, const float degrees,
                 const stdAc::fanspeed_t fan,
                 const stdAc::swingv_t swingv, const stdAc::swingh_t swingh,
                 const bool quiet, const bool turbo, const int16_t clock = -1);
#endif  // SEND_PANASONIC_AC
#if SEND_SAMSUNG_AC
  void samsung(IRSamsungAc *ac,
               const bool on, const stdAc::opmode_t mode, const float degrees,
               const stdAc::fanspeed_t fan, const stdAc::swingv_t swingv,
               const bool quiet, const bool turbo, const bool clean,
               const bool beep, const bool dopower = true);
#endif  // SEND_SAMSUNG_AC
#if SEND_TCL112AC
  void tcl112(IRTcl112Ac *ac,
              const bool on, const stdAc::opmode_t mode, const float degrees,
              const stdAc::fanspeed_t fan,
              const stdAc::swingv_t swingv, const stdAc::swingh_t swingh,
              const bool turbo, const bool light, const bool econo,
              const bool filter);
#endif  // SEND_TCL112AC
#if SEND_TOSHIBA_AC
  void toshiba(IRToshibaAC *ac,
               const bool on, const stdAc::opmode_t mode, const float degrees,
               const stdAc::fanspeed_t fan);
#endif  // SEND_TOSHIBA_AC
#if SEND_WHIRLPOOL_AC
  void whirlpool(IRWhirlpoolAc *ac, const whirlpool_ac_remote_model_t model,
                 const bool on, const stdAc::opmode_t mode, const float degrees,
                 const stdAc::fanspeed_t fan, const stdAc::swingv_t swingv,
                 const bool turbo, const bool light,
                 const int16_t sleep = -1, const int16_t clock = -1);
#endif  // SEND_WHIRLPOOL_AC
static stdAc::state_t handleToggles(const stdAc::state_t desired,
                                    const stdAc::state_t *prev = NULL);
};  // IRac class

namespace IRAcUtils {
  String resultAcToString(const decode_results * const results);
  bool decodeToState(const decode_results *decode, stdAc::state_t *result,
                     const stdAc::state_t *prev = NULL);
}  // namespace IRAcUtils
#endif  // IRAC_H_
