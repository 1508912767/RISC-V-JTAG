# JTAG
RISC-V CPU是通过JTAG进行调试的, 芯来科技提供了JTAG的System-Verilog和C语言的接口, 搭配蜂鸟调试器试用。然而在实际量产过程中, 不可能让客户一直使用调试器, 效率太低

因此在MCU中集成了JTAG的功能, 通过MCU的GPIO进行反转，模拟JTAG时序。

1. JTAG-SV: 
   - System-Verilog实现的JTAG接口，适用于FPGA或ASIC设计。
   - 提供了完整的JTAG协议支持，包括指令寄存器、数据寄存器等。

2. JTAG-C:
   - C语言实现的JTAG接口，适用于嵌入式系统开发。模拟上述System-Verilog接口的功能。
   - 提供了简单易用的API，方便开发者进行JTAG调试和测试。
   - 实现了四线jtag和两线cJTAG

   该文件 jtag_function.h 及其对应的实现文件 jtag_function.c，主要实现了基于 JTAG（Joint Test Action Group）协议的调试、烧录、寄存器/内存访问等功能，适用于嵌入式芯片的调试和测试。以下是详细分析报告：

---

## 1. 主要功能概述

### 1.1 JTAG 状态机与信号控制
- 实现了 JTAG TAP（Test Access Port）状态机的各个状态跳转函数，如 `jtag_idle_to_shift_ir`、`jtag_sdr_to_e1d` 等。
- 提供了对 JTAG 相关引脚（TCK、TMS、TDI、TDO、RST）的 GPIO 控制宏和初始化。
- 支持标准 JTAG 四线模式和 cJTAG 两线模式的切换（`switch_jtag2cjtag`）。

### 1.2 JTAG 数据传输
- 实现了 IR（指令寄存器）和 DR（数据寄存器）的发送与接收（`send_ir`、`send_dr`、`jtag_send_ir`、`jtag_send_dr`）。
- 支持自定义长度的数据移位，适配不同芯片的数据结构。

### 1.3 Debug Module（DM）寄存器访问
- 提供了对 RISC-V Debug Module 寄存器的读写接口（`write_dm_register`、`read_dm_register`）。
- 支持通过 JTAG 访问 DTM（Debug Transport Module）寄存器。

### 1.4 CPU 控制
- 实现了 CPU 的 halt、resume、enable dmi 等操作（`halt_cpu`、`resume_cpu`、`enable_dmi`）。

### 1.5 CSR/内存访问
- 支持通过多种方式（Abstract Command、Program Buffer、System Bus）访问 CPU 的 CSR 寄存器和内存（`read_csr`、`write_csr`、`read_memory`、`write_memory` 等）。

### 1.6 Flash 操作
- 实现了 Flash 的初始化、擦除、写入、读取等功能（`flash_init`、`flash_erase`、`flash_write`、`flash_read`）。
- 适配特定芯片（如 3775/3780）的 Flash 控制寄存器和命令。

### 1.7 调试与测试
- 提供了调试测试代码（`dbg_test_code`、`dbg_test_init`），可用于验证 JTAG 通信和芯片功能。

---

## 2. 结构与接口设计

- **数据结构**：定义了 JTAG 数据结构 `JtagTestData`，用于存储移位数据。
- **接口抽象**：通过函数指针和结构体（如 `jtag_i`、`mcu_jtag_t`）实现了 JTAG 及 GPIO 的接口抽象，便于移植和扩展。
- **状态枚举**：定义了多种枚举类型，描述内存/寄存器访问大小、CPU 状态、命令错误类型、JTAG 状态等。

---

## 3. 适用场景

- 适用于基于 RISC-V 架构的芯片调试、烧录、寄存器/内存访问、Flash 操作等场景。
- 可作为 MCU 或 FPGA 上 JTAG 调试器固件的核心模块。
- 支持多种访问模式，适配不同的调试需求和硬件平台。

---

## 4. 特色与亮点

- **支持多种 JTAG 模式**：兼容标准 JTAG 和 cJTAG，适应不同硬件需求。
- **完整的状态机实现**：细致实现了 TAP 状态机的每一步，便于调试和扩展。
- **丰富的寄存器/内存/Flash 操作接口**：可直接用于芯片烧录、调试和功能验证。
- **接口抽象良好**：便于后续移植到不同的硬件平台或操作系统。

---

## 5. 总结

该文件为嵌入式系统提供了完整的 JTAG 调试、烧录、寄存器/内存/Flash 操作的底层驱动和接口，适合用于芯片开发、测试、量产等多种场景。代码结构清晰，接口丰富，便于集成和扩展，是一套功能完善的 JTAG 控制实现。