[    2.729360] [pid:434,cpu1,in irq] dump_backtrace+0x98/0x118
[    2.729365] [pid:434,cpu1,in irq] show_stack+0x18/0x24
[    2.729366] [pid:434,cpu1,in irq] dump_stack_lvl+0x48/0x60
[    2.729375] [pid:434,cpu1,in irq] dump_stack+0x18/0x24
[    2.729376] [pid:434,cpu1,in irq] panic+0x330/0x3cc
[    2.729384] [pid:434,cpu1,in irq] nmi_panic+0x8c/0x90
[    2.729386] [pid:434,cpu1,in irq] arm64_serror_panic+0x94/0xa0
[    2.729390] [pid:434,cpu1,in irq] arm64_is_fatal_ras_serror+0x3c/0xb4
[    2.729392] [pid:434,cpu1,in irq] do_serror+0x60/0x78
[    2.729393] [pid:434,cpu1,in irq] el1h_64_error_handler+0x30/0x48
[    2.729402] [pid:434,cpu1,in irq] el1h_64_error+0x64/0x68
[    2.729404] [pid:434,cpu1,in irq] kmem_cache_free+0x174/0x358
[    2.729406] [pid:434,cpu1,in irq] acpi_os_release_object+0x10/0x20
[    2.729419] [pid:434,cpu1,in irq] acpi_ps_free_op+0x24/0x48
[    2.729423] [pid:434,cpu1,in irq] acpi_ps_delete_parse_tree+0x68/0xa0
[    2.729425] [pid:434,cpu1,in irq] acpi_ps_complete_this_op+0xb0/0x24c
[    2.729426] [pid:434,cpu1,in irq] acpi_ps_complete_op+0x3c/0x350
[    2.729427] [pid:434,cpu1,in irq] acpi_ps_parse_loop+0x12c/0x690
[    2.729428] [pid:434,cpu1,in irq] acpi_ps_parse_aml+0x8c/0x3a0
[    2.729429] [pid:434,cpu1,in irq] acpi_ps_execute_method+0x120/0x24c
[    2.729431] [pid:434,cpu1,in irq] acpi_ns_evaluate+0x1e8/0x2c0
[    2.729443] [pid:434,cpu1,in irq] acpi_evaluate_object+0x128/0x2b8
[    2.729444] [pid:434,cpu1,in irq] __acpi_power_on+0x30/0x11c
[    2.729452] [pid:434,cpu1,in irq] acpi_power_on_unlocked+0x60/0x80
[    2.729454] [pid:434,cpu1,in irq] acpi_power_on_list+0x4c/0xdc
[    2.729456] [pid:434,cpu1,in irq] acpi_power_transition+0x80/0xc4
[    2.729459] [pid:434,cpu1,in irq] acpi_device_set_power+0x230/0x46c
[    2.729465] [pid:434,cpu1,in irq] acpi_dev_resume+0xbc/0xdc
[    2.729466] [pid:434,cpu1,in irq] acpi_subsys_runtime_resume+0x18/0x44
[    2.729468] [pid:434,cpu1,in irq] __rpm_callback+0x48/0x1dc
[    2.729485] [pid:434,cpu1,in irq] rpm_callback+0x6c/0x78
[    2.729487] [pid:434,cpu1,in irq] rpm_resume+0x514/0x778
[    2.729490] [pid:434,cpu1,in irq] __pm_runtime_resume+0x4c/0x90
[    2.729492] [pid:434,cpu1,in irq] clk_pm_runtime_get.part.0.isra.0+0x1c/0x88
[    2.729501] [pid:434,cpu1,in irq] clk_core_prepare+0x30/0x2d0
[    2.729506] [pid:434,cpu1,in irq] clk_prepare+0x28/0x44


/* clk -> power */
clk_unprepare(clk)
	clk_core_unprepare_lock(clk->core);
		clk_core_unprepare(core);
			if (--core->prepare_count > 0)
				return;
			if (core->ops->unprepare)
				core->ops->unprepare(core->hw);
			clk_core_unprepare(core->parent);
			clk_pm_runtime_put(core);
				pm_runtime_put_sync(core->dev);
					__pm_runtime_idle(dev, RPM_GET_PUT);
						if (rpmflags & RPM_GET_PUT)
							rpm_drop_usage_count(dev);
						rpm_idle(dev, rpmflags);
							callback = RPM_GET_CALLBACK(dev, runtime_idle);
							callback(dev);
							
clk_prepare(clk)
	clk_core_prepare_lock(clk->core);
		clk_core_prepare(core);
			if (core->prepare_count == 0) {
				clk_pm_runtime_get(core);
					pm_runtime_resume_and_get(core->dev);
						 __pm_runtime_resume(dev, RPM_GET_PUT);
							rpm_resume(dev, rpmflags);
								callback = RPM_GET_CALLBACK(dev, runtime_resume); 
								rpm_callback(callback, dev);
				clk_core_prepare(core->parent);
				if (core->ops->prepare)
					core->ops->prepare(core->hw); 
			}
			core->prepare_count++;


/* runtime suspend */
pm_runtime_suspend(struct device *dev)
|| pm_runtime_autosuspend(struct device *dev) 
|| pm_request_autosuspend(struct device *dev) 
|| pm_runtime_put_autosuspend(struct device *dev)
|| pm_runtime_put_sync_suspend(struct device *dev)
|| pm_runtime_put_sync_autosuspend(struct device *dev)
	__pm_runtime_suspend(struct device *dev, int rpmflags)
		rpm_suspend(dev, rpmflags);
			callback = RPM_GET_CALLBACK(dev, runtime_suspend);
			rpm_callback(callback, dev);

pm_runtime_force_suspend(struct device *dev)
	callback = RPM_GET_CALLBACK(dev, runtime_suspend);
	callback(dev);


int pm_schedule_suspend(struct device *dev, unsigned int delay)
	if (!delay)
		rpm_suspend(dev, RPM_ASYNC);
	hrtimer_start(&dev->power.suspend_timer, expires, HRTIMER_MODE_ABS);
		pm_suspend_timer_fn(struct hrtimer *timer)
			if (expires > 0 && expires < ktime_get_mono_fast_ns()) {
				dev->power.timer_expires = 0;
				rpm_suspend(dev, dev->power.timer_autosuspends ? (RPM_ASYNC | RPM_AUTO) : RPM_ASYNC); 
			}

	
/* runtime resume */
	
__pm_runtime_resume(struct device *dev, int rpmflags)
	rpm_resume(dev, rpmflags);
	
pm_runtime_barrier(dev);
	rpm_resume(dev, rpmflags);

__pm_runtime_disable(struct device *dev, bool check_resume)
	if (dev->power.disable_depth > 0)
		dev->power.disable_depth++; go out;
	if (check_resume && dev->power.request_pending && dev->power.request == RPM_REQ_RESUME)
		pm_runtime_get_noresume(dev);
		rpm_resume(dev, 0);
		pm_runtime_put_noidle(dev);
	update_pm_runtime_accounting(dev);
	if (!dev->power.disable_depth++)
		__pm_runtime_barrier(dev);

pm_runtime_forbid(struct device *dev)
	rpm_resume(dev, 0);

update_autosuspend(struct device *dev, int old_delay, int old_use)
	...
	rpm_resume(dev, 0);

//pm_runtime_barrier
__driver_probe_device(struct device_driver *drv, struct device *dev)
	pm_runtime_get_suppliers(dev);
	pm_runtime_get_sync(dev->parent); //if (dev->parent)
	pm_runtime_barrier(dev);
	really_probe(dev, drv);
	pm_request_idle(dev);
	pm_runtime_put(dev->parent);
	pm_runtime_put_suppliers(dev);
	
device_shutdown(void)
	pm_runtime_barrier(dev);

dpm_suspend(pm_message_t state)
	while (!list_empty(&dpm_prepared_list)) 
	device_suspend(dev);
		if (dpm_async_fn(dev, async_suspend))
			__device_suspend(dev, pm_transition, false);
				dpm_wait_for_subordinate(dev, async);
					dpm_wait_for_children(dev, async);
					dpm_wait_for_consumers(dev, async);
				pm_runtime_barrier(dev);
				dpm_watchdog_set(&wd, dev);
				if (dev->pm_domain || dev->type || dev->class->pm || dev->bus->pm || dev->driver->pm) 
					callback = pm_op(dev->xx->pm, state);
				dpm_run_callback(callback, dev, state, info);
				dpm_watchdog_clear(&wd); 
				
/*rpm suspend/resume/idle */

rpm_suspend(struct device *dev, int rpmflags)
	rpm_check_suspend_allowed(dev);
	if (dev->power.runtime_status == RPM_RESUMING && !(rpmflags & RPM_ASYNC))
		return -EAGAIN;
	if ((rpmflags & RPM_AUTO) && dev->power.runtime_status != RPM_SUSPENDING)
		//start timer to auto suspend
	pm_runtime_cancel_pending(dev);
	if (dev->power.runtime_status == RPM_SUSPENDING)
		//wait last suspending
	if (rpmflags & RPM_ASYNC)
		dev->power.request = (rpmflags & RPM_AUTO) ? RPM_REQ_AUTOSUSPEND : RPM_REQ_SUSPEND; 
		//queue work dev->power.work
	__update_runtime_status(dev, RPM_SUSPENDING);
	callback = RPM_GET_CALLBACK(dev, runtime_suspend);
	rpm_callback(callback, dev);
		callback(dev);
		__rpm_put_suppliers(dev, false); //DL_FLAG_PM_RUNTIME need be set
	__update_runtime_status(dev, RPM_SUSPENDED);
	wake_up_all(&dev->power.wait_queue);
	if (dev->power.deferred_resume) 
		rpm_resume(parent, 0);
	if (parent && !parent->power.ignore_children)
		rpm_idle(parent, RPM_ASYNC);
	if (dev->power.links_count > 0)
		rpm_suspend_suppliers(dev); //DL_FLAG_PM_RUNTIME need be set
	//if fail ...
		
rpm_resume(struct device *dev, int rpmflags)
	if (dev->power.runtime_error) || if (dev->power.disable_depth > 0) return error;
	dev->power.request = RPM_REQ_NONE;
	if (dev->power.runtime_status == RPM_RESUMING || dev->power.runtime_status == RPM_SUSPENDING) {
		//wait last suspending or resuming
	if (rpmflags & RPM_ASYNC)
		dev->power.request = RPM_REQ_RESUME; 
		//queue work dev->power.work
	if (!parent && dev->parent)
		...
		pm_runtime_get_noresume(parent);
		rpm_resume(parent, 0);
	__update_runtime_status(dev, RPM_RESUMING);
	callback = RPM_GET_CALLBACK(dev, runtime_resume);
	rpm_callback(callback, dev); 
		rpm_get_suppliers(dev); //DL_FLAG_PM_RUNTIME need be set
		callback(dev)
	wake_up_all(&dev->power.wait_queue);
	if (retval >= 0)
		rpm_idle(dev, RPM_ASYNC); 
	if (parent && !dev->power.irq_safe)
		pm_runtime_put(parent);

rpm_idle(struct device *dev, int rpmflags)
	dev->power.request = RPM_REQ_NONE;
	callback = RPM_GET_CALLBACK(dev, runtime_idle);
	if (rpmflags & RPM_ASYNC)
		dev->power.request = RPM_REQ_IDLE;
		queue_work(pm_wq, &dev->power.work);
	callback(dev);
	wake_up_all(&dev->power.wait_queue);

rpm_callback(callback, dev);
	__rpm_callback(cb, dev);
		if (dev->power.links_count > 0 && dev->power.runtime_status == RPM_RESUMING)
			rpm_get_suppliers(dev);
				list_for_each_entry_rcu(link, &dev->links.suppliers,
					pm_runtime_get_sync(link->supplier);
		cb(dev);
		if (dev->power.links_count > 0 && dev->power.runtime_status == RPM_SUSPENDING) || resmue fail
			__rpm_put_suppliers(dev, false);
				list_for_each_entry_rcu(link, &dev->links.suppliers
					pm_runtime_release_supplier(link);
		
/* acpi */
acpi_subsys_runtime_resume(struct device *dev)
	acpi_dev_resume(dev);
		acpi_dev_pm_full_power(adev);
			acpi_device_set_power(adev, ACPI_STATE_D0);
				acpi_power_transition(device, target_state);
					 acpi_power_on_list(&device->power.states[state].resources);
						list_for_each_entry(entry, list, node)
							acpi_power_on(entry->resource);
								acpi_power_on_unlocked(resource);
									if (resource->ref_count++) // already power on
									__acpi_power_on(resource);
										acpi_evaluate_object(handle, "_ON", NULL, NULL); 
								
		acpi_device_wakeup_disable(adev);
	pm_generic_runtime_resume(dev);
		pm->runtime_resume(dev);

/* dt */



device_initialize(dev);
	device_pm_init(dev);
		device_pm_init_common(dev);
		device_pm_sleep_init(dev);
		pm_runtime_init(dev);
			INIT_WORK(&dev->power.work, pm_runtime_work);
			hrtimer_init(&dev->power.suspend_timer, CLOCK_MONOTONIC, HRTIMER_MODE_ABS);
			dev->power.suspend_timer.function = pm_suspend_timer_fn;

/* pm_runtime_work */
static void pm_runtime_work(struct work_struct *work)
	if (!dev->power.request_pending)
		goto out;
	switch (dev->power.request) {
	case RPM_REQ_IDLE:
		rpm_idle(dev, RPM_NOWAIT);
	case RPM_REQ_SUSPEND:
		rpm_suspend(dev, RPM_NOWAIT);
	case RPM_REQ_AUTOSUSPEND: 
		rpm_suspend(dev, RPM_NOWAIT);
	case RPM_REQ_RESUME:
		rpm_resume(dev, RPM_NOWAIT);
	}
	
	
dev->power.direct_complete
dev->power.runtime_status 	
dev->power.request	
dev->power.idle_notification
dev->power.memalloc_noio


rpm_get_suppliers 
pm_runtime_release_supplier
__rpm_put_suppliers
rpm_put_suppliers
rpm_suspend_suppliers
pm_runtime_get_suppliers
pm_runtime_put_suppliers




pm_runtime_force_suspend(struct device *dev)
//directlly call callback


while(try--) {
	pm_runtime_get(dev);
	if (pm_runtime_active(dev))
		break;
	else
		pm_rumtime_put(dev);
}
if (try < 0)
	pr_err("")
	
	
	
	
	
	
	
	
	

# runtime pm & clk

