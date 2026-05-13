GPIO





/* gpio irq */
static int cdns_gpio_probe(struct platform_device *pdev)
	irq = platform_get_irq(pdev, 0);
	if (irq >= 0) {
		struct gpio_irq_chip *girq;
		girq = &cgpio->gc.irq;
		gpio_irq_chip_set_chip(girq, &cdns_gpio_irqchip);
		girq->parent_handler = cdns_gpio_irq_handler;
		girq->num_parents = 1;
		girq->parents = devm_kcalloc(&pdev->dev, 1, sizeof(*girq->parents), GFP_KERNEL);
		girq->parents[0] = irq;
		girq->default_type = IRQ_TYPE_NONE; 
		girq->handler = handle_level_irq;
	}
	devm_gpiochip_add_data(&pdev->dev, &cgpio->gc, cgpio);

devm_gpiochip_add_data(dev, gc, data)
gpiochip_add_data_with_key(struct gpio_chip *gc, void *data, struct lock_class_key *lock_key, struct lock_class_key *request_key)
	gdev = kzalloc(sizeof(*gdev), GFP_KERNEL);
	gdev->dev.bus = &gpio_bus_type;
	gdev->dev.parent = gc->parent;
	gdev->chip = gc;
	gpiochip_get_ngpios(gc, &gdev->dev);
	gdev->descs = kcalloc(gc->ngpio, sizeof(*gdev->descs), GFP_KERNEL);
	gdev->base = gc->base < 0 ? gpiochip_find_base(gc->ngpio) : gc->base;
	gpiodev_add_to_list(gdev);
	gdev->descs[i].gdev = gdev; 
	of_gpiochip_add(gc);
		of_gpiochip_add_pin_range(chip); 
		of_gpiochip_scan_gpios(chip);
	gpiochip_add_pin_ranges(gc);
		gc->add_pin_ranges(gc); // if (gc->add_pin_ranges)
	acpi_gpiochip_add(gc); 
		adev = ACPI_COMPANION(chip->parent);
		acpi_gpio = kzalloc(sizeof(*acpi_gpio), GFP_KERNEL);
		acpi_gpio->chip = chip;
		acpi_attach_data(adev->handle, acpi_gpio_chip_dh, acpi_gpio); 
		acpi_gpiochip_request_regions(acpi_gpio);
			acpi_install_address_space_handler(handle, ACPI_ADR_SPACE_GPIO, acpi_gpio_adr_space_handler, NULL, achip);				
		acpi_gpiochip_scan_gpios(acpi_gpio); 
			device_for_each_child_node(chip->parent, fwnode)
				acpi_gpiochip_parse_own_gpio(achip, fwnode, &name, &lflags, &dflags);
				gpiod_hog(desc, name, lflags, dflags);
		acpi_dev_clear_dependencies(adev);
	machine_gpiochip_add(gc);
	//irq
	gpiochip_irqchip_init_valid_mask(gc); //->girq->init_valid_mask(gc, girq->valid_mask, gc->ngpio);
	gpiochip_irqchip_init_hw(gc); //->girq->init_hw(gc);
	gpiochip_add_irqchip(gc, lock_key, request_key);
		if (gpiochip_hierarchy_is_hierarchical(gc))
			gpiochip_hierarchy_create_domain(gc)
				gpiochip_hierarchy_setup_domain_ops(&gc->irq.child_irq_domain_ops); //gpiochip_hierarchy_irq_domain_alloc()
				domain = irq_domain_create_hierarchy(gc->irq.parent_domain, 0, gc->ngpio, gc->irq.fwnode, &gc->irq.child_irq_domain_ops, gc);
					__irq_domain_create(fwnode, size, size, 0, ops, host_data);
						domain = kzalloc_node(struct_size(domain, revmap, size), GFP_KERNEL, of_node_to_nid(to_of_node(fwnode)));
						domain->ops = ops;
						domain->host_data = host_data;
						domain->root = domain;
					__irq_domain_publish(domain);
						debugfs_add_domain_dir(domain);
						list_add(&domain->link, &irq_domain_list);
						pr_debug("Added domain %s\n", domain->name);
				gpiochip_set_hierarchical_irqchip(gc, gc->irq.chip);
		else
			gpiochip_simple_create_domain(gc);
				domain = irq_domain_create_simple(fwnode, gc->ngpio, gc->irq.first, &gpiochip_domain_ops, gc);
					domain = __irq_domain_add(fwnode, size, size, 0, ops, host_data);
					if (first_irq > 0)
						if (IS_ENABLED(CONFIG_SPARSE_IRQ)
							irq_alloc_descs(first_irq, first_irq, size, of_node_to_nid(to_of_node(fwnode)));
								start = irq_find_free_area(from, cnt);
								alloc_descs(start, cnt, node, affinity, owner);
									for (i = 0; i < cnt; i++)
										desc = alloc_desc(start + i, node, flags, mask, owner);
										irq_insert_desc(start + i, desc);
										irq_sysfs_add(start + i, desc); 
										irq_add_debugfs_entry(start + i, desc); //try trigger ops
						irq_domain_associate_many(domain, first_irq, 0, size);	
					__irq_domain_publish(domain);
		if (gc->irq.parent_handler)
			for (i = 0; i < gc->irq.num_parents; i++) //normally gic irq startup here
				irq_set_chained_handler_and_data(gc->irq.parents[i], gc->irq.parent_handler, data);
					// irq = gc->irq.parents[i]; handle = gc->irq.parent_handler;
					desc = irq_get_desc_buslock(irq, &flags, 0);
					desc->irq_common_data.handler_data = data;
					__irq_do_set_handler(desc, handle, 1, NULL); 
						desc->handle_irq = handle;
						desc->name = name; 
						if (handle != handle_bad_irq && is_chained) {
							...
							irq_activate_and_startup(desc, IRQ_RESEND);
								irq_startup(desc, resend, IRQ_START_FORCE); //affinity here
									__irq_startup(desc);
										if (d->chip->irq_startup) {
											ret = d->chip->irq_startup(d);
											irq_state_clr_disabled(desc);
											irq_state_clr_masked(desc);
										} else {
											irq_enable(desc); 
										}
										irq_state_set_started(desc);
						}
		gpiochip_set_irq_hooks(gc);
		gpiochip_irqchip_add_allocated_domain(gc, domain, false);
			gc->to_irq = gpiochip_to_irq;
			gc->irq.domain = domain;
			gc->irq.initialized = true;
		acpi_gpiochip_request_interrupts(gc); // acpi irq event
	gpiochip_setup_dev(gdev); //if (gpiolib_initialized) //sysfs



gpiolib_dev_init(void)  //core_initcall(gpiolib_dev_init);
	bus_register(&gpio_bus_type);
	driver_register(&gpio_stub_drv); 
	alloc_chrdev_region(&gpio_devt, 0, GPIO_DEV_MAX, GPIOCHIP_NAME);
	gpiolib_initialized = true;
	gpiochip_setup_devs();
		list_for_each_entry(gdev, &gpio_devices, list)
			gpiochip_setup_dev(gdev);
				gcdev_register(gdev, gpio_devt);
				gpiochip_sysfs_register(gdev);



//set simple handler 
static const struct irq_domain_ops gpiochip_domain_ops = {
	.map→   = gpiochip_irq_map,
		irq_set_chip_data(irq, gc);
		irq_set_chip_and_handler(irq, gc->irq.chip, gc->irq.handler); 
		if (gc->irq.threaded) irq_set_nested_thread(irq, 1);
		irq_set_noprobe(irq);
		irq_set_parent(irq, (gc->irq.num_parents == 1) ? gc->irq.parents[0] : gc->irq.map[hwirq]);
	.unmap→ = gpiochip_irq_unmap,
	.xlate→ = irq_domain_xlate_twocell,
}
//or set hierarchy handler
ops->alloc = gpiochip_hierarchy_irq_domain_alloc;
	gc->irq.child_irq_domain_ops.translate(d, fwspec, &hwirq, &type);
	girq->child_to_parent_hwirq(gc, hwirq, type, &parent_hwirq, &parent_type);
	irq_domain_set_info(d, irq, hwirq, gc->irq.chip, gc, girq->handler, NULL, NULL);
	irq_set_probe(irq);
	girq->populate_parent_alloc_arg(gc, &gpio_parent_fwspec, parent_hwirq, parent_type);
	irq_domain_alloc_irqs_parent(d, irq, 1, &gpio_parent_fwspec);


//girq->handler = test_handle;
[    1.601392] [pid:0,cpu0,in irq]Call trace:                                  
[    1.601395] [pid:0,cpu0,in irq] dump_backtrace+0x98/0x118                   
[    1.601405] [pid:0,cpu0,in irq] show_stack+0x18/0x24                        affinity:
[    1.601409] [pid:0,cpu0,in irq] dump_stack_lvl+0x48/0x60                    
[    1.601417] [pid:0,cpu0,in irq] dump_stack+0x18/0x24                        
[    1.601421] [pid:0,cpu0,in irq] test_handle+0x34/0x54                       
[    1.601426] [pid:0,cpu0,in irq] handle_irq_desc+0x40/0x58                   
[    1.601432] [pid:0,cpu0,in irq] generic_handle_domain_irq+0x30/0x84         
[    1.601436] [pid:0,cpu0,in irq] cdns_gpio_irq_handler+0xa0/0x138            
[    1.601439] [pid:0,cpu0,in irq] handle_irq_desc+0x40/0x58                   -------------------- irq
[    1.601443] [pid:0,cpu0,in irq] generic_handle_domain_irq+0x30/0x84         
[    1.601446] [pid:0,cpu0,in irq] gic_handle_irq+0x50/0x138                   
[    1.601448] [pid:0,cpu0,in irq] call_on_irq_stack+0x24/0x30                 
[    1.601451] [pid:0,cpu0,in irq] do_interrupt_handler+0x80/0x84              
[    1.601455] [pid:0,cpu0,in irq] el1_interrupt+0x34/0x68                     
[    1.601461] [pid:0,cpu0,in irq] el1h_64_irq_handler+0x18/0x24               
[    1.601463] [pid:0,cpu0,in irq] el1h_64_irq+0x64/0x68                       
[    1.601465] [pid:0,cpu0,in irq] cpuidle_enter_state+0xc0/0x4e4              
[    1.601468] [pid:0,cpu0,in irq] cpuidle_enter+0x38/0x50                     
[    1.601473] [pid:0,cpu0,in irq] do_idle+0x1f4/0x264                         
[    1.601477] [pid:0,cpu0,in irq] cpu_startup_entry+0x34/0x3c                 
[    1.601479] [pid:0,cpu0,in irq] kernel_init+0x0/0x1e8                       
[    1.601481] [pid:0,cpu0,in irq] arch_post_acpi_subsys_init+0x0/0x8          
[    1.601486] [pid:0,cpu0,in irq] start_kernel+0x52c/0x684                    
[    1.601489] [pid:0,cpu0,in irq] __primary_switched+0xbc/0xc4                

affinity:






-------------------- irq






