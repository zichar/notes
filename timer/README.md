# Linux Clock Timer 框架梳理

本文档基于 `clock_timer.c` 中整理的调用路径，按“框架层次 + 代码执行流程”拆分成多个文档，便于快速定位。

## 文档索引

- `01-framework-overview.md`  
  clocksource / clockevents / timekeeping / tick / hrtimer / timer wheel / softirq 的角色与关系。
- `02-boot-init-flow.md`  
  从 `start_kernel()` 开始的初始化路径。
- `03-runtime-time-flow.md`  
  周期 tick 与高精度模式下的时间推进和调度相关流程。
- `04-timer-exec-flow.md`  
  普通 timer 与 hrtimer 的添加、触发、软中断执行链路。
- `05-code-verify-qemu-linux.md`  
  基于 `~/qemu/linux` 的逐项代码验证与补缺记录。
- `clock_timer_flow.md`  
  一页完整版调用链（Mermaid）。
- `clock_timer_flow_simple.md`  
  评审汇报用简化版调用链（Mermaid）。

## 推荐阅读顺序

1. `01-framework-overview.md`（建立整体心智模型）
2. `02-boot-init-flow.md`（理解系统如何进入可计时状态）
3. `03-runtime-time-flow.md`（理解时间如何推进）
4. `04-timer-exec-flow.md`（理解具体定时器何时被执行）
