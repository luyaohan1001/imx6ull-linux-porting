# This folder holds the compressed kernel (zImage) and the device tree binary. The files are transferred to IMX6ULL emmc and then boot from EMMC.
# Different from tftp, which holds the rootfs mounted in real time and use resource in real time, nfs is more static kind of way just to transfer files.
# You need to setup tftp first because kernel starts before mounting the filesystem.

# Setup nfs on ubuntu 16.04:

	$ sudo vim /etc/exports

		add the following:

		/home/luyaohan1001/Projects/imx6ull-linux-porting/nfs *(rw,sync,no_root_squash)

	# Restart the NFS service

		$ sudo /etc/init.d/nfs-kernel-server restart

# When transferring, on putty / screen:

	# nfs <address-on-sd-card> <ip-addr-of-ubuntu-host>:<absolute-path-of-file-to-transfer>

# Setup environment variable for bootargs so that the rootfs is to be specified located at ubuntu through nfs. (TFTP) is required.

	# console=tty1 would set LCD as console
	# console=ttymxc0,115200 set serial USB as second console

	=> setenv bootargs 'console=tty1 console=ttymxc0,115200 root=/dev/nfs nfsroot=10.0.0.111:/home/luyaohan1001/Projects/imx6ull-linux-porting/nfs/rootfs,proto=tcp rw ip=10.0.0.200:10.0.0.111:10.0.0.1:255.255.255.0::eth0:off'

	=> saveenv

	


