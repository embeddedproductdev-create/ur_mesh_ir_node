#include "include/custom_irsend.h"
#include <driver/gpio.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

const int8_t kPeriodOffset = -5;
const uint8_t kDutyDefault = 50;
const uint8_t kDutyMax = 100;
const uint16_t kNECBits = 32;
const uint16_t kNoRepeat = 0;

const uint16_t kNecTick = 560;
const uint16_t kNecHdrMarkTicks = 16;
const uint16_t kNecHdrMark = kNecHdrMarkTicks * kNecTick;
const uint16_t kNecHdrSpaceTicks = 8;
const uint16_t kNecHdrSpace = kNecHdrSpaceTicks * kNecTick;
const uint16_t kNecBitMarkTicks = 1;
const uint16_t kNecBitMark = kNecBitMarkTicks * kNecTick;
const uint16_t kNecOneSpaceTicks = 3;
const uint16_t kNecOneSpace = kNecOneSpaceTicks * kNecTick;
const uint16_t kNecZeroSpaceTicks = 1;
const uint16_t kNecZeroSpace = kNecZeroSpaceTicks * kNecTick;
const uint16_t kNecRptSpaceTicks = 4;
const uint16_t kNecRptSpace = kNecRptSpaceTicks * kNecTick;
const uint16_t kNecRptLength = 4;
const uint16_t kNecMinCommandLengthTicks = 193;
const uint32_t kNecMinCommandLength = kNecMinCommandLengthTicks * kNecTick;
const uint32_t kNecMinGap =
    kNecMinCommandLength -
    (kNecHdrMark + kNecHdrSpace + kNECBits * (kNecBitMark + kNecOneSpace) +
     kNecBitMark);
const uint16_t kNecMinGapTicks =
    kNecMinCommandLengthTicks -
    (kNecHdrMarkTicks + kNecHdrSpaceTicks +
     kNECBits * (kNecBitMarkTicks + kNecOneSpaceTicks) + kNecBitMarkTicks);

void irsend_configuration(bool inverted, bool use_modulation)
{
	printf("Inside irsend_configuration\n");
    if (inverted)
    {
        IRObject.outputOn = LOW;
        IRObject.outputOff = HIGH;
    }
    else
    {
        IRObject.outputOn = HIGH;
        IRObject.outputOff = LOW;
    }
    IRObject.modulation = use_modulation;
    if (IRObject.modulation)
        IRObject._dutycycle = kDutyDefault;
    else
        IRObject._dutycycle = kDutyMax;
    printf("Leaving irsend_configuration\n");
}

void irsend_begin()
{
	printf("inside irsend_begin\n");
    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = (1ULL << IRObject.IRpin);
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = 0;
    // configure GPIO with the given settings
    gpio_config(&io_conf);
    printf("GPIO configured and leaving irsend_begin\n");
}

void sendNEC(uint64_t data, uint16_t nbits, uint16_t repeat)
{
	printf("inside sendNEC\n");
    sendGeneric(kNecHdrMark, kNecHdrSpace, kNecBitMark, kNecOneSpace, kNecBitMark,
                kNecZeroSpace, kNecBitMark, kNecMinGap, kNecMinCommandLength,
                data, nbits, 38, true, 0, 33);
    printf("leaving sendNEC\n");
}

void enableIROut(uint32_t freq, uint8_t duty)
{
	printf("inside enableIROut\n");
    // Set the duty cycle to use if we want freq. modulation.
    if (IRObject.modulation)
    {
        if(duty < kDutyMax)
            IRObject._dutycycle = duty;
        else
            IRObject._dutycycle = kDutyMax;
    }
    else
    {
        IRObject._dutycycle = kDutyMax;
    }
    if (freq < 1000) // Were we given kHz? Supports the old call usage.
        freq *= 1000;
    uint32_t period = calcUSecPeriod(freq, false);
    // Nr. of uSeconds the LED will be on per pulse.
    IRObject.onTimePeriod = (period * IRObject._dutycycle) / kDutyMax;
    // Nr. of uSeconds the LED will be off per pulse.
    IRObject.offTimePeriod = period - IRObject.onTimePeriod;
    printf("Leaving enableIROut\n");
}

void sendGeneric(const uint16_t headermark, const uint32_t headerspace,
                 const uint16_t onemark, const uint32_t onespace,
                 const uint16_t zeromark, const uint32_t zerospace,
                 const uint16_t footermark, const uint32_t gap,
                 const uint32_t mesgtime, const uint64_t data,
                 const uint16_t nbits, const uint16_t frequency,
                 const bool MSBfirst, const uint16_t repeat,
                 const uint8_t dutycycle)
{
	printf("inside sendGeneric\n");
    // Setup
    enableIROut(frequency, dutycycle);
    // We always send a message, even for repeat=0, hence '<= repeat'.
    for (uint16_t r = 0; r <= repeat; r++)
    {
        //resetting the timer
        uint64_t start_usecs = esp_timer_get_time();

        // Header
        if (headermark)
            mark(headermark);
        if (headerspace)
            space(headerspace);

        // Data
        printf("Sending data...\n");
        sendData(onemark, onespace, zeromark, zerospace, data, nbits, MSBfirst);
        printf("completed sending data... \n");
        // Footer
        if (footermark)
            mark(footermark);
        uint32_t elapsed = esp_timer_get_time() - start_usecs;
        // Avoid potential unsigned integer underflow. e.g. when mesgtime is 0.
        if (elapsed >= mesgtime)
            space(gap);
        else
            space(max(gap, mesgtime - elapsed));
    }
    printf("Leaving sendGeneric\n");
}

uint32_t calcUSecPeriod(uint32_t hz, bool use_offset) {
  printf("Inside calcUSecPeriod\n");
  if (hz == 0)
    hz = 1;  // Avoid Zero hz. Divide by Zero is nasty.
  uint32_t period = (1000000UL + hz / 2) / hz;  // The equiv of round(1000000/hz).
  // Apply the offset and ensure we don't result in a <= 0 value.
  if (use_offset)
  {
	  printf("Leaving calcUSecPeriod\n");
	  return max((uint32_t)1, period + IRObject.periodOffset);
  }
  else
  {
	printf("Leaving calcUSecPeriod\n");
    return max((uint32_t)1, period);
  }
}

uint64_t min(uint64_t param1, uint64_t param2)
{
    if(param1 < param2)
        return param1;
    else
        return param2;
}

uint64_t max(uint64_t param1, uint64_t param2)
{
    if(param1 > param2)
        return param1;
    else
        return param2;
}

void mark(uint16_t usec)
{
  printf("Inside mark \n");
  // Handle the simple case of no required frequency modulation.
  if (!IRObject.modulation || IRObject._dutycycle >= 100) {
    gpio_set_level(IRObject.IRpin, HIGH);
    vTaskDelay(pdMS_TO_TICK(usec));
    gpio_set_level(IRObject.IRpin, LOW);
  }
  printf("Leaving mark \n");
}

void space(uint32_t time)
{
    gpio_set_level(IRObject.IRpin, LOW);
    if(time==0) return;
    vTaskDelay(time);
}


void sendData(uint16_t onemark, uint32_t onespace, uint16_t zeromark,
                      uint32_t zerospace, uint64_t data, uint16_t nbits,
                      bool MSBfirst) {
  if (nbits == 0)  // If we are asked to send nothing, just return.
    return;
  if (MSBfirst) {  // Send the MSB first.
    // Send 0's until we get down to a bit size we can actually manage.
    while (nbits > sizeof(data) * 8) {
      mark(zeromark);
      space(zerospace);
      nbits--;
    }
    // Send the supplied data.
    for (uint64_t mask = 1ULL << (nbits - 1); mask; mask >>= 1)
      if (data & mask) {  // Send a 1
        mark(onemark);
        space(onespace);
      } else {  // Send a 0
        mark(zeromark);
        space(zerospace);
      }
  } else {  // Send the Least Significant Bit (LSB) first / MSB last.
    for (uint16_t bit = 0; bit < nbits; bit++, data >>= 1)
      if (data & 1) {  // Send a 1
        mark(onemark);
        space(onespace);
      } else {  // Send a 0
        mark(zeromark);
        space(zerospace);
      }
  }
}
