#include <DFMiniMp3.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

#define ACTIVE LOW
#define INACTIVE HIGH
#define pbRoar A15

//Settingan Pin Input
//const uint8_t pbRoar = 42;
const uint8_t pbNextRoar = 47;
const uint8_t pbPrevRoar = 49;
const uint8_t pbOnOffRoar = 51;
const uint8_t pbOnOffStomp = 53;
const uint8_t pinBusy = 45;
const uint8_t arduRX = 10;

//Settingan Pin Output
const uint8_t ledRoar = 41;
const uint8_t ledNextRoar = 52;
const uint8_t ledPrevRoar = 50;
const uint8_t ledOnOffRoar = 46;
const uint8_t ledOnOffStomp = 48;
//const uint8_t ledAlarm = 54;
//const uint8_t buzzer = 47;
const uint8_t arduTX = 11;
const uint8_t relayNanoPin = 24;
//#define ledOnOffSystem

//Settingan Pin LCD
const uint8_t rs = 22;
const uint8_t en = 4;
const uint8_t d4 = 3;
const uint8_t d5 = 43;
const uint8_t d6 = 2;
const uint8_t d7 = 44;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

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
      lcd.clear();
      lcd.setCursor(10, 1);
      lcd.print("#");
      lcd.setCursor(11, 1);
      lcd.print(globalTrack);
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

//Settingan Variabel Kondisi
bool showOnOffRoarCondition = false;
bool showOnOffStompCondition = false;
bool onOffRoarCondition = false;
bool onOffStompCondition = false;
bool isPBNextRoarPressed = false;
bool isPBPrevRoarPressed = false;
bool isPBOnOffRoarPressed = false;
bool isPBOnOffStompPressed = false;
bool isPBRoarPressed = false;
int analogPBRoar = 0;
bool roarStandbyCondition = false;
bool stepStandbyCondition = false;
bool settingCondition = false;
bool errorCondition = false;
bool finishCondition = false;

//Settingan Tipe
/*
   1 : next roar
   2 : prev roar
   3 : shut voice down
   4 : shut step-sound down
*/
uint8_t settingType = 0;
uint8_t roarNumber = 1;//1~15

//Settingan Default
uint8_t volume = 30; //default
const uint8_t lcdCol = 20;
const uint8_t lcdRow = 4;

//Settingan Delay
const uint16_t commandRespondDelay = 10;
const uint16_t resetDelay = 10;
const uint16_t errorDelay = 10;
const uint16_t relayDelay = 100;
const uint16_t restartDelay = 50;
const uint16_t endSettingDelay = 50;
const uint16_t nyalaLedDelay = 100;

//Daftar Fungsi
void inisialisasiLCD();
void inisialisasiDFPlayer();
bool bacaInputSetting();
void tampilStartSetting();
void settings(uint8_t sType);
void tampilEndSetting();
void tampilStandby();
bool bacaPBRoar();
void roar(uint8_t rType);
void wait();
void nyalakanLed(const int led, const int &pb, bool &pbCondition);

//Daftar Fungsi Set
void setRoarTypeNext();
void setRoarTypePrev();
void setOnOffRoar();
void setOnOffStomp();
void setError();

void setup() {
  pinMode(pbRoar, INPUT_PULLUP);
  pinMode(pbNextRoar, INPUT_PULLUP);
  pinMode(pbPrevRoar, INPUT_PULLUP);
  pinMode(pbOnOffRoar, INPUT_PULLUP);
  pinMode(pbOnOffStomp, INPUT_PULLUP);
  pinMode(pinBusy, INPUT);

  pinMode(ledNextRoar, OUTPUT);
  pinMode(ledPrevRoar, OUTPUT);
  pinMode(ledOnOffRoar, OUTPUT);
  pinMode(ledOnOffStomp, OUTPUT);
  //  pinMode(ledOnOffSystem, OUTPUT);
  //  pinMode(ledBuzzer, OUTPUT);
  //pinMode(buzzer, OUTPUT);
  pinMode(relayNanoPin, OUTPUT);
  //  digitalWrite(ledOnOffSystem, HIGH);
  //Serial.begin(115200);
  inisialisasiLCD();
  delay(1000);
  inisialisasiDFPlayer();
  delay(1000);
}

void loop() {
  if (bacaInputSetting()) {
    tampilStartSetting();
    settings(settingType);
    tampilEndSetting();
  } else {
    tampilStandby();
    if (bacaPBRoar()) {
      roar(roarNumber);
    }
  }
}

void inisialisasiLCD() {
  lcd.begin(lcdCol, lcdRow);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MEMULAI SISTEM");
}
void inisialisasiDFPlayer() {
  mp3.begin();
  mp3.setVolume(volume);
  onOffRoarCondition = true;
}
bool bacaInputSetting() {
  isPBNextRoarPressed = digitalRead(pbNextRoar);
  isPBPrevRoarPressed = digitalRead(pbPrevRoar);
  isPBOnOffRoarPressed = digitalRead(pbOnOffRoar);
  isPBOnOffStompPressed = digitalRead(pbOnOffStomp);
  if (isPBNextRoarPressed == ACTIVE) {
    nyalakanLed(ledNextRoar, pbNextRoar, isPBNextRoarPressed);
    isPBNextRoarPressed = INACTIVE;
    settingType = 1;
    return true;
  } else if (isPBPrevRoarPressed == ACTIVE) {
    nyalakanLed(ledPrevRoar, pbPrevRoar, isPBPrevRoarPressed);
    isPBPrevRoarPressed = INACTIVE;
    settingType = 2;
    return true;
  } else if (isPBOnOffRoarPressed == ACTIVE) {
    nyalakanLed(ledOnOffRoar, pbOnOffRoar, isPBOnOffRoarPressed);
    isPBOnOffRoarPressed = INACTIVE;
    settingType = 3;
    return true;
  } else if (isPBOnOffStompPressed == ACTIVE) {
    nyalakanLed(ledOnOffStomp, pbOnOffStomp, isPBOnOffStompPressed);
    isPBOnOffStompPressed = INACTIVE;
    settingType = 4;
    return true;
  } else {
    return false;
  }
}
void tampilStartSetting() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mode Setting");
  lcd.setCursor(0, 1);
  lcd.print("START");
}
void settings(uint8_t sType) {
  switch (sType) {
    case 1://next
      setRoarTypeNext(); break;
    case 2://previous
      setRoarTypePrev(); break;
    case 3://onOffRoar
      setOnOffRoar(); break;
    case 4://OnOffStomp
      setOnOffStomp(); break;
    default:
      errorCondition = true;
      setError(); break;
  }
}
void tampilEndSetting() {
  if (errorCondition) {
    errorCondition = false;
    return;
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Mode Setting");
    lcd.setCursor(0, 1);
    lcd.print("FINISH");
    delay(endSettingDelay);
  }
  roarStandbyCondition = false;
  showOnOffRoarCondition = false;
  showOnOffStompCondition = false;
}
void tampilStandby() {
  if (roarStandbyCondition) {
    if (showOnOffRoarCondition) {
      if (showOnOffStompCondition) {
        return;
      } else {
        lcd.setCursor(0, 2);
        lcd.print("Stomp: ");
        if (onOffStompCondition) {
          lcd.setCursor(7, 2);
          lcd.print("ON");
        } else {
          lcd.setCursor(7, 2);
          lcd.print("OFF");
        }
        showOnOffStompCondition = true;
      }
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Roar : ");
      if (onOffRoarCondition) {
        lcd.setCursor(7, 1);
        lcd.print("ON");
        lcd.setCursor(12, 1);
        lcd.print("Type #");
        lcd.setCursor(18, 1);
        lcd.print(roarNumber);
      } else {
        lcd.setCursor(7, 1);
        lcd.print("OFF");
      }
      showOnOffRoarCondition = true;
    }
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("STANDBY  ");
    roarStandbyCondition = true;
  }
}
bool bacaPBRoar() {
  analogPBRoar = analogRead(pbRoar);
  //Serial.println(analogPBRoar);
  isPBRoarPressed = analogPBRoar<20? ACTIVE : INACTIVE;
  if (isPBRoarPressed == ACTIVE) {
    
    return true;
  } else {
    analogPBRoar = 0;
    return false;
  }
}
void roar(uint8_t rType) {
  if (onOffRoarCondition) {
    mp3.playMp3FolderTrack(rType);
    wait();//menunggu sampai suara berhenti
    roarStandbyCondition = false;
    showOnOffRoarCondition = false;
    showOnOffStompCondition = false;
  }
}
void setRoarTypeNext() {
  if (roarNumber == 7) {
    roarNumber = 1;
  } else {
    roarNumber++;
  }
}
void setRoarTypePrev() {
  if (roarNumber == 1) {
    roarNumber = 7;
  } else {
    roarNumber--;
  }
}
void setOnOffRoar() {
  if (onOffRoarCondition) {
    volume = 0;
    mp3.setVolume(volume);
    delay(commandRespondDelay);
    onOffRoarCondition = false;
    showOnOffRoarCondition = false;
  } else {
    volume = 15;
    mp3.setVolume(volume);
    delay(commandRespondDelay);
    onOffRoarCondition = true;
    showOnOffRoarCondition = false;
  }
}
void setOnOffStomp() {
  if (onOffStompCondition) {
    onOffStompCondition = false;
    showOnOffStompCondition = false;
    digitalWrite(relayNanoPin, LOW);
    delay(relayDelay);
  } else {
    onOffStompCondition = true;
    showOnOffStompCondition = true;
    digitalWrite(relayNanoPin, HIGH);
    delay(relayDelay);
  }
}
void setError() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ERROR!");
  mp3.reset();
  delay(resetDelay);
  delay(errorDelay);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RESTART");
  delay(restartDelay);
}
void wait() {
  while (!digitalRead(pinBusy)) {
    // calling mp3.loop() periodically allows for notifications
    // to be handled without interrupts
    mp3.loop();
    delay(1);
  }
}
void nyalakanLed(const int led, const int &pb, bool &pbCondition) {
  pbCondition = digitalRead(pb);
  while (pbCondition == ACTIVE) {
    pbCondition = digitalRead(pb);
    delay(1);
    digitalWrite(led, HIGH);
  }
  delay(nyalaLedDelay);
  digitalWrite(led, LOW);
}
