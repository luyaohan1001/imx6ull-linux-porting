# 正点原子 i.MX6ULL：uuu 烧写 + 串口交互指南（macOS）

## 1. 安装 uuu

下载地址：<https://github.com/nxp-imx/mfgtools/releases>

Apple 芯片的 Mac 下载 release 附件里带 `mac_arm` 的文件；Intel Mac 如果没有预编译版本，需要按仓库 README 自行编译。

```bash
cd ~/Downloads
chmod +x uuu_mac_arm
xattr -d com.apple.quarantine uuu_mac_arm   # 解除 macOS 的下载拦截
sudo mv uuu_mac_arm /usr/local/bin/uuu
uuu -h                                     # 能打印帮助即安装成功
```

## 2. 硬件连接

板子上要接两根 USB 线，用途不同：

| 接口 | 作用 | 电脑上的表现 |
|---|---|---|
| **USB OTG** | uuu 下载、烧写 | `uuu -lsusb` 能看到 |
| **USB_TTL**（CH340 USB 转串口） | 串口控制台 | `/dev/cu.usbserial-XXXX` 或 `/dev/cu.wchusbserial-XXXX` |

- 确认连接 UART1 和 USB 转串口的跳线帽已插好（以板子丝印和手册为准）。
- USB 下载模式的拨码开关：`01000000`（以手册为准）。

## 3. 打开串口控制台

**先开串口，再给板子上电或运行 uuu**，不然会错过启动打印和倒计时。

```bash
ls /dev/cu.*                              # 插拔 USB_TTL 线前后各看一次，新出现的就是板子
screen /dev/cu.usbserial-XXXX 115200      # 115200 8N1
```

- 用 `cu.*` 而不是 `tty.*`，后者可能会卡住。
- 退出 screen：`Ctrl-A`，然后 `K`，再按 `Y`。
- 提示 "resource busy" 时，说明有旧会话还占着串口：
  ```bash
  screen -ls
  screen -X -S <id> quit
  ```

## 4. 用 uuu 加载 U-Boot

### 4.1 只加载到内存运行（安全，不写入任何存储）

```bash
sudo uuu -lsusb          # 确认识别到板子
sudo uuu u-boot.imx      # 下载到内存并运行
```

适合测试刚修改的 U-Boot。断电后就回到原状。

### 4.2 写入 eMMC

```bash
sudo uuu -b emmc u-boot.imx
```

注意：这一步依赖 U-Boot 里的 fastboot 功能。正点原子自带的 2016.03 版 U-Boot 可能没开启，会卡在第二阶段。可选的解决办法：

- 在 U-Boot 配置里开启 fastboot 后重新编译；或者
- 按 4.1 把 U-Boot 跑起来，再在串口里执行 `ums 0 mmc 1`，把 eMMC 当 U 盘挂到电脑上写入。

## 5. 进入 U-Boot 命令行

U-Boot 启动后会打印 `Hit any key to stop autoboot: 3` 这样的倒计时，**在倒计时结束前按任意键**，就会停在命令提示符（`=>` 或板子自定义的提示符）。

如果串口停在 fastboot 等待状态，按 `Ctrl-C` 退出。

### 常用命令

| 命令 | 作用 |
|---|---|
| `help` / `? <命令>` | 列出所有命令 / 查看某个命令的用法 |
| `version` | U-Boot 版本和编译时间 |
| `bdinfo` | 板级信息（内存地址、波特率等） |
| `printenv` | 打印所有环境变量 |
| `setenv 名字 值` | 设置环境变量（仅在内存里生效） |
| `saveenv` | 把环境变量保存到存储 |
| `mmc list` / `mmc dev 1` / `mmc info` | 查看、切换 SD/eMMC 设备 |
| `fatls mmc 1:1` | 列出 eMMC 第 1 个分区（FAT）里的文件 |
| `md 0x80000000 20` | 查看内存内容 |
| `ums 0 mmc 1` | 把 eMMC 模拟成 U 盘（需开启该功能） |
| `boot` / `reset` | 按 bootcmd 启动 / 复位 |

### 值得先看的环境变量

- `bootcmd`：自动启动时执行的命令
- `bootargs`：传给 Linux 内核的参数，比如 `console=ttymxc0,115200 root=...`
- `bootdelay`：倒计时秒数

## 6. 常见问题

| 现象 | 排查 |
|---|---|
| `uuu -lsusb` 看不到设备 | 检查拨码开关是否是 USB 模式、线是否插在 OTG 口、是否用了 `sudo` |
| 串口没有任何输出 | 检查波特率 115200、跳线帽、是否选错了 `/dev/cu.*` |
| 串口出现乱码 | 波特率不对 |
| 错过倒计时 | 先开 screen 再上电；或 `setenv bootdelay 5` 后 `saveenv` |
