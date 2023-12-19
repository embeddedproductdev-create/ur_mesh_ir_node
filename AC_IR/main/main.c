#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "custom_irsend.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#define IR_CTRL_SEND_PIN    26
#define VOL_UP_HEX_VALUE    0x00f9ff00
#define NUM_OF_BITS		    32
#define NO_REPEAT		    0

const char *TAG = "[DEBUG]";

IRSend_t IRObject;

void app_main(void)
{
	IRObject.IRpin = IR_CTRL_SEND_PIN;
	irsend_configuration(0, 0);
	irsend_begin();
	while(1)
	{
		printf("Sending Volume Down command\n");
		sendNEC(VOL_UP_HEX_VALUE, NUM_OF_BITS, NO_REPEAT);
		printf("b\n");
		vTaskDelay(100); //1s delay
	}
}
