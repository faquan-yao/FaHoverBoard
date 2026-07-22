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

在 `Core/Src/main.c` 中配置（**同一时间只开一种自检/校准**）：

```c
#define RUN_ENCODER_TEST  0   /* 1 = 编码器自检（不进入平衡环） */
#define RUN_MPU_TEST      0   /* 1 = MPU6050 姿态自检 */
#define RUN_ACCEL_CAL     0   /* 1 = 加速度计六面校准并写入 Flash */
```

| 模式 | 宏设置 | 行为 |
|------|--------|------|
| 正常平衡 | 三者均为 `0` | 初始化后进入平衡控制；若 Flash 有有效校准则自动加载 |
| 编码器自检 | `RUN_ENCODER_TEST = 1` | `Enc_SelfTest()`，不返回 |
| MPU 自检 | `RUN_MPU_TEST = 1` | `MPU_SelfTest()`，不返回 |
| 六面校准 | `RUN_ACCEL_CAL = 1` | `AccelCal_SelfTest()`，校准后写入 Flash，不返回 |

测完/校完后务必将对应宏改回 `0`，再编译下载。

初始化顺序（与 `main.c` 一致）：

1. 电机 / 编码器初始化 →（可选）编码器自检  
2. MPU6050 I2C 探测 + DMP 初始化（自动尝试加载 Flash 加速度校准）→（可选）MPU 自检 / 六面校准  
3. PID 初始化 → 平衡主循环  

---

## 编码器自检

用于验证左右轮编码器接线、计数方向与测速。测试由电机开环驱动，**不进入平衡环**。

- 实现：`Core/Src/encoder_test.c`、`Core/Inc/encoder_test.h`
- 参数：`Core/Src/hardware_cfg.h`
- 默认占空比：**25%**（PWM 满量程 0～1000 → 比较值 250）
- 约定：MotorA = 左轮，MotorB = 右轮；前进 = `MOTOR_CCW`（与 `MotorControl(speed>0)` 一致）

### 硬件参数

| 参数 | 值 | 说明 |
|------|-----|------|
| 编码器线数 | 500 PPR | 正交四倍频 → 电机轴每转 2000 脉冲 |
| 减速比 | 1:28 | 输出轴每转 = \(2000 \times 28 = 56000\) 脉冲 |
| 采样周期 | 20 ms | TIM6 |
| 左轮编码器 | TIM3 | |
| 右轮编码器 | TIM5 | |

### 开启步骤

1. `RUN_ENCODER_TEST = 1`，`RUN_MPU_TEST = 0`
2. **架空车轮**（勿着地乱跑），编译下载
3. 串口 115200，看到 `==== Encoder self-test (motor-driven) ====`
4. 自动执行三阶段（见下）；阶段 3 会一直转并打印，复位可停
5. 测完将 `RUN_ENCODER_TEST` 改回 **0**

### 测试阶段

| 阶段 | 动作 | 结束条件 |
|------|------|----------|
| Phase1 `P1-BOTH-FWD` | 左右轮同时向前转 | 各累计约 1 圈（56000 脉冲）后停车 |
| Phase2 `P2-L-FWD-R-REV` | 左轮向前、右轮向后 | 各累计约 1 圈后停车 |
| Phase3 `P3-CONST-FWD` | 两轮以 25% 占空比匀速向前 | 不结束；每 100 ms 打印编码器 |

单侧到位会先停该侧电机；总超时约 30 s 则报 `TIMEOUT`。

### 串口输出格式

```text
[P1-BOTH-FWD] L: cnt=... total=... d=... rev=+0.512 FWD rpm=+40.0 | R: ...
```

| 字段 | 含义 |
|------|------|
| 行首标签 | 当前阶段名 |
| `cnt` | TIM 当前原始计数 |
| `total` | 本阶段起算后的累计脉冲（带符号） |
| `d` | 最近 20 ms 脉冲增量 |
| `rev` | 输出轴累计圈数 = `total / 56000` |
| `FWD` / `REV` / `STOP` | 由 RPM 符号判断 |
| `rpm` | 输出轴转速（RPM） |

### 通过标准

| 检查项 | 预期 |
|--------|------|
| Phase1 | 两轮同向转；结束时两侧 \|`rev`\| ≈ **1.0**，打印 `P1-BOTH-FWD OK` |
| Phase2 | 左 `FWD`、右 `REV`（`total`/`rpm` 符号相反）；两侧 \|`rev`\| ≈ **1.0** |
| Phase3 | `rpm` 大致稳定；`total`/`rev` 持续同向增加 |
| 整圈误差 | 一般应在数个百分点以内（约 56000±几％） |

### 常见问题

- **TIMEOUT / 轮不转**：查电机供电、STBY、TB6612 接线；可先跑 `MotorHwTest()`
- **轮转但 `total` 不涨**：编码器 A/B 或 TIM 引脚问题
- **Phase1 圈数差很多**：某一侧编码器异常
- **Phase2 左右同号**：方向定义或 A/B 对调；可试 `MOTOR_B_INVERT`
- **Phase3 转速不稳**：开环正常，多与供电/摩擦有关

### 相关 API

- `ENC_GetRawCnt` / `ENC_GetTotals` / `ENC_ResetTotals`
- `ENC_GetPulseDiffs` / `ENC_GetSpeeds`（输出轴 RPM）

---

## 加速度计六面校准

对 MPU6050 加速度计做六面（±X/±Y/±Z）静态校准，结果写入片内 Flash **掉电保存**，之后每次上电 DMP 初始化时自动加载并应用到硬件/DMP 偏置。

- 实现：`Core/Src/accel_cal.c`、`Core/Inc/accel_cal.h`
- Flash：`Core/Src/flash_storage.c`（末页 `0x0807F800`，2KB）
- 默认：**不启用**（`RUN_ACCEL_CAL = 0`）

### 开启步骤

1. `RUN_ENCODER_TEST = 0`，`RUN_MPU_TEST = 0`，`RUN_ACCEL_CAL = 1`
2. 编译下载，串口 115200
3. 按提示依次将模块六个面朝上（或按提示方向）平放静止；每面倒计时 5 s 后采样约 2 s
4. 成功后打印 `Saved to flash OK` 与 bias/scale，并尝试写入 MPU
5. 将 `RUN_ACCEL_CAL` 改回 **0**，重新编译下载进入正常模式（会自动 `AccelCal_ApplyToMpu()`）

### 六面顺序

| 顺序 | 提示 | 期望 |
|------|------|------|
| 1 | Z-up | `az ≈ +1g` |
| 2 | Z-down | `az ≈ -1g` |
| 3 | Y-up | `ay ≈ +1g` |
| 4 | Y-down | `ay ≈ -1g` |
| 5 | X-up | `ax ≈ +1g` |
| 6 | X-down | `ax ≈ -1g` |

计算：`bias = (pos+neg)/2`，`scale = 2/(pos-neg)`（按轴）。带 magic + CRC32 校验。

### 注意

- 校准过程需**平稳、对准**，否则 `face span too small` 失败
- 正常运行时若 Flash 无有效记录，跳过加载（不影响启动）
- 缩放主要用于软校正 API `AccelCal_Correct()`；写入 MPU 的主要是 **bias**

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
