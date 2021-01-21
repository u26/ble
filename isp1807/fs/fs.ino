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
File file(InternalFS);

#define FILENAME "/data.txt"

void setup()
{
  Serial.begin(115200);
}

void loop(){

  while (Serial.available()){

    char buf[64];
    char dt = Serial.read();
    if(dt == 'd' ){
      Serial.println("delete");
      deleteFile(FILENAME);
    
    }else if(dt == 'w' ){

      Serial.println("write");
      sprintf(buf,"12345678\n");
      save( FILENAME, buf, strlen(buf));
    
    }else if(dt == 'r' ){

      Serial.println("read");
//      readFile(FILENAME);
      readline(FILENAME);
    }
  }
}

void readline(char* filename){

  InternalFS.begin();
  
  if(file.open(FILENAME, FILE_O_READ)){
  
    Serial.println(FILENAME " file exists");

    uint32_t pos=0;
    uint32_t len=0;
    char buf[128] = {0};
    char c[2]={0};
    while(1){

      len = file.read( c, 1);
      if(len <= 0){
        break;  
      }
      if( c[0] =='\n' ){
        buf[pos] = 0;
        Serial.print(buf);
        pos=0;
      }
      buf[pos] = c[0];
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
    char buffer[64] = { 0 };
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

void save(char* filename, char* buf, int len){

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
