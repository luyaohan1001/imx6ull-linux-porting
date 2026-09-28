# NXP i.MX6ULL EVK reference sources

Untouched NXP sources for the official i.MX 6ULL evaluation kit
(**MCIMX6ULL-EVK**), kept only to compare against. Don't edit or build them.

- Board: [Evaluation Kit for the i.MX 6ULL and 6ULZ Applications Processor (MCIMX6ULL-EVK)](https://www.nxp.com/design/design-center/development-boards-and-designs/i-mx-evaluation-and-development-boards/evaluation-kit-for-the-i-mx-6ull-and-6ulz-applications-processor:MCIMX6ULL-EVK)
- Release: NXP `rel_imx_4.1.15_2.1.0_ga` (Linux 4.1.15, U-Boot 2016.03)

| Folder | What it is | Ported copy (the one we build) |
|--------|------------|--------------------------------|
| `linux-imx-rel_imx_4.1.15_2.1.0_ga_nxp_evk/` | NXP Linux kernel | `kernel-porting/ported/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek/` |
| `uboot-imx-rel_imx_4.1.15_2.1.0_ga_nxp/` | NXP U-Boot | `uboot-porting/ported/uboot-imx-rel_imx_4.1.15_2.1.0_ga_alientek/` |

The original tarballs are in `kernel-porting/recipe/` and `uboot-porting/recipe/`.

## What the Alientek port changed

The Alientek trees are these trees plus a few board files. No kernel driver or
core file was modified.

**Kernel**

| Alientek file | Started from (EVK) | Change |
|---------------|--------------------|--------|
| `arch/arm/boot/dts/imx6ull-alientek-emmc-luyaohan1001.dts` | `arch/arm/boot/dts/imx6ull-14x14-evk.dts` | LCD, touch, GPIO1_IO09, experiment nodes (below) |
| `arch/arm/configs/imx_alientek_emmc_defconfig` | `arch/arm/configs/imx_v7_mfg_defconfig` | identical copy |
| `arch/arm/boot/dts/Makefile` | same file | builds the new `.dtb` |

**U-Boot**

| Alientek file | Started from (EVK) | Change |
|---------------|--------------------|--------|
| `board/freescale/mx6ull_alientek_emmc/` | `board/freescale/mx6ullevk/` | no 74LV595 I/O expander; Ethernet PHY reset on GPIO5_IO07/08; ATK 800x480 panel; board banner |
| `.../mx6ull_alientek_emmc/imximage.cfg` (DDR init) | `.../mx6ullevk/imximage.cfg` | path only, DDR settings unchanged |
| `include/configs/mx6ull_alientek_emmc.h` | `include/configs/mx6ullevk.h` | PHY: Micrel addr 2 -> SMSC addr 0; panel name; fixed `fdt_file` |
| `configs/mx6ull_alientek_emmc_defconfig` | `configs/mx6ull_14x14_evk_emmc_defconfig` | points at the new board |
| `arch/arm/cpu/armv7/mx6/Kconfig` | same file | registers the board |
| `drivers/net/phy/phy.c` | same file | one-time LAN8720 soft reset |

## Hardware differences

| | NXP EVK | Alientek eMMC board |
|---|---|---|
| Ethernet PHY | Micrel KSZ8081, address 2 | SMSC/Microchip LAN8720, address 0 |
| PHY reset | through a 74LV595 shift register | direct GPIO5_IO07 / GPIO5_IO08 |
| LCD | 4.3" 480x272 | ATK 4.3" 800x480 |
| Touch | SoC resistive controller (`&tsc`) | Goodix GT9147 capacitive, I2C2 @ 0x14 |
| GPIO1_IO09 | SD card power switch | touch interrupt |
| DDR init | EVK DDR3 settings | same |

## Comparing

Diff git's copies, which avoids macOS case-insensitivity noise (e.g.
`xt_DSCP.c` vs `xt_dscp.c`) and build leftovers:

```sh
# Whole kernel tree: what the port added or changed
git diff --stat \
  HEAD:nxp-evk-reference/linux-imx-rel_imx_4.1.15_2.1.0_ga_nxp_evk \
  HEAD:kernel-porting/ported/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek

# One file against the EVK file it was copied from
git diff \
  HEAD:nxp-evk-reference/linux-imx-rel_imx_4.1.15_2.1.0_ga_nxp_evk/arch/arm/boot/dts/imx6ull-14x14-evk.dts \
  HEAD:kernel-porting/ported/linux-imx-rel_imx_4.1.15_2.1.0_ga_alientek/arch/arm/boot/dts/imx6ull-alientek-emmc-luyaohan1001.dts
```

Use `HEAD:` for committed content, or `:` (the index) for staged content. A
plain `diff -ru` between the folders also works, but shows the macOS
case-collision files.
