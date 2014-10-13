/*
  SD card datalogger

 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 10
 	 
 */
#include <AcceleroMMA7361.h> 
#include <SdFat.h>
#include <EasyTransfer.h>
//#define DEBUG   //Debug einschalten verlangsammt 110ms
#define Temperatur
// Rolling average
#define filterSamples   10              // filterSamples should  be an odd number, no smaller than 3
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





// serielle übertragung port 1
//create object
EasyTransfer ET;
struct SEND_DATA_STRUCTURE{
//put your variable definitions here for the data you want to send
//THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
int SeriallMain;
int SeriallCarburtor;
int SeriallNos;
};
//give a name to the group of data
SEND_DATA_STRUCTURE mydata;
//-----------------------------------------------------------------



//Beschleunigungssensor
AcceleroMMA7361 accelero;
int x;
int y;
int z;


SdFat sd;
SdFile myFile;
byte stream = LOW;
const int chipSelect = 10; // used as the CS pin, the hardware CS pin (10 on most Arduino boards,
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
float Abrollumfang = 2.553; // Abrollumfang hinterreifen
int streckencounter=0; // anzahl der impulse der strecke 
int KardanwellePIN = 3;
// MAP Sensor

int MAPPIN = 3;
float MAPCal = 0;
int MAP = 0;
float MAPPSI = 0;


// Digital pind für Transbrake und Lachgas
int Transbrake = 25;
int LachgasFogger = 29;
int LachgasPlate = 27;

int start = HIGH;
// aufzeichnug
char myChar = 10; // LF für datenstrom
int sampl = 5; // anzahl samles vor dem Speichern
boolean StartAufzeichung = false; //false ; // steuerung der Aufzeichnung


void setup()
{
   pinMode(31, INPUT); //pin für streaming
   digitalWrite(31, HIGH);
   stream = digitalRead (31); 
 
  pinMode(33, INPUT); //pin für start aufzeichnung
  digitalWrite(33, HIGH);
   
   pinMode(35, OUTPUT); //pin für start aufzeichnung
  digitalWrite(35, LOW);
 // Open serial communications and wait for port to open:
 

 
 if (stream == LOW ){
 Serial.begin(9600);
 Serial1.begin(57600);
//start the library, pass in the data details and the name of the serial port. Can be Serial, Serial1, Serial2, etc.
ET.begin(details(mydata), &Serial1 );

 sampl = 0;
 }
 else {
   Serial.begin(9600);
   
   }
// beschleunigung
  accelero.begin(40, 41, 42, 43, A6, A7, A7);
  accelero.setARefVoltage(5);                   //sets the AREF voltage to 3.3V
  accelero.setSensitivity(LOW);                   //sets the sensitivity to +/-6G
  accelero.calibrate();


  // wait for MAX chip to stabilize
  delay(500);
  pinMode(LachgasPlate, INPUT);  // Digital pin als Eingang definieren
  pinMode(LachgasFogger, INPUT);  // Digital pin als Eingang definieren
  pinMode(Transbrake, INPUT); // Digital pin als Eingang definieren
   pinMode(MotorPIN, INPUT); // Digital pin als Eingang definieren
    pinMode(KardanwellePIN, INPUT); // Digital pin als Eingang definieren
    digitalWrite(KardanwellePIN,HIGH);
    digitalWrite(MotorPIN,HIGH);
  // Kallibrierung Druck Sensoren
  
  for(int x =0 ; x < 1000 ; x++){
/* 
 FuelMainCal = FuelMainCal + analogRead(FuelMainPIN); 
 FuelCarburtorCal = FuelCarburtorCal + analogRead(FuelCarburtorPIN); 
 FuelNOSCal = FuelNOSCal + analogRead(FuelNOSPIN); 
 MAPCal = MAPCal + analogRead(MAPPIN); 
}

/*  
 FuelMainCal = (int)((FuelMainCal /1000)+ .5);
 FuelCarburtorCal = (int)((FuelCarburtorCal /1000)+ .5); 
 FuelNOSCal = (int)((FuelNOSCal /1000)+ .5); 
 MAPCal = (int)((MAPCal /1000)+ .5);

FuelMainCal = 0;
 FuelCarburtorCal = 0;
 FuelNOSCal = 0;
 MAPCal = 0;
 */
FuelMainCal = digitalSmooth(analogRead(FuelMainPIN), sensSmoothArray10);
FuelCarburtorCal =digitalSmooth( analogRead(FuelCarburtorPIN), sensSmoothArray11);
FuelNOSCal = digitalSmooth(analogRead(FuelNOSPIN), sensSmoothArray12);
MAPCal =digitalSmooth(analogRead(MAPPIN), sensSmoothArray13);

}
if ( stream == LOW ){
  String dataString = "##;##";
  Serial.println( dataString);
   dataString = "Zeit;Motordrehzahl;Kardanwelle;Geschwindigkeit;Strecke;Transbrake;LachgasFogger;LachgasPlate;MAP;FuelMain;FuelCarburator;FuelNOS;BordSpannung;Lambda;AccelX;Zylinder 1;Zylinder 2;Zylinder 3;Zylinder 4;Zylinder 5;Zylinder 6;Zylinder 7;Zylinder 8;";
  Serial.println(dataString);
 }
 else
 {
  Serial.print("Initializing SD card...");
 
  // Initialize SdFat or print a detailed error message and halt
  // Use half speed like the native library.
  // change to SPI_FULL_SPEED for more performance.
  if (!sd.begin(chipSelect, SPI_FULL_SPEED)) sd.initErrorHalt();
  
// datentyp für csv festlegen notwendig füe LiveGraph.2.0.software
  String dataString = "##;##";
  // open the file for write at end like the Native SD library
  if (!myFile.open("datalog.csv", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening datalog.csv for write failed");
  }
    myFile.println(dataString);
    myFile.close();
 


 

// Überschrift schreiben
 dataString = "Zeit;Motordrehzahl;Kardanwelle;Geschwindigkeit;Strecke;Transbrake;LachgasFogger;LachgasPlate;MAP;FuelMain;FuelCarburator;FuelNOS;BordSpannung;Lambda;AccelX;Zylinder 1;Zylinder 2;Zylinder 3;Zylinder 4;Zylinder 5;Zylinder 6;Zylinder 7;Zylinder 8;";
  // open the file for write at end like the Native SD library
  if (!myFile.open("datalog.csv", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening datalog.csv for write failed");
  }
    myFile.println(dataString);
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




// Drucksensoren auslesen und berechnen

FuelMain = digitalSmooth(analogRead(FuelMainPIN), sensSmoothArray10);
FuelCarburtor =digitalSmooth( analogRead(FuelCarburtorPIN), sensSmoothArray11);
FuelNOS = digitalSmooth(analogRead(FuelNOSPIN), sensSmoothArray12);
MAP =digitalSmooth(analogRead(MAPPIN), sensSmoothArray13);
BordSpannung =digitalSmooth( analogRead(BordspannungPIN), sensSmoothArray14);

FuelMainPSI = (FuelMain - FuelMainCal)/7.14;
FuelCarburtorPSI = (FuelCarburtor - FuelCarburtorCal)/7.14;
FuelNOSPSI = (FuelNOS - FuelNOSCal)/7.14;
MAPPSI = (MAP - MAPCal)/10.87;
BordspannungVolt = BordSpannung * 3.0000 * 0.0049;


char buffer[40];
String FuelMain_PSI = dtostrf(FuelMainPSI, 4, 1, buffer);

String FuelCarburtor_PSI = dtostrf(FuelCarburtorPSI, 4, 1, buffer);

String FuelNOS_PSI = dtostrf(FuelNOSPSI, 4, 1, buffer);

String MAP_PSI = dtostrf(MAPPSI, 6, 2, buffer);

String Bordspannung_Volt = dtostrf(BordspannungVolt, 5, 2, buffer);

//Lambda auslesen

LambdaAnalog = digitalSmooth(analogRead (LambdaPIN), sensSmoothArray9);
LambdaRaw = 0.12 * LambdaAnalog *0.0049 + 0.7;
String Lambda = dtostrf(LambdaRaw, 4, 2, buffer);

// Drehzahlen berechnen

Motordrehzahl = digitalSmooth(60000000/zeituebergabe/4, sensSmoothArray15);

Kardanwellenrehzahl = digitalSmooth(36450000/zeituebergabe2, sensSmoothArray16);  // auf Annahme am Hinterrad mit 8 impulsen pro umdrehung

#ifdef DEBUG  
   Serial.print("Motor U/min ");
   Serial.println(Motordrehzahl);
   Serial.print("Kardanwelle U/min ");
   Serial.println(Kardanwellenrehzahl);
#endif  




#ifdef DEBUG  
  Serial.print("fogger");
  Serial.println(digitalRead(LachgasFogger));
  Serial.print("Transbrake");
  Serial.println(digitalRead(Transbrake));
  Serial.print("Lachgas Platte");
  Serial.println(digitalRead(LachgasPlate));
#endif

/*
 // Zeit und Streckencounter auf null setzen wenn Transbrake gelöst wird
 
if (digitalRead(Transbrake)== 1) { start = 1;}
if (digitalRead(Transbrake)== 0 && start == 1) { start = 2;}
if (start == 2) { 
    start = 0;   
    ZeitOffset = millis(); 
  streckencounter = 0;
} 
*/  
  // make a string for assembling the data to log:
  
  // read  sensors and append to the string:
  
  dataString += String(millis()- ZeitOffset); // Zeitstempel für Messung eventuell auf microsekunden umstellen
  dataString += ";";
  dataString += String(Motordrehzahl); // Motorumdrehung
  dataString += ";";
  dataString += String(Kardanwellenrehzahl); // Kardanwellenrehzahl
  dataString += ";";
  dataString += String(3600.0/(zeituebergabe2/1000.0)*Abrollumfang/8.0 ); // Geschwindigkeit
  dataString += ";";
  dataString += String(Abrollumfang/8*streckencounter); // Strecke
  dataString += ";";
  dataString += String(digitalRead(Transbrake)); // Transbrak
  dataString += ";";
  dataString += String(digitalRead(LachgasFogger)); // LachgasFogger
  dataString += ";";
  dataString += String(digitalRead(LachgasPlate)); // LachgasPlate
  dataString += ";";
  dataString += String(MAP_PSI); // MAP in PSI
  dataString += ";";
  dataString += String(FuelMain_PSI); // FuelMain
  dataString += ";";
  dataString += String(FuelCarburtor_PSI); // FuelCarburator
  dataString += ";";
  dataString += String(FuelNOS_PSI); // FuelNOS
  dataString += ";";
  dataString += String(Bordspannung_Volt); // Bordspannung
  dataString += ";";
  dataString += String(Lambda); // Lambda
  dataString += ";";
  dataString += String(accelero.getXAccel()); // X Beschleunigung
  dataString += ";";
    for (int thermoCS = 0; thermoCS <= 7; thermoCS++) {
 //   float  sensor = Zylinder_summe[thermoCS] * 1.8 + 32  ;
     int sensor_farenheit = Zylinder[thermoCS] * 1.8 + 32  ;
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
   mydata.SeriallMain = FuelMain - FuelMainCal;
   mydata.SeriallCarburtor = FuelCarburtor - FuelCarburtorCal;
   mydata.SeriallNos = FuelNOS - FuelNOSCal;
//send the data
ET.sendData();
  
  
  
  digitalWrite(35, LOW);
}
else
{

 if (StartAufzeichung == true ){
   digitalWrite(35, HIGH);
  if (!myFile.open("datalog.csv", O_RDWR | O_CREAT | O_AT_END)) {
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
start = digitalRead (33);
if ( start == LOW && StartAufzeichung == false) {
 ZeitOffset = millis(); 
 streckencounter = 0;

StartAufzeichung = true;
}


}


void Motor(){ 
      detachInterrupt(0);                         // Interrupt ausschalten damit er uns nicht beißt
      unsigned long m = micros();                 // Microsekundenzähler auslesen
      unsigned long v = m - last;                 // Differenz zum letzten Durchlauf berechnen
      if (v > 30000 )   { 
       zeit = v;                                // Wert in dauer übernehmen
      last = m; 
    } 
      if (v > 1600 && v < zeit * 3  ) {                             // ignorieren wenn <= 1.6 ms (Kontaktpreller)
      zeit = v;                                // Wert in dauer übernehmen
      last = m;         // und wieder den letzten Wert merken
     zeitglatt_neu = zeitglatt + zeit;
     zeitglatt = zeitglatt_neu;
     
     zeitcounter ++;

     
     
     if ( zeitcounter >=3) { 
       zeituebergabe = zeitglatt /  zeitcounter ;
       zeitcounter = 0 ;
       zeitglatt = 0;
       zeitglatt_neu = 0;
       }
       
     //StartAufzeichung = true;  // beim ersten drehen des motors aufzeichung starten  
      }  
      
      attachInterrupt(0, Motor, FALLING );   
     // Interrupt wieder einschalten.
   }
  
void Kardanwelle(){ 
      detachInterrupt(1);                         // Interrupt ausschalten damit er uns nicht beißt
      unsigned long m2 = micros();                 // Microsekundenzähler auslesen
      unsigned long v2 = m2 - last2;                 // Differenz zum letzten Durchlauf berechnen
  
      if (v2 > 1600    ) {            // ignorieren wenn <= 4 ms (Kontaktpreller)
      zeit2 = v2;                                // Wert in dauer übernehmen
      last2 = m2;         // und wieder den letzten Wert merken
     zeitglatt_neu2 = zeitglatt2 + zeit2;
     zeitglatt2 = zeitglatt_neu2;
     streckencounter++;
    zeitcounter2 ++;
//Serial.println(zeit2);
    // zeituebergabe2=zeit2;
     
   if ( zeitcounter2 >=3) { 
      zeituebergabe2 = zeitglatt2 /  zeitcounter2 ;
     zeitcounter2 = 0 ;
      zeitglatt2 = 0;
       zeitglatt_neu2 = 0;
       }
        
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
