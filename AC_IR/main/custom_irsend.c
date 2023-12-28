#include "include/main.h"

IRSend_t IRObject;
void IR_init()
{
  IRObject.IRpin = IR_CTRL_SEND_PIN;
  IRObject._dutycycle = 75;
  irsend_configuration(INVERTED_FALSE, USE_MODULATION);
  IR_gpio_configuration();
}

void irsend_configuration(bool inverted, bool use_modulation)
{
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
}

void IR_gpio_configuration()
{
  // zero-initialize the config structure.
  gpio_config_t o_conf = {}, i_conf = {};
  // disable interrupt
  o_conf.intr_type = GPIO_INTR_DISABLE;
  i_conf.intr_type = GPIO_INTR_DISABLE;
  // set as output mode
  o_conf.mode = GPIO_MODE_OUTPUT;
  i_conf.mode = GPIO_MODE_INPUT;
  // bit mask of the pins that you want to set,e.g.GPIO18/19
  o_conf.pin_bit_mask = (1ULL << IRObject.IRpin);
  i_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
  // disable pull-down mode
  o_conf.pull_down_en = 0;
  i_conf.pull_down_en = 0;
  // disable pull-up mode
  o_conf.pull_up_en = 0;
  i_conf.pull_up_en = 1;
  // configure GPIO with the given settings
  gpio_config(&o_conf);
  gpio_config(&i_conf);
}

void enableIROut(uint32_t freq, uint8_t duty)
{
  if (IRObject.modulation)
  {
    if (duty < kDutyMax)
      IRObject._dutycycle = duty;
    else
      IRObject._dutycycle = kDutyMax;
  }
  else
    IRObject._dutycycle = 65;
  if (freq < 1000)
    freq *= 1000;
  uint32_t period = calcUSecPeriod(freq, false);
  // Nr. of uSeconds the LED will be on per pulse.
  IRObject.onTimePeriod = (period * IRObject._dutycycle) / kDutyMax;
  // Nr. of uSeconds the LED will be off per pulse.
  IRObject.offTimePeriod = period - IRObject.onTimePeriod;
  //    printf("Leaving enableIRout\r\n");
}

void sendGeneric(const uint16_t headermark, const uint32_t headerspace,
                 const uint16_t onemark, const uint32_t onespace,
                 const uint16_t zeromark, const uint32_t zerospace,
                 const uint16_t footermark, const uint32_t gap,
                 const uint8_t *dataptr, const uint16_t nbytes,
                 const uint16_t frequency, const bool MSBfirst,
                 const uint16_t repeat, const uint8_t dutycycle)
{
  enableIROut(frequency, dutycycle);
  for (uint16_t r = 0; r <= repeat; r++)
  {
    if (headermark)
      mark(headermark);
    if (headerspace)
      space(headerspace);
    for (uint16_t i = 0; i < nbytes; i++)
    {
      sendData(onemark, onespace, zeromark, zerospace, *(dataptr + i), 8,
               MSBfirst);
    }
    if (footermark)
      mark(footermark);
    space(gap);
  }
}

void sendGenericmsgtime(const uint16_t headermark, const uint32_t headerspace,
                        const uint16_t onemark, const uint32_t onespace,
                        const uint16_t zeromark, const uint32_t zerospace,
                        const uint16_t footermark, const uint32_t gap,
                        const uint32_t mesgtime, const uint64_t data,
                        const uint16_t nbits, const uint16_t frequency,
                        const bool MSBfirst, const uint16_t repeat,
                        const uint8_t dutycycle)
{
  enableIROut(frequency, dutycycle);
  for (uint16_t r = 0; r <= repeat; r++)
  {
    uint64_t start_usecs = esp_timer_get_time();

    if (headermark)
      mark(headermark);

    if (headerspace)
      space(headerspace);
    sendData(onemark, onespace, zeromark, zerospace, data, nbits, MSBfirst);

    if (footermark)
      mark(footermark);
    uint32_t elapsed = esp_timer_get_time() - start_usecs;
    if (elapsed >= mesgtime)
    {
      space(gap);
    }
    else
    {
      uint64_t delay1 = max(gap, mesgtime - elapsed);
      space(delay1);
    }
  }
}

uint8_t sumBytes(const uint8_t *const start, const uint16_t length)
{
  uint8_t checksum = 0;
  const uint8_t *ptr;
  for (ptr = start; ptr - start < length; ptr++)
    checksum += *ptr;
  return checksum;
}

uint32_t calcUSecPeriod(uint32_t hz, bool use_offset)
{
  if (hz == 0)
    hz = 1;                                    // Avoid Zero hz. Divide by Zero is nasty.
  uint32_t period = (1000000UL + hz / 2) / hz; // The equiv of round(1000000/hz).
  // Apply the offset and ensure we don't result in a <= 0 value.
  if (use_offset)
  {
    return max((uint32_t)1, period + IRObject.periodOffset);
  }
  else
  {
    return max((uint32_t)1, period);
  }
}

uint64_t min(uint64_t param1, uint64_t param2)
{
  if (param1 < param2)
    return param1;
  else
    return param2;
}

uint64_t max(uint64_t param1, uint64_t param2)
{
  if (param1 > param2)
  {
    return param1;
  }
  return param2;
}

uint16_t mark(uint16_t usec)
{
  // Handle the simple case of no required frequency modulation.
  //  if (IRObject.modulation || IRObject._dutycycle >= 100) {
  //	gpio_set_level(IRObject.IRpin, HIGH);
  //	ets_delay_us(usec);
  //    gpio_set_level(IRObject.IRpin, LOW);
  ////    printf("abcd");
  //  }

  // Not simple, so do it assuming frequency modulation.
  uint16_t counter = 0;
  uint64_t usecTimer = esp_timer_get_time();
  // Cache the time taken so far. This saves us calling time, and we can be
  // assured that we can't have odd math problems. i.e. unsigned under/overflow.
  uint32_t elapsed = esp_timer_get_time() - usecTimer;

  while (elapsed < usec)
  { // Loop until we've met/exceeded our required time.
    gpio_set_level(IRObject.IRpin, HIGH);
    // Calculate how long we should pulse on for.
    // e.g. Are we to close to the end of our requested mark time (usec)?
    ets_delay_us(IRObject.onTimePeriod);
    gpio_set_level(IRObject.IRpin, LOW);
    counter++;
    if (elapsed + IRObject.onTimePeriod >= usec)
      return counter; // LED is now off & we've passed our allotted time.
    // Wait for the lesser of the rest of the duty cycle, or the time remaining.
    ets_delay_us(min(usec - elapsed - IRObject.onTimePeriod, (uint32_t)IRObject.offTimePeriod));
    elapsed = esp_timer_get_time() - usecTimer; // Update & recache the actual elapsed time.
  }
  return counter;
}

void space(uint32_t time)
{
  gpio_set_level(IRObject.IRpin, LOW);
  if (time == 0)
    return;
  ets_delay_us(time);
}

void sendData(uint16_t onemark, uint32_t onespace, uint16_t zeromark,
              uint32_t zerospace, uint64_t data, uint16_t nbits,
              bool MSBfirst)
{
  if (nbits == 0) // If we are asked to send nothing, just return.
    return;
  if (MSBfirst)
  { // Send the MSB first.
    // Send 0's until we get down to a bit size we can actually manage.
    while (nbits > sizeof(data) * 8)
    {
      mark(zeromark);
      space(zerospace);
      nbits--;
    }
    // Send the supplied data.
    for (uint64_t mask = 1ULL << (nbits - 1); mask; mask >>= 1)
      if (data & mask)
      { // Send a 1
        mark(onemark);
        space(onespace);
      }
      else
      { // Send a 0
        mark(zeromark);
        space(zerospace);
      }
  }
  else
  { // Send the Least Significant Bit (LSB) first / MSB last.
    for (uint16_t bit = 0; bit < nbits; bit++, data >>= 1)
      if (data & 1)
      { // Send a 1
        mark(onemark);
        space(onespace);
      }
      else
      { // Send a 0
        mark(zeromark);
        space(zerospace);
      }
  }
}

void poll_button()
{
  if (!gpio_get_level(TEMP_INC_BUTTON))
  {
    button_pressed = true;
    setTemp_Daikin280(++curr_temp_Daikin280);
    setTemp_Daikin216(++curr_temp_Daikin216);
    printf("Temp_inc_button pressed\r\n");
  }
  else if (!gpio_get_level(TEMP_DEC_BUTTON))
  {
    button_pressed = true;
    setTemp_Daikin280(--curr_temp_Daikin280);
    setTemp_Daikin216(--curr_temp_Daikin216);
    printf("Temp_dec_button pressed\r\n");
  }
  else if (!gpio_get_level(POWER_BUTTON))
  {
    button_pressed = true;
    if(data_Daikin280.Power)
      setPower_Daikin280(false);
    else
      setPower_Daikin280(true);
    if(data_Daikin216.Power)
      setPower_Daikin216(false);
    else
      setPower_Daikin216(true);
    printf("Power button pressed\r\n");
  }
  else if (!gpio_get_level(MODE_BUTTON))
  {
    button_pressed = true;
//    switch (curr_mode_Daikin280)
//    {
//    case DaikinAuto:
//    	curr_mode_Daikin280 = DaikinDry;
//      break;
//    case DaikinDry:
//    	curr_mode_Daikin280 = DaikinCool;
//      break;
//    case DaikinCool:
//    	curr_mode_Daikin280 = DaikinHeat;
//      break;
//    case DaikinHeat:
//    	curr_mode_Daikin280 = DaikinFan;
//      break;
//    case DaikinFan:
//    	curr_mode_Daikin280 = DaikinAuto;
//      break;
//    default:
//      printf("Invalid Mode\r\n");
//      break;
//    }
//    setMode_Daikin280(curr_mode_Daikin280);
    if(data_Daikin280.SwingV)
    	data_Daikin280.SwingV = 0b0000;
    else
    	data_Daikin280.SwingV = 0b1111;
    if(data_Daikin216.SwingV)
      data_Daikin216.SwingV = 0b0000;
    else
      data_Daikin216.SwingV = 0b1111;
    printf("Swing Vertical button pressed\r\n");
  }
  else if (!gpio_get_level(FAN_BUTTON))
  {
    button_pressed = true;
    switch (curr_fan_Daikin280)
    {
    case DaikinFanMin:
      curr_fan_Daikin280 = DaikinFanMed;
      break;
    case DaikinFanMed:
      curr_fan_Daikin280 = DaikinFanMax;
      break;
    case DaikinFanMax:
      curr_fan_Daikin280 = DaikinFanAuto;
      break;
    case DaikinFanAuto:
      curr_fan_Daikin280 = DaikinFanQuiet;
      break;
    case DaikinFanQuiet:
      curr_fan_Daikin280 = DaikinFanMin;
      break;
    default:
      printf("Invalid Fan Mode \r\n");
      break;
    }
    switch (curr_fan_Daikin216)
    {
    case DaikinFanMin:
      curr_fan_Daikin216 = DaikinFanMed;
      break;
    case DaikinFanMed:
      curr_fan_Daikin216 = DaikinFanMax;
      break;
    case DaikinFanMax:
      curr_fan_Daikin216 = DaikinFanAuto;
      break;
    case DaikinFanAuto:
      curr_fan_Daikin216 = DaikinFanQuiet;
      break;
    case DaikinFanQuiet:
      curr_fan_Daikin216 = DaikinFanMin;
      break;
    default:
      printf("Invalid Fan Mode \r\n");
      break;
    }
    setFan_Daikin280(curr_fan_Daikin280);
    setFan_Daikin216(curr_fan_Daikin216);
    printf("fan button pressed\r\n");
  }
  if(button_pressed)
	{
		button_pressed = false;
		// checksum_Daikin280();
		// send_Daikin280();
    checksum_Daikin216();
    send_Daikin216();
		vTaskDelay(10);
	}
}
