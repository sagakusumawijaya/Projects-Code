#include <Servo.h>

// Receive a NDEF message from a Peer
// Requires SPI. Tested with Seeed Studio NFC Shield v2

#include "SPI.h"
#include "PN532_SPI.h"
#include "snep.h"
#include "NdefMessage.h"
#include <NfcAdapter.h>



#define ss D2
#define pinServo D1
#define pinSolenoid D3

Servo motorServo;

const int posisiTerbuka = 0;
const int posisiTertutup = 120;

const unsigned int MAX_RECORDS = 4;
const unsigned int MAX_PAYLOAD_LENGTH = 200;
const unsigned int MAX_CODE_LENGTH = 11;
const char KODE_AKSES[] = { 'f', 'a', 'u', 'z', 'a', 'n', 'a', 'z', 'u', 'a', 'f' };
bool diizinkanMengakses = false;

bool terbuka = false;

PN532_SPI pn532spi(SPI, ss);
SNEP nfc(pn532spi);
uint8_t ndefBuf[128];

NfcAdapter nfctag = NfcAdapter(pn532spi);

void buka(){
  digitalWrite(pinSolenoid, HIGH);
  delay(500);
  motorServo.write(posisiTerbuka);
  delay(1500);
  digitalWrite(pinSolenoid, LOW);
  delay(500);
  terbuka = true;
}

void tutup(){
  digitalWrite(pinSolenoid, HIGH);
  delay(500);
  motorServo.write(posisiTertutup);
  delay(1500);
  digitalWrite(pinSolenoid, LOW);
  delay(500);
  terbuka = false;
}


void setup() {
  motorServo.attach(pinServo);
  pinMode(pinSolenoid, OUTPUT);
  Serial.begin(9600);
  nfctag.begin();
  Serial.println("DOOR LOCK SYSTEM BY FAUZAN");
}

void loop() {
  Serial.println("Waiting for message from Peer");
  int msgSize = nfc.read(ndefBuf, sizeof(ndefBuf));
  if (msgSize > 0) {
    NdefMessage msg  = NdefMessage(ndefBuf, msgSize);
	byte pl[MAX_RECORDS][MAX_PAYLOAD_LENGTH];
	int plLength = 0;
	for (int i = 0; i < msg.getRecordCount(); i++) {
		msg.getRecord(i).getPayload(pl[i]);
		plLength = msg.getRecord(i).getPayloadLength();
		char kodeDiterima[MAX_CODE_LENGTH];
		if (plLength - 3 != MAX_CODE_LENGTH) {
			diizinkanMengakses = false;
			break;
		}
		for (int j = 3; j < plLength; j++) {
			kodeDiterima[j - 3] = (char)pl[i][j];
			Serial.print(kodeDiterima[j - 3]);
			if (kodeDiterima[j - 3] == KODE_AKSES[j - 3]) {
				diizinkanMengakses = true;
			}
			else {
				diizinkanMengakses = false;
				break;
			}
		}
	}
	Serial.println();
	if (diizinkanMengakses) {
		if (terbuka) {
			Serial.println("TERTUTUP");
			tutup();
		}
		else {
			Serial.println("TERBUKA");
			buka();
		}
	}
    
  }
  else if (nfctag.tagPresent()) {
	  NfcTag tag = nfctag.read();
	  tag.print();
  }
  else {
    Serial.println("Failed");
  }
  delay(3000);
}
