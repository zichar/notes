# 基于 `~/qemu/linux` 的代码验证与补缺记录

> 目的：把 `clock_timer.c` 笔记与实际内核代码逐项对齐，记录差异和修正结论。

## 1. 验证范围

- `init/main.c`
- `kernel/time/timer.c`
- `kernel/time/hrtimer.c`
- `kernel/time/timekeeping.c`
- `kernel/time/clocksource.c`
- `kernel/time/clockevents.c`
- `kernel/time/tick-common.c`
- `kernel/time/tick-oneshot.c`
- `kernel/time/tick-sched.c`
- `kernel/softirq.c`

## 2. 关键差异与修正

### 2.1 初始化函数名

- 笔记使用：`init_timers()`
- 实际代码：`timers_init()`（`init/main.c` 调用，定义在 `kernel/time/timer.c`）
- 处理：文档统一改为 `timers_init()`。

### 2.2 clocksource 切换后的 timekeeping 更新函数

- 笔记使用：`timekeeping_update(...)`
- 实际代码：`change_clocksource()` 中调用 `timekeeping_update_from_shadow(&tk_core, TK_UPDATE_ALL)`
- 处理：文档统一改为 `timekeeping_update_from_shadow(...)`。

### 2.3 hres tick 回调名

- 笔记使用：`tick_sched_timer`
- 实际代码：`tick_setup_sched_timer(true)` 使用 `tick_nohz_handler`
- 处理：文档统一改为 `tick_nohz_handler()`。

### 2.4 sched_clock 定时器初始化形式

- 笔记使用：`hrtimer_init() + function 赋值`
- 实际代码：`hrtimer_setup(&sched_clock_timer, sched_clock_poll, ...)`
- 处理：文档改为 `hrtimer_setup()` 形式。

### 2.5 softirq action 调用接口

- 笔记使用：`h->action(h)`
- 实际代码：`h->action()`
- 处理：文档改为 `action()` 无参数调用。

### 2.6 `run_timer_softirq()` 路径细节

- 笔记简化为单个 base。
- 实际代码会跑 `BASE_LOCAL`，并在 `CONFIG_NO_HZ_COMMON` 下继续跑 `BASE_GLOBAL/BASE_DEF`。
- 处理：文档补齐多 base 执行链路。

### 2.7 定时器删除 API 表述

- 笔记用 `del_timer` 语义表达。
- 实际代码主接口是 `timer_delete()` 族，并有 `timer_delete_sync()/timer_shutdown_sync()`。
- 处理：文档改为 `timer_delete` 族，并补充同步/防重臂接口。

## 3. 仍建议保留的“概念简化”

以下表述虽然不等同逐行代码，但作为学习图谱保留是合理的：

- periodic 与 hres 都执行“推进时间、更新调度、编程下一事件”三步
- timer wheel 与 hrtimer 并行存在，分别通过 `TIMER_SOFTIRQ` 与 `HRTIMER_SOFTIRQ`
- `clocksource` 负责读时间，`clockevents` 负责触发下一中断，`timekeeping` 负责时间换算与发布

## 4. 验证结论

`clock_timer.c` 的主干理解正确；已对函数名、关键调用点和执行细节做版本对齐，当前文档可直接对应 `~/qemu/linux` 代码阅读。
