#include <SPI.h>
#include <Ethernet.h>

const uint16_t INITIAL_VALUE = 100;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192, 168, 1, 18 }; //IP Ethernet
byte serv[] = { 192, 168, 100, 163 }; //IPv4 Address
const uint16_t port = 8080;

EthernetClient client;

const uint8_t analogInput = A0;
int nilai;

const uint16_t UKURAN_BUFFER = 2000;
uint16_t dataBuffer[UKURAN_BUFFER];  // Data yang digunakan untuk menyimpan nilai ADC
unsigned long waktuAwal = 0;
unsigned long waktuAkhir = 0;
unsigned long selisihWaktu = 0;
unsigned long waktuAverage = 0;

unsigned long microDetikPuncak[UKURAN_BUFFER];

bool indeksNilaiPuncak[UKURAN_BUFFER] = { false };

void setup() {
  pinMode(analogInput, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Ethernet.begin(mac, ip);
  // Setup all registers
  {
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
    Serial3.begin(115200);
    adc_start(ADC);
  }

}

void loop() {
  /*---------------------------------------------------*/
  /*-----------configure Peripheral DMA----------------*/
  /*---------------------------------------------------*/
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

  waktuAwal = micros();
  /*-------Menunggu sampai proses ADC sebesar ukuran buffer selesai-----*/
  /*---------------Artinya mengunggu sampai buffer penuh----------------*/
  while ((adc_get_status(ADC) & ADC_ISR_ENDRX) == 0) {};
  waktuAkhir = micros();

  selisihWaktu = getSelisihWaktu(waktuAwal, waktuAkhir);
  waktuAverage = selisihWaktu / UKURAN_BUFFER;
  cekPuncak(dataBuffer, UKURAN_BUFFER);
  kirimData();
}

unsigned long getSelisihWaktu(unsigned long& awal, unsigned long& akhir) {
  if (akhir > awal) {
    return (akhir - awal);
  }
  else {
    unsigned long x = ((2 ^ 32) - 1) - awal;
    return (akhir + x);
  }
}

void kirimData() {
  if (client.connect(serv, port)) {
    client.print("GET /BacaArusTanpaGPS/data.php?");
    for (uint16_t i = 0; i < UKURAN_BUFFER; i++) {
      if (indeksNilaiPuncak[i]) {
        client.print(microDetikPuncak[i]);
      }
    }
    client.stop();
  }
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
void cekPuncak(uint16_t data[], uint16_t ukuran) {
  uint16_t nilaiLebihTinggi = 0;    //digunakan untuk mencari nilai tertinggi
  uint16_t blockElseDieksekusi = 0; //digunakan untuk menandai puncak baru saja dilewati

  for (uint16_t i = 1; i < ukuran - 1; i++) {
    if ((data[i-1] > INITIAL_VALUE) && (data[i] > INITIAL_VALUE)) {
      if (data[i - 1] < data[i]) {
        nilaiLebihTinggi = data[i]; //menjejak nilai sampai nilai tertinggi
        blockElseDieksekusi = 0;
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
  for (uint16_t i = 0; i < ukuran; i++) {
    if (indeksNilaiPuncak[i]) {
      microDetikPuncak[i] = waktuAverage * (i + 1);
    }
  }
}
