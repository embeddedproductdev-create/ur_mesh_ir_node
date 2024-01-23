#ifndef CUSTOM_IRRECV_H
#define CUSTOM_IRRECV_H

#define IR_CTRL_RECV_PIN 34
#define BAUDRATE 115200
#define CAPTURE_BUFF_SIZE 1024
#define TIMEOUT 50 // milliseconds
#define MIN_UNKNOWN_SIZE 12
#define TOLERANCE_PERC 25

#define MAX_STATE_SIZE 53 // Bytes

/* Receiver states */
#define kIdleState 2
#define kMarkState 3
#define kSpaceState 4
#define kStopState 5
#define kTolerance 25
#define kUseDefTol 255
#define kRawTick 2

#define kStartOffset 1
#define kHeader 2
#define kFooter 2

/* STRING DEFS */
#define D_WARN_BUFFERFULL                                     \
  "WARNING: IR code is too big for buffer (>= %d). "          \
  "This result shouldn't be trusted until this is resolved. " \
  "Edit & increase `kCaptureBufferSize`."

/* VARIABLE DECLARATIONS */
typedef enum decode_type
{
  UNKNOWN = -1,
  UNUSED = 0,
  RC5,
  RC6,
  NEC,
  SONY,
  PANASONIC, // (5)
  JVC,
  SAMSUNG,
  WHYNTER,
  AIWA_RC_T501,
  LG, // (10)
  SANYO,
  MITSUBISHI,
  DISH,
  SHARP,
  COOLIX, // (15)
  DAIKIN,
  DENON,
  KELVINATOR,
  SHERWOOD,
  MITSUBISHI_AC, // (20)
  RCMM,
  SANYO_LC7461,
  RC5X,
  GREE,
  PRONTO, // Technically not a protocol, but an encoding. (25)
  NEC_LIKE,
  ARGO,
  TROTEC,
  NIKAI,
  RAW,         // Technically not a protocol, but an encoding. (30)
  GLOBALCACHE, // Technically not a protocol, but an encoding.
  TOSHIBA_AC,
  FUJITSU_AC,
  MIDEA,
  MAGIQUEST, // (35)
  LASERTAG,
  CARRIER_AC,
  HAIER_AC,
  MITSUBISHI2,
  HITACHI_AC, // (40)
  HITACHI_AC1,
  HITACHI_AC2,
  GICABLE,
  HAIER_AC_YRW02,
  WHIRLPOOL_AC, // (45)
  SAMSUNG_AC,
  LUTRON,
  ELECTRA_AC,
  PANASONIC_AC,
  PIONEER, // (50)
  LG2,
  MWM,
  DAIKIN2,
  VESTEL_AC,
  TECO, // (55)
  SAMSUNG36,
  TCL112AC,
  LEGOPF,
  MITSUBISHI_HEAVY_88,
  MITSUBISHI_HEAVY_152, // 60
  DAIKIN216,
  SHARP_AC,
  GOODWEATHER,
  INAX,
  DAIKIN160, // 65
  NEOCLIMA,
  DAIKIN176,
  DAIKIN128,
  AMCOR,
  DAIKIN152, // 70
  MITSUBISHI136,
  MITSUBISHI112,
  HITACHI_AC424,
  SONY_38K,
  EPSON, // 75
  SYMPHONY,
  HITACHI_AC3,
  DAIKIN64,
  AIRWELL,
  DELONGHI_AC, // 80
  DOSHISHA,
  MULTIBRACKETS,
  CARRIER_AC40,
  CARRIER_AC64,
  HITACHI_AC344, // 85
  CORONA_AC,
  MIDEA24,
  ZEPEAL,
  SANYO_AC,
  VOLTAS, // 90
  METZ,
  TRANSCOLD,
  TECHNIBEL_AC,
  MIRAGE,
  ELITESCREENS, // 95
  PANASONIC_AC32,
  MILESTAG2,
  ECOCLIM,
  XMP,
  TRUMA, // 100
  HAIER_AC176,
  TEKNOPOINT,
  KELON,
  TROTEC_3550,
  SANYO_AC88, // 105
  BOSE,
  ARRIS,
  RHOSS,
  AIRTON,
  COOLIX48, // 110
  HITACHI_AC264,
  KELON168,
  HITACHI_AC296,
  DAIKIN200,
  HAIER_AC160, // 115
  CARRIER_AC128,
  TOTO,
  CLIMABUTLER,
  TCL96AC,
  BOSCH144, // 120
  SANYO_AC152,
  DAIKIN312,
  GORENJE,
  WOWWEE,
  CARRIER_AC84, // 125
  YORK,
  // Add new entries before this one, and update it to point to the last entry.
  kLastDecodeType = YORK,
} decode_type_t;

typedef union state
{
  struct
  {
    uint64_t value;
    uint32_t address;
    uint32_t command;
  };
  uint8_t state_array[MAX_STATE_SIZE];
} state_u;

typedef struct match_result
{
  bool success;  // Was the match successful
  uint64_t data; // The data found
  uint16_t used; // How many buffer positions were used
} match_result_t;

typedef struct irparams
{
  uint8_t recv_pin;  // pin for IR data from detector
  uint8_t rcvstate; // state machine
  uint16_t timer;   // state timer, counts 50uS ticks.
  uint8_t overflow; // Buffer overflow indicator.
  uint8_t timeout;  // Nr. of milliSeconds before we give up.
  uint16_t bits;    // Number of bits in decoded value
  uint16_t bufsize; // max. nr. of entries in the capture buffer.
  uint16_t *rawbuf; // raw data
  uint16_t rawlen;  // counter of entries in rawbuf.
} irparams_t;

/* GLOBAL VARIABLES */
extern uint16_t bits;             // Number of bits in decoded value
extern volatile uint16_t *rawbuf; // Raw intervals in 0.5 us ticks
extern uint16_t rawlen;           // Number of records in rawbuf
extern bool overflow;
extern bool repeat; // Is the result a repeat code?

extern uint16_t unknown_threshold;
extern uint8_t tolerance;
extern volatile irparams_t params;
extern irparams_t *irparams_save;
extern uint8_t recvpin;
extern decode_type_t decode_type;
extern state_u state;

/* FUNCTION DECLARATION */
void *send_handler(void *args);
void recv_setup(const bool save_buffer);
void IR_recv_gpio_configuration();
bool decode();
void IRAM_ATTR gpio_isr_handler(void *arg);
void listen_for_ir(void *arg);
void gpio_intr_setup();
void copyIrParams(volatile irparams_t *src, irparams_t *dst);
void resume();
void crudeNoiseFilter(const uint16_t floor);
bool find_protocol(uint8_t max_skip, bool resumed);
void printToHumanReadableBasic();
char *typeToString();
char *resultToHex();
bool hasACState(const decode_type_t protocol);
char *uint64ToString(uint64_t input, uint8_t base);

/* DECODING FUNCTION DECLARATIONS */
bool decodeDaikin216(uint16_t offset);

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
                       const bool MSBfirst);
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
                      const bool MSBfirst);
bool match(uint32_t measured, uint32_t desired, uint8_t tolerance);
bool matchMark(uint32_t measured, uint32_t desired, uint8_t tolerance, int16_t excess);
bool matchSpace(uint32_t measured, uint32_t desired, uint8_t tolerance, int16_t excess);
uint64_t reverseBits(uint64_t input, uint16_t nbits);
match_result_t matchData(
    volatile uint16_t *data_ptr, const uint16_t nbits, const uint16_t onemark,
    const uint32_t onespace, const uint16_t zeromark, const uint32_t zerospace,
    const uint8_t tolerance, const int16_t excess, const bool MSBfirst,
    const bool expectlastspace);
uint16_t matchBytes(volatile uint16_t *data_ptr, uint8_t *result_ptr,
                    const uint16_t remaining, const uint16_t nbytes,
                    const uint16_t onemark, const uint32_t onespace,
                    const uint16_t zeromark, const uint32_t zerospace,
                    const uint8_t tolerance, const int16_t excess,
                    const bool MSBfirst, const bool expectlastspace);
bool matchAtLeast(uint32_t measured, uint32_t desired,uint8_t tolerance, uint16_t delta);
uint32_t ticksLow(const uint32_t usecs,const uint8_t tolerance, const uint16_t delta);
uint32_t ticksHigh(const uint32_t usecs,const uint8_t tolerance, const uint16_t delta);
uint8_t _validTolerance(const uint8_t percentage);

#endif
