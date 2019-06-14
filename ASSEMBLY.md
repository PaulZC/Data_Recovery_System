# ASSEMBLY

This guide contains instructions on how to assemble and configure the DRS.

If your DRS has been assembled previously and you only want to make it ready for flight, the please skip forward to Step n.

## Prerequisites

- The DRS PCB should have been prepared and tested following the instructions in [PCB.md](https://github.com/PaulZC/Data_Recovery_System/blob/master/PCB.md)
- The Raspberry Pi should have been configured and have the [DRS_Python_Tools](https://github.com/PaulZC/DRS_Python_Tools) installed following the instructions in [PI.md](https://github.com/PaulZC/Data_Recovery_System/blob/master/PI.md)
- The two HiTec servos should have been prepared following the instructions in [SERVO.md](https://github.com/PaulZC/Balloon_Cut-Down_Device/blob/master/SERVO.md)
- The two archery release aids should have been prepared following the instructions in [SHARK.md](https://github.com/PaulZC/Balloon_Cut-Down_Device/blob/master/SHARK.md)

## Step 1: Install the Raspberry Pi on the underside of the DRS PCB

![Bottom](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/Bottom.JPG)

- Push an [11mm 2x20 Header](https://shop.pimoroni.com/products/2x20-pin-gpio-header-for-raspberry-pi-2-b-a?variant=1132812269) on the Raspberry Pi's GPIO pins
- The header pins are just long enough to reach the PCB when the Pi is mounted
- Apply a liberal coating of [Acrylic Protective Lacquer (conformal coating)](https://uk.rs-online.com/web/p/conformal-coatings/3217324/) to both sides of the Raspberry Pi PCB, covering all components except the connectors
- Once the coating is dry, mount the Pi on the underside of the PCB using:
  - Four 30mm M2.5 stainless steel screws [McMaster part 91292a037](https://www.mcmaster.com/91292a037)
  - Four 19mm aluminium spacers [McMaster part 94669a112](https://www.mcmaster.com/94669a112)
  - Sixteen M2.5 fiber washers [McMaster part 95225a310](https://www.mcmaster.com/95225a310)
  - Four M2.5 Nylock nuts [McMaster part 93625a102](https://www.mcmaster.com/93625a102)
- Solder all 40 header pins to the PCB

## Step 2: Mount the two archery releases on the PCB

- Use six 1/4" 4-40 UNC screws [McMaster part 92196a106](https://www.mcmaster.com/92196a106)
- Apply threadlock to each screw first
- Release A (for the tether) goes on the underside of the PCB, release B (for the parachute) goes on top

## Step 3: Mount the two servos on the PCB

- Press the supplied rubber inserts into the holes on the sides of the servos
- Insert the brass inserts into the bottom of each rubber insert
- Mount the servos using:
  - Four 14mm M2.5 stainless steel screws [McMaster part 91292a017](https://www.mcmaster.com/91292a017)
  - Four M2.5 Nylock nuts [McMaster part 93625a102](https://www.mcmaster.com/93625a102)
  - Four M2.5 stainless steel washers [McMaster part 90965a115](https://www.mcmaster.com/90965a115)
- Place the washers under the screw heads
- The Nylock nuts will tighten against the flat face on the end of each brass insert
- Do not install the servo arms yet
- Servo A goes on the underside of the PCB, servo B goes on top

## Step 4: Upload the Arduino code

- After following the instructions in [PCB.md](https://github.com/PaulZC/Data_Recovery_System/blob/master/PCB.md) the SAMD microcontroller will have been configured with its bootloader and the PCB components will have been tested using the Arduino code in the [Test folder](https://github.com/PaulZC/Data_Recovery_System/tree/master/Arduino/Data_Recovery_System_V1_Test)
- The next step is to upload the DRS Arduino code to the microcontroller
- Open the Arduino IDE and connect the DRS to your computer using a micro-USB cable
- Check the Tools\Board and Tools\Port settings. The DRS should appear as an "Adafruit Feather M0"

![Board](https://github.com/PaulZC/F9P_RAWX_Logger/blob/master/img/Board.JPG)

- Download and open [Data_Recovery_System_V1.ino](https://github.com/PaulZC/Data_Recovery_System/tree/master/Arduino/Data_Recovery_System_V1) in the IDE
- Click on the arrow icon below the Edit menu. This will compile the code and upload it onto the DRS
- As soon as the upload is finished, click on the Tools menu and then "Serial Monitor"
- Change the baud rate to 115200 using the pull-down menu at the bottom of the serial monitor window
- All being well, after 10 seconds you should see messages saying:

![Serial_Monitor](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/Serial_Monitor.JPG)

## Step 5: Configure the servo positions

The open and closed servo positions for DRS1-5 are included in the Arduino code. Only follow this procedure is the positions need to be modified (e.g. after removing the arm from a servo) or you are using a new PCB

The servos cannot be moved unless: at least one 9V battery has been installed; the latching relay is 'ON'.

The servos are powered by a 5V regulator which is powered by the 9V batteries.

The 'ON' and 'OFF' buttons, which control the state of the latching relay, are only operational when the Raspberry Pi is powered up. They draw 3.3V power from the Pi (not the microcontroller).

- Connect 24V DC power to the power connector. If connecting to the 4-pin header: the two outer pins are 0V; the two inner pins are 24V
- The Pi will boot up and attempt to connect to WiFi
- If WiFi is not available, you can connect a laptop to the Pi using the Ethernet port. The Pi acts as a DHCP server (see step 25 in [PI.md](https://github.com/PaulZC/Data_Recovery_System/blob/master/PI.md))
- Clip a 9V battery into one set of battery clips (only one battery is required, not two)
- Press the ON button to enable power for the servos
- To place the DRS into 'Set Servo A' mode:
  - Push and hold down the OPEN_A and CLOSE_A switches
  - Push and release the RESET switch
  - Wait until the NeoPixel has gone cyan before releasing the OPEN_A and CLOSE_A switches
- The servo will move to its mid-range position
- You can now install the servo arm. Orient the arm at four o' clock (with the PCB oriented as in the above photo)
- Use a 4mm M3 low profile shoulder screw ([McMaster part 90323a211](https://www.mcmaster.com/90323a211)) to fix the servo linkage to the archery release trigger block
- Do not connect the linkage to the servo arm yet
- Push the archery release trigger so that the release jaws are closed
- Pressing the CLOSE_B switch will step the servo arm towards the archery release aid. Pressing OPEN_B will step it away. Push and hold for larger movements
- Move the arm until it is positioned as shown:

![Set_Servo_1](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/Set_Servo_1.JPG)

- Press the CLOSE_A switch to store the position in flash memory. The NeoPixel will flash green as the write takes place
- Pull the archery release trigger so that the release jaws are open
- Move the servo arm using OPEN_B and CLOSE_B until it is positioned as shown:

![Set_Servo_2](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/Set_Servo_2.JPG)

- Press the OPEN_A switch to store the position in flash memory. The NeoPixel will flash green as the write takes place
- Press the RESET switch to leave 'Set Servo A' mode
- You can now secure the trigger linkage to the servo arm using another shoulder screw
- Repeat for servo B
  - Push and hold down the OPEN_B and CLOSE_B switches
  - Push and release the RESET switch
  - Release the OPEN_B and CLOSE_B switches after the NeoPixel has gone cyan
  - Use the OPEN_A and CLOSE_A switches to move the servo
  - Use the OPEN_B and CLOSE_B switches to store the servo positions
  - Press RESET when done

The serial monitor messages will list the new servo positions. You should copy these values into the Arduino code and re-upload to the DRS when convenient.

## Step 6: Test the servos

The servos can be tested using the serial monitor. Make sure the Baud rate is set to 115200 and the line ending is set to Newline.

The serial monitor will display the NMEA messages from the MAX-M8Q GNSS receiver.

- Type OPEN_A into the text box at the top of the serial monitor and either click 'Send' or press enter
- Servo A will move to the open position
- CLOSE_A will move servo A to the closed position
- Likewise OPEN_B and CLOSE_B can be used to move servo B
- These commands are only actioned if received through the serial monitor. They are ignored if sent via the Raspberry Pi's serial port

## Step 7: Other serial monitor commands

The Arduino code will also respond to the following commands if they are submitted from the serial monitor through the micro-USB port:
- SOUNDER_ON will enable sounder beeps every 5 seconds
- SOUNDER_OFF will disable sounder beeps
- GoGoGo will cause the microcontroller to go into 'Gone' (Drop) mode: servo A will be opened, Iridium messages will be set every 2 minutes

'Gone' mode can only be disabled by:
- Pressing and holding OPEN_A, CLOSE_A, OPEN_B and CLOSE_B
- Pressing and releasing RESET
- Keeping OPEN_A, CLOSE_A, OPEN_B and CLOSE_B held down until the NeoPixel goes green

The Raspberry Pi's serial port cannot be used to move the servos or enable/disable the sounder. GoGoGo is the only valid command that can be sent by the Pi.

## Step 8: Attach the tether and parachute

It is not good practice to move the servos by hand. They can be damaged by being back-driven.
So, the best way to close the servos when attaching the tether and parachute is using the serial monitor commands described above.

The tether is a length of [Size 4N nylon cord (2.4mm diameter)](https://www.ropesdirect.co.uk/4n-nylon-cord-132m.html).
Cut a ~0.75m length and seal the ends with flame to prevent fraying. Close the jaws of Servo A mid-way along the tether.

The Rocketman 4' parachute has a stitched loop at the center point of the suspension lines. Close the jaws of servo B around the loop as shown below:

![DRS_Assembly_1](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/DRS_Assembly_1.JPG)

## Step 9: Power down and connect the batteries

Before powering down the DRS, so you can finish the assembly, please check:
  - That the microcontroller has been reset to 'Waiting for Go' mode if required
  - Both archery releases are closed and the tether and parachute are attached

- Reset the Omron relay by pressing the OFF button. This disconnects the 9V batteries. The microcontroller will stay on as it is drawing power from the micro-USB cable
- Turn off the microcontroller by unplugging the micro-USB cable
- Shutdown the Pi over WiFi or Ethernet with: sudo shutdown now
- Wait for the green LED on the Pi to stop blinking
- Disconnect the 24V power to the Pi
- Install the low extraction force power connector plug if required

## Step 10: Insert the batteries

- Insert two fresh Energizer Ultimate Lithium 9V PP3 (L522) into the battery clips, securing them with hot glue or double-sided adhesive tape

## Step 11: Close up the case

- The 3D printed case is made from two identical parts which clip together with a simple tongue and groove joint
- Apply a thin layer of silicone sealant across the 'wall' pieces in each half of the case that divide the servos and releases from the Raspberry Pi
- Place the PCB into the bottom half of the case
- Position the top half and press toegther to engage the tongue and groove joint
- Tape the two halves of the case together using a long length of duct tape. Ensure that the ends of the tape are folded over into tabs. These will be useful when pulling the case out of the foam enclosure (it is a tight fit!)

![DRS_Assembly_1](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/DRS_Assembly_1.JPG)

## Step 12: Insert the case into the foam enclosure

- Push the case into the foam enclosure
- Securely knot the nylon cord which runs through the foam around the end of the case. Trim and seal the cord ends with flame

![DRS_Assembly_2](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/DRS_Assembly_2.JPG)

- Attach the low extraction force power connector socket cable. Ensure the arrows are aligned
- Secure nylon line to the power cable approx. 100mm from the connector. You might find it useful to tie a single knot in the power cable and feed the tie-wrap through that

![DRS_Assembly_3](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/DRS_Assembly_3.JPG)

![DRS_Assembly_4](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/DRS_Assembly_4.JPG)

## Step 13: Fold the parachute

Fold the parachute following the instructions on the [Rocketman website](https://the-rocketman.com/packing/)

Our method follows the 'dog barf' method - but without the 'dog barf' (cellulose insulation) itself

## Step 14: Insert into the launch tube

The launch tube is made from:
- 350mm of 150mm [ventilation ducting](https://www.amazon.co.uk/Blauberg-UK-150-Extractor-Ventilation/dp/B06XDG47JB/ref=sr_1_8)
- a 150mm [pipe spigot](https://www.amazon.co.uk/Spigot-Flange-Mounting-Ventilation-Connection/dp/B01KABZEJU/ref=sr_1_15)
- two aluminium brackets (one bent so it fits under the other)
- secured together with eight M4 stainless steel button head screws, sixteen washers and eight Nylock nuts

![DRS_Assembly_5](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/DRS_Assembly_5.JPG)

- Pass the tether and power cords through the pairs of holes in the aluminium brackets
- Hold the parachute against the bottom of the foam enclosure while drawing the cords through the brackets
- Once fully inserted, the parachute should be compressed over to one side of the tube and trapped by the tether. This should leave the power connector available for inspection

![DRS_Assembly_6](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/DRS_Assembly_6.JPG)

- Securely knot the tether and power cords. Cut the cord short and seal the ends with flame
- The DRS is ready to be secured to the gondola

![DRS_Assembly_7](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/DRS_Assembly_7.JPG)



