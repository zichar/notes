# Linux Clock Timer 一页调用链总图

> 基于 `clock_timer.c` 整理，并按 `~/qemu/linux` 实际代码校验：覆盖初始化、periodic/hres 运行、timer/hrtimer softirq 执行路径。

## 1) 总体调用链（Mermaid）

```mermaid
flowchart TD
  A[start_kernel] --> B[timers_init]
  A --> C[hrtimers_init]
  A --> D[softirq_init]
  A --> E[timekeeping_init]
  A --> F[time_init]
  A --> G[sched_clock_init]

  B --> B1[open_softirq TIMER_SOFTIRQ run_timer_softirq]
  C --> C1[open_softirq HRTIMER_SOFTIRQ hrtimer_run_softirq]

  A --> H[clocksource_register_hz]
  H --> H1[clocksource_select]
  H1 --> H2[timekeeping_notify best]
  H2 --> H3[timekeeping_update_from_shadow]

  A --> I[clockevents_register_device]
  I --> I1[tick_check_new_device]
  I1 --> I2{mode}
  I2 -->|periodic| I3[tick_setup_periodic]
  I2 -->|oneshot| I4[tick_setup_oneshot]

  %% periodic runtime
  I3 --> J[tick_handle_periodic]
  J --> J1[do_timer]
  J1 --> J2[jiffies_64 plusplus]
  J --> J3[update_wall_time]
  J3 --> J4[timekeeping_advance]
  J --> J5[update_process_times]
  J5 --> J6[run_local_timers]
  J6 --> J7[raise_softirq TIMER_SOFTIRQ]
  J5 --> J8[scheduler_tick]

  %% hres switch and runtime
  J6 --> K[hrtimer_run_queues]
  K --> K1{need switch to hres}
  K1 -->|yes| K2[hrtimer_switch_to_hres]
  K2 --> K3[tick_switch_to_oneshot hrtimer_interrupt]
  K2 --> K4[tick_setup_sched_timer true]
  K4 --> K5[tick_nohz_handler callback]
  K5 --> K6[tick_sched_do_timer]
  K5 --> K7[update_process_times]
  K5 --> K8[hrtimer_forward next tick]

  K3 --> L[hrtimer_interrupt]
  L --> L1[run hard hrtimer queue]
  L --> L2[raise_softirq HRTIMER_SOFTIRQ]
  L --> L3[tick_program_event next]

  %% softirq execution
  M[do_softirq] --> M1[__do_softirq]
  M1 --> N[TIMER_SOFTIRQ]
  M1 --> O[HRTIMER_SOFTIRQ]

  N --> N1[run_timer_softirq]
  N1 --> N2[__run_timers]
  N2 --> N3[collect_expired_timers]
  N2 --> N4[expire_timers call_timer_fn]

  O --> O1[hrtimer_run_softirq]
  O1 --> O2[__hrtimer_run_queues soft]
  O2 --> O3[__run_hrtimer timer function]
```

## 2) 阅读要点

- **clocksource -> timekeeping**：解决“现在几点”。
- **clockevents -> tick**：解决“下次何时触发中断”。
- **periodic 与 hres**：做的是同一组业务动作（推进时间、调度记账、安排下一次触发），仅触发机制不同。
- **timer 与 hrtimer 并行**：分别挂在 `TIMER_SOFTIRQ` 与 `HRTIMER_SOFTIRQ`，精度和数据结构不同。

## 3) 三个高频问题速记

1. **谁推进 `jiffies`？**  
   `do_timer()` / `tick_sched_do_timer()`。
2. **谁更新 wall time？**  
   `update_wall_time()` -> `timekeeping_advance()` -> `timekeeping_update_from_shadow()`。
3. **定时器回调在哪里跑？**  
   普通 timer 在 `run_timer_softirq()`，soft hrtimer 在 `hrtimer_run_softirq()`，hard hrtimer 在 `hrtimer_interrupt()` 中断路径。
