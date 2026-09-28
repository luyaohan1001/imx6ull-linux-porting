# Install curses library for menuconfig

	$ sudo apt update
	$ sudo apt install libncurses-dev
	$ apt install vim-gtk3
	$ apt install kmod
	$ apt install net-tools
	$ apt install ethtool
	$ apt install ifupdown
	$ apt install language-pack-en-base
	$ apt install rsyslog
	$ apt install htop
	$ apt install iputils-ping
	$ apt install cutecom

	# ftp related tools
	$ apt install vsftpd
	$ apt install nfs-kernel-server rpcbind


0. linux-imx-rel_imx_4.1.15_2.1.0_ga.tar.bz2 is the official kernel for NXP I.MX6ULL EVK development board. We will port the kernel based on this release.

1. Make our own version

		$ tar -xvf linux-imx-rel_imx_4.1.15_2.1.0_ga.tar.bz2

		$ mv linux-imx-rel_imx_4.1.15_2.1.0_ga linux-imx-rel_imx_4.1.15_2.1.0_luyaohan1001 (or linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek)

2. Edit the Makefile

		# Line 252, add

		ARCH ?= arm
		CROSS_COMPILE ?= arm-linux-genueabihf

3. In linux-imx-rel_imx_4.1.15_2.1.0_ga_luyaohan1001

		$ make clean 

		$ make imx_v7_mfg_defconfig # configure the kernel

		# Expect:
			......
			configuration writtento .config 
			......


4. Compile the kernel

		$ make -j16

		# When finished, expect something like:
		...
		AS      arch/arm/boot/compressed/piggy.lzo.o
  	LD      arch/arm/boot/compressed/vmlinux
  	OBJCOPY arch/arm/boot/zImage
  	Kernel: arch/arm/boot/zImage is ready


	
