#include <TinyGPS++.h>	// http://arduiniana.org/libraries/tinygpsplus/
#include <SPI.h>
#include <Ethernet.h>

const uint16_t INITIAL_VALUE = 250;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 1, 177 }; //IP Ethernet
byte serv[] = { 192, 168, 1, 179 }; //IPv4 Address
const uint16_t port = 80;

TinyGPSPlus gps;
EthernetClient client;

const uint8_t analogInput = A0;
int nilai;
static const uint16_t UKURAN_BUFFER = 5000;  //5000 maksimal by eksperimen
uint16_t dataBuffer[UKURAN_BUFFER];  // Data yang digunakan untuk menyimpan nilai ADC
uint32_t usAwal = 0, usAkhir = 0;
uint32_t microsAwal = 0, microsAkhir = 0;
uint32_t selisihMicros = 0;
uint32_t averageMicros = 0;

bool firstExecution = true;
uint64_t msSekarang = 0, msSebelumnya = 0, selisihMS = 0;
uint8_t sSebelumnya = 0, selisihDetik = 0;
bool penandaPertamaKaliDieksekusi = false, detikMasihSama = false;

//----------------Waktu Tersinkron dengan Puncak Gelombang---------------//
char Waktu[] = "00:00:00:000000"; //Jam:Menit:Detik:MikroDetik
char Tanggal[] = "00/00/2000";  //Tanggal/Bulan/Tahun

struct TanggalWaktu {
	uint8_t day = 1;
	uint8_t month = 1;
	uint16_t year = 2019;
	uint8_t h = 0;
	uint8_t m = 0;
	uint8_t s = 0;
	uint32_t us = 0;
	uint32_t micros = 0;
};

TanggalWaktu tAwal, tAkhir, tSync, tAwalSebelumnya;

//uint32_t microDetikPuncak[UKURAN_BUFFER];
uint64_t microDetikPuncak[UKURAN_BUFFER];
uint8_t detikPuncak[UKURAN_BUFFER];
uint8_t menitPuncak[UKURAN_BUFFER];
uint8_t jamPuncak[UKURAN_BUFFER];
uint8_t tanggalPuncak[UKURAN_BUFFER];
uint8_t bulanPuncak[UKURAN_BUFFER];
uint16_t tahunPuncak[UKURAN_BUFFER];

bool indeksNilaiPuncak[UKURAN_BUFFER] = { false };

const unsigned long GPS_BAUDRATE = 9600;
const unsigned long SM_BAUDRATE = 115200;

void setup() {
	pinMode(analogInput, INPUT);
	pinMode(LED_BUILTIN, OUTPUT);
	Ethernet.begin(mac, ip);
	// Setup all registers
	pmc_enable_periph_clk(ID_ADC);
	adc_init(ADC, SystemCoreClock, ADC_FREQ_MIN, ADC_STARTUP_FAST);
	adc_disable_interrupt(ADC, 0xFFFFFFFF);
	adc_set_resolution(ADC, ADC_10_BITS);
	adc_configure_power_save(ADC, ADC_MR_SLEEP_NORMAL, ADC_MR_FWUP_OFF);
	adc_configure_timing(ADC, 1, ADC_SETTLING_TIME_3, 1);
	adc_set_bias_current(ADC, 1);
	adc_disable_tag(ADC);
	adc_disable_ts(ADC);
	adc_stop_sequencer(ADC);
	adc_disable_channel_differential_input(ADC, ADC_CHANNEL_7);
	adc_disable_all_channel(ADC);
	adc_enable_channel(ADC, ADC_CHANNEL_7);
	adc_configure_trigger(ADC, ADC_TRIG_SW, 1);
	Serial3.begin(GPS_BAUDRATE);
	adc_start(ADC);
	Serial.begin(SM_BAUDRATE);



	//Sinkronisasi MicroSecond
	bool flag1 = true;
	bool flag2 = true;
	bool flag3 = true;
	bool kondisi = true;
	uint8_t sPrevious = 0;
	while (kondisi) {
		while (flag1 || flag2 || flag3) {
			if (Serial3.available()) {
				if (gps.encode(Serial3.read())) {
					if (gps.time.isValid()) {
						tSync.h = syncHour((gps.time.hour() + 7));
						tSync.m = gps.time.minute();
						tSync.s = gps.time.second();
						if (flag1) {
							sPrevious = tSync.s;
							flag1 = false;
							Serial.println("sPrevious Filled, flag1 is false");
						}
					}
					if (gps.date.isValid()) {
						tSync.day = gps.date.day();
						tSync.month = gps.date.month();
						tSync.year = gps.date.year();
						flag2 = false;
						Serial.println("flag2 is false");
					}
					if (sPrevious != tSync.s) {
						tSync.micros = micros();
						tSync.us = 0;
						flag3 = false;
						kondisi = false;
						Serial.println("flag3 is false, kondisi is false");
					}
				}
			}
		}
	}
	Serial.println("setup() finished");
}

void loop() {
	/*---------------------------------------------------*/
	/*-----------configure Peripheral DMA----------------*/
	/*---------------------------------------------------*/
	//PDC_ADC->PERIPH_RPR = (uint32_t)dataBuffer; // address of buffer
	PDC_ADC->PERIPH_RPR = (uint32_t)dataBuffer; // address of buffer
			   /*
			   Harus dikonversi menjadi 32 bit karena
			   ADC akan menuliskan data ke alamat buffer
			   dengan format 32 bit.
			   */
	PDC_ADC->PERIPH_RCR = UKURAN_BUFFER; /*
			   Ini adalah settingan untuk ukuran buffer.
			   Ukuran buffer ada pada datasheet bagian 26.2.5.
			   Maksimal ((2^16)-1)
			   */
	PDC_ADC->PERIPH_PTCR = PERIPH_PTCR_RXTEN; // enable receive

	setWaktu(tAwal);
	microsAwal = micros();
	/*-------Menunggu sampai proses ADC sebesar ukuran buffer selesai-----*/
	/*---------------Artinya mengunggu sampai buffer penuh----------------*/
	while ((adc_get_status(ADC) & ADC_ISR_ENDRX) == 0) {};
	microsAkhir = micros();
	setWaktu(tAkhir);

	tAwal.micros = microsAwal;
	tAkhir.micros = microsAkhir;
	selisihMicros = getSelisihMicros(microsAwal, microsAkhir);
	if (firstExecution) {
		if (isSame(tAwal, tSync)) {
			tAwal.us = tSync.us + (getSelisihMicros(tSync.micros, tAwal.micros) % 1000000);
		}
		else {
			uint32_t tm = getSelisihMicros(tSync.micros, tAwal.micros) % 1000000;
			uint32_t to = 1000000 - tAwalSebelumnya.us;
			tAwal.us = tm - to;
		}
		firstExecution = false;
	}
	else if (isSame(tAwal, tAwalSebelumnya)) {  //Jika masih dalam detik yang sama
		tAwal.us = tAwalSebelumnya.us + (getSelisihMicros(tAwalSebelumnya.micros, tAwal.micros) % 1000000);
	}
	else {
		uint32_t tm = getSelisihMicros(tAwalSebelumnya.micros, tAwal.micros) % 2000000;
		uint32_t to = 1000000 - tAwalSebelumnya.us;
		tAwal.us = tm - to;
	}

	copyP2ToP1(tAwalSebelumnya, tAwal);
	averageMicros = selisihMicros / UKURAN_BUFFER;
	tAkhir.us = (tAwal.us + selisihMicros) % 1000000;

	cekPuncak(dataBuffer, UKURAN_BUFFER);
	kirimData();  //mengirim data ke server dan menampilkan data di serial monitor
	setDefaultIndeksNilaiPuncak();

}

inline void setDefaultIndeksNilaiPuncak() {
	for (uint16_t i = 0; i < UKURAN_BUFFER; i++) {
		indeksNilaiPuncak[i] = false;
		microDetikPuncak[i] = 0;
	}
}

inline uint32_t getSelisihMicros(uint32_t& awal, uint32_t& akhir) {
	uint32_t returnValue = 0;
	if (akhir > awal) {
		returnValue = akhir - awal;
		return returnValue;
	}
	else {
		uint32_t x = ((2 ^ 32) - 1) - awal;
		returnValue = akhir + x;
		return returnValue;
	}
}

inline void setWaktu(TanggalWaktu& t) {
	bool flag1 = true;
	bool flag2 = true;
	while (flag1 || flag2) {
		if (Serial3.available()) {
			if (gps.encode(Serial3.read())) {
				if (gps.time.isValid()) {
					t.h = syncHour((gps.time.hour() + 7));
					t.m = gps.time.minute();
					t.s = gps.time.second();
					flag1 = false;
				}
				if (gps.date.isValid()) {
					t.day = gps.date.day();
					t.month = gps.date.month();
					t.year = gps.date.year();
					flag2 = false;
				}
				if (t.s != gps.time.second()) {
					t.us = 0;
				}
			}
		}
	}
}

inline void copyP2ToP1(TanggalWaktu& P1, const TanggalWaktu& P2) {
	P1.year = P2.year;
	P1.month = P2.month;
	P1.day = P2.day;
	P1.h = P2.h;
	P1.m = P2.m;
	P1.s = P2.s;
	P1.us = P2.us;
	P1.micros = P2.micros;
}

inline bool isSame(const TanggalWaktu& P1, const TanggalWaktu& P2) {
	if (P1.year == P2.year) {
		if (P1.month == P2.month) {
			if (P1.day == P2.day) {
				if (P1.h == P2.h) {
					if (P1.m == P2.m) {
						if (P1.s == P2.s) {
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

inline void kirimData() {
	uint16_t ds = 1;
	for (uint16_t i = 0; i < UKURAN_BUFFER; i++) {
		if (indeksNilaiPuncak[i]) {
			Waktu[0] = (jamPuncak[i] / 10) + 48;
			Waktu[1] = (jamPuncak[i] % 10) + 48;
			Waktu[3] = (menitPuncak[i] / 10) + 48;
			Waktu[4] = (menitPuncak[i] % 10) + 48;
			Waktu[6] = (detikPuncak[i] / 10) + 48;
			Waktu[7] = (detikPuncak[i] % 10) + 48;
			Waktu[9] = (microDetikPuncak[i] / 100000) + 48;
			Waktu[9] >= 48 && Waktu[9] <= 57 ? Waktu[9] += 0 : Waktu[9] = 48;
			Waktu[10] = ((microDetikPuncak[i] / 10000) % 10) + 48;
			Waktu[11] = ((microDetikPuncak[i] / 1000) % 10) + 48;
			Waktu[12] = ((microDetikPuncak[i] / 100) % 10) + 48;
			Waktu[13] = ((microDetikPuncak[i] / 10) % 10) + 48;
			Waktu[14] = (microDetikPuncak[i] % 10) + 48;

			Tanggal[0] = (tanggalPuncak[i] / 10) + 48;
			Tanggal[1] = (tanggalPuncak[i] % 10) + 48;
			Tanggal[3] = (bulanPuncak[i] / 10) + 48;
			Tanggal[4] = (bulanPuncak[i] % 10) + 48;
			Tanggal[8] = ((tahunPuncak[i] / 10) % 10) + 48;
			Tanggal[9] = (tahunPuncak[i] % 10) + 48;

			//client.setConnectionTimeout(50);
			if (client.connect(serv, port)) {
				Serial.print("Kirim data ke-");
				Serial.print(i + 1);
				Serial.print(" ke server.\t");
				Serial.print("Data server ke-");
				Serial.println(ds);
				ds++;

				client.print("GET /bacaarus/datates.php?");
				client.print("input1=");
				client.print(Waktu);
				client.print("&input2=");
				client.println(Tanggal);

				client.stop();
			}
			Serial.print(i + 1);
			Serial.print('\t');
			Serial.println(Tanggal);
			Serial.print('\t');
			Serial.println(Waktu);
			Serial.println();
		}
	}
	//client.stop();
}

//--------------------------------------------------//
/*
  Fungsi ini digunakan untuk memeriksa data hasil
  pembacaan ADC, apakah ada sinyal puncak.
  Setelah pembacaan, dilakukan perekaman data waktu di setiap sinyal puncak.
  Setiap data waktu langsung disimpan pada variabel yang mengandung kata puncak.
  Parameter pertama adalah array data nilai ADC yang terekam.
  Parameter kedua adalah ukuran array.
*/
//--------------------------------------------------//
inline void cekPuncak(uint16_t data[], uint16_t ukuran) {
	uint16_t nilaiLebihTinggi = 0;    //digunakan untuk mencari nilai tertinggi
	uint16_t blockElseDieksekusi = 0; //digunakan untuk menandai puncak baru saja dilewati


	for (uint16_t i = 1; i < ukuran - 1; i++) {
		if ((data[i - 1] > INITIAL_VALUE) && (data[i] > INITIAL_VALUE)) {
			if (data[i - 1] < data[i]) {
				nilaiLebihTinggi = data[i]; //menjejak nilai sampai nilai tertinggi
				blockElseDieksekusi = 0;
				data[i - 1] = 0;  //set default data ke 0
			}
			else {
				blockElseDieksekusi++;
				if (blockElseDieksekusi == 1) {
					indeksNilaiPuncak[i - 1] = true;
				}
				nilaiLebihTinggi = data[i - 1];
			}
		}
	}

	//---------------------------------------------------------//
	/*Perulangan untuk merekam waktu pada saat ada nilai Puncak*/
	//---------------------------------------------------------//
	if (isSame(tAwal, tAkhir)) {
		for (uint16_t i = 0; i < ukuran; i++) {
			if (indeksNilaiPuncak[i]) {
				microDetikPuncak[i] = ((averageMicros * (i + 1)) + tAwal.us) % 1000000;
				detikPuncak[i] = tAwal.s;
				menitPuncak[i] = tAwal.m;
				jamPuncak[i] = tAwal.h;
				tanggalPuncak[i] = tAwal.day;
				bulanPuncak[i] = tAwal.month;
				tahunPuncak[i] = tAwal.year;
			}
		}
	}
	else {
		for (uint16_t i = 0; i < ukuran; i++) {
			if (indeksNilaiPuncak[i]) {
				if ((averageMicros*(i + 1) + tAwal.us) > 999999) {
					do {
						microDetikPuncak[i] = (averageMicros * (i + 1)) - (tAwal.us - 1000000);
						detikPuncak[i] = tAkhir.s;
						menitPuncak[i] = tAkhir.m;
						jamPuncak[i] = tAkhir.h;
						tanggalPuncak[i] = tAkhir.day;
						bulanPuncak[i] = tAkhir.month;
						tahunPuncak[i] = tAkhir.year;
						i++;
					} while (i < ukuran);
				}
			}
		}
	}
}

inline uint8_t syncHour(uint8_t h) {
	switch (h) {
	case 24:
		return 0;
	case 25:
		return 1;
	case 26:
		return 2;
	case 27:
		return 3;
	case 28:
		return 4;
	case 29:
		return 5;
	case 30:
		return 6;
	default:
		return h;
	}
}
