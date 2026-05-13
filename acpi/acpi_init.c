

acpi_init(void)
	acpi_kobj = kobject_create_and_add("acpi", firmware_kobj); //sysfs
	init_prmt(); // PRMT
	acpi_bus_init(); // platform comm address space handler register ACPI_ADR_SPACE_PLATFORM_COMM
		acpi_os_initialize1();
			kacpid_wq = alloc_workqueue("kacpid", 0, 1);
			kacpi_notify_wq = alloc_workqueue("kacpi_notify", 0, 1);
			kacpi_hotplug_wq = alloc_ordered_workqueue("kacpi_hotplug", 0);
			acpi_osi_init(); //_OSI init
				acpi_install_interface_handler(acpi_osi_handler);
				acpi_osi_setup_late(); // add interface into acpi_gbl_supported_interfaces
		acpi_load_tables();
			acpi_ev_install_region_handlers();
			acpi_tb_load_namespace();
			acpi_ns_initialize_objects();	
				acpi_walk_namespace(ACPI_TYPE_ANY, ACPI_ROOT_OBJECT, ACPI_UINT32_MAX, acpi_ns_init_one_object, NULL, &info, NULL); 
		acpi_ec_ecdt_probe(); // ECDT
		acpi_enable_subsystem(ACPI_NO_ACPI_ENABLE);
		acpi_initialize_objects(ACPI_FULL_INITIALIZATION);
		acpi_early_processor_osc();
		acpi_bus_osc_negotiate_platform_control();
		acpi_bus_osc_negotiate_usb_control();
		acpi_install_table_handler(acpi_bus_table_handler, NULL);
		acpi_sysfs_init();
		acpi_early_processor_set_pdc(); 
		acpi_ec_dsdt_probe();
		acpi_sleep_init();
		acpi_bus_init_irq();
		acpi_install_notify_handler(ACPI_ROOT_OBJECT, ACPI_SYSTEM_NOTIFY, &acpi_bus_notify, NULL);
		acpi_root_dir = proc_mkdir(ACPI_BUS_FILE_ROOT, NULL);
		bus_register(&acpi_bus_type);
	pci_mmcfg_late_init();
	acpi_iort_init();
	acpi_viot_early_init();
	acpi_hest_init();
	acpi_ghes_init();
	acpi_scan_init(void)	
		acpi_bus_scan(ACPI_ROOT_OBJECT)
			acpi_walk_namespace(..., acpi_bus_check_add, ) //acpi_bus_check_add 函数创建acpi device
			acpi_bus_attach(device, NULL); // acpi device driver probe
				acpi_default_enumeration(device);
					acpi_create_platform_device(device, NULL);
						platform_device_register_full(&pdevinfo); //创建platform device 
	acpi_ec_init();
	acpi_debugfs_init();
	acpi_sleep_proc_init();
	acpi_wakeup_device_init();
	acpi_debugger_init();
	acpi_setup_sb_notify_handler();
	acpi_viot_init();
	acpi_agdi_init();

acpi_bus_check_add(acpi_handle handle, bool check_dep, struct acpi_device **adev_p)
	switch (acpi_type) type = ACPI_BUS_TYPE_xxx // type
	acpi_add_single_object(&device, handle, type, !check_dep);
		device = kzalloc(sizeof(struct acpi_device), GFP_KERNEL);
		acpi_init_device_object(device, handle, type, acpi_device_release); //**//
			fwnode_init(&device->fwnode, &acpi_device_fwnode_ops);
			acpi_init_properties(device);
				acpi_evaluate_object_typed(adev->handle, "_DSD", NULL, &buf, ACPI_TYPE_PACKAGE);
				acpi_extract_properties(adev->handle, buf.pointer, &adev->data)
				acpi_enumerate_nondev_subnodes(adev->handle, buf.pointer,
									&adev->data, acpi_fwnode_handle(adev)) 
					// 1. create and binding fwnode tree 
					// 2. evaluate acpi obj (method)
				acpi_tie_nondev_subnodes(&adev->data)
			device_initialize(&device->dev);
			acpi_init_coherency(device); // _CCA	
		acpi_bus_get_power_flags(device); //_PS0, _PR0, _PSC, _IRC, _DSW, _DSC power related ...
		acpi_bus_get_wakeup_device_flags(device); 
		result = acpi_tie_acpi_dev(device);
		if (!result) result = acpi_device_add(device); //dev add, name, files
		acpi_device_add_finalize(device);
	acpi_scan_init_hotplug(device);
	
	
acpi_enumerate_nondev_subnodes(adev->handle/scope, buf.pointer, &adev->data, acpi_fwnode_handle(adev))
	for (i = 0; i < desc->package.count; i += 2) { 
		acpi_add_nondev_subnodes(scope, links, &data->subnodes, parent); 
			/* ACPI_TYPE_STRING/ ACPI_TYPE_LOCAL_REFERENCE / ACPI_TYPE_PACKAGE */
			acpi_nondev_subnode_data_ok(handle, link, list, parent);
				acpi_evaluate_object_typed(handle, NULL, NULL, &buf, ACPI_TYPE_PACKAGE); //evaluate (method)
				acpi_nondev_subnode_extract(buf.pointer, handle, link, list, parent))
					dn = kzalloc(sizeof(struct acpi_data_node *), GFP_KERNEL);
					dn->name = link->package.elements[0].string.pointer;
					fwnode_init(&dn->fwnode, &acpi_data_fwnode_ops); //sub fwnode
					dn->parent = parent;
					acpi_extract_properties(handle, desc, &dn->data);
					acpi_enumerate_nondev_subnodes(scope, desc, &dn->data, &dn->fwnode)) // circle
	}

acpi_status
acpi_evaluate_object_typed(acpi_handle handle,
							acpi_string pathname,
							struct acpi_object_list *external_params,
							struct acpi_buffer *return_buffer,
							acpi_object_type return_type)
	acpi_evaluate_object(target_handle, NULL, external_params, return_buffer);
		info = ACPI_ALLOCATE_ZEROED(sizeof(struct acpi_evaluate_info));
		info->prefix_node = acpi_ns_validate_handle(handle); 
		info->param_count = (u16)external_params->count;
		info->parameters = ACPI_ALLOCATE_ZEROED(((acpi_size)info->param_count + 1) * sizeof(void *));
		for (i = 0; i < info->param_count; i++) {
			acpi_ut_copy_eobject_to_iobject(&external_params->pointer[i], &info->parameters[i]);
		}
		acpi_ns_evaluate(info); /**/
			info->return_object = NULL; 
			info->node_flags = info->node->flags;
			info->obj_desc = acpi_ns_get_attached_object(info->node);
			info->predefined = acpi_ut_match_predefined_method(info->node->name.ascii);
			info->full_pathname = acpi_ns_get_normalized_pathname(info->node, TRUE);
			switch (acpi_ns_get_type(info->node)) {
				...
				case ACPI_TYPE_METHOD:
					acpi_ps_execute_method(info);
						acpi_ds_begin_method_execution(info->node, info->obj_desc, NULL);
						acpi_ps_update_parameter_list(info, REF_INCREMENT);
						acpi_ps_create_scope_op(info->obj_desc->method.aml_start);
						walk_state = acpi_ds_create_walk_state(info->obj_desc->method.owner_id, NULL, NULL, NULL); ]
						acpi_ds_init_aml_walk(walk_state, op, info->node, 
												info->obj_desc->method.aml_start,
												info->obj_desc->method.aml_length, info,
												info->pass_number);
						acpi_ps_parse_aml(walk_state);
							thread = acpi_ut_create_thread_state(); 
							while (walk_state) {
							}
					break;
				default: // just get the current object value
					info->return_object = ACPI_CAST_PTR(union acpi_operand_object, info->node);
					break;
			}
			
		acpi_ns_resolve_references(info);
		acpi_ut_get_object_size(info->return_object, &buffer_space_needed);
		acpi_ut_copy_iobject_to_eobject(info->return_object, return_buffer);


static bool acpi_extract_properties(acpi_handle scope, union acpi_object *desc, struct acpi_device_data *data)
	for (i = 0; i < desc->package.count; i += 2) {
		guid = &desc->package.elements[i];
		properties = &desc->package.elements[i + 1];
		- acpi_data_add_buffer_props(scope, data, properties);
		- acpi_data_add_props(data, (const guid_t *)guid->buffer.pointer, properties);
	}
