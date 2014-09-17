/*
  SD card datalogger

 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 10
 	 
 */
#include <SdFat.h>
//#define DEBUG   //Debug einschalten verlangsammt 110ms
#define Temperatur

SdFat sd;
SdFile myFile;

const int chipSelect = 10; // used as the CS pin, the hardware CS pin (10 on most Arduino boards,
unsigned long ZeitOffset = 0;
// Thermoelement 
int Zylinder[8];


// Umdrehung Motor
int Motordrehzahl = 0;
unsigned long last=0;
unsigned long zeit=30000;
int counter = 0;
int zeitcounter = 0;
unsigned long zeitglatt=0;
unsigned long zeitglatt_neu = 0;
unsigned long zeituebergabe = 15000;

// Umdrehung Kardanwelle
int Kardanwellenrehzahl = 0;
unsigned long last2=0;
unsigned long zeit2=1000000;
int counter2 = 0;
int zeitcounter2 = 0;
unsigned long zeitglatt2=0;
unsigned long zeitglatt_neu2 = 0;
unsigned long zeituebergabe2 = 36450;
float Abrollumfang = 2.472; // Abrollumfang hinterreifen
int streckencounter=0; // anzahl der impulse der strecke 
// MAP Sensor

int MAP = 3;





// Digital pind für Transbrake und Lachgas
int Transbrake = 25;
int Lachgas = 23;

int start = 0;
// aufzeichnug
char myChar = 10; // LF für datenstrom
int sampl = 5; // anzahl samles vor dem Speichern
boolean StartAufzeichung = false; //false ; // steuerung der Aufzeichnung


void setup()
{
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   

  // wait for MAX chip to stabilize
  delay(500);
 
  pinMode(Lachgas, INPUT);  // Digital pin als ausgang definieren
  pinMode(Transbrake, INPUT); // Digital pin als ausgang definieren
 
  Serial.print("Initializing SD card...");
 
  // Initialize SdFat or print a detailed error message and halt
  // Use half speed like the native library.
  // change to SPI_FULL_SPEED for more performance.
  if (!sd.begin(chipSelect, SPI_FULL_SPEED)) sd.initErrorHalt();
 


 

// Überschrift schreiben
String dataString = "Zeit;Motordrehzahl;Kardanwelle;Geschwindigkeit;Strecke;Transbrake;Lachgas;MAP;FuelMain;FuelCarburator;FuelNOS;Zylinder 1;Zylinder 2;Zylinder 3;Zylinder 4;Zylinder 5;Zylinder 6;Zylinder 7;Zylinder 8;";
  // open the file for write at end like the Native SD library
  if (!myFile.open("datalog.csv", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening datalog.csv for write failed");
  }
    myFile.println(dataString);
    myFile.close();
    

ZeitOffset = millis(); // offset des timers festlegen

 attachInterrupt(0, Motor, FALLING);
 attachInterrupt(1, Kardanwelle, LOW);
 
}

void loop()
{
  
String dataString = ""; // daten string anlegen
for (int i=0; i <= sampl; i++){ // Daten block zum speichern erzeugen
  
#ifdef Temperatur


   // basic readout test, just print the current temp
   for (int thermoCS=0; thermoCS <= 7; thermoCS++){
  Zylinder[thermoCS] = analogRead(thermoCS+8) ;
  
  
  #ifdef DEBUG  
   Serial.print("Zylinder ");
   Serial.print(thermoCS+1);
   Serial.print("C = "); 
   Serial.println(Zylinder[thermoCS]);
  #endif   
  }
#endif  

// Drehzahlen berechnen

Motordrehzahl = 60000000/zeituebergabe/4;

Kardanwellenrehzahl = 36450000/zeituebergabe2;  // auf Annahme am Hinterrad mit 8 impulsen pro umdrehung

#ifdef DEBUG  
   Serial.print("Motor U/min ");
   Serial.println(Motordrehzahl);
   Serial.print("Kardanwelle U/min ");
   Serial.println(Kardanwellenrehzahl);
#endif  




#ifdef DEBUG  
  Serial.print("X: ");
  Serial.println(X);
  Serial.print("Y: ");
  Serial.println(Y);
  Serial.print("Z: ");
  Serial.println(Z);
#endif

 // Zeit und Streckencounter auf null setzen wenn Transbrake gelöst wird
 
if (digitalRead(Transbrake)== 1) { start = 1;}
if (digitalRead(Transbrake)== 0 && start == 1) { start = 2;}
if (start == 2) { 
    start = 0;   
    ZeitOffset = millis(); 
  streckencounter = 0;
} 
  
  // make a string for assembling the data to log:
  
  // read  sensors and append to the string:
  
  dataString += String(millis()- ZeitOffset); // Zeitstempel für Messung eventuell auf microsekunden umstellen
  dataString += ";";
  dataString += String(Motordrehzahl); // Motorumdrehung
  dataString += ";";
  dataString += String(Kardanwellenrehzahl); // Kardanwellenrehzahl
  dataString += ";";
  dataString += String(3600/(zeituebergabe2/1000)*Abrollumfang/8); // Geschwindigkeit
  dataString += ";";
  dataString += String(Abrollumfang/8*streckencounter); // Strecke
  dataString += ";";
  dataString += String(digitalRead(Transbrake)*1000); // Transbrak
  dataString += ";";
  dataString += String(digitalRead(Lachgas)*1000); // Lachgas
  dataString += ";";
  dataString += String((18.75*((analogRead(MAP)*0.0049)))-24.075); // MAP in PSI
  dataString += ";";
  dataString += String(analogRead(MAP)); // FuelMain
  dataString += ";";
  dataString += String(analogRead(MAP)); // FuelCarburator
  dataString += ";";
  dataString += String(analogRead(MAP)); // FuelNOS
  dataString += ";";
    for (int thermoCS = 0; thermoCS <= 7; thermoCS++) {
    int sensor = Zylinder[thermoCS];
    dataString += String(sensor);
    if (thermoCS < 7) {
      dataString += ";"; 
    }
  }
dataString += myChar; // cr linefeed anhängen

}
// Datensatz speichern 
 if (StartAufzeichung == true ){
  if (!myFile.open("datalog.csv", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening datalog.csv for write failed");
  }
   myFile.print(dataString);
   myFile.close(); 
  }
    // print to the serial port too:
#ifdef DEBUG    
    Serial.println(dataString);
#endif  

if ( Motordrehzahl > 3000 && Transbrake == 1) {StartAufzeichung = true; }


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

     
     
     if ( zeitcounter >=10) { 
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
  
      if (v2 > 4000    ) {            // ignorieren wenn <= 4 ms (Kontaktpreller)
      zeit2 = v2;                                // Wert in dauer übernehmen
      last2 = m2;         // und wieder den letzten Wert merken
     //zeitglatt_neu2 = zeitglatt2 + zeit2;
     //zeitglatt2 = zeitglatt_neu2;
     streckencounter++;
  //   zeitcounter2 ++;
//Serial.println(zeit2);
     zeituebergabe2=zeit2;
     
  //  if ( zeitcounter2 >=10) { 
   //    zeituebergabe2 = zeitglatt2 /  zeitcounter2 ;
   //   zeitcounter2 = 0 ;
   //    zeitglatt2 = 0;
    //   zeitglatt_neu2 = 0;
     //  }
        
       } 
        
      attachInterrupt(1, Kardanwelle, LOW ); 
       // Interrupt wieder einschalten.
   }  


