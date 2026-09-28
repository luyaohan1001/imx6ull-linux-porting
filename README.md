# i.MX6ULL Linux porting (Alientek eMMC board)

Porting NXP's `rel_imx_4.1.15_2.1.0_ga` release (Linux 4.1.15, U-Boot 2016.03)
from the NXP MCIMX6ULL-EVK to the 正点原子 (Alientek) i.MX6ULL eMMC board, with
a BusyBox root filesystem. Everything is built on macOS (Apple Silicon)
through the `./imx` tool.

## Layout

| Path | What |
|------|------|
| `imx` | Build / load tool (Python). Alias: `imx` (in `~/.zshrc`) |
| `Dockerfile.imx` | Build container: Ubuntu 16.04, `arm-linux-gnueabihf-gcc` 5.4 |
| `kernel-porting/ported/linux-imx-…_alientek/` | The board kernel |
| `uboot-porting/ported/uboot-imx-…_alientek/` | The board U-Boot |
| `rootfs-porting/ported/busybox-1.29.0/` | BusyBox (hand-tuned `.config`) |
| `rootfs-porting/overlay/` | Files copied into the rootfs (`/etc/inittab`, `fstab`, `init.d/rcS`) |
| `*/recipe/` | Original tarballs and the porting notes (`NOTES.md`) |
| `nxp-evk-reference/` | Untouched NXP EVK sources, for diffing ([README](nxp-evk-reference/README.md)) |
| `datasheets/` | One subfolder per document, with its source link ([README](datasheets/README.md)) |
| `tftpboot/`, `nfs/`, `flashing-linux-sd/` | Network boot and SD-card notes from the earlier Ubuntu setup |

## Building on macOS

### Why it needs Docker

A native build on macOS fails for two reasons:

- **Case-insensitive filesystem.** The kernel has files whose names differ only
  in case (`net/netfilter/xt_DSCP.c` and `xt_dscp.c`, `xt_HL.c` / `xt_hl.c`,
  and headers such as `xt_CONNMARK.h` / `xt_connmark.h`; 12 pairs). On APFS
  they overwrite each other, which is why they always show as modified in
  `git status`. Never commit them, and don't try to fix them with
  `git checkout`.
- **No ARM toolchain.** macOS has no `arm-linux-gnueabihf-gcc`, and Linux 4.1
  does not build with modern GCC.

### How `imx` works

`imx` is a small Python script that does not compile anything itself:

1. `docker build` makes the image `imx6ull-kernel-builder` from `Dockerfile.imx`
   (cached after the first time).
2. `docker run` starts a throwaway container with three mounts:
   - `/repo`: this repository, **read-only**
   - `/build`: the Docker volume `imx6ull-kernel-src`, a **case-sensitive** disk
     that persists between runs
   - `/out`: `<tree>/out/` on the Mac, where results come back
3. Inside, a bash script `rsync`s the source tree into `/build`, rewrites the
   case-colliding files from the git index (so each one gets its real
   content), then runs `make`.
4. The results are copied to `<tree>/out/`.

### Commands

```sh
imx                    # kernel: distclean + imx_alientek_emmc_defconfig + build
imx menuconfig         # same, with menuconfig first (also writes out/defconfig)
imx dtbs               # kernel: device tree only
imx uboot              # U-Boot: mx6ull_alientek_emmc_defconfig + build
imx rootfs             # BusyBox + glibc + overlay -> rootfs.tar.gz, uRamdisk
imx rootfs menuconfig  # same, with BusyBox menuconfig first
imx compdb             # clangd compile_commands.json (imx / imx uboot do it too)
imx load [ramdisk|emmc|prompt]   # boot the board over USB, see below
```

| Output | Where |
|--------|-------|
| `zImage`, `imx6ull-alientek-emmc-luyaohan1001.dtb`, `config` | `kernel-porting/ported/linux-imx-…_alientek/out/` |
| `u-boot.imx`, `u-boot.bin` | `uboot-porting/ported/uboot-imx-…_alientek/out/` |
| `rootfs.tar.gz`, `rootfs.cpio.gz`, `uRamdisk`, `busybox.config` | `rootfs-porting/out/` |

Config changes made in `imx menuconfig` are only kept if you copy
`out/defconfig` over `arch/arm/configs/imx_alientek_emmc_defconfig`; every full
build starts from the defconfig.

### Booting the board (`imx load`)

Nothing is written to eMMC: U-Boot, the kernel, the device tree and the rootfs
go straight into RAM over USB.

1. Boot switches to USB download mode (`01000000`), USB OTG and USB-TTL cables
   in, power on.
2. `imx load` then:
   - runs `sudo uuu`, which writes U-Boot, `zImage` (0x80800000), the dtb
     (0x83000000) and `uRamdisk` (0x83800000) into RAM, and stops it once
     `SDP: jump` is done (uuu would otherwise wait forever for a next stage)
   - opens the serial console (`/dev/cu.usbserial-*`, override with
     `IMX_SERIAL`), stops U-Boot's 3-second autoboot and types `setenv bootargs`
     and `bootz`
   - hands the terminal to `screen` (quit with `Ctrl-A K Y`)

`imx load` boots our BusyBox rootfs from RAM; `imx load emmc` uses the root
filesystem already on eMMC partition 2; `imx load prompt` stops at `=>`. Never
type `boot` at the U-Boot prompt: it loads the old kernel from eMMC over the one
just sent.

### Code navigation (clangd)

`imx compdb` turns the build's `*.o.cmd` files into `compile_commands.json` in
the kernel and U-Boot trees. It maps container paths to the Mac, copies
headers that exist only in the build tree (`autoconf.h`, `asm-offsets.h`,
U-Boot's `asm/arch`, …) and the cross compiler's own headers to
`<tree>/out/lsp/`, and drops gcc-only flags. `.vscode/settings.json` turns on
clangd and turns off the Microsoft C/C++ IntelliSense. Only files compiled in
the current config have exact flags.

## KDB (kernel debugger)

### How it was enabled

1. Kernel config, in `arch/arm/configs/imx_alientek_emmc_defconfig`:

   ```
   CONFIG_KGDB=y
   CONFIG_KGDB_SERIAL_CONSOLE=y
   CONFIG_KGDB_KDB=y
   ```

   `CONFIG_MAGIC_SYSRQ=y` was already on. `KGDB` alone only gives the gdb
   stub; without `KGDB_KDB` the kernel waits silently for gdb after a break.
2. Boot arguments: `imx load` passes `kgdboc=ttymxc0,115200`, so kdb shares the
   serial console.

### Using it

```sh
cat /sys/module/kgdboc/parameters/kgdboc   # ttymxc0,115200
echo g > /proc/sysrq-trigger               # stop the kernel, get kdb>
```

| kdb command | Does |
|-------------|------|
| `help` | list commands |
| `ps`, `bt`, `btp <pid>`, `bta` | processes and stack traces |
| `md <addr or symbol>`, `mm` | read / modify memory |
| `rd` | registers |
| `bp <symbol>`, `bl`, `bc <n>` | set / list / clear breakpoints (e.g. `bp sys_sync`, then run `sync`) |
| `dmesg`, `lsmod`, `summary` | kernel log, modules, overview |
| `go`, `reboot` | resume / reboot |

With kdb built in, an oops or `echo c > /proc/sysrq-trigger` stops in kdb
instead of just panicking. For source-level debugging with gdb from the Mac,
also enable `CONFIG_DEBUG_INFO` and switch with the kdb `kgdb` command.

## SSH

**Not enabled yet.** Current state:

- The rootfs has no SSH server; BusyBox has `telnetd`, `ifconfig`/`ip`,
  `udhcpc` and `login`, but no `sshd`.
- The kernel has the FEC Ethernet driver but no LAN8720 PHY driver
  (`CONFIG_SMSC_PHY`); the generic PHY driver is used.
- The device tree still has the EVK's PHY addresses (`ethphy0` = 2,
  `ethphy1` = 1). U-Boot, which works, uses 0 and 1, so ENET1 is likely broken
  in Linux.
- The device tree still has the EVK's 74HC595 `gpio_spi` node on GPIO5_IO07/08,
  which are the Alientek board's Ethernet PHY reset lines.

Quick test, unencrypted, on your own network only: bring up the port U-Boot
uses (ENET2) with `ifconfig ethN <ip> up` and start `telnetd -l /bin/sh`; from
the Mac, use `telnet` (from Homebrew) or `nc <ip> 23`.

Plan to enable SSH:

1. Device tree: `ethphy0` `reg` 2 -> 0, remove the `gpio_spi` / `spi4` node;
   optionally `CONFIG_SMSC_PHY=y`.
2. Cross-compile Dropbear in the build container and add it to the rootfs, with
   host keys generated at build time (the rootfs lives in RAM, so keys made on
   the board would change every boot), `/etc/passwd`, and the Mac's public key
   in `/root/.ssh/authorized_keys`.
3. In `rootfs-porting/overlay/etc/init.d/rcS`: configure the interface (static
   IP or `udhcpc`) and start `dropbear`.
