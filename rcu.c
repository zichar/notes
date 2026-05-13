rcu --tree

start_kernel()
	rcu_init()
		kfree_rcu_batch_init();
		sanitize_kthread_prio();
		rcu_init_geometry();  //rcu_num_nodes, num_rcu_lvl
		rcu_init_one();      //rnp: rcu node point
		if (use_softirq)
			open_softirq(RCU_SOFTIRQ, rcu_core_si);
		pm_notifier(rcu_pm_notify, 0);
		rcutree_prepare_cpu(cpu);  //rdp: rcu data point
		rcu_cpu_starting(cpu); 
		rcutree_online_cpu(cpu);
		rcu_gp_wq = alloc_workqueue("rcu_gp", WQ_MEM_RECLAIM, 0);
		rcu_alloc_par_gp_wq();
		(void)start_poll_synchronize_rcu_expedited();
		
rcu_core_si()
	rcu_core()
		
		
		
update_process_times()
	rcu_sched_clock_irq(user_tick);
		raw_cpu_inc(rcu_data.ticks_this_gp);
		if (smp_load_acquire(this_cpu_ptr(&rcu_data.rcu_urgent_qs)))
			if (!rcu_is_cpu_rrupt_from_idle() && !user)
				set_tsk_need_resched(current); 
				set_preempt_need_resched();
			__this_cpu_write(rcu_data.rcu_urgent_qs, false);
		rcu_flavor_sched_clock_irq(user);
			if (rcu_preempt_depth() > 0 || (preempt_count() & (PREEMPT_MASK | SOFTIRQ_MASK)))
				if (rcu_preempt_need_deferred_qs(t))
					set_tsk_need_resched(t);
					set_preempt_need_resched();
			else if (rcu_preempt_need_deferred_qs(t))
				rcu_preempt_deferred_qs(t);
				return;
			else if (!WARN_ON_ONCE(rcu_preempt_depth()))
				rcu_qs();
					if (rcu_ctrlblk.donetail != rcu_ctrlblk.curtail)
						rcu_ctrlblk.donetail = rcu_ctrlblk.curtail;
						raise_softirq_irqoff(RCU_SOFTIRQ);
					WRITE_ONCE(rcu_ctrlblk.gp_seq, rcu_ctrlblk.gp_seq + 2);	
				return;
		if (rcu_pending(user))
			invoke_rcu_core(); //rcu_softirq or rcu_core_thread
		if (user || rcu_is_cpu_rrupt_from_idle())
			rcu_note_voluntary_context_switch(current);

rcu_pending(user)
	check_cpu_stall(rdp); 
	if (rcu_nocb_need_deferred_wakeup(rdp, RCU_NOCB_WAKE))
		return 1;
	if ((user || rcu_is_cpu_rrupt_from_idle()) && rcu_nohz_full_cpu())
		return 0;
	gp_in_progress = rcu_gp_in_progress();
	if (rdp->core_needs_qs && !rdp->cpu_no_qs.b.norm && gp_in_progress)
		return 1;
	if (!rcu_rdp_is_offloaded(rdp) && rcu_segcblist_ready_cbs(&rdp->cblist))
		return 1;
	if (!gp_in_progress
		&& rcu_segcblist_is_enabled(&rdp->cblist) 
		&& !rcu_rdp_is_offloaded(rdp) 
		&& !rcu_segcblist_restempty(&rdp->cblist, RCU_NEXT_READY_TAIL))
		return 1;
	if (rcu_seq_current(&rnp->gp_seq) != rdp->gp_seq || unlikely(READ_ONCE(rdp->gpwrap)))
		return 1;
	return 0;
	
	
	
	
	


















