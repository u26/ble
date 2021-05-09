/*
MIT License

Copyright (c) 2021 u26

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// Software I2C
// https://github.com/felias-fogg/SlowSoftI2CMaster

#include <SlowSoftI2CMaster.h>
SlowSoftI2CMaster si = SlowSoftI2CMaster(A4, A5, true); // P0_28 P0_29

//#include <Wire.h>

//float LM75B_get_temp(uint8_t addr){
//
////  Wire.begin();
////  Wire.requestFrom(addr, 2);
//  delay(2);
//    
//  float temp = (((Wire.read()<<8)|Wire.read()) >> 5) * 0.125 ;
//  Wire.end();
//  
//  Serial.println(temp);
//  
//  return temp;
//}

#define P0_02 2 // A0
#define P0_03 3 // A1
#define P0_06 6 // TX --- SERIAL RX
#define P0_07 7
#define P0_08 8 // RX --- SERIAL TX 

#define P0_04 4
#define P0_05 5

#define P0_11 11 // BoardのLED1赤
#define P0_28 28 // A4
#define P0_29 29 // A5
#define P0_30 30 // A6


void setup() {
  
  Serial.begin(115200);
  pinMode(11, OUTPUT);
//  pinMode(2, OUTPUT);
//  pinMode(3, OUTPUT);
//  pinMode(4, OUTPUT);
//  pinMode(5, OUTPUT);
//  pinMode(7, OUTPUT);
//  pinMode(28, OUTPUT);
//  pinMode(29, OUTPUT);
//  pinMode(30, OUTPUT);

  if (!si.i2c_init()) 
    Serial.println(F("Initialization error. SDA or SCL are low"));
  else
    Serial.println(F("...done"));
  
}

float readVal(uint8_t addr)
{
  uint8_t msb, lsb;
 
  if (si.i2c_start(addr | I2C_READ)) {

    msb = si.i2c_read(false);
//    Serial.print(msb, HEX);

    lsb = si.i2c_read(true);
//    Serial.print(lsb, HEX);

    si.i2c_stop();
  }else{
    Serial.print("[ERROR]:readVal");
  }

  return (((msb<<8)|lsb) >> 5) * 0.125 ;
}

void sleepLM75(uint8_t addr, bool enable){

  uint8_t val;

  si.i2c_start(0x90 | I2C_WRITE);
  si.i2c_write(0x01);
  si.i2c_stop();

  si.i2c_start(0x90 | I2C_READ);
  val = si.i2c_read(true);
  si.i2c_stop();
  Serial.print("config:");
  Serial.println(val, HEX);


  if(enable){
    if (si.i2c_start(addr | I2C_WRITE)) {
      si.i2c_write(0x01);
      si.i2c_write( val & 0xfE );
      si.i2c_stop();
      Serial.println("sleep");
    }
  }else{
    if (si.i2c_start(addr | I2C_WRITE)) {
      si.i2c_write(0x01);
      si.i2c_write(val|0x01);
      si.i2c_stop();
      Serial.println("normal");
    }
  }

  
  si.i2c_start(0x90 | I2C_READ);
  val = si.i2c_read(true);
  si.i2c_stop();
  Serial.print("config:");
  Serial.println(val, HEX);


  // pointer registerを温度に戻す
  si.i2c_start(0x90 | I2C_WRITE);
  si.i2c_write(0x00);
  si.i2c_stop();
}

int sw=0;
void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("hello");

  led_check(11);
//  led_check(2);
//  led_check(3);
//  led_check(4);
//  led_check(5);
//  led_check(7);
//  led_check(28);
//  led_check(29);
//  led_check(30);

  char buf[64] = {0};
//  float temp = readVal(0x48);
  float temp = readVal(0x90);
  sprintf(buf, "{\"temp\":%.2f}\n", temp);
  Serial.print(buf);

  if(sw > 0){
    sleepLM75(0x90,true);
    sw = 0;
  }else{
    sleepLM75(0x90,false);
    sw = 1;
  }

  delay(5000);

}



void led_check(int port){

//  Serial.print("LED:");
//  Serial.println( port);

  digitalWrite(port, HIGH);
  delay(100);
  digitalWrite(port, LOW);
//  delay(500);
}
