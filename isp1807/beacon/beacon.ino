/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include <bluefruit.h>
#include <Wire.h>

const int LPS22HB = 0x5c;
const int HDC2010 = 0x40;
const int BMX055_ACC = 0x18;
const int BMX055_GYR = 0x68;
const int BMX055_MAG = 0x10;



// Beacon uses the Manufacturer Specific Data field in the advertising
// packet, which means you must provide a valid Manufacturer ID. Update
// the field below to an appropriate value. For a list of valid IDs see:
// https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers
// 0x004C is Apple (for example)
#define MANUFACTURER_ID   0x004C 

// AirLocate UUID: E2C56DB5-DFFB-48D2-B060-D0F5A71096E0
uint8_t beaconUuid[16] = 
{ 
  0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 
  0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 
//  0xE2, 0xC5, 0x6D, 0xB5, 0xDF, 0xFB, 0x48, 0xD2, 
//  0xB0, 0x60, 0xD0, 0xF5, 0xA7, 0x10, 0x96, 0xE0, 
};

// A valid Beacon packet consists of the following information:
// UUID, Major, Minor, RSSI @ 1M
//BLEBeacon beacon(beaconUuid, 0xAAAA, 0x0000, -54);

const int ledPin1 =  4;
const int ledPin2 =  7;


void writeReg(uint8_t addr, uint8_t dat)
{
  Wire.beginTransmission(HDC2010);
  Wire.write(addr);
  Wire.write(dat);
  Wire.endTransmission();
}

void regWrite(int device, uint8_t addr, uint8_t dat)
{
  Wire.beginTransmission(device);
  Wire.write(addr);
  Wire.write(dat);
  Wire.endTransmission();
}
void readReg(uint8_t addr, uint8_t *buf, uint8_t buflen)
{
  Wire.beginTransmission(HDC2010);
  Wire.write(addr);
  Wire.endTransmission(false);
  Wire.requestFrom(HDC2010, buflen);
  while (Wire.available() < buflen)
    delay(1);
  for (int i = 0; i < buflen; i++) {
    *buf = Wire.read(); buf++;
  }
}

void startMeasurement()
{
  writeReg(0x0E, 0x80);// Soft Reset
  //  Wire.beginTransmission(HDC2010Addr);
  //  Wire.write(0x0E); // Configuration Register
  //  Wire.write(0x80); // Soft Reset
  //  Wire.endTransmission();
  delay(10);
  writeReg(0x0E, 0x50); // 1Hz
  writeReg(0x0F, 0x01); // Humidity + Temperature/both 14bit/Start measurement
  //  Wire.beginTransmission(HDC2010Addr);
  //  Wire.write(0x0E); // Configuration Register
  //  Wire.write(0x50); // 1Hz
  //  Wire.write(0x01); // Humidity + Temperature/both 14bit/Start measurement
  //  Wire.endTransmission();
}

float readTemperature()
{
  uint8_t buf[2];
  readReg(0x00, buf, 2);
  int t = buf[0] | buf[1] << 8;
  //  Wire.beginTransmission(HDC2010Addr);
  //  Wire.write(0x00);
  //  Wire.endTransmission(false);
  //  Wire.requestFrom(HDC2010Addr, 2);
  //  while (Wire.available() < 2)
  //    delay(1);
  //  int t = Wire.read() | Wire.read() << 8;
  return (t * 165.0) / 65536.0 - 40;
}

float readHumidity()
{
  uint8_t buf[2];
  readReg(0x02, buf, 2);
  int t = buf[0] | buf[1] << 8;
//  Wire.beginTransmission(HDC2010Addr);
//  Wire.write(0x02);
//  Wire.endTransmission(false);
//  Wire.requestFrom(HDC2010Addr, 2);
//  while (Wire.available() < 2)
//    delay(1);
//  int t = Wire.read() | Wire.read() << 8;
  return (t * 100.0) / 65536.0;
}




void setup() 
{
//  pinMode(ledPin1, OUTPUT);
//  pinMode(ledPin2, OUTPUT);

  Serial.begin(115200);

  Wire.begin();
  regWrite(BMX055_GYR, 0x11, 0x20); // DEEP_SUSPEND mode
  regWrite(BMX055_ACC, 0x11, 0x20); // DEEP_SUSPEND mode
  regWrite(BMX055_MAG, 0x4B, 0x00); // Suspend mode
  regWrite(LPS22HB, 0x10, 0x00);    // Power-down mode

  startMeasurement();
  Wire.end();

  Bluefruit.begin();

  // off Blue LED for lowest power consumption
  Bluefruit.autoConnLed(false);
  Bluefruit.setTxPower(0);    // Check bluefruit.h for supported values
  Bluefruit.setName("MKNOD BLE DEV");
//  Bluefruit.setName("Bluefruit52");

  // Manufacturer ID is required for Manufacturer Specific Data
//  beacon.setManufacturer(MANUFACTURER_ID);

//  // Setup the advertising packet
//  startAdv();
//
//  Serial.println("Broadcasting beacon, open your beacon app to test");
//
//  // Suspend Loop() to save power, since we didn't have any code there
//  suspendLoop();
}


float LM75B_get_temp(uint8_t addr){

  Wire.requestFrom(addr, 2);
  delay(2);
    
  float temp = (((Wire.read()<<8)|Wire.read()) >> 5) * 0.125 ;
  Serial.println(temp);
  
  return temp;
}

uint16_t minor = 0;
uint16_t major = 0xABCD;

void startAdv(void)
{  
  // Advertising packet
  // Set the beacon payload using the BLEBeacon class populated
  // earlier in this example

//  Wire.begin();
//  float temp = readTemperature();
//  float humi = readHumidity();
//  Wire.end();
//
//  int h = temp;
//  uint16_t _h = h;
//  uint16_t _l = humi;
  
//  minor = (_h << 8) | (_l & 0x00ff); 
//  Serial.println(minor,HEX);

//  float temp = LM75B_get_temp(0x48);
//  minor = temp;

  BLEBeacon beacon(beaconUuid, major, minor, -54);
  beacon.setManufacturer(MANUFACTURER_ID);
  Bluefruit.Advertising.setBeacon(beacon);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * Apple Beacon specs
   * - Type: Non connectable, undirected
   * - Fixed interval: 100 ms -> fast = slow = 100 ms
   */
  //Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_ADV_NONCONN_IND);
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(160, 160);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

int cnt=0;
static bool onoff = false;
void loop() 
{

//  Serial.println( cnt++ );
//
//  if (onoff) {
//    digitalWrite(ledPin1, LOW);
//  } else {
//    digitalWrite(ledPin1, HIGH);
//  }
//  onoff = !onoff;

  startAdv();

//  delay(1000);

  delay(60000);
  //  suspendLoop();

}
