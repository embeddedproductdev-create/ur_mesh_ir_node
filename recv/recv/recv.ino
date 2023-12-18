#include "IRremote.hpp"

const int irReceiverPin = 33;

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(irReceiverPin, ENABLE_LED_FEEDBACK);
}

void loop() {
  if (IrReceiver.decode()) {
    IrReceiver.printIRResultShort(&Serial);
    IrReceiver.resume();
  }
}
