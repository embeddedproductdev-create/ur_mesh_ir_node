#include "include/main.h"

void sendNEC(uint64_t data, uint16_t nbits, uint16_t repeat)
{
	sendGenericmsgtime(kNecHdrMark, kNecHdrSpace, kNecBitMark, kNecOneSpace, kNecBitMark,
                kNecZeroSpace, kNecBitMark, kNecMinGap, kNecMinCommandLength,
                data, nbits, 40, true, 0, 33);
}

void ir_send_NEC_command(commands command)
{
	switch(command)
	{
		case Vol_up:
			sendNEC(VOLUME_UP_CMD, NUM_OF_BITS_32, NO_REPEAT);
			break;
		case Vol_down:
			sendNEC(VOLUME_DOWN_CMD, NUM_OF_BITS_32, NO_REPEAT);
			break;
		case Up:
			sendNEC(UP_BUTTON_CMD, NUM_OF_BITS_32, NO_REPEAT);
			break;
		case Down:
			sendNEC(DOWN_BUTTON_CMD, NUM_OF_BITS_32, NO_REPEAT);
			break;
		case Left:
			sendNEC(LEFT_BUTTON_CMD, NUM_OF_BITS_32, NO_REPEAT);
			break;
		case Right:
			sendNEC(RIGHT_BUTTON_CMD, NUM_OF_BITS_32, NO_REPEAT);
			break;
		case Home:
			sendNEC(HOME_BUTTON_CMD, NUM_OF_BITS_32, NO_REPEAT);
			break;
		case Power:
			sendNEC(POWER_BUTTON_CMD, NUM_OF_BITS_32, NO_REPEAT);
			break;
		case Back:
			sendNEC(BACK_BUTTON_CMD, NUM_OF_BITS_32, NO_REPEAT);
			break;
		default:
			printf("Invalid NEC command \r\n");
			break;
	}
}