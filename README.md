# FaHoverBoard

STM32F103 平衡车固件（双环：速度环 + 角度平衡环）。

## 串口

| 项目 | 值 |
|------|-----|
| 外设 | USART1 |
| 波特率 | **115200** |
| 换行 | CRLF（`\r\n`） |

调试打印通过 `printf` → `__io_putchar` 输出。

## 运行模式切换

在 `Core/Src/main.c` 中配置（**同一时间只开一种自检**）：

```c
#define RUN_ENCODER_TEST  0   /* 1 = 编码器自检（不进入平衡环） */
#define RUN_MPU_TEST      0   /* 1 = MPU6050 自检（不进入平衡环） */
```

| 模式 | 宏设置 | 行为 |
|------|--------|------|
| 正常平衡 | 两者均为 `0` | 初始化后进入平衡控制循环 |
| 编码器自检 | `RUN_ENCODER_TEST = 1` | 调用 `Enc_SelfTest()`，不返回 |
| MPU 自检 | `RUN_MPU_TEST = 1` | DMP 就绪后调用 `MPU_SelfTest()`，不返回 |

测完后务必将对应宏改回 `0`，再编译下载。

初始化顺序（与 `main.c` 一致）：

1. 电机 / 编码器初始化 →（可选）编码器自检  
2. MPU6050 I2C 探测 + DMP 初始化 →（可选）MPU 自检  
3. PID 初始化 → 平衡主循环  

---

## 编码器自检

用于验证左右轮编码器接线、计数方向与测速。

- 实现：`Core/Src/encoder_test.c`、`Core/Inc/encoder_test.h`
- 参数：`Core/Src/hardware_cfg.h`

### 硬件参数

| 参数 | 值 | 说明 |
|------|-----|------|
| 编码器线数 | 500 PPR | 正交四倍频 → 电机轴每转 2000 脉冲 |
| 减速比 | 1:28 | 输出轴每转 = \(2000 \times 28 = 56000\) 脉冲 |
| 采样周期 | 20 ms | TIM6 |
| 左轮 | TIM3 | |
| 右轮 | TIM5 | |

### 开启步骤

1. `RUN_ENCODER_TEST = 1`，`RUN_MPU_TEST = 0`
2. 编译下载，打开串口 115200
3. 看到 `==== Encoder self-test ====` 后，约每 **100 ms** 一行
4. 用手拨左右轮（或开环驱动）观察数据
5. 测完改回 `RUN_ENCODER_TEST = 0`

### 串口输出格式

```text
L: cnt=32780 total=56000 d=1120 rev=+1.000 FWD rpm=+60.0 | R: cnt=32768 total=0 d=0 rev=+0.000 STOP rpm=+0.0
```

| 字段 | 含义 |
|------|------|
| `cnt` | TIM 当前原始计数（采样周期内会回到中点附近） |
| `total` | 累计脉冲（带符号） |
| `d` | 最近 20 ms 脉冲增量 |
| `rev` | 输出轴累计圈数 = `total / 56000` |
| `FWD` / `REV` / `STOP` | 方向（由 RPM 符号；\|rpm\|≤1 为 STOP） |
| `rpm` | 输出轴转速（RPM） |

`L` = 左轮，`R` = 右轮。

### 通过标准

| 检查项 | 预期 |
|--------|------|
| 手拨左轮 | 仅 `L` 的 `total` / `d` / `rev` / `rpm` 变化 |
| 手拨右轮 | 仅 `R` 变化 |
| 正反转 | `FWD` ↔ `REV`，`total` / `rev` 符号相反 |
| 输出轴整转 1 圈 | \|`total`\| ≈ **56000**，\|`rev`\| ≈ **1.000** |
| 静止 | `d≈0`，`rpm≈0`，`STOP` |

### 常见问题

- **无变化**：查编码器供电/地线，以及 TIM3/TIM5 GPIO
- **整圈约 28000 或 2000**：倍频或减速比理解不对
- **方向反了**：对调 A/B，或改 CubeMX 通道极性
- **`cnt` 跳但 `total` 不涨**：确认 TIM6 中断与 `Enc_Init()` 正常

### 相关 API

- `ENC_GetRawCnt` / `ENC_GetTotals` / `ENC_ResetTotals`
- `ENC_GetPulseDiffs` / `ENC_GetSpeeds`（输出轴 RPM）

---

## MPU6050 自检

用于验证 DMP 姿态与俯仰角速度。

- 实现：`Core/Src/mpu6050_test.c`、`Core/Inc/mpu6050_test.h`
- 接口：`MPU6050_DMP_Get_Date()`（`Core/Inc/mpu6050.h`）

### 开启步骤

1. `RUN_ENCODER_TEST = 0`，`RUN_MPU_TEST = 1`
2. 编译下载，串口 115200
3. 等待 `MPU6050 Ready!` 与 `==== MPU6050 self-test ====`
4. 约每 **100 ms** 打印一行姿态数据
5. 测完改回 `RUN_MPU_TEST = 0`

### 串口输出格式

```text
pitch,roll,yaw,gyro_dps
```

示例：

```text
1.23,-0.45,12.67,3.10
```

| 字段 | 单位 | 含义 |
|------|------|------|
| `pitch` | deg | 俯仰角 |
| `roll` | deg | 横滚角 |
| `yaw` | deg | 偏航角 |
| `gyro_dps` | deg/s | 俯仰轴角速度（平衡环 D 项） |

### 通过标准

| 检查项 | 预期 |
|--------|------|
| 静止 | 角度相对稳定，`gyro_dps` 接近 0 |
| 俯仰 / 横滚 / 偏航 | 对应 `pitch` / `roll` / `yaw` 跟随变化 |
| 快速俯仰 | \|`gyro_dps`\| 明显增大 |

### 常见问题

- **无 ACK / init failure**：查 I2C2（PB10/SCL、PB11/SDA）、供电、AD0（0x68 / 0x69）
- **角度乱跳**：平稳放置后上电，减小振动与电源噪声
- **直立 pitch 不是 0**：静止直立时读 `pitch`，写入 `ANGLE_MECH_ZERO`

---

## 正常平衡模式

`RUN_ENCODER_TEST = 0` 且 `RUN_MPU_TEST = 0` 时进入主循环：

- **速度环**（约 10 Hz）：反馈为左右轮平均输出轴 RPM，目标为全局变量 `speed_target_rpm`（默认 `0`）
- **角度环**：目标角 = `ANGLE_MECH_ZERO + SPEED_TRIM_SIGN * angle_trim`
- 串口周期打印：`p` / `g` / `tgt` / `bal` / `duty`

### 常用可调参数

| 参数 | 位置 | 说明 |
|------|------|------|
| `speed_target_rpm` | `pid_balance.c`（`extern` 于 `pid_balance.h`） | 速度环目标，输出轴 RPM，默认 0 |
| `ANGLE_MECH_ZERO` | `pid_balance.h` | 机械直立对应的 pitch（deg） |
| `GYRO_BIAS` | `pid_balance.h` | 陀螺仪偏置（deg/s） |
| `SPEED_TRIM_SIGN` | `pid_balance.h` | 速度→倾角微调符号，方向反了可改为 `+1.0f` |
| `GYRO_D_SIGN` | `pid_balance.h` | 陀螺 D 项符号 |
| `BalancePID` / `SpeedPID` | `pid_balance.c` | PID 增益与限幅 |

编码器测速公式（输出轴 RPM）：

\[
\mathrm{RPM} = \frac{\Delta\mathrm{pulse} \times 60000}{\mathrm{ENCODER\_RESOLUTION} \times \mathrm{GEAR\_RATIO} \times \mathrm{SAMPLE\_PERIOD\_MS}}
\]

---

## 电机硬件测试（可选）

接线排查时可在 `main.c` 中临时取消注释：

```c
MotorHwTest();
```

会强制驱动相关 GPIO/PWM 一段时间，见 `Core/Src/tb6612fng.c`。用完请重新注释。
