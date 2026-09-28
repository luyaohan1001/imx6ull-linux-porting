List of files changed during the porting:
	Config file: ./include/configs/mx6ull_alientek_emmc.h
	Driver file: ./board/freescale/mx6ull_alientek_emmc/mx6ull_alientek_emmc.c
	Net Driver: drivers/net/phy/phy.c 

# Change default .dtb at kernel bootup, edit ./include/configs/mx6ull_alientek_emmc.h
	At line ~195, change to this:
	"findfdt="\
	"if test $fdt_file = undefined; then " \
		"setenv fdt_file imx6ull-alientek-emmc-luyaohan1001.dtb; " \
	"fi;\0" \
		
# How to compile:
$ sudo chmod 777 ./mx6ull_alientek_emmc.sh
$ ./mx6ull_alientek_emmc.sh

# What's useful after the compilation:
	./uboot.imx

# How to flash to the disk
$ ./imxdownload u-boot.bin /dev/<device>

putty connection: baud rate 115200

	# change mode all, add read and write
	$ sudo chmod a+rw /dev/ttyUSB0

This version of uboot has been test on Ubuntu 16.04 through USB-TTY on putty.

1. Connect both USB TTL and USB OTG cable.
2. Check if CH340 driver detects the connect of the board:
	$ dmesg | grep tty

	# expect 'ttyUSB0 is connected...'

3. Flash the SD card. After uboot is loaded after reset, type 'Enter' in Cutecom to allow command prompt to uboot. 

4. Setup the following to allow internet connections between the board the the ubuntu host when the two are connected with Ethernet cable.
5. On ubuntu host, go to connections. Disable IPv6, enable IPv4 and use manual ip address assignment.
6. The address on ubuntu can be, for example 10.0.0.9, the netmask is 255.255.255.0, gateway can be empty.
7. The environment variables in uboot needs to be setup similar to this:
	$ uboot terminal > setenv ipaddr 10.0.0.200 		-> ip address of alientek board
	$ uboot terminal > setenv ethaddr b8:ae:1d:01:00:00	-> mac address of alientek board
	$ uboot terminal > setenv gatewayip 10.0.0.0		-> gateway 
	$ uboot terminal > setenv netmask 255.255.255.0		-> netmask
	$ uboot terminal > setenv serverip 10.0.0.9		-> ip address of the ubuntu host
	$ uboot terminal > saveenv				-> save the variables

8. Now we try to ping the host on the board:
	$ uboot terminal > ping 10.0.0.9 
	# expect:
		'FEC1 waiting for PHY auto negotiation to complete...done'
		'Using FEC1 device'
		'host 10.0.0.9 is alive'
	

