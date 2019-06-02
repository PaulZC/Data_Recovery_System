// ###########################
// # Data Recovery System V1 #
// ###########################

//#define DRS1
#define DRS2
//#define DRS3
//#define DRS4
//#define DRS5

// A hybrid of the Iridium9603NBeacon_V5 and Cut-Down_Device

// Uses Cristian Maglie's FlashStorage library to store the BEACON_INTERVAL and the servo settings
// https://github.com/cmaglie/FlashStorage
// BEACON_INTERVAL can be updated via an Iridium Mobile Terminated message

// With grateful thanks to Mikal Hart:
// Based on Mikal's IridiumSBD Beacon example: https://github.com/mikalhart/IridiumSBD
// Requires Mikal's TinyGPS library: https://github.com/mikalhart/TinyGPS
// and PString: http://arduiniana.org/libraries/pstring/

// With grateful thanks to:
// Adafruit: https://learn.adafruit.com/using-atsamd21-sercom-to-add-more-spi-i2c-serial-ports/creating-a-new-serial
// MartinL: https://forum.arduino.cc/index.php?topic=341054.msg2443086#msg2443086

// The Iridium_9603_Beacon PCB is based extensively on the Adafruit Feather M0 (Adalogger)
// https://www.adafruit.com/products/2796

// GNSS data provided by u-blox MAX-M8Q
// https://www.u-blox.com/en/product/max-m8-series

// WB2812B NeoPixel is connected to D13 in parallel with a standard Red LED (for the bootloader)
// Support for the WB2812B is provided by Adafruit:
// https://github.com/adafruit/Adafruit_NeoPixel

// Uses RTCZero to provide sleep functionality (on the M0)
// https://github.com/arduino-libraries/RTCZero

// With grateful thanks to CaveMoa for his SimpleSleepUSB example
// https://github.com/cavemoa/Feather-M0-Adalogger
// https://github.com/cavemoa/Feather-M0-Adalogger/tree/master/SimpleSleepUSB
// Note: you will need to close and re-open your serial monitor each time the M0 wakes up

// With thanks to David A. Mellis and Tom Igoe for the smoothing tutorial
// http://www.arduino.cc/en/Tutorial/Smoothing

// Stores both source and destination RockBLOCK serial numbers in flash.
// The default values are:
#define RB_destination 0 // Serial number of the destination RockBLOCK (int). Set to zero to disable RockBLOCK message forwarding

// Define default servo positions and RB_source
#ifdef DRS1
  // Default values for DRS1
  #define defServoOpenA 1869
  #define defServoClosedA 1227
  #define defServoOpenB 1937
  #define defServoClosedB 1513
  #define RB_source 11800
#endif
#ifdef DRS2
// Default values for DRS2
  #define defServoOpenA 1817
  #define defServoClosedA 1368
  #define defServoOpenB 1639
  #define defServoClosedB 1235
  #define RB_source 16366
#endif
#ifdef DRS3
// Default values for DRS3
  #define defServoOpenA 1500
  #define defServoClosedA 1500
  #define defServoOpenB 1500
  #define defServoClosedB 1500
  #define RB_source 16363
#endif
#ifdef DRS4
// Default values for DRS4
  #define defServoOpenA 1500
  #define defServoClosedA 1500
  #define defServoOpenB 1500
  #define defServoClosedB 1500
  #define RB_source 16365
#endif
#ifdef DRS5
// Default values for DRS5
  #define defServoOpenA 1500
  #define defServoClosedA 1500
  #define defServoOpenB 1500
  #define defServoClosedB 1500
  #define RB_source 16369
#endif

// ATSAMD21G18A Pin Allocations:
//
// D0  : Serial1 RX (GNSS)
// D1  : Serial1 TX (GNSS)
// D2  : Servo B power switching
// D4 / TCC1WO2 : Sounder enable
// D5 / TCC0WO5 : Servo PWM
// D6  : 9603N ON_OFF
// D7  : Servo A power switching
// D8  : 5V regulator enable
// D10 : Serial2 TX (9603N)
// D11 : GNSS power switching
// D12 : Serial2 RX (9603N)
// D13 : LEDs
// A0 / D14 : Open Servo B Switch
// A1 / D15 : Open Servo A Switch
// A2 / D16 : Close Servo A Switch
// A3 / D17 : 9603N Ring Indicator
// A4 / D18 : Close Servo B Switch
// A5 / D19 : 9603N power switching
// A7 / D9 : Bus voltage monitoring
// MOSI / D23 : Serial3 TX (Pi)
// SCK / D24 : Serial3 RX (Pi)

// !!! Ensure GPS_EN and Enable_9603N are not enabled at the same time !!!

// Red LED on D13 shows when the SAMD is in bootloader mode (LED will fade up/down)

// WB2812B on D13 indicates what the software is doing:
// Cyan: setServo mode (setServoA and setServoB)
// Blue: power is on; GNSS is powered up; waiting for the Go command (init_GPS and pi_echo)
// Magenta: Go command received; opening ServoA (drop and init)
// Blue: waiting for a GPS fix (start_GPS and read_GPS)
// White: Iridium transmit (start_9603)
// Green flash (2 seconds) indicates successful transmission
// Red flash (2+ seconds) entering sleep
// LED will flash Red after: Iridium transmission (successful or failure); low battery detected
// WB2812B blue LED has the highest forward voltage and is slightly dim at 3.3V. The red and green values are adapted accordingly (222 instead of 255).

static const int ServoB = 2; // Servo B Enable (Pull high to enable the servo)
static const int Sounder = 4; // Sounder Enable (Pull low to enable the sounder)
const uint8_t pwmPin = 5;
static const int IridiumSleepPin = 6; // Iridium 9603N ON/OFF
static const int ServoA = 7; // Servo A Enable (Pull high to enable the servo)
static const int enable5V = 8; // Enable the MPM3610 5V regulator (Pull high to enable 5V)
static const int GPS_EN = 11; // GNSS Enable (Pull low to enable the GNSS)
static const int ledPin = 13; // WB2812B + Red LED
static const int openSwitchB = 14;
static const int openSwitchA = 15;
static const int closeSwitchA = 16;
static const int ringIndicator = 17; // 9603N Ring Indicator
static const int closeSwitchB = 18;
static const int Enable_9603N = 19; // 9603N Enable (Pull high to enable power for the 9603N)

#include <IridiumSBD.h> // Requires V2: https://github.com/mikalhart/IridiumSBD
#include <TinyGPS.h> // NMEA parsing: http://arduiniana.org
#include <PString.h> // String buffer formatting: http://arduiniana.org

#include <Adafruit_NeoPixel.h> // Support for the WB2812B

#include <RTCZero.h> // M0 Real Time Clock
RTCZero rtc; // Create an rtc object

// Define how often messages are sent in MINUTES (max 1440)
// This is the _quickest_ messages will be sent. Could be much slower than this depending on:
// gnss fix time; Iridium timeout; etc.
// The default value will be overwritten with the one stored in Flash - if one exists
// The value can be changed via a Mobile Terminated message
int BEACON_INTERVAL = 2;

// Flash Storage
#include <FlashStorage.h>
typedef struct { // Define a struct to hold the flash variable(s)
  int PREFIX; // Flash storage prefix (0xB5); used to test if flash has been written to before 
  int INTERVAL; // Message interval in minutes
  // RockBLOCK source serial number: stored as an int; i.e. the RockBLOCK serial number of the 9603N attached to this beacon
  int RBSOURCE; 
  // RockBLOCK destination serial number: stored as an int; i.e. the RockBLOCK serial number of the 9603N you would like the messages delivered _to_
  int RBDESTINATION; // Set this to zero to disable RockBLOCK gateway message forwarding
  int servoOpenA; // Servo A PWM value for the open position
  int servoClosedA; // Servo A PWM value for the closed position
  int servoOpenB; // Servo B PWM value for the open position
  int servoClosedB; // Servo B PWM value for the closed position
  int GONE; // Flag to indicate if GoGoGo has been received (set loop_step to init not init_GPS after reset)
  int SOUNDER; // Flag to indicate if the sounder has been disabled via SBD message
  int CSUM; // Flash storage checksum; the modulo-256 sum of PREFIX to servoClosedB; used to check flash data integrity
} FlashVarsStruct;
FlashStorage(flashVarsMem, FlashVarsStruct); // Reserve memory for the flash variables
FlashVarsStruct flashVars; // Define the global to hold the variables
int RBSOURCE = RB_source;
int RBDESTINATION = RB_destination;

// Serial2 pin and pad definitions (in Arduino files Variant.h & Variant.cpp)
#define PIN_SERIAL2_RX       (34ul)               // Pin description number for PIO_SERCOM on D12
#define PIN_SERIAL2_TX       (36ul)               // Pin description number for PIO_SERCOM on D10
#define PAD_SERIAL2_TX       (UART_TX_PAD_2)      // SERCOM pad 2 (SC1PAD2)
#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_3)    // SERCOM pad 3 (SC1PAD3)
// Instantiate the Serial2 class
Uart Serial2(&sercom1, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX);
HardwareSerial &ssIridium(Serial2);

// Serial3 pin and pad definitions (in Arduino files Variant.h & Variant.cpp)
// Serial3 Tx : MOSI (Digital Pin 23, Port B Pin 10, SERCOM4 Pad 2)
// Serial3 Rx : SCK (Digital Pin 24, Port B Pin 11, SERCOM4 Pad 3)
#define PIN_SERIAL3_RX       (24ul)               // Pin description number for PIO_SERCOM on D24
#define PIN_SERIAL3_TX       (23ul)               // Pin description number for PIO_SERCOM on D23
#define PAD_SERIAL3_TX       (UART_TX_PAD_2)      // SERCOM4 Pad 2 (SC4PAD2)
#define PAD_SERIAL3_RX       (SERCOM_RX_PAD_3)    // SERCOM4 Pad 3 (SC4PAD3)
// Instantiate the Serial3 class
Uart Serial3(&sercom4, PIN_SERIAL3_RX, PIN_SERIAL3_TX, PAD_SERIAL3_RX, PAD_SERIAL3_TX);
HardwareSerial &ssPi(Serial3);

#define ssGPS Serial1 // Use M0 Serial1 to interface to the MAX-M8Q

// Leave the "#define GALILEO" uncommented to use: GPS + Galileo + GLONASS + SBAS
// Comment the "#define GALILEO" out to use the default u-blox M8 GNSS: GPS + SBAS + QZSS + GLONASS
#define GALILEO

// Set Nav Mode to Portable
static const uint8_t setNavPortable[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0x01, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Set Nav Mode to Airborne <1G
static const uint8_t setNavAir[] = {
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0x01, 0x00, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
  0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const int len_setNav = 42;

// Set NMEA Config
// Set trackFilt to 1 to ensure course (COG) is always output
// Set Main Talker ID to 'GP' to avoid having to modify TinyGPS
static const uint8_t setNMEA[] = {
  0xb5, 0x62, 0x06, 0x17, 0x14, 0x00, 0x20, 0x40, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const int len_setNMEA = 26;

// Set GNSS Config to GPS + Galileo + GLONASS + SBAS (Causes the M8 to restart!)
static const uint8_t setGNSS[] = {
  0xb5, 0x62, 0x06, 0x3e, 0x3c, 0x00,
  0x00, 0x20, 0x20, 0x07,
  0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01,
  0x02, 0x08, 0x08, 0x00, 0x01, 0x00, 0x01, 0x01,
  0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x03,
  0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x05,
  0x06, 0x04, 0x0e, 0x00, 0x01, 0x00, 0x01, 0x01 };
static const int len_setGNSS = 66;

// Clear Stored Configuration
// Clears msgConf, navConf, rxmConf
static const uint8_t clearConf[] = {
  0xb5, 0x62, 0x06, 0x09, 0x0C, 0x00,
  0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
// Save Configuration
// Saves msgConf, navConf, rxmConf
static const uint8_t saveConf[] = {
  0xb5, 0x62, 0x06, 0x09, 0x0C, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
// Load Configuration
// Loads msgConf, navConf, rxmConf
static const uint8_t loadConf[] = {
  0xb5, 0x62, 0x06, 0x09, 0x0C, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00 };
static const int len_Conf = 18;

IridiumSBD isbd(ssIridium, IridiumSleepPin); // This should disable the 9603
TinyGPS tinygps;
long iterationCounter = 0; // Increment each time a transmission is attempted

//#define NoLED // Uncomment this line to disable the LED
#define swap_red_green // Uncomment this line if your WB2812B has red and green reversed
#ifdef swap_red_green
  Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, ledPin, NEO_GRB + NEO_KHZ800); // GRB WB2812B
#else
  Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, ledPin, NEO_RGB + NEO_KHZ800); // RGB WB2812B
#endif
#define LED_Brightness 32 // 0 - 255 for WB2812B

#define GPS_ON LOW
#define GPS_OFF HIGH
#define SOUNDER_ON LOW
#define SOUNDER_OFF HIGH
#define SERVO_ON HIGH
#define SERVO_OFF LOW
#define IRIDIUM_ON HIGH
#define IRIDIUM_OFF LOW
#define FIVE_V_ON HIGH
#define FIVE_V_OFF LOW

#define VBUS A7 // Bus voltage analog pin
#define VBAT_LOW 6.5 // Minimum battery voltage for MPM3610

// Loop Steps
#define setServoA     0
#define setServoB     1
#define init_GPS      2
#define pi_echo       3
#define drop          4
#define init          5
#define start_GPS     6
#define read_GPS      7
#define start_9603    8
#define zzz           9
#define wake          10
#define close_a       11
#define open_a        12
#define close_b       13
#define open_b        14

// Variables used by Loop
int year;
byte month, day, hour, minute, second, hundredths;
unsigned long dateFix, locationFix;
float latitude, longitude;
long altitude;
float speed;
short satellites;
long course;
long hdop;
bool fixFound = false;
bool charsSeen = false;
int loop_step = init_GPS;
float vbat = 9.0;
unsigned long tnow;
bool open_servo_B_flag = false;

// Servo Position:
// Min Position = 900
// Mid Position = 1500
// Max Position = 2100
const int minServo = 900;
const int midServo = 1500;
const int maxServo = 2100;
int PWM = midServo;
int servoOpenA = midServo;
int servoClosedA = midServo;
int servoOpenB = midServo;
int servoClosedB = midServo;

// Drop countdown (secs)
#define countdown 30

// Storage for the average voltage during Iridium callbacks
const int numReadings = 25;   // number of samples
int readings[numReadings];    // the readings from the analog input
int readIndex = 0;            // the index of the current reading
long int total = 0;           // the running total
int latest_reading = 0;       // the latest reading
int average_reading = 0;      // the average reading

// IridiumSBD Callbacks
bool ISBDCallback()
{
#ifndef NoLED
  // 'Flash' the LED
  if ((millis() / 1000) % 2 == 1) {
    LED_dim_white();
  }
  else {
    LED_white();
  }
#endif

  // Check the 'battery' voltage now we are drawing current for the 9603
  // If voltage is low, stop Iridium send
  // Average voltage over numReadings to smooth out any short dips
  get_vbat_smooth();

  if (vbat < VBAT_LOW) {
    Serial.print("***!!! LOW VOLTAGE (ISBDCallback) ");
    Serial.print(vbat,2);
    Serial.println("V !!!***");
    return false; // Returning false causes IridiumSBD to terminate
  }
  else {     
    return true;
  }

  delay(1);
}
// V2 console and diagnostic callbacks (replacing attachConsole and attachDiags)
void ISBDConsoleCallback(IridiumSBD *device, char c) { Serial.write(c); }
void ISBDDiagsCallback(IridiumSBD *device, char c) { Serial.write(c); }

// Interrupt handler for SERCOM1 (essential for Serial2 comms)
void SERCOM1_Handler()
{
  Serial2.IrqHandler();
}

// Interrupt handler for SERCOM4 (essential for Serial3 comms)
void SERCOM4_Handler()
{
  Serial3.IrqHandler();
}

// RTC alarm interrupt
void alarmMatch()
{
  int rtc_mins = rtc.getMinutes(); // Read the RTC minutes
  int rtc_hours = rtc.getHours(); // Read the RTC hours
  if (BEACON_INTERVAL > 1440) BEACON_INTERVAL = 1440; // Limit BEACON_INTERVAL to one day
  rtc_mins = rtc_mins + BEACON_INTERVAL; // Add the BEACON_INTERVAL to the RTC minutes
  while (rtc_mins >= 60) { // If there has been an hour roll over
    rtc_mins = rtc_mins - 60; // Subtract 60 minutes
    rtc_hours = rtc_hours + 1; // Add an hour
  }
  rtc_hours = rtc_hours % 24; // Check for a day roll over
  rtc.setAlarmMinutes(rtc_mins); // Set next alarm time (minutes)
  rtc.setAlarmHours(rtc_hours); // Set next alarm time (hours)
}

// Read and smooth the 'battery' voltage
// Average voltage over numReadings to smooth out any short dips
void get_vbat_smooth() {
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  latest_reading = analogRead(VBUS);
  readings[readIndex] = latest_reading;
  // add the reading to the total:
  total = total + latest_reading;
  // advance to the next position in the array:
  readIndex = readIndex + 1;
  // if we're at the end of the array...wrap around to the beginning:
  if (readIndex >= numReadings) readIndex = 0;
  // calculate the average:
  average_reading = total / numReadings; // Seems to work OK with integer maths - but total does need to be long int
  vbat = float(average_reading) * (4.3 * 3.3 / 1023.0); // Calculate average battery voltage
}

// Read battery voltage, compensating for resistor divider
void get_vbat() {
  vbat = analogRead(VBUS) * (4.3 * 3.3 / 1023.0);
}

// Initialise the smoothed 'battery' voltage
void init_vbat()
{
  // Initialise voltage sample buffer with current readings
  total = 0;
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = analogRead(VBUS);
    total = total + readings[thisReading];
    delay(1);
  }
  get_vbat_smooth();
}

void LED_off() // Turn NeoPixel off
{
  pixels.setPixelColor(0,0,0,0);
  pixels.show();
}

void LED_dim_white() // Set LED to dim white
{
  pixels.setBrightness(LED_Brightness / 2); // Dim the LED brightness
  pixels.setPixelColor(0, pixels.Color(222,222,255)); // Set color.
  pixels.show(); // This sends the updated pixel color to the hardware.
  pixels.setBrightness(LED_Brightness); // Reset the LED brightness
}

void LED_dim_blue() // Set LED to dim blue
{
  pixels.setBrightness(LED_Brightness / 2); // Dim the LED brightness
  pixels.setPixelColor(0, pixels.Color(0,0,255)); // Set color.
  pixels.show(); // This sends the updated pixel color to the hardware.
  pixels.setBrightness(LED_Brightness); // Reset the LED brightness
}

void LED_dim_cyan() // Set LED to dim cyan
{
  pixels.setBrightness(LED_Brightness / 2); // Dim the LED brightness
  pixels.setPixelColor(0, pixels.Color(0,222,255)); // Set color.
  pixels.show(); // This sends the updated pixel color to the hardware.
  pixels.setBrightness(LED_Brightness); // Reset the LED brightness
}

void LED_white() // Set LED to white
{
  pixels.setPixelColor(0, pixels.Color(222,222,255)); // Set color.
  pixels.show(); // This sends the updated pixel color to the hardware.
}

void LED_red() // Set LED to red
{
  pixels.setPixelColor(0, pixels.Color(222,0,0)); // Set color.
  pixels.show(); // This sends the updated pixel color to the hardware.
}

void LED_green() // Set LED to green
{
  pixels.setPixelColor(0, pixels.Color(0,222,0)); // Set color.
  pixels.show(); // This sends the updated pixel color to the hardware.
}

void LED_blue() // Set LED to blue
{
  pixels.setPixelColor(0, pixels.Color(0,0,255)); // Set color.
  pixels.show(); // This sends the updated pixel color to the hardware.
}

void LED_cyan() // Set LED to cyan
{
  pixels.setPixelColor(0, pixels.Color(0,222,255)); // Set color.
  pixels.show(); // This sends the updated pixel color to the hardware.
}

void LED_magenta() // Set LED to magenta
{
  pixels.setPixelColor(0, pixels.Color(222,0,255)); // Set color.
  pixels.show(); // This sends the updated pixel color to the hardware.
}

// Send message in u-blox UBX format
// Calculates and appends the two checksum bytes
// Doesn't add the 0xb5 and 0x62 sync chars (these need to be included at the start of the message)
void sendUBX(const uint8_t *message, const int len) {
  int csum1 = 0; // Checksum bytes
  int csum2 = 0;
  for (int i=0; i<len; i++) { // For each byte in the message
    ssGPS.write(message[i]); // Write the byte
    if (i >= 2) { // Don't include the sync chars in the checksum
      csum1 = csum1 + message[i]; // Update the checksum bytes
      csum2 = csum2 + csum1;
    }
  }
  csum1 = csum1 & 0xff; // Limit checksums to 8-bits
  csum2 = csum2 & 0xff;
  ssGPS.write((uint8_t)csum1); // Send the checksum bytes
  ssGPS.write((uint8_t)csum2);
}

// http://forum.arduino.cc/index.php?topic=288234.0
const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data
boolean newData = false;

void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

const byte numPiChars = 32;
char receivedPiChars[numPiChars]; // an array to store the received data
boolean newPiData = false;

void recvPiWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while ((ssPi.available()) && (newPiData == false)) {
    rc = ssPi.read();

    if (rc != endMarker) {
      receivedPiChars[ndx] = rc;
      ndx++;
      if (ndx >= numPiChars) {
        ndx = numPiChars - 1;
      }
    }
    else {
      receivedPiChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newPiData = true;
    }
  }
}

void setup()
{
  pinMode(enable5V, OUTPUT);
  digitalWrite(enable5V, FIVE_V_OFF); // Disable 5V regulator

  pinMode(ServoB, OUTPUT);
  digitalWrite(ServoB, SERVO_OFF); // Disable switched 5V power for servo B

  pinMode(ServoA, OUTPUT);
  digitalWrite(ServoA, SERVO_OFF); // Disable switched 5V power for servo A

  pinMode(Enable_9603N, OUTPUT);
  digitalWrite(Enable_9603N, IRIDIUM_OFF); // Disable switched 5V power for Iridium 9603N

  pinMode(GPS_EN, OUTPUT); // GPS & MPL3115A2 enable
  digitalWrite(GPS_EN, GPS_OFF); // Disable the GPS and MPL3115A2
  
  pinMode(Sounder, OUTPUT);
  digitalWrite(Sounder, SOUNDER_OFF); // Disable the sounder

  pinMode(openSwitchA, INPUT_PULLUP);
  pinMode(closeSwitchA, INPUT_PULLUP);
  pinMode(openSwitchB, INPUT_PULLUP);
  pinMode(closeSwitchB, INPUT_PULLUP);
  
  pinMode(pwmPin, OUTPUT); // Redundant?

  //pinMode(IridiumSleepPin, OUTPUT); // Iridium 9603N Sleep Pin
  //digitalWrite(IridiumSleepPin, LOW); // Disable the Iridium 9603
  pinMode(ringIndicator, INPUT); // Define an input for the Iridium 9603 Ring Indicator

  pixels.begin(); // This initializes the NeoPixel library.
  delay(100); // Seems necessary to make the NeoPixel start reliably 
  pixels.setBrightness(LED_Brightness); // Initialize the LED brightness
  LED_off(); // Turn NeoPixel off

  // Update default servo positions
  servoOpenA = defServoOpenA;
  servoClosedA = defServoClosedA;
  servoOpenB = defServoOpenB;
  servoClosedB = defServoClosedB;
  
  // See if global variables have already been stored in flash
  // If they have, read them. If not, initialise them.
  flashVars = flashVarsMem.read(); // Read the flash memory
  int csum = flashVars.PREFIX + flashVars.INTERVAL + flashVars.RBSOURCE + flashVars.RBDESTINATION + flashVars.servoOpenA + flashVars.servoClosedA + flashVars.servoOpenB + flashVars.servoClosedB + flashVars.GONE + flashVars.SOUNDER; // Sum the prefix and data
  csum = csum & 0xff; // Limit checksum to 8-bits
  if ((flashVars.PREFIX == 0xB5) and (csum == flashVars.CSUM)) { // Check prefix and checksum match
    // Flash data is valid so update globals using the stored values
    BEACON_INTERVAL = flashVars.INTERVAL;
    RBSOURCE = flashVars.RBSOURCE;
    RBDESTINATION = flashVars.RBDESTINATION;
    servoOpenA = flashVars.servoOpenA;
    servoClosedA = flashVars.servoClosedA;
    servoOpenB = flashVars.servoOpenB;
    servoClosedB = flashVars.servoClosedB;
  }
  else {
    // Flash data is corrupt or hasn't been initialised so do that now
    flashVars.PREFIX = 0xB5; // Initialise the prefix
    flashVars.INTERVAL = BEACON_INTERVAL; // Initialise the beacon interval
    flashVars.RBSOURCE = RBSOURCE; // Initialise the source RockBLOCK serial number
    flashVars.RBDESTINATION = RBDESTINATION; // Initialise the destination RockBLOCK serial number
    flashVars.servoOpenA = servoOpenA; // Initialise the servo settings
    flashVars.servoClosedA = servoClosedA;
    flashVars.servoOpenB = servoOpenB;
    flashVars.servoClosedB = servoClosedB;
    flashVars.GONE = false; // Initialise the GONE flag
    flashVars.SOUNDER = true; // Initialise the SOUNDER flag
    csum = flashVars.PREFIX + flashVars.INTERVAL + flashVars.RBSOURCE + flashVars.RBDESTINATION + flashVars.servoOpenA + flashVars.servoClosedA + flashVars.servoOpenB + flashVars.servoClosedB + flashVars.GONE + flashVars.SOUNDER; // Initialise the checksum
    csum = csum & 0xff;
    flashVars.CSUM = csum;
    flashVarsMem.write(flashVars); // Write the flash variables
  }

  rtc.begin(); // Start the RTC now that BEACON_INTERVAL has been updated
  rtc.setAlarmSeconds(rtc.getSeconds()); // Initialise RTC Alarm Seconds
  alarmMatch(); // Set next alarm time using updated BEACON_INTERVAL
  rtc.enableAlarm(rtc.MATCH_HHMMSS); // Alarm Match on hours, minutes and seconds
  rtc.attachInterrupt(alarmMatch); // Attach alarm interrupt
  
  iterationCounter = 0; // Make sure iterationCounter is set to zero (indicating a reset)

  // Force 48MHz oscillator to run in standby mode
  //SYSCTRL->VREG.bit.RUNSTDBY = 1;
  SYSCTRL->DFLLCTRL.bit.RUNSTDBY = 1;

  // Initialise PWM output for servo
  // Output 50Hz PWM on timer TCC0 (14-bit resolution)
  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(3) |          // Divide the 48MHz clock source by divisor 3: 48MHz/3=16MHz
                    GCLK_GENDIV_ID(4);            // Select Generic Clock (GCLK) 4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |           // Set the duty cycle to 50/50 HIGH/LOW
                     GCLK_GENCTRL_GENEN |         // Enable GCLK4
                     GCLK_GENCTRL_SRC_DFLL48M |   // Set the 48MHz clock source
                     GCLK_GENCTRL_RUNSTDBY |      // Keep the clock running during standby
                     GCLK_GENCTRL_ID(4);          // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  // Enable the port multiplexer for the PWM channel: timer TCC0 outputs
  // Digital pin 5 is PA15
  // PA15 Peripheral E is TC3WO1
  // PA15 Peripheral F is TCC0WO5
  // PORT->Group[g_APinDescription[pwmPin].ulPort].PINCFG[g_APinDescription[pwmPin].ulPin].bit.PMUXEN = 1;
  PORT->Group[PORTA].PINCFG[15].bit.PMUXEN = 1; // Set the PMUXEN bit in the PINCFG register to allow TCC) to control the pin
  // Connect the TCC0 timer to the port outputs - port pins are paired odd PMUO and even PMUXE
  // F & E specify the timers: TCC0, TCC1 and TCC2
  // Digital pin 5 is PA15 (odd). PA14 (even) is Digital pin 2. So need to refer to pin 2 ODD!
  // PORT->Group[g_APinDescription[2].ulPort].PMUX[g_APinDescription[2].ulPin >> 1].reg = PORT_PMUX_PMUXO_F;
  PORT->Group[PORTA].PMUX[14 >> 1].reg = PORT_PMUX_PMUXO_F; // Set the odd nibble to peripheral F

  // Feed GCLK4 to TCC0 and TCC1
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         // Enable GCLK4 to TCC0 and TCC1
                     GCLK_CLKCTRL_GEN_GCLK4 |     // Select GCLK4
                     GCLK_CLKCTRL_ID_TCC0_TCC1;   // Feed GCLK4 to TCC0 and TCC1
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  // Dual slope PWM operation: timers countinuously count up to PER register value then down 0
  REG_TCC0_WAVE |= TCC_WAVE_POL(0xF) |         // Reverse the output polarity on all TCC0 outputs
                    TCC_WAVE_WAVEGEN_DSBOTTOM;    // Setup dual slope PWM on TCC0
  while (TCC0->SYNCBUSY.bit.WAVE);               // Wait for synchronization

  // Each timer counts up to a maximum or TOP value set by the PER register,
  // this determines the frequency of the PWM operation:
  // 20000 = 50Hz, 10000 = 100Hz, 2500  = 400Hz
  REG_TCC0_PER = 20000;      // Set the frequency of the PWM on TCC0 to 50Hz
  while(TCC0->SYNCBUSY.bit.PER);

  // The CCBx register value corresponds to the pulsewidth in microseconds (us)
  // We want to use TCC0WO5 as the output
  // TCC0 has four CC capture compare channels connected to eight WO waveform outputs
  // The output matrix has not been changed so we need to use CC(B)1
  // WO[7] WO[6] WO[5] WO[4] WO[3] WO[2] WO[1] WO[0]
  //  CC3   CC2   CC1   CC0   CC3   CC2   CC1   CC0
  REG_TCC0_CCB1 = PWM;       // TCC0 CCB1 - center the servo on D5
  while(TCC0->SYNCBUSY.bit.CCB1);

  // Divide the 16MHz signal by 8 giving 2MHz (0.5us) TCC0 timer tick and enable the outputs
  REG_TCC0_CTRLA |= TCC_CTRLA_PRESCALER_DIV8 |    // Divide GCLK4 by 8
                    //TCC_CTRLA_RUNSTDBY |            // Keep the timer running in standby
                    TCC_CTRLA_ENABLE;             // Enable the TCC0 output
  while (TCC0->SYNCBUSY.bit.ENABLE);              // Wait for synchronization

  // Set up the slow pulse for the sounder (D4 / PA08 / TCC1WO2)
  // PA08 Peripheral E is TCC0WO0
  // PA08 Peripheral F is TCC1WO2
  // Enable the port multiplexer for the PWM channel: timer TCC1 outputs
  //PORT->Group[g_APinDescription[Sounder].ulPort].PINCFG[g_APinDescription[Sounder].ulPin].bit.PMUXEN = 1;
  
  //PORT->Group[PORTA].PINCFG[8].reg |= PORT_PINCFG_PMUXEN; // This enables the sounder - leave it commented out
  
  // Connect the TCC1 timer to the port outputs - port pins are paired odd PMUO and even PMUXE
  // F & E specify the timers: TCC0, TCC1 and TCC2
  // Digital pin 4 is PA08 (even).
  //PORT->Group[g_APinDescription[4].ulPort].PMUX[g_APinDescription[4].ulPin >> 1].reg = PORT_PMUX_PMUXE_F;
  PORT->Group[PORTA].PMUX[8 >> 1].reg = PORT_PMUX_PMUXE_F;

  // Dual slope PWM operation: timers countinuously count up to PER register value then down 0
  REG_TCC1_WAVE |= TCC_WAVE_POL(0xF) |         // Reverse the output polarity on all TCC1 outputs
                    TCC_WAVE_WAVEGEN_DSBOTTOM;    // Setup dual slope PWM on TCC1
  while (TCC1->SYNCBUSY.bit.WAVE);               // Wait for synchronization

  // Each timer counts up to a maximum or TOP value set by the PER register,
  // this determines the frequency of the PWM operation:
  REG_TCC1_PER = 39062;      // Set the frequency of the PWM on TCC1 to 0.2Hz
  while(TCC1->SYNCBUSY.bit.PER);

  // The CCBx register value corresponds to the pulsewidth
  // We want to use TCC1WO2 as the output
  // TTC1 only has two CC channels
  // The output matrix has not been changed so we need to use CC(B)0
  // WO[3] WO[2] WO[1] WO[0]
  //  CC1   CC0   CC1   CC0
  REG_TCC1_CCB0 = 37109;       // TCC1 CCB2 - set sounder duty cycle (5:95)
  while(TCC1->SYNCBUSY.bit.CCB0);

  // Divide the 16MHz signal by 1024 giving 15625Hz (64us) TCC1 timer tick and enable the outputs
  REG_TCC1_CTRLA |= TCC_CTRLA_PRESCALER_DIV1024 |    // Divide GCLK4 by 1024
                    TCC_CTRLA_RUNSTDBY |            // Keep the timer running in standby
                    TCC_CTRLA_ENABLE;             // Enable the TCC1 output
  while (TCC1->SYNCBUSY.bit.ENABLE);              // Wait for synchronization

    
  // Set loop_step
  loop_step = init_GPS; // Initialise loop_step
  int switchState = readSwitchState();
  if (switchState == 0x03) // If both A switches are pressed
  {
    LED_cyan(); // Set LED to Cyan to show we are entering setServo mode
    while (switchState != 0x00) {switchState = readSwitchState();} // Wait for the switches to be released
    loop_step = setServoA;
  }
  else if (switchState == 0x0C) // If both B switches are pressed
  {
    LED_cyan(); // Set LED to Cyan to show we are entering setServo mode
    while (switchState != 0x00) {switchState = readSwitchState();} // Wait for the switches to be released
    loop_step = setServoB;
  }
  else if (switchState == 0x0F) // If all four switches are pressed
  {
    // Clear the GONE flag and leave loop_step set to init_GPS
    flashVars.PREFIX = 0xB5; // Initialise the prefix (hopefully redundant!)
    flashVars.GONE = false; // Clear the GONE flag
    flashVars.SOUNDER = true; // Set the SOUNDER flag
    csum = flashVars.PREFIX + flashVars.INTERVAL + flashVars.RBSOURCE + flashVars.RBDESTINATION + flashVars.servoOpenA + flashVars.servoClosedA + flashVars.servoOpenB + flashVars.servoClosedB + flashVars.GONE + flashVars.SOUNDER; // Update the checksum
    csum = csum & 0xff;
    flashVars.CSUM = csum;
    flashVarsMem.write(flashVars); // Write the flash variables
    LED_green();
    while (switchState != 0x00) {switchState = readSwitchState();} // Wait for the switches to be released
    LED_off();

  }
  if (flashVars.GONE == true) // If the GONE flag is true, set loop_set to init (not init_GPS)
  {
    loop_step = init;
  }
}

int readSwitchState()
{
  int switchState = 0;
  if (digitalRead(closeSwitchA) == LOW) switchState |= 0x01; // Read switch state
  if (digitalRead(openSwitchA) == LOW) switchState |= 0x02;
  if (digitalRead(closeSwitchB) == LOW) switchState |= 0x04;
  if (digitalRead(openSwitchB) == LOW) switchState |= 0x08;
  return (switchState);  
}

void loop()
{
  unsigned long loopStartTime = millis();

  switch(loop_step) {

    case setServoA:
    {
      // Power up the servo
      digitalWrite(enable5V, FIVE_V_ON);
      digitalWrite(ServoA, SERVO_ON);

      bool decOne = false; // Flag to indicate that the PWM should be decremented by one
      bool incOne = false; // Flag to indicate that the PWM should be incremented by one

      int plusSwitchState = digitalRead(closeSwitchB); // Read the plus switch state
      int minusSwitchState = digitalRead(openSwitchB); // Read the minus switch state
    
      if ((plusSwitchState == 0) and (minusSwitchState == 1)) {
        // If only the plusSwitch is low (pressed) decrement the PWM value
        decOne = true;
      }
      else if ((plusSwitchState == 1) and (minusSwitchState == 0)) {
        // If only the minusSwitch is low (pressed) increment the PWM value
        incOne = true;
      }
      // If neither or both switches are pressed, do nothing
      
      if ((incOne == true) || (decOne == true)) {
        // If either switch was pressed
        if (incOne == true) {
          if (PWM < maxServo) {
            PWM = PWM + 1;
          }
        }
        else {
          if (PWM > minServo) {
            PWM = PWM - 1;
          }
        }
      }

      // Update the servo PWM
      REG_TCC0_CCB1 = PWM; // TCC0 CCB1 - change the servo PWM on D5
      while(TCC0->SYNCBUSY.bit.CCB1);

      // Wait (blocking!!)
      delay(10);
        
      bool updateFlash = false; // Flag to indicate that the flash storage should be updated

      int openSwitchState = digitalRead(openSwitchA); // Read the open switch state
      int closeSwitchState = digitalRead(closeSwitchA); // Read the close switch state
    
      if ((openSwitchState == 0) and (closeSwitchState == 1)) {
        // If only the openSwitch is low (pressed) update servoOpen
        updateFlash = true;
        servoOpenA = PWM;
      }
      else if ((openSwitchState == 1) and (closeSwitchState == 0)) {
        // If only the closeSwitch is low (pressed) update servoClose
        updateFlash = true;
        servoClosedA = PWM;
      }
      // If neither or both switches are pressed, do nothing
        
      if (updateFlash == true) {
        // If either switch was pressed, update flash memory
        flashVars.PREFIX = 0xB5; // Initialise the prefix (hopefully redundant!)
        flashVars.servoOpenA = servoOpenA; // Update the ServoA settings
        flashVars.servoClosedA = servoClosedA;
        int csum = flashVars.PREFIX + flashVars.INTERVAL + flashVars.RBSOURCE + flashVars.RBDESTINATION + flashVars.servoOpenA + flashVars.servoClosedA + flashVars.servoOpenB + flashVars.servoClosedB + flashVars.GONE + flashVars.SOUNDER; // Update the checksum
        csum = csum & 0xff;
        flashVars.CSUM = csum;
        flashVarsMem.write(flashVars); // Write the flash variables

        LED_green();
        delay(2000); // Wait (blocking!)
        LED_cyan();
        
        // Wait until both open and close switches have been released
        while ((openSwitchState == 0) || (openSwitchState == 0)) {
          openSwitchState = digitalRead(openSwitchA); // Read the open switch state
          closeSwitchState = digitalRead(closeSwitchA); // Read the close switch state
        }
      }
    }
    break;

    case setServoB:
    {
      // Power up the servo
      digitalWrite(enable5V, FIVE_V_ON);
      digitalWrite(ServoB, SERVO_ON);

      bool decOne = false; // Flag to indicate that the PWM should be decremented by one
      bool incOne = false; // Flag to indicate that the PWM should be incremented by one

      int plusSwitchState = digitalRead(closeSwitchA); // Read the plus switch state
      int minusSwitchState = digitalRead(openSwitchA); // Read the minus switch state
    
      if ((plusSwitchState == 0) and (minusSwitchState == 1)) {
        // If only the plusSwitch is low (pressed) decrement the PWM value
        decOne = true;
      }
      else if ((plusSwitchState == 1) and (minusSwitchState == 0)) {
        // If only the minusSwitch is low (pressed) increment the PWM value
        incOne = true;
      }
      // If neither or both switches are pressed, do nothing
      
      if ((incOne == true) || (decOne == true)) {
        // If either switch was pressed
        if (incOne == true) {
          if (PWM < maxServo) {
            PWM = PWM + 1;
          }
        }
        else {
          if (PWM > minServo) {
            PWM = PWM - 1;
          }
        }
      }

      // Update the servo PWM
      REG_TCC0_CCB1 = PWM; // TCC0 CCB1 - change the servo PWM on D5
      while(TCC0->SYNCBUSY.bit.CCB1);

      // Wait (blocking!!)
      delay(10);
        
      bool updateFlash = false; // Flag to indicate that the flash storage should be updated

      int openSwitchState = digitalRead(openSwitchB); // Read the open switch state
      int closeSwitchState = digitalRead(closeSwitchB); // Read the close switch state
    
      if ((openSwitchState == 0) and (closeSwitchState == 1)) {
        // If only the openSwitch is low (pressed) update servoOpen
        updateFlash = true;
        servoOpenB = PWM;
      }
      else if ((openSwitchState == 1) and (closeSwitchState == 0)) {
        // If only the closeSwitch is low (pressed) update servoClose
        updateFlash = true;
        servoClosedB = PWM;
      }
      // If neither or both switches are pressed, do nothing
        
      if (updateFlash == true) {
        // If either switch was pressed, update flash memory
        flashVars.PREFIX = 0xB5; // Initialise the prefix (hopefully redundant!)
        flashVars.servoOpenB = servoOpenB; // Update the ServoB settings
        flashVars.servoClosedB = servoClosedB;
        int csum = flashVars.PREFIX + flashVars.INTERVAL + flashVars.RBSOURCE + flashVars.RBDESTINATION + flashVars.servoOpenA + flashVars.servoClosedA + flashVars.servoOpenB + flashVars.servoClosedB + flashVars.GONE + flashVars.SOUNDER; // Update the checksum
        csum = csum & 0xff;
        flashVars.CSUM = csum;
        flashVarsMem.write(flashVars); // Write the flash variables
        
        LED_green();
        delay(2000); // Wait (blocking!)
        LED_cyan();
        
        // Wait until both open and close switches have been released
        while ((openSwitchState == 0) || (openSwitchState == 0)) {
          openSwitchState = digitalRead(openSwitchB); // Read the open switch state
          closeSwitchState = digitalRead(closeSwitchB); // Read the close switch state
        }
      }
    }
    break;

    case init_GPS:

#ifndef NoLED
      LED_blue(); // Set LED to Blue
#endif
    
      // Start the serial console
      Serial.begin(115200);
      delay(10000); // Wait 10 secs - allow time for user to open serial monitor
    
      // Send welcome message
      Serial.println("Data Recovery System V1");

      // Echo the BEACON_INTERVAL
      Serial.print("Using a BEACON_INTERVAL of ");
      Serial.print(BEACON_INTERVAL);
      Serial.println(" minutes");

      // Echo RBDESTINATION and RBSOURCE
      Serial.print("Using an RBDESTINATION of ");
      Serial.println(RBDESTINATION);
      Serial.print("Using an RBSOURCE of ");
      Serial.println(RBSOURCE);

      // Echo the servo settings
      Serial.print("servoOpenA : ");
      Serial.println(servoOpenA);
      Serial.print("servoClosedA : ");
      Serial.println(servoClosedA);
      Serial.print("servoOpenB : ");
      Serial.println(servoOpenB);
      Serial.print("servoClosedB : ");
      Serial.println(servoClosedB);

      // Power up the GNSS
      Serial.println("Powering up the GNSS...");
      digitalWrite(GPS_EN, GPS_ON); // Enable the GNSS

      delay(2000); // Allow time for GNSS to start
    
      // Start the GPS serial port
      ssGPS.begin(9600);
      // Start the Pi port
      ssPi.begin(9600);

      delay(1000); // Allow time for the ports to open

      // Configure GPS
      Serial.println("Configuring GPS...");

      // Clear the MAX_M8Q configuration
      sendUBX(clearConf, len_Conf); // Clear stored configuration
      delay(100);
      sendUBX(loadConf, len_Conf); // Load configuration
      delay(3000);

      // Disable all messages except GGA and RMC
      ssGPS.println("$PUBX,40,GLL,0,0,0,0*5C"); // Disable GLL
      delay(100);
      ssGPS.println("$PUBX,40,ZDA,0,0,0,0*44"); // Disable ZDA
      delay(100);
      ssGPS.println("$PUBX,40,VTG,0,0,0,0*5E"); // Disable VTG
      delay(100);
      ssGPS.println("$PUBX,40,GSV,0,0,0,0*59"); // Disable GSV
      delay(100);
      ssGPS.println("$PUBX,40,GSA,0,0,0,0*4E"); // Disable GSA
      delay(1100);
      
      //sendUBX(setNavPortable, len_setNav); // Set Portable Navigation Mode
      sendUBX(setNavAir, len_setNav); // Set Airborne <1G Navigation Mode
      delay(1100);

      sendUBX(setNMEA, len_setNMEA); // Set NMEA: to always output COG; and set main talker to GP (instead of GN)
      delay(1100);

#ifdef GALILEO
      sendUBX(setGNSS, len_setGNSS); // Set GNSS - causes M8 to restart!
      delay(3000); // Wait an extra time for GNSS to restart
#endif

      // Save the MAX_M8Q configuration
      sendUBX(saveConf, len_Conf); // Save configuration
      delay(100);

      while(ssGPS.available()){ssGPS.read();} // Flush RX buffer so we don't try to transmit UBX acknowledgements

      loop_step = pi_echo;

      break;

    case pi_echo:

#ifndef NoLED
      // 'Flash' the LED
      if ((millis() / 1000) % 2 == 1) {
        LED_dim_blue();
      }
      else {
        LED_blue();
      }
#endif
      while(ssGPS.available()) // Echo GNSS messages to the Pi and the console
      {
        char c = ssGPS.read();
        ssPi.write(c);
        Serial.write(c);
      }
      recvPiWithEndMarker(); // Check for a command from the Pi
      if (newPiData)
      {
        String pi_str = String((char *)receivedPiChars); // Convert message into a String
        int starts_at = -1;
        starts_at = pi_str.indexOf("GoGoGo"); // See if message contains "GoGoGo"
        if (starts_at >= 0) { // If it does:
          loop_step = drop; // Drop now!
        }
        newPiData = false;
      }
      recvWithEndMarker(); // Check for a command from the console
      if (newData)
      {
        String pi_str = String((char *)receivedChars); // Convert message into a String
        int starts_at = -1;
        starts_at = pi_str.indexOf("GoGoGo"); // See if message contains "GoGoGo"
        if (starts_at >= 0) { // If it does:
          loop_step = drop; // Drop now!
        }
        starts_at = pi_str.indexOf("CLOSE_A"); // See if message contains "CLOSE_A"
        if (starts_at >= 0) { // If it does:
          loop_step = close_a;
        }
        starts_at = pi_str.indexOf("OPEN_A"); // See if message contains "OPEN_A"
        if (starts_at >= 0) { // If it does:
          loop_step = open_a;
        }
        starts_at = pi_str.indexOf("CLOSE_B"); // See if message contains "CLOSE_B"
        if (starts_at >= 0) { // If it does:
          loop_step = close_b;
        }
        starts_at = pi_str.indexOf("OPEN_B"); // See if message contains "OPEN_B"
        if (starts_at >= 0) { // If it does:
          loop_step = open_b;
        }
        starts_at = pi_str.indexOf("SOUNDER_ON"); // See if message contains "SOUNDER_ON"
        if (starts_at >= 0) { // If it does:
          PORT->Group[PORTA].PINCFG[8].reg |= PORT_PINCFG_PMUXEN; // Enable pin multiplexer
        }
        starts_at = pi_str.indexOf("SOUNDER_OFF"); // See if message contains "SOUNDER_OFF"
        if (starts_at >= 0) { // If it does:
          PORT->Group[PORTA].PINCFG[8].reg &= ~PORT_PINCFG_PMUXEN; // Disable pin multiplexer
        }
        newData = false;
      }
      
      break;

    case open_a:
    
      // Set the PWM for servoOpenA
      PWM = servoOpenA;
      // Update the servo PWM
      REG_TCC0_CCB1 = PWM; // TCC0 CCB1 - change the servo PWM on D5
      while(TCC0->SYNCBUSY.bit.CCB1);
      // Enable 5V
      Serial.println("Enabling the 5V supply...");
      digitalWrite(enable5V, FIVE_V_ON); // Enable 5V
      delay(1000);
      // Enable ServoA
      Serial.println("Enabling ServoA...");
      digitalWrite(ServoA, SERVO_ON); // Enable ServoA
      delay(3000);
      // Disable ServoA
      Serial.println("Disabling ServoA...");
      digitalWrite(ServoA, SERVO_OFF); // Disable ServoA
      delay(100);
      // Disable 5V
      Serial.println("Disabling the 5V supply...");
      digitalWrite(enable5V, FIVE_V_OFF); // Disable 5V
      delay(100);
      // Set the PWM to servoClosedB just in case
      PWM = servoClosedB;
      // Update the servo PWM
      REG_TCC0_CCB1 = PWM; // TCC0 CCB1 - change the servo PWM on D5
      while(TCC0->SYNCBUSY.bit.CCB1);

      loop_step = pi_echo;

      break;

    case close_a:
    
      // Set the PWM for servoClosedA
      PWM = servoClosedA;
      // Update the servo PWM
      REG_TCC0_CCB1 = PWM; // TCC0 CCB1 - change the servo PWM on D5
      while(TCC0->SYNCBUSY.bit.CCB1);
      // Enable 5V
      Serial.println("Enabling the 5V supply...");
      digitalWrite(enable5V, FIVE_V_ON); // Enable 5V
      delay(1000);
      // Enable ServoA
      Serial.println("Enabling ServoA...");
      digitalWrite(ServoA, SERVO_ON); // Enable ServoA
      delay(3000);
      // Disable ServoA
      Serial.println("Disabling ServoA...");
      digitalWrite(ServoA, SERVO_OFF); // Disable ServoA
      delay(100);
      // Disable 5V
      Serial.println("Disabling the 5V supply...");
      digitalWrite(enable5V, FIVE_V_OFF); // Disable 5V
      delay(100);
      // Set the PWM to servoClosedB just in case
      PWM = servoClosedB;
      // Update the servo PWM
      REG_TCC0_CCB1 = PWM; // TCC0 CCB1 - change the servo PWM on D5
      while(TCC0->SYNCBUSY.bit.CCB1);

      loop_step = pi_echo;

      break;

    case open_b:
    
      // Set the PWM for servoOpenB
      PWM = servoOpenB;
      // Update the servo PWM
      REG_TCC0_CCB1 = PWM; // TCC0 CCB1 - change the servo PWM on D5
      while(TCC0->SYNCBUSY.bit.CCB1);
      // Enable 5V
      Serial.println("Enabling the 5V supply...");
      digitalWrite(enable5V, FIVE_V_ON); // Enable 5V
      delay(1000);
      // Enable ServoB
      Serial.println("Enabling ServoB...");
      digitalWrite(ServoB, SERVO_ON); // Enable ServoB
      delay(3000);
      // Disable ServoB
      Serial.println("Disabling ServoB...");
      digitalWrite(ServoB, SERVO_OFF); // Disable ServoB
      delay(100);
      // Disable 5V
      Serial.println("Disabling the 5V supply...");
      digitalWrite(enable5V, FIVE_V_OFF); // Disable 5V
      delay(100);
      // Set the PWM to servoClosedB just in case
      PWM = servoClosedB;
      // Update the servo PWM
      REG_TCC0_CCB1 = PWM; // TCC0 CCB1 - change the servo PWM on D5
      while(TCC0->SYNCBUSY.bit.CCB1);

      loop_step = pi_echo;

      break;

    case close_b:
    
      // Set the PWM for servoClosedB
      PWM = servoClosedB;
      // Update the servo PWM
      REG_TCC0_CCB1 = PWM; // TCC0 CCB1 - change the servo PWM on D5
      while(TCC0->SYNCBUSY.bit.CCB1);
      // Enable 5V
      Serial.println("Enabling the 5V supply...");
      digitalWrite(enable5V, FIVE_V_ON); // Enable 5V
      delay(1000);
      // Enable ServoB
      Serial.println("Enabling ServoB...");
      digitalWrite(ServoB, SERVO_ON); // Enable ServoB
      delay(3000);
      // Disable ServoB
      Serial.println("Disabling ServoB...");
      digitalWrite(ServoB, SERVO_OFF); // Disable ServoB
      delay(100);
      // Disable 5V
      Serial.println("Disabling the 5V supply...");
      digitalWrite(enable5V, FIVE_V_OFF); // Disable 5V
      delay(100);

      loop_step = pi_echo;

      break;

    case drop:
      {

#ifndef NoLED
      LED_magenta(); // Set LED to Magenta
#endif

      // Countdown
      int tminus = countdown;
      while (tminus >= 0)
      {
        Serial.print("Dropping in ");
        Serial.print(tminus);
        Serial.println(" seconds!");
        ssPi.print("Dropping in ");
        ssPi.print(tminus);
        ssPi.println(" seconds!");
        delay(1000);
        tminus--;
      }
      
      // Power down the GNSS
      Serial.println("Powering down the GNSS...");
      digitalWrite(GPS_EN, GPS_OFF); // Disable the GNSS
      delay(100);

      // Set the PWM for servoOpenA
      PWM = servoOpenA;
      // Update the servo PWM
      REG_TCC0_CCB1 = PWM; // TCC0 CCB1 - change the servo PWM on D5
      while(TCC0->SYNCBUSY.bit.CCB1);
      // Enable 5V
      Serial.println("Enabling the 5V supply...");
      digitalWrite(enable5V, FIVE_V_ON); // Enable 5V
      delay(1000);
      // Enable ServoA
      Serial.println("Opening ServoA...");
      digitalWrite(ServoA, SERVO_ON); // Enable ServoA
      delay(3000);
      // Disable ServoA
      Serial.println("Disabling ServoA...");
      digitalWrite(ServoA, SERVO_OFF); // Disable ServoA
      delay(100);
      // Disable 5V
      Serial.println("Disabling the 5V supply...");
      digitalWrite(enable5V, FIVE_V_OFF); // Disable 5V
      delay(100);
      // Set the PWM to servoClosedB just in case
      PWM = servoClosedB;
      // Update the servo PWM
      REG_TCC0_CCB1 = PWM; // TCC0 CCB1 - change the servo PWM on D5
      while(TCC0->SYNCBUSY.bit.CCB1);

      // Set the GONE flag
      flashVars.PREFIX = 0xB5; // Initialise the prefix (hopefully redundant!)
      flashVars.GONE = true; // Set the GONE flag
      int csum = flashVars.PREFIX + flashVars.INTERVAL + flashVars.RBSOURCE + flashVars.RBDESTINATION + flashVars.servoOpenA + flashVars.servoClosedA + flashVars.servoOpenB + flashVars.servoClosedB + flashVars.GONE + flashVars.SOUNDER; // Update the checksum
      csum = csum & 0xff;
      flashVars.CSUM = csum;
      flashVarsMem.write(flashVars); // Write the flash variables

      // Set loop_set to init
      loop_step = init;
      }

      break;

    case init:

#ifndef NoLED
      LED_magenta(); // Set LED to Magenta
#endif

      // Start the serial console
      Serial.begin(115200);
      //delay(10000); // Wait 10 secs - allow time for user to open serial monitor
    
      // Send welcome message
      Serial.println("Data Recovery System V1");

      // Echo the BEACON_INTERVAL
      Serial.print("Using a BEACON_INTERVAL of ");
      Serial.print(BEACON_INTERVAL);
      Serial.println(" minutes");
      
      // Echo RBDESTINATION and RBSOURCE
      Serial.print("Using an RBDESTINATION of ");
      Serial.println(RBDESTINATION);
      Serial.print("Using an RBSOURCE of ");
      Serial.println(RBSOURCE);

      // Enable the sounder
      if (flashVars.SOUNDER == true) {
        Serial.println("Enabling the sounder...");
        PORT->Group[PORTA].PINCFG[8].reg |= PORT_PINCFG_PMUXEN; // Enable sounder pin multiplexer
      }

      // Setup the IridiumSBD
      // (attachConsole and attachDiags methods have been replaced with ISBDConsoleCallback and ISBDDiagsCallback)
      isbd.setPowerProfile(IridiumSBD::DEFAULT_POWER_PROFILE); // Change power profile to default (redundant!)
      isbd.useMSSTMWorkaround(false); // Redundant?
      //isbd.adjustSendReceiveTimeout(60); // Adjust timeout

      // Check battery voltage
      // If voltage is low, go to sleep
      init_vbat(); // Use init_vbat to make sure the current true smoothed voltage is used (after coming out of deep sleep)
      if (vbat < VBAT_LOW) {
        Serial.print("***!!! LOW VOLTAGE (init) ");
        Serial.print(vbat,2);
        Serial.println(" !!!***");
        loop_step = zzz;
      }
      else {
        loop_step = start_GPS;
      }
      
      break;
      
    case start_GPS:

#ifndef NoLED
      LED_blue(); // Set LED to Blue
#endif
    
      // Power up the GNSS
      Serial.println("Powering up the GNSS...");
      digitalWrite(GPS_EN, GPS_ON); // Enable the GNSS

      delay(2000); // Allow time for GNSS to start
    
      // Check battery voltage now we are drawing current for the GPS
      // If voltage is low, go to sleep
      init_vbat(); // Use init_vbat to make sure the true smoothed voltage is used
      if (vbat < VBAT_LOW) {
        Serial.print("***!!! LOW VOLTAGE (start_GPS) ");
        Serial.print(vbat,2);
        Serial.println("V !!!***");
        loop_step = zzz;
      }
      else {
        loop_step = read_GPS;
      }
      
      break;

    case read_GPS:
      // Start the GPS serial port
      ssGPS.begin(9600);

      delay(1000); // Allow time for the port to open

      // Load configuration
      Serial.println("Loading GNSS configuration...");

      sendUBX(loadConf, len_Conf); // Load configuration
      delay(3000);

      while(ssGPS.available()){ssGPS.read();} // Flush RX buffer so we don't confuse TinyGPS with UBX acknowledgements

      // Reset TinyGPS and begin listening to the GPS
      Serial.println("Beginning to listen for GPS traffic...");
      fixFound = false; // Reset fixFound
      charsSeen = false; // Reset charsSeen
      tinygps = TinyGPS();
      
      // Look for GPS signal for up to 2 minutes
      for (tnow = millis(); !fixFound && millis() - tnow < 2UL * 60UL * 1000UL;)
      {
        if (ssGPS.available())
        {
          charsSeen = true;
          if (tinygps.encode(ssGPS.read()))
          {
            tinygps.f_get_position(&latitude, &longitude, &locationFix);
            tinygps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &dateFix);
            altitude = tinygps.altitude(); // Altitude in cm (long) - checks that we have received a GGA message
            speed = tinygps.f_speed_mps(); // Get speed - checks that we have received an RMC message
            satellites = tinygps.satellites(); // Get number of satellites
            course = tinygps.course(); // Get course over ground
            hdop = tinygps.hdop(); // Get horizontal dilution of precision
            fixFound = locationFix != TinyGPS::GPS_INVALID_FIX_TIME && 
                       dateFix != TinyGPS::GPS_INVALID_FIX_TIME && 
                       altitude != TinyGPS::GPS_INVALID_ALTITUDE &&
                       speed != TinyGPS::GPS_INVALID_F_SPEED &&
                       satellites != TinyGPS::GPS_INVALID_SATELLITES &&
                       course != TinyGPS::GPS_INVALID_ANGLE &&
                       hdop != TinyGPS::GPS_INVALID_HDOP &&
                       year != 2000;
          }
        }

        // if we haven't seen any GPS data in 10 seconds, then stop waiting
        if (!charsSeen && millis() - tnow > 10000) {
          break;
        }

        // Check battery voltage now we are drawing current for the GPS
        // If voltage is low, stop looking for GNSS and go to sleep
        get_vbat_smooth();
        if (vbat < VBAT_LOW) {
          break;
        }

#ifndef NoLED
        // 'Flash' the LED
        if ((millis() / 1000) % 2 == 1) {
          LED_dim_blue();
        }
        else {
          LED_blue();
        }
#endif

      }

      Serial.println(charsSeen ? fixFound ? F("A GPS fix was found!") : F("No GPS fix was found.") : F("Wiring error: No GPS data seen."));
      Serial.print("Latitude (degrees): "); Serial.println(latitude, 6);
      Serial.print("Longitude (degrees): "); Serial.println(longitude, 6);
      Serial.print("Altitude (m): "); Serial.println(altitude / 100); // Convert altitude from cm to m

      if (vbat < VBAT_LOW) {
        Serial.print("***!!! LOW VOLTAGE (read_GPS) ");
        Serial.print(vbat,2);
        Serial.println("V !!!***");
        loop_step = zzz;
      }
      else if (!charsSeen) {
        Serial.println("***!!! No GPS data received !!!***");
        loop_step = zzz;
      }
      else {
        loop_step = start_9603;
      }
      
      break;

    case start_9603:

#ifndef NoLED
      LED_white(); // Set LED to White
#endif

      // Power down the GNSS
      Serial.println("Powering down the GNSS...");
      digitalWrite(GPS_EN, GPS_OFF); // Disable the GNSS
      delay(100);

      // Enable 5V
      Serial.println("Enabling the 5V supply...");
      digitalWrite(enable5V, FIVE_V_ON); // Enable 5V
      delay(1000);

      // Enable power for the 9603N
      Serial.println("Enabling power for the 9603N...");
      digitalWrite(Enable_9603N, IRIDIUM_ON); // Enable 9603N
      delay(2000);

      // Start talking to the 9603 and power it up
      Serial.println("Beginning to talk to the 9603...");

      ssIridium.begin(19200);
      delay(1000);

      if (isbd.begin() == ISBD_SUCCESS) // isbd.begin switches the 9603 on
      {
        char outBuffer[120]; // Always try to keep message short (maximum should be ~101 chars including RockBLOCK destination and source)
    
        if (fixFound)
        {
          if (RBDESTINATION > 0) {
            sprintf(outBuffer, "RB%07d,%d%02d%02d%02d%02d%02d,", RBDESTINATION, year, month, day, hour, minute, second);
          }
          else {
            sprintf(outBuffer, "%d%02d%02d%02d%02d%02d,", year, month, day, hour, minute, second);
          }
          int len = strlen(outBuffer);
          PString str(outBuffer + len, sizeof(outBuffer) - len);
          str.print(latitude, 6);
          str.print(",");
          str.print(longitude, 6);
          str.print(",");
          str.print(altitude / 100); // Convert altitude from cm to m
          str.print(",");
          str.print(speed, 1); // Speed in metres per second
          str.print(",");
          str.print(course / 100); // Convert from 1/100 degree to degrees
          str.print(",");
          str.print((((float)hdop) / 100),1); // Convert from 1/100 m to m
          str.print(",");
          str.print(satellites);
          str.print(",");
          str.print("0,"); // Set pressure to zero
          str.print("0.0,"); // Set temperature to zero
          str.print(vbat, 2);
          str.print(",");
          str.print(float(iterationCounter), 0);
          if (RBDESTINATION > 0) { // Append source RockBLOCK serial number (as text) to the end of the message
            char sourceBuffer[12];
            sprintf(sourceBuffer, "RB%07d", RBSOURCE);
            str.print(",");
            str.print(sourceBuffer);
          }
        }
    
        else
        {
          // No GPS fix found!
          hour = 99; // Make hour and minute invalid for PARACHUTE checking
          minute = 99;
          if (RBDESTINATION > 0) {
            sprintf(outBuffer, "RB%07d,19700101000000,0.0,0.0,0,0.0,0,0.0,0,", RBDESTINATION);
          }
          else {
            sprintf(outBuffer, "19700101000000,0.0,0.0,0,0.0,0,0.0,0,");
          }
          int len = strlen(outBuffer);
          PString str(outBuffer + len, sizeof(outBuffer) - len);
          str.print("0,"); // Set pressure to zero
          str.print("0.0,"); // Set temperature to zero
          str.print(vbat, 2);
          str.print(",");
          str.print(float(iterationCounter), 0);
          if (RBDESTINATION > 0) { // Append source RockBLOCK serial number (as text) to the end of the message
            char sourceBuffer[12];
            sprintf(sourceBuffer, "RB%07d", RBSOURCE);
            str.print(",");
            str.print(sourceBuffer);
          }
        }

        Serial.print("Transmitting message '");
        Serial.print(outBuffer);
        Serial.println("'");
        uint8_t mt_buffer[100]; // Buffer to store Mobile Terminated SBD message
        size_t mtBufferSize = sizeof(mt_buffer); // Size of MT buffer
        init_vbat(); // init_vbat to make sure the current true smoothed voltage is used now that the 9603N is active

        if (isbd.sendReceiveSBDText(outBuffer, mt_buffer, mtBufferSize) == ISBD_SUCCESS) { // Send the message; download an MT message if there is one
          if (mtBufferSize > 0) { // Was an MT message received?
            // Check message content
            mt_buffer[mtBufferSize] = 0; // Make sure message is NULL terminated
            String mt_str = String((char *)mt_buffer); // Convert message into a String
            Serial.print("Received a MT message: "); Serial.println(mt_str);

            // Check if the message contains a correctly formatted BEACON_INTERVAL: "[INTERVAL=nnn]"
            int new_interval = 0;
            int starts_at = -1;
            int ends_at = -1;
            starts_at = mt_str.indexOf("[INTERVAL="); // See if message contains "[INTERVAL="
            if (starts_at >= 0) { // If it does:
              ends_at = mt_str.indexOf("]", starts_at); // Find the following "]"
              if (ends_at > starts_at) { // If the message contains both "[INTERVAL=" and "]"
                String new_interval_str = mt_str.substring((starts_at + 10),ends_at); // Extract the value after the "="
                Serial.print("Extracted an INTERVAL of: "); Serial.println(new_interval_str);
                new_interval = (int)new_interval_str.toInt(); // Convert it to int
              }
            }
            if ((new_interval > 0) and (new_interval <= 1440)) { // Check new interval is valid
              Serial.print("New BEACON_INTERVAL received. Setting BEACON_INTERVAL to ");
              Serial.print(new_interval);
              Serial.println(" minutes.");
              BEACON_INTERVAL = new_interval; // Update BEACON_INTERVAL
              // Update flash memory
              flashVars.PREFIX = 0xB5; // Reset the prefix (hopefully redundant!)
              flashVars.INTERVAL = new_interval; // Store the new beacon interval
              int csum = flashVars.PREFIX + flashVars.INTERVAL + flashVars.RBSOURCE + flashVars.RBDESTINATION + flashVars.servoOpenA + flashVars.servoClosedA + flashVars.servoOpenB + flashVars.servoClosedB + flashVars.GONE + flashVars.SOUNDER; // Update the checksum
              csum = csum & 0xff;
              flashVars.CSUM = csum;
              flashVarsMem.write(flashVars); // Write the flash variables
            }

            // Check if the message contains a correctly formatted RBSOURCE: "[RBSOURCE=nnnnn]"
            int new_source = -1;
            starts_at = -1;
            ends_at = -1;
            starts_at = mt_str.indexOf("[RBSOURCE="); // See is message contains "[RBSOURCE="
            if (starts_at >= 0) { // If it does:
              ends_at = mt_str.indexOf("]", starts_at); // Find the following "]"
              if (ends_at > starts_at) { // If the message contains both "[RBSOURCE=" and "]"
                String new_source_str = mt_str.substring((starts_at + 10),ends_at); // Extract the value after the "="
                Serial.print("Extracted an RBSOURCE of: "); Serial.println(new_source_str);
                new_source = (int)new_source_str.toInt(); // Convert it to int
              }
            }
            // toInt returns zero if the conversion fails, so it is not possible to distinguish between a source of zero and an invalid value!
            // An invalid value will cause RBSOURCE to be set to zero
            if (new_source >= 0) { // If new_source was received
              Serial.print("New RBSOURCE received. Setting RBSOURCE to ");
              Serial.println(new_source);
              RBSOURCE = new_source; // Update RBSOURCE
              // Update flash memory
              flashVars.PREFIX = 0xB5; // Reset the prefix (hopefully redundant!)
              flashVars.RBSOURCE = new_source; // Store the new RockBLOCK source serial number
              int csum = flashVars.PREFIX + flashVars.INTERVAL + flashVars.RBSOURCE + flashVars.RBDESTINATION + flashVars.servoOpenA + flashVars.servoClosedA + flashVars.servoOpenB + flashVars.servoClosedB + flashVars.GONE + flashVars.SOUNDER; // Update the checksum
              csum = csum & 0xff;
              flashVars.CSUM = csum;
              flashVarsMem.write(flashVars); // Write the flash variables
            }

            // Check if the message contains a correctly formatted RBDESTINATION: "[RBDESTINATION=nnnnn]"
            int new_destination = -1;
            starts_at = -1;
            ends_at = -1;
            starts_at = mt_str.indexOf("[RBDESTINATION="); // See is message contains "[RBDESTINATION="
            if (starts_at >= 0) { // If it does:
              ends_at = mt_str.indexOf("]", starts_at); // Find the following "]"
              if (ends_at > starts_at) { // If the message contains both "[RBDESTINATION=" and "]"
                String new_destination_str = mt_str.substring((starts_at + 15),ends_at); // Extract the value after the "="
                Serial.print("Extracted an RBDESTINATION of: "); Serial.println(new_destination_str);
                new_destination = (int)new_destination_str.toInt(); // Convert it to int
              }
            }
            // toInt returns zero if the conversion fails, so it is not possible to distinguish between a destination of zero and an invalid value!
            // An invalid value will cause RBDESTINATION to be set to zero
            if (new_destination >= 0) { // If new_destination was received
              Serial.print("New RBDESTINATION received. Setting RBDESTINATION to ");
              Serial.println(new_destination);
              RBDESTINATION = new_destination; // Update RBDESTINATION
              // Update flash memory
              flashVars.PREFIX = 0xB5; // Reset the prefix (hopefully redundant!)
              flashVars.RBDESTINATION = new_destination; // Store the new RockBLOCK destination serial number
              int csum = flashVars.PREFIX + flashVars.INTERVAL + flashVars.RBSOURCE + flashVars.RBDESTINATION + flashVars.servoOpenA + flashVars.servoClosedA + flashVars.servoOpenB + flashVars.servoClosedB + flashVars.GONE + flashVars.SOUNDER; // Update the checksum
              csum = csum & 0xff;
              flashVars.CSUM = csum;
              flashVarsMem.write(flashVars); // Write the flash variables
            }

            // Check if the message contains a correctly formatted PARACHUTE command: "[PARACHUTE=hh:mm]"
            int parachute_hh = -1;
            int parachute_mm = -1;
            starts_at = -1;
            ends_at = -1;
            starts_at = mt_str.indexOf("[PARACHUTE="); // See is message contains "[PARACHUTE="
            if (starts_at >= 0) { // If it does:
              ends_at = mt_str.indexOf(":", starts_at); // Find the following ":"
              if (ends_at > starts_at) { // If the message contains both "[PARACHUTE=" and ":"
                String new_hh_str = mt_str.substring((starts_at + 11),ends_at); // Extract the value after the "="
                Serial.print("Extracted a PARACHUTE hh of: "); Serial.println(new_hh_str);
                parachute_hh = (int)new_hh_str.toInt(); // Convert it to int
              }
              starts_at = ends_at; // Point to the :
              ends_at = mt_str.indexOf("]", starts_at); // Find the following "]"
              if (ends_at > starts_at) { // If the message contains "[PARACHUTE=" and ":" and "]"
                String new_mm_str = mt_str.substring((starts_at + 1),ends_at); // Extract the value after the ":"
                Serial.print("Extracted a PARACHUTE mm of: "); Serial.println(new_mm_str);
                parachute_mm = (int)new_mm_str.toInt(); // Convert it to int
              }
            }
            // toInt returns zero if the conversion fails, so it is not possible to distinguish between a time of 00:00 and an invalid value!
            if ((parachute_hh >= 0) && (parachute_mm >= 0)){ // If new parachute time was received
              int parachute_time = (parachute_hh * 60) + parachute_mm;
              int gnss_time = (((int)hour) * 60) + ((int)minute);
              //Serial.print("Parachute time in minutes is: "); Serial.println(parachute_time);
              //Serial.print("GNSS time in minutes is: "); Serial.println(gnss_time);
              if (((parachute_time > (gnss_time - 15)) && (parachute_time < (gnss_time + 15)))
                || ((parachute_time > 1425) && (gnss_time < 15)) || ((gnss_time > 1425) && (parachute_time < 15)))
              {
                Serial.println("Valid PARACHUTE command received!");
                open_servo_B_flag = true;
              }
            }
            
            // Check if the message contains a correctly formatted SOUNDER_OFF command: "[SOUNDER_OFF]"
            starts_at = -1;
            starts_at = mt_str.indexOf("[SOUNDER_OFF]"); // See is message contains "[SOUNDER_OFF]"
            if (starts_at >= 0) { // If it does:
              Serial.print("SOUNDER_OFF received.");
              // Update flash memory
              flashVars.PREFIX = 0xB5; // Reset the prefix (hopefully redundant!)
              flashVars.SOUNDER = false; // Update the SOUNDER flag
              int csum = flashVars.PREFIX + flashVars.INTERVAL + flashVars.RBSOURCE + flashVars.RBDESTINATION + flashVars.servoOpenA + flashVars.servoClosedA + flashVars.servoOpenB + flashVars.servoClosedB + flashVars.GONE + flashVars.SOUNDER; // Update the checksum
              csum = csum & 0xff;
              flashVars.CSUM = csum;
              flashVarsMem.write(flashVars); // Write the flash variables
              Serial.println("Disabling the sounder...");
              PORT->Group[PORTA].PINCFG[8].reg &= ~PORT_PINCFG_PMUXEN; // Disable sounder pin multiplexer
            }
            
            // Check if the message contains a correctly formatted SOUNDER_ON command: "[SOUNDER_ON]"
            starts_at = -1;
            starts_at = mt_str.indexOf("[SOUNDER_ON]"); // See is message contains "[SOUNDER_ON]"
            if (starts_at >= 0) { // If it does:
              Serial.print("SOUNDER_ON received.");
              // Update flash memory
              flashVars.PREFIX = 0xB5; // Reset the prefix (hopefully redundant!)
              flashVars.SOUNDER = true; // Update the SOUNDER flag
              int csum = flashVars.PREFIX + flashVars.INTERVAL + flashVars.RBSOURCE + flashVars.RBDESTINATION + flashVars.servoOpenA + flashVars.servoClosedA + flashVars.servoOpenB + flashVars.servoClosedB + flashVars.GONE + flashVars.SOUNDER; // Update the checksum
              csum = csum & 0xff;
              flashVars.CSUM = csum;
              flashVarsMem.write(flashVars); // Write the flash variables
              Serial.println("Enabling the sounder...");
              PORT->Group[PORTA].PINCFG[8].reg |= PORT_PINCFG_PMUXEN; // Enable sounder pin multiplexer
            }
            
          }
          
#ifndef NoLED
          LED_green(); // Set LED to Green for 2 seconds
          delay(2000);
#endif
        }
        ++iterationCounter; // Increment iterationCounter (regardless of whether send was successful)
      }
      
      loop_step = zzz;

      break;

    case zzz:
    
#ifndef NoLED
      LED_red(); // Set LED to Red
#endif

      // Get ready for sleep
      Serial.println("Putting 9603N and GNSS to sleep...");
      isbd.sleep(); // Put 9603 to sleep
      delay(1000);
      ssIridium.end(); // Close GPS, Iridium and eRIC serial ports
      ssGPS.end();
      ssPi.end();
      delay(1000); // Wait for serial ports to clear
  
      // Disable: GPS and 9603N
      digitalWrite(GPS_EN, GPS_OFF); // Disable the GNSS (should be off already!)
      digitalWrite(Enable_9603N, IRIDIUM_OFF); // Disable the 9603N

      // Now that power draw has been minimised, open ServoB if required
      if (open_servo_B_flag == true) {
        Serial.println("Opening ServoB...");

        // Set the PWM for servoOpenB
        PWM = servoOpenB;
        // Update the servo PWM
        REG_TCC0_CCB1 = PWM; // TCC0 CCB1 - change the servo PWM on D5
        while(TCC0->SYNCBUSY.bit.CCB1);
        // Enable ServoB
        Serial.println("Enabling ServoB...");
        digitalWrite(ServoB, SERVO_ON); // Enable ServoB
        delay(3000);
        // Disable ServoB
        Serial.println("Disabling ServoB...");
        digitalWrite(ServoB, SERVO_OFF); // Disable ServoB
        delay(100);
        
        open_servo_B_flag = false;
      }

      // Disable 5V
      Serial.println("Disabling the 5V supply...");
      digitalWrite(enable5V, FIVE_V_OFF); // Disable 5V
      delay(100);

      // Turn LED off
      LED_off();
  
      // Close and detach the serial console (as per CaveMoa's SimpleSleepUSB)
      Serial.println("Going to sleep until next alarm time...");
      delay(1000); // Wait for serial port to clear
      Serial.end(); // Close the serial console
      USBDevice.detach(); // Safely detach the USB prior to sleeping

      // Speed up TCC1 so sounder beeps remain constant during sleep
      REG_TCC1_COUNT = 0;
      while(TCC1->SYNCBUSY.bit.COUNT);
      REG_TCC1_PER = 19531;
      while(TCC1->SYNCBUSY.bit.PER);
      REG_TCC1_CCB0 = 18555;
      while(TCC1->SYNCBUSY.bit.CCB0);

      // Sleep until next alarm match
      rtc.standbyMode();

      // Slow TTC1 down again
      REG_TCC1_PER = 39062;      // Set the frequency of the PWM on TCC1 to 0.2Hz
      while(TCC1->SYNCBUSY.bit.PER);
      REG_TCC1_CCB0 = 37109;       // TCC1 CCB2 - set sounder duty cycle (5:95)
      while(TCC1->SYNCBUSY.bit.CCB0);

      // Wake up!
      loop_step = wake;
  
      break;

    case wake:
      // Attach and reopen the serial console
      USBDevice.attach(); // Re-attach the USB
      delay(1000);  // Delay added to make serial more reliable

      // Now loop back to init
      loop_step = init;

      break;
  }
}
