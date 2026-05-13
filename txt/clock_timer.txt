start_kernel()
	init_timers(void)
		init_timer_cpus();
		posix_cputimers_init_work();
		open_softirq(TIMER_SOFTIRQ, run_timer_softirq);
	hrtimers_init();
		hrtimers_prepare_cpu(smp_processor_id());
		open_softirq(HRTIMER_SOFTIRQ, hrtimer_run_softirq);
	softirq_init();
	timekeeping_init();
	time_init();
	sched_clock_init();
		static_branch_inc(&sched_clock_running);
		generic_sched_clock_init();
			if (cd.actual_read_sched_clock == jiffy_sched_clock_read)
				sched_clock_register(jiffy_sched_clock_read, BITS_PER_LONG, HZ);
			update_sched_clock();
			hrtimer_init(&sched_clock_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL_HARD);
			sched_clock_timer.function = sched_clock_poll;
			hrtimer_start(&sched_clock_timer, cd.wrap_kt, HRTIMER_MODE_REL_HARD);

/* default clocksource */
core_initcall(init_jiffies_clocksource); 
	__clocksource_register(&clocksource_jiffies);

 /* clocksource */
clocksource_register_hz(&clocksource_gpt, c);
	__clocksource_update_freq_scale(cs, scale, freq); 	
	clocksource_enqueue(cs);
	clocksource_enqueue_watchdog(cs);
	clocksource_select(); //multi called
		__clocksource_select(false); //multi called
			timekeeping_notify(best)
				stop_machine(change_clocksource, clock, NULL); /**/
					timekeeping_forward_now(tk);
					if (change) //change clock source
						tk_setup_internals(tk, new);
							tk->tkr_mono.clock = clock;
							tk->tkr_raw.clock = clock;
					timekeeping_update(tk, TK_CLEAR_NTP | TK_MIRROR | TK_CLOCK_WAS_SET);
				tick_clock_notify();
				return tk->tkr_mono.clock == clock ? 0 : -1; 
	clocksource_select_watchdog(false);
	__clocksource_suspend_select(cs); 

 
 /* clock event */
void clockevents_register_device(struct clock_event_device *dev)
	list_add(&dev->list, &clockevent_devices);
	tick_check_new_device(dev);
		td = &per_cpu(tick_cpu_device, cpu);
		clockevents_exchange_device(curdev, newdev);
		tick_setup_device(td, newdev, cpu, cpumask_of(cpu));
			if (td->mode == TICKDEV_MODE_PERIODIC)
				tick_setup_periodic(newdev, 0);
					tick_set_periodic_handler(dev, broadcast);
						if (!broadcast) dev->event_handler = tick_handle_periodic;
						else dev->event_handler = tick_handle_periodic_broadcast;
					// if oneshot change state and clockevents_program_event(dev, next, false) 
			else
				tick_setup_oneshot(newdev, handler, next_event);
					newdev->event_handler = handler;
					clockevents_switch_state(newdev, CLOCK_EVT_STATE_ONESHOT);
					clockevents_program_event(newdev, next_event, true);
					
		if (newdev->features & CLOCK_EVT_FEAT_ONESHOT)
			tick_oneshot_notify();
	clockevents_notify_released();
		while (!list_empty(&clockevents_released))
			list_move(&dev->list, &clockevent_devices);
			tick_check_new_device(dev);
	
	
 /* timekeeping */
void tick_handle_periodic(struct clock_event_device *dev)
	tick_periodic(cpu);
		//(1) do timer
		if (tick_do_timer_cpu == cpu)
			tick_next_period = ktime_add_ns(tick_next_period, TICK_NSEC);
			do_timer(1);
				jiffies_64 += ticks;
				calc_global_load();
			update_wall_time();
				if (timekeeping_advance(TK_ADV_TICK))     //timekeeping relatives
					clock_was_set_delayed();
		//(2) update_process_times
		update_process_times(user_mode(get_irq_regs()));  //schedule relatives, changed to hres timer
		profile_tick(CPU_PROFILING);
	//(3) next timer
	if (clockevent_state_one_shot(dev))
		next = ktime_add_ns(next, TICK_NSEC);
		clockevents_program_event(dev, next, false);
			dev->set_next_event((unsigned long) clc, dev);
		if (timekeeping_valid_for_hres())
			tick_periodic(cpu);


static bool timekeeping_advance(enum timekeeping_adv_mode mode)
	1. check if "offset"<-(mono - mono_last) bigger than tk->cycle_interval<-(NSEC_PER_SEC/HZ)
	2. do accumulate and adjust for time as well as ntp error
	3. if clock_set do timekeeping_update(tk, clock_set);
	
static void timekeeping_update(struct timekeeper *tk, unsigned int action)
	if (action & TK_CLEAR_NTP)
		tk->ntp_error = 0;
		ntp_clear();
	tk_update_leap_state(tk);
	tk_update_ktime_data(tk);
	update_vsyscall(tk);
	update_pvclock_gtod(tk, action & TK_CLOCK_WAS_SET); 
	tk->tkr_mono.base_real = tk->tkr_mono.base + tk->offs_real;
	update_fast_timekeeper(&tk->tkr_mono, &tk_fast_mono);
	update_fast_timekeeper(&tk->tkr_raw,  &tk_fast_raw);
	if (action & TK_CLOCK_WAS_SET) tk->clock_was_set_seq++;
	if (action & TK_MIRROR) memcpy(&shadow_timekeeper, &tk_core.timekeeper, sizeof(tk_core.timekeeper));


/* update_process_times */
void update_process_times(int user_tick)
	account_process_tick(p, user_tick);
	run_local_timers();
		hrtimer_run_queues();
			if (__hrtimer_hres_active(cpu_base))
				return;
			... //switch to hres timer
		raise_softirq(TIMER_SOFTIRQ); //if need
	rcu_sched_clock_irq(user_tick);
	if (in_irq()) irq_work_tick();
	scheduler_tick();
		curr->sched_class->task_tick(rq, curr, 0);
	if (IS_ENABLED(CONFIG_POSIX_TIMERS)) run_posix_cpu_timers();



// switch to hres timer
hrtimer_run_queues();
	if (__hrtimer_hres_active(cpu_base))
		return;
	if (tick_check_oneshot_change(!hrtimer_is_hres_enabled()))
		hrtimer_switch_to_hres();
			tick_init_highres();
				tick_switch_to_oneshot(hrtimer_interrupt);
					struct tick_device *td = this_cpu_ptr(&tick_cpu_device);
					td->mode = TICKDEV_MODE_ONESHOT;
					td->dev->event_handler = handler; /* changed handler */
					clockevents_switch_state(dev, CLOCK_EVT_STATE_ONESHOT);
					tick_broadcast_switch_to_oneshot();
			base->hres_active = 1;
			hrtimer_resolution = HIGH_RES_NSEC;
			tick_setup_sched_timer();
				hrtimer_init(&ts->sched_timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS_HARD);
				ts->sched_timer.function = tick_sched_timer;
				hrtimer_set_expires(&ts->sched_timer, tick_init_jiffy_update());
				hrtimer_forward(&ts->sched_timer, now, TICK_NSEC);
				hrtimer_start_expires(&ts->sched_timer, HRTIMER_MODE_ABS_PINNED_HARD);
					hrtimer_start_range_ns(timer, soft, delta, mode);
						__hrtimer_start_range_ns(timer, tim, delta_ns, mode, base)
							hrtimer_force_reprogram(new_base->cpu_base, 1);
								__hrtimer_reprogram(cpu_base, cpu_base->next_timer, expires_next);
									tick_program_event(expires_next, 1); 
										clockevents_program_event(dev, expires, force);
				tick_nohz_activate(ts, NOHZ_MODE_HIGHRES);
			retrigger_next_event(NULL);
		return;
	now = hrtimer_update_base(cpu_base);
	if (!ktime_before(now, cpu_base->softirq_expires_next))
		raise_softirq_irqoff(HRTIMER_SOFTIRQ); 
	__hrtimer_run_queues(cpu_base, now, flags, HRTIMER_ACTIVE_HARD);


void hrtimer_interrupt(struct clock_event_device *dev)
	if (!ktime_before(now, cpu_base->softirq_expires_next))
		raise_softirq_irqoff(HRTIMER_SOFTIRQ); 
	__hrtimer_run_queues(cpu_base, now, flags, HRTIMER_ACTIVE_HARD); /* hard hres timer */
	expires_next = hrtimer_update_next_event(cpu_base);
	if (!tick_program_event(expires_next, 0))
		return;
	//error 	

static enum hrtimer_restart tick_sched_timer(struct hrtimer *timer)
	ktime_t now = ktime_get();
	//(1) do timer
	tick_sched_do_timer(ts, now);  /* tick and time */
		if (tick_do_timer_cpu == cpu)
			tick_do_update_jiffies64(now);
				jiffies_64 += ticks;
				calc_global_load();
				update_wall_time();	
	//(2) update_process_times
	if (regs)
		tick_sched_handle(ts, regs);
			update_process_times(user_mode(regs)); /* timers & schedule */
			profile_tick(CPU_PROFILING);
	else
		ts->next_tick = 0;
	//(3) next timer
	hrtimer_forward(timer, now, TICK_NSEC);
		hrtimer_add_expires(timer, interval);

/* timer */
add_timer(&timer);
	__mod_timer(timer, timer->expires, MOD_TIMER_NOTPENDING);
mod_timer(&timer);
	__mod_timer(timer, expires, 0);
	
del_timer(&timer);
	timer_delete(timer);
		__timer_delete(timer, false);
			if (timer_pending(timer) || shutdown)
				base = lock_timer_base(timer, &flags);
				ret = detach_if_pending(timer, base, true);


/* soft irq */
do_softirq();
	if (in_interrupt()) retrun;
	pending = local_softirq_pending();
	if (pending)
		do_softirq_own_stack();
			call_on_irq_stack(NULL, ____do_softirq);
				__do_softirq();

__do_softirq();
	pending = local_softirq_pending();
	softirq_handle_begin();
	in_hardirq = lockdep_softirq_start();
	account_softirq_enter(current);
	//restart
	set_softirq_pending(0);
	local_irq_enable();
	h = softirq_vec;
	while ((softirq_bit = ffs(pending)))
		h += softirq_bit - 1;
		vec_nr = h - softirq_vec;
		prev_count = preempt_count();
		kstat_incr_softirqs_this_cpu(vec_nr);
		h->action(h);
		h++;
		pending >>= softirq_bit;
	local_irq_disable();
	pending = local_softirq_pending();
	if (pending) //restart
		if (time_before(jiffies, end) && !need_resched() && --max_restart)
			goto restart;
		wakeup_softirqd();
	account_softirq_exit(current);
	lockdep_softirq_end(in_hardirq);
	softirq_handle_end();
	
run_timer_softirq
	__run_timers(base);
		while (time_after_eq(jiffies, base->clk) && time_after_eq(jiffies, base->next_expiry))
			levels = collect_expired_timers(base, heads);
			base->clk++;
			base->next_expiry = __next_timer_interrupt(base); 
			while (levels--)
				expire_timers(base, heads + levels);
					while (!hlist_empty(head))
						timer = hlist_entry(head->first, struct timer_list, entry);
						base->running_timer = timer;
						detach_timer(timer, true);
						fn = timer->function;
						call_timer_fn(timer, fn, baseclk); //irq safe or not

hrtimer_run_softirq(struct softirq_action *h)
	now = hrtimer_update_base(cpu_base);
	__hrtimer_run_queues(cpu_base, now, flags, HRTIMER_ACTIVE_SOFT); /* soft hres timer */
		for_each_active_base(base, cpu_base, active)
			basenow = ktime_add(now, base->offset);
			while ((node = timerqueue_getnext(&base->active)))
				timer = container_of(node, struct hrtimer, node);
				__run_hrtimer(cpu_base, base, timer, &basenow, flags);
					__remove_hrtimer(timer, base, HRTIMER_STATE_INACTIVE, 0);
					restart = timer->function(timer); //tick_sched_timer
	cpu_base->softirq_activated = 0;
	hrtimer_update_softirq_timer(cpu_base, true);




	