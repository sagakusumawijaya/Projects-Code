#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#define S_RX    3                // Define software serial RX pin
#define S_TX    2                // Define software serial TX pin
#define pinPBLCD A0
#define pinPBTW A1

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
TinyGPSPlus gps;
SoftwareSerial softSerial(S_RX, S_TX);

//---------------------DEVICE IP & MAC VARIABLE----------------------//
byte mac[] = { 0x2A, 0x00, 0x22, 0x22, 0x22, 0x44 };
IPAddress ip(192, 168, 1, 20);
const unsigned int localPort = 1369;
//-------------------------------------------------------------------//


//-------------------------------------------------------------------//
//------------------------ETHERNET CONFIGURATION---------------------//
uint8_t rIPBaru[4] = {192, 168, 1, 21};
uint8_t rIPBaruSebelumnya[4] = {rIPBaru[0], rIPBaru[1], rIPBaru[2], rIPBaru[3]};
IPAddress remote_IP(192, 168, 1, 10);
int remote_Port = 9631;
int remote_Port_Baru = 9632;
int remote_Port_Baru_Sebelumnya;
char UDP_TX_Buffer[80];
char receivedBuffer[UDP_TX_PACKET_MAX_SIZE];
int packetSize = 0;
EthernetUDP udp;
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//

byte last_second;
char Time[]  = "TIME: 00:00:00";
char WaktuTersinkron[] = "TIME: 00:00:00";

const int ALAMAT_JARAK_ERROR_DI_EEPROM = 0;
int32_t jarakErrorDiEEPROM;

byte panah[8] = {
  0b00000,
  0b00000,
  0b00100,
  0b00010,
  0b11111,
  0b00010,
  0b00100,
  0b00000,
};

bool modeSetting = false;
bool modeStandby = false;

uint16_t analogPinPBLCD = 1023;
uint16_t analogPinPBTW = 1023;

uint16_t BATAS_ANALOG_PB_TW = 900;
uint16_t BATAS_ANALOG_PB_LCD = 950;

const uint8_t JENIS_INPUT_PB_TW = 1;
const uint8_t JENIS_INPUT_PB_LCD = 2;

const uint8_t JENIS_TOMBOL_SELECT = 1;
const uint8_t JENIS_TOMBOL_UP = 2;
const uint8_t JENIS_TOMBOL_DOWN = 3;
const uint8_t JENIS_TOMBOL_LEFT = 4;
const uint8_t JENIS_TOMBOL_RIGHT = 5;
const uint8_t JENIS_TOMBOL_NONE = 6;

const uint8_t JENIS_HALAMAN_SETTING_12 = 0;
const uint8_t JENIS_HALAMAN_SETTING_34 = 1;
const uint8_t JENIS_HALAMAN_SETTING_5 = 9;
const uint8_t JENIS_HALAMAN_SETTING_IP = 2;
const uint8_t JENIS_HALAMAN_SETTING_PORT = 3;
const uint8_t JENIS_HALAMAN_SETTING_JARAK = 8;
const uint8_t JENIS_HALAMAN_DEVICE_SETTING = 4;
const uint8_t JENIS_HALAMAN_PRE_TW = 5;
const uint8_t JENIS_HALAMAN_POST_TW = 6;
const uint8_t JENIS_HALAMAN_STANDBY = 7;
const uint8_t JENIS_HALAMAN_ERROR_EEPROM = 10;

const uint16_t delayInisialisasi = 2000;
const uint16_t delayBacaInputTombol = 150;
const uint16_t delayPacket = 500;
const byte karakterPanah = 0;

const unsigned long c = 299792458; //299.792.458 meter/sekon

uint16_t nilaiAnalogPinPBLCD;
uint16_t nilaiAnalogPinPBTW;
uint8_t jenisInput;
uint8_t jenisTombol;
uint8_t halamanSebelumnya;
uint8_t halaman;
uint8_t barisPanahSebelumnya;
uint8_t barisPanah;
uint8_t kolomPanah;
uint8_t barisKursor;
uint8_t kolomKursor;
uint8_t kolomKursorSebelumnya;
uint16_t jarak;
uint16_t jarakBaru;
uint16_t jarakBaruSebelumnya;

int64_t waktuMicroTWTerbacaDiSini = 0;
int64_t waktuMicroTWTerbacaDiSana = 0;

int64_t waktuMicroTersinkronDiSini;
int64_t waktuMicroTersinkronDiSana;
int64_t waktuMicroTersinkron;

int64_t selisihWaktuMicro;
int32_t jarakErrorDariSini = 0;

uint8_t jamTersinkronDiSini;
uint8_t menitTersinkronDiSini;
uint8_t detikTersinkronDiSini;

uint8_t jamTersinkronDiSana;
uint8_t menitTersinkronDiSana;
uint8_t detikTersinkronDiSana;

uint8_t jamTersinkron;
uint8_t menitTersinkron;
uint8_t detikTersinkron;

uint8_t jamTerjadiTWDiSini;
uint8_t menitTerjadiTWDiSini;
uint8_t detikTerjadiTWDiSini;

uint8_t jamTerjadiTWDiSana;
uint8_t menitTerjadiTWDiSana;
uint8_t detikTerjadiTWDiSana;
int diSiniLebihBesar;/**1 = Waktu sinkronisasi micros() lebih awal
                          -1 = Waktu sinkronisasi micros() lebih akhir
                          0 = Waktu sinkronisasi micros() sama
                          1 = nilai abs() > 0
                          -1 = nilai abs() < 0
                          0 = nilai abs() == 0
*/


//-------------------------------------------------------------------------------------//
//------------------------------------FUNGSI COMPARE-----------------------------------//
bool isHalaman(uint8_t hal) {
  if (halaman == hal) {
    return true;
  } else {
    return false;
  }
}
bool isBarisPanah(uint8_t bp) {
  if (barisPanah == bp) {
    return true;
  } else {
    return false;
  }
}
bool isKolomPanah(uint8_t kp) {
  if (kolomPanah == kp) {
    return true;
  } else {
    return false;
  }
}
bool isModeSetting(bool ms) {
  if (modeSetting == ms) {
    return true;
  } else {
    return false;
  }
}
bool isBarisKursor(uint8_t bk) {
  if (barisKursor == bk) {
    return true;
  } else {
    return false;
  }
}
bool isKolomKursor(uint8_t kk) {
  if (kolomKursor == kk) {
    return true;
  } else {
    return false;
  }
}
bool isHalamanSebelumnya(uint8_t hal) {
  if (halamanSebelumnya == hal) {
    return true;
  } else {
    return false;
  }
}
bool isBarisPanahSebelumnya(uint8_t bps) {
  if (barisPanahSebelumnya == bps) {
    return true;
  } else {
    return false;
  }
}
bool isJarak(uint16_t j) {
  if (jarak == j) {
    return true;
  } else {
    return false;
  }
}
bool isJarakBaru(uint16_t jb) {
  if (jarakBaru == jb) {
    return true;
  } else {
    return false;
  }
}
bool isJarakBaruSebelumnya(uint16_t jbs) {
  if (jarakBaruSebelumnya == jbs) {
    return true;
  } else {
    return false;
  }
}
bool isKolomKursorSebelumnya(uint8_t kks) {
  if (kolomKursorSebelumnya == kks) {
    return true;
  } else {
    return false;
  }
}
bool isRIPBaru(uint8_t rip[]) {
  for (int i = 0; i <= 3; i++) {
    if (rIPBaru[i] != rip[i]) {
      return false;
    }
  }
  return true;
}
bool isRemotePort(int rp) {
  if (remote_Port == rp) {
    return true;
  } else {
    return false;
  }
}
bool isRemotePortBaru(int rpb) {
  if (remote_Port_Baru == rpb) {
    return true;
  } else {
    return false;
  }
}
bool isRemotePortBaruSebelumnya(int rpbs) {
  if (remote_Port_Baru_Sebelumnya == rpbs) {
    return true;
  } else {
    return false;
  }
}
bool isJamTersinkronDiSini(uint8_t j) {
  if (jamTersinkronDiSini == j) {
    return true;
  } else {
    return false;
  }
}
bool isMenitTersinkronDiSini(uint8_t m) {
  if (menitTersinkronDiSini == m) {
    return true;
  } else {
    return false;
  }
}
bool isDetikTersinkronDiSini(uint8_t d) {
  if (detikTersinkronDiSini == d) {
    return true;
  } else {
    return false;
  }
}
bool isJamTersinkronDiSana(uint8_t j) {
  if (jamTersinkronDiSana == j) {
    return true;
  } else {
    return false;
  }
}
bool isMenitTersinkronDiSana(uint8_t m) {
  if (menitTersinkronDiSana == m) {
    return true;
  } else {
    return false;
  }
}
bool isDetikTersinkronDiSana(uint8_t d) {
  if (detikTersinkronDiSana == d) {
    return true;
  } else {
    return false;
  }
}
bool isJamTersinkron(uint8_t j) {
  if (jamTersinkron == j) {
    return true;
  } else {
    return false;
  }
}
bool isMenitTersinkron(uint8_t m) {
  if (menitTersinkron == m) {
    return true;
  } else {
    return false;
  }
}
bool isDetikTersinkron(uint8_t d) {
  if (detikTersinkron == d) {
    return true;
  } else {
    return false;
  }
}
bool isJamTerjadiTWDiSini(uint8_t j) {
  if (jamTerjadiTWDiSini == j) {
    return true;
  } else {
    return false;
  }
}
bool isMenitTerjadiTWDiSini(uint8_t m) {
  if (menitTerjadiTWDiSini == m) {
    return true;
  } else {
    return false;
  }
}
bool isDetikTerjadiTWDiSini(uint8_t d) {
  if (detikTerjadiTWDiSini == d) {
    return true;
  } else {
    return false;
  }
}
bool isJamTerjadiTWDiSana(uint8_t j) {
  if (jamTerjadiTWDiSana == j) {
    return true;
  } else {
    return false;
  }
}
bool isMenitTerjadiTWDiSana(uint8_t m) {
  if (menitTerjadiTWDiSana == m) {
    return true;
  } else {
    return false;
  }
}
bool isDetikTerjadiTWDiSana(uint8_t d) {
  if (detikTerjadiTWDiSana == d) {
    return true;
  } else {
    return false;
  }
}
bool isWaktuMicroTWTerbacaDiSini(int64_t t) {
  if (waktuMicroTWTerbacaDiSini == t) {
    return true;
  } else {
    return false;
  }
}
bool isWaktuMicroTWTerbacaDiSana(int64_t t) {
  if (waktuMicroTWTerbacaDiSana == t) {
    return true;
  } else {
    return false;
  }
}
bool isJarakErrorDariSini(int32_t j) {
  if (jarakErrorDariSini == j) {
    return true;
  } else {
    return false;
  }
}
bool isWaktuMicroTersinkronDiSini(int64_t t) {
  if (waktuMicroTersinkronDiSini == t) {
    return true;
  } else {
    return false;
  }
}
bool isWaktuMicroTersinkronDiSana(int64_t t) {
  if (waktuMicroTersinkronDiSana == t) {
    return true;
  } else {
    return false;
  }
}
bool isWaktuMicroTersinkron(int64_t t) {
  if (waktuMicroTersinkron == t) {
    return true;
  } else {
    return false;
  }
}
bool isSelisihWaktuMicro(int64_t t) {
  if (selisihWaktuMicro == t) {
    return true;
  } else {
    return false;
  }
}
bool isDiSiniLebihBesar(int x) {
  if (diSiniLebihBesar == x) {
    return true;
  } else {
    return false;
  }
}
bool isJarakErrorDiEEPROM(int32_t j) {
  if (jarakErrorDiEEPROM == j) {
    return true;
  } else {
    return false;
  }
}
//-------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------//


//------------------------------------------------------------------------------------//
//---------------------------FUNGSI INISIALISASI--------------------------------------//
void inisialisasiLCD() {
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("INISIALISASI LCD");
  lcd.createChar(karakterPanah, panah);
  delay(delayInisialisasi);
}
void inisialisasiGPS() {
  softSerial.begin(9600);
  lcd.clear();
  lcd.print("INISIALISASI GPS");
  delay(delayInisialisasi);
}
void inisialisasiEthernet() {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("INISIALISASI");
  lcd.setCursor(4, 1);
  lcd.print("ETHERNET");
  Ethernet.begin(mac, ip);
  udp.begin(localPort);
  delay(delayInisialisasi);
}
//-----------------------------------------------------------------------------------//
//-----------------------------------------------------------------------------------//




bool adaInput() {
  analogPinPBTW = analogRead(pinPBTW);
  analogPinPBLCD = analogRead(pinPBLCD);
  if (analogPinPBTW < BATAS_ANALOG_PB_TW) {
    jenisInput = JENIS_INPUT_PB_TW;
    return true;
  } else if (analogPinPBLCD < BATAS_ANALOG_PB_LCD) {
    jenisInput = JENIS_INPUT_PB_LCD;
    return true;
  } else {
    return false;
  }
}
void eksekusiInput() {
  if (jenisInput == JENIS_INPUT_PB_TW) {
    waktuMicroTWTerbacaDiSini = micros();
    delay(100);
    bool kondisi = true;
    while (kondisi) {
      if (softSerial.available() > 0) {
        if (gps.encode(softSerial.read())) {
          if (gps.time.isValid()) {
            setDetikTerjadiTWDiSini(gps.time.second());
            setMenitTerjadiTWDiSini(gps.time.minute());
            setJamTerjadiTWDiSini(gps.time.hour() + 7);
            setHalaman(JENIS_HALAMAN_PRE_TW);
            //waktuMicroTWTerbacaDiSini = waktuMicroTWTerbacaDiSini + (micros()-waktuMicroTWTerbacaDiSini);
            kondisi = false;
            if (!kondisi) {
              lcd.clear();
              WaktuTersinkron[6]  = (getJamTerjadiTWDiSini())   / 10 + 48;
              WaktuTersinkron[7]  = (getJamTerjadiTWDiSini())   % 10 + 48;
              WaktuTersinkron[9]  = (getMenitTerjadiTWDiSini()) / 10 + 48;
              WaktuTersinkron[10] = (getMenitTerjadiTWDiSini()) % 10 + 48;
              WaktuTersinkron[12] = (getDetikTerjadiTWDiSini()) / 10 + 48;
              WaktuTersinkron[13] = (getDetikTerjadiTWDiSini()) % 10 + 48;
              lcd.setCursor(0, 0);
              lcd.print("Waktu Tersinkron");
              lcd.setCursor(1, 1);
              lcd.print(WaktuTersinkron);
              delay(5000);
            }
          }
        }
      }
    }
    delay(300);
    setHalaman(JENIS_HALAMAN_PRE_TW);
    eksekusiTW();
  } else if (jenisInput == JENIS_INPUT_PB_LCD) {
    setHalaman(JENIS_HALAMAN_SETTING_12);
    setBarisPanah(0);
    setModeSetting(true);
    eksekusiTombol();
  }
}
void eksekusiTW() {
  int64_t waktuSekarang = millis();
  bool kondisi = true;
  bool belumTerkirim = true;
  uint8_t urutan = 1;
  //-----------------Menunggu data diterima & Mengirim data-----------------//
  //--------------------------------PRE TW----------------------------------//
  while (kondisi) {
    tampil(getHalaman());
    if (udp.available()) {
      terimaDataDariSana(urutan, kondisi);
    } else if ((millis() - waktuSekarang) > 15000) {
      break;
    } else if (belumTerkirim) {
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Mengirim Data");
      lcd.setCursor(5, 1);
      lcd.print("Mulai");
      delay(2000);
      kirimWaktuTWTerbacaDiSini();
      kirimWaktuMicroTWTerbacaDiSini();
      kirimWaktuTersinkronDiSini();
      kirimKonfirmasiSelesaiPengiriman();
      belumTerkirim = false;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Mengirim Data");
      lcd.setCursor(5, 1);
      lcd.print("Selesai");
      delay(2000);
      lcd.clear();
      setHalamanSebelumnya(1);//hanya untuk pembeda pada tampil(getHalaman())
    }
  }
  //--------------------------------POST TW----------------------------------//
  sinkronisasiWaktu();
  kalkulasiError();
  setHalaman(JENIS_HALAMAN_POST_TW);
  tampil(getHalaman());
  delay(30000);
  setDefaultTW();
}
void setDefaultTW() {
  setJamTerjadiTWDiSini(0);
  setMenitTerjadiTWDiSini(0);
  setDetikTerjadiTWDiSini(0);
  setJamTerjadiTWDiSana(0);
  setMenitTerjadiTWDiSana(0);
  setDetikTerjadiTWDiSana(0);
  setWaktuMicroTWTerbacaDiSini(0);
  setWaktuMicroTWTerbacaDiSana(0);
}
void sinkronisasiWaktu() {
  int jam, menit, detik;
  jam = getJamTersinkronDiSini() - getJamTersinkronDiSana();
  menit = getMenitTersinkronDiSini() - getMenitTersinkronDiSana();
  detik = getDetikTersinkronDiSini() - getDetikTersinkronDiSana();
  //------------------------------------//
  if (abs(jam) > 0) {
    setDiSiniLebihBesar(1);
  } else if (abs(jam) == 0) {
    if (abs(menit) > 0) {
      setDiSiniLebihBesar(1);
    } else if (abs(menit) == 0) {
      if (abs(detik) > 0) {
        setDiSiniLebihBesar(1);
      } else if (abs(detik) == 0) {
        setDiSiniLebihBesar(0);
      } else { //abs(detik)<0
        setDiSiniLebihBesar(-1);
      }
    } else { //abs(menit)<0
      setDiSiniLebihBesar(-1);
    }
  } else { //abs(jam)<0
    setDiSiniLebihBesar(-1);
  }
  //------------------------------------//

  if (isDiSiniLebihBesar(1)) {
    setJamTersinkron(getJamTersinkronDiSini());
    setMenitTersinkron(getMenitTersinkronDiSini());
    setDetikTersinkron(getDetikTersinkronDiSini());
    setWaktuMicroTersinkron(getWaktuMicroTersinkronDiSini());
  } else if (isDiSiniLebihBesar(0)) {
    setJamTersinkron(getJamTersinkronDiSini());
    setMenitTersinkron(getMenitTersinkronDiSini());
    setDetikTersinkron(getDetikTersinkronDiSini());
    setWaktuMicroTersinkron(getWaktuMicroTersinkronDiSini());
  } else { //diSiniLebihBesar == -1 isDiSiniLebihBesar(-1)
    setJamTersinkron(getJamTersinkronDiSana());
    setMenitTersinkron(getMenitTersinkronDiSana());
    setDetikTersinkron(getDetikTersinkronDiSana());
    setSelisihWaktuMicro( (getJamTersinkronDiSana() - getJamTersinkronDiSini()) * 3600 * 1000000 +
                          (getMenitTersinkronDiSana() - getMenitTersinkronDiSini()) * 60 * 1000000 +
                          (getDetikTersinkronDiSana() - getDetikTersinkronDiSini()) * 1000000
                        );
    setWaktuMicroTersinkron(getWaktuMicroTersinkronDiSana());
    setWaktuMicroTWTerbacaDiSana(getWaktuMicroTWTerbacaDiSana() - getSelisihWaktuMicro());
  }
}
void kalkulasiError() {
  setJarakErrorDariSini((getJarak() + ((getWaktuMicroTWTerbacaDiSini() - getWaktuMicroTWTerbacaDiSana())*c)) / 2);
  setJarakErrorDiEEPROM(getJarakErrorDariSini());
  EEPROM.put(ALAMAT_JARAK_ERROR_DI_EEPROM, jarakErrorDiEEPROM);
}
void terimaDataDariSana(uint8_t &urutan, bool &kondisi) {
  switch (urutan) {
    case 1:
      udp.read(receivedBuffer, UDP_TX_PACKET_MAX_SIZE);
      setDetikTerjadiTWDiSana(atoi(receivedBuffer));
      receivedBuffer[packetSize] = '\0';
      packetSize = 0;
      urutan++;
      break;
    case 2:
      udp.read(receivedBuffer, UDP_TX_PACKET_MAX_SIZE);
      setMenitTerjadiTWDiSana(atoi(receivedBuffer));
      receivedBuffer[packetSize] = '\0';
      packetSize = 0;
      urutan++;
      break;
    case 3:
      udp.read(receivedBuffer, UDP_TX_PACKET_MAX_SIZE);
      setJamTerjadiTWDiSana(atoi(receivedBuffer));
      receivedBuffer[packetSize] = '\0';
      packetSize = 0;
      urutan++;
      break;
    case 4:
      udp.read(receivedBuffer, UDP_TX_PACKET_MAX_SIZE);
      setWaktuMicroTWTerbacaDiSana(atoi(receivedBuffer));
      receivedBuffer[packetSize] = '\0';
      packetSize = 0;
      urutan++;
      break;
    case 5:
      udp.read(receivedBuffer, UDP_TX_PACKET_MAX_SIZE);
      setJamTersinkronDiSana(atoi(receivedBuffer));
      receivedBuffer[packetSize] = '\0';
      packetSize = 0;
      urutan++;
      break;
    case 6:
      udp.read(receivedBuffer, UDP_TX_PACKET_MAX_SIZE);
      setMenitTersinkronDiSana(atoi(receivedBuffer));
      receivedBuffer[packetSize] = '\0';
      packetSize = 0;
      urutan++;
      break;
    case 7:
      udp.read(receivedBuffer, UDP_TX_PACKET_MAX_SIZE);
      setMenitTersinkronDiSana(atoi(receivedBuffer));
      receivedBuffer[packetSize] = '\0';
      packetSize = 0;
      urutan++;
      break;
    case 8:
      udp.read(receivedBuffer, UDP_TX_PACKET_MAX_SIZE);
      if (receivedBuffer == "Selesai") {
        kondisi = false;
        urutan = 1;
        //tampilkan selesai menerima data & mengkalkulasi
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Selesai Menerima");
        lcd.setCursor(1, 1);
        lcd.print("Mengkalkulasi");
        delay(1500);
      }
      receivedBuffer[packetSize] = '\0';
      packetSize = 0;
      break;
    default:
      break;
  }
}
void kirimWaktuTersinkronDiSini() {
  itoa(getDetikTersinkronDiSini(), UDP_TX_Buffer, 10);
  udp.beginPacket(remote_IP, remote_Port);
  udp.write(UDP_TX_Buffer);
  udp.endPacket();
  delay(delayPacket);
  itoa(getMenitTersinkronDiSini(), UDP_TX_Buffer, 10);
  udp.beginPacket(remote_IP, remote_Port);
  udp.write(UDP_TX_Buffer);
  udp.endPacket();
  delay(delayPacket);
  itoa(getJamTersinkronDiSini(), UDP_TX_Buffer, 10);
  udp.beginPacket(remote_IP, remote_Port);
  udp.write(UDP_TX_Buffer);
  udp.endPacket();
  delay(delayPacket);
}
void kirimWaktuTWTerbacaDiSini() {
  itoa(getDetikTerjadiTWDiSini(), UDP_TX_Buffer, 10);
  udp.beginPacket(remote_IP, remote_Port);
  udp.write(UDP_TX_Buffer);
  udp.endPacket();
  delay(delayPacket);
  itoa(getMenitTerjadiTWDiSini(), UDP_TX_Buffer, 10);
  udp.beginPacket(remote_IP, remote_Port);
  udp.write(UDP_TX_Buffer);
  udp.endPacket();
  delay(delayPacket);
  itoa(getJamTerjadiTWDiSini(), UDP_TX_Buffer, 10);
  udp.beginPacket(remote_IP, remote_Port);
  udp.write(UDP_TX_Buffer);
  udp.endPacket();
  delay(delayPacket);
}
void kirimWaktuMicroTWTerbacaDiSini() {
  ultoa(waktuMicroTWTerbacaDiSini, UDP_TX_Buffer, 10);
  udp.beginPacket(remote_IP, remote_Port);
  udp.write(UDP_TX_Buffer);
  udp.endPacket();
  delay(delayPacket);
}
void kirimKonfirmasiSelesaiPengiriman() {
  udp.beginPacket(remote_IP, remote_Port);
  udp.write("Selesai");
  udp.endPacket();
  delay(delayPacket);
}
void eksekusiTombol() {
  while (isModeSetting(true)) {
    tampil(getHalaman());
    bacaInputTombol();
    delay(delayBacaInputTombol);
    switch (jenisTombol) {
      case JENIS_TOMBOL_SELECT :
        eksekusiTombolSelect();
        break;
      case JENIS_TOMBOL_UP :
        eksekusiTombolUp();
        break;
      case JENIS_TOMBOL_DOWN :
        eksekusiTombolDown();
        break;
      case JENIS_TOMBOL_LEFT :
        eksekusiTombolLeft();
        break;
      case JENIS_TOMBOL_RIGHT :
        eksekusiTombolRight();
        break;
      default:
        break;
    }
  }
}
void tampil(uint8_t hal) {
  switch (hal) {
    case JENIS_HALAMAN_SETTING_12:
      if (isHalamanSebelumnya(getHalaman()) && isBarisPanahSebelumnya(getBarisPanah())) {
        return;
      } else {
        lcd.clear();
        lcd.noCursor();
        lcd.setCursor(1, 0);
        lcd.print("1. IP Setting");
        lcd.setCursor(1, 1);
        lcd.print("2. Port Setting");
        lcd.setCursor(getKolomPanah(), getBarisPanah());
        lcd.write(karakterPanah);
        setHalamanSebelumnya(getHalaman());
        setBarisPanahSebelumnya(getBarisPanah());
      }
      break;
    case JENIS_HALAMAN_SETTING_34:
      if (isHalamanSebelumnya(getHalaman()) && isBarisPanahSebelumnya(getBarisPanah())) {
        return;
      } else {
        lcd.clear();
        lcd.noCursor();
        lcd.setCursor(1, 0);
        lcd.print("3. Device Conf");
        lcd.setCursor(1, 1);
        lcd.print("4. Dist Setting");
        lcd.setCursor(getKolomPanah(), getBarisPanah());
        lcd.write(karakterPanah);
        setHalamanSebelumnya(getHalaman());
        setBarisPanahSebelumnya(getBarisPanah());
      }
      break;
    case JENIS_HALAMAN_SETTING_5:
      if (isHalamanSebelumnya(getHalaman()) && isBarisPanahSebelumnya(getBarisPanah())) {
        return;
      } else {
        lcd.clear();
        lcd.noCursor();
        lcd.setCursor(1, 0);
        lcd.print("5. Dist Saved");
        lcd.setCursor(1, 1);
        lcd.print("6. Finish");
        lcd.setCursor(getKolomPanah(), getBarisPanah());
        lcd.write(karakterPanah);
        setHalamanSebelumnya(getHalaman());
        setBarisPanahSebelumnya(getBarisPanah());
      }
      break;
    case JENIS_HALAMAN_SETTING_IP:
      if (isHalamanSebelumnya(getHalaman()) && isRIPBaru(rIPBaruSebelumnya)) {
        if (isKolomKursor(getKolomKursorSebelumnya())) {
          return;
        } else {
          lcd.noCursor();
          lcd.setCursor(getKolomKursor(), getBarisKursor());
          lcd.cursor();
          setKolomKursorSebelumnya(getKolomKursor());
        }
        return;
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(remote_IP);
        printIPBaru();
        lcd.setCursor(getKolomKursor(), getBarisKursor());
        if (isHalamanSebelumnya(getHalaman())) {
          lcd.setCursor(getKolomKursor(), getBarisKursor());
        } else {
          lcd.setCursor(0, 1);
          setKolomKursor(0);
        }
        lcd.cursor();
        setHalamanSebelumnya(getHalaman());
        setRIPBaruSebelumnya(rIPBaru);
      }
      break;
    case JENIS_HALAMAN_SETTING_PORT:
      if (isHalamanSebelumnya(getHalaman()) && isRemotePortBaru(getRemotePortBaruSebelumnya())) {
        if (isKolomKursor(getKolomKursorSebelumnya())) {
          return;
        } else {
          lcd.noCursor();
          lcd.setCursor(getKolomKursor(), getBarisKursor());
          lcd.cursor();
          setKolomKursorSebelumnya(getKolomKursor());
        }
        return;
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("OLD");
        lcd.setCursor(4, 0);
        lcd.print(getRemotePort());
        lcd.setCursor(0, 1);
        lcd.print("NEW");
        printRemotePortBaru();
        lcd.setCursor(getKolomKursor(), getBarisKursor());
        lcd.setCursor(0, 1);
        if (isHalamanSebelumnya(getHalaman())) {
          lcd.setCursor(getKolomKursor(), getBarisKursor());
        } else {
          lcd.setCursor(4, 1);
          setKolomKursor(4);
        }
        lcd.cursor();
        setHalamanSebelumnya(getHalaman());
        setRemotePortBaruSebelumnya(getRemotePortBaru());
      }
      break;
    case JENIS_HALAMAN_DEVICE_SETTING:
      if (isHalamanSebelumnya(getHalaman())) {
        return;
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(ip);
        lcd.setCursor(0, 1);
        lcd.print(localPort);
        lcd.setCursor(6, 1);
        lcd.print(getJarak());
        if (getJarak() < 100) {
          lcd.setCursor(9, 1);
        } else if (getJarak() >= 100) {
          lcd.setCursor(10, 1);
        }
        lcd.print("KM");
        setHalamanSebelumnya(getHalaman());
      }
      break;
    case JENIS_HALAMAN_SETTING_JARAK:
      if (isHalamanSebelumnya(getHalaman()) && isJarakBaru(getJarakBaruSebelumnya())) {
        if (isKolomKursor(getKolomKursorSebelumnya())) {
          return;
        } else {
          lcd.noCursor();
          lcd.setCursor(getKolomKursor(), getBarisKursor());
          lcd.cursor();
          setKolomKursorSebelumnya(getKolomKursor());
        }
        return;
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("OLD");
        lcd.setCursor(4, 0);
        lcd.print(getJarak());
        lcd.setCursor(8, 0);
        lcd.print("KM");
        lcd.setCursor(0, 1);
        lcd.print("NEW");
        printJarakBaru();
        lcd.setCursor(8, 1);
        lcd.print("KM");
        lcd.setCursor(getKolomKursor(), getBarisKursor());
        lcd.setCursor(0, 1);
        if (isHalamanSebelumnya(getHalaman())) {
          lcd.setCursor(getKolomKursor(), getBarisKursor());
        } else {
          lcd.setCursor(4, 1);
          setKolomKursor(4);
        }
        lcd.cursor();
        setHalamanSebelumnya(getHalaman());
        setJarakBaruSebelumnya(getJarakBaru());
      }
      break;
    case JENIS_HALAMAN_PRE_TW:
      if (isHalamanSebelumnya(getHalaman())) {
        return;
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Kirim Data dan");
        lcd.setCursor(0, 1);
        lcd.print("Terima Data");
        setHalamanSebelumnya(getHalaman());
      }
      break;
    case JENIS_HALAMAN_POST_TW:
      if (isHalamanSebelumnya(getHalaman())) {
        return;
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Jarak Error");
        lcd.setCursor(0, 1);
        lcd.print(getJarakErrorDariSini());
        lcd.setCursor(11, 1);
        lcd.print("METER");
        setHalamanSebelumnya(getHalaman());
      }
      break;
    case JENIS_HALAMAN_ERROR_EEPROM:
      if (isHalamanSebelumnya(getHalaman())) {
        return;
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Jarak di EEPROM");
        lcd.setCursor(0, 1);
        lcd.print(getJarakErrorDiEEPROM());
        lcd.setCursor(10, 1);
        lcd.print("METER");
        setHalamanSebelumnya(getHalaman());
      }
      break;
  }
}
void bacaInputTombol() {
  nilaiAnalogPinPBLCD = analogRead(pinPBLCD);
  if (nilaiAnalogPinPBLCD > 1000) {
    jenisTombol = JENIS_TOMBOL_NONE;
  } else if ( (nilaiAnalogPinPBLCD > 600) && (nilaiAnalogPinPBLCD <= 800) ) {
    jenisTombol = JENIS_TOMBOL_SELECT;
  } else if ( (nilaiAnalogPinPBLCD > 450) && (nilaiAnalogPinPBLCD <= 550) ) {
    jenisTombol = JENIS_TOMBOL_LEFT;
  } else if ( (nilaiAnalogPinPBLCD > 250) && (nilaiAnalogPinPBLCD <= 400) ) {
    jenisTombol = JENIS_TOMBOL_DOWN;
  } else if ( (nilaiAnalogPinPBLCD > 100) && (nilaiAnalogPinPBLCD <= 200) ) {
    jenisTombol = JENIS_TOMBOL_UP;
  } else if ( (nilaiAnalogPinPBLCD >= 0) && (nilaiAnalogPinPBLCD <= 75) ) {
    jenisTombol = JENIS_TOMBOL_RIGHT;
  } else {
    jenisTombol = JENIS_TOMBOL_NONE;
  }
}
void eksekusiTombolSelect() {
  if (isHalaman(JENIS_HALAMAN_SETTING_12)) {
    if (isBarisPanah(0)) {
      setHalaman(JENIS_HALAMAN_SETTING_IP);
    } else if (isBarisPanah(1)) {
      setHalaman(JENIS_HALAMAN_SETTING_PORT);
    } else {
      return;
    }
  } else if (isHalaman(JENIS_HALAMAN_SETTING_34)) {
    if (isBarisPanah(0)) {
      setHalaman(JENIS_HALAMAN_DEVICE_SETTING);
    } else if (isBarisPanah(1)) {
      setHalaman(JENIS_HALAMAN_SETTING_JARAK);
    } else {
      return;
    }
  } else if (isHalaman(JENIS_HALAMAN_SETTING_5)) {
    if (isBarisPanah(0)) {
      setHalaman(JENIS_HALAMAN_ERROR_EEPROM);
    } else if (isBarisPanah(1)) {
      setModeSetting(false);
    } else {
      return;
    }
  } else if (isHalaman(JENIS_HALAMAN_SETTING_IP)) {
    setHalaman(JENIS_HALAMAN_SETTING_12);
    setBarisPanah(0);
    setKolomKursor(0);
  } else if (isHalaman(JENIS_HALAMAN_SETTING_PORT)) {
    setHalaman(JENIS_HALAMAN_SETTING_12);
    setBarisPanah(0);
    setKolomKursor(4);
  } else if (isHalaman(JENIS_HALAMAN_DEVICE_SETTING)) {
    setHalaman(JENIS_HALAMAN_SETTING_12);
    setBarisPanah(0);
  } else if (isHalaman(JENIS_HALAMAN_SETTING_JARAK)) {
    setHalaman(JENIS_HALAMAN_SETTING_12);
    setBarisPanah(0);
    setKolomKursor(4);
  } else if (isHalaman(JENIS_HALAMAN_ERROR_EEPROM)) {
    setHalaman(JENIS_HALAMAN_SETTING_12);
    setBarisPanah(0);
  } else {
    return;
  }
}
void eksekusiTombolUp() {
  if (isHalaman(JENIS_HALAMAN_SETTING_12)) {
    if (isBarisPanah(0)) {
      return;
    } else if (isBarisPanah(1)) {
      setBarisPanah(0);
    } else {
      return;
    }
  } else if (isHalaman(JENIS_HALAMAN_SETTING_34)) {
    if (isBarisPanah(0)) {
      setBarisPanah(1);
      setHalaman(JENIS_HALAMAN_SETTING_12);
    } else if (isBarisPanah(1)) {
      setBarisPanah(0);
    } else {
      return;
    }
  } else if (isHalaman(JENIS_HALAMAN_SETTING_5)) {
    if (isBarisPanah(0)) {
      setBarisPanah(1);
      setHalaman(JENIS_HALAMAN_SETTING_34);
    } else if (isBarisPanah(1)) {
      setBarisPanah(0);
    } else {
      return;
    }
  } else if (isHalaman(JENIS_HALAMAN_SETTING_IP)) {
    increaseIP();
  } else if (isHalaman(JENIS_HALAMAN_SETTING_PORT)) {
    increasePort();
  } else if (isHalaman(JENIS_HALAMAN_DEVICE_SETTING)) {
    return;
  } else if (isHalaman(JENIS_HALAMAN_SETTING_JARAK)) {
    increaseJarak();
  } else {
    return;
  }
}
void eksekusiTombolDown() {
  if (isHalaman(JENIS_HALAMAN_SETTING_12)) {
    if (isBarisPanah(0)) {
      setBarisPanah(1);
    } else if (isBarisPanah(1)) {
      setBarisPanah(0);
      setHalaman(JENIS_HALAMAN_SETTING_34);
    } else {
      return;
    }
  } else if (isHalaman(JENIS_HALAMAN_SETTING_34)) {
    if (isBarisPanah(0)) {
      setBarisPanah(1);
    } else if (isBarisPanah(1)) {
      setBarisPanah(0);
      setHalaman(JENIS_HALAMAN_SETTING_5);
    } else {
      return;
    }
  } else if (isHalaman(JENIS_HALAMAN_SETTING_5)) {
    if (isBarisPanah(0)) {
      setBarisPanah(1);
    } else if (isBarisPanah(1)) {
      return;
    } else {
      return;
    }
  } else if (isHalaman(JENIS_HALAMAN_SETTING_IP)) {
    decreaseIP();
  } else if (isHalaman(JENIS_HALAMAN_SETTING_PORT)) {
    decreasePort();
  } else if (isHalaman(JENIS_HALAMAN_DEVICE_SETTING)) {
    return;
  } else if (isHalaman(JENIS_HALAMAN_SETTING_JARAK)) {
    decreaseJarak();
  } else {
    return;
  }
}
void eksekusiTombolLeft() {
  if (isHalaman(JENIS_HALAMAN_SETTING_12)) {
    return;
  } else if (isHalaman(JENIS_HALAMAN_SETTING_34)) {
    return;
  } else if (isHalaman(JENIS_HALAMAN_SETTING_5)) {
    return;
  } else if (isHalaman(JENIS_HALAMAN_SETTING_IP)) {
    if (isKolomKursor(0)) {
      return;
    } else {
      setKolomKursor(getKolomKursor() - 1);
    }
  } else if (isHalaman(JENIS_HALAMAN_SETTING_PORT)) {
    if (isKolomKursor(4)) {
      return;
    } else {
      setKolomKursor(getKolomKursor() - 1);
    }
  } else if (isHalaman(JENIS_HALAMAN_DEVICE_SETTING)) {
    return;
  } else if (isHalaman(JENIS_HALAMAN_SETTING_JARAK)) {
    if (isKolomKursor(4)) {
      return;
    } else {
      setKolomKursor(getKolomKursor() - 1);
    }
  } else {
    return;
  }
}
void eksekusiTombolRight() {
  if (isHalaman(JENIS_HALAMAN_SETTING_12)) {
    return;
  } else if (isHalaman(JENIS_HALAMAN_SETTING_34)) {
    return;
  } else if (isHalaman(JENIS_HALAMAN_SETTING_5)) {
    return;
  } else if (isHalaman(JENIS_HALAMAN_SETTING_IP)) {
    if (isKolomKursor(14)) {
      return;
    } else {
      setKolomKursor(getKolomKursor() + 1);
    }
  } else if (isHalaman(JENIS_HALAMAN_SETTING_PORT)) {
    if (isKolomKursor(7)) {
      return;
    } else {
      setKolomKursor(getKolomKursor() + 1);
    }
  } else if (isHalaman(JENIS_HALAMAN_DEVICE_SETTING)) {
    return;
  } else if (isHalaman(JENIS_HALAMAN_SETTING_JARAK)) {
    if (isKolomKursor(6)) {
      return;
    } else {
      setKolomKursor(getKolomKursor() + 1);
    }
  } else {
    return;
  }
}
void increaseIP() {
  switch (getKolomKursor()) {
    case 0:
      if (rIPBaru[0] < 155) {
        rIPBaru[0] += 100;
      } else {
        return;
      }
      break;
    case 1:
      if (rIPBaru[0] < 245) {
        rIPBaru[0] += 10;
      } else {
        return;
      }
      break;
    case 2:
      if (rIPBaru[0] < 255) {
        rIPBaru[0] += 1;
      } else {
        return;
      }
      break;
    case 4:
      if (rIPBaru[1] < 155) {
        rIPBaru[1] += 100;
      } else {
        return;
      }
      break;
    case 5:
      if (rIPBaru[1] < 245) {
        rIPBaru[1] += 10;
      } else {
        return;
      }
      break;
    case 6:
      if (rIPBaru[1] < 255) {
        rIPBaru[1] += 1;
      } else {
        return;
      }
      break;
    case 8:
      if (rIPBaru[2] < 155) {
        rIPBaru[2] += 100;
      } else {
        return;
      }
      break;
    case 9:
      if (rIPBaru[2] < 245) {
        rIPBaru[2] += 10;
      } else {
        return;
      }
      break;
    case 10:
      if (rIPBaru[2] < 255) {
        rIPBaru[2] += 1;
      } else {
        return;
      }
      break;
    case 12:
      if (rIPBaru[3] < 155) {
        rIPBaru[3] += 100;
      } else {
        return;
      }
      break;
    case 13:
      if (rIPBaru[3] < 245) {
        rIPBaru[3] += 10;
      } else {
        return;
      }
      break;
    case 14:
      if (rIPBaru[3] < 255) {
        rIPBaru[3] += 1;
      } else {
        return;
      }
      break;
    default:
      return;
      break;
  }
  IPAddress rIP(rIPBaru[0], rIPBaru[1], rIPBaru[2], rIPBaru[3]);
  remote_IP = rIP;
}
void decreaseIP() {
  switch (getKolomKursor()) {
    case 0:
      if (rIPBaru[0] >= 100) {
        rIPBaru[0] -= 100;
      } else {
        return;
      }
      break;
    case 1:
      if (rIPBaru[0] >= 10) {
        rIPBaru[0] -= 10;
      } else {
        return;
      }
      break;
    case 2:
      if (rIPBaru[0] > 0) {
        rIPBaru[0] -= 1;
      } else {
        return;
      }
      break;
    case 4:
      if (rIPBaru[1] >= 100) {
        rIPBaru[1] -= 100;
      } else {
        return;
      }
      break;
    case 5:
      if (rIPBaru[1] >= 10) {
        rIPBaru[1] -= 10;
      } else {
        return;
      }
      break;
    case 6:
      if (rIPBaru[1] > 0) {
        rIPBaru[1] -= 1;
      } else {
        return;
      }
      break;
    case 8:
      if (rIPBaru[2] >= 100) {
        rIPBaru[2] -= 100;
      } else {
        return;
      }
      break;
    case 9:
      if (rIPBaru[2] >= 10) {
        rIPBaru[2] -= 10;
      } else {
        return;
      }
      break;
    case 10:
      if (rIPBaru[2] > 0) {
        rIPBaru[2] -= 1;
      } else {
        return;
      }
      break;
    case 12:
      if (rIPBaru[3] >= 100) {
        rIPBaru[3] -= 100;
      } else {
        return;
      }
      break;
    case 13:
      if (rIPBaru[3] >= 10) {
        rIPBaru[3] -= 10;
      } else {
        return;
      }
      break;
    case 14:
      if (rIPBaru[3] > 0) {
        rIPBaru[3] -= 1;
      } else {
        return;
      }
      break;
    default:
      return;
      break;
  }
  IPAddress rIP(rIPBaru[0], rIPBaru[1], rIPBaru[2], rIPBaru[3]);
  remote_IP = rIP;
}
void increasePort() {
  switch (getKolomKursor()) {
    case 4:
      if (getRemotePortBaru() <= 8999) {
        setRemotePortBaru(getRemotePortBaru() + 1000);
      } else {
        return;
      }
      break;
    case 5:
      if (getRemotePortBaru() <= 9899) {
        setRemotePortBaru(getRemotePortBaru() + 100);
      } else {
        return;
      }
      break;
    case 6:
      if (getRemotePortBaru() <= 9989) {
        setRemotePortBaru(getRemotePortBaru() + 10);
      } else {
        return;
      }
      break;
    case 7:
      if (getRemotePortBaru() <= 9998) {
        setRemotePortBaru(getRemotePortBaru() + 1);
      } else {
        return;
      }
      break;
    default:
      return;
      break;
  }
  setRemotePort(getRemotePortBaru());
}
void decreasePort() {
  switch (getKolomKursor()) {
    case 4:
      if (getRemotePortBaru() >= 2000) {
        setRemotePortBaru(getRemotePortBaru() - 1000);
      } else {
        return;
      }
      break;
    case 5:
      if (getRemotePortBaru() >= 1100) {
        setRemotePortBaru(getRemotePortBaru() - 100);
      } else {
        return;
      }
      break;
    case 6:
      if (getRemotePortBaru() >= 1010) {
        setRemotePortBaru(getRemotePortBaru() - 10);
      } else {
        return;
      }
      break;
    case 7:
      if (getRemotePortBaru() >= 1001) {
        setRemotePortBaru(getRemotePortBaru() - 1);
      } else {
        return;
      }
      break;
    default:
      return;
      break;
  }
  setRemotePort(getRemotePortBaru());
}
void increaseJarak() {
  switch (getKolomKursor()) {
    case 4:
      if (getJarakBaru() <= 899) {
        setJarakBaru(getJarakBaru() + 100);
      } else {
        return;
      }
      break;
    case 5:
      if (getJarakBaru() <= 989) {
        setJarakBaru(getJarakBaru() + 10);
      } else {
        return;
      }
      break;
    case 6:
      if (getJarakBaru() <= 998) {
        setJarakBaru(getJarakBaru() + 1);
      } else {
        return;
      }
      break;
    default:
      return;
      break;
  }
  setJarak(getJarakBaru());
}
void decreaseJarak() {
  switch (getKolomKursor()) {
    case 4:
      if (getJarakBaru() >= 100) {
        setJarakBaru(getJarakBaru() - 100);
      } else {
        return;
      }
      break;
    case 5:
      if (getJarakBaru() >= 10) {
        setJarakBaru(getJarakBaru() - 10);
      } else {
        return;
      }
      break;
    case 6:
      if (getJarakBaru() >= 1) {
        setJarakBaru(getJarakBaru() - 1);
      } else {
        return;
      }
      break;
    default:
      return;
      break;
  }
  setJarak(getJarakBaru());
}
void printIPBaru() {
  lcd.setCursor(0, 1);
  lcd.print(rIPBaru[0] / 100);
  lcd.setCursor(1, 1);
  lcd.print((rIPBaru[0] % 100) / 10);
  lcd.setCursor(2, 1);
  lcd.print(rIPBaru[0] % 10);
  lcd.setCursor(3, 1);
  lcd.print('.');
  lcd.setCursor(4, 1);
  lcd.print(rIPBaru[1] / 100);
  lcd.setCursor(5, 1);
  lcd.print((rIPBaru[1] % 100) / 10);
  lcd.setCursor(6, 1);
  lcd.print(rIPBaru[1] % 10);
  lcd.setCursor(7, 1);
  lcd.print('.');
  lcd.setCursor(8, 1);
  lcd.print(rIPBaru[2] / 100);
  lcd.setCursor(9, 1);
  lcd.print((rIPBaru[2] % 100) / 10);
  lcd.setCursor(10, 1);
  lcd.print(rIPBaru[2] % 10);
  lcd.setCursor(11, 1);
  lcd.print('.');
  lcd.setCursor(12, 1);
  lcd.print(rIPBaru[3] / 100);
  lcd.setCursor(13, 1);
  lcd.print((rIPBaru[3] % 100) / 10);
  lcd.setCursor(14, 1);
  lcd.print(rIPBaru[3] % 10);
}
void printRemotePortBaru() {
  if (getRemotePortBaru() < 10) {
    lcd.setCursor(4, 1);
    lcd.print("000");
    lcd.setCursor(7, 1);
    lcd.print(getRemotePortBaru());
  } else if (getRemotePortBaru() >= 10 && getRemotePortBaru() < 100) {
    lcd.setCursor(4, 1);
    lcd.print("00");
    lcd.setCursor(6, 1);
    lcd.print(getRemotePortBaru());
  } else if (getRemotePortBaru() >= 100 && getRemotePortBaru() < 1000) {
    lcd.setCursor(4, 1);
    lcd.print('0');
    lcd.setCursor(5, 1);
    lcd.print(getRemotePortBaru());
  } else {
    lcd.setCursor(4, 1);
    lcd.print(getRemotePortBaru());
  }
}
void printJarakBaru() {
  if (getJarakBaru() < 10) {
    lcd.setCursor(4, 1);
    lcd.print("00");
    lcd.setCursor(6, 1);
    lcd.print(getJarakBaru());
  } else if (getJarakBaru() >= 10 && getJarakBaru() < 100) {
    lcd.setCursor(4, 1);
    lcd.print('0');
    lcd.setCursor(5, 1);
    lcd.print(getJarakBaru());
  } else {
    lcd.setCursor(4, 1);
    lcd.print(getJarakBaru());
  }
}
void standby() {
  if (getModeStandby()) {
    tampilWaktu();
    return;
  } else {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("STANDBY");
    tampilWaktu();
    setModeStandby(true);
  }
}
void tampilWaktu() {
  if (softSerial.available() > 0) {
    if (gps.encode(softSerial.read())) {
      if (gps.time.isValid()) {
        Time[6]  = (gps.time.hour() + 7)   / 10 + 48;
        Time[7]  = (gps.time.hour() + 7)   % 10 + 48;
        Time[9]  = gps.time.minute() / 10 + 48;
        Time[10]  = gps.time.minute() % 10 + 48;
        Time[12] = gps.time.second() / 10 + 48;
        Time[13] = gps.time.second() % 10 + 48;
      }
      if (last_second != gps.time.second()) {
        last_second = gps.time.second();
        lcd.setCursor(1, 1);
        lcd.print(Time);
      }
    }
  }
}

//------------------------------------------------------------------------------------------//
//----------------------------------------FUNGSI SET----------------------------------------//
void setHalaman(uint8_t hal) {
  halaman = hal;
}
void setHalamanSebelumnya(uint8_t hal) {
  halamanSebelumnya = hal;
}
void setBarisPanah(uint8_t bp) {
  barisPanah = bp;
}
void setKolomPanah(uint8_t kp) {
  kolomPanah = kp;
}
void setModeSetting(bool ms) {
  modeSetting = ms;
}
void setBarisKursor(uint8_t bk) {
  barisKursor = bk;
}
void setKolomKursor(uint8_t kk) {
  kolomKursor = kk;
}
void setModeStandby(bool ms) {
  modeStandby = ms;
}
void setBarisPanahSebelumnya(uint8_t bps) {
  barisPanahSebelumnya = bps;
}
void setJarak(uint16_t j) {
  jarak = j;
}
void setJarakBaru(uint16_t jb) {
  jarakBaru = jb;
}
void setJarakBaruSebelumnya(uint16_t jbs) {
  jarakBaruSebelumnya = jbs;
}
void setKolomKursorSebelumnya(uint8_t kks) {
  kolomKursorSebelumnya = kks;
}
void setRIPBaruSebelumnya(uint8_t rip[]) {
  for (int i = 0; i <= 3; i++) {
    rIPBaruSebelumnya[i] = rip[i];
  }
}
void setRemotePort(int rp) {
  remote_Port = rp;
}
void setRemotePortBaru(int rpb) {
  remote_Port_Baru = rpb;
}
void setRemotePortBaruSebelumnya(int rpbs) {
  remote_Port_Baru_Sebelumnya = rpbs;
}
void setJamTersinkronDiSini(uint8_t j) {
  jamTersinkronDiSini = j;
}
void setMenitTersinkronDiSini(uint8_t m) {
  menitTersinkronDiSini = m;
}
void setDetikTersinkronDiSini(uint8_t d) {
  detikTersinkronDiSini = d;
}
void setJamTersinkronDiSana(uint8_t j) {
  jamTersinkronDiSana = j;
}
void setMenitTersinkronDiSana(uint8_t m) {
  menitTersinkronDiSana = m;
}
void setDetikTersinkronDiSana(uint8_t d) {
  detikTersinkronDiSana = d;
}
void setJamTersinkron(uint8_t j) {
  jamTersinkron = j;
}
void setMenitTersinkron(uint8_t m) {
  menitTersinkron = m;
}
void setDetikTersinkron(uint8_t d) {
  detikTersinkron = d;
}
void setJamTerjadiTWDiSini(uint8_t j) {
  jamTerjadiTWDiSini = j;
}
void setMenitTerjadiTWDiSini(uint8_t m) {
  menitTerjadiTWDiSini = m;
}
void setDetikTerjadiTWDiSini(uint8_t d) {
  detikTerjadiTWDiSini = d;
}
void setJamTerjadiTWDiSana(uint8_t j) {
  jamTerjadiTWDiSana = j;
}
void setMenitTerjadiTWDiSana(uint8_t m) {
  menitTerjadiTWDiSana = m;
}
void setDetikTerjadiTWDiSana(uint8_t d) {
  detikTerjadiTWDiSana = d;
}
void setWaktuMicroTWTerbacaDiSini(int64_t t) {
  waktuMicroTWTerbacaDiSini = t;
}
void setWaktuMicroTWTerbacaDiSana(int64_t t) {
  waktuMicroTWTerbacaDiSana = t;
}
void setJarakErrorDariSini(int32_t j) {
  jarakErrorDariSini = j;
}
void setWaktuMicroTersinkronDiSini(int64_t t) {
  waktuMicroTersinkronDiSini = t;
}
void setWaktuMicroTersinkronDiSana(int64_t t) {
  waktuMicroTersinkronDiSana = t;
}
void setWaktuMicroTersinkron(int64_t t) {
  waktuMicroTersinkron = t;
}
void setSelisihWaktuMicro(int64_t t) {
  selisihWaktuMicro = t;
}
void setDiSiniLebihBesar(int x) {
  diSiniLebihBesar = x;
}
void setJarakErrorDiEEPROM(int32_t x) {
  jarakErrorDiEEPROM = x;
}
//---------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------//



//---------------------------------------------------------------------------------//
//------------------------------------FUNGSI GET-----------------------------------//
uint8_t getHalaman() {
  return halaman;
}
uint8_t getHalamanSebelumnya() {
  return halamanSebelumnya;
}
uint8_t getBarisPanah() {
  return barisPanah;
}
uint8_t getKolomPanah() {
  return kolomPanah;
}
bool getModeSetting() {
  return modeSetting;
}
uint8_t getBarisKursor() {
  return barisKursor;
}
uint8_t getKolomKursor() {
  return kolomKursor;
}
bool getModeStandby() {
  return modeStandby;
}
uint8_t getBarisPanahSebelumnya() {
  return barisPanahSebelumnya;
}
uint16_t getJarak() {
  return jarak;
}
uint16_t getJarakBaru() {
  return jarakBaru;
}
uint16_t getJarakBaruSebelumnya() {
  return jarakBaruSebelumnya;
}
uint8_t getKolomKursorSebelumnya() {
  return kolomKursorSebelumnya;
}
int getRemotePort() {
  return remote_Port;
}
int getRemotePortBaru() {
  return remote_Port_Baru;
}
int getRemotePortBaruSebelumnya() {
  return remote_Port_Baru_Sebelumnya;
}
uint8_t getJamTersinkronDiSini() {
  return jamTersinkronDiSini;
}
uint8_t getMenitTersinkronDiSini() {
  return menitTersinkronDiSini;
}
uint8_t getDetikTersinkronDiSini() {
  return detikTersinkronDiSini;
}
uint8_t getJamTersinkronDiSana() {
  return jamTersinkronDiSana;
}
uint8_t getMenitTersinkronDiSana() {
  return menitTersinkronDiSana;
}
uint8_t getDetikTersinkronDiSana() {
  return detikTersinkronDiSana;
}
uint8_t getJamTersinkron() {
  return jamTersinkron;
}
uint8_t getMenitTersinkron() {
  return menitTersinkron;
}
uint8_t getDetikTersinkron() {
  return detikTersinkron;
}
uint8_t getJamTerjadiTWDiSini() {
  return jamTerjadiTWDiSini;
}
uint8_t getMenitTerjadiTWDiSini() {
  return menitTerjadiTWDiSini;
}
uint8_t getDetikTerjadiTWDiSini() {
  return detikTerjadiTWDiSini;
}
uint8_t getJamTerjadiTWDiSana() {
  return jamTerjadiTWDiSana;
}
uint8_t getMenitTerjadiTWDiSana() {
  return menitTerjadiTWDiSana;
}
uint8_t getDetikTerjadiTWDiSana() {
  return detikTerjadiTWDiSana;
}
int64_t getWaktuMicroTWTerbacaDiSini() {
  return waktuMicroTWTerbacaDiSini;
}
int64_t getWaktuMicroTWTerbacaDiSana() {
  return waktuMicroTWTerbacaDiSana;
}
int32_t getJarakErrorDariSini() {
  return jarakErrorDariSini;
}
int64_t getWaktuMicroTersinkronDiSini() {
  return waktuMicroTersinkronDiSini;
}
int64_t getWaktuMicroTersinkronDiSana() {
  return waktuMicroTersinkronDiSana;
}
int64_t getWaktuMicroTersinkron() {
  return waktuMicroTersinkron;
}
int64_t getSelisihWaktuMicro() {
  return selisihWaktuMicro;
}
int getDiSiniLebihBesar() {
  return diSiniLebihBesar;
}
int32_t getJarakErrorDiEEPROM() {
  return jarakErrorDiEEPROM;
}
//-------------------------------------------------------------//

void setup() {
  inisialisasiLCD();
  inisialisasiGPS();
  inisialisasiEthernet();
  waktuMicroTersinkronDiSini = micros();
  setDetikTersinkronDiSini(gps.time.second());
  setMenitTersinkronDiSini(gps.time.minute());
  setJamTersinkronDiSini(gps.time.hour() + 7);
  pinMode(pinPBLCD, INPUT_PULLUP);
  pinMode(pinPBTW, INPUT_PULLUP);
  setHalamanSebelumnya(100);
  setBarisPanahSebelumnya(100);
  setKolomKursorSebelumnya(100);
  uint8_t ripSementara[] = {1, 1, 1, 1};
  setRIPBaruSebelumnya(ripSementara);
  setBarisPanah(0);
  setKolomPanah(0);
  setBarisKursor(1);
  setKolomKursor(0);
  setHalaman(0);
  setJarak(50);
  setJarakBaru(0);
  setJarakBaruSebelumnya(999);
  setRemotePortBaruSebelumnya(9999);
  EEPROM.get(ALAMAT_JARAK_ERROR_DI_EEPROM, jarakErrorDiEEPROM);
  //Serial.begin(115200);
  lcd.clear();
}
void loop() {
  bool isInputExist = adaInput();
  if (isInputExist) {
    eksekusiInput();
    setModeStandby(false);
  } else {
    standby();
  }
}
