// ################################
// # Data Recovery System V1 Test #
// ################################

// This code tests the components on V1 of the Data Recovery System PCB

// Power the PCB via USB and - if possible - monitor the current draw at the same time

// The tests won't start until the Serial Monitor is opened

// Don't connect the Raspberry Pi for these tests - the Pi will have its own test program
// Connect the DRS 3V3 line (next to the NeoPixel) to pin 1 of the Raspberry Pi header (marked with "J8")
// Some of the tests will require you to apply power from the two 9V battery connectors

// ATSAMD21G18A Pin Allocations:
//
// D0  : Serial1 RX (GNSS)
// D1  : Serial1 TX (GNSS)
// D2  : Servo B power switching
// D4 / PA08 / TCC1WO2 : Sounder enable
// D5 / PA15 / TCC0WO5 : Servo PWM
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

static const int ServoB = 2; // Servo B Enable (Pull high to enable the servo)
static const int Sounder = 4; // Sounder Enable (Pull low to enable the sounder)
const uint8_t pwmPin = 5;
static const int IridiumSleepPin = 6; // Iridium Sleep connected to D6
static const int ServoA = 7; // Servo A Enable (Pull high to enable the servo)
static const int enable5V = 8; // Enable the MPM3610 5V regulator (Pull high to enable 5V)
static const int GPS_EN = 11; // GNSS Enable on pin D11 (Pull low to enable the GNSS)
static const int ledPin = 13; // WB2812B + Red LED on pin D13
static const int openSwitchB = 14;
static const int openSwitchA = 15;
static const int closeSwitchA = 16;
static const int ringIndicator = 17; // 9602 Ring Indicator on pin D17
static const int closeSwitchB = 18;
static const int Enable_9603N = 19; // 9603N Enable (enables EXT_PWR via P-MOSFET)

#include <IridiumSBD.h> // Requires V2: https://github.com/mikalhart/IridiumSBD

#include <Adafruit_NeoPixel.h> // Support for the WB2812B: https://github.com/adafruit/Adafruit_NeoPixel

#define swap_red_green // Uncomment this line if your WB2812B has red and green reversed
#ifdef swap_red_green
  Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, ledPin, NEO_GRB + NEO_KHZ800); // GRB WB2812B
#else
  Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, ledPin, NEO_RGB + NEO_KHZ800); // RGB WB2812B
#endif
#define LED_Brightness 128 // 0 - 255 for WB2812B

#define GPS_ON LOW
#define GPS_OFF HIGH
#define SOUNDER_ON LOW
#define SOUNDER_OFF HIGH

#define VBUS A7 // Bus voltage analog pin

// Serial2 pin and pad definitions (in Arduino files Variant.h & Variant.cpp)
#define PIN_SERIAL2_RX       (34ul)               // Pin description number for PIO_SERCOM on D12
#define PIN_SERIAL2_TX       (36ul)               // Pin description number for PIO_SERCOM on D10
#define PAD_SERIAL2_TX       (UART_TX_PAD_2)      // SERCOM pad 2 (SC1PAD2)
#define PAD_SERIAL2_RX       (SERCOM_RX_PAD_3)    // SERCOM pad 3 (SC1PAD3)
// Instantiate the Serial2 class
Uart Serial2(&sercom1, PIN_SERIAL2_RX, PIN_SERIAL2_TX, PAD_SERIAL2_RX, PAD_SERIAL2_TX);
HardwareSerial &ssIridium(Serial2); // Use M0 Serial2 to interface to the Iridium 9603N

// Serial3 pin and pad definitions (in Arduino files Variant.h & Variant.cpp)
// eRIC Tx (input) is connected to MOSI (Digital Pin 23, Port B Pin 10, SERCOM4 Pad 2, Serial3 Tx)
// eRIC Rx (output) is connected to SCK (Digital Pin 24, Port B Pin 11, SERCOM4 Pad 3, Serial3 Rx)
#define PIN_SERIAL3_RX       (24ul)               // Pin description number for PIO_SERCOM on D24
#define PIN_SERIAL3_TX       (23ul)               // Pin description number for PIO_SERCOM on D23
#define PAD_SERIAL3_TX       (UART_TX_PAD_2)      // SERCOM4 Pad 2 (SC4PAD2)
#define PAD_SERIAL3_RX       (SERCOM_RX_PAD_3)    // SERCOM4 Pad 3 (SC4PAD3)
// Instantiate the Serial3 class
Uart Serial3(&sercom4, PIN_SERIAL3_RX, PIN_SERIAL3_TX, PAD_SERIAL3_RX, PAD_SERIAL3_TX);
HardwareSerial &ssPi(Serial3);

IridiumSBD isbd(ssIridium, IridiumSleepPin); // This should disable the 9603

#define ssGPS Serial1 // Use M0 Serial1 to interface to the MAX-M8Q

// Servo Position:
// Min Position = 900
// Mid Position = 1500
// Max Position = 2100
int PWM = 1500;
int servoOpen = 1500;
int servoClosed = 1500;
const int minServo = 900;
const int maxServo = 2100;

// Globals
float vbat = 5.0;
unsigned long tnow;

// NeoPixel functions

void LED_off() // Turn NeoPixel off
{
  pixels.setPixelColor(0,0,0,0);
  pixels.show();
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

// Read battery voltage, compensating for resistor divider
void get_vbat() {
  vbat = analogRead(VBUS) * (4.3 * 3.3 / 1023.0);
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

void waitForLF() {
  Serial.println("Press Send to continue...");
  Serial.println();
  while (newData == false) {
    recvWithEndMarker();
  }
  newData = false;
}

// Iridium SBD V2 console and diagnostic callbacks (replacing attachConsole and attachDiags)
void ISBDConsoleCallback(IridiumSBD *device, char c) { Serial.write(c); }
void ISBDDiagsCallback(IridiumSBD *device, char c) { Serial.write(c); }

void setup()
{
  pinMode(enable5V, OUTPUT);
  digitalWrite(enable5V, LOW); // Disable 5V regulator

  pinMode(ServoB, OUTPUT);
  digitalWrite(ServoB, LOW); // Disable switched 5V power for servo B

  pinMode(ServoA, OUTPUT);
  digitalWrite(ServoA, LOW); // Disable switched 5V power for servo A

  pinMode(Enable_9603N, OUTPUT);
  digitalWrite(Enable_9603N, LOW); // Disable switched 5V power for Iridium 9603N

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
  
  //PORT->Group[PORTA].PINCFG[8].reg |= PORT_PINCFG_PMUXEN;
  
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
  
}

void loop()
{
  // Start the serial console
  Serial.begin(115200);
  while (!Serial) ; // Wait for the user to open the serial console

  // Send welcome message
  Serial.println("Data Recovery System V1 Test");
  Serial.println();
  Serial.println("Check that the Serial Monitor baud rate is set to 115200");
  Serial.println("and that the line ending is set to Newline");
  Serial.println();
  Serial.println("Confirm that the beacon is being powered via USB");
  waitForLF();

  // Check VBUS
  get_vbat(); // Read 'battery' voltage
  Serial.print("VBUS is ");
  Serial.print(vbat);
  Serial.print("V : ");
  if ((vbat >= 4.60) and (vbat <= 5.20))
    Serial.println("PASS");
  else
    Serial.println("FAIL!");
  Serial.println();

  // Check current draw
  Serial.println("Check current draw is approx. 15mA");
  waitForLF();

  // Test LED2
  Serial.println("Check LED2 is illuminated");
  digitalWrite(ledPin, HIGH);
  waitForLF();
  
  Serial.println("Check LED2 is off");
  digitalWrite(ledPin, LOW);
  waitForLF();

  // Test NeoPixel (LED1)
  Serial.println("Check NeoPixel is red");
  LED_red();
  delay(100);
  LED_red();
  waitForLF();
  
  Serial.println("Check NeoPixel is green");
  LED_green();
  waitForLF();
  
  Serial.println("Check NeoPixel is blue");
  LED_blue();
  waitForLF();
  
  LED_off();

  // Power up GNSS
  Serial.println("Powering up MAX-M8Q");
  Serial.println("Check current draw rises to approx. 40mA");
  digitalWrite(GPS_EN, GPS_ON); // Enable the GNSS
  waitForLF();

  // Check GNSS
  // Start the GPS serial port
  ssGPS.begin(9600);

  delay(1000); // Allow time for the port to open

  // Configure GNSS
  Serial.println("Configuring MAX-M8Q...");

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
      
  // Flush GNSS serial buffer
  while(ssGPS.available()){ssGPS.read();} // Flush RX buffer

  Serial.println();

  for (tnow = millis(); millis() - tnow < 1UL * 10UL * 1000UL;) {
    while(ssGPS.available()){Serial.write(ssGPS.read());}
  }

  Serial.println();
  Serial.println();
  Serial.println("Confirm that GNSS is producing _only_ GNGGA and GNRMC messages");
  Serial.println("Any other messages - or no messages - is a fail");
  waitForLF();

  digitalWrite(GPS_EN, GPS_OFF); // Disable the GNSS

  // Test open and close switches

  if (digitalRead(closeSwitchA) == LOW)
  {
    Serial.println("!!! CLOSE_A switch failure !!!");
  }
  else
  {
    Serial.println("Press the CLOSE_A switch");
    while (digitalRead(closeSwitchA) == HIGH) ;
  }

  if (digitalRead(openSwitchA) == LOW)
  {
    Serial.println("!!! OPEN_A switch failure !!!");
  }
  else
  {
    Serial.println("Press the OPEN_A switch");
    while (digitalRead(openSwitchA) == HIGH) ;
  }

  if (digitalRead(closeSwitchB) == LOW)
  {
    Serial.println("!!! CLOSE_B switch failure !!!");
  }
  else
  {
    Serial.println("Press the CLOSE_B switch");
    while (digitalRead(closeSwitchB) == HIGH) ;
  }

  if (digitalRead(openSwitchB) == LOW)
  {
    Serial.println("!!! OPEN_B switch failure !!!");
  }
  else
  {
    Serial.println("Press the OPEN_B switch");
    while (digitalRead(openSwitchB) == HIGH) ;
  }

  // Wait for the user to release all switches
  while (digitalRead(closeSwitchA) == LOW) ;
  while (digitalRead(closeSwitchB) == LOW) ;
  while (digitalRead(openSwitchA) == LOW) ;
  while (digitalRead(openSwitchB) == LOW) ;
  
  Serial.println("Switches are OK");
  Serial.println();

  // Test the sounder

  Serial.println("Check sounder is off");
  waitForLF();

  PORT->Group[PORTA].PINCFG[8].reg |= PORT_PINCFG_PMUXEN;
  Serial.println("Check sounder is on");
  waitForLF();
  
  PORT->Group[PORTA].PINCFG[8].reg &= ~PORT_PINCFG_PMUXEN;
  Serial.println("Check sounder is off");
  waitForLF();

  // Check the power relay and 9V battery conections

  Serial.println("Ensure you have linked 3V3 (next to the NeoPixel) to Pin 1 of the Raspberry Pi header");
  waitForLF();
  Serial.println();

  Serial.println("Connect a 9V battery on the top of the PCB");
  waitForLF();
  Serial.println();
  
  Serial.println("Press the OFF button");
  waitForLF();
  Serial.println();

  // Check VBUS
  get_vbat(); // Read 'battery' voltage
  Serial.print("VBUS is ");
  Serial.print(vbat);
  Serial.print("V : ");
  if ((vbat >= 4.60) and (vbat <= 5.20))
    Serial.println("PASS");
  else
    Serial.println("FAIL!");
  Serial.println();

  // Check current draw
  Serial.println("Check current draw is approx. 15mA");
  waitForLF();

  Serial.println("Press the ON button");
  waitForLF();
  Serial.println();

  // Check VBUS
  get_vbat(); // Read 'battery' voltage
  Serial.print("VBUS is ");
  Serial.print(vbat);
  Serial.print("V : ");
  if ((vbat >= 8.0) and (vbat <= 10.0))
    Serial.println("PASS");
  else
    Serial.println("FAIL!");
  Serial.println();

  // Check current draw
  Serial.println("Check current draw is approx. 0mA");
  waitForLF();

  Serial.println("Move the 9V battery to the bottom of the PCB");
  waitForLF();
  Serial.println();
  
  // Check VBUS
  get_vbat(); // Read 'battery' voltage
  Serial.print("VBUS is ");
  Serial.print(vbat);
  Serial.print("V : ");
  if ((vbat >= 8.0) and (vbat <= 10.0))
    Serial.println("PASS");
  else
    Serial.println("FAIL!");
  Serial.println();

  // Check current draw
  Serial.println("Check current draw is approx. 0mA");
  waitForLF();

  // Enable the 5V regulator

  Serial.println("Enabling the 5V regulator");
  digitalWrite(enable5V, HIGH);
  Serial.println();

  // Test Servo A

  Serial.println("Connect a servo to the Servo_A pins");
  waitForLF();
  digitalWrite(ServoA, HIGH); // Enable power for Servo A
  Serial.println("Check servo moves to mid-range");
  waitForLF();
  // Update the servo PWM
  REG_TCC0_CCB1 = minServo; // TCC0 CCB1 - change the servo PWM on D5
  while(TCC0->SYNCBUSY.bit.CCB1);
  Serial.println("Check servo moves to min position");
  waitForLF();
  // Update the servo PWM
  REG_TCC0_CCB1 = maxServo; // TCC0 CCB1 - change the servo PWM on D5
  while(TCC0->SYNCBUSY.bit.CCB1);
  Serial.println("Check servo moves to max position");
  waitForLF();
  // Update the servo PWM
  REG_TCC0_CCB1 = servoOpen; // TCC0 CCB1 - change the servo PWM on D5
  while(TCC0->SYNCBUSY.bit.CCB1);
  Serial.println("Check servo moves back to mid-range");
  waitForLF();
  digitalWrite(ServoA, LOW); // Disable power for Servo A

  // Test Servo B

  Serial.println("Connect a servo to the Servo_B pins");
  waitForLF();
  digitalWrite(ServoB, HIGH); // Enable power for Servo B
  Serial.println("Check servo moves to mid-range");
  waitForLF();
  // Update the servo PWM
  REG_TCC0_CCB1 = minServo; // TCC0 CCB1 - change the servo PWM on D5
  while(TCC0->SYNCBUSY.bit.CCB1);
  Serial.println("Check servo moves to min position");
  waitForLF();
  // Update the servo PWM
  REG_TCC0_CCB1 = maxServo; // TCC0 CCB1 - change the servo PWM on D5
  while(TCC0->SYNCBUSY.bit.CCB1);
  Serial.println("Check servo moves to max position");
  waitForLF();
  // Update the servo PWM
  REG_TCC0_CCB1 = servoOpen; // TCC0 CCB1 - change the servo PWM on D5
  while(TCC0->SYNCBUSY.bit.CCB1);
  Serial.println("Check servo moves back to mid-range");
  waitForLF();
  digitalWrite(ServoB, LOW); // Disable power for Servo B

  Serial.println();

  // Enable and test 9603N
  Serial.println("Powering up the Iridium 9603N");
  Serial.println();
  digitalWrite(Enable_9603N, HIGH); // Enable the 9603N
  delay(2000); // Wait for 9603N to power up

  isbd.setPowerProfile(IridiumSBD::DEFAULT_POWER_PROFILE);
  isbd.useMSSTMWorkaround(false); // Redundant?
  isbd.adjustSendReceiveTimeout(60);
  ssIridium.begin(19200);
  delay(1000);

  if (isbd.begin() == ISBD_SUCCESS) { // isbd.begin powers up the 9603
    Serial.println();
    Serial.println("Iridium 9603N begin was successful : PASS");
  }
  else {
    Serial.println();
    Serial.println("Iridium 9603N begin was unsuccessful : FAIL!");
  }
  Serial.println();
  
  isbd.sleep(); // Put 9603N to sleep
  delay(1000);

  // Put processor to sleep, confirm minimal current draw

  ssIridium.end(); // Close GPS and Iridium serial ports
  ssGPS.end();

  digitalWrite(Enable_9603N, LOW); // Disable the 9603N

  // Power down
  digitalWrite(enable5V, LOW); // Disable 5V
  
  Serial.println("Press the OFF button");
  waitForLF();
  Serial.println();

  // Check VBUS
  get_vbat(); // Read 'battery' voltage
  Serial.print("VBUS is ");
  Serial.print(vbat);
  Serial.print("V : ");
  if ((vbat >= 4.60) and (vbat <= 5.20))
    Serial.println("PASS");
  else
    Serial.println("FAIL!");
  Serial.println();

  // Check current draw
  Serial.println("Check current draw is approx. 15mA");
  waitForLF();

  Serial.println();
  Serial.println("Last test: putting the processor into deep sleep");
  Serial.println("Confirm current draw falls to approx. 1mA");
  delay(1000); // Wait for serial port to clear
  Serial.end(); // Close the serial console
  USBDevice.detach(); // Safely detach the USB prior to sleeping

  // Deep sleep...
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  __WFI();

  while (true) ; // Wait for reset
}


