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
#define X_Beschleunigung
#define Y_Beschleunigung
#define Z_Beschleunigung
#define G_Empfindlichkeit // Beschleunigungssensor auf 6G empfindlichkeit einstellen
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
unsigned long zeit2=60000000UL;
int counter2 = 0;

// Beschleunigungssensor


float BeschleunigungsKonstante = 0.0061224;

int analogPinX = 0;
int analogPinY = 1;
int analogPinZ = 2;
int kalibrierungX = 0;
int kalibrierungY = 0;
int kalibrierungZ = 0;
int i=0;
const float alpha = 0.5;


float X = 0;
float Y = 0;
float Z = 0;
double fXg = 0;
double fYg = 0;
double fZg = 0;



// Digital pind für Transbrake und Lachgas
int Transbrake = 25;
int Lachgas = 23;
int empfindlichkeit = 31;
int start = 0;
// aufzeichnug
char myChar = 10; // LF für datenstrom
int sampl = 7; // anzahl samles vor dem Speichern
boolean StartAufzeichung = false; // steuerung der Aufzeichnung


void setup()
{
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   

  // wait for MAX chip to stabilize
  delay(500);
 
  pinMode(Lachgas, INPUT);  // Digital pin als ausgang definieren
  pinMode(Transbrake, INPUT); // Digital pin als ausgang definieren
  pinMode(empfindlichkeit, OUTPUT); //  Digital pin als ausgang definieren
  Serial.print("Initializing SD card...");
 
  // Initialize SdFat or print a detailed error message and halt
  // Use half speed like the native library.
  // change to SPI_FULL_SPEED for more performance.
  if (!sd.begin(chipSelect, SPI_FULL_SPEED)) sd.initErrorHalt();
 
 digitalWrite(empfindlichkeit, LOW);   // Beschleunigungssensor auf 1.5G einstellen 

#ifdef G_Empfindlichkeit
BeschleunigungsKonstante = 0.02381; // fakrot für 6G
digitalWrite(empfindlichkeit, HIGH);   // Beschleunigungssensor auf 6G einstellen 
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

        Serial.print("Kalibrierung X: ");
        Serial.println(kalibrierungX);
        Serial.print("Kalibrierung Y: ");
        Serial.println(kalibrierungY);
        Serial.print("Kalibrierung Z: ");
        Serial.println(kalibrierungZ);
  delay (1000);


// Überschrift schreiben
String dataString = "Zeit;Motordrehzahl;Kardanwelle;Transbrake;Lachgas;BeschleunigungX;BeschleunigungY;BeschleunigungZ;Zylinder 1;Zylinder 2;Zylinder 3;Zylinder 4;Zylinder 5;Zylinder 6;Zylinder 7;Zylinder 8;";
  // open the file for write at end like the Native SD library
  if (!myFile.open("datalog.csv", O_RDWR | O_CREAT | O_AT_END)) {
    sd.errorHalt("opening datalog.csv for write failed");
  }
    myFile.println(dataString);
    myFile.close();
    

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

Kardanwellenrehzahl = 60000000/zeit2;

#ifdef DEBUG  
   Serial.print("Motor U/min ");
   Serial.println(Motordrehzahl);
   Serial.print("Kardanwelle U/min ");
   Serial.println(Kardanwellenrehzahl);
#endif  


#ifdef X_Beschleunigung
X = (analogRead(analogPinX)-kalibrierungX)*BeschleunigungsKonstante;
fXg = X * alpha + (fXg * (1.0 - alpha)); // glättung mit tiefpass

#endif
#ifdef Y_Beschleunigung
Y = (analogRead(analogPinY)-kalibrierungY)*BeschleunigungsKonstante;
fYg = Y * alpha + (fYg * (1.0 - alpha));

#endif
#ifdef Z_Beschleunigung
Z = (analogRead(analogPinZ)-kalibrierungZ)*BeschleunigungsKonstante;
fZg = Z * alpha + (fZg * (1.0 - alpha));

#endif


#ifdef DEBUG  
  Serial.print("X: ");
  Serial.println(X);
  Serial.print("Y: ");
  Serial.println(Y);
  Serial.print("Z: ");
  Serial.println(Z);
#endif


if (digitalRead(Transbrake)== 1) { start = 1;}
if (digitalRead(Transbrake)== 0 && start == 1) { start = 2;}
if (start == 2) { start = 0;  ZeitOffset = millis(); }
  
  // make a string for assembling the data to log:
  
  // read  sensors and append to the string:
  
  dataString += String(millis()- ZeitOffset); // Zeitstempel für Messung eventuell auf microsekunden umstellen
  dataString += ";";
  dataString += String(Motordrehzahl); // Motorumdrehung
  dataString += ";";
  dataString += String(Kardanwellenrehzahl); // Kardanwellenrehzahl
  dataString += ";";
  dataString += String(digitalRead(Transbrake)*1000); // Transbrak
  dataString += ";";
  dataString += String(digitalRead(Lachgas)*1000); // Lachgas
  dataString += ";";
  #ifdef X_Beschleunigung
  dataString += String(fXg); // Beschleunigung X
  dataString += ";";
  #endif
  #ifdef Y_Beschleunigung
  dataString += String(fYg); // Beschleunigung X
  dataString += ";";
  #endif
  #ifdef Z_Beschleunigung
  dataString += String(fZg); // Beschleunigung X
  dataString += ";";
  #endif
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
       
     StartAufzeichung = true;  // beim ersten drehen des motors aufzeichung starten  
      }  
      
      attachInterrupt(0, Motor, FALLING );   
     // Interrupt wieder einschalten.
   }
  
void Kardanwelle(){ 
      detachInterrupt(1);                         // Interrupt ausschalten damit er uns nicht beißt
      unsigned long m2 = micros();                 // Microsekundenzähler auslesen
      unsigned long v2 = m2 - last2;                 // Differenz zum letzten Durchlauf berechnen
      if (v2 > 5000) {                             // ignorieren wenn <= 5ms (Kontaktpreller)
      zeit2 = v2;                                // Wert in dauer übernehmen
          last2 = m2;                                 // und wieder den letzten Wert merken
        }
      attachInterrupt(1, Kardanwelle, FALLING ); 
       // Interrupt wieder einschalten.
   }  


