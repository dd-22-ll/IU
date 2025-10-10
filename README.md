# I2C 任务整合说明

本说明汇总了项目中 I2C 相关的关键流程，便于快速参考如何初始化控制器、进入从模式以及在应用任务中完成接入。

## 初始化与资源管理
- `I2C_Init` 负责在首次调用时创建 I2C 控制器的互斥锁，确保多线程环境下的并发安全。
- `pI2C_MasterInit` / `pI2C_SlaveInit` 会在内部调用 `I2C_Init`，根据 `I2C_TYP` 选择目标控制器并执行软复位、配置 SCL 参数或从机地址，最后开启外设。

## 主机模式使用
- 轮询模式可通过 `bI2C_MasterProcess` 一站式完成：在互斥锁保护下写入起始地址，循环调用 `bI2C_MasterWrite` / `bI2C_MasterRead` 完成数据交互，并在必要时发送 STOP 条件。
- 轮询过程依赖的细分函数——如 `bI2C_MasterReady`、`bI2C_MasterWriteAddr`、`bI2C_MasterWrite`、`bI2C_MasterRead`——都包含超时保护与错误处理，防止总线异常挂起。
- 若使用中断模式，则可通过 `bI2C_MasterInt` 配置收发缓冲区，并调用 `bI2C_MasterIntRw` 进入中断驱动流程；`bI2C_MasterIntRwInternal` 会启用 START/数据/STOP/错误等中断源，ISR 中根据硬件状态推进读写流程并最终释放信号量。

## 从机模式使用
- 初始化从机时直接调用 `pI2C_SlaveInit(controller, role, addrMode, slaveAddr)`；该函数会设置 `I2C_EN10`、`I2C_SAR` 与 `I2C_GC_EN`，无需手动操作寄存器。
- 轮询收发可通过 `bI2C_SlaveWrite`（主机读从机）与 `bI2C_SlaveRead`（主机写从机）；若需获知主机实际写入的字节数，可改用 `I2C_SlaveReceive`，该接口会在互斥锁保护下调用底层轮询逻辑并返回主机写入的总字节数。
- 如需中断收发，可分别使用 `bI2C_SlaveIntWrite` / `bI2C_SlaveIntRead`。函数会注册对应的 ISR（`I2C_Slave1Isr` / `I2C_Slave2Isr`），在中断中完成缓冲区搬运并在传输结束后关闭中断。

## 应用任务中的集成
- 在系统启动阶段调用 `APP_I2CSlaveInit`，该函数位于 `APP_HS.c`，内部通过 `pI2C_SlaveInit(I2C_1, I2C_SLAVE, I2C_SADDR8, 0x51)` 进入从模式并输出标准模式（100KHz）地址信息。
- 初始化成功后，会把返回的 `I2C1_Type*` 句柄保存到静态变量，并通过 `APP_GetI2CSlaveHandle()` 对外提供，方便其它模块重用；同时创建 `APP_I2CSlaveThread` 线程，在线程中调用 `I2C_SlaveReceive` 阻塞等待主机写入并打印接收到的字节内容（ASCII 与十六进制两种形式）。
- 如果初始化失败或线程创建失败，函数会打印错误日志，提醒检查硬件连线或地址配置。

## 使用提示
- 进入任意从机收发函数前，应确保初始化返回的句柄依旧有效，并保证发送/接收缓冲区在整个传输期间有效（例如使用静态或全局数组）。
- 使用 10 位地址时，记得把 `addrMode` 设为 `I2C_SADDR10` 并传入完整地址值，库内部会自动处理高位匹配。
- 函数返回 `false` 时代表检测到超时、NACK 或 STOP，需要上层根据实际情况进行重试或重新初始化。

以上内容覆盖了 I2C 相关任务的主要流程，可作为项目配置与调试的快速参考。

## 按键处理方案
- `KEY_Init` 会分别初始化电源键 (`PKEY_Init`)、模拟键盘 (`AKEY_Init`) 与 GPIO 键 (`GKEY_Init`)，然后创建优先级为 `osPriorityAboveNormal`、周期为 10ms 的 `KEY_Thread`。线程内顺序调用三个子扫描函数并在末尾 `osDelay(KEY_THREAD_PERIOD)`，形成统一的去抖与长按检测框架。 【F:KEY.c†L24-L55】【F:APP_CFG.h†L27-L43】【F:KEY.h†L48-L66】
- 每类按键的子扫描函数都在任务上下文内执行状态机：例如电源键通过 `ubRTC_GetKey` 读取硬件脚位，根据不同状态在按下、长按计数、松开时向 UI 事件队列投递 `KEY_QueueSend` 消息；模拟键、GPIO 键也采用类似的状态机，在任务节拍内完成去抖和长按次数统计。 【F:PKEY.c†L28-L89】【F:AKEY.c†L52-L118】【F:GKEY.c†L40-L110】
- 采用任务而非中断的好处：
  - **统一处理不同硬件来源**：模拟键需要先通过 ADC 转换得到电压档位，GPIO 键与 RTC 电源键则直接读寄存器；统一在任务中调度可避免在 ISR 中执行耗时的 ADC、打印或队列操作。 【F:AKEY.c†L95-L116】【F:PKEY.c†L45-L86】
  - **便于实现去抖与长按逻辑**：状态机依赖毫秒级的节拍与计数，任务内循环天然适合基于时间片的去抖判断，避免使用多组软定时器或在中断内维护复杂逻辑。 【F:AKEY.c†L64-L112】【F:PKEY.c†L45-L85】
  - **降低系统中断负担**：只需在 10ms 的任务节拍中轮询即可满足人机交互需求，同时避免因键盘抖动频繁触发中断影响音视频传输等高优先级业务。 【F:KEY.c†L46-L55】【F:APP_CFG.h†L27-L43】

通过上述设计，项目中的各类按键都能在统一的任务框架下完成检测、去抖和事件上报，并保持与 UI 线程的松耦合。
