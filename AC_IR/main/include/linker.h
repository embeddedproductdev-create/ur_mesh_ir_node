/*
 * linker.h
 *
 *  Created on: 18-Dec-2023
 *      Author: EmbeddedDevelopment
 */

#ifndef MAIN_LINKER_H_
#define MAIN_LINKER_H_

#include "../lib/IRremoteESP8266/IRremoteESP8266.h"
#include "../lib/IRremoteESP8266/IRsend.h"
#include "../lib/IRremoteESP8266/IRrecv.h"
#include "../lib/IRremoteESP8266/IRutils.h"

#define IR_SEND_CTRL_PIN        2
#define IR_RECV_CTRL_PIN        33
#define IR_CAPTURE_BUFFER_SIZE  1024
#define IR_CAPTURE_TIMEOUT_MS   50

/*WIFI PARAMETERS*/
//const char *SSID = "Qmax 2.4GHz";
//const char *PWD = "Qmax!2345";

#endif /* MAIN_LINKER_H_ */
