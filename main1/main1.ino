#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <WiFi.h>

#define IR_SEND_CTRL_PIN        2
#define IR_RECV_CTRL_PIN        33
#define IR_CAPTURE_BUFFER_SIZE  1024
#define IR_CAPTURE_TIMEOUT_MS   50

typedef enum{
  Vol_up,
  Vol_down,
  Up,
  Down,
  Left,
  Right,
  Home,
  Power,
  Back
}commands;

/*WIFI PARAMETERS*/
const char *SSID = "Qmax 2.4GHz";
const char *PWD = "Qmax!2345";

IRsend irsend(IR_SEND_CTRL_PIN);
IRrecv irrecv(IR_RECV_CTRL_PIN, IR_CAPTURE_BUFFER_SIZE, IR_CAPTURE_TIMEOUT_MS, true);

decode_results results;

void send_Ir_commands(commands command)
{
  switch(command)
  {
    case Vol_up:
      Serial.println("Sending Volume Up command ...");
      irsend.sendNEC(0x00f9ff00, 32);
      break;
    case Vol_down:
      Serial.println("Sending Volume down command ... ");
      irsend.sendNEC(0x00f9ba45, 32);
      break;
    case Up:
      Serial.println("Sending D-pad Up command ...");
      irsend.sendNEC(0x00f9c23d, 32);
      break;
    case Down:
      Serial.println("Sending D-pad Down command ...");
      irsend.sendNEC(0x00f950af, 32);
      break;
    case Left:
      Serial.println("Sending D-pad Left command ...");
      irsend.sendNEC(0x00f9609f, 32);
      break;
    case Right:
      Serial.println("Sending D-pad Right command ...");
      irsend.sendNEC(0x00f9708f, 32);
      break;
    case Home:
      Serial.println("Sending Home button command ...");
      irsend.sendNEC(0x00f9e21d, 32);
      break;
    case Power:
      Serial.println("Sending Power button command ...");
      irsend.sendNEC(0x00f9ea15, 32);
      break;
    case Back:
      Serial.println("Sending Back button command ...");
      irsend.sendNEC(0x00f9f2d0, 32);
      break;
    default:
      Serial.println("Invalid IR command !!!");
      break;
  }
}

void recv_Ir_commands(void){
  if (irrecv.decode(&results)) 
  {
    serialPrintUint64(results.value, HEX);
    Serial.println("");
    irrecv.resume();
  }
//  delay(100);
}

void setup(){
  Serial.begin(115200);
  irrecv.enableIRIn();
  irsend.begin();
}

void loop(){
//  send_Ir_commands("Vol_up");
//  delay(1000);
//  send_Ir_commands("Vol_down");
//  delay(1000);
  recv_Ir_commands();
}
