/*
  SD card datalogger
 
 This example shows how to log data from three analog sensors 
 to an SD card using the SD library.
 	
 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 
 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe
 
 This example code is in the public domain.
 	 
 */

#include <SD.h>
#include "max6675.h"
// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const int chipSelect = 4;

int pin = 20; //Start pin für cs 
int thermoDO = 19;
int thermoCLK = 18;
int Zylinder[8];

void setup()
{
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
    Serial.println("MAX6675 test");
  // wait for MAX chip to stabilize
  delay(500);


  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}

void loop()
{
  
   // basic readout test, just print the current temp
   for (int thermoCS=0; thermoCS <= 7; thermoCS++){
  MAX6675 thermocouple(thermoCLK, thermoCS+pin, thermoDO);
  Zylinder[thermoCS] = thermocouple.readCelsius();
   Serial.print("Zylinder ");
   Serial.print(thermoCS+1);
   Serial.print("C = "); 
   Serial.println(Zylinder[thermoCS]);
  }
  
  
  
  
  
  // make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
  
  dataString += String(millis());
  dataString += ",";
  
  for (int thermoCS = 0; thermoCS <= 7; thermoCS++) {
    int sensor = Zylinder[thermoCS];
    dataString += String(sensor);
    if (thermoCS < 7) {
      dataString += ","; 
    }
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  } 
}









