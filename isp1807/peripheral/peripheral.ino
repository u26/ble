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
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
#include <Wire.h>

using namespace Adafruit_LittleFS_Namespace;

#define FILENAME "/data.txt"
File file(InternalFS);

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

void setup()
{
  Serial.begin(115200);
  delay(100);
 
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

void send( char* buf,int cnt){
  
  bleuart.write( buf, cnt );
}

void loop()
{
//  startAdv();

  char buf[64] = {0};
  char buf2[64] = {0};
  char* bufp = buf;


//  float temp = LM75B_get_temp(0x48);
//  sprintf(buf, "{\"temp\":%.2f}", temp);
//  int cnt = strlen(buf);
//
//  if( state == CONNECTED ){
//    send(buf, cnt);
//  }  

  float temp = LM75B_get_temp(0x48);
  sprintf(buf, "{\"temp\":%.2f}\n", temp);
  Serial.print(buf);

  int cnt = strlen(buf);

  if( state == CONNECTED ){

    Serial.println("read");
    readline(FILENAME);
    deleteFile(FILENAME);

    Serial.println("send");
    send(buf, cnt);
    
  }else{
    
    Serial.println("save");
    saveFile( FILENAME, buf, cnt);
  }

  delay(10000);
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
float LM75B_get_temp(uint8_t addr){

  Wire.begin();
  Wire.requestFrom(addr, 2);
  delay(2);
    
  float temp = (((Wire.read()<<8)|Wire.read()) >> 5) * 0.125 ;
  Wire.end();
  
  Serial.println(temp);
  
  return temp;
}


/**
 * FS
 */
void readline(char* filename){

  InternalFS.begin();
  
  if(file.open(FILENAME, FILE_O_READ)){
  
    Serial.println(FILENAME " file exists");

    uint32_t pos=0;
    uint32_t len=0;
    char buf[128] = {0};
    char c[2] = {0};
    
    while(1){

      len = file.read( c, 1);
      if(len <= 0){
        break;  
      }

      buf[pos] = c[0];

      if( c[0] =='\n' ){
        
        buf[pos] = 0;
        len = strlen(buf);

        Serial.println(buf);
        Serial.println(len);
        send(buf, len);
        pos=0;
      }
      pos++;
    }
      
    file.close();
  }
  
  InternalFS.end();
  
}

void deleteFile(char* filename){

  InternalFS.begin();

  InternalFS.remove(filename);
  
  InternalFS.end();
}

void readFile(char* filename){

  InternalFS.begin();
  
  if(file.open(FILENAME, FILE_O_READ)){
  
    Serial.println(FILENAME " file exists");

    uint32_t readlen;
    char buffer[128] = { 0 };
    while(1){

      readlen = file.read(buffer, sizeof(buffer));
      if(readlen<=0){
        break;  
      }
      buffer[readlen] = 0;
      Serial.println(buffer);
    }
      
    file.close();
  }
  
  InternalFS.end();
  
}

void saveFile(char* filename, char* buf, int len){

  InternalFS.begin();
 
  if(file.open(filename, FILE_O_WRITE)){
    
    Serial.println("Save");
    file.write(buf, len);
    file.close();
    
  } else {
    
    Serial.println("Failed!");
  }
  
  InternalFS.end();
}
