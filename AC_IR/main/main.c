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

#define IR_CTRL_SEND_PIN    22
//uint8_t data[35] = {0x11, 0xDA, 0x27, 0x00, 0xC5, 0x00, 0x00, 0xD7, 0x11, 0xDA, 0x27, 0x00,
//		0x42, 0x00, 0x00, 0x54, 0x11, 0xDA, 0x27, 0x00, 0x00, 0x38, 0x2A, 0x00, 0x4F, 0x00,
//		0x00, 0x06, 0x60, 0x00, 0x00, 0xC0, 0x00, 0x00, 0xE9};
uint8_t data[35];
IRSend_t IRObject;

void init_data()
{
	for(uint8_t index=0; index<35; index++)
	{
		data[index] = 0x00;
	}
	data[0] = 0x11;
	data[1] = 0xda;
	data[2] = 0x27;
	data[4] = 0xc5;
	data[8] = 0x11;
	data[9] = 0xda;
	data[10] = 0x27;
	data[12] = 0x42;
	data[16] = 0x11;
	data[17] = 0xDA;
	data[18] = 0x27;
	data[21] = 0x49;
	data[22] = 0x1E;
	data[24] = 0xB0;
	data[27] = 0x06;
    data[28] = 0x60;
    data[31] = 0xC0;
    daikin_280_checksum();
}

void app_main(void)
{
	init_data();
	gpio_configuration();
//	resetLte();
//	LTE_initialization();
	IRObject.IRpin = IR_CTRL_SEND_PIN;
	IRObject.modulation = true;
	irsend_configuration(0, 0);
	irsend_begin();
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
//		vTaskDelay(pdMS_TO_TICKS(500)); //0.5s delay
//		ir_send_NEC_command(0);
//		vTaskDelay(pdMS_TO_TICKS(500)); //0.5s delay
//		ir_send_NEC_command(1);
//		vTaskDelay(pdMS_TO_TICKS(500)); //0.5s delay
		/*Send Daikin AC command*/
//		setPower(false);
//		sendDaikin280IRCommand((void *)1);
//		vTaskDelay(pdMS_TO_TICKS(5000)); //5s delay
//		init_data();
//		setPower(true);
		setTemp(25);
		sendDaikin280IRCommand((void *)1);
		vTaskDelay(pdMS_TO_TICKS(5000)); //5s delay
		setTemp(20);
//		init_data();
//		setTemp(24);
		sendDaikin280IRCommand((void *)1);
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}




