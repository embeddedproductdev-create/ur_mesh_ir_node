#include "include/main.h"

/* GLOBAL VARIABLE INITIALIZATION */

uint16_t bits;
uint8_t tolerance;
uint8_t recvpin;
uint16_t rawlen;
uint16_t unknown_threshold;
volatile uint16_t *rawbuf;
bool overflow;
bool repeat;
decode_type_t decode_type;
state_u state;
volatile irparams_t params;
irparams_t *params_save;

static QueueHandle_t gpio_evt_queue = NULL;

/// Does the given protocol use a complex state as part of the decode?
/// @param[in] protocol The decode_type_t protocol we are enquiring about.
/// @return True if the protocol uses a state array. False if just an integer.
bool hasACState(const decode_type_t protocol)
{
    switch (protocol)
    {
    // This is kept sorted by name
    case AMCOR:
    case ARGO:
    case BOSCH144:
    case CARRIER_AC84:
    case CARRIER_AC128:
    case CORONA_AC:
    case DAIKIN:
    case DAIKIN128:
    case DAIKIN152:
    case DAIKIN160:
    case DAIKIN176:
    case DAIKIN2:
    case DAIKIN200:
    case DAIKIN216:
    case DAIKIN312:
    case ELECTRA_AC:
    case FUJITSU_AC:
    case GREE:
    case HAIER_AC:
    case HAIER_AC_YRW02:
    case HAIER_AC160:
    case HAIER_AC176:
    case HITACHI_AC:
    case HITACHI_AC1:
    case HITACHI_AC2:
    case HITACHI_AC3:
    case HITACHI_AC264:
    case HITACHI_AC296:
    case HITACHI_AC344:
    case HITACHI_AC424:
    case KELON168:
    case KELVINATOR:
    case MIRAGE:
    case MITSUBISHI136:
    case MITSUBISHI112:
    case MITSUBISHI_AC:
    case MITSUBISHI_HEAVY_88:
    case MITSUBISHI_HEAVY_152:
    case MWM:
    case NEOCLIMA:
    case PANASONIC_AC:
    case RHOSS:
    case SAMSUNG_AC:
    case SANYO_AC:
    case SANYO_AC88:
    case SANYO_AC152:
    case SHARP_AC:
    case TCL96AC:
    case TCL112AC:
    case TEKNOPOINT:
    case TOSHIBA_AC:
    case TROTEC:
    case TROTEC_3550:
    case VOLTAS:
    case WHIRLPOOL_AC:
    case YORK:
        return true;
    default:
        return false;
    }
}

uint8_t _validTolerance(const uint8_t percentage) {
    return (percentage > 100) ? tolerance : percentage;
}

uint32_t ticksHigh(const uint32_t usecs, const uint8_t tolerance,
                           const uint16_t delta) {
  return ((uint32_t)(usecs * (1.0 + _validTolerance(tolerance) / 100.0)) + 1 +
          delta);
}

uint32_t ticksLow(const uint32_t usecs, const uint8_t tolerance,
                          const uint16_t delta) {
  // max() used to ensure the result can't drop below 0 before the cast.
  return ((uint32_t)max(
      (int32_t)(usecs * (1.0 - _validTolerance(tolerance) / 100.0) - delta),
      (int32_t)0));
}

char *uint64ToString(uint64_t input, uint8_t base)
{
    static char result[50] = "";
    // prevent issues if called with base <= 1
    if (base < 2)
        base = 10;
    // Check we have a base that we can actually print.
    // i.e. [0-9A-Z] == 36
    if (base > 36)
        base = 10;

    do
    {
        char c = input % base;
        input /= base;

        if (c < 10)
            c += '0';
        else
            c += 'A' - 10;
        strncat(result, &c, 1);
    } while (input);
    return result;
}

char *typeToString(decode_type_t protocol)
{
    static char result[15] = "";
    if (protocol > kLastDecodeType || protocol == UNKNOWN)
    {
        strcpy(result, "Unknown");
    }
    else
    {
    strcpy(result,"Daikin216");
    }
    return result;
}

char *resultToHex()
{
    static char output[150] = "", ch = '0';
    if (hasACState(decode_type))
    {
        for (uint16_t i = 0; bits > i * 8; i++)
        {
            if (state.state_array[i] < 0x10)
            	strncat(output, &ch, 1);
            char *str = uint64ToString(state.state_array[i], 16);
            strcat(output, str);
        }
    }
    return output;
}

void printToHumanReadableBasic()
{
    printf("==================================================\r\n");
    printf("Protocol : %s", typeToString(decode_type));
    printf("Code     : %s", resultToHex());
    printf("==================================================\r\n");
}

bool match(uint32_t measured, uint32_t desired, uint8_t tolerance)
{
    measured *= kRawTick;
    return (measured >= ticksLow(desired, tolerance, 0) &&
            measured <= ticksHigh(desired, tolerance, 0));
}

bool matchMark(uint32_t measured, uint32_t desired, uint8_t tolerance,
               int16_t excess)
{
    return match(measured, desired + excess, tolerance);
}

bool matchSpace(uint32_t measured, uint32_t desired, uint8_t tolerance,
                int16_t excess)
{
    return match(measured, desired - excess, tolerance);
}

uint64_t reverseBits(uint64_t input, uint16_t nbits) {
  if (nbits <= 1) return input;  // Reversing <= 1 bits makes no change at all.
  // Cap the nr. of bits to rotate to the max nr. of bits in the input.
  nbits = min(nbits, (uint16_t)(sizeof(input) * 8));
  uint64_t output = 0;
  for (uint16_t i = 0; i < nbits; i++) {
    output <<= 1;
    output |= (input & 1);
    input >>= 1;
  }
  // Merge any remaining unreversed bits back to the top of the reversed bits.
  return (input << nbits) | output;
}

match_result_t matchData(
    volatile uint16_t *data_ptr, const uint16_t nbits, const uint16_t onemark,
    const uint32_t onespace, const uint16_t zeromark, const uint32_t zerospace,
    const uint8_t tolerance, const int16_t excess, const bool MSBfirst,
    const bool expectlastspace)
{
    match_result_t result;
    result.success = false; // Fail by default
    result.data = 0;
    if (expectlastspace)
    { // We are expecting data with a final space.
        for (result.used = 0; result.used < nbits * 2;
             result.used += 2, data_ptr += 2)
        {
            // Is the bit a '1'?
            if (matchMark(*data_ptr, onemark, tolerance, excess) &&
                matchSpace(*(data_ptr + 1), onespace, tolerance, excess))
            {
                result.data = (result.data << 1) | 1;
            }
            else if (matchMark(*data_ptr, zeromark, tolerance, excess) &&
                     matchSpace(*(data_ptr + 1), zerospace, tolerance, excess))
            {
                result.data <<= 1; // The bit is a '0'.
            }
            else
            {
                if (!MSBfirst)
                    result.data = reverseBits(result.data, result.used / 2);
                return result; // It's neither, so fail.
            }
        }
        result.success = true;
    }
    else
    { // We are expecting data without a final space.
        // Match all but the last bit, as it may not match easily.
        result = matchData(data_ptr, nbits ? nbits - 1 : 0, onemark, onespace,
                           zeromark, zerospace, tolerance, excess, true, true);
        if (result.success)
        {
            // Is the bit a '1'?
            if (matchMark(*(data_ptr + result.used), onemark, tolerance, excess))
                result.data = (result.data << 1) | 1;
            else if (matchMark(*(data_ptr + result.used), zeromark, tolerance,
                               excess))
                result.data <<= 1; // The bit is a '0'.
            else
                result.success = false;
            if (result.success)
                result.used++;
        }
    }
    if (!MSBfirst)
        result.data = reverseBits(result.data, nbits);
    return result;
}

uint16_t matchBytes(volatile uint16_t *data_ptr, uint8_t *result_ptr,
                            const uint16_t remaining, const uint16_t nbytes,
                            const uint16_t onemark, const uint32_t onespace,
                            const uint16_t zeromark, const uint32_t zerospace,
                            const uint8_t tolerance, const int16_t excess,
                            const bool MSBfirst, const bool expectlastspace) {
  // Check if there is enough capture buffer to possibly have the desired bytes.
  if (remaining + expectlastspace < (nbytes * 8 * 2) + 1)
    return 0;  // Nope, so abort.
  uint16_t offset = 0;
  for (uint16_t byte_pos = 0; byte_pos < nbytes; byte_pos++) {
    bool lastspace = (byte_pos + 1 == nbytes) ? expectlastspace : true;
    match_result_t result = matchData(data_ptr + offset, 8, onemark, onespace,
                                      zeromark, zerospace, tolerance, excess,
                                      MSBfirst, lastspace);
    if (result.success == false) return 0;  // Fail
    result_ptr[byte_pos] = (uint8_t)result.data;
    offset += result.used;
  }
  return offset;
}

bool matchAtLeast(uint32_t measured, uint32_t desired,
                          uint8_t tolerance, uint16_t delta) {
  measured *= kRawTick;  // Convert to uSecs.
  if (measured == 0) return true;
  return measured >= ticksLow(min(desired,
                                       (uint32_t)(params.timeout*1000)),
                              tolerance, delta);
}

uint16_t _matchGeneric(volatile uint16_t *data_ptr,
                       uint64_t *result_bits_ptr,
                       uint8_t *result_bytes_ptr,
                       const bool use_bits,
                       const uint16_t remaining,
                       const uint16_t nbits,
                       const uint16_t hdrmark,
                       const uint32_t hdrspace,
                       const uint16_t onemark,
                       const uint32_t onespace,
                       const uint16_t zeromark,
                       const uint32_t zerospace,
                       const uint16_t footermark,
                       const uint32_t footerspace,
                       const bool atleast,
                       const uint8_t tolerance,
                       const int16_t excess,
                       const bool MSBfirst)
{
    // If we are expecting byte sizes, check it's a factor of 8 or fail.
    if (!use_bits && nbits % 8 != 0)
        return 0;
    // Calculate if we expect a trailing space in the data section.
    const bool kexpectspace = footermark || (onespace != zerospace);
    // Calculate how much remaining buffer is required.
    uint16_t min_remaining = nbits * 2 - (kexpectspace ? 0 : 1);

    if (hdrmark)
        min_remaining++;
    if (hdrspace)
        min_remaining++;
    if (footermark)
        min_remaining++;
    // Don't need to extend for footerspace because it could be the end of message

    // Check if there is enough capture buffer to possibly have the message.
    if (remaining < min_remaining)
        return 0; // Nope, so abort.
    uint16_t offset = 0;

    // Header
    if (hdrmark && !matchMark(*(data_ptr + offset++), hdrmark, tolerance, excess))
        return 0;
    if (hdrspace && !matchSpace(*(data_ptr + offset++), hdrspace, tolerance,
                                excess))
        return 0;

    // Data
    if (use_bits)
    { // Bits.
        match_result_t result = matchData(data_ptr + offset, nbits,
                                                  onemark, onespace,
                                                  zeromark, zerospace, tolerance,
                                                  excess, MSBfirst, kexpectspace);
        if (!result.success)
            return 0;
        *result_bits_ptr = result.data;
        offset += result.used;
    }
    else
    { // bytes
        uint16_t data_used = matchBytes(data_ptr + offset, result_bytes_ptr,
                                                remaining - offset, nbits / 8,
                                                onemark, onespace,
                                                zeromark, zerospace, tolerance,
                                                excess, MSBfirst, kexpectspace);
        if (!data_used)
            return 0;
        offset += data_used;
    }
    // Footer
    if (footermark && !matchMark(*(data_ptr + offset++), footermark, tolerance,
                                 excess))
        return 0;
    // If we have something still to match & haven't reached the end of the buffer
    if (footerspace && offset < remaining)
    {
        if (atleast)
        {
            if (!matchAtLeast(*(data_ptr + offset), footerspace, tolerance, excess))
                return 0;
        }
        else
        {
            if (!matchSpace(*(data_ptr + offset), footerspace, tolerance, excess))
                return 0;
        }
        offset++;
    }
    return offset;
}

uint16_t matchGeneric(volatile uint16_t *data_ptr,
                      uint8_t *result_ptr,
                      const uint16_t remaining,
                      const uint16_t nbits,
                      const uint16_t hdrmark,
                      const uint32_t hdrspace,
                      const uint16_t onemark,
                      const uint32_t onespace,
                      const uint16_t zeromark,
                      const uint32_t zerospace,
                      const uint16_t footermark,
                      const uint32_t footerspace,
                      const bool atleast,
                      const uint8_t tolerance,
                      const int16_t excess,
                      const bool MSBfirst)
{
    return _matchGeneric(data_ptr, NULL, result_ptr, false, remaining, nbits,
                         hdrmark, hdrspace, onemark, onespace,
                         zeromark, zerospace, footermark, footerspace, atleast,
                         tolerance, excess, MSBfirst);
}

bool find_protocol(uint8_t max_skip, bool resumed)
{
    // Keep looking for protocols until we've run out of entries to skip or we
    // find a valid protocol message.
    for (uint16_t offset = kStartOffset;
         offset <= (max_skip * 2) + kStartOffset;
         offset += 2)
    {
        // if (decodeDaikin280(offset))
        //     return true;
        if (decodeDaikin216(offset))
            return true;
        // if (decodeHitachiAc296(offset, kHitachiAc296Bits, true))
        //     return true;

        // Giving up
//        if (decodeHash())
//            return true;

//        if (!resumed)
//            resume();
//        return false;
    }
  return false;
}

void crudeNoiseFilter(const uint16_t floor)
{
    if (floor == 0)
        return; // Nothing to do.
    const uint16_t kTickFloor = floor / kRawTick;
    const uint16_t kBufSize = params.bufsize;
    uint16_t offset = kStartOffset;
    while (offset < rawlen && offset + 2 < kBufSize)
    {
        uint16_t curr = rawbuf[offset];
        uint16_t next = rawbuf[offset + 1];
        uint16_t addition = curr + next;
        if (curr < kTickFloor)
        { // Is it too short?
            // Shuffle the buffer down. i.e. Remove the mark & space pair.
            // Note: `memcpy()` can't be used as rawbuf is `volatile`.
            for (uint16_t i = offset + 2; i <= rawlen && i < kBufSize; i++)
                rawbuf[i - 2] = rawbuf[i];
            if (offset > 1)
            { // There is a previous pair we can add to.
                // Merge this pair into into the previous space.
                rawbuf[offset - 1] += addition;
            }
            rawlen -= 2; // Adjust the length.
        }
        else
        {
            offset++; // Move along.
        }
    }
}

void resume()
{
    params.rcvstate = kIdleState;
    params.rawlen = 0;
    params.overflow = false;
}

void copyIrParams(volatile irparams_t *src, irparams_t *dst)
{
    // Typecast src and dst addresses to (char *)
    char *csrc = (char *)src; // NOLINT(readability/casting)
    char *cdst = (char *)dst; // NOLINT(readability/casting)

    // Save the pointer to the destination's rawbuf so we don't lose it as
    // the for-loop/copy after this will overwrite it with src's rawbuf pointer.
    // This isn't immediately obvious due to typecasting/different variable names.
    uint16_t *dst_rawbuf_ptr;
    dst_rawbuf_ptr = dst->rawbuf;

    // Copy contents of src[] to dst[]
    for (uint16_t i = 0; i < sizeof(irparams_t); i++)
        cdst[i] = csrc[i];

    // Restore the buffer pointer
    dst->rawbuf = dst_rawbuf_ptr;

    // Copy the rawbuf
    for (uint16_t i = 0; i < dst->bufsize; i++)
        dst->rawbuf[i] = src->rawbuf[i];
}

bool decode()
{
    irparams_t *save = NULL;
    uint8_t max_skip = 0;
    uint16_t noise_floor = 0;

    // Process only if an IR message been received
    if (params.rcvstate != kStopState)
        return false;

    // Clear the entry we are currently pointing to when we got the timeout.
    // i.e. Stopped collecting IR data.
    if (!params.overflow)
        params.rawbuf[params.rawlen] = 0;

    bool resumed = false; // Flag indicating if we have resumed.

    if (save == NULL)
        save = params_save;
    if (save == NULL)
    {
        rawbuf = params.rawbuf;
        rawlen = params.rawlen;
        overflow = params.overflow;
    }
    else
    {
        copyIrParams(&params, save); // Duplicate the interrupt's memory
        resume();                    // It's now safe to rearm. The IR message won't be overridden.
        resumed = true;

        // Point the results at the saved copy
        rawbuf = save->rawbuf;
        rawlen = save->rawlen;
        overflow = save->overflow;
    }

    // Reset any previously partially processed results
    decode_type = UNKNOWN;
    bits = 0;
    state.value = 0;
    state.address = 0;
    state.command = 0;
    repeat = false;

    crudeNoiseFilter(noise_floor);

    // Keep looking for protocols until we've run out of entries to skip or we
    // find a valid protocol message.
    if (find_protocol(max_skip, resumed))
        return true;
    return false;
}

void IR_recv_gpio_configuration()
{
    gpio_config_t i_conf = {};
    i_conf.intr_type = GPIO_INTR_ANYEDGE;
    i_conf.mode = GPIO_MODE_INPUT;
    i_conf.pin_bit_mask = (1ULL << recvpin);
    i_conf.pull_up_en = 1;
    gpio_config(&i_conf);
}

void recv_setup(const bool save_buffer)
{
    unknown_threshold = MIN_UNKNOWN_SIZE;
    tolerance = min(TOLERANCE_PERC, (uint8_t)100);
    recvpin = IR_CTRL_RECV_PIN;
    IR_recv_gpio_configuration();
}

void listen_for_ir(void *arg)
{
    while(1)
    {
        printf("Interrupt occurred\r\n");
        uint32_t now = esp_timer_get_time();
        static uint32_t start = 0;
        vTaskDelay(1);
        uint16_t rawlen = params.rawlen;
        if (rawlen >= params.bufsize)
        {
            params.overflow = true;
            params.rcvstate = kStopState;
        }

        if (params.rcvstate == kStopState)
            continue;

        if (params.rcvstate == kIdleState)
        {
            params.rcvstate = kMarkState;
            params.rawbuf[rawlen] = 1;
        }
        else
        {
            if (now < start)
                params.rawbuf[rawlen] = (UINT32_MAX - start + now) / kRawTick;
            else
                params.rawbuf[rawlen] = (now - start) / kRawTick;
        }
        params.rawlen++;
        start = now;
        vTaskDelay(1);
    }

}

void gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
//    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void gpio_intr_setup()
{
    // Create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // State GPIO task
    xTaskCreate(listen_for_ir, "listen_for_ir", 2048, NULL, 10, NULL);
    // install gpio isr service
    gpio_install_isr_service(0);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(recvpin, gpio_isr_handler, NULL);
    printf("Minimum free heap size : %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
}

void *recv_handler(void *args)
{
    recv_setup(true); // true - save buffer
    gpio_intr_setup();
    while (1)
    {
    	vTaskDelay(1);
        if (decode())
        {
            uint32_t now = esp_timer_get_time();
            printf("Timestamp : %06lu.%03lu\r\n", now / 1000, now % 1000);
            if (overflow)
                printf(D_WARN_BUFFERFULL "\n", CAPTURE_BUFF_SIZE);
            // Display the tolerance percentage if it has been change from the default.
            printToHumanReadableBasic();
        }
    }
}
