#include "include/main.h"

#define TAG "UART"
bool button_pressed = false;

void app_main(void)
{
	IR_init();

	data_init_Daikin280();
	data_init_Daikin216();
	data_init_Hitachi296();

	LTE_gpio_configuration();

//	resetLte();
//	LTE_initialization();
//	ConnectToNetwork();

	while(1)
	{
		poll_button();
		vTaskDelay(1);
	}
}




