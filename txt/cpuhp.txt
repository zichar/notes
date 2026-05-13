



/* bus_type sysfs */
int device_add(struct device *dev)
	device_add_attrs(dev);
		device_create_file(dev, &dev_attr_online);

struct bus_type cpu_subsys = {
	.online = cpu_subsys_online,
	.offline = cpu_subsys_offline,
}


/* init */
int __init topology_init(void)
	for_each_possible_cpu(i)
		struct cpu *cpu = &per_cpu(cpu_data.cpu, i);
		cpu->hotpluggable = cpu_can_disable(i);
		register_cpu(cpu, i);

int register_cpu(struct cpu *cpu, int num)
	cpu->node_id = cpu_to_node(num);
	memset(&cpu->dev, 0x00, sizeof(struct device));
	cpu->dev.id = num;
	cpu->dev.bus = &cpu_subsys;
	...
	device_register(&cpu->dev);
	register_cpu_under_node(num, cpu_to_node(num));


kernel_init(void *unused)
	kernel_init_freeable();
		smp_init();
			idle_threads_init();
			cpuhp_threads_init();
				cpuhp_init_state();
				smpboot_register_percpu_thread(&cpuhp_threads);
				kthread_unpark(this_cpu_read(cpuhp_state.thread));
			bringup_nonboot_cpus(setup_max_cpus);
			smp_cpus_done(setup_max_cpus);
		sched_init_smp();
		
		
static struct smp_hotplug_thread cpuhp_threads = {
	.store→ →       →       = &cpuhp_state.thread,
	.thread_should_run→     = cpuhp_should_run,
	.thread_fn→     →       = cpuhp_thread_fun,
	.thread_comm→   →       = "cpuhp/%u",
	.selfparking→   →       = true,
};




		
int smpboot_register_percpu_thread(struct smp_hotplug_thread *plug_thread)
	for_each_online_cpu(cpu)
		__smpboot_create_thread(plug_thread, cpu);
			td = kzalloc_node(sizeof(*td), GFP_KERNEL, cpu_to_node(cpu));
			tsk = kthread_create_on_cpu(smpboot_thread_fn, td, cpu, ht->thread_comm);
			kthread_set_per_cpu(tsk, cpu);
			kthread_park(tsk);
			*per_cpu_ptr(ht->store, cpu) = tsk; 
			if (ht->create)
				if (!wait_task_inactive(tsk, TASK_PARKED))
					WARN_ON(1);
				else
					ht->create(cpu);
		smpboot_unpark_thread(plug_thread, cpu);
	list_add(&plug_thread->list, &hotplug_threads);
		
		
void cpuhp_thread_fun(unsigned int cpu)	
	st->should_run = cpuhp_next_state(bringup, &state, st, st->target);
	cpuhp_invoke_callback(cpu, state, bringup, st->node, &st->last);
	if (!st->should_run)
		complete_ap_thread(st, bringup);
		
/* online */	
int cpu_subsys_online(struct device *dev)		
	cpu_device_up(dev);
		cpu_up(dev->id, CPUHP_ONLINE);
			try_online_node(cpu_to_node(cpu));
			_cpu_up(cpu, 0, target);
				if (st->state > CPUHP_BRINGUP_CPU)
					cpuhp_kick_ap_work(cpu);
					return
				target = min((int)target, CPUHP_BRINGUP_CPU);
				cpuhp_up_callbacks(cpu, st, target);
					cpuhp_invoke_callback_range(true, cpu, st, target);
				cpu_up_down_serialize_trainwrecks(tasks_frozen);
					cpuset_wait_for_hotplug(); // if (!tasks_frozen)
/* offline */						
cpu_subsys_offline(struct device *dev)
	cpu_device_down(dev);
		cpu_down(dev->id, CPUHP_OFFLINE);
			cpu_down_maps_locked(cpu, target);
				for_each_cpu_and(cpu, cpu_online_mask, housekeeping_cpumask(HK_TYPE_DOMAIN))
					if (cpu != work.cpu)
						work_on_cpu(cpu, __cpu_down_maps_locked, &work); //=>switch cpu

work_on_cpu(cpu, __cpu_down_maps_locked, &work);
	schedule_work_on(cpu, &wfc.work);
	flush_work(&wfc.work);
		__flush_work(work, false);
			wait_for_completion(&barr.done);
				destroy_work_on_stack(&barr.work); 

__cpu_down_maps_locked
	 _cpu_down(work->cpu, 0, work->target);
		if (st->state > CPUHP_TEARDOWN_CPU)
			cpuhp_kick_ap_work(cpu);
			return
		cpuhp_down_callbacks(cpu, st, target);
			cpuhp_invoke_callback_range(false, cpu, st, target); 


cpuhp_invoke_callback_range(false, cpu, st, target); 
	__cpuhp_invoke_callback_range(bringup, cpu, st, target, false);
		while (cpuhp_next_state(bringup, &state, st, target))]
			cpuhp_invoke_callback(cpu, state, bringup, NULL, NULL);
				step = cpuhp_get_step(state);
				if (!step->multi_instance)
					cb = bringup ? step->startup.single : step->teardown.single;
					cb(cpu);
				else
					cbm = bringup ? step->startup.multi : step->teardown.multi;
					cbm(cpu, node); //hlist_for_each(node, &step->list)

						
smpboot_create_threads
perf_event_init_cpu				
random_prepare_cpu
workqueue_prepare_cpu					
hrtimers_prepare_cpu						
smpcfd_prepare_cpu					
relay_prepare_cpu						
slab_prepare_cpu						
rcutree_prepare_cpu						
rcutree_prepare_cpu						
timers_prepare_cpu						
cpuhp_kick_ap_alive,cpuhp_bringup_ap/bringup_cpu
sched_cpu_starting	
smpboot_unpark_threads	
irq_affinity_online_cpu
perf_event_init_cpu
lockup_detector_online_cpu
workqueue_online_cpu
random_online_cpu
rcutree_online_cpu
sched_cpu_activate				
						
						



CPUHP_TEARDOWN_CPU:
int takedown_cpu(unsigned int cpu)
	stop_machine_cpuslocked(take_cpu_down, NULL, cpumask_of(cpu));
		int take_cpu_down(void *_param)
			__cpu_disable();
			//target = max((int)st->target, CPUHP_AP_OFFLINE);
			cpuhp_invoke_callback_range_nofail(false, cpu, st, target);	
			tick_handover_do_timer();
			tick_offline_cpu(cpu); 
			stop_machine_park(cpu); 
	wait_for_ap_thread(st, false);
	hotplug_cpu__broadcast_tick_pull(cpu);
	__cpu_die(cpu);
	cpuhp_bp_sync_dead(cpu);
	tick_cleanup_dead_cpu(cpu);
	rcutree_migrate_callbacks(cpu);


cpu idle 
enter tfa
psci power off self

























