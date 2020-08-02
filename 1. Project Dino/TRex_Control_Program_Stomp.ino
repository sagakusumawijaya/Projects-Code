//Code For Right Side Leg
#include <DFMiniMp3.h>
#include <SoftwareSerial.h>

#define ACTIVE LOW
#define INACTIVE HIGH
#define pbStomp A0

//Settingan Pin Input
//const uint8_t pbStomp = 7;
const uint8_t arduRX = 10;
const uint8_t pinBusy = 9;

//Settingan Pin Output
const uint8_t arduTX = 11;

//uint8_t volume = 30;
//uint8_t volumePrev = volume;
//int analogVolume = 0;

bool kondisi = false;

// implement a notification class,
// its member methods will get called
class Mp3Notify
{
  public:
    static void OnError(uint16_t errorCode) {
      // see DfMp3_Error for code meaning
      Serial.println();
      Serial.print("Com Error ");
      Serial.println(errorCode);
    }
    static void OnPlayFinished(uint16_t globalTrack) {
      Serial.println();
      Serial.print("Play finished for #");
      Serial.println(globalTrack);
    }
    static void OnCardOnline(uint16_t code) {
      Serial.println();
      Serial.print("Card online ");
      Serial.println(code);
    }
    static void OnCardInserted(uint16_t code) {
      Serial.println();
      Serial.print("Card inserted ");
      Serial.println(code);
    }
    static void OnCardRemoved(uint16_t code) {
      Serial.println();
      Serial.print("Card removed ");
      Serial.println(code);
    }
};

//Deklarasi Objek
//SoftwareSerial mySerial(arduRX, arduTX);
SoftwareSerial secondarySerial(arduRX, arduTX); // RX, TX
DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(secondarySerial);

void wait() {
  while (!digitalRead(pinBusy)) {
    // calling mp3.loop() periodically allows for notifications
    // to be handled without interrupts
    mp3.loop();
    delay(1);
  }
}

void setup() {
  delay(1000);
  pinMode(pbStomp, INPUT_PULLUP);
  pinMode(pinBusy, INPUT);
  mp3.begin();
  mp3.setVolume(30);
}

void loop() {
  if (analogRead(pbStomp) <= 20) {
    Serial.println(analogRead(pbStomp));
    mp3.playMp3FolderTrack(1);
    wait();
    kondisi = true;
    if ((analogRead(pbStomp) <= 20) && kondisi) {
      while (kondisi) {
        if (analogRead(pbStomp) >= 200 && digitalRead(pinBusy)) {
          kondisi = false;
        }
      }
    }
    delay(10);
  }
}
