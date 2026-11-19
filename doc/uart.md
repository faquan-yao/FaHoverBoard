# 串口收发

USART1 同时负责两件事：把日志和波形发给上位机，以及接收 VOFA+ 的调参命令。发送走软件环形缓冲加 `TXE` 中断；接收走 `RXNE` 中断加另一块环形缓冲，命令解析放在主循环里。

相关代码：

| 文件 | 作用 |
|------|------|
| `Core/Src/usart.c` | 初始化、发送环、接收环、中断里搬字节 |
| `Core/Src/stm32f1xx_it.c` | `USART1_IRQHandler()` 转到 `UartTx_IRQHandler()` |
| `Application/log.c` | 文本日志最终调用 `printf` |
| `Application/Vofa.c` | JustFloat 发送，以及把收到的字节拼成调参命令 |

## 硬件

| 项目 | 值 |
|------|-----|
| 外设 | USART1 |
| TX | PA9，复用推挽 |
| RX | PA10，浮空输入 |
| 波特率 | 115200 |
| 格式 | 8 数据位，1 停止位，无校验 |
| 中断 | `USART1_IRQn`，优先级 5 |

`HAL_UART_MspInit()` 打开 NVIC。接收中断要等 `Vofa_PidTune_Init()` → `UartRx_Start()` 才使能 `RXNE`。发送中断 `TXE` 平时是关的，缓冲里有新数据时才打开。

## 中断入口

USART1 中断只有一个入口：

```c
void USART1_IRQHandler(void)
{
    UartTx_IRQHandler();
}
```

这个函数同时处理发送和接收。主循环不调用它。

## 发送

文本日志的路径是：

`Log()` → `printf()` → `_write()` / `__io_putchar()` → `UartTx_Put()` → 发送环 → `TXE` 中断写 `DR`。

发送环长 4096 字节，下标 `uart_tx_w` 写入、`uart_tx_r` 读出。故意空出一个位置，所以 `w == r` 表示空。`UartTx_Put()` 在环满时会原地等待，直到中断腾出空间，因此文本字节不会被丢掉，但发得太快时会卡住调用者。

有数据写入后 `UartTx_Kick()` 打开 `TXE`。中断里若环已空就关掉 `TXE`，否则从环里取一个字节写入 `USART1->DR`。硬件把 `DR` 发完后再次置位 `TXE`，直到环空。

`LOG_VOFA` 等级下的波形不走这条环。`Log(LOG_VOFA, ...)` 取出格式串里的浮点参数，调用 `Vofa_JustFloat_Send()`，用 `HAL_UART_Transmit()` 直接阻塞发送。一帧是若干小端 `float`，帧尾固定 `00 00 80 7F`。这条路径和上面的发送环共用同一个 `DR`，两边同时发会互相抢。

## 接收

上位机发出的每个字节把 `RXNE` 置位，进入 `UartTx_IRQHandler()`。中断读一次 `DR`（同时清掉 `RXNE`），调用 `UartRx_Push()` 放进接收环。接收环长 128 字节，同样空一个位置表示空。环满时新字节直接丢弃。

若此时没有 `RXNE` 但溢出标志 `ORE` 置位，中断读 `SR` 再读 `DR` 把它清掉，避免接收卡死。清溢出时数据寄存器里的那个字节不会进入接收环。

主循环每圈开头调用 `Vofa_PidTune_Poll()`，用 `UartRx_Read()` 取出字节：

- `!`、`\r`、`\n` 是一行的结束符。命令缓冲里已有内容时，补上 `'\0'`，交给 `Vofa_ApplyCmd()`。
- 其它字符追加到 `vofa_cmd`，最长 39 字节。超出则丢弃这一行，从头再收。
- 没有结束符时，字节只停在 `vofa_cmd` 里，不会解析，也不会打 `bluetooth receive` 日志。

`Vofa_ApplyCmd()` 去掉行首空格后识别命令。键名不区分大小写，后面必须是 `=` 和数值。`BILIM`、`BMAX`、`SILIM`、`SMAX` 先于两个字母的键匹配，避免 `BI` 吃掉 `BILIM`。

| 命令 | 写入 |
|------|------|
| `BK=` | `BalancePID.Kp` |
| `BI=` | `BalancePID.Ki`，写成 0 时同时清掉积分 |
| `BD=` | `BalancePID.Kd` |
| `BMAX=` | 平衡环输出限幅，取绝对值后正负对称 |
| `BILIM=` | 平衡环积分限幅 |
| `SK=` `SI=` `SD=` | 速度环 Kp / Ki / Kd |
| `SMAX=` `SILIM=` | 速度环输出限幅、积分限幅 |
| `ST=` | `speed_target_rpm` |
| `?` 或 `GET` | 按文本回读当前参数 |

例如上位机发送 `BK=4.700!`：中断依次放入 `B` `K` `=` `4` `.` `7` `0` `0`，遇到 `!` 后主循环调用 `Vofa_ApplyCmd("BK=4.700")`，把平衡环 Kp 写成 4.7。下一拍控制循环读到的就是新值。

## 注意

- 结束符只有 `!`、`\r`、`\n`。只发 `BK=4.7` 而没有这三个字符时，命令不会生效。
- 接收环只有 128 字节，主循环长时间不调用 `Vofa_PidTune_Poll()` 时，多出来的字节会被丢掉。
- 文本发送在环满时会等待；JustFloat 用阻塞发送。两条发送路径不要同时使用。
