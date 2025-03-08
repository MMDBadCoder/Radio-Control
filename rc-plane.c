#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ESP32Servo.h>  // Library for controlling servos and ESC

// Pin definitions
#define CE_PIN 4            // Chip Enable pin for nRF24L01
#define CSN_PIN 5           // Chip Select Not pin for nRF24L01
#define MAIN_MOTOR_PIN 33   // Pin for the brushless motor ESC signal
#define WINGS_SERVO_PIN 26  // Pin for the wings servo motor
#define TAIL_SERVO_PIN 27   // Pin for the tail servo motor
#define LED_PIN 25          // Pin for the connection status LED

// Create RF24 object for radio communication
RF24 radio(CE_PIN, CSN_PIN);

// Create Servo objects for wings, tail, and ESC
Servo wingsServo;
Servo tailServo;
Servo esc;

// Define the radio address (must match the sender's address)
const byte address[6] = "00001";

// Data structure to store received data
struct DataPacket {
  int8_t joystickX;     // Joystick X-axis value (-100 to 100)
  int8_t joystickY;     // Joystick Y-axis value (-100 to 100)
  uint8_t pot1;         // Potentiometer 1 value (0 to 100)
  uint8_t pot2;         // Potentiometer 2 value (0 to 100)
  bool joystickButton;  // Joystick button state
  bool onOffKey;        // On/Off key state
};

// Variables for connection timeout and LED blinking
unsigned long lastReceivedTime = 0;  // Timestamp of the last received message
const unsigned long timeout = 1000;  // 1-second timeout for connection loss
bool isConnected = false;            // Track connection status

void setup() {
  Serial.begin(115200);  // Initialize serial communication for debugging

  // Initialize the radio module
  Serial.println("Initializing radio...");
  while (!radio.begin()) {
    Serial.println("Radio initialization failed. Retrying...");
    delay(1000);  // Wait 1 second before retrying
  }
  Serial.println("Radio initialized successfully!");

  // Configure radio settings
  radio.openReadingPipe(1, address);  // Open a reading pipe with the specified address
  radio.setPALevel(RF24_PA_HIGH);     // Set power amplification level to high
  radio.setDataRate(RF24_2MBPS);      // Set data rate to 2 Mbps
  radio.startListening();             // Start listening for incoming data

  // Attach servos and ESC to their respective pins
  wingsServo.attach(WINGS_SERVO_PIN);
  tailServo.attach(TAIL_SERVO_PIN);
  esc.attach(MAIN_MOTOR_PIN, 1000, 2000);  // Attach ESC with a custom pulse range
  esc.write(0);                         // Initialize ESC to 0 speed

  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // Turn off LED initially
}

void loop() {
  // Check if data is available from the radio
  if (radio.available()) {
    DataPacket data;

    // Read the incoming data into the struct
    radio.read(&data, sizeof(data));

    // Print received data to the serial monitor for debugging
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

    // Update the last received time and connection status
    lastReceivedTime = millis();
    isConnected = true;

    // Control motors and LED based on received data
    controlMotors(data);

    // Keep the LED ON as long as the connection is active
    digitalWrite(LED_PIN, HIGH);
  } else {
    // If no data is received for more than the timeout period, assume connection is lost
    if (millis() - lastReceivedTime > timeout) {
      isConnected = false;
    }
  }

  // Blink the LED if not connected
  if (!isConnected) {
    digitalWrite(LED_PIN, (millis() / 500) % 2);  // Blink LED every 500ms
  }
}

// Function to control the motors based on received data
void controlMotors(DataPacket data) {
  if (data.onOffKey) {
    esc.write(0);  // Stop the motor
  } else {
    // Control the brushless motor (ESC) speed using potentiometer 1 value
    int motorSpeed = map(data.pot1, 0, 100, 0, 180);  // Map potentiometer value to ESC range
    esc.write(motorSpeed);                            // Set motor speed
  }

  // Control the wings servo using joystick Y-axis value
  int wingsAngle = map(data.joystickY, -100, 100, 0, 180);  // Map joystick Y to servo angle
  wingsServo.write(wingsAngle);                             // Set wings servo angle

  // Control the tail servo using joystick X-axis value
  int tailAngle = map(data.joystickX, -100, 100, 180, 0);  // Map joystick X to servo angle
  tailServo.write(tailAngle);                              // Set tail servo angle

  // If the on/off key is pressed, stop the motor
  if (data.onOffKey) {
    esc.write(0);  // Stop the motor
  }
}