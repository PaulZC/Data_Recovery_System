1. Burn the latest Raspberry Pi Raspbian onto the 512GB micro SD card
- Visit https://www.raspberrypi.org/downloads/
- Download the latest "Raspbian Stretch with desktop and recommended software"
- Extract the .img with e.g. http://www.7-zip.org/download.html
- Burn it to the SD card using e.g. https://sourceforge.net/projects/win32diskimager/

2. Insert the SD card into a Raspberry Pi 3 B+
- Connect screen, mouse, keyboard and (finally) power

3. Let the Pi boot. It will automatically resize the SD card file system to its full size

4. Go through the set up process to set the country and connect to a WiFi network

5. Skip the update software step (we’ll be doing an update and upgrade shortly)

6. Let the Pi restart if requested

7. Open a terminal window by clicking on the >_ icon

8. In  the terminal window:
- sudo raspi-config
- Select option2 then option N1 to change the hostname to e.g. drs1
- Select option 3 then option B1 then option B2 to boot into “Console Autologin”
- Select option 4 then option I4 to change the WiFi country if required
- Select option 5 then option P2, select Yes to enable the SSH server
- Select option 5 then option P6, select No for the login shell over serial, then select Yes to enable the hardware serial port
- Select finish and reboot the Pi

9. After the reboot, you should be presented with a console prompt (e.g.) pi@drs1

10. (You can start an X session if you want to using startx)

11. Remove Wolfram and LibreOffice to fee up some storage space
- sudo apt-get purge wolfram-engine
- sudo apt-get clean
- sudo apt-get autoremove
- sudo apt-get remove --purge libreoffice*
- sudo apt-get clean
- sudo apt-get autoremove

12. Update and upgrade raspbian
- sudo apt-get update
- sudo apt-get upgrade

13. If you want to change WiFi networks from the console, you can do this by editing wpa_supplicant.conf
- sudo nano /etc/wpa_supplicant/wpa_supplicant.conf

14. Make a note of the Pi’s wireless (wlan0) IP number (hopefully it won’t change when you reboot)
- ifconfig -a

15. Shutdown the Pi
- sudo shutdown now

16. Wait until the green LED stops blinking, then remove the power

17. Disconnect the screen, keyboard and mouse

18. Install the four 128GB micro-SD cards in their miniature USB adapters

19. Power the Pi and you should see LEDs light up on all four micro-SD adapters

20. Connect to the Pi over SSH using (e.g.) PuTTY
- https://www.chiark.greenend.org.uk/~sgtatham/putty/latest.html
- Log in as user pi with the password raspberry (unless you changed it!)

21. Run lsblk and check that you now have four drives called sda, sdb, sdc and sdd. (The 512GB card is called mmcblk0)

22. Create a RAID-1 system following the instructions in section 3 of:
- https://blog.alexellis.io/hardened-raspberry-pi-nas/ 
- sudo fdisk /dev/sda
- o
- n
- p
- enter (default)
- enter (default)
- enter (default)
- w
- sudo fdisk /dev/sdb
- o
- n
- p
- enter (default)
- enter (default)
- enter (default)
- w
- sudo fdisk /dev/sdc
- o
- n
- p
- enter (default)
- enter (default)
- enter (default)
- w
- sudo fdisk /dev/sdd
- o
- n
- p
- enter (default)
- enter (default)
- enter (default)
- w
- sudo apt-get -qy install mdadm
- sudo mdadm --create --verbose /dev/md0 --level=mirror --raid-devices=4 /dev/sda1 /dev/sdb1 /dev/sdc1 /dev/sdd1
- y
- sudo mkdir -p /mnt/raid1
- sudo mkfs.ext4 /dev/md0
- sudo mount /dev/md0 /mnt/raid1/
- echo "/dev/md0  /mnt/raid1/ ext4 defaults,noatime 0 1" | sudo tee -a /etc/fstab
- sudo mdadm --detail --scan | sudo tee -a /etc/mdadm/mdadm.conf
- sudo reboot

23. Let the Pi reboot then connect again with PuTTY

24. Check that the RAID drive is ok
- df /mnt/raid1
- should return something like:
- Filesystem     1K-blocks     Used    Available Use% Mounted on
- /dev/md0       122317432 61464 115999484   1% /mnt/raid1

25. When we mount the Pi onto the DRS, it is going to be difficult to connect either screen, mouse and keyboard or to go in over WiFi (if you need to connect to a new WiFi network). So, we’ll set up a static IP address and DHCP server on the Ethernet port so we can SSH in through that. Your WiFi is probably using 192.168.0.n addresses, so we’ll use 192.168.1.n addresses for Ethernet
- sudo nano /etc/dhcpcd.conf
- uncomment and change the two lines below “Example static IP configuration:”
- interface eth0
- static ip_address=192.168.1.2/24
- ctrl-x
- y
- enter

26. Install dnsmasq:
- sudo apt-get install dnsmasq

27. Create a new dnsmasq.conf file:
- sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.orig
- sudo nano /etc/dnsmasq.conf
- interface=eth0
- dhcp-range=192.168.1.3,192.168.1.20,255.255.255.0,24h
- ctrl-x
- y
- enter

28. Reload dnsmasq:
- sudo systemctl reload dnsmasq

29. Check the Ethernet connection by SSH-ing in over Ethernet. The Pi is 192.168.1.2

30. Download the Python tools which will enable the Arduino part of the DRS, copy the incoming GNSS serial data to DRS_serial_log.txt, and tell the DRS to “GoGoGo”:
- git clone https://github.com/PaulZC/DRS_Python_Tools

31. We now need to make DRS_Python_Tools/DRS_tx_off.py run on startup to correct Paul’s booboo. The serial Tx pin can provide just enough power to try and start the SAMD processor. DRS_tx_off.py prevents this by making the Tx pin low. DRS_on.py has to put the Tx pin back into alt5 mode before we can use the serial port again.
- sudo nano /etc/profile
- scroll to the end of the file and add
- sudo python /home/pi/DRS_Python_Tools/DRS_tx_off.py
- ctrl-x
- y
- enter

32. Shutdown the Pi. Disconnect the power once the green LED has stopped flashing. The LEDs on the four micro-SD card adapters should also stop flashing.
- sudo shutdown now

33. You can now bolt the Pi to the DRS circuit board.

The [DRS_Python_Tools repo](https://github.com/PaulZC/DRS_Python_Tools) contains:

**DRS_tx_off.py** pulls the Pi’s serial Tx pin low to prevent it from supplying power to the SAMD processor. This now runs automatically on boot.

**DRS_on.py** will set the latching relay so it provides power to the SAMD processor and restores the serial Tx pin to alt5 mode.

**DRS_log_serial.py** runs for 5 seconds, appends any serial data to DRS_serial_log.txt and then shows the tail of the log file so you can see if the GNSS receiver has established a fix.

**DRS_GoGoGo.py** sends the Go command to the SAMD processor. Confirm with y or Y. The drop will take place after 30 seconds (allowing time for the Pi to shutdown).

