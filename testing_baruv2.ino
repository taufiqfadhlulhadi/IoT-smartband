#include <NeoSWSerial.h>
#define USE_ARDUINO_INTERRUPTS true    
#include <PulseSensorPlayground.h>     
#include <SimpleTimer.h>
#include <TinyGPS++.h>


const int PulseWire = A0;       
const int LED13 = 13;          
int Threshold = 550;           


int inputBPM = analogRead(PulseWire);

PulseSensorPlayground pulseSensor;  
SimpleTimer firstTimer(20000);
SimpleTimer secondTimer(60000);

int myBPM = 0;
String html = "";


int RXPin = 2;
int TXPin = 3;
int x = 0;
float lati = 0;
float longi = 0;
int totalBPM = 0;
int meanBPM = 0;

// Create a software serial port called "gpsSerial"
NeoSWSerial gpsSerial(RXPin, TXPin);
NeoSWSerial EEBlue(6, 5); // RX | TX
NeoSWSerial serialSIM800(7, 8);

TinyGPSPlus gps;

void setup()
{
  // Start the Arduino hardware serial port at 9600 baud
  Serial.begin(9600);
  EEBlue.begin(9600);
  gpsSerial.begin(9600);
  serialSIM800.begin(9600);
  gpsSerial.listen();

  pulseSensor.analogInput(inputBPM);
  pulseSensor.blinkOnPulse(LED13);       //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);

  // Double-check the "pulseSensor" object was created and "began" seeing a signal.
  if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");  //This prints one time at Arduino power-up,  or on Arduino reset.
  }
}

void loop()
{
  if (firstTimer.isReady())
  {
    pulseSensor.resume();
    secondTimer.reset();
    x = 0;
    totalBPM = 0;
    meanBPM = 0;
    while (secondTimer.isReady() == false)
    {
      //Serial.println("TEST");
      bpm();
      delay(1000);
    }
    meanBPM = totalBPM / x;
    delay(40);
    EEBlue.listen();
    EEBlue.print(meanBPM);
    Serial.print(meanBPM);
    delay(40);
    Serial.println(gps.location.lat(), 6);
    Serial.println(gps.location.lng(), 6);
    delay(500);
    displayInfo();
    firstTimer.reset();
    secondTimer.reset();
    delay(500);
    gpsSerial.listen();
  }
  else
  {

    pulseSensor.pause();
    if (pulseSensor.isPaused())
    {
      while (gpsSerial.available() > 0 && firstTimer.isReady() == false)
      {
        //Serial.write(gpsSerial.read());
        gps.encode(gpsSerial.read());
      }
    }
  }
}

void bpm()
{
  myBPM = pulseSensor.getBeatsPerMinute();

  if (pulseSensor.sawStartOfBeat()) {
    Serial.println("♥  A HeartBeat Happened ! ");
    Serial.print("BPM: ");
    Serial.println(myBPM);
    totalBPM = totalBPM + myBPM;
    x++;
  }
  delay(20);
}

//void dapatkanlokasi()
//{
//  gpsSerial.listen();
//
//  if (gpsSerial.available() > 0)
//  {
//    if (gps.encode(gpsSerial.read()))
//      displayInfo();
//  }
//
//  if (millis() > 5000 && gps.charsProcessed() < 10)
//  {
//    Serial.println("No GPS detected");
//    delay(1000);
//  }
//}

void displayInfo()
{
     if (gps.location.isValid())
     {
  html = "";
  Serial.print("Latitude: ");
  Serial.println(gps.location.lat(), 6);
  Serial.print("Longitude: ");
  Serial.println(gps.location.lng(), 6);
  Serial.print("Altitude: ");
  Serial.println(gps.altitude.meters());
  html += F("AT+HTTPPARA=\"URL\",\"http://afproject.my.id/taufiq.php?lattitude=");
  html += String(gps.location.lat(), 6);
  html += F("&longitude=");
  html += String(gps.location.lng(), 6);
  html += F("&detakjantung=");
  html += String(meanBPM);
  //EEBlue.print(html);
  modulgsm();
     }
     else
     {
       Serial.println("Location: Not Available");
     }
  Serial.println();
  Serial.println();
  delay(1000);
}

void modulgsm()
{
  serialSIM800.listen();
  Serial.println("Setup modul GSM");
  serialSIM800.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(800);
  serialSIM800.println("AT+SAPBR=3,1,\"APN\",\"telkomsel\"");
  delay(800);
  serialSIM800.println("AT+SAPBR=3,1,\"PWD\",\"wap123\"");
  delay(800);
  serialSIM800.println("AT+SAPBR=1,1");
  delay(1000);
  serialSIM800.println("AT+SAPBR=2,1");
  delay(1000);
  serialSIM800.println("AT+HTTPINIT");
  delay(1000);
  //masukan url website
  serialSIM800.print(html);
  serialSIM800.println("\"");
  delay(1000);
  // 0 = GET, 1 = POST, 2 = HEAD
  serialSIM800.println("AT+HTTPACTION=0");
  delay(3000);
  serialSIM800.println("");
  delay(100);
  serialSIM800.println("AT+HTTPTERM");
  delay(800);
  Serial.print(html);
}
