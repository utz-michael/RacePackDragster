/*
  Zeit einstellen Unix zeit berechnen und in format T1421790978 seriell senden
  
  SD card datalogger

 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 10
 	 
 */
#include <TimeLib.h>  
#include <Wire.h>  
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t 
#include <SPI.h>
//#include <AcceleroMMA7361.h> 
#include <SdFat.h>
#include <EasyTransfer.h>
//#define DEBUG   //Debug einschalten verlangsammt 110ms
#define Temperatur
// Rolling average
#define filterSamples   9              // filterSamples should  be an odd number, no smaller than 3
int sensSmoothArray1 [filterSamples];   // array for holding raw sensor values for sensor1 
int sensSmoothArray2 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray3 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray4 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray5 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray6 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray7 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray8 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray9 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray10 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray11 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray12 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray13 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray14 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray15 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray16 [filterSamples];   // array for holding raw sensor values for sensor2 
int sensSmoothArray17 [filterSamples];   // array for holding raw sensor values for sensor2 
// Datum
uint16_t year1 = 2009;
uint8_t month1 = 10;
uint8_t day1 = 1;

// time 20:30:40
uint8_t hour1 = 20;
uint8_t minute1 = 30;
uint8_t second1 = 40;

// serielle übertragung port 1
//create object
EasyTransfer ET;
struct SEND_DATA_STRUCTURE{
//put your variable definitions here for the data you want to send
//THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
float SeriallMain;
float SeriallCarburtor;
float SeriallNos;

  
int   SeriallRPM1;
int   SeriallRPM2 ;
   
float   SeriallMAP;
float   SeriallBAT;
float   SeriallLambda;

int   SeriallEGT1;
int   SeriallEGT2;
   
int   SeriallEGT3;
int   SeriallEGT4;
   
int   SeriallEGT5;
int   SeriallEGT6; 
   
int   SeriallEGT7;
int   SeriallEGT8;
};
//give a name to the group of data
SEND_DATA_STRUCTURE mydata;
//-----------------------------------------------------------------



/*
//Beschleunigungssensor
AcceleroMMA7361 accelero;
int x;
int y;
int z;
*/

SdFat sd;
SdFile myFile;
char filename[] = "LOG00.csv";
boolean Logging = true;

int stream = LOW;
const int chipSelect = SS; // used as the CS pin, the hardware CS pin (10 on most Arduino boards,
unsigned long ZeitOffset = 0;
// Thermoelement 
int Zylinder[8];
int Zylinder_summe[8];
// Bordspannung messung

int BordSpannung = 0;
float BordspannungVolt = 0 ;
int BordspannungPIN = 4;

// Lambda messung

int LambdaAnalog = 0;
float LambdaRaw = 0 ;
int LambdaPIN = 5;




// FuelPressure Sensor


float FuelMainCal = 0; 
float FuelCarburtorCal = 0; 
float FuelNOSCal = 0; 

float FuelMainPSI = 0; 
float FuelCarburtorPSI = 0; 
float FuelNOSPSI = 0; 

int FuelMain = 0; 
int FuelCarburtor = 0; 
int FuelNOS = 0; 

int FuelMainPIN = 0; 
int FuelCarburtorPIN = 1; 
int FuelNOSPIN = 2;





// Umdrehung Motor
int Motordrehzahl = 0;
unsigned long last=0;
unsigned long zeit=30000;
int counter = 0;
int zeitcounter = 0;
unsigned long zeitglatt=0;
unsigned long zeitglatt_neu = 0;
unsigned long zeituebergabe = 15000;
int MotorPIN = 2;

// Umdrehung Kardanwelle
int Kardanwellenrehzahl = 0;
unsigned long last2=0;
unsigned long zeit2=1000000;
int counter2 = 0;
int zeitcounter2 = 0;
unsigned long zeitglatt2=0;
unsigned long zeitglatt_neu2 = 0;
unsigned long zeituebergabe2 = 36450;
float Abrollumfang = 0.319; // 1/8 Abrollumfang hinterreifen 2.553 
int streckencounter=0; // anzahl der impulse der strecke 
int KardanwellePIN = 3;

// MAP Sensor

int MAPPIN = 3;
float MAPCal = 0;
int MAP = 0;
float MAPPSI = 0;

// NOSPressure Sensor

int NOSPressurePIN = 6;
float NOSPressureCal = 0;
int NOSPressure = 0;
float NOSPressurePSI = 0;


// Digital pind für Transbrake und Lachgas
int Transbrake = 25;
//int LachgasFogger = 29;
//int LachgasPlate = 27;

int start = HIGH;
// aufzeichnug
char myChar = 10; // LF für datenstrom
int sampl = 40; // anzahl samles vor dem Speichern
boolean StartAufzeichung = false; //false ; // steuerung der Aufzeichnung
boolean bla = false;
boolean lauf = false;
unsigned long Aufzeichnungsdauer = 0;

void setup()
{
  SPI_SETUP();
  pinMode(chipSelect,OUTPUT);
  digitalWrite(chipSelect, HIGH );
  
  
  
  
  pinMode(31, INPUT); //pin für streaming
   digitalWrite(31, HIGH);
  stream = digitalRead (31); 
 
  pinMode(33, INPUT_PULLUP); //pin für start aufzeichnung
  //digitalWrite(33, HIGH);
   
   pinMode(35, OUTPUT); //pin für start aufzeichnung
  digitalWrite(35, LOW);
 // Open serial communications and wait for port to open:
/* 
// beschleunigung
  accelero.begin(40, 41, 42, 43, A6, A7, A7);
  accelero.setARefVoltage(5);                   //sets the AREF voltage to 3.3V
  accelero.setSensitivity(LOW);                   //sets the sensitivity to +/-6G
  accelero.calibrate();
*/
 
 if (stream == LOW ){
 Serial.begin(9600);
 Serial1.begin(57600);
//start the library, pass in the data details and the name of the serial port. Can be Serial, Serial1, Serial2, etc.
ET.begin(details(mydata), &Serial1 );

 sampl = 0;
 
 //Zeit Setzten
 
 setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time http://www.unixtime.de/ ");      
}
 
 
 
 
 else {
   Serial.begin(9600);
   
   }

  // wait for MAX chip to stabilize
  //delay(500);
//  pinMode(LachgasPlate, INPUT);  // Digital pin als Eingang definieren
//  pinMode(LachgasFogger, INPUT);  // Digital pin als Eingang definieren
  pinMode(Transbrake, INPUT); // Digital pin als Eingang definieren
   pinMode(MotorPIN, INPUT); // Digital pin als Eingang definieren
    pinMode(KardanwellePIN, INPUT); // Digital pin als Eingang definieren
    digitalWrite(KardanwellePIN,HIGH);
    digitalWrite(MotorPIN,HIGH);
  // Kallibrierung Druck Sensoren
  
//  for(int x =0 ; x < 1000 ; x++){

//FuelMainCal = digitalSmooth(analogRead(FuelMainPIN), sensSmoothArray10);
//FuelCarburtorCal =digitalSmooth( analogRead(FuelCarburtorPIN), sensSmoothArray11);
//FuelNOSCal = digitalSmooth(analogRead(FuelNOSPIN), sensSmoothArray12);
//MAPCal =digitalSmooth(analogRead(MAPPIN), sensSmoothArray13);
//NOSPressureCal = digitalSmooth(analogRead(NOSPressurePIN), sensSmoothArray17);
//}

if ( stream == LOW ){
  String dataString = "##;##";
  Serial.println( dataString);
   dataString = "Time;EngineRPM;DriveshaftRPM;Speed;Distance;Transbrake;REVO;FuelMain;FuelCarburator;FuelNOS;NOSPressure;BatteryPower;AFR;EGT 1;EGT 2;EGT 3;EGT 4;EGT 5;EGT 6;EGT 7;EGT 8;";
  Serial.println(dataString);
 }
 else
 {
   setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
  Serial.println("RTC has set the system time");  
  Serial.print("Initializing SD card...");

  // Initialize SdFat or print a detailed error message and halt
  // Use half speed like the native library.
  // change to SPI_FULL_SPEED for more performance.
  if (!sd.begin(chipSelect, SPI_FULL_SPEED)) sd.initErrorHalt();
 
  //Aktuele zeit und Datum einlesen  
 year1 = year();
 month1 = month();
 day1 = day();

// time 20:30:40
 hour1 = hour();
 minute1 = minute();
 second1 = second();
  
  
  SdFile::dateTimeCallback(dateTime);
  
  
   // File creation
 
    for (uint8_t f = 0;  f < 100; f++) {
      filename[3] = f/10 + '0';
      filename[4] = f%10 + '0';
      Serial.println(filename);
      //Serial.println(f);
      if (!sd.exists(filename)) {                // only open a new file if it doesn't exist        
      myFile.open(filename, O_CREAT);  // Create a new file
         
       
         Serial.println("exist"); 
        myFile.close();
       break;                                    // leave the loop!
      }
      
      else {
         Serial.println("not exist"); 
         }
   
    }
 
 
 
  
// datentyp für csv festlegen notwendig füe LiveGraph.2.0.software
  String dataString = "##;##";

  // open the file for write at end like the Native SD library
  if (!myFile.open(filename, O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening LOGxx.csv for write failed");
  }
    myFile.println(dataString);
    
    myFile.close();
 


 

// Überschrift schreiben
 dataString = "Time;EngineRPM;DriveshaftRPM;Speed;Distance;Transbrake;REVO;FuelMain;FuelCarburator;FuelNOS;NOSPressure;BatteryPower;AFR;EGT 1;EGT 2;EGT 3;EGT 4;EGT 5;EGT 6;EGT 7;EGT 8;";
  // open the file for write at end like the Native SD library
  if (!myFile.open(filename, O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening datalog.csv for write failed");
  }
   
    // printTimestamps(myFile);
    myFile.print(dataString);
    digitalClockDisplay(); 
    myFile.close();
 }   

ZeitOffset = millis(); // offset des timers festlegen

 attachInterrupt(0, Motor, FALLING);
 attachInterrupt(1, Kardanwelle, FALLING);
 
}

void loop()
{
  
String dataString = ""; // daten string anlegen
for (int i=0; i <= sampl; i++){ // Daten block zum speichern erzeugen
  
#ifdef Temperatur


   // basic readout test, just print the current temp
  
  
  Zylinder[0] = digitalSmooth(analogRead(8), sensSmoothArray1) ; 
  Zylinder[1] = digitalSmooth(analogRead(9), sensSmoothArray2) ; 
  Zylinder[2] = digitalSmooth(analogRead(10), sensSmoothArray3) ; 
  Zylinder[3] = digitalSmooth(analogRead(11), sensSmoothArray4) ; 
  Zylinder[4] = digitalSmooth(analogRead(12), sensSmoothArray5) ; 
  Zylinder[5] = digitalSmooth(analogRead(13), sensSmoothArray6) ; 
  Zylinder[6] = digitalSmooth(analogRead(14), sensSmoothArray7) ; 
  Zylinder[7] = digitalSmooth(analogRead(15), sensSmoothArray8) ; 
  
  
#endif  
//}

// for (int thermoCS=0; thermoCS <= 7; thermoCS++){
   
//   Zylinder_summe[thermoCS] = Zylinder_summe[thermoCS] / 10 ;
//   }

  
   

//////////////////////////////////////////////////


// Drucksensoren auslesen und berechnen

FuelMain = digitalSmooth(analogRead(FuelMainPIN), sensSmoothArray10);
FuelCarburtor =digitalSmooth( analogRead(FuelCarburtorPIN), sensSmoothArray11);
FuelNOS = digitalSmooth(analogRead(FuelNOSPIN), sensSmoothArray12);
MAP =digitalSmooth(analogRead(MAPPIN), sensSmoothArray13);
BordSpannung =digitalSmooth( analogRead(BordspannungPIN), sensSmoothArray14);
NOSPressure =digitalSmooth( analogRead(NOSPressurePIN), sensSmoothArray17);
/*
FuelMainPSI = (FuelMain - FuelMainCal)* 0.140056;
FuelCarburtorPSI = (FuelCarburtor - FuelCarburtorCal)* 0.140056;
FuelNOSPSI = (FuelNOS - FuelNOSCal)* 0.140056;
NOSPressurePSI = (NOSPressure - NOSPressureCal)* 1.46484375;
*/
FuelMainPSI = (FuelMain - 102) * 0.122549;
FuelCarburtorPSI =(FuelCarburtor - 102) * 0.122549;
FuelNOSPSI = (FuelNOS - 102 ) * 0.122549;
//NOSPressurePSI = NOSPressure* 1.46484375;

//NOSPressurePSI = NOSPressure * 1.5897; Alte Berechnung
NOSPressurePSI = (NOSPressure-102) * 1.8382352; // Berechnung anhand von David Langfield 0.5V - 4.5V = 0 - 1500 PSI

MAPPSI = MAP * 0.0049;// REVO Spannung 
BordspannungVolt = (BordSpannung  * 0.0196)+ 0.839;


char buffer[40];
String FuelMain_PSI = dtostrf(FuelMainPSI, 4, 1, buffer);

String FuelCarburtor_PSI = dtostrf(FuelCarburtorPSI, 4, 1, buffer);

String FuelNOS_PSI = dtostrf(FuelNOSPSI, 4, 1, buffer);

String MAP_PSI = dtostrf(MAPPSI, 6, 2, buffer);

String Bordspannung_Volt = dtostrf(BordspannungVolt, 5, 2, buffer);

//Lambda auslesen

LambdaAnalog = digitalSmooth(analogRead (LambdaPIN), sensSmoothArray9);
LambdaRaw = LambdaAnalog *0.00784 + 10.00;
String Lambda = dtostrf(LambdaRaw, 5, 2, buffer);

// Drehzahlen berechnen

Motordrehzahl = digitalSmooth(15000000/zeituebergabe, sensSmoothArray15);

Kardanwellenrehzahl = digitalSmooth(32175000/zeituebergabe2, sensSmoothArray16);  // auf Annahme am Hinterrad mit 8 impulsen pro umdrehung

#ifdef DEBUG  
   Serial.print("Motor U/min ");
   Serial.println(Motordrehzahl);
   Serial.print("Kardanwelle U/min ");
   Serial.println(Kardanwellenrehzahl);
#endif  




#ifdef DEBUG  
//  Serial.print("fogger");
//  Serial.println(digitalRead(LachgasFogger));
  Serial.print("Transbrake");
  Serial.println(digitalRead(Transbrake));
//  Serial.print("Lachgas Platte");
//  Serial.println(digitalRead(LachgasPlate));
#endif


  // make a string for assembling the data to log:
  
  // read  sensors and append to the string:
  
  dataString += String(millis()- ZeitOffset); // Zeitstempel für Messung eventuell auf microsekunden umstellen
  dataString += ";";
  dataString += String(Motordrehzahl); // Motorumdrehung
  dataString += ";";
  dataString += String(Kardanwellenrehzahl); // Kardanwellenrehzahl
  dataString += ";";
  dataString += String(3600.0/(zeituebergabe2/1000.0)*Abrollumfang ); // Geschwindigkeit
  dataString += ";";
  dataString += String(Abrollumfang*streckencounter); // Strecke
  dataString += ";";
  dataString += String(digitalRead(Transbrake)); // Transbrak
  dataString += ";";
  dataString += String(MAP_PSI); // REVO in Volt
  dataString += ";";
  dataString += String(FuelMain_PSI); // FuelMain
  dataString += ";";
  dataString += String(FuelCarburtor_PSI); // FuelCarburator
  dataString += ";";
  dataString += String(FuelNOS_PSI); // FuelNOS
  dataString += ";";
  dataString += String(NOSPressurePSI); // NOSPressure
  dataString += ";";
  dataString += String(Bordspannung_Volt); // Bordspannung
  dataString += ";";
  dataString += String(Lambda); // Lambda
  //dataString += ";";
 // dataString += String(digitalSmooth(accelero.getXAccel(), sensSmoothArray17)); // X Beschleunigung
  dataString += ";";
    for (int thermoCS = 0; thermoCS <= 7; thermoCS++) {
 //   float  sensor = Zylinder_summe[thermoCS] * 1.8 + 32  ;
     int sensor_farenheit = Zylinder[thermoCS] * 1.764 + 42.0  ; // 0.0049/0.005 = 0.98  >> 0.98* 1.8 = 1.764  
   // String sensor_farenheit = dtostrf(sensor, 5, 0, buffer);
    dataString += String(sensor_farenheit);
    if (thermoCS < 7) {
      dataString += ";"; 
    }
  }
dataString += myChar; // cr linefeed anhängen

}
// Datensatz speichern 
if (stream == LOW ) {
  digitalWrite(35, HIGH);
  Serial.print(dataString);
 
   //this is how you access the variables. [name of the group].[variable name]
   mydata.SeriallMain = FuelMainPSI;
   mydata.SeriallCarburtor = FuelCarburtorPSI;
   mydata.SeriallNos = FuelNOSPSI;
   
  
   
   mydata.SeriallRPM1 = Motordrehzahl; 
   mydata.SeriallRPM2 = Kardanwellenrehzahl; // Kardanwellenrehzahl
   
   mydata.SeriallMAP = MAPPSI; // MAP in PSI
   mydata.SeriallBAT = BordspannungVolt; // Bordspannung
   mydata.SeriallLambda = LambdaRaw; // Lambda
   
   mydata.SeriallEGT1 = Zylinder[0] * 1.764 + 42.0  ;
   mydata.SeriallEGT2 = Zylinder[1] * 1.764 + 42.0  ;
   
   mydata.SeriallEGT3 = Zylinder[2] * 1.764 + 42.0  ;
   mydata.SeriallEGT4 = Zylinder[3] * 1.764 + 42.0  ;
   
   mydata.SeriallEGT5 = Zylinder[4] * 1.764 + 42.0  ;
   mydata.SeriallEGT6 = Zylinder[5] * 1.764 + 42.0  ;
   
   mydata.SeriallEGT7 = Zylinder[6] * 1.764 + 42.0  ;
   mydata.SeriallEGT8 = Zylinder[7] * 1.764 + 42.0  ;
  
   
//send the data
ET.sendData();
 if(Serial.available())
  {
     time_t t = processSyncMessage();
     if(t >0)
     {
        RTC.set(t);   // set the RTC and the system time to the received value
        setTime(t);          
     }
  }
digitalClockDisplaySeriall(); 
  
  
  digitalWrite(35, LOW);
}
else
{

 if (StartAufzeichung == true ){
   digitalWrite(35, HIGH);
  if (!myFile.open(filename, O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening datalog.csv for write failed");
  }
   myFile.print(dataString);
   myFile.close(); 
  }
  }
    // print to the serial port too:
#ifdef DEBUG    
  //  Serial.println(dataString);
#endif  

//start der aufzeichnung und zurücksetzen der zeit auf null
//start = digitalRead (33);


if ( NOSPressurePSI >= 500 && digitalRead(Transbrake) == 1 && StartAufzeichung == false) {
 ZeitOffset = millis(); 
 streckencounter = 0;

StartAufzeichung = true;
}

//aufzeichnung nach 10s automatisch beenden

if (digitalRead(Transbrake) == HIGH && StartAufzeichung == true && bla == false ) {bla= true;}
if (digitalRead(Transbrake) == LOW  && StartAufzeichung == true && bla == true ) {bla=false; lauf = true; Aufzeichnungsdauer = millis();}
if (  millis()- Aufzeichnungsdauer >= 10000  && lauf == true ) { StartAufzeichung = false; }

}


void Motor(){ 
      detachInterrupt(0);                         // Interrupt ausschalten damit er uns nicht beißt
      unsigned long m = micros();                 // Microsekundenzähler auslesen
      unsigned long v = m - last;        // Differenz zum letzten Durchlauf berechnen
  /*    
      
      if (v > 30000 )   { 
       zeit = v;                                // Wert in dauer übernehmen
      last = m; 
    } 
    */
     // if (v > 1600 && v < zeit * 3  ) {        // ignorieren wenn <= 1.6 ms (Kontaktpreller)
      if (v > 1700 ) {        // ignorieren wenn <= 1.6 ms (Kontaktpreller)
      zeit = v;                                // Wert in dauer übernehmen
      last = m;         // und wieder den letzten Wert merken
       zeituebergabe = zeit;    
    /*
     zeitglatt_neu = zeitglatt + zeit;
     zeitglatt = zeitglatt_neu;
     
     zeitcounter ++;

     
     
     if ( zeitcounter >=3) { 
       zeituebergabe = zeitglatt /  zeitcounter ;
       zeitcounter = 0 ;
       zeitglatt = 0;
       zeitglatt_neu = 0;
       }
       */
     //StartAufzeichung = true;  // beim ersten drehen des motors aufzeichung starten  
      }  
      
      attachInterrupt(0, Motor, FALLING );   
     // Interrupt wieder einschalten.
   }
  
void Kardanwelle(){ 
      detachInterrupt(1);                         // Interrupt ausschalten damit er uns nicht beißt
      unsigned long m2 = micros();                 // Microsekundenzähler auslesen
      unsigned long v2 = m2 - last2;                 // Differenz zum letzten Durchlauf berechnen
  
      if (v2 > 4100    ) {            // ignorieren wenn <= 4 ms (Kontaktpreller)
      zeit2 = v2;                                // Wert in dauer übernehmen
      last2 = m2;         // und wieder den letzten Wert merken
      streckencounter++;
      zeituebergabe2 = zeit2;
   /*   
     zeitglatt_neu2 = zeitglatt2 + zeit2;
     zeitglatt2 = zeitglatt_neu2;
     
    zeitcounter2 ++;
//Serial.println(zeit2);
    // zeituebergabe2=zeit2;
     
   if ( zeitcounter2 >=3) { 
      zeituebergabe2 = zeitglatt2 /  zeitcounter2 ;
     zeitcounter2 = 0 ;
      zeitglatt2 = 0;
       zeitglatt_neu2 = 0;
       }
     */   
       } 
        
      attachInterrupt(1, Kardanwelle, FALLING ); 
       // Interrupt wieder einschalten.
   }  
int digitalSmooth(int rawIn, int *sensSmoothArray){     // "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
  int j, k, temp, top, bottom;
  long total;
  static int i;
 // static int raw[filterSamples];
  static int sorted[filterSamples];
  boolean done;

  i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot

  // Serial.print("raw = ");

  for (j=0; j<filterSamples; j++){     // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }

  done = 0;                // flag to know when we're done sorting              
  while(done != 1){        // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filterSamples - 1); j++){
      if (sorted[j] > sorted[j + 1]){     // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j+1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }

/*
  for (j = 0; j < (filterSamples); j++){    // print the array to debug
    Serial.print(sorted[j]); 
    Serial.print("   "); 
  }
  Serial.println();
*/

  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  bottom = max(((filterSamples * 15)  / 100), 1); 
  top = min((((filterSamples * 85) / 100) + 1  ), (filterSamples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j< top; j++){
    total += sorted[j];  // total remaining indices
    k++; 
    // Serial.print(sorted[j]); 
    // Serial.print("   "); 
  }

//  Serial.println();
//  Serial.print("average = ");
//  Serial.println(total/k);
  return total / k;    // divide by number of samples
}

void SPI_SETUP()
{
pinMode(SS, OUTPUT);

// wake up the SPI bus
SPI.begin();

// This device reads MSB first:
SPI.setBitOrder(MSBFIRST);

/*
SPI.setDataMode()
Mode Clock Polarity (CPOL) Clock Phase (CPHA)
SPI_MODE0 0 0
SPI_MODE1 0 1
SPI_MODE2 1 0
SPI_MODE3 1 1
*/
SPI.setDataMode(SPI_MODE0);

/*
SPI.setClockDivider()
sets SPI clock to a fraction of the system clock
Arduino UNO system clock = 16 MHz
Mode SPI Clock
SPI_CLOCK_DIV2 8 MHz
SPI_CLOCK_DIV4 4 MHz
SPI_CLOCK_DIV8 2 MHz
SPI_CLOCK_DIV16 1 MHz
SPI_CLOCK_DIV32 500 Hz
SPI_CLOCK_DIV64 250 Hz
SPI_CLOCK_DIV128 125 Hz
*/

SPI.setClockDivider(SPI_CLOCK_DIV2); // SPI clock 1000Hz
}

void digitalClockDisplay(){
  // digital clock display of the time
  myFile.print(hour());
  printDigits(minute());
  printDigits(second());
  myFile.print(" ");
  myFile.print(day());
  myFile.print("/");
  myFile.print(month());
  myFile.print("/");
  myFile.println(year()); 
  //myFile.print(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  myFile.print(":");
  if(digits < 10)
    myFile.print('0');
  myFile.print(digits);
}
//------------------------------------------------------------------------------
/*
 * User provided date time callback function.
 * See SdFile::dateTimeCallback() for usage.
 */
void dateTime( uint16_t* date, uint16_t* time) {
  // User gets date and time from GPS or real-time
  // clock in real callback function

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(year1, month1, day1);

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(hour1, minute1, second1);
}
//------------------------------------------------------------------------------

/*
 * Function to print all timestamps.
 */
void digitalClockDisplaySeriall(){
  // digital clock display of the time
  Serial.print(hour());
  printDigitsSeriall(minute());
  printDigitsSeriall(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void printDigitsSeriall(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}


/*  code to process time sync messages from the serial port   */
#define TIME_MSG_LEN  11   // time sync to PC is HEADER followed by unix time_t as ten ascii digits
#define TIME_HEADER  'T'   // Header tag for serial time sync message

time_t processSyncMessage() {
  // return the time if a valid sync message is received on the serial port.
  while(Serial.available() >=  TIME_MSG_LEN ){  // time message consists of a header and ten ascii digits
    char c = Serial.read() ; 
    Serial.print(c);  
    if( c == TIME_HEADER ) {       
      time_t pctime = 0;
      for(int i=0; i < TIME_MSG_LEN -1; i++){   
        c = Serial.read();          
        if( c >= '0' && c <= '9'){   
          pctime = (10 * pctime) + (c - '0') ; // convert digits to a number    
        }
      }   
      return pctime; 
    }  
  }
  return 0;
}
