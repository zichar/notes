# Linux Timer 执行流程（timer wheel + hrtimer + softirq）

> 依据 `clock_timer.c` 的普通定时器/高精度定时器路径整理。

## 1. 普通定时器（`timer_list`）

## 1.1 添加与修改

```text
add_timer(&timer)
  -> __mod_timer(timer, timer->expires, MOD_TIMER_NOTPENDING)

mod_timer(&timer, expires)
  -> __mod_timer(timer, expires, 0)
```

### 1.2 删除（当前实现）

```text
timer_delete(&timer)
  -> __timer_delete(timer, false)
     -> lock_timer_base()
     -> detach_if_pending(timer, base, true)
```

补充：当前内核还提供 `timer_delete_sync()` / `timer_shutdown()` / `timer_shutdown_sync()` 作为更安全的同步/防重臂删除接口。

---

## 2. softirq 调度执行骨架

`do_softirq()` -> `__do_softirq()`（内部 `handle_softirqs()`）：

1. 读取 `pending` 位图
2. 清 pending，开中断
3. 遍历 `softirq_vec`，调用各自 `action()`（当前接口无参数）
4. 处理过程中若又产生 pending，根据预算条件 restart
5. 超预算则唤醒 `ksoftirqd`

这条骨架同时服务 `TIMER_SOFTIRQ` 和 `HRTIMER_SOFTIRQ`。

---

## 3. TIMER_SOFTIRQ（timer wheel 到期执行）

入口：`run_timer_softirq()`

```text
run_timer_softirq()
  -> run_timer_base(BASE_LOCAL)
  -> run_timer_base(BASE_GLOBAL) [NO_HZ_COMMON]
  -> run_timer_base(BASE_DEF)    [NO_HZ_COMMON]
     -> __run_timers(base)
        -> while (jiffies >= base->clk && jiffies >= base->next_expiry)
             -> collect_expired_timers()
             -> base->clk++
             -> timer_recalc_next_expiry(base)
             -> expire_timers()
                -> detach_timer(timer, true)
                -> fn = timer->function
                -> call_timer_fn(timer, fn, baseclk)
```

要点：

- 以 jiffies 驱动；
- 按层级桶批量收集到期定时器；
- 回调在 softirq 上下文执行。

---

## 4. HRTIMER_SOFTIRQ（soft hrtimer 到期执行）

入口：`hrtimer_run_softirq(void)`

```text
hrtimer_run_softirq()
  -> now = hrtimer_update_base(cpu_base)
  -> __hrtimer_run_queues(cpu_base, now, flags, HRTIMER_ACTIVE_SOFT)
     -> for_each_active_base(...)
        -> while ((node = timerqueue_getnext(&base->active)))
           -> timer = container_of(node, struct hrtimer, node)
           -> __run_hrtimer(...)
              -> __remove_hrtimer(...)
              -> restart = timer->function(timer)
  -> hrtimer_update_softirq_timer(cpu_base, true)
```

要点：

- hrtimer 队列基于时间排序结构（timerqueue/rbtree）；
- hard hrtimer 在中断路径执行，soft hrtimer 在此处执行；
- 回调返回值决定是否重启定时器。

---

## 5. 两类定时器对比

| 维度 | timer wheel (`timer_list`) | hrtimer |
| --- | --- | --- |
| 精度 | jiffies 级 | ns 级 |
| 结构 | 分层桶/链表 | timerqueue + rbtree |
| 触发基础 | 周期 tick 或 jiffies 推进 | clockevents oneshot 精确重编程 |
| softirq | `TIMER_SOFTIRQ` | `HRTIMER_SOFTIRQ` |
| 典型用途 | 超时检测、低开销延迟 | 高精度周期/超时控制 |

---

## 6. 实践提醒

- 回调处于中断/softirq 语境，不能睡眠。
- 重活应转移到 workqueue 或线程上下文。
- 删除定时器需注意并发与生命周期，避免回调访问已释放对象。
