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
imx qemu [qemu args]   # boot the kernel + rootfs in QEMU, see below
imx qemu debug         # same, paused, gdb server on :3334 (IMX_GDB_PORT)
imx gdb                # arm gdb on out/vmlinux, attached to imx qemu debug
imx module [dir]       # build an out-of-tree module (default modules/hello)
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

### Running in QEMU (`imx qemu`)

`imx qemu` boots `zImage` and `rootfs-porting/out/rootfs.cpio.gz` on QEMU's
`mcimx6ul-evk` machine (`brew install qemu`), with the serial console in the
terminal. Press Enter for a shell. To quit, type `reboot` (QEMU runs with
`-no-reboot`, so a reboot exits) or press `Ctrl-A X`; `Ctrl-A C` toggles the
QEMU monitor. Extra arguments go to QEMU, e.g. `imx qemu -s -S` waits for gdb
on port 1234. kdb works too (`kgdboc=ttymxc0` is passed).

QEMU emulates the i.MX6**UL**, not the 6ULL, and not all of it:

- It uses `qemu/imx6ul-qemu.dts`, NXP's 6UL EVK tree plus the Cortex-A7
  architected timer, with `gpt1` disabled (QEMU's GPT model runs at 0 Hz). The
  Alientek board tree can't be used: it needs 6ULL-only blocks such as the
  SNVS IOMUXC at 0x02290000. `imx` builds it into `out/imx6ul-qemu.dtb`.
- `drivers/clk/clk.c` has a bounds check for mux parent indices: QEMU's clock
  controller returns an out-of-range mux value, and without the check the
  kernel crashes in `imx6ul_clocks_init` before the console is up.
- No LCD, touch, camera, SD/eMMC (hence harmless `mmc0: Timeout` messages).

So QEMU tests the kernel core and the rootfs; board-specific hardware still
needs the real board.

### Single-stepping the boot (`imx qemu debug` + `imx gdb`)

```sh
imx qemu debug    # terminal 1: QEMU waits for gdb before running anything
imx gdb           # terminal 2: gdb on out/vmlinux, stopped at stext
```

The defconfig has `CONFIG_DEBUG_INFO=y`, and every kernel build copies
`vmlinux` (symbols + DWARF) to `out/`. `imx gdb` uses `arm-none-eabi-gdb` (Arm
GNU Toolchain), maps the Docker build path to the source tree on the Mac, and
sets these breakpoints (VS Code sets the same ones), in boot order:

| Breakpoint | Where | Stops when |
|------------|-------|------------|
| `stext` | `arch/arm/kernel/head.S` | first kernel instruction, MMU still off |
| `start_kernel` | `init/main.c` | first C function |
| `populate_rootfs` | `init/initramfs.c` | the initramfs (`rootfs.cpio.gz`) is unpacked (rootfs initcall) |
| `prepare_namespace` | `init/do_mounts.c` | a disk rootfs is mounted from `root=`; only without an initramfs `/init`, so not hit with `imx qemu` |
| `run_init_process` | `init/main.c` | `/init` is executed: the kernel hands over to the rootfs |

The list is `BOOT_BREAKPOINTS` in `imx` and `postRemoteConnectCommands` in
`.vscode/launch.json`. Then:

| gdb | Does |
|-----|------|
| `si` / `ni` | one instruction (into / over calls) |
| `b start_kernel`, `c` | run to the C entry point (`init/main.c`) |
| `s` / `n` | one source line (into / over calls) |
| `bt`, `info registers`, `x/8i $pc` | stack, registers, disassembly |
| `layout src` / `layout asm` | source / assembly view |

The zImage decompressor writes the kernel over software breakpoints, so
`imx gdb` (and the VS Code config) mark the kernel image `0x80008000`-
`0x80990000` read-only for gdb (`mem ... ro`); gdb then uses hardware
breakpoints there automatically, and plain `break` works. If the kernel grows
past `__init_end` (see `System.map`), raise that end address. Kernel symbols work
before the MMU is on because `PAGE_OFFSET` (0x80000000) equals the DDR base, so
virtual and physical kernel addresses are the same. The kernel is built with
`-O2`, so `n` sometimes jumps around and some variables show as optimized out.

The gdb server uses port 3334, not QEMU's default 1234, so it doesn't clash with
other QEMU sessions.

**In VS Code:** Run and Debug → **Kernel boot (QEMU)** (F5). `.vscode/tasks.json`
starts `imx qemu debug` in a terminal (that terminal is the serial console) and
`.vscode/launch.json` attaches the Microsoft C/C++ debugger (`cppdbg`) with
`arm-none-eabi-gdb`, stops at `stext`, and maps the build paths to the source.
Click in the margin for breakpoints, F10/F11 to step, and use the Debug Console
for gdb commands (`-exec si`, `-exec x/8i $pc`). Stopping the session stops
QEMU. Run `imx` first after kernel changes; the debugger uses `out/vmlinux`.

### Kernel modules (`imx module`)

Our own modules live outside the kernel tree, one folder each under
`modules/` with a Kbuild `Makefile` (`obj-m := hello.o`). `imx module
modules/hello` builds it in Docker against the kernel build tree of the last
`imx` (a module must match that kernel's config and symbol versions, so run
`imx` first after kernel config changes) and puts `hello.ko` in
`modules/hello/out/` and in the kernel's `out/ko/`.

`imx qemu` gives the guest a network (QEMU user mode on ENET1 = `eth1`,
`10.0.2.15`; the Mac is `10.0.2.2`) and serves `out/ko/` with QEMU's built-in
TFTP server, so a rebuilt module is loaded without rebuilding the rootfs:

```sh
imx module                     # on the Mac
# in QEMU:
cd /tmp && tftp -g -r hello.ko 10.0.2.2
insmod hello.ko who=qemu       # hello: hello, qemu!
cat /sys/module/hello/parameters/who
rmmod hello                    # hello: goodbye, qemu
```

Kernel builds also install the kernel's own loadable modules (`=m` options)
into `out/modules/`, and `imx rootfs` copies them to `/lib/modules/4.1.15/` in
the rootfs. BusyBox's `depmod` on the target enables `modprobe`.

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
