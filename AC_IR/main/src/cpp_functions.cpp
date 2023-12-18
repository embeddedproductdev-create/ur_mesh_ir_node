/*
 * cpp_functions.cpp
 *
 *  Created on: 18-Dec-2023
 *      Author: EmbeddedDevelopment
 */
#include "../include/linker.h"
#include "../include/cpp_functions.h"

extern "C" {

IRsend irsend(IR_SEND_CTRL_PIN);

//IRsend irsend;
// IRrecv irrecv();
// decode_results results;

//IRsend irsend(IR_SEND_CTRL_PIN);

void loop()
{
//	irrecv.enableIRIn();
//	irsend.begin();
	while (1) {
		recv_Ir_commands();
	}
}
void recv_Ir_commands(void){
//  if (irrecv.decode(&results))
//  {
//	irrecv.resume();
//  }
	;
}

//void send_Ir_commands(commands command)
//{
//  switch(command)
//  {
//	case Vol_up:
//	  irsend.sendNEC(0x00f9ff00, 32);
//	  break;
//	case Vol_down:
//	  irsend.sendNEC(0x00f9ba45, 32);
//	  break;
//	case Up:
//	  irsend.sendNEC(0x00f9c23d, 32);
//	  break;
//	case Down:
//	  irsend.sendNEC(0x00f950af, 32);
//	  break;
//	case Left:
//	  irsend.sendNEC(0x00f9609f, 32);
//	  break;
//	case Right:
//	  irsend.sendNEC(0x00f9708f, 32);
//	  break;
//	case Home:
//	  irsend.sendNEC(0x00f9e21d, 32);
//	  break;
//	case Power:
//	  irsend.sendNEC(0x00f9ea15, 32);
//	  break;
//	case Back:
//	  irsend.sendNEC(0x00f9f2d0, 32);
//	  break;
//	default:
//	  break;
//  }
//}
}



