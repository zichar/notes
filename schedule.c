schedule

schedule()
	sched_submit_work(current);
	do {
		preempt_disable();
		__schedule(SM_NONE);
		sched_preempt_enable_no_resched();
	} while(need_resched());
	sched_update_worker(current);


__schedule(unsigned int sched_mode)
	cpu = smp_processor_id();
	rq = cpu_rq(cpu);
	prev = rq->curr;
	if (!(sched_mode & SM_MASK_PREEMPT) && prev_state)
		if (signal_pending_state(prev_state, prev))
			WRITE_ONCE(prev->__state, TASK_RUNNING);
		else
			deactivate_task(rq, prev, DEQUEUE_SLEEP | DEQUEUE_NOCLOCK);
	next = pick_next_task(rq, prev, &rf);
	clear_tsk_need_resched(prev);
	clear_preempt_need_resched();
	if (likely(prev != next))
		rq = context_switch(rq, prev, next, &rf); 
	else
		__balance_callbacks(rq);

context_switch(rq, prev, next, &rf);
	if (!next->mm)
		enter_lazy_tlb(prev->active_mm, next);
		next->active_mm = prev->active_mm;
		if (prev->mm)
			mmgrab_lazy_tlb(prev->active_mm);
		else
			prev->active_mm = NULL;
	else
		membarrier_switch_mm(rq, prev->active_mm, next->mm);
		switch_mm_irqs_off(prev->active_mm, next->mm, next);
		...
	switch_to(prev, next, prev);
	barrier();
	finish_task_switch(prev);



//for sres timer
tick_handle_periodic(struct clock_event_device *dev)
	//(1) do timer
	//(2) update_process_times (schedule)
	tick_periodic(cpu);
		update_process_times(user_mode(get_irq_regs()))
			run_local_timers();
			scheduler_tick();
				curr->sched_class->task_tick(rq, curr, 0); //=> task_tick_fair()
				trigger_load_balance(rq);
	//(3) update timer

//for hres timer
static enum hrtimer_restart tick_sched_timer(struct hrtimer *timer)
	//(1) do timer
	tick_sched_do_timer(ts, now);  /* tick and time */
	//(2) update_process_times (schedule)
	if (regs)
		tick_sched_handle(ts, regs);
			update_process_times(user_mode(regs)); /* timers & schedule */
				run_local_timers();
				scheduler_tick();
					curr->sched_class->task_tick(rq, curr, 0); //=> task_tick_fair()
					trigger_load_balance(rq);
	//(3) update timer

task_tick_fair(struct rq *rq, struct task_struct *curr, int queued)
	for_each_sched_entity(se) {
		cfs_rq = cfs_rq_of(se); 
		entity_tick(cfs_rq, se, queued);
			update_curr(cfs_rq);
				curr->vruntime += calc_delta_fair(delta_exec, curr);
				update_deadline(cfs_rq, curr);
					if (cfs_rq->nr_running > 1)
						resched_curr(rq_of(cfs_rq));
				update_min_vruntime(cfs_rq);
	}



wake_up_process(struct task_struct *p)
	try_to_wake_up(p, TASK_NORMAL, 0);
		ttwu_queue(p, cpu, wake_flags);
			update_rq_clock(rq);
			ttwu_do_activate(rq, p, wake_flags, &rf);
				check_preempt_curr(rq, p, wake_flags);
					if (p->sched_class == rq->curr->sched_class)
						rq->curr->sched_class->check_preempt_curr(rq, p, flags);
					else if (sched_class_above(p->sched_class, rq->curr->sched_class))
						resched_curr(rq);
				ttwu_do_wakeup(p);
					WRITE_ONCE(p->__state, TASK_RUNNING);






