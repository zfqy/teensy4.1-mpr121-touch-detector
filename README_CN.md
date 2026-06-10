# Teensy 4.1 + MPR121 触摸检测项目

这个项目用于让 Teensy 4.1 读取 MPR121 电容触摸传感器的触摸状态，并把 MPR121 的 0、1 通道结果输出到 Teensy 的数字引脚。当前示例默认只启用 2 个通道，但代码结构可以扩展支持 MPR121 的全部 12 个触摸通道。

当前代码文件：

```text
mpr121_teensy_touch/mpr121_teensy_touch.ino
```

## 功能

- 通过 I2C 读取 MPR121。
- 检测 MPR121 的 E0、E1 两个触摸通道。
- 当前默认启用 2 个通道，但可通过修改配置和输出映射扩展到 12 个通道。
- 串口输出触摸和释放事件。
- Teensy 数字 PIN1 输出 E0 的检测结果。
- Teensy 数字 PIN2 输出 E1 的检测结果。
- 触摸时输出高电平，未触摸时输出低电平。
- 可通过开关打开 Raw 调试信息。
- 可通过开关打开 touched mask 汇总信息。
- 使用软件基线检测，便于观察和调试不同电极环境下的变化。

## 硬件连接

MPR121 与 Teensy 4.1 的连接如下：

| MPR121 | Teensy 4.1 |
| --- | --- |
| SDA | PIN 18 / SDA0 |
| SCL | PIN 19 / SCL0 |
| IRQ | 数字 PIN 0 |
| ADDR | GND |
| GND | GND |
| VIN / VCC | 3.3V 或模块支持的供电电压 |

ADDR 接地时，MPR121 的 I2C 地址为：

```cpp
0x5A
```

输出引脚：

| MPR121 通道 | Teensy 输出引脚 | 未触摸 | 触摸 |
| --- | --- | --- | --- |
| E0 | PIN 1 | LOW / 0V | HIGH / 3.3V |
| E1 | PIN 2 | LOW / 0V | HIGH / 3.3V |

注意：Teensy 4.1 是 3.3V 逻辑电平。不要把 PIN1/PIN2 直接接到会向 Teensy 反灌 5V 的设备上。

## 软件准备

需要安装：

- Arduino IDE 2.x
- Teensy by PJRC 板卡支持包
- Adafruit MPR121 库
- Adafruit BusIO 库

在 Arduino IDE 中安装 Teensy 支持：

1. 打开 `File > Preferences`
2. 在 `Additional boards manager URLs` 中加入：

```text
https://www.pjrc.com/teensy/package_teensy_index.json
```

3. 打开 `Tools > Board > Boards Manager`
4. 搜索并安装 `Teensy by PJRC`

安装库：

1. 打开 `Tools > Manage Libraries`
2. 搜索并安装 `Adafruit MPR121`
3. 如果提示依赖，安装 `Adafruit BusIO`

## 使用方法

1. 用 Arduino IDE 打开：

```text
mpr121_teensy_touch/mpr121_teensy_touch.ino
```

2. 选择板子：

```text
Tools > Board > Teensy > Teensy 4.1
Tools > USB Type > Serial
```

3. 编译并上传。

第一次上传 Teensy 时，如果 Arduino IDE 找不到板子，可以在上传阶段按一下 Teensy 4.1 板子上的白色按钮。

4. 打开串口监视器：

```text
Tools > Serial Monitor
```

波特率选择：

```text
115200
```

正常启动时会看到类似：

```text
MPR121 multi-channel touch test starting...
Calibrating software baseline. Do not touch electrodes...
Software baseline: 0=... 1=...
Ready. Touch one or more electrodes.
```

启动时不要触摸电极，因为程序会先校准软件基线。

## 串口输出

正常触摸 E0 时：

```text
Electrode 0 touched
Electrode 0 released
```

正常触摸 E1 时：

```text
Electrode 1 touched
Electrode 1 released
```

如果开启 touched mask 输出，还会看到类似：

```text
Touched mask: 0b01 | channels: 0
```

当前默认关闭 touched mask 输出。

## 重要配置

代码顶部有几个常用配置：

```cpp
constexpr uint8_t CHANNEL_0_OUTPUT_PIN = 1;
constexpr uint8_t CHANNEL_1_OUTPUT_PIN = 2;
constexpr uint8_t ELECTRODE_COUNT = 2;

constexpr uint8_t TOUCH_THRESHOLD = 5;
constexpr uint8_t RELEASE_THRESHOLD = 3;

constexpr bool ENABLE_DEBUG_OUTPUT = false;
constexpr bool ENABLE_TOUCH_MASK_OUTPUT = false;
```

含义：

- `CHANNEL_0_OUTPUT_PIN`：E0 触摸结果输出到 Teensy PIN1。
- `CHANNEL_1_OUTPUT_PIN`：E1 触摸结果输出到 Teensy PIN2。
- `ELECTRODE_COUNT`：当前只检测 E0、E1，因此设为 2。
- `TOUCH_THRESHOLD`：触摸判定阈值。
- `RELEASE_THRESHOLD`：释放判定阈值。
- `ENABLE_DEBUG_OUTPUT`：是否显示 Raw 调试数据。
- `ENABLE_TOUCH_MASK_OUTPUT`：是否显示 touched mask 汇总状态。

## 扩展到 12 通道

MPR121 最多支持 12 个触摸通道，也就是 E0 到 E11。当前代码为了配合本项目的实际需求，只启用了 E0、E1：

```cpp
constexpr uint8_t ELECTRODE_COUNT = 2;
```

如果需要检测全部 12 个通道，可以把它改成：

```cpp
constexpr uint8_t ELECTRODE_COUNT = 12;
```

这样 `calibrateSoftwareBaseline()`、`readSoftwareTouched()`、Raw 调试输出、touched mask 输出等逻辑都会按 12 个通道循环处理。

需要注意的是，当前只有 E0 和 E1 被映射到了 Teensy 数字输出引脚：

```cpp
constexpr uint8_t CHANNEL_0_OUTPUT_PIN = 1;
constexpr uint8_t CHANNEL_1_OUTPUT_PIN = 2;
```

如果希望 E2 到 E11 也输出到 Teensy 的数字引脚，需要继续增加输出引脚配置，并扩展 `updateTouchOutputPins()` 函数。

另外，不同通道的电极面积、线长和安装位置可能不同，扩展到 12 通道后更建议使用每通道独立阈值，而不是所有通道共用同一个 `TOUCH_THRESHOLD`。

## 调试方法

如果触摸没有反应，先打开 Raw 调试输出：

```cpp
constexpr bool ENABLE_DEBUG_OUTPUT = true;
```

重新烧录后，串口会周期性输出：

```text
Raw: 0:filtered/baseline/delta  1:filtered/baseline/delta
```

例如：

```text
Raw: 0:1/11/10  1:10/11/1
```

三个数字分别是：

- `filtered`：当前滤波后的实时读数。
- `baseline`：启动时校准出来的空闲基线。
- `delta`：`baseline - filtered`，也就是当前变化量。

触摸时，通常 `filtered` 会下降，`delta` 会变大。

当前判断逻辑是：

```cpp
delta >= TOUCH_THRESHOLD
```

如果触摸时 delta 明显变大但没有触发，可以降低 `TOUCH_THRESHOLD`。

如果没有触摸也误触发，可以提高 `TOUCH_THRESHOLD`，或改善电极和线缆环境。

## 阈值问题

不同场景下需要不同阈值，这是电容触摸传感器的常见问题。影响因素包括：

- 电极面积。
- 导线长度。
- 导线是否靠近金属、USB 线、电源线。
- 手和电路地之间的耦合情况。
- 桌面材料。
- 环境湿度。
- 是否有未使用通道悬空。

当前项目使用固定阈值：

```cpp
TOUCH_THRESHOLD = 5
RELEASE_THRESHOLD = 3
```

这个值适合当前测试环境，但换电极、换安装位置、换线长后可能需要重新调整。

如果后续要做成更稳定的产品，可以考虑进一步改成：

- 每个通道单独阈值。
- 开机后自动采集环境噪声并估算阈值。
- 长时间未触摸时慢速更新 baseline。
- 增加连续多次检测确认，避免瞬间噪声误触发。
- 给未使用通道关闭检测，或不要让未使用电极悬空。

## 常见问题

### 烧录后 Teensy 板载灯不闪了

这是正常现象。Teensy 出厂默认程序通常会让板载 LED 闪烁。烧录本项目后，默认 blink 程序被替换。

当前代码中板载 LED 会作为心跳灯，每 500ms 翻转一次，用来确认程序仍在运行。

### Arduino IDE 找不到 Teensy Loader

如果编译或导出 hex 时看到类似错误：

```text
Opening Teensy Loader...
Unable find Teensy Loader.  (p)  Is the Teensy Loader application running?
Is a firewall (eg, ZoneAlarm) blocking localhost communication?
quitexit status 1
```

可以先检查：

- Teensy Loader 是否已经打开。
- Teensy Loader 中 `Operation > Auto` 是否勾选。
- Arduino IDE 是否已经安装 `Teensy by PJRC`。
- 防火墙是否允许 `teensy.exe`、`teensy_post_compile.exe` 和 `arduino-ide.exe`。

本项目实际调试中遇到过：Teensy Loader 已经打开，但 Arduino IDE 仍然反复提示找不到 Teensy Loader。最后通过重启电脑解决。

因此，如果确认安装和设置都没问题，但仍然反复报这个错误，可以直接重启电脑再试。

### 串口看到 MPR121 not found at 0x5A

说明程序运行了，但没有通过 I2C 找到 MPR121。检查：

- SDA 是否接 PIN18。
- SCL 是否接 PIN19。
- ADDR 是否接 GND。
- Teensy 和 MPR121 是否共地。
- MPR121 供电是否正确。
- SDA/SCL 是否接反。

### 串口监视器连接不上或没有输出

如果程序已经烧录成功，但串口监视器连接不上，或者没有看到启动信息，先检查 Arduino IDE 的端口选择：

```text
Tools > Port
```

需要选中 Teensy 对应的端口。Windows 上有时显示为：

```text
COMx (Teensy 4.1)
```

也可能显示为类似：

```text
usb:0/140000/0/A/1
```

如果选错了 COM 口，串口监视器就不会连接到 Teensy，也看不到程序输出。

串口监视器波特率建议选择：

```text
115200
```

### 触摸没有反应

先打开：

```cpp
ENABLE_DEBUG_OUTPUT = true
```

观察触摸时对应通道的 `delta` 是否变大。

如果 delta 完全不变，通常是电极没有接到对应通道，或触摸面积太小。

如果 delta 变大但没有触发，调整阈值。

### 没触摸也误触发

可能原因：

- 阈值太低。
- 电极线太长。
- 电极靠近金属或电源线。
- 开机校准时手靠得太近。
- 未使用通道悬空。

建议先重新上电，并在校准期间不要触碰电极。

## 当前输出逻辑

代码中的核心输出函数：

```cpp
void updateTouchOutputPins(uint16_t touched) {
  digitalWrite(CHANNEL_0_OUTPUT_PIN, (touched & (1 << 0)) ? HIGH : LOW);
  digitalWrite(CHANNEL_1_OUTPUT_PIN, (touched & (1 << 1)) ? HIGH : LOW);
}
```

因此：

- E0 触摸时，PIN1 为 HIGH。
- E0 松手后，PIN1 为 LOW。
- E1 触摸时，PIN2 为 HIGH。
- E1 松手后，PIN2 为 LOW。

## 程序结构与 Arduino C++ 特点

本项目的控制程序是 Arduino 风格的 C++。`.ino` 文件看起来很像 C 语言，但实际会由 Arduino/Teensy 工具链用 C++ 编译器编译。

Arduino C++ 的一个重要特点是：通常不需要自己写 `main()` 函数，只需要实现：

```cpp
void setup() {
  // 开机或重启后执行一次
}

void loop() {
  // setup() 之后反复执行
}
```

Arduino/Teensy 框架在背后大致会做类似这样的事情：

```cpp
int main() {
  init();
  setup();

  while (true) {
    loop();
  }
}
```

因此，`loop()` 不是自己永远不退出，而是外层框架不断重复调用它。一次 `loop()` 执行结束后，会返回给 Arduino 框架，然后框架马上再次调用 `loop()`。

本项目程序大致由以下几部分组成：

### 1. 库的导入

```cpp
#include <Wire.h>
#include <Adafruit_MPR121.h>
```

其中：

- `Wire.h` 用于 I2C 通讯。
- `Adafruit_MPR121.h` 用于控制 MPR121 触摸传感器。

### 2. 全局常量、变量和对象实例化

例如：

```cpp
constexpr uint8_t MPR121_ADDR = 0x5A;
constexpr uint8_t CHANNEL_0_OUTPUT_PIN = 1;
constexpr uint8_t TOUCH_THRESHOLD = 5;

Adafruit_MPR121 cap = Adafruit_MPR121();
```

这些内容用于定义：

- MPR121 的 I2C 地址。
- Teensy 使用的输入/输出引脚。
- 触摸和释放阈值。
- 调试开关。
- MPR121 控制对象 `cap`。

### 3. 功能函数的实现

程序中把一些具体任务拆成了独立函数，例如：

```cpp
calibrateSoftwareBaseline();
readSoftwareTouched();
updateTouchOutputPins();
printTouchedChannels();
```

这样可以让 `setup()` 和 `loop()` 更容易阅读。

各函数的主要作用：

- `calibrateSoftwareBaseline()`：启动时采集无触摸状态，建立软件基线。
- `readSoftwareTouched()`：读取当前传感器值，并判断哪些通道被触摸。
- `updateTouchOutputPins()`：根据 E0/E1 的触摸状态更新 Teensy PIN1/PIN2。
- `printTouchedChannels()`：可选地打印 touched mask 汇总信息。

### 4. `setup()` 的实现

`setup()` 只在上电、重启或烧录后执行一次。

本项目中，`setup()` 主要做：

- 启动 USB 串口。
- 设置 IRQ、输出引脚和板载 LED 的模式。
- 启动 I2C。
- 初始化 MPR121。
- 设置触摸阈值。
- 校准软件基线。
- 初始化输出引脚状态。
- 打印启动信息。

### 5. `loop()` 的实现

`loop()` 会在 `setup()` 完成后被反复调用。

本项目中，`loop()` 主要做：

- 翻转板载 LED，作为程序运行心跳。
- 根据调试开关，选择是否打印 Raw 数据。
- 读取 E0/E1 的当前触摸状态。
- 更新 Teensy PIN1/PIN2 的高低电平。
- 当触摸状态变化时，打印 touched/released 事件。

可以把整体执行流程理解为：

```text
开机
  -> Arduino/Teensy 框架初始化
  -> 执行 setup() 一次
  -> 反复执行 loop()
       -> 读取传感器
       -> 判断触摸状态
       -> 更新输出引脚
       -> 打印事件或调试信息
```
