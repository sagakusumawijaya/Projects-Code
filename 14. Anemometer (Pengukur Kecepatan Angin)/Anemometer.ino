/*
 Name:    Project_Kecepatan_Angin.ino
 Created: 5/27/2019 06:39:41 PM
 Author:  Saga Kusuma Wijaya
*/
#include <LiquidCrystal.h>

#define relayHigh     13
#define relayLow      11
#define ledHigh       9
#define ledLow        8
#define buttonUp      7
#define buttonDown      6
#define windMeterValuePin A0
#define Active        LOW
#define Inactive      HIGH

const uint8_t rs = 12;
const uint8_t en = 10;
const uint8_t d4 = 5;
const uint8_t d5 = 4;
const uint8_t d6 = 3;
const uint8_t d7 = 2;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

bool firstExecOfCondition = true;
bool aktifkanBlinky = false;
bool lowerLimitCondition = false;
bool upperLimitCondition = false;
bool actionCondition = false;
bool ledHighCondition = true;
bool ledLowCondition = false;
bool ledCondition = true;
bool conditionButtonUp = true;
bool conditionButtonDown = true;
bool standbyCondition = true;
uint32_t windVelocity = 0;
uint32_t counterTimeUpperLower = 0;
uint32_t counterTimeNowUpperLower = 0;
uint32_t counterTimeBeforeUpperLower = 0;

const uint32_t UPPER_LIMIT_WIND_VELOCITY = 100;
const uint32_t LOWER_LIMIT_WIND_VELOCITY = 0;
const uint32_t DELAY_BUTTON = 100000;			// 100 mili detik
const uint32_t DELAY_SETTING_BUTTON = 2000000;  // microsecond, digunakan ketika read button long, 2 detik
const uint32_t DELAY_BLINKY = 1000000;			// 1 detik
const uint32_t DELAY_RELAY_ACTIVE = 60000000;	// 60 DETIK, 1 menit

uint32_t microsBlinky = 0;

uint32_t lowerLimit = 25; // batas bawah dalam satuan kpj
uint32_t upperLimit = 45; // batas atas dalam satuan kpj

uint32_t counterWindData = 0;
uint32_t updateInterval = 1000000; // microsecond

uint8_t menu = 0;

const uint8_t SETTING_UPPER_LIMIT = 1;
const uint8_t SETTING_LOWER_LIMIT = 2;

typedef enum {
	noneCondition = 0,
	lowerCondition,
	middleCondition,
	upperCondition
}CONDITION;

CONDITION condition;
CONDITION conditionSebelumnya;

void enableAction(bool en, CONDITION cond = noneCondition);

void setup() {
	Serial.begin(9600);
	while (!Serial);
	pinMode(relayHigh, OUTPUT);
	pinMode(relayLow, OUTPUT);
	pinMode(ledHigh, OUTPUT);
	pinMode(ledLow, OUTPUT);
	pinMode(buttonUp, INPUT);
	pinMode(buttonDown, INPUT);
	pinMode(windMeterValuePin, INPUT);
	lcd.begin(16, 2);
	delay(50);
	lcd.clear();
	lcd.print("Starting");
	delay(2000);
	lcd.clear();
	setCondition(noneCondition);
}

void loop() {
	windVelocity = map(analogRead(windMeterValuePin), 0, 1023,
		LOWER_LIMIT_WIND_VELOCITY, UPPER_LIMIT_WIND_VELOCITY);
	if (readButtonLong()) {
		blinky(false);
		noneAction();
		menuSetting();
		standbyCondition = false;
	}
	else {
		if (windVelocity > lowerLimit && windVelocity < upperLimit) {
			if (isCondition(middleCondition)) {

			}
			else {
				middleAction();
				setCondition(middleCondition);
				displayStandby();
			}
		}
		else if (windVelocity <= lowerLimit) {
			lowerLimitAction();
			setCondition(lowerCondition);
			displayStandby();
			waitBlinky(60000);
		}
		else if (windVelocity >= upperLimit) {
			upperLimitAction();
			setCondition(upperCondition);
			displayStandby();
			waitBlinky(60000);
		}
	}
}

void waitBlinky(uint32_t t) {
	uint32_t tNow = micros();
	while (true) {
		digitalWrite(ledLow, HIGH);
		delay(500);
		digitalWrite(ledHigh, LOW);
		delay(500);
		if (getDelta(tNow, micros()) > (t*1000)) {
			return;
		}
	}
}

inline bool readButtonLong() {
	readButton();
	if (conditionButtonUp == Active && conditionButtonDown == Inactive) { // bUp ditekan, bDown tak-ditekan
		delay(DELAY_BUTTON);
		uint32_t now = micros();
		while (true) {
			readButton();
			if (conditionButtonUp == Active) {
				if (getDelta(now, micros()) > DELAY_SETTING_BUTTON) {
					menu = SETTING_UPPER_LIMIT;
					//return true;
				}
			}
			else {
				if (getDelta(now, micros()) > DELAY_SETTING_BUTTON) {
					menu = SETTING_UPPER_LIMIT;
					return true;
				}
				return false;
			}
		}
	}
	else if (conditionButtonUp == Inactive && conditionButtonDown == Active) {
		delay(DELAY_BUTTON);
		uint32_t now = micros();
		while (true) {
			readButton();
			if (conditionButtonDown == Active) {
				if (getDelta(now, micros()) > DELAY_SETTING_BUTTON) {
					menu = SETTING_LOWER_LIMIT;
					//return true;
				}
			}
			else {
				if (getDelta(now, micros()) > DELAY_SETTING_BUTTON) {
					menu = SETTING_LOWER_LIMIT;
					return true;
				}
				return false;
			}
		}
	}
	else {
		return false;
	}
}

inline void readButton() {
	conditionButtonUp = digitalRead(buttonUp);
	conditionButtonDown = digitalRead(buttonDown);
}

void menuSetting() {
	bool whileCondition = true;
	switch (menu) {
	case SETTING_UPPER_LIMIT:
		lcd.clear();
		lcd.setCursor(2, 0);
		lcd.print("UPPER LIMIT");
		lcd.setCursor(0, 1);
		lcd.print(upperLimit);
		lcd.setCursor(4, 1);
		lcd.print("KPJ");
		while (whileCondition) {
			readButton();
			if (conditionButtonUp == Active && conditionButtonDown == Inactive) {
				delay(DELAY_BUTTON);
				uint32_t now = micros();
				bool whileCondition2 = true;
				bool pertamaDieksekusi = true;
				while (whileCondition2) {
					readButton();
					if (conditionButtonUp == Active) {
						delay(DELAY_BUTTON);
						readButton();
						if (conditionButtonUp == Active) {
							delay(DELAY_BUTTON);
							if (getDelta(now, micros()) > DELAY_SETTING_BUTTON && (!pertamaDieksekusi)) {
								while (true) {
									readButton();
									if (conditionButtonUp == Inactive) {
										whileCondition = false;
										whileCondition2 = false;
										return;
									}
								}
							}
						}
						else {
							delay(DELAY_BUTTON);
							upperLimit++;
							lcd.clear();
							lcd.setCursor(2, 0);
							lcd.print("UPPER LIMIT");
							lcd.setCursor(0, 1);
							lcd.print(upperLimit);
							lcd.setCursor(4, 1);
							lcd.print("KPJ");
							whileCondition2 = false;
							
							now = micros();

						}
					}
					pertamaDieksekusi = false;
				}
			}
			else if (conditionButtonUp == Inactive && conditionButtonDown == Active) {
				delay(DELAY_BUTTON);
				uint32_t now = micros();
				bool whileCondition2 = true;
				bool pertamaDieksekusi = true;
				while (whileCondition2) {
					readButton();
					if (conditionButtonDown == Active) {
						delay(DELAY_BUTTON);
						readButton();
						if (conditionButtonUp == Active) {
							delay(DELAY_BUTTON);
							if (getDelta(now, micros()) > DELAY_SETTING_BUTTON && (!pertamaDieksekusi)) {
								while (true) {
									readButton();
									if (conditionButtonDown == Inactive) {
										whileCondition = false;
										whileCondition2 = false;
										return;
									}
								}
							}
						}
						else {
							delay(DELAY_BUTTON);
							upperLimit--;
							lcd.clear();
							lcd.setCursor(2, 0);
							lcd.print("UPPER LIMIT");
							lcd.setCursor(0, 1);
							lcd.print(upperLimit);
							lcd.setCursor(4, 1);
							lcd.print("KPJ");
							whileCondition2 = false;
							now = micros();
						}
					}
					pertamaDieksekusi = false;
				}
			}
		}
		break;
	case SETTING_LOWER_LIMIT:
		lcd.clear();
		lcd.setCursor(2, 0);
		lcd.print("LOWER LIMIT");
		lcd.setCursor(0, 1);
		lcd.print(lowerLimit);
		lcd.setCursor(4, 1);
		lcd.print("KPJ");
		while (whileCondition) {
			readButton();
			if (conditionButtonUp == Active && conditionButtonDown == Inactive) {
				delay(DELAY_BUTTON);
				uint32_t now = micros();
				bool whileCondition2 = true;
				bool pertamaDieksekusi = true;
				while (whileCondition2) {
					readButton();
					if (conditionButtonUp == Active) {
						delay(DELAY_BUTTON);
						readButton();
						if (conditionButtonUp == Active) {
							delay(DELAY_BUTTON);
							if (getDelta(now, micros()) > DELAY_SETTING_BUTTON && (!pertamaDieksekusi)) {
								while (true) {
									readButton();
									if (conditionButtonUp == Inactive) {
										whileCondition = false;
										whileCondition2 = false;
										return;
									}
								}
							}
						}
						else {
							delay(DELAY_BUTTON);
							lowerLimit++;
							lcd.clear();
							lcd.setCursor(2, 0);
							lcd.print("LOWER LIMIT");
							lcd.setCursor(0, 1);
							lcd.print(lowerLimit);
							lcd.setCursor(4, 1);
							lcd.print("KPJ");
							whileCondition2 = false;
							now = micros();
						}	
					}
					pertamaDieksekusi = false;
				}
			}
			else if (conditionButtonUp == Inactive && conditionButtonDown == Active) {
				delay(DELAY_BUTTON);
				uint32_t now = micros();
				bool whileCondition2 = true;
				bool pertamaDieksekusi = true;
				while (whileCondition2) {
					readButton();
					if (conditionButtonDown == Active) {
						delay(DELAY_BUTTON);
						readButton();
						if (conditionButtonUp == Active) {
							delay(DELAY_BUTTON);
							if (getDelta(now, micros()) > DELAY_SETTING_BUTTON && (!pertamaDieksekusi)) {
								while (true) {
									readButton();
									if (conditionButtonDown == Inactive) {
										whileCondition = false;
										whileCondition2 = false;
										return;
									}
								}
							}
						}
						else {
							delay(DELAY_BUTTON);
							lowerLimit--;
							lcd.clear();
							lcd.setCursor(2, 0);
							lcd.print("LOWER LIMIT");
							lcd.setCursor(0, 1);
							lcd.print(lowerLimit);
							lcd.setCursor(4, 1);
							lcd.print("KPJ");
							whileCondition2 = false;
							now = micros();
						}			
					}
					pertamaDieksekusi = false;
				}
			}
		}
		break;
	default:
		break;
	}
}

// Fungsi untuk mendapatkan nilai selisih counter dengan skala micros.
uint32_t getDelta(uint32_t before, uint32_t now) {
	uint32_t returnValue = 0;
	if (before < now) {
		returnValue = before - now;
		return returnValue;
	}
	else {
		uint32_t x = ((2 ^ 32) - 1) - before;
		returnValue = now + x;
		return returnValue;
	}
}

inline void upperLimitAction() {
	digitalWrite(relayHigh, Active);
	digitalWrite(relayLow, Inactive);
	lowerLimitCondition = false;
	upperLimitCondition = true;
}

inline void lowerLimitAction() {
	digitalWrite(relayHigh, Inactive);
	digitalWrite(relayLow, Active);
	lowerLimitCondition = true;
	upperLimitCondition = false;
}

inline void middleAction() {
	digitalWrite(relayHigh, Inactive);
	digitalWrite(relayLow, Inactive);
	lowerLimitCondition = false;
	upperLimitCondition = false;
}

inline void noneAction() {
	digitalWrite(relayHigh, Inactive);
	digitalWrite(relayLow, Inactive);
	lowerLimitCondition = false;
	upperLimitCondition = false;
}

void displayStandby() {
	if (standbyCondition) {
		if (getDelta(counterWindData, micros()) > updateInterval) {
			standbyCondition = false;
		}
	}
	else {
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("Wind Speed: ");
		lcd.setCursor(13, 0);
		lcd.print(windVelocity);
		lcd.setCursor(0, 1);
		lcd.print("L: ");
		lcd.setCursor(3, 1);
		lcd.print(lowerLimit);
		lcd.setCursor(8, 1);
		lcd.print("U: ");
		lcd.setCursor(11, 1);
		lcd.print(upperLimit);
		counterWindData = micros();
		standbyCondition = true;
	}
	Serial.println("Display Standby");
}

void blinky(bool enBlinky) {
	if (enBlinky) {
		if (microsBlinky == 0) {
			microsBlinky = micros();
		}
		else if (getDelta(microsBlinky, micros()) > DELAY_BLINKY) {
			digitalWrite(ledHigh, ledCondition);
			digitalWrite(ledLow, !ledCondition);
			ledCondition = !ledCondition;
			microsBlinky = micros();
			Serial.println("Blinky");
		}
	}
	else {
		digitalWrite(ledHigh, LOW);
		digitalWrite(ledLow, LOW);
	}
}

void setCondition(CONDITION c) {
	condition = c;
}

CONDITION getCondition() {
	return condition;
}

bool isCondition(CONDITION c) {
	if (condition == c) {
		return true;
	}
	else {
		return false;
	}
}

void setConditionSebelumnya(CONDITION c) {
	conditionSebelumnya = c;
}

CONDITION getConditionSebelumnya() {
	return conditionSebelumnya;
}

bool isConditionSebelumnya(CONDITION c) {
	if (conditionSebelumnya == c) {
		return true;
	}
	else {
		return false;
	}
}

void enableAction(bool en, CONDITION cond = noneCondition) {
	if (en) {
		if (actionCondition) {
			if (firstExecOfCondition) {
				switch (cond) {
				case noneCondition:
					noneAction();
					lowerLimitCondition = false;
					upperLimitCondition = false;
					break;
				case lowerCondition:
					lowerLimitAction();
					lowerLimitCondition = true;
					break;
				case middleCondition:
					middleAction();
					lowerLimitCondition = false;
					upperLimitCondition = false;
					break;
				case upperCondition:
					upperLimitAction();
					upperLimitCondition = true;
					break;
				default:
					break;
				}
				firstExecOfCondition = false;
				delay(10000);
				return;
			}
			if ((getDelta(counterTimeUpperLower, micros()) > DELAY_RELAY_ACTIVE)) {
				switch (cond) {
				case noneCondition:
					noneAction();
					aktifkanBlinky = false;
					actionCondition = false;
					lowerLimitCondition = false;
					upperLimitCondition = false;
					break;
				case lowerCondition:
					Serial.println("Lower Condition");
					lowerLimitAction();
					aktifkanBlinky = true;
					lowerLimitCondition = true;
					break;
				case middleCondition:
					Serial.println("Middle Condition");
					middleAction();
					aktifkanBlinky = false;
					actionCondition = false;
					lowerLimitCondition = false;
					upperLimitCondition = false;
					break;
				case upperCondition:
					Serial.println("Upper Condition");
					upperLimitAction();
					aktifkanBlinky = true;
					upperLimitCondition = true;
					break;
				default:
					break;
				}
			}
			else {
				if (lowerLimitCondition || upperLimitCondition) {
					aktifkanBlinky = true;
					return;
				}
				switch (cond) {
				case noneCondition:
					aktifkanBlinky = false;
					break;
				case lowerCondition:
					aktifkanBlinky = true;
					break;
				case middleCondition:
					aktifkanBlinky = false;
					break;
				case upperCondition:
					aktifkanBlinky = true;
					break;
				default:
					break;
				}
				return;
			}
		}
		else {
			aktifkanBlinky = false;
			actionCondition = true;
			counterTimeUpperLower = micros();
		}
		
	}
	else {
		noneAction();
	}
}
