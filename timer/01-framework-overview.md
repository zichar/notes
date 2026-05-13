# Linux Clock Timer 各框架总览

> 依据 `clock_timer.c` 整理，并已对照 `~/qemu/linux`（重点是 `kernel/time/*`、`kernel/softirq.c`、`init/main.c`）校验。

## 1. 总体分层

Linux 计时与定时相关子系统可以抽象为 7 层：

1. **clocksource（时钟源）**  
   提供“当前时间读数”的底层硬件计数器抽象（如 arch timer、jiffies）。
2. **timekeeping（时间维护）**  
   将 clocksource 的 cycle 转换成 ns，并维护 `CLOCK_MONOTONIC` / `CLOCK_REALTIME` 等内核时间。
3. **clockevents（时钟事件）**  
   提供“下一次中断什么时候到来”的硬件事件编程能力。
4. **tick（周期/oneshot 管理）**  
   管理系统节拍模式，驱动周期 tick 或高精度 oneshot。
5. **hrtimer（高精度定时器）**  
   高精度（ns 级）定时器框架，基于红黑树定时队列。
6. **timer wheel（普通内核定时器）**  
   基于 jiffies 的低开销定时器（`timer_list`）。
7. **softirq（执行上下文）**  
   `TIMER_SOFTIRQ` 与 `HRTIMER_SOFTIRQ` 负责到期定时器回调执行。

---

## 2. 关键子系统关系

### 2.1 clocksource -> timekeeping

- `clocksource_register_hz()` 注册 clocksource。
- `clocksource_select()` 选出最优时钟源。
- `timekeeping_notify(best)` 切换 timekeeper 使用的 clocksource。
- 在 `change_clocksource()` 中通过 `timekeeping_update_from_shadow(..., TK_UPDATE_ALL)` 刷新快路径时间数据。

核心含义：  
**clocksource 负责“读时间”，timekeeping 负责“算时间并发布时间”。**

### 2.2 clockevents -> tick

- `clockevents_register_device()` 注册时钟事件设备。
- `tick_check_new_device()` 选择每 CPU 当前 event device。
- 根据模式走：
  - `tick_setup_periodic()`：周期模式；
  - `tick_setup_oneshot()`：单次触发模式（高精度/NOHZ 依赖）。

核心含义：  
**clockevents 负责“触发中断”，tick 负责“怎么用这个中断模型驱动系统前进”。**

### 2.3 tick -> timekeeping / scheduler / timer

无论周期还是高精度模式，本质都做 3 件事：

1. 更新时间（`jiffies` + `update_wall_time()`）
2. 更新进程时间片与调度（`update_process_times()` -> `scheduler_tick()`）
3. 编程下一次事件（`clockevents_program_event()` 或 hrtimer forward）

### 2.4 timer wheel 与 hrtimer 并行存在

- **timer wheel**（`add_timer/mod_timer`）：
  - 精度依赖 jiffies；
  - 到期后在 `run_timer_softirq()` 中执行。
- **hrtimer**：
  - 精度更高；
  - 到期硬中断路径先处理 hard hrtimer，soft hrtimer 由 `HRTIMER_SOFTIRQ` 继续处理。

---

## 3. 典型关键函数地图

### 3.1 初始化阶段（当前内核）

- `timers_init()`
- `hrtimers_init()`
- `timekeeping_init()`
- `time_init()`
- `sched_clock_init()`

### 3.2 运行阶段（周期模式）

- `tick_handle_periodic()`
- `tick_periodic()`
- `do_timer()`
- `update_wall_time()`
- `update_process_times()`

### 3.3 运行阶段（高精度模式）

- `hrtimer_run_queues()`（触发切换条件）
- `hrtimer_switch_to_hres()`
- `tick_switch_to_oneshot(hrtimer_interrupt)`
- `tick_setup_sched_timer(true)` -> `tick_nohz_handler()`

### 3.4 软中断执行

- `__do_softirq()`
- `run_timer_softirq()` -> `__run_timers()`
- `hrtimer_run_softirq()` -> `__hrtimer_run_queues()`

---

## 4. 一句话理解

- **clocksource**：我现在几点了  
- **clockevents**：我下次什么时候中断你  
- **timekeeping**：把“硬件计数”变成“内核时间”  
- **tick**：按策略推进系统节拍  
- **hrtimer/timer**：按到期时间执行回调  
- **softirq**：在合适上下文批量跑回调

## 5. 代码校验索引（`~/qemu/linux`）

- `init/main.c`：`start_kernel()` 内时间/定时子系统初始化顺序
- `kernel/time/timer.c`：`timers_init()`、`run_timer_softirq()`、`__run_timers()`
- `kernel/time/hrtimer.c`：`hrtimers_init()`、`hrtimer_run_queues()`、`hrtimer_interrupt()`
- `kernel/time/timekeeping.c`：`timekeeping_advance()`、`timekeeping_notify()`、`timekeeping_update_from_shadow()`
- `kernel/time/tick-common.c`：`tick_handle_periodic()`
- `kernel/time/tick-oneshot.c`：`tick_switch_to_oneshot()`、`tick_init_highres()`
- `kernel/time/tick-sched.c`：`tick_setup_sched_timer()`
- `kernel/softirq.c`：`do_softirq()`、`__do_softirq()`
