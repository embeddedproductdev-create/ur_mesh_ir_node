
//MODED 14CORE

#include "SI1133.h"

#define SI1133_SDA 34  //using gpio_34 as SDA
#define SI1133_SCL 35  //using gpio_35 as SCL

SI1133::SI1133() {
  _addr = SI1133_ADDR;
}

boolean SI1133::begin(void) {
Wire.begin(SI1133_SDA, SI1133_SCL);
 
  uint8_t id = read8(SI1133_REG_PARTID);
  if (id != 0x33) return false; // Verify Si1133 i2c Address
  
  reset();

  //Selected Channel 0(16bits) - 1(24bits)
  writeParam(SI1133_PARAM_CHLIST,0X01);

  //=======================================================

  //Configuration for Channel 0
  //Set Photodiode Channel Rate
  writeParam(SI1133_PARAM_ADCCONFIG0,0x78 );

  writeParam(SI1133_PARAM_ADCSENS0,0x09);
  //Resolution of the data
  writeParam(SI1133_PARAM_ADCPSOT0,0x00);
  writeParam(SI1133_PARAM_MEASCONFIG0,COUNT0);

  //=======================================================

  // writeParam(SI1133_PARAM_ADCCONFIG1,0x62 );
  // writeParam(SI1133_PARAM_ADCSENS1,0);
  // writeParam(SI1133_PARAM_ADCPSOT1,BITS_24);
  // writeParam(SI1133_PARAM_MEASCONFIG1,COUNT1);

  write8(SI1133_REG_COMMAND, SI1133_START);
 //Channel0 =uv
 //Channel1 =full ir 
   Serial.println(Wire.read());
   return true;
}

void SI1133::reset() {
//creo q falta reiniciar el irqstatus
  write8(SI1133_REG_COMMAND, SI1133_RESET_SW);
  delay(10);  
}

uint8_t SI1133::read8(uint8_t reg) {
    Wire.beginTransmission(_addr);
    Wire.write((uint8_t)reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)_addr, (uint8_t)1);  
    return Wire.read();
}

uint16_t SI1133::read16(uint8_t a) {
  uint16_t ret;
  Wire.beginTransmission(_addr); // Start data transmission
  Wire.write(a); // Send record to read
  Wire.endTransmission(); // Transmission done
  Wire.requestFrom(_addr, (uint8_t)2);// Sending 2 byte ready to read
  ret = Wire.read(); // Data recieve
  ret |= (uint16_t)Wire.read() << 8;
  return ret;
}
void SI1133::write8(uint8_t reg, uint8_t val) {

  Wire.beginTransmission(_addr); // Initial transmission
  Wire.write(reg); // Sending reg to write
  Wire.write(val); // Write value
  Wire.endTransmission(); // end 
}


uint8_t SI1133::writeParam(uint8_t p, uint8_t v) {

  //Serial.print("Param 0x"); Serial.print(p, HEX);
  //Serial.print(" = 0x"); Serial.println(v, HEX);
  
  write8(SI1133_REG_HOSTIN0, v);
  write8(SI1133_REG_COMMAND, p | SI1133_PARAM_SET);
  return read8(SI1133_REG_RESPONSE1);
}

uint8_t SI1133::readParam(uint8_t p) {
  write8(SI1133_REG_COMMAND, p | SI1133_PARAM_QUERY);
  return read8(SI1133_REG_RESPONSE1);
}

uint32_t SI1133::readUV(void) {
	uint32_t temp;
	temp=read8(SI1133_REG_HOSTOUT0);
	temp<<=8;
	temp|=read8(SI1133_REG_HOSTOUT1);
 	return temp; 
}
uint32_t SI1133::readIR(void) {
	uint32_t temp;
	read8(SI1133_REG_HOSTOUT2);
	temp<<=8;
	temp|=read8(SI1133_REG_HOSTOUT3);
	temp<<=8;
	temp|=read8(SI1133_REG_HOSTOUT4);
 	return temp; 
}

uint32_t SI1133::printOut() {
	uint32_t temp;
	temp = read8(SI1133_REG_HOSTOUT0);
  Serial.print("HOSTOUT0 : ");  Serial.println(temp);
	temp = read8(SI1133_REG_HOSTOUT1);
  Serial.print("HOSTOUT1 : ");  Serial.println(temp);
  temp = read8(SI1133_REG_HOSTOUT2);
  Serial.print("HOSTOUT2 : ");  Serial.println(temp);
  temp = read8(SI1133_REG_HOSTOUT3);
  Serial.print("HOSTOUT3 : ");  Serial.println(temp);
  temp = read8(SI1133_REG_HOSTOUT4);
  Serial.print("HOSTOUT4 : ");  Serial.println(temp);
  temp = read8(SI1133_REG_HOSTOUT5);
  Serial.print("HOSTOUT5 : ");  Serial.println(temp);
  temp = read8(SI1133_REG_HOSTOUT6);
  Serial.print("HOSTOUT6 : ");  Serial.println(temp);
  temp = read8(SI1133_REG_HOSTOUT7);
  Serial.print("HOSTOUT7 : ");  Serial.println(temp);
  temp = read8(SI1133_REG_HOSTOUT8);
  Serial.print("HOSTOUT8 : ");  Serial.println(temp);
  temp = read8(SI1133_REG_HOSTOUT9);
  Serial.print("HOSTOUT9 : ");  Serial.println(temp);
 	return temp; 
}