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

bool button_pressed = false;

uint8_t curr_temp_daikin280 = 25;
uint8_t curr_daikinPower = false;
int8_t curr_daikinMode = DaikinAuto;
int8_t curr_daikinFan = DaikinFanMin;

void app_main(void)
{
	IRObject.IRpin = IR_CTRL_SEND_PIN;
	IRObject._dutycycle = 75;

	data_init_Daikin280();
	data_init_Hitachi296();

	LTE_gpio_configuration();

	irsend_configuration(INVERTED_FALSE, USE_MODULATION);
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
//		toggle_daikin();
//		send_Hitachi296(NO_REPEAT, MSB_FIRST_TRUE);
//		vTaskDelay(pdMS_TO_TICKS(5000));
		if(!gpio_get_level(TEMP_INC_BUTTON))
		{
			button_pressed = true;
			printf("Temp_inc_button pressed\r\n");
			setTemp_Daikin280(++curr_temp_daikin280);
		}
		else if(!gpio_get_level(TEMP_DEC_BUTTON))
		{
			button_pressed = true;
			printf("Temp_dec_button pressed\r\n");
			setTemp_Daikin280(--curr_temp_daikin280);
		}
		else if(!gpio_get_level(POWER_BUTTON))
		{
			button_pressed = true;
			printf("Power button pressed\r\n");
			if(curr_daikinPower)
				curr_daikinPower = 0;
			else
				curr_daikinPower = 1;
			printf("curr_daikinPower : %d\r\n",curr_daikinPower);
			printf("data[21] : %x\r\n",data[21]);
			setPower_Daikin280(curr_daikinPower);
		}
		else if(!gpio_get_level(MODE_BUTTON))
		{
			button_pressed = true;
			switch(curr_daikinMode)
			{
				case DaikinAuto:
					setMode_Daikin280(curr_daikinMode);
					curr_daikinMode = DaikinDry;
					break;
				case DaikinDry:
					setMode_Daikin280(curr_daikinMode);
					curr_daikinMode = DaikinCool;
					break;
				case DaikinCool:
					setMode_Daikin280(curr_daikinMode);
					curr_daikinMode = DaikinHeat;
					break;
				case DaikinHeat:
					setMode_Daikin280(curr_daikinMode);
					curr_daikinMode = DaikinFan;
					break;
				case DaikinFan:
					setMode_Daikin280(curr_daikinMode);
					curr_daikinMode = DaikinAuto;
					break;
				default:
					printf("Invalid Mode\r\n");
					break;
			}

			printf("mode button pressed\r\n");
		}
		else if(!gpio_get_level(FAN_BUTTON))
		{
			button_pressed = true;
			switch(curr_daikinFan)
			{
				case DaikinFanMin:
					setFan_Daikin280(curr_daikinFan);
					curr_daikinFan = DaikinFanMed;
					break;
				case DaikinFanMed:
					setFan_Daikin280(curr_daikinFan);
					curr_daikinFan = DaikinFanMax;
					break;
				case DaikinFanMax:
					setFan_Daikin280(curr_daikinFan);
					curr_daikinFan = DaikinFanAuto;
					break;
				case DaikinFanAuto:
					setFan_Daikin280(curr_daikinFan);
					curr_daikinFan = DaikinFanQuiet;
					break;
				case DaikinFanQuiet:
					setFan_Daikin280(curr_daikinFan);
					curr_daikinFan = DaikinFanMin;
					break;
				default:
					printf("Invalid Fan Mode \r\n");
					break;
			}

			printf("fan button pressed\r\n");
		}
		if(button_pressed)
		{
			button_pressed = false;
			daikin_280_checksum();
			sendDaikin280IRCommand((void *)1);
			vTaskDelay(10);
			continue;
		}
		vTaskDelay(1);
	}
}




