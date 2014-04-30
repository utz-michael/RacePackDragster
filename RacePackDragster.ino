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
#define DEBUG   //Debug einschalten

// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const int chipSelect = 4;

int pin = 30; //Start pin für cs 
int thermoDO = 29;
int thermoCLK = 28;
int Zylinder[8];

// Umdrehung Motor
int umdrehungen = 0;
unsigned long last=0;
unsigned long zeit=60000000UL;
int counter = 0;

// Umdrehung Kardanwelle
int umdrehungen2 = 0;
unsigned long last2=0;
unsigned long zeit2=60000000UL;
int counter2 = 0;


void setup()
{
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
#ifdef DEBUG
    Serial.println("MAX6675 test");
#endif
  // wait for MAX chip to stabilize
  delay(500);

#ifdef DEBUG
  Serial.print("Initializing SD card...");
#endif  
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
#ifdef DEBUG    
    Serial.println("Card failed, or not present");
#endif    
    // don't do anything more:
    return;
  }
#ifdef DEBUG  
  Serial.println("card initialized.");
#endif  
 attachInterrupt(0, zuendung, RISING);
 attachInterrupt(1, KardanWelle, RISING);
}

void loop()
{
 

   // basic readout test, just print the current temp
   for (int thermoCS=0; thermoCS <= 7; thermoCS++){
  MAX6675 thermocouple(thermoCLK, thermoCS+pin, thermoDO);
  Zylinder[thermoCS] = thermocouple.readCelsius();
#ifdef DEBUG  
   Serial.print("Zylinder ");
   Serial.print(thermoCS+1);
   Serial.print("C = "); 
   Serial.println(Zylinder[thermoCS]);
#endif   
  }
  
  if (counter >= 10) {   // Motor umdrehungen berechenen auf basis 1 impuls pro umdrehung und glättung mit 10 messungen
  zeit = zeit / counter;
  umdrehungen = 60000000UL/zeit;
  counter =0 ;
  }
  
   if (counter >= 10) {   // KardanWelle umdrehungen berechenen auf basis 1 impuls pro umdrehung und glättung mit 10 messungen
  zeit2 = zeit2 / counter2;
  umdrehungen2 = 60000000UL/zeit;
  counter2 =0 ;
  }
#ifdef DEBUG  
  Serial.print("Motor U/min ");
  Serial.println(umdrehungen);
   Serial.print("KardanWelle U/min ");
  Serial.println(umdrehungen2);
#endif  
  
  
  // make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
  
  dataString += String(millis());
  dataString += ",";
  dataString += String(umdrehungen);
  dataString += ",";
  dataString += String(umdrehungen2);
  dataString += ",";
  for (int thermoCS = 0; thermoCS <= 8; thermoCS++) {
    int sensor = Zylinder[thermoCS];
    dataString += String(sensor);
    if (thermoCS < 8) {
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
#ifdef DEBUG    
    Serial.println(dataString);
#endif    
  }  
  // if the file isn't open, pop up an error:
  else {
#ifdef DEBUG    
    Serial.println("error opening datalog.txt");
#endif    
  } 
}


void zuendung(){ 
      detachInterrupt(0);                         // Interrupt ausschalten damit er uns nicht beißt
      unsigned long m = micros();                 // Microsekundenzähler auslesen
      unsigned long v = m - last;                 // Differenz zum letzten Durchlauf berechnen
      zeit = zeit + v;
      counter++;
      last = m;       // und wieder den letzten Wert merken
      attachInterrupt(0, zuendung, RISING );    // Interrupt wieder einschalten.
   }
  
void KardanWelle(){ 
      detachInterrupt(1);                         // Interrupt ausschalten damit er uns nicht beißt
      unsigned long m2 = micros();                 // Microsekundenzähler auslesen
      unsigned long v2 = m2 - last2;                 // Differenz zum letzten Durchlauf berechnen
      zeit2 = zeit2 + v2;
      counter2++;
      last2 = m2;       // und wieder den letzten Wert merken
      attachInterrupt(0, KardanWelle, RISING );    // Interrupt wieder einschalten.
   }  

