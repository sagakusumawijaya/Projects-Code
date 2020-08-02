#include <DFMiniMp3.h> // https://github.com/Makuna/DFMiniMp3
#include <SoftwareSerial.h>
#include <NewPing.h> // https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home

#define arduTX 4
#define arduRX 5
#define pinBusy 10
#define TRIGGER_PIN 11
#define ECHO_PIN 12
#define MAX_DISTANCE 200
const int jarakMinimal = 20;
const int jarakMaksimal = 150;

unsigned int jarak = 0;
uint8_t jumlahBacaDalamRange = 0;
const uint8_t indeks = 20;
unsigned int bacaJarak[indeks];
unsigned int averageDistance = 0;

const int delaymp3 = 50;
const int delaysensor = 15;

// NewPing setup of pins and maximum distance
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

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
  delay(500);
  while (true) {
    // calling mp3.loop() periodically allows for notifications
    // to be handled without interrupts
    mp3.loop();
    delay(1);
    if (digitalRead(pinBusy)) {
      break;
    }
  }
}

unsigned int jarakRataRata() {
  averageDistance = 0;
  for (uint8_t i = 0; i < indeks; i++) {
    bacaJarak[i] = sonar.ping_cm();
    delay(delaysensor);
    Serial.print("Jarak ke-");
    Serial.print(i + 1);
    Serial.print(" = ");
    Serial.println(bacaJarak[i]);
    if(bacaJarak[i] > jarakMinimal && bacaJarak[i] < jarakMaksimal){
      jumlahBacaDalamRange+=1;
    }
  }
  for (uint8_t i = 0; i < indeks; i++) {
    averageDistance += bacaJarak[i];
  }
  return (averageDistance / indeks);
}

void setup() {
  pinMode(pinBusy, INPUT);
  mp3.begin();
  delay(delaymp3);
  mp3.reset();
  delay(delaymp3);
  mp3.setVolume(15);
  Serial.begin(9600);
}

void loop() {
  jarak = jarakRataRata();
  if (jarak > jarakMinimal && jarak < jarakMaksimal && (jumlahBacaDalamRange > (indeks/2))) {
    Serial.println();
    Serial.print("JUMLAH BACA DALAM RANGE = ");
    Serial.println(jumlahBacaDalamRange);
    Serial.print("JARAK TOTAL = ");
    Serial.print(jarak);
    Serial.println("cm");
    Serial.println(); Serial.println();
    mp3.playMp3FolderTrack(1);
    wait();
    jumlahBacaDalamRange=0;
  } else {
    Serial.println();
    Serial.print("JUMLAH BACA DALAM RANGE = ");
    Serial.println(jumlahBacaDalamRange);
    Serial.print("JARAK TOTAL = ");
    Serial.print(jarak);
    Serial.println("cm");
    Serial.println(); Serial.println();
    jumlahBacaDalamRange=0;
  }
}
