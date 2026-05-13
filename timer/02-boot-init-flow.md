# Linux Clock Timer 启动初始化流程

> 依据 `clock_timer.c` 给出的函数路径，并结合 `~/qemu/linux` 实际代码校验。

## 1. 主流程骨架

```text
start_kernel()
  -> tick_init()
  -> rcu_init_nohz()
  -> timers_init()
  -> hrtimers_init()
  -> softirq_init()
  -> timekeeping_init()
  -> time_init()
  -> ...(后续初始化若干步骤)...
  -> sched_clock_init()
```

---

## 2. 定时器与软中断初始化

### 2.1 `timers_init()`

- `init_timer_cpus()`：初始化每 CPU timer 基础结构。
- `posix_cputimers_init_work()`：POSIX CPU timer 相关工作初始化。
- `open_softirq(TIMER_SOFTIRQ, run_timer_softirq)`：注册普通 timer 的 softirq 入口。

### 2.2 `hrtimers_init()`

- `hrtimers_prepare_cpu(smp_processor_id())`：初始化当前 CPU 的 hrtimer base。
- `open_softirq(HRTIMER_SOFTIRQ, hrtimer_run_softirq)`：注册 hrtimer softirq 入口。

---

## 3. sched_clock 初始化路径

`sched_clock_init()` 关键节点：

1. `static_branch_inc(&sched_clock_running)`
2. `generic_sched_clock_init()`
3. 若读函数仍是 `jiffy_sched_clock_read`，则：
   - `sched_clock_register(jiffy_sched_clock_read, BITS_PER_LONG, HZ)`
4. `update_sched_clock()`
5. 启动一个用于轮询 wrap 的 hrtimer：
   - `hrtimer_setup(&sched_clock_timer, sched_clock_poll, CLOCK_MONOTONIC, HRTIMER_MODE_REL_HARD)`
   - `hrtimer_start(&sched_clock_timer, cd.wrap_kt, HRTIMER_MODE_REL_HARD)`

含义：`sched_clock` 不是独立时间体系，它服务于调度时延统计等快速时间戳需求。

---

## 4. clocksource 注册与选择

### 4.1 默认 clocksource

- `core_initcall(init_jiffies_clocksource)`
- `__clocksource_register(&clocksource_jiffies)`

这是兜底时钟源，保证系统早期可用。

### 4.2 新 clocksource 注册后

```text
clocksource_register_hz(&clocksource_gpt, c)
  -> __clocksource_update_freq_scale()
  -> clocksource_enqueue()
  -> clocksource_enqueue_watchdog()
  -> clocksource_select()
       -> __clocksource_select(false)
          -> timekeeping_notify(best)
             -> stop_machine(change_clocksource, ...)
                -> timekeeping_forward_now()
                -> tk_setup_internals()
                -> timekeeping_update_from_shadow(&tk_core, TK_UPDATE_ALL)
             -> tick_clock_notify()
```

关键点：  
clocksource 切换通过 `stop_machine()` 保障一致性，切换后统一调用 `timekeeping_update_from_shadow()` 同步快慢路径时间数据。

---

## 5. clockevent 设备注册路径

```text
clockevents_register_device(dev)
  -> list_add(&dev->list, &clockevent_devices)
  -> tick_check_new_device(dev)
     -> clockevents_exchange_device(curdev, newdev)
     -> tick_setup_device(...)
        -> tick_setup_periodic() or tick_setup_oneshot()
```

### 5.1 periodic

- `tick_set_periodic_handler()`
- 绑定 `dev->event_handler = tick_handle_periodic`（或 broadcast 版本）

### 5.2 oneshot

- `clockevents_switch_state(newdev, CLOCK_EVT_STATE_ONESHOT)`
- `clockevents_program_event(newdev, next_event, true)`

当设备支持 oneshot 时，还会触发：

- `tick_oneshot_notify()`
- `clockevents_notify_released()` 里再次检查候选设备

---

## 6. 初始化阶段结果

启动完成后，系统具备：

1. 可读时间（clocksource + timekeeping）
2. 可编程下一次中断（clockevents）
3. 可执行定时器回调（timer/hrtimer + softirq）
4. 可从 periodic 向 oneshot/hres 迁移的能力（tick/hrtimer）
