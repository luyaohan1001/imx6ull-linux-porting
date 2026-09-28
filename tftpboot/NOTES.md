# Setup tftp on Ubuntu 16.04

	$ sudo apt-get install tftp-hpa tftpd-hpa

	$ sudo apt-get install xinetd

# Make a local folder for storing things we need to transfer using TFTP

	$ mkdir /home/luyaohan1001/Projects/imx6ull-linux-porting/tftpboot

	$ chmod 777 /home/luyaohan1001/Projects/imx6ull-linux-porting/tftpboot

# Configure TFTP on ubuntu 16.04

	$ sudo vim /etc/xinetd.d/tftp

	# Add the following:

		server tftp
		{
			socket_type = dgram
			protocol = udp
			wait = yes
			user = root
			server = /usr/sbin/in.tftpd
			server_args = -s /home/luyaohan1001/Projects/imx6ull-linux-porting/tftpboot/ 		#(!) Must have the last '/' forward slash!
			disable = no
			per_source = 11
			cps = 100 2
			flags = IPv4
		}

	--------

	$ sudo vim /etc/default/tftpd-hpa

	# Enter the following:

		TFTP_USERNAME="tftp"
		TFTP_DIRECTORY="/home/luyaohan1001/Projects/imx6ull-linux-porting/tftpboot/"
		TFTP_ADDRESS=":69"
		TFTP_OPTIONS="-l -c -s"

# Restart tftp service

	$ sudo service tftpd-hpa start

# Start the kernel using tftp. This will transfer location zImage and .dtb files to emmc and start from there.:

	# tftp <address> <file>
	=> tftp 80800000 zImage 
	=> tftp 83000000 imx6ull-alientek-emmc-luyaohan1001.dtb
	=> bootz 80800000 - 83000000

	In order to make less type, we can save certain boot command as environment variable, and use 'run' command to either boot from Net or EMMC.

	=> setenv mybootemmc 'fatload mmc 0:1 80800000 zimage; fatload mmc 0:1 83000000 imx6ull-14x14-evk.dtb;bootz 80800000 - 8300000'

	=> setenv mybootnet 'tftp 80800000 zImage; tftp 83000000 imx6ull-alientek-emmc-luyaohan1001.dtb; bootz 80800000 - 83000000'

	=> saveenv

	At next bootup, or after reset, simply run

	=> run mybootemmc

	=> run mybootnet


The tftp needs to be used in putty (ttyusb serial terminal.)
This version of uboot has been test on Ubuntu 16.04 through USB-TTY on putty.


	Note that the IPV4 static address on Ubuntu host also needs to be set as:

	$ In Ubuntu >

	IPv4 addr: 10.0.0.111
	Netmask:   255.255.255.0
	Gateway:   10.0.0.1

1. Connect both USB TTL and USB OTG cable.
2. Check if CH340 driver detects the connect of the board:
	$ dmesg | grep tty

	# expect 'ttyUSB0 is connected...'

3. Flash the SD card. After uboot is loaded after reset, type 'Enter' in Cutecom to allow command prompt to uboot. 

4. Setup the following to allow internet connections between the board the the ubuntu host when the two are connected with Ethernet cable.
5. On ubuntu host, go to connections. Disable IPv6, enable IPv4 and use manual ip address assignment.
6. The address on ubuntu can be, for example 10.0.0.1, the netmask is 255.255.255.0, gateway can be empty.
7. The environment variables in uboot needs to be setup similar to this:

	$ putty >

				setenv ipaddr 10.0.0.200                  ==>  imx6ull ip (slave)
				setenv ethaddr b8:ae:1d:01:00:00          ==>  mac address of alientek board
				setenv gatewayip 10.0.0.1                 ==>  gateway 
				setenv netmask 255.255.255.0              ==>  netmask
				setenv serverip 10.0.0.111                ==>  ip address of the ubuntu host
				saveenv				                            ==>  save the variables


8. Now we try to ping the host on the board:
	$ putty > ping 10.0.0.111 
	# expect:
		'FEC1 waiting for PHY auto negotiation to complete...done'
		'Using FEC1 device'
		'host 10.0.0.111 is alive'
	


