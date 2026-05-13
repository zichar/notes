







irq_domain


# irq_domain_create_tree()
irq_domain_create_tree(handle, &gic_irq_domain_ops

struct irq_domain *irq_domain_create_tree(struct fwnode_handle *fwnode, const struct irq_domain_ops *ops, void *host_data)
	__irq_domain_add(fwnode, 0, ~0, 0, ops, host_data);

__irq_domain_add(fwnode, 0, ~0, 0, ops, host_data);


struct irq_domain *__irq_domain_add(struct fwnode_handle *fwnode, unsigned int size,
→       →       →       →           irq_hw_number_t hwirq_max, int direct_max,
→       →       →       →           const struct irq_domain_ops *ops,
→       →       →       →           void *host_data)
	__irq_domain_create(fwnode, size, hwirq_max, direct_max, ops, host_data);
		domain = kzalloc_node(struct_size(domain, revmap, size), GFP_KERNEL, of_node_to_nid(to_of_node(fwnode)));
		domain->fwnode = fwnode_handle_get(fwnode);
		domain->ops = ops;
		domain->host_data = host_data;
		domain->hwirq_max = hwirq_max;
		domain->root = domain;
		irq_domain_check_hierarchy(domain); //if (domain->ops->alloc) domain->flags |= IRQ_DOMAIN_FLAG_HIERARCHY;
	__irq_domain_publish(domain);
		debugfs_add_domain_dir(domain);
		list_add(&domain->link, &irq_domain_list);




# platform_get_irq()
int platform_get_irq(struct platform_device *dev, unsigned int num)
	platform_get_irq_optional(dev, num);
		of_irq_get(dev->dev.of_node, num); 
			of_irq_parse_one(dev, index, &oirq);
			domain = irq_find_host(oirq.np);
				list_for_each_entry(h, &irq_domain_list, link) {
					if (h->ops->select && fwspec->param_count)
						h->ops->select(h, fwspec, bus_token);
					else if (h->ops->match)
						rc = h->ops->match(h, to_of_node(fwnode), bus_token);
					else
						rc = ((fwnode != NULL) && (h->fwnode == fwnode) &&
							((bus_token == DOMAIN_BUS_ANY) || (h->bus_token == bus_token)));
				}
			irq_create_of_mapping(&oirq);
				irq_create_fwspec_mapping(&fwspec);
					domain = irq_find_matching_fwspec(fwspec, DOMAIN_BUS_WIRED/DOMAIN_BUS_ANY);
					irq_domain_translate(domain, fwspec, &hwirq, &type)
					virq = irq_find_mapping(domain, hwirq);
					if (virq) {
						goto out;
					}
					if (irq_domain_is_hierarchy(domain))
						virq = irq_domain_alloc_irqs_locked(domain, -1, 1, NUMA_NO_NODE, fwspec, false, NULL);
					else
						virq = irq_create_mapping_affinity_locked(domain, hwirq, NULL);
					irqd_set_trigger_type(irq_get_irq_data(virq), type);
		acpi_irq_get(ACPI_HANDLE(&dev->dev), num, r); 
			acpi_irq_parse_one(handle, index, &fwspec, &flags);
			domain = irq_find_matching_fwnode(fwspec.fwnode, DOMAIN_BUS_ANY); //else: domain = irq_default_domain;
				... //same
			irq_create_fwspec_mapping(&fwspec);


virq = irq_domain_alloc_irqs_locked(domain, -1, 1, NUMA_NO_NODE, fwspec/arg, false, NULL);
	virq = irq_domain_alloc_descs(irq_base, nr_irqs, 0, node, affinity);
		virq = __irq_alloc_descs(-1, hint/1, cnt, node, THIS_MODULE, affinity);
			virq = alloc_descs(start, cnt, node, affinity, owner);
	irq_domain_alloc_irq_data(domain, virq, nr_irqs)
	irq_domain_alloc_irqs_hierarchy(domain, virq, nr_irqs, arg);
		domain->ops->alloc(domain, irq_base, nr_irqs, arg);
	for (i = 0; i < nr_irqs; i++)
		irq_domain_trim_hierarchy(virq + i);
	for (i = 0; i < nr_irqs; i++)
		irq_domain_insert_irq(virq + i);


virq = irq_create_mapping_affinity_locked(domain, hwirq, NULL);
	virq = irq_domain_alloc_descs(-1, 1, hwirq, of_node_to_nid(of_node), affinity);
	irq_domain_associate_locked(domain, virq, hwirq)
	


# requst_thread_irq()
request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags, const char *name, void *dev)
	request_threaded_irq(irq, handler, NULL, flags, name, dev);
	
int request_threaded_irq(unsigned int irq, irq_handler_t handler,
→       →       →        irq_handler_t thread_fn, unsigned long irqflags,
→       →       →        const char *devname, void *dev_id)
	desc = irq_to_desc(irq); 
	if (!handler) handler = irq_default_primary_handler; //return IRQ_WAKE_THREAD;
	action = kzalloc(sizeof(struct irqaction), GFP_KERNEL);
	action->handler = handler;
	action->thread_fn = thread_fn;
	action->flags = irqflags;
	action->name = devname;
	action->dev_id = dev_id;
	irq_chip_pm_get(&desc->irq_data);
	__setup_irq(irq, desc, action);
		new->irq = irq; 
		nested = irq_settings_is_nested_thread(desc);
		if (nested)
			new->handler = irq_nested_primary_handler; //return IRQ_NONE;
		else
			if (irq_settings_can_thread(desc))
				irq_setup_forced_threading(new);
		if (new->thread_fn && !nested)
			setup_irq_thread(new, irq, false);
				if (new->secondary)
					setup_irq_thread(new->secondary, irq, true); 
		if (desc->irq_data.chip->flags & IRQCHIP_ONESHOT_SAFE) 
			new->flags &= ~IRQF_ONESHOT;
		if (!desc->action)
			irq_request_resources(desc); //desc->irq_data->chip->irq_request_resources(desc);
		old_ptr = &desc->action;
		old = *old_ptr;
		if (old)
			...
		if (new->flags & IRQF_ONESHOT) 
			new->thread_mask = 1UL << ffz(thread_mask); 
		else if (new->handler == irq_default_primary_handler &&
				!(desc->irq_data.chip->flags & IRQCHIP_ONESHOT_SAFE)) 
			pr_err(...)
		if (!shared) {
			__irq_set_trigger(desc, new->flags & IRQF_TRIGGER_MASK);
			irq_activate(desc); 
			... //flags
			if (!(new->flags & IRQF_NO_AUTOEN) && irq_settings_can_autoenable(desc))
				irq_startup(desc, IRQ_RESEND, IRQ_START_COND); 
		} else if (new->flags & IRQF_TRIGGER_MASK) {
			... //check
		}
		*old_ptr = new;
		if (shared && (desc->istate & IRQS_SPURIOUS_DISABLED))
			desc->istate &= ~IRQS_SPURIOUS_DISABLED;
			__enable_irq(desc);
		irq_pm_install_action(desc, new);
		register_irq_proc(irq, desc); 
		new->dir = NULL;
		register_handler_proc(irq, new);
//
int irq_startup(struct irq_desc *desc, bool resend, bool force)
	if (irqd_is_started(d)) {
		irq_enable(desc);
	} else {
		switch (__irq_startup_managed(desc, aff, force){
		case IRQ_STARTUP_NORMAL:
			if (d->chip->flags & IRQCHIP_AFFINITY_PRE_STARTUP)
				irq_setup_affinity(desc);
			__irq_startup(desc);
			if (!(d->chip->flags & IRQCHIP_AFFINITY_PRE_STARTUP))
				irq_setup_affinity(desc);
		case IRQ_STARTUP_MANAGED:
			irq_do_set_affinity(d, aff, false);
			__irq_startup(desc);
		case IRQ_STARTUP_ABORT:
			irqd_set_managed_shutdown(d);
		}
		if (resend)
			check_irq_resend(desc, false);
	}
//	
static int __irq_startup(struct irq_desc *desc)
	if (d->chip->irq_startup) {
		ret = d->chip->irq_startup(d);
		irq_state_clr_disabled(desc);
		irq_state_clr_masked(desc);
	} else {
		irq_enable(desc);
	}
	irq_state_set_started(desc);
		irqd_set(&desc->irq_data, IRQD_IRQ_STARTED); 
//		
irq_enable(struct irq_desc *desc)
	if (!irqd_irq_disabled(&desc->irq_data)) {
		unmask_irq(desc);
	} else {
		irq_state_clr_disabled(desc);
		if (desc->irq_data.chip->irq_enable) {
			desc->irq_data.chip->irq_enable(&desc->irq_data);
			irq_state_clr_masked(desc); 
		} else {
			unmask_irq(desc);
				if (desc->irq_data.chip->irq_unmask)
					desc->irq_data.chip->irq_unmask(&desc->irq_data);
		}
	}
//	
__irq_startup_managed(struct irq_desc *desc, const struct cpumask *aff, bool force)
	if (!irqd_affinity_is_managed(d))
		return IRQ_STARTUP_NORMAL;
	irqd_clr_managed_shutdown(d);
	if (cpumask_any_and(aff, cpu_online_mask) >= nr_cpu_ids) {
		if (WARN_ON_ONCE(force)) 
			return IRQ_STARTUP_ABORT; 
		return IRQ_STARTUP_ABORT; 
	}
	if (WARN_ON(irq_domain_activate_irq(d, false)))
		return IRQ_STARTUP_ABORT;
	return IRQ_STARTUP_MANAGED;
	
	
	
	
	
	
#irq_domain_set_info()
.alloc = gic_irq_domain_alloc,
	ret = gic_irq_domain_translate(domain, fwspec, &hwirq, &type);
	for (i = 0; i < nr_irqs; i++)
		gic_irq_domain_map(domain, virq + i, hwirq + i);
			irq_domain_set_info(d, irq, hw, chip, d->host_data, handle_percpu_devid_irq, NULL, NULL);
//		
void irq_domain_set_info(struct irq_domain *domain, unsigned int virq,
→       →       →        irq_hw_number_t hwirq, const struct irq_chip *chip,	
→       →       →        void *chip_data, irq_flow_handler_t handler,
→       →       →        void *handler_data, const char *handler_name)
	irq_domain_set_hwirq_and_chip(domain, virq, hwirq, chip, chip_data);
		irq_data = irq_domain_get_irq_data(domain, virq);
		irq_data->hwirq = hwirq;
		irq_data->chip = (struct irq_chip *)(chip ? chip : &no_irq_chip);
		irq_data->chip_data = chip_data;
	__irq_set_handler(virq, handler, 0, handler_name);
		__irq_do_set_handler(desc, handle, is_chained, name);
			desc->handle_irq = handle;
			if (handle != handle_bad_irq && is_chained) {
				desc->action = &chained_action;
				irq_activate_and_startup(desc, IRQ_RESEND);
				desc->irq_common_data.handler_data = data;
			}
	irq_set_handler_data(virq, handler_data);
	


#set_handle_irq();
set_handle_irq(gic_handle_irq);
	handle_arch_irq = handle_irq;

.S
el1h_64_fiq_handler(struct pt_regs *regs)/el1h_64_irq_handler(struct pt_regs *regs)
	el1_interrupt(regs, handle_arch_fiq);/el1_interrupt(regs, handle_arch_irq)

//
void handle_percpu_devid_irq(struct irq_desc *desc) 
	action->handler(irq, raw_cpu_ptr(action->percpu_dev_id));





//
gic_handle_irq(struct pt_regs *regs)
	__gic_handle_irq_from_irqsoff(regs);/__gic_handle_irq_from_irqson(regs);
		irqnr = gic_read_iar();
		__gic_handle_irq(irqnr, regs);
			gic_complete_ack(irqnr);
				generic_handle_domain_irq(gic_data.domain, irqnr)
					handle_irq_desc(irq_resolve_mapping(domain, hwirq));
						data = irq_desc_get_irq_data(desc);
						generic_handle_irq_desc(desc);
							desc->handle_irq(desc);

















//
int driver_probe_device(struct device_driver *drv, struct device *dev)
	pm_runtime_get_suppliers(dev); 
	if (dev->parent) 
		pm_runtime_get_sync(dev->parent);
	pm_runtime_barrier(dev);
	really_probe(dev, drv);
	if (dev->parent)
		...
		
		
		
		



