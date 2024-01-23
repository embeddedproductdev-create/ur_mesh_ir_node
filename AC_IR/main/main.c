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

	//Thread ID initialization
    pthread_t recv_handler_tid;
	pthread_t send_handler_tid;


	//Create Threads
	if(pthread_create(&recv_handler_tid, NULL, recv_handler, NULL) !=0){
        perror("Error in creating modbus_req_handler_thread : ");
    }
	if(pthread_create(&send_handler_tid, NULL, send_handler, NULL) !=0){
        perror("Error in creating modbus_req_handler_thread : ");
    }

	pthread_join(recv_handler_tid, NULL);
	pthread_join(send_handler_tid, NULL);
}




