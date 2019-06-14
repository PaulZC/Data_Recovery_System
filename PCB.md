# PCB Assembly

This guide contains instructions on how to assemble the DRS PCB: starting with the bare PCB and finishing with a populated PCB which has been tested with test code.

### Blank PCB

Start by having the blank PCBs manufactured. If you are based in the UK or Europe, I can recommend
[Multi-CB](https://www.multi-circuit-boards.eu/en/index.html) who can manufacture PCBs in 1-8 working days and
can process the Eagle .brd file direct - there's no need to generate Gerber files.

You will find the PCB and paste stencil design files in the [Eagle directory](https://github.com/PaulZC/Data_Recovery_System/tree/master/Eagle)

The PCB and trigger linkage pieces are incorporated into a single board. The design includes two score lines which Multi-CB
will machine into the board if you ask them to, allowing the trigger linkage pieces to be easily snapped off.

My recommended options are:
- Layers: 4 layers
- Format: Panel
- Panel Type: Multiplier
- Mechanical treatment: Milled and Scored
- Panelization: Yes, by Multi-CB
- Panel size (x/y): 128.27 X 304.8 mm
- Piece size (x/y): 128.27 X 304.8 mm
- PCBs / x-, y-Axis: 1 X 1 Pieces
- Surface finish: Chemical gold (ENIG)
- Material: FR4, 1.55mm
- Cu layers: 35um
- Solder stop: both sides, green
- Marking print: both sides, white
- **Comments: PCB includes two score lines. VSCORE data is in layer 148**

![PCB_1](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/PCB_1.JPG)

### Add solder paste

Multi-CB can also provide you with a solder paste (SMD) stencil for the PCB. The [Eagle directory](https://github.com/PaulZC/Data_Recovery_System/tree/master/Eagle)
contains a separate design for the paste stencil which is 10mm wider than the DRS PCB, to allow you to fix it down with tape.

My recommended options are:
- SMD stencil for: top layer
- Size (x/y): 118.11 X 88.9 mm
- Type: custom
- Pad reduction: yes, 10%
- Thickness: 100um
- Existing fiducials: lasered through
- Text type: half lasered
- Double-sided brushing: yes

Secure the blank PCB onto a flat work surface by locating it between two engineer's squares. I use a sheet of toughened glass
as the work surface as it is both very flat and easy to clean.

Use the two round fiducials to line up the stencil with the PCB. Secure the stencil with tape.

![PCB_2](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/PCB_2.JPG)

Apply solder paste close to the component cut-outs and then scrape the paste over the stencil using a knife blade
or a similar straight edge. Take appropriate safety precautions when working with solder paste - particularly if you are using
tin-lead solder instead of lead-free.

### Position the surface mount components

Position the components onto the blobs of solder paste using tweezers. A magnifier lamp or a USB microscope will
help you place the components in the correct position. J1 - the 20-way Samtec connector - is probably the trickiest
component to position. Take extra time to make sure the legs are centered accurately on the pads.

### Reflow the surface mount components

Use a reflow oven to heat the circuit board to the correct temperatures to melt the solder. A reflow oven doesn't need to be
expensive. The one shown below was assembled from:

- Quest 9L 800W mini-oven
- Inkbird PID temperature controller and 40A solid state relay
- Type K thermocouple

Several people have published good reflow oven construction guides, including [this one](http://tt7hab.blogspot.com/2018/06/the-reflow-oven.html) and [this one](http://www.die4laser.com/toaster/index.html).

![V5_Assembly_8](https://github.com/PaulZC/Iridium_9603_Beacon/blob/master/img/V5_Assembly_8.JPG)

Use the correct temperature profile for your solder paste, but you won't go far wrong with 160C for four minutes, followed by
210C for one minute, followed by a cool-down with the door open. Use a flashlight to check that the solder has melted across
the whole PCB at 210C. Hold the temperature at 210C a little longer if some of the solder paste still appears 'gray' instead of 'silver'.

### Check for shorts

Carefully examine all the solder joints and correct any shorts you find.

The 'trick' to removing a short is to add more solder or solder paste and then to use
copper solder braid or wick to remove all the solder in one go.

![Shorts_1](https://github.com/PaulZC/Iridium_9603_Beacon/blob/master/img/Shorts_1.JPG)

![Shorts_2](https://github.com/PaulZC/Iridium_9603_Beacon/blob/master/img/Shorts_2.JPG)

![Shorts_3](https://github.com/PaulZC/Iridium_9603_Beacon/blob/master/img/Shorts_3.JPG)

Solder any remaining surface mount components by hand and then use a cotton bud (q-tip) dipped in Iso-Propyl Alcohol
(IPA / Propanol / rubbing alcohol) to remove any flux residue.

All being well, your PCB should look like this:

![PCB_3](https://github.com/PaulZC/Data_Recovery_System/blob/master/img/PCB_3.JPG)

### Press-in nuts

The Iridium 9603N module is held in place by two 2-56 screws. The threaded 2-56 press-in nuts now need to be pressed into the rear of the
PCB. These are McMaster part number 95117A411. The nuts are best pressed into place using an arbor press.

### Add the non-surface mount components

The non-surface mount components (battery clips, relay, switches, SMA conector, DC-DC converter, sounder 3-pin servo headers, 4-pin power header) can now be soldered in by hand.

### Install the bootloader

The SAMD21G18A processor now needs to be configured with a bootloader using a J-Link programmer or similar, connecting it to the five pads on the right hand edge of the PCB.
Full details are included in the [Iridium_9603_Beacon repo](https://github.com/PaulZC/Iridium_9603_Beacon/blob/master/LEARN.md#how-do-i-install-the-atsamd21g18-bootloader).

### Test the PCB

Before connecting the Iridium 9603N, it is a good idea to test the completed PCB. The [Arduino](https://github.com/PaulZC/Data_Recovery_System/tree/master/Arduino)
directory contains a sketch called Data_Recovery_System_V1_Test.ino which will test all of the components on the PCB for you. Messages are displayed
on the Arduino IDE Serial Monitor as each test is passed.

### Install the Iridium 9603N module

Take appropriate ESD precautions throughout and especially when handling the 9603N module.

Connect the 9603N to the beacon PCB using a HIROSE (HRS) u.FL-u.FL cable or similar. The cable needs to be 50 Ohm and approximately 25mm long.

Carefully fold the 9603N module over onto the PCB, insert the 20-way connector into the Samtec socket, then secure the module using:
- two 4.5mm OD x 6mm spacers (McMaster 94669A100)
- two 2-56 x 7/16" screws (McMaster 92185A081)

Screw the GNSS+Iridium antenna onto the SMA connector

### Retest the PCB

Now that the 9603N has been installed, retest the completed PCB using the [Arduino test code](https://github.com/PaulZC/Data_Recovery_System/tree/master/Arduino/Data_Recovery_System_V1_Test).

### Secure J1 with Epoxy

I recommend securing J1 to the PCB with a small amount of epoxy to make it more robust. 3M Scotch-Weld 2216 is an ideal choice as it remains slightly flexible when cured.
You will need to temporarily remove the 9603N while you apply the epoxy. Apply the mixed epoxy around the body and legs of J1 using a toothpick,
taking great care to keep the epoxy clear of the connector opening and pins. Make sure the epoxy is fully cured before re-installing the 9603N.

### Lacquer the PCB

I recommend giving the PCB a coat of lacquer. You will need to temporarily remove the 9603N while you apply the lacquer. Cover all of the surface mount components with
[Acrylic Protective Lacquer (conformal coating)](https://uk.rs-online.com/web/p/conformal-coatings/3217324/) avoiding J1, CON1, CON2, the switches and IO pads.



