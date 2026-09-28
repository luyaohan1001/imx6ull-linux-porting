This folder contains the board-support-package (BSP) from the NSP. This is the u-boot version NSP maintained for imx6ull.

The uboot maintained by NXP is uboot-imx-rel_imx_4.1.15_2.1.0_ga.tar.bz2

# This uboot can be perfectly ran on alientek IMX6ULL Alpha development board. We can try to experiment with it:

	$ touch autobot.sh

	$ vim autobot.sh
	#!/bin/bash
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- distclean
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- mx6ull_14x14_evk_emmc_defconfig
	make V=1 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4

	# Now compile!

	$ make mx6ull_14x14_evk_emmc_defconfig 

	$ sudo chmod +x ./autobot.sh

	$ ./autobot.sh
	
	Expect some long log and in the end:
	......
	e 0x87800000 -d u-boot.bin u-boot.imx 
	Image Type:   Freescale IMX Boot Image
	Image Ver:    2 (i.MX53/6/7 compatible)
	Mode:         DCD
	Data Size:    425984 Bytes = 416.00 kB = 0.41 MB
	Load Address: 877ff420
	Entry Point:  87800000


	# After the compilation, we will find u-boot.bin and u-boot.imx 

	# Insert SD card, download the .bin file to the sd card

	$ sudo fdisk -l # This allows us to know which /dev/sd<x> is the SD card.

	# Flash to SD card

	$ ./imxdownload u-boot.bin /dev/sd<x>

	# Now plugin the sd card to the development board.

	Once boot, expect the following log from serial terminal (cutecom or putty):

	U-Boot 2016.03 (Mar 27 2022 - 20:50:34 -0400)

	CPU:   Freescale i.MX6ULL rev1.1 69 MHz (running at 396 MHz)
	CPU:   Industrial temperature grade (-40C to 105C) at 57C
	Reset cause: POR
	Board: MX6ULL 14x14 EVK
	I2C:   ready
	DRAM:  512 MiB
	MMC:   FSL_SDHC: 0, FSL_SDHC: 1
	unsupported panel TFT7016
	In:    serial
	Out:   serial
	Err:   serial
	switch to partitions #0, OK
	mmc0 is current device
	Net:   Board Net Initialization Failed
	No ethernet found.
	Normal Boot
	Hit any key to stop autoboot:  

	# And we should hit key here.

	=> mmc list 

		FSL_SDHC: 0 (SD)
		FSL_SDHC: 1

	# The second one is the emmc.

	# To look at info on the SD card, run

	=> mmc dev 0 
	=> mmc info

	# To look at info on the emmc, run

	=> mmc dev 1
	=> mmv info

# We have tried to play with the uboot maintained by NXP. We have to make out own uboot modifications. 

	$ cp -r ../../nxp-evk-reference/uboot-imx-rel_imx_4.1.15_2.1.0_ga_nxp uboot-imx-rel_imx_4.1.15_2.1.0_ga_alientek   # from uboot-porting/ported/

	$ cd uboot-imx-rel_imx_4.1.15_2.1.0_ga_nxp_alientek

	# We need our own configurations at compilation.

	$ cd configs

	$ cp mx6ull_14x14_evk_emmc_defconfig mx6ull_alientek_emmc_defconfig

	Change the according content to the following:

		CONFIG_SYS_EXTRA_OPTIONS="IMX_CONFIG=board/freescale/mx6ull_alientek_emmc/imximage.cfg,MX6ULL_EVK_EMMC_REWORK"
		CONFIG_ARM=y
		CONFIG_ARCH_MX6=y
		CONFIG_TARGET_MX6ULL_ALIENTEK_EMMC=y
		CONFIG_CMD_GPIO=y

	# In the include/configs

	$ cd ./include/configs

	$ cp include/configs/mx6ullevk.h mx6ull_alientek_emmc.h

	Find the following: 

	#ifndef __MX6ULLEVK_CONFIG_H

	#define __MX6ULLEVK_CONFIG_H

	And change to:

	#ifndef __MX6ULL_ALIENTEK_EMMC_CONFIG_H

	#define __MX6ULL_ALIENTEK_EMMC_CONFIG_H

	# Now we need to add the board support file.

	$ cd board/freescale/

	$ cp mx6ullevk/ -r mx6ull_alientek_emmc

	$ cd mx6ull_alientek_emmc

	$ mv mx6ullevk.c mx6ull_alientek_emmc.c

	# Edit the Makefile

	$ vim Makefile

	Change the entire Makefile to the following:

		# (C) Copyright 2015 Freescale Semiconductor, Inc.
		#
		# SPDX-License-Identifier: GPL-2.0+
		#
		obj-y := mx6ull_alientek_emmc.o
		extra-$(CONFIG_USE_PLUGIN) := plugin.bin
		$(obj)/plugin.bin: $(obj)/plugin.o
		$(OBJCOPY) -O binary --gap-fill 0xff $< $@

	# Under mx6ull_alientek_emmc dir, edit the imximage.cfg file

	$ vim imximage.cfg

	Find :
		PLUGIN board/freescale/mx6ullevk/plugin.bin 0x00907000
	And change to:
		PLUGIN board/freescale/mx6ull_alientek_emmc /plugin.bin 0x00907000

	# Under mx6ull_alientek_emmc dir, edit Kconfig file

		if TARGET_MX6ULL_ALIENTEK_EMMC

		config SYS_BOARD
			default "mx6ull_alientek_emmc"

		config SYS_VENDOR
			default "freescale"

		config SYS_SOC
			default "mx6"

		config SYS_CONFIG_NAME
			default "mx6ull_alientek_emmc"

		endif


	# Under mx6ull_alientek_emmc dir, edit the MAINTAINERS file

		MX6ULL_ALIENTEK_EMMC BOARD
		M: Luyao Han <luyaohan1001@gmail.com>
		S: Maintained
		F: board/freescale/mx6ull_alientek_emmc/
		F: include/configs/mx6ull_alientek_emmc.h

	# Edit Kconfig

		$ vim arch/arm/cpu/armv7/mx6/Kconfig

		At line 207, add the following:

		config TARGET_MX6ULL_ALIENTEK_EMMC
			bool "Support mx6ull_alientek_emmc"
			select MX6ULL
			select DM
			select DM_THERMAL

		Before the last line of endif, add this single line:

		source "board/freescale/mx6ull_alientek_emmc/Kconfig"


	# Now we can compile our own u-boot

		$ touch autobot.sh

		$ vim autobot.sh

		Add the following lines:

			#!/bin/bash
			make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- distclean
			make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- mx6ull_alientek_emmc_defconfig
			make V=1 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4

		$ chmod +x autobot.sh

		$ ./autobot.sh

		# After the compilation we can 'grep' our own include header file:

		$ grep -nR "mx6ull_alientek_emmc.h

		If we can see this header file is used at a lot of places, it means the new board has been added successfully.


# Change the LCD display parameters:

		The LCD display parameters is changed in the following file:

		./board/freescale/mx6ull_alientek_emmc/mx6ull_alientek_emmc.c

		edit the following struct:

			struct display_info_t const displays[] = {{
				.bus = MX6UL_LCDIF1_BASE_ADDR,
				.addr = 0,
				.pixfmt = 24,
				.detect = NULL,
				.enable	= do_enable_parallel_lcd,
				.mode	= {
					.name			= "ATK-LCD-4.3-800x480",
					.xres           = 800,
					.yres           = 480,
					.pixclock       = 30303,
					.left_margin    = 88,
					.right_margin   = 40,
					.upper_margin   = 32,
					.lower_margin   = 13,
					.hsync_len      = 48,
					.vsync_len      = 3,
					.sync           = 0,
					.vmode          = FB_VMODE_NONINTERLACED
			} } };

	Change the following in line 775, ~/Projects/linux/IMX6ULL/linux-os-porting/kernel-porting/ported/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek/arch/arm/boot/dts/imx6ull-alientek-emmc-luyaohan1001.dts:

			&lcdif {
				pinctrl-names = "default";
				pinctrl-0 = <&pinctrl_lcdif_dat
							 &pinctrl_lcdif_ctrl>;
				display = <&display0>;
				status = "okay";

							display0: display {
											bits-per-pixel = <16>;
											bus-width = <24>;

											display-timings {
															native-mode = <&timing0>;
															timing0: timing0 {
															clock-frequency = <35500000>;
															hactive = <800>;
															vactive = <480>;
															hfront-porch = <210>;
															hback-porch = <46>;
															hsync-len = <20>;
															vback-porch = <23>;
															vfront-porch = <22>;
															vsync-len = <3>;

															hsync-active = <0>;
															vsync-active = <0>;
															de-active = <1>;
						/* rgb to hdmi: pixelclk-ative should be set to 1 */
															pixelclk-active = <0>;
															};
											};
							};
			};


		
	# Note that the name of the display has been changed to "ATK-LCD-4.3-800x480". If we don't change the environment variable in u-boot startup console, we will find something like 'unsupported panel TFT43AB'.

	At the uboot startup console, enter 'Enter' at the 3 seconds countdown.

	=> setenv panel ATK-LCD-4.3-800x480

	=> saveenv

	This environment variable will be saved to the emmc (not SD card).

	Now reboot and we should expect following on the LCD:
		
	A large NXP logo (only one) and 'U-Boot 2016.03 (Mar 28 2022...)'


