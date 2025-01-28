# Radio Control

This project demonstrates how to create a wireless joystick control system using an Arduino and an ESP32. It uses the nRF24L01 wireless module to transmit joystick and potentiometer data, as well as a button press and on/off key states. This system can be used in remote control applications or custom input devices.

## Components Needed

- **Arduino (Sender)**
  - Arduino Uno (or any other compatible board)
  - nRF24L01 module
  - Joystick (2-axis, analog)
  - 2 potentiometers (analog)
  - Push button for joystick press
  - On/Off key switch
  - LED (for feedback)
  
- **ESP32 (Receiver)**
  - ESP32 development board
  - nRF24L01 module
  - USB cable for programming and powering the ESP32

## Features

- Wireless data transmission of joystick X/Y coordinates, two potentiometer values, joystick button press, and on/off key state.
- Configurable data send rate (every 50ms by default).
- Robust communication with retry mechanism (up to 3 retries) for sending data.
- Real-time status feedback with an LED indicator for successful/failed transmissions.
  
## Setup

### Sender (Arduino)
1. Connect the nRF24L01 module to the Arduino:
   - VCC → 3.3V
   - GND → GND
   - CE → Pin 9
   - CSN → Pin 10
   - SCK → Pin 13
   - MOSI → Pin 11
   - MISO → Pin 12

2. Connect the joystick and potentiometers to the appropriate analog pins:
   - Joystick X → A0
   - Joystick Y → A1
   - Potentiometer 1 → A2
   - Potentiometer 2 → A3
   - Joystick button → Pin 2
   - On/Off key → Pin 3

3. Upload the Arduino sender code to the Arduino board.

### Receiver (ESP32)
1. Connect the nRF24L01 module to the ESP32:
   - VCC → 3.3V
   - GND → GND
   - CE → Pin 4
   - CSN → Pin 5
   - SCK → Pin 18
   - MOSI → Pin 23
   - MISO → Pin 19

2. Upload the ESP32 receiver code to the ESP32.

3. Open the Serial Monitor on the ESP32 to see the received joystick data.

## Code Explanation

### Sender (Arduino)
The Arduino reads the joystick and potentiometer data, and transmits it over the nRF24L01 module. The transmission is performed every 50 milliseconds or whenever there is a significant change in the values. The sender also indicates the transmission success or failure using an LED.

### Receiver (ESP32)
The ESP32 listens for data from the Arduino and displays the received joystick values (X, Y), potentiometer readings (P1, P2), button states, and the on/off key state via the Serial Monitor.

## Wiring Diagram

Include a simple diagram or breadboard image of the wiring setup here.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Troubleshooting

- If the Arduino doesn’t send data, make sure the nRF24L01 is connected properly and the module is powered with 3.3V (not 5V).
- If the ESP32 doesn’t receive data, ensure that both devices are using the same frequency and address.
- If there is interference, try to use `RF24_PA_HIGH` for better transmission reliability.

## Contributions

Feel free to fork this project and make pull requests. Contributions and suggestions are welcome!


