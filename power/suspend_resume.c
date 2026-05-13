/* suspend */

kernel/power/main.c
state_store()
if (state < PM_SUSPEND_MAX)
	pm_suspend(state)
if (state == PM_SUSPEND_MAX)
	hibernate()
	
	




pm_suspend(state)
	enter_state(state);
		suspend_prepare(state);
			pm_prepare_console();
			pm_notifier_call_chain_robust(PM_SUSPEND_PREPARE, PM_POST_SUSPEND);
			suspend_freeze_processes(); 
		suspend_devices_and_enter(state);
			platform_suspend_begin(state);
			suspend_console();
			dpm_suspend_start(PMSG_SUSPEND);
				dpm_prepare(state);
					while (!list_empty(&dpm_list) && !error) {
						device_prepare(dev, PMSG_SUSPEND);
							==>(dev->pm_domain, dev->type, dev->class, dev->driver->pm)->prepare
						if (!list_empty(&dev->power.entry)) 
							list_move_tail(&dev->power.entry, &dpm_prepared_list);
					}
				dpm_suspend(state);
					devfreq_suspend();
					cpufreq_suspend();
					while (!list_empty(&dpm_prepared_list)) {
						device_suspend(dev); 
							__device_suspend(dev, pm_transition, false);
								pm_runtime_barrier(dev);
								//if (dev->power.direct_complete) {
								==>(dev->pm_domain, dev->type, dev->class, dev->driver->pm)->suspend
								complete_all(&dev->power.completion);
						if (!list_empty(&dev->power.entry))
							list_move(&dev->power.entry, &dpm_suspended_list); 
					}
					async_synchronize_full();
			do {
				suspend_enter(state, &wakeup); 
					 platform_suspend_prepare(state); //suspend_ops->prepare()
					 dpm_suspend_late(PMSG_SUSPEND);
						while (!list_empty(&dpm_suspended_list)) {
							device_suspend_late(dev);
								 __device_suspend_late(dev, pm_transition, false);
									==>(dev->pm_domain, dev->type, dev->class, dev->driver->pm)->suspend_late
							if (!list_empty(&dev->power.entry))
								list_move(&dev->power.entry, &dpm_late_early_list);
						}
					 platform_suspend_prepare_late(state);
					 dpm_suspend_noirq(PMSG_SUSPEND);
						device_wakeup_arm_wake_irqs();
						suspend_device_irqs();
							for_each_irq_desc(irq, desc)
								suspend_device_irq(desc);
									if (irqd_is_wakeup_set(irqd))
										__enable_irq(desc);
										irqd_set(irqd, IRQD_IRQ_ENABLED_ON_SUSPEND);
							desc->istate |= IRQS_SUSPENDED; 
							__disable_irq(desc); 
							if (chipflags & IRQCHIP_MASK_ON_SUSPEND) 
								mask_irq(desc);		
						dpm_noirq_suspend_devices(state);
							while (!list_empty(&dpm_late_early_list)) {
								device_suspend_noirq(dev);
									__device_suspend_noirq(dev, pm_transition, false);
										==>(dev->pm_domain, dev->type, dev->class, dev->driver->pm)->suspend_noirq
								if (!list_empty(&dev->power.entry))
									list_move(&dev->power.entry, &dpm_noirq_list);
							}
					 platform_suspend_prepare_noirq(state); //suspend_ops->wake();
					 if (state == PM_SUSPEND_TO_IDLE) s2idle_loop();goto Platform_wake;
					 pm_sleep_disable_secondary_cpus(); 
					 arch_suspend_disable_irqs(); 
					 system_state = SYSTEM_SUSPEND; 
					 syscore_suspend();
					 if (!error) {
						*wakeup = pm_wakeup_pending();
						suspend_ops->enter(state); //if ok
						//------//
						syscore_resume();
					 }
					 system_state = SYSTEM_RUNNING;
					 arch_suspend_enable_irqs();
					 pm_sleep_enable_secondary_cpus();
					 platform_resume_noirq(state);
					 dpm_resume_noirq(PMSG_RESUME);->resume_noirq  //acpi dev pnp->resume_noirq   platform dev cix2020->resume_noirq <- soc
						dpm_noirq_resume_devices(state);
							while (!list_empty(&dpm_noirq_list)) {
								list_move_tail(&dev->power.entry, &dpm_late_early_list);
								device_resume_noirq(dev, state, false); 
							}
						resume_device_irqs();
						device_wakeup_disarm_wake_irqs(); 
					 platform_resume_early(state);
					 dpm_resume_early(PMSG_RESUME);->resume_early
						while (!list_empty(&dpm_late_early_list)) {
							list_move_tail(&dev->power.entry, &dpm_suspended_list); 
							device_resume_early(dev, state, false);
						}
					 platform_resume_finish(state);
			while (!error && !wakeup && platform_suspend_again(state));
			dpm_resume_end(PMSG_RESUME);
				dpm_resume(state);->resume
					while (!list_empty(&dpm_suspended_list)) {
						device_resume(dev, state, false);
						list_move_tail(&dev->power.entry, &dpm_prepared_list);
					}
				dpm_complete(state);->complete
					while (!list_empty(&dpm_prepared_list))
						device_complete(dev, state);
			resume_console(); 
			platform_resume_end(state);
			platform_recover(state);
		suspend_finish();
			



/* acpi */

static struct dev_pm_domain acpi_general_pm_domain = {
	.ops = {
		.suspend_late = acpi_subsys_suspend_late,
		.suspend_noirq = acpi_subsys_suspend_noirq,
	}
}

int acpi_subsys_suspend_late(struct device *dev)
	pm_generic_suspend_late(dev);
	ret ? ret : acpi_dev_suspend(dev, device_may_wakeup(dev));
		if (wakeup && acpi_device_can_wakeup(adev))
			acpi_device_wakeup_enable(adev, target_state);
		acpi_dev_pm_low_power(dev, adev, target_state);	
			ret = acpi_dev_pm_get_state(dev, adev, system_state, NULL, &state);
			ret ? ret : acpi_device_set_power(adev, state);
				...
				acpi_power_transition(device, state); 
					 acpi_power_off_list(&device->power.states[device->power.state].resources);
						list_for_each_entry_reverse(entry, list, node)
							acpi_power_off(entry->resource);
								acpi_power_off_unlocked(resource);
									__acpi_power_off(resource);
										acpi_evaluate_object(handle, "_OFF", NULL, NULL);

acpi_subsys_suspend_noirq(struct device *dev)
	pm_generic_suspend_noirq(dev);
		const struct dev_pm_ops *pm = dev->driver ? dev->driver->pm : NULL;
		pm && pm->suspend_noirq ? pm->suspend_noirq(dev) : 0;


pci_pm_resume(struct device *dev)
pci_pm_reenable_device(pci_dev);
pci_reenable_device(pci_dev);
do_pci_enable_device(dev, (1 << PCI_NUM_RESOURCES) - 1); 
pci_set_power_state(dev, PCI_D0);
pci_platform_power_transition
platform_pci_set_power_state
int acpi_pci_set_power_state(struct pci_dev *dev, pci_power_t state)
	acpi_device_set_power(adev, state_conv[state]);  


pci_pm_runtime_suspend(struct device *dev)
 pci_finish_runtime_suspend(pci_dev);
pci_set_power_state(dev, PCI_D0);
pci_platform_power_transition
platform_pci_set_power_state
int acpi_pci_set_power_state(struct pci_dev *dev, pci_power_t state)
	acpi_device_set_power(adev, state_conv[state]);  
	
	

pci_pm_resume_noirq
	pci_pm_power_up_and_verify_state(pci_dev);
		pci_pm_default_resume_early(pci_dev);
			pci_power_up
				acpi_pci_set_power_state

platform_resume_noirq
	system_ops->wake() => acpi_pm_finish
		void acpi_resume_power_resources(void)
			__acpi_power_on(resource);


acpi_add_power_resource(acpi_handle handle)
	if (acpi_power_get_state(resource, &state_dummy)) 
		__acpi_power_on(resource);
		

/* DT SCMI */


genpd_resume_noirq
	genpd_finish_resume(dev, pm_generic_resume_noirq);
		genpd_sync_power_on(genpd, true, 0);
			list_for_each_entry(link, &genpd->child_links, child_node)
				genpd_sync_power_on(link->parent, use_lock, depth + 1);
			_genpd_power_on(genpd, false);
				genpd->power_on(genpd);
		pm_generic_resume_noirq(dev);
			drv->pm->resume_noirq(dev)
			
scmi_pd->genpd.power_on = scmi_pd_power_on;			
		

			
genpd_suspend_noirq(struct device *dev)		
	genpd_finish_suspend(dev, pm_generic_suspend_noirq, pm_generic_resume_noirq);
		suspend_noirq(dev);
		genpd_sync_power_off(genpd, true, 0);
			_genpd_power_off(genpd, false)
			
			
			
			
scmi_pd->genpd.power_on = scmi_pd_power_on;

scmi_pd_power_on(struct generic_pm_domain *domain)			
	scmi_pd_power(domain, true); 			
			




/* */
power src放在PNP0A08节点逻辑：
 系统启动，acpi添加power resource会调用一次on， power on，ref=1
suspend，无操作
硬件x8 power domian 掉电
resume流程platform_resume_noirq，中发现 x8 domian被掉电（读asl），且ref=1，不匹配，调用power on
sky1 pcie resume正常访问寄存器			
			

static const struct platform_suspend_ops acpi_suspend_ops = {
	.valid = acpi_suspend_state_valid,
	.begin = acpi_suspend_begin,
	.prepare_late = acpi_pm_prepare,
	.enter = acpi_suspend_enter,
	.wake = acpi_pm_finish,
	.end = acpi_pm_end,
}

