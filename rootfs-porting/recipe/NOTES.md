# Untar busybox-1.29.0

	$ tar -vxjf busybox-1.29.0.tar.bz2

# Edit Makefile, change the following to:

		CROSS_COMPILE ?= arm-linux-gnueabihf-
		ARCH ?= arm

# Now configure busybox

	$ make menuconfig (can also use make allyesconfig / make allnoconfig to select / unselect everything.)

	1. Do not Build static library. Since we will boot from nfs, static library will have dependency DNS parsing failure.

	-> Settings
		-> Build static binary (no shared libs) ==> UNselect
	
	2. -> Settings
		-> vi-style line editing commands	==> select

	3. -> Linux Module Utilities
			-> Simplified modutils ==> UNselect

	4. -> Linux System Utilities
			-> mdev (16 kb) ==> select all below

				[*] Support /etc/mdev.conf                                                                                     
				[*]		Support subdirs/symlinks                                                                                 
				[*]			Support regular expressions substitutions when renaming device                                         
				[*]		Support command execution at device addition/removal                                                     
				[*]	Support loading of firmware                                                                                

	Exit > Exit >  Do you wish to save your new configuration? ==> Yes.

# Compile busybox

	$ make

	$ make install CONFIG_PREFIX=/home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs

	When successfully install, expect:


		......
	/home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs//usr/sbin/svlogd -> ../../bin/busybox
  /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs//usr/sbin/telnetd -> ../../bin/busybox
  /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs//usr/sbin/tftpd -> ../../bin/busybox
  /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs//usr/sbin/ubiattach -> ../../bin/busybox
  /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs//usr/sbin/ubidetach -> ../../bin/busybox
  /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs//usr/sbin/ubimkvol -> ../../bin/busybox
  /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs//usr/sbin/ubirename -> ../../bin/busybox
  /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs//usr/sbin/ubirmvol -> ../../bin/busybox
  /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs//usr/sbin/ubirsvol -> ../../bin/busybox
  /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs//usr/sbin/ubiupdatevol -> ../../bin/busybox
  /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs//usr/sbin/udhcpd -> ../../bin/busybox


	--------------------------------------------------
	You will probably need to make your busybox binary
	setuid root to ensure all configured applets will
	work properly.
	--------------------------------------------------

# Add share libraries .so files to /lib  directcory
	$ mkdir /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs/lib

	$ cd /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/libc/lib/

	$ cp *so* *.a /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs/lib/ -d

	$ rm /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs/lib/ld-linux-armhf.so.3

	$ cp ld-linux-armhf.so.3 /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs/lib/

	$ cd /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/libc/lib

	$ cp *so* *.a /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs/lib/ -d

# Add share libraries .so files to .usr/lib directory

	$ mkdir /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs/usr/lib/ 

	$ cd /usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/arm-linux-gnueabihf/libc/usr/lib

	$ cp *so* *.a /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs/usr/lib/ -d

	$ cd /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs

	$ du ./lib ./usr/lib/ -sh

	# Expect:

		57M ./lib
		67M ./usr/lib

# Create other folders:

	$ cd /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs

	$ mkdir dev proc mnt sys tmp root 

	(!) Do not create etc folder yet! If you create it now there will be problems at booting up such as "open /dev/console : No such file or directory"

	Boot up first time and we should expect:

		VFS: Mounted root (nfs filesystem) on device 0:15.
		devtmpfs: mounted
		Freeing unused kernel memory: 400K (8090e000 - 80972000
		can't run '/etc/init.d/rcS': No such file or directory

		Please press Enter to activate this console. 
		/ # ls

		bin      lib      mnt      root     sys      usr
		dev      linuxrc  proc     sbin     tmp


# Create startup scripts in imx6ull console instead (not on ubuntu 16.04 anymore).

	$ mkdir /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs/etc/init.d/

	$ touch /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs/etc/init.d/rcS

		#!/bin/sh
		PATH=/sbin:/bin:/usr/sbin:/usr/bin:$PATH
		LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lib:/usr/lib
		export PATH LD_LIBRARY_PATH
		mount -a
		mkdir /dev/pts
		mount -t devpts devpts /dev/pts

		echo /sbin/mdev > /proc/sys/kernel/hotplug
		mdev -s

	$ touch /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs/etc/fstab

		#<file system> <mount point> <type> <options> <dump> <pass>
		proc /proc proc defaults 0 0
		tmpfs /tmp tmpfs defaults 0 0
		sysfs /sys sysfs defaults 0 0

	$ touch /home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs/etc/inittab

		#etc/inittab
		::sysinit:/etc/init.d/rcS
		console::askfirst:-/bin/sh
		::restart:/sbin/init
		::ctrlaltdel:/sbin/reboot
		::shutdown:/bin/umount -a -r
		::shutdown:/sbin/swapoff -a

	$ sudo chmod +x rcS  # We have to give execution permission to rcS! 


# Create /lib/modules/4.1.15 for storing LKM modules.

	$ mkdir -p /lib/modules/4.1.15



	
