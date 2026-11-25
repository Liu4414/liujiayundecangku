# smartcar — 六轮底盘 + 五舵机机械臂

STM32F103C8T6 固件：DS4 手柄 → ESP32（Bluetooth Classic）→ UART 12 字节帧 → STM32 → 6 路 TT 电机 + PCA9685 五路舵机。

## 硬件

| 部件 | 型号 / 数量 |
|---|---|
| 主控 | STM32F103C8T6（蓝板，72 MHz） |
| 电机驱动 | TB6612FNG × 3（每片 A/B 两通道，共 6 路） |
| 舵机驱动 | PCA9685（I2C，16 路） |
| 电池 | 2S 7.4 V 5000 mAh，带保护板 |
| 遥控 | PS4 DualShock 4 + ESP32-WROOM-32（Bluepad32） |
| 底盘 | 4 个 45° 麦轮 + 2 个中间全向轮，6 个 TT 减速电机 |
| 舵机 | MG996R ×2、MG995 ×2、MG90S ×1 |

## 引脚分配

| 功能 | 引脚 |
|---|---|
| 电机 PWM | M1 PA6 / M2 PA7 / M3 PB0 / M4 PB1 / M5 PB6 / M6 PB7（均 TIM3_CH1~4 与 TIM4_CH1~2，20 kHz） |
| 电机方向 | M1 PA4/PA5、M2 PA0/PA1、M3 PA12/PA11、M4 PB13/PB12、M5 PB5/PB4、M6 PB9/PB8 |
| TB6612 STBY | PB14（三片并联） |
| PCA9685 | PB10 = SCL、PB11 = SDA（I2C2） |
| 调试串口 | USART1 PA9/PA10 |
| 遥控串口 | USART2 PA2/PA3（ESP32 GPIO17 → PA3） |
| LED | PC13 |

## 串口协议（12 字节，CRC8 poly 0x07）

```
[0] AA  [1] 55  [2] seq
[3] lx  [4] ly  [5] rx  [6] ry      (-100 ~ +100)
[7] btnL [8] btnH                   (16 位按键位图)
[9] l2  [10] r2                     (0 ~ 255)
[11] crc8 = CRC([2]..[10])
```

## 手柄功能

| 按键 | 功能 |
|---|---|
| 左摇杆 ↑↓ / ←→ | 前进后退 / 左右平移 |
| 右摇杆 ←→ | 左转右转 |
| 右摇杆 ↑↓ | 机械臂升降（CH13） |
| L1 / L2 | 夹爪张开 / 闭合（CH12） |
| R2 / R1 | 机械臂前伸 / 后缩（CH14） |
| 十字 ←→ | 云台左转右转（CH15，360° 连续舵机） |
| 十字 ↑↓ | 车斗仰起 / 倾倒（CH11） |
| × | 急停 |
| □ | 刹车 |
| △ | 半速（按住） |
| ○ | 全部舵机回中 |

## 编译

```bash
# 需要 STM32CubeCLT（含 arm-none-eabi-gcc、CMake、Ninja）
cmake --preset Debug
cmake --build build/Debug
```

Windows 下若 PATH 里没有 Ninja：

```powershell
$env:PATH = "D:\tools\ST\STM32CubeCLT_1.22.0\Ninja\bin;$env:PATH"
& "D:\tools\ST\STM32CubeCLT_1.22.0\CMake\bin\cmake.exe" --build build\Debug
```

## 烧录

```powershell
& "D:\tools\ST\STM32CubeCLT_1.22.0\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe" `
  -c port=SWD mode=UR -w build\Debug\smartcar.elf -v -rst
```

## 串口命令台（USART1，115200 8N1）

| 命令 | 作用 |
|---|---|
| `?` | 帮助 |
| `m <1-6> <v>` | 单电机，v = -1000 ~ 1000 |
| `a <v>` | 六电机同速 |
| `s` / `b` | 滑行停止 / 短路制动 |
| `e` / `d` | 使能 / 切断 STBY |
| `sv <1-5> <p>` | 单舵机，p = -100 ~ 100 |
| `st` | 打印状态 |
| `x` | 退出手动，回到遥控 |

## 注意

- **PA11/PA12 被用作 M3 的方向脚**，即 USB D−/D+，因此板载 USB 口不能插（SWD 烧录不受影响）。
- 上电时所有舵机会同时冲向各自中位，装配前先手动把机械结构摆到中位。
- 云台是 360° 连续舵机：1500 µs = 停，>1500 正转，<1500 反转。
- 舵机必须独立 6 V 供电，不能与 ESP32 共用 5 V 降压，否则车一动就重启。
