#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "custom_irsend.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include "include/EC200U.h"

#define IR_CTRL_SEND_PIN    22
#define VOL_UP_HEX_VALUE    0x00f9ff00

IRSend_t IRObject;
int cmd;

void app_main(void)
{
	gpio_configuration();
	resetLte();
	LTE_initialization();
	IRObject.IRpin = IR_CTRL_SEND_PIN;
	irsend_configuration(0, 0);
	irsend_begin();
	while(1)
	{
		if(network_flag==0){
			if(MQTT_NetworkOpen(CLIENT_IDX,"54.215.188.103",1883)==2){
				MQTT_NetworkClose(CLIENT_IDX);
			}
		}
		else if(client_flag==0){
			MQTT_ClientConnect(CLIENT_IDX,"QmaxSystems","Qmax_mosquitto_!@#","AC_IR_Control");
			SubscribeTopic(CLIENT_IDX,2,"IR_Commands", 0);
		}
		ReadMessage(CLIENT_IDX);
		vTaskDelay(1);
		ir_send_NEC_command((commands) cmd);
	}
}
