// ###########
//     #
//     #
//     #
//     #
//     #
//     #

#include <LiquidCrystal.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Define pins
#define JOYSTICK_X A0
#define JOYSTICK_Y A1
#define JOYSTICK_BUTTON 2
#define POT1 A3
#define POT2 A2
#define ON_OFF_KEY 3
#define LED_PIN 7

// Initialize the LCD
LiquidCrystal lcd(8, 6, 5, 4, A4, A5);
static unsigned long lastLCDUpdate = 0;
const unsigned long lcdUpdateInterval = 200;

// Create RF24 object
RF24 radio(9, 10);  // CE, CSN

// Define address
const byte address[6] = "00001";

// Data structure to send
struct DataPacket {
  int8_t joystickX;
  int8_t joystickY;
  uint8_t pot1;
  uint8_t pot2;
  bool joystickButton;
  bool onOffKey;
};

// Variables for smoothing
int lastJoystickX = 0;
int lastJoystickY = 0;
int lastPot1 = 0;
int lastPot2 = 0;
bool lastJoystickButton;
bool lastOnOffKey;

// Timing variables
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 50;  // Send every 50ms

// Retry variables
const int maxRetries = 3;  // Maximum number of retries for sending

// Sleep mode variables
bool inSleepMode = true;
int unsuccessfulSendCount = 0;  // Track the number of unsuccessful sends

void setup() {
  Serial.begin(9600);

  // Initialize pins
  pinMode(JOYSTICK_BUTTON, INPUT_PULLUP);
  pinMode(ON_OFF_KEY, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  // Initialize radio with retries
  Serial.println("Initializing radio...");
  while (!radio.begin()) {
    Serial.println("Radio init failed. Retrying...");
    delay(1000);  // Wait 1 second before retrying
  }
  Serial.println("Radio initialized successfully!");

  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_HIGH);  // Higher power
  radio.setDataRate(RF24_2MBPS);   // Faster data rate
  radio.stopListening();

  lcd.begin(16, 2);
}

void loop() {
  if (inSleepMode) {
    // Sleep mode logic
    digitalWrite(LED_PIN, (millis() / 500) % 2);  // Blink LED every 500ms
    delay(500);                                   // Sleep for 500ms
  }

  // Read inputs
  int newJoystickX = map(analogRead(JOYSTICK_X), 0, 1023, -100, 100);
  int newJoystickY = map(analogRead(JOYSTICK_Y), 0, 1023, -100, 100);
  int newPot1 = map(analogRead(POT1), 0, 1023, 0, 100);
  int newPot2 = map(analogRead(POT2), 0, 1023, 0, 100);
  bool newJoystickButton = !digitalRead(JOYSTICK_BUTTON);
  bool newOnOffKey = !digitalRead(ON_OFF_KEY);

  // Simple smoothing
  lastJoystickX = (lastJoystickX + newJoystickX) / 2;
  lastJoystickY = (lastJoystickY + newJoystickY) / 2;
  lastPot1 = (lastPot1 + newPot1) / 2;
  lastPot2 = (lastPot2 + newPot2) / 2;

  // Check if values have changed significantly
  bool valuesChanged = (abs(lastJoystickX - newJoystickX) > 1) || (abs(lastJoystickY - newJoystickY) > 1) || (abs(lastPot1 - newPot1) > 1) || (abs(lastPot2 - newPot2) > 1) || (lastJoystickButton != newJoystickButton) || (lastOnOffKey != newOnOffKey);

  lastJoystickButton = newJoystickButton;
  lastOnOffKey = newOnOffKey;

  unsigned long currentTime = millis();
  if (valuesChanged || (currentTime - lastSendTime >= sendInterval)) {
    // Prepare data packet
    DataPacket data;
    data.joystickX = lastJoystickX;
    data.joystickY = lastJoystickY;
    data.pot1 = lastPot1;
    data.pot2 = lastPot2;
    data.joystickButton = newJoystickButton;
    data.onOffKey = newOnOffKey;

    // Timestamp before sending data
    unsigned long sendStartTime = millis();

    // Send data with retries
    bool success = false;
    for (int i = 0; i < maxRetries; i++) {
      success = radio.write(&data, sizeof(data));
      if (success) {
        break;  // Exit retry loop if successful
      }
    }

    // Timestamp after sending data
    unsigned long sendEndTime = millis();

    unsigned long sendTime = sendEndTime - sendStartTime;

    // Update LED and last send time
    digitalWrite(LED_PIN, success ? HIGH : LOW);  // Indicate success/failure
    lastSendTime = currentTime;

    // Debugging output
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
    updateLCD(data, sendTime);

    // If sending is successful, reset unsuccessful count and exit sleep mode
    if (success) {
      inSleepMode = false;
      unsuccessfulSendCount = 0;  // Reset unsuccessful send count
    } else {
      unsuccessfulSendCount++;
      // After 10 unsuccessful attempts, enter sleep mode
      if (unsuccessfulSendCount >= 10) {
        inSleepMode = true;
        unsuccessfulSendCount = 0;  // Reset count after entering sleep mode
        Serial.println("Switching to sleep mode");
      }
    }
  }
}

void updateLCD(DataPacket data, long sendTime) {
  if (millis() - lastLCDUpdate >= lcdUpdateInterval) {

    // Move back to the start of the first line
    lcd.setCursor(0, 0);

    // Print the values as comma-separated integers
    char firstLine[17];  // 16 characters for the LCD + 1 for the null terminator
    snprintf(firstLine, sizeof(firstLine), "%d,%d,%d,%d,%1d,%1d   ", data.joystickX, data.joystickY, data.pot1, data.pot2, data.joystickButton, data.onOffKey);
    firstLine[16] = '\0';
    lcd.print(firstLine);

    lcd.setCursor(0, 1);               // Move to the second line
    if (inSleepMode) {
      lcd.print("Connecting......");
    } else {
      char secondLine[17];
      snprintf(secondLine, sizeof(secondLine), "%d millis           ", sendTime);
      lcd.print(secondLine);
    }

    lastLCDUpdate = millis();
  }
}
