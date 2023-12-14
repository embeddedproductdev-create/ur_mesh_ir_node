#include "IRac.h"
#ifndef UNIT_TEST
#include <Arduino.h>
#endif
#include <string.h>
#ifndef ARDUINO
#include <string>
#endif
#include "IRsend.h"
#include "IRremoteESP8266.h"
#include "IRutils.h"
#include "ir_Coolix.h"
#include "ir_Haier.h"
#include "ir_Panasonic.h"
#include "ir_Samsung.h"
#include "ir_Tcl.h"
#include "ir_Toshiba.h"
#include "ir_Whirlpool.h"

IRac::IRac(const uint16_t pin, const bool inverted, const bool use_modulation) {
  _pin = pin;
  _inverted = inverted;
  _modulation = use_modulation;
}

// Is the given protocol supported by the IRac class?
bool IRac::isProtocolSupported(const decode_type_t protocol) {
  switch (protocol) {
#if SEND_COOLIX
    case decode_type_t::COOLIX:
#endif
#if SEND_HAIER_AC
    case decode_type_t::HAIER_AC:
#endif
#if SEND_HAIER_AC_YRW02
    case decode_type_t::HAIER_AC_YRW02:
#endif
#if SEND_NEOCLIMA
    case decode_type_t::NEOCLIMA:
#endif
#if SEND_PANASONIC_AC
    case decode_type_t::PANASONIC_AC:
#endif
#if SEND_SAMSUNG_AC
    case decode_type_t::SAMSUNG_AC:
#endif
#if SEND_TCL112AC
    case decode_type_t::TCL112AC:
#endif
#if SEND_TOSHIBA_AC
    case decode_type_t::TOSHIBA_AC:
#endif
#if SEND_WHIRLPOOL_AC
    case decode_type_t::WHIRLPOOL_AC:
#endif
      return true;
    default:
      return false;
  }
}
#if SEND_COOLIX
void IRac::coolix(IRCoolixAC *ac,
                  const bool on, const stdAc::opmode_t mode,
                  const float degrees, const stdAc::fanspeed_t fan,
                  const stdAc::swingv_t swingv, const stdAc::swingh_t swingh,
                  const bool turbo, const bool light, const bool clean,
                  const int16_t sleep) {
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  // No Filter setting available.
  // No Beep setting available.
  // No Clock setting available.
  // No Econo setting available.
  // No Quiet setting available.
  if (swingv != stdAc::swingv_t::kOff || swingh != stdAc::swingh_t::kOff) {
    // Swing has a special command that needs to be sent independently.
    ac->setSwing();
    ac->send();
  }
  if (turbo) {
    // Turbo has a special command that needs to be sent independently.
    ac->setTurbo();
    ac->send();
  }
  if (sleep > 0) {
    // Sleep has a special command that needs to be sent independently.
    ac->setSleep();
    ac->send();
  }
  if (light) {
    // Light has a special command that needs to be sent independently.
    ac->setLed();
    ac->send();
  }
  if (clean) {
    // Clean has a special command that needs to be sent independently.
    ac->setClean();
    ac->send();
  }
  // Power gets done last, as off has a special command.
  ac->setPower(on);
  ac->send();
}
#endif  // SEND_COOLIX
#if SEND_HAIER_AC
void IRac::haier(IRHaierAC *ac,
                 const bool on, const stdAc::opmode_t mode, const float degrees,
                 const stdAc::fanspeed_t fan, const stdAc::swingv_t swingv,
                 const bool filter, const int16_t sleep, const int16_t clock) {
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwing(ac->convertSwingV(swingv));
  // No Horizontal Swing setting available.
  // No Quiet setting available.
  // No Turbo setting available.
  // No Light setting available.
  ac->setHealth(filter);
  // No Clean setting available.
  // No Beep setting available.
  ac->setSleep(sleep >= 0);  // Sleep on this A/C is either on or off.
  if (clock >=0) ac->setCurrTime(clock);
  if (on)
    ac->setCommand(kHaierAcCmdOn);
  else
    ac->setCommand(kHaierAcCmdOff);
  ac->send();
}
#endif  // SEND_HAIER_AC

#if SEND_HAIER_AC_YRW02
void IRac::haierYrwo2(IRHaierACYRW02 *ac,
                      const bool on, const stdAc::opmode_t mode,
                      const float degrees, const stdAc::fanspeed_t fan,
                      const stdAc::swingv_t swingv, const bool turbo,
                      const bool filter, const int16_t sleep) {
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwing(ac->convertSwingV(swingv));
  // No Horizontal Swing setting available.
  // No Quiet setting available.
  ac->setTurbo(turbo);
  // No Light setting available.
  ac->setHealth(filter);
  // No Clean setting available.
  // No Beep setting available.
  ac->setSleep(sleep >= 0);  // Sleep on this A/C is either on or off.
  ac->setPower(on);
  ac->send();
}
#endif  // SEND_HAIER_AC_YRW02
#if SEND_PANASONIC_AC
void IRac::panasonic(IRPanasonicAc *ac, const panasonic_ac_remote_model_t model,
                     const bool on, const stdAc::opmode_t mode,
                     const float degrees, const stdAc::fanspeed_t fan,
                     const stdAc::swingv_t swingv, const stdAc::swingh_t swingh,
                     const bool quiet, const bool turbo, const int16_t clock) {
  ac->setModel(model);
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwingVertical(ac->convertSwingV(swingv));
  ac->setSwingHorizontal(ac->convertSwingH(swingh));
  ac->setQuiet(quiet);
  ac->setPowerful(turbo);
  // No Light setting available.
  // No Econo setting available.
  // No Filter setting available.
  // No Clean setting available.
  // No Beep setting available.
  // No Sleep setting available.
  if (clock >= 0) ac->setClock(clock);
  ac->send();
}
#endif  // SEND_PANASONIC_AC

#if SEND_SAMSUNG_AC
void IRac::samsung(IRSamsungAc *ac,
                   const bool on, const stdAc::opmode_t mode,
                   const float degrees,
                   const stdAc::fanspeed_t fan, const stdAc::swingv_t swingv,
                   const bool quiet, const bool turbo, const bool clean,
                   const bool beep, const bool dopower) {
  // dopower is for unit testing only. It should only ever be false in tests.
  if (dopower) ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwing(swingv != stdAc::swingv_t::kOff);
  // No Horizontal swing setting available.
  ac->setQuiet(quiet);
  ac->setPowerful(turbo);
  // No Light setting available.
  // No Econo setting available.
  // No Filter setting available.
  ac->setClean(clean);
  ac->setBeep(beep);
  // No Sleep setting available.
  // No Clock setting available.
  // Do setMode() again as it can affect fan speed.
  ac->setMode(ac->convertMode(mode));
  ac->send();
}
#endif  // SEND_SAMSUNG_AC
#if SEND_TCL112AC
void IRac::tcl112(IRTcl112Ac *ac,
                  const bool on, const stdAc::opmode_t mode,
                  const float degrees, const stdAc::fanspeed_t fan,
                  const stdAc::swingv_t swingv, const stdAc::swingh_t swingh,
                  const bool turbo, const bool light, const bool econo,
                  const bool filter) {
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwingVertical(swingv != stdAc::swingv_t::kOff);
  ac->setSwingHorizontal(swingh != stdAc::swingh_t::kOff);
  // No Quiet setting available.
  ac->setTurbo(turbo);
  ac->setLight(light);
  ac->setEcono(econo);
  ac->setHealth(filter);
  // No Clean setting available.
  // No Beep setting available.
  // No Sleep setting available.
  // No Clock setting available.
  ac->send();
}
#endif  // SEND_TCL112AC

#if SEND_TOSHIBA_AC
void IRac::toshiba(IRToshibaAC *ac,
                   const bool on, const stdAc::opmode_t mode,
                   const float degrees, const stdAc::fanspeed_t fan) {
  ac->setPower(on);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  // No Vertical swing setting available.
  // No Horizontal swing setting available.
  // No Quiet setting available.
  // No Turbo setting available.
  // No Light setting available.
  // No Filter setting available.
  // No Clean setting available.
  // No Beep setting available.
  // No Sleep setting available.
  // No Clock setting available.
  ac->send();
}
#endif  // SEND_TOSHIBA_AC
#if SEND_WHIRLPOOL_AC
void IRac::whirlpool(IRWhirlpoolAc *ac, const whirlpool_ac_remote_model_t model,
                     const bool on, const stdAc::opmode_t mode,
                     const float degrees,
                     const stdAc::fanspeed_t fan, const stdAc::swingv_t swingv,
                     const bool turbo, const bool light,
                     const int16_t sleep, const int16_t clock) {
  ac->setModel(model);
  ac->setMode(ac->convertMode(mode));
  ac->setTemp(degrees);
  ac->setFan(ac->convertFan(fan));
  ac->setSwing(swingv != stdAc::swingv_t::kOff);
  // No Horizontal swing setting available.
  // No Quiet setting available.
  ac->setSuper(turbo);
  ac->setLight(light);
  // No Filter setting available
  // No Clean setting available.
  // No Beep setting available.
  ac->setSleep(sleep >= 0);  // Sleep is either on/off, so convert to boolean.
  if (clock >= 0) ac->setClock(clock);
  ac->setPowerToggle(on);
  ac->send();
}
#endif  // SEND_WHIRLPOOL_AC

// Create a new state base on desired & previous states but handle
// any state changes for options that need to be toggled.
// Args:
//   desired: The state_t structure describing the desired a/c state.
//   prev:    Ptr to the previous state_t structure.
//
// Returns:
//   A stdAc::state_t with the needed settings.
stdAc::state_t IRac::handleToggles(const stdAc::state_t desired,
                                   const stdAc::state_t *prev) {
  stdAc::state_t result = desired;
  // If we've been given a previous state AND the it's the same A/C basically.
  if (prev != NULL && desired.protocol == prev->protocol &&
      desired.model == prev->model) {
    // Check if we have to handle toggle settings for specific A/C protocols.
    switch (desired.protocol) {
      case decode_type_t::COOLIX:
        if ((desired.swingv == stdAc::swingv_t::kOff) ^
            (prev->swingv == stdAc::swingv_t::kOff))  // It changed, so toggle.
          result.swingv = stdAc::swingv_t::kAuto;
        else
          result.swingv = stdAc::swingv_t::kOff;  // No change, so no toggle.
        result.turbo = desired.turbo ^ prev->turbo;
        result.light = desired.light ^ prev->light;
        result.clean = desired.clean ^ prev->clean;
        result.sleep = ((desired.sleep >= 0) ^ (prev->sleep >= 0)) ? 0 : -1;
        break;
      /*case decode_type_t::DAIKIN128:
        result.power = desired.power ^ prev->power;
        result.light = desired.light ^ prev->light;
        break;
      case decode_type_t::MIDEA:
        if ((desired.swingv == stdAc::swingv_t::kOff) ^
            (prev->swingv == stdAc::swingv_t::kOff))  // It changed, so toggle.
          result.swingv = stdAc::swingv_t::kAuto;
        else
          result.swingv = stdAc::swingv_t::kOff;  // No change, so no toggle.
        break;*/
      case decode_type_t::WHIRLPOOL_AC:
        result.power = desired.power ^ prev->power;
        break;
      case decode_type_t::PANASONIC_AC:
        // CKP models use a power mode toggle.
        if (desired.model == panasonic_ac_remote_model_t::kPanasonicCkp)
          result.power = desired.power ^ prev->power;
        break;
      default:
        {};
    }
  }
  return result;
}

// Send A/C message for a given device using common A/C settings.
// Args:
//   vendor:  The type of A/C protocol to use.
//   model:   The specific model of A/C if supported/applicable.
//   on:      Should the unit be powered on? (or in some cases, toggled)
//   mode:    What operating mode should the unit perform? e.g. Cool, Heat etc.
//   degrees: What temperature should the unit be set to?
//   celsius: Use degrees Celsius, otherwise Fahrenheit.
//   fan:     Fan speed.
// The following args are all "if supported" by the underlying A/C classes.
//   swingv:  Control the vertical swing of the vanes.
//   swingh:  Control the horizontal swing of the vanes.
//   quiet:   Set the unit to quiet (fan) operation mode.
//   turbo:   Set the unit to turbo operating mode. e.g. Max fan & cooling etc.
//   econo:   Set the unit to economical operating mode.
//   light:   Turn on the display/LEDs etc.
//   filter:  Turn on any particle/ion/allergy filter etc.
//   clean:   Turn on any settings to reduce mold etc. (Not self-clean mode.)
//   beep:    Control if the unit beeps upon receiving commands.
//   sleep:   Nr. of mins of sleep mode, or use sleep mode. (< 0 means off.)
//   clock:   Nr. of mins past midnight to set the clock to. (< 0 means off.)
// Returns:
//   boolean: True, if accepted/converted/attempted. False, if unsupported.
bool IRac::sendAc(const decode_type_t vendor, const int16_t model,
                  const bool power, const stdAc::opmode_t mode,
                  const float degrees, const bool celsius,
                  const stdAc::fanspeed_t fan,
                  const stdAc::swingv_t swingv, const stdAc::swingh_t swingh,
                  const bool quiet, const bool turbo, const bool econo,
                  const bool light, const bool filter, const bool clean,
                  const bool beep, const int16_t sleep, const int16_t clock) {
  // Convert the temperature to Celsius.
  float degC;
  if (celsius)
    degC = degrees;
  else
    degC = fahrenheitToCelsius(degrees);
  bool on = power;
  // A hack for Home Assistant, it appears to need/want an Off opmode.
  if (mode == stdAc::opmode_t::kOff) on = false;
  // Per vendor settings & setup.
  switch (vendor) {
#if SEND_COOLIX
    case COOLIX:
    {
      IRCoolixAC ac(_pin, _inverted, _modulation);
      coolix(&ac, on, mode, degC, fan, swingv, swingh,
             turbo, light, clean, sleep);
      break;
    }
#endif  // SEND_COOLIX

#if SEND_HAIER_AC
    case HAIER_AC:
    {
      IRHaierAC ac(_pin, _inverted, _modulation);
      ac.begin();
      haier(&ac, on, mode, degC, fan, swingv, filter, sleep, clock);
      break;
    }
#endif  // SEND_HAIER_AC
#if SEND_HAIER_AC_YRW02
    case HAIER_AC_YRW02:
    {
      IRHaierACYRW02 ac(_pin, _inverted, _modulation);
      ac.begin();
      haierYrwo2(&ac, on, mode, degC, fan, swingv, turbo, filter, sleep);
      break;
    }
#endif  // SEND_HAIER_AC_YRW02

#if SEND_PANASONIC_AC
    case PANASONIC_AC:
    {
      IRPanasonicAc ac(_pin, _inverted, _modulation);
      ac.begin();
      panasonic(&ac, (panasonic_ac_remote_model_t)model, on, mode, degC, fan,
                swingv, swingh, quiet, turbo, clock);
      break;
    }
#endif  // SEND_PANASONIC_AC
#if SEND_SAMSUNG_AC
    case SAMSUNG_AC:
    {
      IRSamsungAc ac(_pin, _inverted, _modulation);
      ac.begin();
      samsung(&ac, on, mode, degC, fan, swingv, quiet, turbo, clean, beep);
      break;
    }
#endif  // SEND_SAMSUNG_AC
#if SEND_TCL112AC
    case TCL112AC:
    {
      IRTcl112Ac ac(_pin, _inverted, _modulation);
      ac.begin();
      tcl112(&ac, on, mode, degC, fan, swingv, swingh, turbo, light, econo,
             filter);
      break;
    }
#endif  // SEND_TCL112AC
#if SEND_TOSHIBA_AC
    case TOSHIBA_AC:
    {
      IRToshibaAC ac(_pin, _inverted, _modulation);
      ac.begin();
      toshiba(&ac, on, mode, degC, fan);
      break;
    }
#endif  // SEND_TOSHIBA_AC
#if SEND_WHIRLPOOL_AC
    case WHIRLPOOL_AC:
    {
      IRWhirlpoolAc ac(_pin, _inverted, _modulation);
      ac.begin();
      whirlpool(&ac, (whirlpool_ac_remote_model_t)model, on, mode, degC, fan,
                swingv, turbo, light, sleep, clock);
      break;
    }
#endif  // SEND_WHIRLPOOL_AC
    default:
      return false;  // Fail, didn't match anything.
  }
  return true;  // Success.
}

// Send A/C message for a given device using state_t structures.
// Args:
//   desired: The state_t structure describing the desired new a/c state.
//   prev:    Ptr to the previous state_t structure.
//
// Returns:
//   boolean: True, if accepted/converted/attempted. False, if unsupported.
bool IRac::sendAc(const stdAc::state_t desired, const stdAc::state_t *prev) {
  stdAc::state_t final = this->handleToggles(desired, prev);
  return this->sendAc(final.protocol, final.model, final.power, final.mode,
                      final.degrees, final.celsius, final.fanspeed,
                      final.swingv, final.swingh, final.quiet, final.turbo,
                      final.econo, final.light, final.filter, final.clean,
                      final.beep, final.sleep, final.clock);
}

// Compare two AirCon states.
// Returns: True if they differ, False if they don't.
// Note: Excludes clock.
bool IRac::cmpStates(const stdAc::state_t a, const stdAc::state_t b) {
  return a.protocol != b.protocol || a.model != b.model || a.power != b.power ||
      a.mode != b.mode || a.degrees != b.degrees || a.celsius != b.celsius ||
      a.fanspeed != b.fanspeed || a.swingv != b.swingv ||
      a.swingh != b.swingh || a.quiet != b.quiet || a.turbo != b.turbo ||
      a.econo != b.econo || a.light != b.light || a.filter != b.filter ||
      a.clean != b.clean || a.beep != b.beep || a.sleep != b.sleep;
}

stdAc::opmode_t IRac::strToOpmode(const char *str,
                                const stdAc::opmode_t def) {
  if (!strcasecmp(str, "AUTO") || !strcasecmp(str, "AUTOMATIC"))
    return stdAc::opmode_t::kAuto;
  else if (!strcasecmp(str, "OFF") || !strcasecmp(str, "STOP"))
    return stdAc::opmode_t::kOff;
  else if (!strcasecmp(str, "COOL") || !strcasecmp(str, "COOLING"))
    return stdAc::opmode_t::kCool;
  else if (!strcasecmp(str, "HEAT") || !strcasecmp(str, "HEATING"))
    return stdAc::opmode_t::kHeat;
  else if (!strcasecmp(str, "DRY") || !strcasecmp(str, "DRYING") ||
           !strcasecmp(str, "DEHUMIDIFY"))
    return stdAc::opmode_t::kDry;
  else if (!strcasecmp(str, "FAN") || !strcasecmp(str, "FANONLY") ||
           !strcasecmp(str, "FAN_ONLY"))
    return stdAc::opmode_t::kFan;
  else
    return def;
}

stdAc::fanspeed_t IRac::strToFanspeed(const char *str,
                                      const stdAc::fanspeed_t def) {
  if (!strcasecmp(str, "AUTO") || !strcasecmp(str, "AUTOMATIC"))
    return stdAc::fanspeed_t::kAuto;
  else if (!strcasecmp(str, "MIN") || !strcasecmp(str, "MINIMUM") ||
           !strcasecmp(str, "LOWEST"))
    return stdAc::fanspeed_t::kMin;
  else if (!strcasecmp(str, "LOW"))
    return stdAc::fanspeed_t::kLow;
  else if (!strcasecmp(str, "MED") || !strcasecmp(str, "MEDIUM") ||
           !strcasecmp(str, "MID"))
    return stdAc::fanspeed_t::kMedium;
  else if (!strcasecmp(str, "HIGH") || !strcasecmp(str, "HI"))
    return stdAc::fanspeed_t::kHigh;
  else if (!strcasecmp(str, "MAX") || !strcasecmp(str, "MAXIMUM") ||
           !strcasecmp(str, "HIGHEST"))
    return stdAc::fanspeed_t::kMax;
  else
    return def;
}

stdAc::swingv_t IRac::strToSwingV(const char *str,
                                  const stdAc::swingv_t def) {
  if (!strcasecmp(str, "AUTO") || !strcasecmp(str, "AUTOMATIC") ||
      !strcasecmp(str, "ON") || !strcasecmp(str, "SWING"))
    return stdAc::swingv_t::kAuto;
  else if (!strcasecmp(str, "OFF") || !strcasecmp(str, "STOP"))
    return stdAc::swingv_t::kOff;
  else if (!strcasecmp(str, "MIN") || !strcasecmp(str, "MINIMUM") ||
           !strcasecmp(str, "LOWEST") || !strcasecmp(str, "BOTTOM") ||
           !strcasecmp(str, "DOWN"))
    return stdAc::swingv_t::kLowest;
  else if (!strcasecmp(str, "LOW"))
    return stdAc::swingv_t::kLow;
  else if (!strcasecmp(str, "MID") || !strcasecmp(str, "MIDDLE") ||
           !strcasecmp(str, "MED") || !strcasecmp(str, "MEDIUM") ||
           !strcasecmp(str, "CENTRE") || !strcasecmp(str, "CENTER"))
    return stdAc::swingv_t::kMiddle;
  else if (!strcasecmp(str, "HIGH") || !strcasecmp(str, "HI"))
    return stdAc::swingv_t::kHigh;
  else if (!strcasecmp(str, "HIGHEST") || !strcasecmp(str, "MAX") ||
           !strcasecmp(str, "MAXIMUM") || !strcasecmp(str, "TOP") ||
           !strcasecmp(str, "UP"))
    return stdAc::swingv_t::kHighest;
  else
    return def;
}

stdAc::swingh_t IRac::strToSwingH(const char *str,
                                  const stdAc::swingh_t def) {
  if (!strcasecmp(str, "AUTO") || !strcasecmp(str, "AUTOMATIC") ||
      !strcasecmp(str, "ON") || !strcasecmp(str, "SWING"))
    return stdAc::swingh_t::kAuto;
  else if (!strcasecmp(str, "OFF") || !strcasecmp(str, "STOP"))
    return stdAc::swingh_t::kOff;
  else if (!strcasecmp(str, "LEFTMAX") || !strcasecmp(str, "LEFT MAX") ||
           !strcasecmp(str, "MAXLEFT") || !strcasecmp(str, "MAX LEFT") ||
           !strcasecmp(str, "FARLEFT") || !strcasecmp(str, "FAR LEFT"))
    return stdAc::swingh_t::kLeftMax;
  else if (!strcasecmp(str, "LEFT"))
    return stdAc::swingh_t::kLeft;
  else if (!strcasecmp(str, "MID") || !strcasecmp(str, "MIDDLE") ||
           !strcasecmp(str, "MED") || !strcasecmp(str, "MEDIUM") ||
           !strcasecmp(str, "CENTRE") || !strcasecmp(str, "CENTER"))
    return stdAc::swingh_t::kMiddle;
  else if (!strcasecmp(str, "RIGHT"))
    return stdAc::swingh_t::kRight;
  else if (!strcasecmp(str, "RIGHTMAX") || !strcasecmp(str, "RIGHT MAX") ||
           !strcasecmp(str, "MAXRIGHT") || !strcasecmp(str, "MAX RIGHT") ||
           !strcasecmp(str, "FARRIGHT") || !strcasecmp(str, "FAR RIGHT"))
    return stdAc::swingh_t::kRightMax;
  else if (!strcasecmp(str, "WIDE"))
    return stdAc::swingh_t::kWide;
  else
    return def;
}

// Assumes str is the model or an integer >= 1.
int16_t IRac::strToModel(const char *str, const int16_t def) {
  if (!strcasecmp(str, "LKE") || !strcasecmp(str, "PANASONICLKE")) {
    return panasonic_ac_remote_model_t::kPanasonicLke;
  } else if (!strcasecmp(str, "NKE") || !strcasecmp(str, "PANASONICNKE")) {
    return panasonic_ac_remote_model_t::kPanasonicNke;
  } else if (!strcasecmp(str, "DKE") || !strcasecmp(str, "PANASONICDKE")) {
    return panasonic_ac_remote_model_t::kPanasonicDke;
  } else if (!strcasecmp(str, "JKE") || !strcasecmp(str, "PANASONICJKE")) {
    return panasonic_ac_remote_model_t::kPanasonicJke;
  } else if (!strcasecmp(str, "CKP") || !strcasecmp(str, "PANASONICCKP")) {
    return panasonic_ac_remote_model_t::kPanasonicCkp;
  } else if (!strcasecmp(str, "RKR") || !strcasecmp(str, "PANASONICRKR")) {
    return panasonic_ac_remote_model_t::kPanasonicRkr;
  // Whirlpool A/C models
  } else if (!strcasecmp(str, "DG11J13A") || !strcasecmp(str, "DG11J104") ||
             !strcasecmp(str, "DG11J1-04")) {
    return whirlpool_ac_remote_model_t::DG11J13A;
  } else if (!strcasecmp(str, "DG11J191")) {
    return whirlpool_ac_remote_model_t::DG11J191;
  } else {
    int16_t number = atoi(str);
    if (number > 0)
      return number;
    else
      return def;
  }
}

bool IRac::strToBool(const char *str, const bool def) {
  if (!strcasecmp(str, "ON") || !strcasecmp(str, "1") ||
      !strcasecmp(str, "YES") || !strcasecmp(str, "TRUE"))
    return true;
  else if (!strcasecmp(str, "OFF") || !strcasecmp(str, "0") ||
           !strcasecmp(str, "NO") || !strcasecmp(str, "FALSE"))
    return false;
  else
    return def;
}

String IRac::boolToString(const bool value) {
  return value ? F("on") : F("off");
}

String IRac::opmodeToString(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kOff:
      return F("off");
    case stdAc::opmode_t::kAuto:
      return F("auto");
    case stdAc::opmode_t::kCool:
      return F("cool");
    case stdAc::opmode_t::kHeat:
      return F("heat");
    case stdAc::opmode_t::kDry:
      return F("dry");
    case stdAc::opmode_t::kFan:
      return F("fan_only");
    default:
      return F("unknown");
  }
}

String IRac::fanspeedToString(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kAuto:
      return F("auto");
    case stdAc::fanspeed_t::kMax:
      return F("max");
    case stdAc::fanspeed_t::kHigh:
      return F("high");
    case stdAc::fanspeed_t::kMedium:
      return F("medium");
    case stdAc::fanspeed_t::kLow:
      return F("low");
    case stdAc::fanspeed_t::kMin:
      return F("min");
    default:
      return F("unknown");
  }
}

String IRac::swingvToString(const stdAc::swingv_t swingv) {
  switch (swingv) {
    case stdAc::swingv_t::kOff:
      return F("off");
    case stdAc::swingv_t::kAuto:
      return F("auto");
    case stdAc::swingv_t::kHighest:
      return F("highest");
    case stdAc::swingv_t::kHigh:
      return F("high");
    case stdAc::swingv_t::kMiddle:
      return F("middle");
    case stdAc::swingv_t::kLow:
      return F("low");
    case stdAc::swingv_t::kLowest:
      return F("lowest");
    default:
      return F("unknown");
  }
}

String IRac::swinghToString(const stdAc::swingh_t swingh) {
  switch (swingh) {
    case stdAc::swingh_t::kOff:
      return F("off");
    case stdAc::swingh_t::kAuto:
      return F("auto");
    case stdAc::swingh_t::kLeftMax:
      return F("leftmax");
    case stdAc::swingh_t::kLeft:
      return F("left");
    case stdAc::swingh_t::kMiddle:
      return F("middle");
    case stdAc::swingh_t::kRight:
      return F("right");
    case stdAc::swingh_t::kRightMax:
      return F("rightmax");
    case stdAc::swingh_t::kWide:
      return F("leftright");
    default:
      return F("unknown");
  }
}

namespace IRAcUtils {
  // Display the human readable state of an A/C message if we can.
  // Args:
  //   result: A Ptr to the captured `decode_results` that contains an A/C mesg.
  // Returns:
  //   A string with the human description of the A/C message. "" if we can't.
  String resultAcToString(const decode_results * const result) {
    switch (result->decode_type) {

#if DECODE_TOSHIBA_AC
      case decode_type_t::TOSHIBA_AC: {
        IRToshibaAC ac(0);
        ac.setRaw(result->state);
        return ac.toString();
      }
#endif  // DECODE_TOSHIBA_AC
#if DECODE_HAIER_AC
      case decode_type_t::HAIER_AC: {
        IRHaierAC ac(0);
        ac.setRaw(result->state);
        return ac.toString();
      }
#endif  // DECODE_HAIER_AC
#if DECODE_HAIER_AC_YRW02
      case decode_type_t::HAIER_AC_YRW02: {
        IRHaierACYRW02 ac(0);
        ac.setRaw(result->state);
        return ac.toString();
      }
#endif  // DECODE_HAIER_AC_YRW02
#if DECODE_SAMSUNG_AC
      case decode_type_t::SAMSUNG_AC: {
        IRSamsungAc ac(0);
        ac.setRaw(result->state, result->bits / 8);
        return ac.toString();
      }
#endif  // DECODE_SAMSUNG_AC
#if DECODE_COOLIX
      case decode_type_t::COOLIX: {
        IRCoolixAC ac(0);
        ac.setRaw(result->value);  // Coolix uses value instead of state.
        return ac.toString();
      }
#endif  // DECODE_COOLIX
#if DECODE_PANASONIC_AC
      case decode_type_t::PANASONIC_AC: {
        if (result->bits > kPanasonicAcShortBits) {
          IRPanasonicAc ac(0);
          ac.setRaw(result->state);
          return ac.toString();
        }
        return "";
      }
#endif  // DECODE_PANASONIC_AC
#if DECODE_WHIRLPOOL_AC
      case decode_type_t::WHIRLPOOL_AC: {
        IRWhirlpoolAc ac(0);
        ac.setRaw(result->state);
        return ac.toString();
      }
#endif  // DECODE_WHIRLPOOL_AC
#if DECODE_TCL112AC
      case decode_type_t::TCL112AC: {
        IRTcl112Ac ac(0);
        ac.setRaw(result->state);
        return ac.toString();
      }
#endif  // DECODE_TCL112AC
      default:
        return "";
    }
  }

  // Convert a valid IR A/C remote message that we understand enough into a
  // Common A/C state.
  //
  // Args:
  //   decode: A PTR to a successful raw IR decode object.
  //   result: A PTR to a state structure to store the result in.
  //   prev:   A PTR to a state structure which has the prev. state. (optional)
  // Returns:
  //   A boolean indicating success or failure.
  bool decodeToState(const decode_results *decode, stdAc::state_t *result,
                     const stdAc::state_t *prev) {
    if (decode == NULL || result == NULL) return false;  // Safety check.
    switch (decode->decode_type){ 

#if DECODE_COOLIX
      case decode_type_t::COOLIX: {
        IRCoolixAC ac(kGpioUnused);
        ac.setRaw(decode->value);  // Uses value instead of state.
        *result = ac.toCommon(prev);
        break;
      }
#endif  // DECODE_COOLIX
#if DECODE_HAIER_AC
      case decode_type_t::HAIER_AC: {
        IRHaierAC ac(kGpioUnused);
        ac.setRaw(decode->state);
        *result = ac.toCommon();
        break;
      }
#endif  // DECODE_HAIER_AC
#if DECODE_HAIER_AC_YRW02
      case decode_type_t::HAIER_AC_YRW02: {
        IRHaierACYRW02 ac(kGpioUnused);
        ac.setRaw(decode->state);
        *result = ac.toCommon();
        break;
      }
#endif  // DECODE_HAIER_AC_YRW02
#if DECODE_PANASONIC_AC
      case decode_type_t::PANASONIC_AC: {
        IRPanasonicAc ac(kGpioUnused);
        ac.setRaw(decode->state);
        *result = ac.toCommon();
        break;
      }
#endif  // DECODE_PANASONIC_AC
#if DECODE_SAMSUNG_AC
      case decode_type_t::SAMSUNG_AC: {
        IRSamsungAc ac(kGpioUnused);
        ac.setRaw(decode->state);
        *result = ac.toCommon();
        break;
      }
#endif  // DECODE_SAMSUNG_AC
#if DECODE_TCL112AC
      case decode_type_t::TCL112AC: {
        IRTcl112Ac ac(kGpioUnused);
        ac.setRaw(decode->state);
        *result = ac.toCommon();
        break;
      }
#endif  // DECODE_TCL112AC
#if DECODE_TOSHIBA_AC
      case decode_type_t::TOSHIBA_AC: {
        IRToshibaAC ac(kGpioUnused);
        ac.setRaw(decode->state);
        *result = ac.toCommon();
        break;
      }
#endif  // DECODE_TOSHIBA_AC
#if DECODE_WHIRLPOOL_AC
      case decode_type_t::WHIRLPOOL_AC: {
        IRWhirlpoolAc ac(kGpioUnused);
        ac.setRaw(decode->state);
        *result = ac.toCommon();
        break;
      }
#endif  // DECODE_WHIRLPOOL_AC
      default:
        return false;
    }
    return true;
  }
 // namespace IRAcUtils
}