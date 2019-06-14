# GoGoGo

Instructions on how to enable, release and track the DRS.

Also included are [instructions](https://github.com/PaulZC/Data_Recovery_System/blob/master/GoGoGo.md#step-10-resetting-the-microcontroller) on how to reset the microcontroller from 'Gone' mode into 'Waiting for GoGoGo' mode (required after performing a drop / test drop).

## Prerequisites

- The DRS should have been prepared following the instructions in [ASSEMBLY.md](https://github.com/PaulZC/Data_Recovery_System/blob/master/ASSEMBLY.md)
- The Raspberry Pi should have been configured for SuperBIT WiFi and have the [DRS_Python_Tools](https://github.com/PaulZC/DRS_Python_Tools) installed following the instructions in [PI.md](https://github.com/PaulZC/Data_Recovery_System/blob/master/PI.md)
- The SAMD microcontroller should have been programmed with the correct Arduino code; the servo open/close positions set and the latching relay set OFF (which is only possible while the Pi is powered)
- The microcontroller code should have been freshly installed. If a test drop has been performed, the code needs to be 'reset' either by re-uploading the code or by following the [instructions below](https://github.com/PaulZC/Data_Recovery_System/blob/master/GoGoGo.md#step-10-resetting-the-microcontroller)

## Step 1: Apply power

- Apply 24V DC power to the DRS via the 2-pin low extraction force connector. The Recom REC10-2405SRWZ/H3/A/M DC-DC converter can operate from 9V to 36V. (A 48V version is available which can operate from 18V to 75V)
- The Raspberry Pi will boot up and attempt to connect to WiFi: SSID SuperBIT with PSK Timmins2019

## Step 2: Log in using SSH

- Connect to the Pi using SSH. Log in as user 'pi' with password 'raspberry'

## Step 3: File transfer

- Transfer files onto the Pi's /mnt/raid1 RAID storage using SFTP

## Step 4: Power up the microcontroller

- A few minutes before the chosen drop time, use the SSH console to cd into the DRS_Python_Tools directory
- Power up the microcontroller by running the script 'DRS_on.py': python DRS_on.py
- The script sets three of the Pi's GPIO pins into the correct state to set the Omron latching relay, switching on 9V battery power to the microcontroller
- The microcontroller will power up and will start to establish a GNSS fix

## Step 5: Check the GNSS fix status

- The NMEA messages from the u-blox MAX-M8Q GNSS receiver are echoed by the microcontroller to the Pi's serial port
- Run the script 'DRS_log_serial.py' from the SSH console: python DRS_log_serial.py
- The script runs for 5 seconds and will append the incoming serial data to DRS_serial_log.txt. The script then displays the tail of the log file
- Look at the lines containing GPGGA. If the GNSS has achieved a fix, you should see the correct date, time, latitude and longitude
- If you only see the correct time, the GNSS receiver has locked onto at least one satellite but has not yet established a fix
- If the GNSS has not established a fix, decide whether to wait for a fix or whether to drop anyway (hoping that the GNSS will establish a fix during the descent - it should!)

## Step 6: Drop

- 30 seconds before the desired drop time, run the script 'DRS_GoGoGo.py': python DRS_GoGoGo.py
- Confirm with 'Y' or 'y'
- The script sends a GoGoGo command to the microcontroller via the serial port
- The script then shuts down the Pi (sudo shutdown now)
- The microcontroller will wait 30 seconds and then open archery release A
- The sounder will start to beep every 5 seconds

## Step 7: During the descent

- The DRS will fall, pulling apart the power connection (disconnecting power to the Pi)
- The parachute will unfurl but will remain attached to archery release B
- The microcontroller is now in 'Gone' mode
- The microcontroller will obtain a GNSS fix and send this as an Iridium Short Burst Data message every 2 minutes
- Messages will be delivered via email to the addresses configured in the Rock7 Operations Delivery Groups
- The message interval can be changed by logging in to RockBLOCK Operations (https://rockblock.rock7.com/Operations) (ask Paul for the username and password) and using the 'Send a Message' option
- Send a message to the RockBLOCK (DRS1 is RockBLOCK 11800) containing the text: [INTERVAL=nn]
- where nn is the message interval in minutes. The square brackets are important
- If you want to try this as a test, make sure you do it outdoors where the DRS has a clear view of the sky
- The [Iridium_9603_Beacon webpage](https://github.com/PaulZC/Iridium_9603_Beacon/blob/master/RockBLOCK.md#configuring-your-beacon-via-rockblock-operations) provides extra information

## Step 8: After landing

- Once you have confirmed that the DRS has landed, you can release it from the parachute via an Iridium message
- Send a message containing the text: [PARACHUTE=hh:mm]
- where hh and mm are the current **UTC** time. The square brackets are important
- The microcontroller will ignore the message if hh and mm do not match current UTC time to +/- 15 minutes. This ensures any old queued messages are ignored
- The microcontroller will open archery release B after the next SBD message cycle

## Step 9: Other commands

- If you wish to disable the sounder / beeper, send the message: [SOUNDER_OFF]
- The sounder can be re-enabled with the message: [SOUNDER_ON]
- The DRS can also be configured to forward all messages to another RockBLOCK using the message: [RBDESTINATION=nnnnn]
- where nnnnn is the RockBLOCK serial number (not the IMEI) of the 9603N you wish the messages to be forwarded to
- Full details can be found on the [Iridium_9603_Beacon webpage](https://github.com/PaulZC/Iridium_9603_Beacon/blob/master/RockBLOCK.md#configuring-your-beacon-via-rockblock-operations)
- The recovery team could use this to track the DRS and release the parachute from anywhere - [without an internet connection](https://github.com/PaulZC/Iridium_9603_Beacon/blob/master/RockBLOCK.md#tracking-your-beacon-without-an-internet-connection)

## Step 10: Resetting the microcontroller

- Once the microcontroller has received a 'GoGoGo' message, a 'Gone' flag is set in flash memory. Resetting the microcontroller or disconnecting and reconnecting the batteries does not clear the flag
- To reset the microcontroller into 'Waiting for Go' mode: press and hold the OPEN_A, CLOSE_A, OPEN_B and CLOSE_B buttons, then press and release the RESET button
- Keep the OPEN_A, CLOSE_A, OPEN_B and CLOSE_B buttons pressed down until the NeoPixel goes green, then release them
- The 'Gone' flag is now clear. The microcontroller returns to [Step 5](https://github.com/PaulZC/Data_Recovery_System/blob/master/GoGoGo.md#step-5-check-the-gnss-fix-status)

## Step 11: Resetting the latching relay

- Resetting the latching relay can only be done while the Pi is powered
- Connect the DRS to 24V using the power pins
- The OFF button can then be pressed which resets (opens) the latching relay contacts
- To be kind to the Pi, SSH in over WiFi or Ethernet and 'sudo shutdown now' before disconnecting the power

