# Add device file tree for our own dev board.
	$ cd arch/arm/boot/dts

	# imx6ull-14x14-evk.dts is the device tree source for our evaluation board.
	$ cp imx6ull-14x14-evk.dts imx6ull-alientek-emmc-luyaohan1001.dts

	We will edit imx6ull-alientek-emmc-luyaohan1001.dts for our own projects.

	# To compile device tree files

	$ make dtbs


	# We also need to change arch/arm/boot/dts/Makefile, around line 400

			........
			imx6ul-9x9-evk-ldo.dtb
		dtb-$(CONFIG_SOC_IMX6ULL) += \
			imx6ull-14x14-ddr3-arm2.dtb \
			imx6ull-14x14-ddr3-arm2-adc.dtb \
			........
			imx6ull-14x14-evk-gpmi-weim.dtb \
			imx6ull-14x14-evk-usb-certi.dtb \
			imx6ull-alientek-emmc-luyaohan1001.dtb \
			imx6ull-9x9-evk.dtb \
			imx6ull-9x9-evk-btwifi.dtb \
			imx6ull-9x9-evk-ldo.dtb
		dtb-$(CONFIG_SOC_IMX6SLL) += \
			imx6sll-lpddr2-arm2.dtb \
			........

	# After 'make' we will get the imx6ull-alientek-emmc-luyaohan1001.dtb from the source imx6ull-alientek-emmc-luyaohan1001.dts


# To compile and get kernel zImage:

	$ chmod 777 imx6ull_alientek_emmc.sh 

	$ ./imx6ull_alientek_emmc.sh 

# After compilation, the 'useful' files:
	./arch/arm/boot/zImage
	./arch/arm/boot/dts/immx6ull-alientek-emmc-luyaohan1001.dtb

	
			
