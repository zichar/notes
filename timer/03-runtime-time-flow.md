# Linux Clock Timer 运行时流程

> 依据 `clock_timer.c`，分别描述 periodic 模式和 high-resolution 模式的运行链路。

## 1. periodic tick 路径

入口：`tick_handle_periodic(struct clock_event_device *dev)`

```text
tick_handle_periodic()
  -> tick_periodic(cpu)
     -> do_timer(1)
        -> jiffies_64 += ticks
        -> calc_global_load()
     -> update_wall_time()
        -> timekeeping_advance(TK_ADV_TICK)
           -> timekeeping_update_from_shadow(...)
     -> update_process_times(user_mode(get_irq_regs()))
        -> account_process_tick()
        -> run_local_timers()
           -> hrtimer_run_queues()
           -> raise_timer_softirq(TIMER_SOFTIRQ) [if need]
        -> rcu_sched_clock_irq()
        -> irq_work_tick()
        -> scheduler_tick()
        -> run_posix_cpu_timers()
```

结束前如果是 oneshot 设备，需手动编程下一事件：

- `clockevents_program_event(dev, next, false)`

---

## 2. timekeeping 关键内部逻辑

### 2.1 `timekeeping_advance()`

`clock_timer.c` 里可归纳为 3 步：

1. 检查当前 offset（`mono - mono_last`）是否达到/超过周期阈值 `cycle_interval`
2. 累加并做 NTP 误差修正
3. 若发生 set/同步事件，调用 `timekeeping_update_from_shadow()`

### 2.2 `timekeeping_update_from_shadow()`

主要动作：

- `TK_CLEAR_NTP` 时清空 NTP 误差并 `ntp_clear()`
- 更新 leap state 与 ktime 数据
- 更新 vsyscall / pvclock / fast_timekeeper
- 更新 `base_real`
- 若 `TK_CLOCK_WAS_SET`，递增 `clock_was_set_seq`
- 通过 `memcpy()` 把 `shadow_timekeeper` 提交到 `timekeeper`

---

## 3. 从 periodic 向 hres 切换

触发点：`hrtimer_run_queues()`

```text
hrtimer_run_queues()
  -> if (hrtimer_hres_active()) return
  -> if (tick_check_oneshot_change(...))
       -> hrtimer_switch_to_hres()
          -> tick_init_highres()
             -> tick_switch_to_oneshot(hrtimer_interrupt)
                -> td->mode = TICKDEV_MODE_ONESHOT
                -> td->evtdev->event_handler = hrtimer_interrupt
                -> clockevents_switch_state(ONESHOT)
          -> base->hres_active = 1
          -> hrtimer_resolution = HIGH_RES_NSEC
          -> tick_setup_sched_timer(true)
             -> hrtimer_setup(..., tick_nohz_handler, ..., HRTIMER_MODE_ABS_HARD)
             -> hrtimer_start_expires(...)
          -> tick_nohz_activate(ts)
          -> retrigger_next_event(NULL)
```

---

## 4. hres 模式运行路径

### 4.1 时钟事件中断入口

`hrtimer_interrupt(struct clock_event_device *dev)`：

1. 更新 `now`
2. 如 soft hrtimer 到期，`raise_timer_softirq(HRTIMER_SOFTIRQ)`
3. 执行 `__hrtimer_run_queues(..., HRTIMER_ACTIVE_HARD)`（hard hrtimer）
4. 计算 `expires_next = hrtimer_update_next_event()`
5. `tick_program_event(expires_next, 0)` 编程下一中断

### 4.2 调度节拍 hrtimer

`tick_nohz_handler(struct hrtimer *timer)` 仍然做三件事：

1. `tick_sched_do_timer()`：推进 jiffies 与 wall time
2. `tick_sched_handle()`：`update_process_times()` + `profile_tick()`
3. `hrtimer_forward_now(timer, TICK_NSEC)`：推进下次触发点

结论：  
hres 模式不是取消 tick 工作，而是把“tick 触发机制”从固定周期中断变成了高精度可编程 oneshot。
