#ifndef _CUSTOM_IRSEND_H
#define _CUSTOM_IRSEND_H

#include <stdint.h>
#include <stdbool.h>

#define LOW 0
#define HIGH 1
#define IR_CTRL_SEND_PIN 22
#define INVERTED_FALSE false
#define USE_MODULATION true

/*OTT Box IR Codes*/
#define VOLUME_UP_CMD 0x00f9ff00
#define VOLUME_DOWN_CMD 0x00f9ba45
#define UP_BUTTON_CMD 0x00f9c23d
#define DOWN_BUTTON_CMD 0x00f950af
#define LEFT_BUTTON_CMD 0x00f9609f
#define RIGHT_BUTTON_CMD 0x00f9708f
#define HOME_BUTTON_CMD 0x00f9e21d
#define BACK_BUTTON_CMD 0x00f9f2d0
#define POWER_BUTTON_CMD 0x00f9ea15

#define NUM_OF_BITS_32 32
#define NO_REPEAT 0

#define MSB_FIRST_TRUE true
#define MSB_FIRST_FALSE false

// function Declarations
void IR_init();
void irsend_configuration(bool inverted, bool use_modulation);
void sendNEC(uint64_t data, uint16_t nbits, uint16_t repeat);
void IR_send_gpio_configuration();
void sendGeneric(const uint16_t headermark, const uint32_t headerspace,
                 const uint16_t onemark, const uint32_t onespace,
                 const uint16_t zeromark, const uint32_t zerospace,
                 const uint16_t footermark, const uint32_t gap,
                 const uint8_t *dataptr, const uint16_t nbytes,
                 const uint16_t frequency, const bool MSBfirst,
                 const uint16_t repeat, const uint8_t dutycycle);
void sendGenericmsgtime(const uint16_t headermark, const uint32_t headerspace,
                        const uint16_t onemark, const uint32_t onespace,
                        const uint16_t zeromark, const uint32_t zerospace,
                        const uint16_t footermark, const uint32_t gap,
                        const uint32_t mesgtime, const uint64_t data,
                        const uint16_t nbits, const uint16_t frequency,
                        const bool MSBfirst, const uint16_t repeat,
                        const uint8_t dutycycle);
void enableIROut(uint32_t freq, uint8_t duty);
uint32_t calcUSecPeriod(uint32_t hz, bool use_offset);
uint64_t min(uint64_t param1, uint64_t param2);
uint64_t max(uint64_t param1, uint64_t param2);
uint16_t mark(uint16_t usec);
void space(uint32_t time);
void sendData(uint16_t onemark, uint32_t onespace, uint16_t zeromark,
              uint32_t zerospace, uint64_t data, uint16_t nbits,
              bool MSBfirst);
uint64_t min(uint64_t param1, uint64_t param2);
uint64_t max(uint64_t param1, uint64_t param2);
uint8_t sumBytes(const uint8_t *const start, const uint16_t length);
void poll_button();
uint8_t control_AC();
void *recv_handler(void *args);


// Variable Declarations
typedef enum
{
	Vol_up_cmd,
	Vol_down_cmd,
	Up_cmd,
	Down_cmd,
	Left_cmd,
	Right_cmd,
	Home_cmd,
	Power_cmd,
	Back_cmd
} OTT_commands;

typedef enum
{
        Bluestar,
		Coolix,
		Daikin,
        Hitachi,
		Voltas,
		Num_of_AC_models
} AC_model;


typedef struct IRsend
{
	uint8_t outputOn;
	uint8_t outputOff;
	uint16_t IRpin;
	uint16_t onTimePeriod;
	uint16_t offTimePeriod;
	int8_t periodOffset;
	uint8_t _dutycycle;
	bool modulation;
} IRSend_t;

extern IRSend_t IRObject;
extern int cmd;
extern char AC_models[20][10];

extern bool button_pressed;
extern bool configured;
extern void (*control_ptr)(void);
extern char curr_selected_protocol[10];

#endif
