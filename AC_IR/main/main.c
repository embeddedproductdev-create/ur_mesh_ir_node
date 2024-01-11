#include "include/main.h"

/*GLOBAL VARIABLE INITIALIZATION*/
bool button_pressed = false;
bool configured = false;
char json_packet[100] = "";

void app_main(void)
{
	IR_init();

	data_init_Daikin200();
	data_init_Daikin280();
	data_init_Daikin216();
	data_init_Hitachi296();

	LTE_gpio_configuration();

	resetLte();
	LTE_initialization();
	ConnectToNetwork();

	while(1)
	{
		ReadMessage(CLIENT_IDX);
		parse_and_control();
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}




