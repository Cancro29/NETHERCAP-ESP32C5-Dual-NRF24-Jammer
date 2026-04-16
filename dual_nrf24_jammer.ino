#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h> // REQUIRED for printPrettyDetails()

// --- Pin Definitions (Mapped to raw GPIO for Firebeetle 2 ESP32-C5) ---
#define CS1   27 // D6
#define CE1   28 // D9
#define CS2   26 // D3
#define CE2   8  // D2

#define PUSH  2  // A1
#define WAKE  6  // D12
#define UP    3  // A2
#define DOWN  5  // A4
#define RIGHT 4  // A3
#define LEFT  7  // D11

#define VBAT  1
#define Power 0

// Explicit SPI pins from the C5 datasheet/schematic
#define SPI_SCK  23
#define SPI_MOSI 24
#define SPI_MISO 25

/*
NOTE : On DFRobot FireBeetle 2 ESP32C5, it is MANDATORY to do hardware reset by pressing RST button after flashing
If not, NRF24 will fail to initialize!!!
I think 2 NRF24 is the minimum for proper jamming across 2,4 GHz frequency range
For maximum power, use NRF24 that has Power Amplifier (PA) and Low Noise Amplifier (LNA), EBYTE E01 have this
This code is designed for ESP32C5, but will work with other ESP32 as long it has enough free GPIO pins and you have knowledge to assign the pins
*/

// --- Object Instantiation ---
RF24 radio1(CE1, CS1);
RF24 radio2(CE2, CS2);


// Channel array for sweeping
const uint8_t channels[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
const uint8_t numChannels = sizeof(channels) / sizeof(channels[0]);

// Performance tracking variables
unsigned long previousMillis = 0;
unsigned long packetsSent = 0;

uint8_t channelIndex1 = 0;
uint8_t channelIndex2 = 0;

void configureRadio_CCW(RF24 &radio) {
  // Set NRF24 to Constant Carrier Wave mode
  radio.setAutoAck(false);
  radio.stopListening();
  radio.setRetries(0, 0);
  radio.setPALevel(RF24_PA_MAX, true);
  radio.setDataRate(RF24_250KBPS);
  radio.setCRCLength(RF24_CRC_DISABLED);
  radio.printPrettyDetails();
  radio.startConstCarrier(RF24_PA_MAX, 45);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  
  printf_begin(); // Initialize the printf formatting for the RF24 library

  Serial.println("\n--- NRF24L01 Dual Diagnostic Boot ---");

  // Power Up peripheral hardware
  pinMode(Power, OUTPUT);
  digitalWrite(Power, HIGH); 
  
  // Prevent Contention by pulling both CS pins HIGH (Inactive) before starting SPI
  pinMode(CS1, OUTPUT);
  digitalWrite(CS1, HIGH); 
  pinMode(CS2, OUTPUT);
  digitalWrite(CS2, HIGH); 

  delay(1000); 

  // Init SPI with the raw GPIO pins
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  // --- Test Radio 1 ---
  Serial.print("Initializing Radio 1... ");
  bool r1_success = radio1.begin();
  Serial.println(r1_success ? "SUCCESS" : "FAILED");
  
  Serial.println("\n--- Radio 1 Register Dump ---");
  radio1.printPrettyDetails();

  // --- Test Radio 2 ---
  Serial.print("\nInitializing Radio 2... ");
  bool r2_success = radio2.begin();
  Serial.println(r2_success ? "SUCCESS" : "FAILED");
  
  Serial.println("\n--- Radio 2 Register Dump ---");
  radio2.printPrettyDetails();

  Serial.println("\nIf Check Serial Monitor, if Pipe0 or Pipe1 has unique address like 0xe7e7e7e7e7e7, then we can proceed");
  Serial.println("\nProceeding");
  delay(5000); 

  // Configure both radios for maximum power and throughput
  digitalWrite(CS2, HIGH); // Switch OFF NRF2
  configureRadio_CCW(radio1);
  digitalWrite(CS1, HIGH); // Switch OFF NRF1
  configureRadio_CCW(radio2);

  Serial.println("\n--- INITIATING INTERFERENCE TEST ---");
  Serial.println("WARNING: Modules are active for longer durations.");
}

void loop() {
  // NOTE : NRF24 NEEDS 130 microseconds delay to finish Phase-Locked Loop (PLL) lock.
  // Meaning changing channels too fast or too slow will reduce its effectiveness
  // Serial prints below provide just enough delay for it to do interference
  // Enable USB CDC On Boot if possible
  // I haven't found the ideal delay for it. You can experiment by yourself
  //////////////////////////////////////////////////////  13042026 : This is the most effective jamming
  // Change channels for both radios
  digitalWrite(CS2, HIGH); // Switch OFF NRF2
  radio1.setChannel(channels[channelIndex1]);
  digitalWrite(CS1, HIGH); // Switch OFF NRF1
  radio2.setChannel(channels[channelIndex2]);
  // Randomize channel, split both NRF24 across 2,4 GHz channels
  channelIndex1 = random(1,63);
  channelIndex2 = random(64,125);
  Serial.println("CH 1 : " + String(channelIndex1));
  Serial.println("CH 2 : " + String(channelIndex2));
  //////////////////////////////////////////////////////  13042026 : This is the most effective jamming
}
