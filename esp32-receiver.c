// ########
// #       #
// #       #
// ########
// #    #
// #     #
// #      #

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Define pins for the nRF24L01
#define CE_PIN 4
#define CSN_PIN 5

// Create RF24 object
RF24 radio(CE_PIN, CSN_PIN);  // CE, CSN

// Define address (matching the sender address)
const byte address[6] = "00001";

// Data structure to receive
struct DataPacket {
  int8_t joystickX;
  int8_t joystickY;
  uint8_t pot1;
  uint8_t pot2;
  bool joystickButton;
  bool onOffKey;
};

void setup() {
  Serial.begin(115200);

  // Initialize radio
  Serial.println("Initializing radio...");
  while (!radio.begin()) {
    Serial.println("Radio init failed. Retrying...");
    delay(1000);  // Wait 1 second before retrying
  }
  Serial.println("Radio initialized successfully!");

  // Open reading pipe to receive data
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_HIGH);  // Set higher power
  radio.setDataRate(RF24_2MBPS);   // Faster data rate
  radio.startListening();          // Start listening for data
}

void loop() {
  // Check if data is available to read
  if (radio.available()) {
    DataPacket data;

    // Read the data into the struct
    radio.read(&data, sizeof(data));

    // Display the received data
    Serial.print("X: ");
    Serial.print(data.joystickX);
    Serial.print(" Y: ");
    Serial.print(data.joystickY);
    Serial.print(" P1: ");
    Serial.print(data.pot1);
    Serial.print(" P2: ");
    Serial.print(data.pot2);
    Serial.print(" B: ");
    Serial.print(data.joystickButton);
    Serial.print(" Key: ");
    Serial.println(data.onOffKey);
  }
}

