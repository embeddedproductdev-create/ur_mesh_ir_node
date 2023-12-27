#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include "include/EC200U.h"
#include <rom/ets_sys.h>
#include "include/custom_irsend.h"
#include "include/custom_daikin.h"
#include "include/custom_hitachi.h"

uint8_t data[35] = {};
IRSend_t IRObject;

void app_main(void)
{
	IRObject.IRpin = IR_CTRL_SEND_PIN;
	IRObject._dutycycle = 75;

	data_init_Daikin280();
	data_init_Hitachi296();

	gpio_configuration();

	irsend_configuration(0, 1);
	irsend_begin();

//	resetLte();
//	LTE_initialization();
//	ConnectToNetwork();

	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;
	xReturned = xTaskCreate(sendDaikin280IRCommand,"SendingIrCommand",4096,( void * )1,tskIDLE_PRIORITY,&xHandle );
	if( xReturned == pdPASS )
	{
		vTaskDelete( xHandle );
	}

	while(1)
	{
		toggle_daikin();
//		send_Hitachi296(NO_REPEAT, MSB_FIRST_TRUE);
//		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}




