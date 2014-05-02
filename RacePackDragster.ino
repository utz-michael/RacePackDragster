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
#include <SdFat.h>
//#include <SD.h>
#include "max6675.h"
//#define DEBUG   //Debug einschalten verlangsammt 110ms

#define Temperatur

//#define X_Beschleunigung
#define Y_Beschleunigung
//#define Z_Beschleunigung
// SD file system.
SdFat sd;

// File for logging data.
SdFile myFile;

// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.
const int chipSelect = 4;
unsigned long ZeitOffset = 0;
// Thermoelement 
int pin = 30; //Start pin für cs 
int thermoDO = 29;
int thermoCLK = 28;
int Zylinder[8];
int TempTimer = 170; // intervall zum abrufen der Temperatur wenn zuschnell kein vernünftiger wert
unsigned long TempMillis = 0;
// Umdrehung Motor
int Motordrehzahl = 0;
unsigned long last=0;
unsigned long zeit=60000000UL;
int counter = 0;

// Umdrehung Kardanwelle
int Kardanwellenrehzahl = 0;
unsigned long last2=0;
unsigned long zeit2=60000000UL;
int counter2 = 0;

// Beschleunigungssensor

float BeschleunigungsKonstante = 0.0602;
int analogPinX = 0;
int analogPinY = 1;
int analogPinZ = 2;
int kalibrierungX = 0;
int kalibrierungY = 0;
int kalibrierungZ = 0;
int i=0;



float X = 0;
float Y = 0;
float Z = 0;

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
//pinMode(53, OUTPUT);                       // set the SS pin as an output (necessary!)
//pinMode(4, OUTPUT);                  // SD select pin
pinMode(10, OUTPUT);                  // Ethernet select pin
digitalWrite(53, LOW);                    // ? (not sure)
//digitalWrite(4, LOW);               // Explicitly enable SD
digitalWrite(10, HIGH);// Explicitly disable Ethernet

  

 
#ifdef DEBUG    
    Serial.println("Card failed, or not present");
#endif    
    // don't do anything more:
    //return;
  
#ifdef DEBUG  
  Serial.println("card initialized.");
#endif  

 // Kallibrierung Beschleunigungssensor
delay (1000);
for ( i=0; i < 50; i++){
kalibrierungX = kalibrierungX + analogRead(analogPinX);
kalibrierungY = kalibrierungY + analogRead(analogPinY);
kalibrierungZ = kalibrierungZ + analogRead(analogPinZ);
}



kalibrierungX = kalibrierungX / i;
kalibrierungY = kalibrierungY / i;
kalibrierungZ = kalibrierungZ / i;



#ifdef DEBUG

  Serial.print("Kalibrierung X: ");
  Serial.println(kalibrierungX);
  Serial.print("Kalibrierung Y: ");
  Serial.println(kalibrierungY);
  Serial.print("Kalibrierung Z: ");
  Serial.println(kalibrierungZ);
delay (5000);

#endif
// Überschrift schreiben


String dataString = "Zeit;Motordrehzahl;Kardanwelle;Beschleunigung;Zylinder 1;Zylinder 2;Zylinder 3;Zylinder 4;Zylinder 5;Zylinder 6;Zylinder 7;Zylinder 8;";

 // Initialize SdFat or print a detailed error message and halt
  // Use half speed like the native library.
  // change to SPI_FULL_SPEED for more performance.
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();

  // open the file for write at end like the Native SD library
  if (!myFile.open("test.txt", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening test.txt for write failed");
  }




    myFile.println(dataString);
    myFile.close();
    

ZeitOffset = millis(); // offset des timers festlegen
 attachInterrupt(0, Motor, RISING);
 attachInterrupt(1, Kardanwelle, RISING);
}

void loop()
{
 
#ifdef Temperatur

if (millis() - TempMillis >= TempTimer ) {
 


   // basic readout test, just print the current temp
   for (int thermoCS=0; thermoCS <= 7; thermoCS++){
  MAX6675 thermocouple(thermoCLK, thermoCS+pin, thermoDO);
  Zylinder[thermoCS] = thermocouple.readCelsius();
  TempMillis = millis ();
  
#ifdef DEBUG  
   Serial.print("Zylinder ");
   Serial.print(thermoCS+1);
   Serial.print("C = "); 
   Serial.println(Zylinder[thermoCS]);
#endif   
  }
#endif  
}
/*
  if (counter >= 10) {   // Motor Motordrehzahl berechenen auf basis 1 impuls pro umdrehung und glättung mit 10 messungen
  zeit = zeit / counter;
  Motordrehzahl = 60000000UL/zeit;
  counter =0 ;
  }
  
   if (counter2 >= 10) {   // Kardanwelle Motordrehzahl berechenen auf basis 1 impuls pro umdrehung und glättung mit 10 messungen
  zeit2 = zeit2 / counter2;
  Kardanwellenrehzahl = 60000000UL/zeit;
  counter2 =0 ;
  }
 */
Motordrehzahl = 60000000UL/zeit;
Kardanwellenrehzahl = 60000000UL/zeit;

 
#ifdef DEBUG  
  Serial.print("Motor U/min ");
  Serial.println(Motordrehzahl);
   Serial.print("Kardanwelle U/min ");
  Serial.println(Kardanwellenrehzahl);
#endif  


#ifdef X_Beschleunigung
X = (analogRead(analogPinX)-kalibrierungX)*BeschleunigungsKonstante;
#endif
#ifdef Y_Beschleunigung
Y = (analogRead(analogPinY)-kalibrierungY)*BeschleunigungsKonstante;
#endif
#ifdef Z_Beschleunigung
Z = (analogRead(analogPinZ)-kalibrierungZ)*BeschleunigungsKonstante;
#endif


#ifdef DEBUG  
  Serial.print("X: ");
  Serial.println(X);
  Serial.print("Y: ");
  Serial.println(Y);
  Serial.print("Z: ");
  Serial.println(Z);
#endif



  
  
  // make a string for assembling the data to log:
  String dataString = "";

  // read  sensors and append to the string:
  
  dataString += String(millis()- ZeitOffset); // Zeitstempel für Messung eventuell auf microsekunden umstellen
  dataString += ";";
  dataString += String(Motordrehzahl); // Motorumdrehung
  dataString += ";";
  dataString += String(Kardanwellenrehzahl); // Kardanwellenrehzahl
  dataString += ";";
  #ifdef X_Beschleunigung
  dataString += String(X); // Beschleunigung X
  dataString += ";";
  #endif
  #ifdef Y_Beschleunigung
  dataString += String(Y); // Beschleunigung X
  dataString += ";";
  #endif
  #ifdef Z_Beschleunigung
  dataString += String(Z); // Beschleunigung X
  dataString += ";";
  #endif
  for (int thermoCS = 0; thermoCS <= 7; thermoCS++) {
    int sensor = Zylinder[thermoCS];
    dataString += String(sensor);
    if (thermoCS < 7) {
      dataString += ";"; 
    }
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
 // File dataFile = SD.open("datalog.csv", FILE_WRITE);
 if (!myFile.open("test.txt", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening test.txt for write failed");
  }
  
    myFile.println(dataString);
    myFile.close();
    
   
    // print to the serial port too:
#ifdef DEBUG    
    Serial.println(dataString);
#endif    
 
  
  // if the file isn't open, pop up an error:

#ifdef DEBUG    
    Serial.println("error opening datalog.csv");
#endif    

  
}


void Motor(){ 
      detachInterrupt(0);                         // Interrupt ausschalten damit er uns nicht beißt
      unsigned long m = micros();                 // Microsekundenzähler auslesen
      unsigned long v = m - last;                 // Differenz zum letzten Durchlauf berechnen
      zeit = zeit + v;  //für glättung ansonsten zeit = v;
      counter++;
      last = m;       // und wieder den letzten Wert merken
      attachInterrupt(0, Motor, RISING );    // Interrupt wieder einschalten.
   }
  
void Kardanwelle(){ 
      detachInterrupt(1);                         // Interrupt ausschalten damit er uns nicht beißt
      unsigned long m2 = micros();                 // Microsekundenzähler auslesen
      unsigned long v2 = m2 - last2;                 // Differenz zum letzten Durchlauf berechnen
      zeit2 = zeit2 + v2; //für glättung ansonsten zeit2 = v;
      counter2++;
      last2 = m2;       // und wieder den letzten Wert merken
      attachInterrupt(0, Kardanwelle, RISING );    // Interrupt wieder einschalten.
   }  

