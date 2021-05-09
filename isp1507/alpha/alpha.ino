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

#include <bluefruit.h>

// Software I2C
// https://github.com/felias-fogg/SlowSoftI2CMaster
#include <SlowSoftI2CMaster.h>
SlowSoftI2CMaster si = SlowSoftI2CMaster(A4, A5, true); // P0_28 P0_29

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




// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
//BLEBas  blebas;  // battery

#define DISCONNECT 0
#define CONNECTED 1
int state = DISCONNECT;
int fname=0;

int data_num=0;

void setup_ble_uart(){

  Serial.println("---------------------------\n");
  Serial.println("BLE UART");
  Serial.println("---------------------------\n");

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behaviour, but provided
  // here in case you want to control this LED manually via PIN 19
  //  Bluefruit.autoConnLed(true);
  Bluefruit.autoConnLed(false);

  // Config the peripheral connection with maximum bandwidth 
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  //  Bluefruit.setTxPower(0);
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("mknod");
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and Start Device Information Service
  bledis.setManufacturer("mknod inc.");
  bledis.setModel("NODE-ISP1807");
  bledis.begin();

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
//  blebas.begin();
//  blebas.write(100);

  // Set up and start advertising
  startAdv();

//  deleteFile(FILENAME);

//
//  Serial.println("Please use Adafruit's Bluefruit LE app to connect in UART mode");
//  Serial.println("Once connected, enter character(s) that you wish to send");

}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
//  Bluefruit.Advertising.restartOnDisconnect(true);
//  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
//  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
//  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  

  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(160, 160);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  

}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);

  state = CONNECTED;
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);

  state = DISCONNECT;

}

/**
 * SENSOR
 */
//float LM75B_get_temp(uint8_t addr){
//
//  Wire.begin();
//  Wire.requestFrom(addr, 2);
//  delay(2);
//    
//  float temp = (((Wire.read()<<8)|Wire.read()) >> 5) * 0.125 ;
//  Wire.end();
//  
//  Serial.println(temp);
//  
//  return temp;
//}

void setup() {
  
  Serial.begin(115200);
  pinMode(11, OUTPUT);

  if (!si.i2c_init()){
    Serial.println(F("Initialization error. SDA or SCL are low"));
  }else{
    Serial.println(F("...done"));
  }
  setup_ble_uart();
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


void loop() {

  led_check(11);

  // wake up
  sleepLM75(0x90,false);

  char buf[64] = {0};
//  float temp = readVal(0x48);
  float temp = readVal(0x90);
  sprintf(buf, "{\"temp\":%.2f}\n", temp);
  Serial.print(buf);
  
  int cnt = strlen(buf);

  if( state == CONNECTED ){
    
    Serial.println("send");
    bleuart.write( buf, cnt );
  }

  // sleep
  sleepLM75(0x90,true);
  delay(5000);

}


void led_check(int port){

//  Serial.print("LED:");
//  Serial.println( port);

  digitalWrite(port, HIGH);
  delay(100);
  digitalWrite(port, LOW);
}
