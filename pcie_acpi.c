


== ACPI ==

subsys_initcall(acpi_init(void))
	acpi_pci_root_init();
		acpi_scan_add_handler_with_hotplug(&pci_root_handler, "pci_root");
	acpi_pci_link_init();
		acpi_scan_add_handler(&pci_link_handler);

static struct acpi_scan_handler pci_root_handler = {
	.ids = root_device_ids,
	.attach = acpi_pci_root_add,
	.detach = acpi_pci_root_remove,
	.hotplug = {
		.enabled = true,
		.scan_dependent = acpi_pci_root_scan_dependent,
	},
};
static struct acpi_scan_handler pci_link_handler = {
	.ids = link_device_ids,
	.attach = acpi_pci_link_add,
	.detach = acpi_pci_link_remove,
};

-- 
acpi_pci_link_add(struct acpi_device *device, const struct acpi_device_id *not_used)
	link = kzalloc(sizeof(struct acpi_pci_link), GFP_KERNEL);
	acpi_pci_link_get_possible(link);
	acpi_pci_link_get_current(link);
	list_add_tail(&link->list, &acpi_link_list);
	acpi_evaluate_object(handle, "_DIS", NULL, NULL);

acpi_pci_root_add(struct acpi_device *device, const struct acpi_device_id *not_used)
	pci_acpi_scan_root(struct acpi_pci_root *root)
		ri = kzalloc(sizeof(*ri), GFP_KERNEL);
		root_ops = kzalloc(sizeof(*root_ops), GFP_KERNEL);
		ri->cfg = pci_acpi_setup_ecam_mapping(root);
			pci_mcfg_lookup(root, &cfgres, &ecam_ops);
				list_for_each_entry(e, &pci_mcfg_list, list)
					if(...)root->mcfg_addr = e->addr;
			adev = acpi_resource_consumer(&cfgres);
			cfg = pci_ecam_create(dev, &cfgres, bus_res, ecam_ops);
		bus = acpi_pci_root_create(root, root_ops, &ri->common, ri->cfg);
			pci_acpi_root_add_resources(info); //pci ranges
			bus = pci_create_root_bus(NULL, busnum, ops->pci_ops, sysdata, &info->resources);			
				bridge = pci_alloc_host_bridge(0);
				pci_register_host_bridge(bridge);
			pci_scan_child_bus(bus);
		host = pci_find_host_bridge(bus);
		list_for_each_entry(child, &bus->children, node)
			pcie_bus_configure_settings(child);
	pci_bus_add_devices(root->bus);

acpi_init()
	pci_mmcfg_late_init();
		acpi_table_parse(ACPI_SIG_MCFG, pci_mcfg_parse);
			pci_mcfg_parse(struct acpi_table_header *header)
				list_add(&e->list, &pci_mcfg_list);

pci_register_host_bridge(bridge);
	bus = pci_alloc_bus(NULL);
	b = pci_find_bus(pci_domain_nr(bus), bridge->busnr);
	pcibios_root_bridge_prepare(bridge);
	pci_set_bus_of_node(bus);
	pci_set_bus_msi_domain(bus);
	device_register(&bus->dev);
	pcibios_add_bus(bus);
	if (bus->ops->add_bus) bus->ops->add_bus(bus);
	resource_list_for_each_entry_safe(window, n, &resources) {...}
	resource_list_for_each_entry_safe(window, n, &resources) {...}
	list_add_tail(&bus->node, &pci_root_buses);

pci_scan_child_bus(bus);
	pci_scan_child_bus_extend(bus, 0);
		for (devfn = 0; devfn < 256; devfn += 8)
			pci_scan_slot(bus, devfn);
				do {
					dev = pci_scan_single_device(bus, devfn + fn);
						dev = pci_get_slot(bus, devfn);
						dev = pci_scan_device(bus, devfn);
							pci_set_acpi_fwnode(dev);
								ACPI_COMPANION_SET(&dev->dev, acpi_pci_find_companion(&dev->dev));
									//adev = pci_acpi_find_companion_hook(pci_dev);
						pci_device_add(dev, bus);
					fn = next_fn(bus, dev, fn);
				} while (fn >= 0)
		used_buses = pci_iov_bus_range(bus);
		for_each_pci_bridge(dev, bus)
			max = pci_scan_bridge_extend(bus, dev, max, 0, 0);
		for_each_pci_bridge(dev, bus) 
			max = pci_scan_bridge_extend(bus, dev, cmax, buses, 1);				

int pci_scan_bridge_extend(struct pci_bus *bus, struct pci_dev *dev, int max, unsigned int available_buses, int pass)
	pci_read_config_dword(dev, PCI_PRIMARY_BUS, &buses);
	primary = buses & 0xFF;
	secondary = (buses >> 8) & 0xFF;
	subordinate = (buses >> 16) & 0xFF;
	pci_read_config_word(dev, PCI_BRIDGE_CONTROL, &bctl);
	pci_write_config_word(dev, PCI_BRIDGE_CONTROL, bctl & ~PCI_BRIDGE_CTL_MASTER_ABORT);
	pci_enable_crs(dev);
	...
	child = pci_find_bus(pci_domain_nr(bus), secondary);
	if (!child) child = pci_add_new_bus(bus, dev, secondary);
	buses = subordinate - secondary;
	cmax = pci_scan_child_bus_extend(child, buses);
	...
	...

/* ECAM ops */
const struct pci_ecam_ops pci_generic_ecam_ops = {
	.pci_ops	= {
		.add_bus	= pci_ecam_add_bus,
		.remove_bus	= pci_ecam_remove_bus,
		.map_bus	= pci_ecam_map_bus,
		.read		= pci_generic_config_read,
		.write		= pci_generic_config_write,
	}
};
EXPORT_SYMBOL_GPL(pci_generic_ecam_ops);

== DTS ==			
sky1_pcie_probe(struct platform_device *pdev)
	pcie = devm_kzalloc(dev, sizeof(*pcie), GFP_KERNEL);	
	bridge = devm_pci_alloc_host_bridge(dev, priv);
		bridge = pci_alloc_host_bridge(size_t priv)
			bridge = kzalloc(sizeof(*bridge) + priv, GFP_KERNEL);
			pci_init_host_bridge(bridge); //basic init
			bridge->dev.release = pci_release_host_bridge_dev; 
		bridge->dev.parent = dev;
		devm_add_action_or_reset(dev, devm_pci_alloc_host_bridge_release, bridge);
		devm_of_pci_bridge_init(dev, bridge); //ugly here
			bridge->swizzle_irq = pci_common_swizzle;
			bridge->map_irq = of_irq_parse_and_map_pci;
			pci_parse_request_of_pci_ranges(dev, bridge);
				devm_of_pci_get_host_bridge_resources(dev, 0, 0xff, &bridge->windows, &bridge->dma_ranges, &iobase);
					(bus_range, range, dma_range)->pci_add_resouce(res)
				devm_request_pci_bus_resources(dev, &bridge->windows);	
					resource_list_for_each_entry(win, resources) {
						case IORESOURCE_IO: parent = &ioport_resource;
						case IORESOURCE_MEM: parent = &iomem_resource;
						devm_request_resource(dev, parent, res);
					}
				resource_list_for_each_entry_safe(win, tmp, &bridge->windows) {
					case IORESOURCE_IO: devm_pci_remap_iospace(dev, res, iobase);
					case IORESOURCE_MEM: res_valid |= !(res->flags & IORESOURCE_PREFETCH);
				}	
	ret = sky1_pcie_parse_dts(pdev, pcie);
	pcie->cfg = pci_ecam_create(dev, pcie->cfg_res, bus->res, &pci_generic_ecam_ops);
	//bridge->ops = (struct pci_ops *)&pci_generic_ecam_ops.pci_ops;
	bridge->ops = &sky1_pcie_own_ops;
	bridge->child_ops = (struct pci_ops *)&pci_generic_ecam_ops.pci_ops; 	
	rc = pci_host_bridge_priv(bridge);
	ret = cdns_pcie_host_setup(rc);
		cdns_pcie_host_start_link(rc); //pcie->ops->start_link(pcie);
		cdns_pcie_host_init(dev, rc);
		pci_host_probe(bridge);
			pci_scan_root_bus_bridge(bridge);
				pci_register_host_bridge(bridge);
				pci_scan_child_bus(b);  //b = bridge->bus
			bus = bridge->bus;
			pci_bus_add_devices(bus);



===  ===


static int __init pcie_portdrv_init(void)
	pcie_init_services();
		pcie_aer_init();
			pcie_port_service_register(&aerdriver);
		pcie_pme_init();
			pcie_port_service_register(&pcie_pme_driver);
		pcie_dpc_init();
			pcie_port_service_register(&dpcdriver);
		pcie_hp_init();
			pcie_port_service_register(&hpdriver_portdrv);
	pci_register_driver(&pcie_portdriver);
device_initcall(pcie_portdrv_init);


=== MSI ==
void pci_set_msi_domain(struct pci_dev *dev)
	d = pci_dev_msi_domain(dev);
		d = dev_get_msi_domain(&dev->dev);
		d = pci_msi_get_device_domain(dev);
			dom = of_msi_map_get_device_domain(&pdev->dev, rid, DOMAIN_BUS_PCI_MSI);
			dom = iort_get_device_domain(&pdev->dev, rid, DOMAIN_BUS_PCI_MSI);
				iort_dev_find_its_id(dev, id, 0, &its_id); /**/
				handle = iort_find_domain_token(its_id);
				return irq_find_matching_fwnode(handle, bus_token);
	if (!d)
		d = dev_get_msi_domain(&dev->bus->dev);
	dev_set_msi_domain(&dev->dev, d);

void pci_set_bus_msi_domain(struct pci_bus *bus)
	for (b = bus, d = NULL; !d && !pci_is_root_bus(b); b = b->parent)
		if (b->self) d = dev_get_msi_domain(&b->self->dev);
	if (!d) d = pci_host_bridge_msi_domain(b);
	dev_set_msi_domain(&bus->dev, d);

pci_host_probe(bridge);
	pci_scan_root_bus_bridge
		pci_register_host_bridge(bridge);
			pci_set_bus_msi_domain(bus); /* no msi domain */
		pci_scan_child_bus_extend(bus, 0);
			for (devfn = 0; devfn < 256; devfn += 8) pci_scan_slot(bus, devfn);
				pci_scan_single_device(bus, devfn + fn);
					dev = pci_scan_device(bus, devfn); 
					pci_device_add
						pci_set_msi_domain(dev);
			for_each_pci_bridge(dev, bus) pci_scan_bridge_extend(bus, dev, max, 0, 0);
			for_each_pci_bridge(dev, bus) pci_scan_bridge_extend(bus, dev, max, buses, 1);
				pci_alloc_child_bus
					pci_set_bus_msi_domain
				pci_scan_child_bus_extend
					pci_scan_slot
						pci_scan_single_device
							pci_device_add
								pci_set_msi_domain(dev);


=== ACPI COMPANION ===		
	
=== Dev (bridge, bus, pci_dev) ===


=== iommu ===
pci_dma_configure
	if (IS_ENABLED(CONFIG_OF) && bridge->parent && bridge->parent->of_node)
		of_dma_configure(dev, bridge->parent->of_node, true);
			 of_dma_configure_id(dev, np, force_dma, NULL);
			    //handle "dma-ranges" in parent node
					bus_np = (np == dev->of_node) ? __of_get_dma_parent(np) : of_node_get(np); //parent node
					of_dma_get_range(bus_np, &map); //parent node "dma-ranges" and handle offset
					mask = DMA_BIT_MASK(ilog2(end) + 1);
					dev->coherent_dma_mask &= mask;
					*dev->dma_mask &= mask;
					dev->bus_dma_limit = end;
					dev->dma_range_map = map;
				coherent = of_dma_is_coherent(np);
				iommu = of_iommu_configure(dev, np, id);
					fwspec = dev_iommu_fwspec_get(dev); if (fwspec->ops) return fwspec->ops; //already done
					if (dev_is_pci(dev))
						pci_request_acs();
						pci_for_each_dma_alias(to_pci_dev(dev), of_pci_iommu_init, &info); //iort_pci_iommu_init
					else
						of_iommu_configure_device(master_np, dev, id);
							of_iommu_configure_dev(master_np, dev);
								of_parse_phandle_with_args(master_np, "iommus", "#iommu-cells", idx, &iommu_spec))
								of_iommu_xlate(dev, &iommu_spec);
									ops = iommu_ops_from_fwnode(fwnode);
									iommu_fwspec_init(dev, &iommu_spec->np->fwnode, ops);
									ops->of_xlate(dev, iommu_spec);
					iommu_probe_device(dev);
				arch_setup_dma_ops(dev, dma_start, size, iommu, coherent);
	else if (has_acpi_companion(bridge))
		acpi_dma_configure(dev, acpi_get_dma_attr(adev));	
			acpi_dma_configure_id(dev, attr, NULL);
				acpi_arch_dma_setup(dev);
				iommu = acpi_iommu_configure_id(dev, input_id);
					acpi_iommu_fwspec_ops(dev); if (ops) return ops; //already
					iort_iommu_configure_id(dev, id_in);
						if (dev_is_pci(dev))
							node = iort_scan_node(ACPI_IORT_NODE_PCI_ROOT_COMPLEX, iort_match_node_callback, &bus->dev);
							pci_for_each_dma_alias(to_pci_dev(dev), iort_pci_iommu_init, &info);
								iort_pci_iommu_init(pdev, pci_dev_id(pdev), &info);
								if (unlikely(pdev->dma_alias_mask))
									for_each_set_bit(devfn, pdev->dma_alias_mask, MAX_NR_DEVFNS)
										iort_pci_iommu_init(pdev, pci_dev_id(pdev), &info);
								for (bus = pdev->bus; !pci_is_root_bus(bus); bus = bus->parent)
									//find up device call iort_pci_iommu_init
						else
							node = iort_scan_node(ACPI_IORT_NODE_NAMED_COMPONENT, iort_match_node_callback, dev);
							iort_nc_iommu_map(dev, node);
							iort_named_component_init(dev, node);
					iommu_probe_device(dev);
					return acpi_iommu_fwspec_ops(dev);
				arch_setup_dma_ops(dev, 0, U64_MAX, iommu, attr == DEV_DMA_COHERENT);

iort_match_node_callback
	if (node->type == ACPI_IORT_NODE_NAMED_COMPONENT) 
		status = !strcmp(ncomp->device_name, buf.pointer) ? AE_OK : AE_NOT_FOUND; 
	else if (node->type == ACPI_IORT_NODE_PCI_ROOT_COMPLEX)
		status = pci_rc->pci_segment_number == pci_domain_nr(bus) ? AE_OK : AE_NOT_FOUND;
	
iort_pci_iommu_init
	 parent = iort_node_map_id(info->node, alias, &streamid, IORT_IOMMU_TYPE);  //add id offset if any
	 return iort_iommu_xlate(info->dev, parent, streamid);





/* device arch */

(1) get a host bridge first
bridge = pci_alloc_host_bridge(size_t priv); // alloc host bridge here: struct pci_host_bridge
pcie->cfg = pci_ecam_create(dev, pcie->cfg_res, bus->res, &pci_generic_ecam_ops);
pci_host_probe(bridge);
	pci_scan_root_bus_bridge(bridge);
		pci_register_host_bridge(bridge);
			bus = pci_alloc_bus(NULL); // alloc bridge->bus here: struct pci_bus
			bridge->bus = bus;
			bus->sysdata = bridge->sysdata;
			bus->ops = bridge->ops;
			bus->number = bus->busn_res.start = bridge->busnr;
			bus->domain_nr = bridge->domain_nr; or pci_bus_find_domain_nr(bus, parent);
			dev_set_name(&bridge->dev, "pci%04x:%02x", pci_domain_nr(bus), bridge->busnr); //pci_domain_nr(bus) = bus->domain_nr; ?
			pcibios_root_bridge_prepare(bridge);	if (acpi_disabled) return 0; //ACPI
				cfg = bridge->bus->sysdata;
				adev = cfg->parent ? NULL : to_acpi_device(cfg->parent);
				ACPI_COMPANION_SET(&bridge->dev, adev);
				set_dev_node(&bridge->bus->dev, acpi_get_node(acpi_device_handle(adev)));
			device_add(&bridge->dev);
			bus->bridge = get_device(&bridge->dev);
			pci_set_bus_of_node(bus);
			pci_set_bus_msi_domain(bus);
			bus->dev.class = &pcibus_class; 
			bus->dev.parent = bus->bridge; //bus->dev.parent = bridge->dev; bridge->dev.parent = &pdev/adev->dev
			dev_set_name(&bus->dev, "%04x:%02x", pci_domain_nr(bus), bus->number);
			device_register(&bus->dev); 
			pcibios_add_bus(bus); //ACPI
				acpi_pci_add_bus(bus);
					if (acpi_pci_disabled || !bus->bridge || !ACPI_HANDLE(bus->bridge)) return;
					acpi_pci_slot_enumerate(bus);
						acpi_walk_namespace(ACPI_TYPE_DEVICE, ACPI_HANDLE(bus->bridge), 1, register_slot, NULL, bus, NULL);
							register_slot(acpi_handle handle, u32 lvl, void *context, void **rv)
								device = check_slot(handle, &sun); /* device = _ADR; sun = _SUN */
								list_for_each_entry(slot, &slot_list, list) if (pci_slot->bus == pci_bus && pci_slot->number == device) return;
								slot = kmalloc(sizeof(*slot), GFP_KERNEL);
								snprintf(name, sizeof(name), "%llu", sun); 
								slot->pci_slot = pci_create_slot(pci_bus, device, name, NULL);
								list_add(&slot->list, &slot_list);
					acpiphp_enumerate_slots(bus);
						bridge = kzalloc(sizeof(struct acpiphp_bridge), GFP_KERNEL); // alloc hotplug bridge: struct acpiphp_bridge
						bridge->pci_dev = pci_dev_get(bus->self);
						bridge->pci_bus = bus;
						if (pci_is_root_bus(bridge->pci_bus))
							root_context = kzalloc(sizeof(*root_context), GFP_KERNEL);
							root_context->root_bridge = bridge;
							acpi_set_hp_context(adev, &root_context->hp); 
						else
							context = acpiphp_get_context(adev);
							bridge->context = context;
							context->bridge = bridge;
						list_add(&bridge->list, &bridge_list);
						acpi_walk_namespace(ACPI_TYPE_DEVICE, handle, 1, acpiphp_add_context, NULL, bridge, NULL);
							acpiphp_add_context(acpi_handle handle, u32 lvl, void *data, void **rv) 
								...
								acpiphp_register_hotplug_slot(slot, sun);
					if (!pci_is_root_bus(bus)) return;
					acpi_evaluate_dsm(ACPI_HANDLE(bus->bridge), &pci_acpi_dsm_guid, 3,DSM_PCI_POWER_ON_RESET_DELAY, NULL); 
			if (bus->ops->add_bus) bus->ops->add_bus(bus);
			pci_create_legacy_files(bus);
			list_add_tail(&bus->node, &pci_root_buses);
		pci_scan_child_bus(bridge->bus); -> pci_scan_child_bus_extend(bus, 0);
			for (devfn = 0; devfn < 256; devfn += 8)
				pci_scan_slot(bus, devfn);
					do {
						dev = pci_scan_single_device(bus, devfn + fn);
							dev = pci_scan_device(bus, devfn);
								if (!pci_bus_read_dev_vendor_id(bus, devfn, &l, 60*1000)) return NULL;
									pci_bus_generic_read_dev_vendor_id(bus, devfn, l, timeout);
										pci_bus_read_config_dword(bus, devfn, PCI_VENDOR_ID, l);
											bus->ops->read(bus, devfn, pos, len, &data);
								dev = pci_alloc_dev(bus); // alloc pci dev: struct pci_dev
									dev = kzalloc(sizeof(struct pci_dev), GFP_KERNEL);
									dev->dev.type = &pci_dev_type;
									dev->bus = pci_bus_get(bus);
								dev->devfn = devfn;
								dev->vendor = l & 0xffff;
								dev->device = (l >> 16) & 0xffff;
								pci_setup_device(dev);
									pci_set_of_node(dev);
									pci_set_acpi_fwnode(dev);
										ACPI_COMPANION_SET(&dev->dev, acpi_pci_find_companion(&dev->dev)); 
									dev_set_name(&dev->dev, "%04x:%02x:%02x.%d", pci_domain_nr(dev->bus),
											dev->bus->number, PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn));
									class = pci_class(dev); dev->revision = class & 0xff; dev->class = class >> 8;
									if (pci_early_dump) early_dump_pci_device(dev);
									...
							pci_device_add(dev, bus); 
								pci_configure_device(dev);
								...
								pci_init_capabilities(dev);
								list_add_tail(&dev->bus_list, &bus->devices);
								pci_set_msi_domain(dev); 
								dev->match_driver = false;
								device_add(&dev->dev); // pci/pcie device do not probe here
						fn = next_fn(bus, dev, fn);
					} while (fn >= 0);
			used_buses = pci_iov_bus_range(bus);
			max += used_buses;
			for_each_pci_bridge(dev, bus)
				pci_scan_bridge_extend(bus, dev, max, 0, 0);
			for_each_pci_bridge(dev, bus)
				pci_scan_bridge_extend(bus, dev, cmax, buses, 1); 
					pci_read_config_dword(dev, PCI_PRIMARY_BUS, &buses);
					primary = buses & 0xFF;	
					secondary = (buses >> 8) & 0xFF;
					subordinate = (buses >> 16) & 0xFF;
					pci_read_config_word(dev, PCI_BRIDGE_CONTROL, &bctl);
					pci_enable_crs(dev);
					// ACPI bios assign or else ...
					child = pci_add_new_bus(bus, dev:struct pci_dev *, secondary); //if (!pci_find_bus(pci_domain_nr(bus), secondary)
						pci_alloc_child_bus(parent, dev/bridge:struct pci_dev *, busnr); 
							child = pci_alloc_bus(parent); //alloc child bus: struct pci_bus
							child->parent = parent; 
							child->sysdata = parent->sysdata;
							child->bus_flags = parent->bus_flags;
							host = pci_find_host_bridge(parent);
							child->ops = host->child_ops ? : parent->ops;
							child->dev.class = &pcibus_class;
							dev_set_name(&child->dev, "%04x:%02x", pci_domain_nr(child), busnr);
							if (!bridge) child->dev.parent = parent->bridge; goto add_dev;
							child->self = bridge;
							child->bridge = get_device(&bridge->dev);
							child->dev.parent = child->bridge;
							pci_set_bus_of_node(child);
							pci_set_bus_speed(child);
							bridge->subordinate = child;
							//add_dev:
							pci_set_bus_msi_domain(child);
							device_register(&child->dev);
							pcibios_add_bus(child);
								acpi_pci_add_bus(bus);
							if (child->ops->add_bus) child->ops->add_bus(child);
							pci_create_legacy_files(child);
					child->primary = primary;
					pci_bus_insert_busn_res(child, secondary, subordinate); 
					child->bridge_ctl = bctl;
					cmax = pci_scan_child_bus_extend(child, subordinate - secondary);
						... scan child dev
	bus = bridge->bus;
	pci_bus_add_devices(bus);
		list_for_each_entry(dev, &bus->devices, bus_list)
			pci_bus_add_device(dev);
				pcibios_bus_add_device(dev);
				...
				pci_create_sysfs_dev_files(dev);
				pci_proc_attach_device(dev);
				...
				dev->match_driver = !dn || of_device_is_available(dn); 
				device_attach(&dev->dev); // pci/pcie device probe
				pci_dev_assign_added(dev, true); 
		list_for_each_entry(dev, &bus->devices, bus_list)
			pci_bus_add_devices(dev->subordinate)

// simple one
bridge = pci_alloc_host_bridge(size_t priv); // alloc host bridge here: struct pci_host_bridge
pcie->cfg = pci_ecam_create(dev, pcie->cfg_res, bus->res, &pci_generic_ecam_ops);
pci_host_probe(bridge);
	pci_scan_root_bus_bridge(bridge);
		pci_register_host_bridge(bridge);
			bus = pci_alloc_bus(NULL); // alloc bridge->bus here: struct pci_bus
			dev_set_name(&bridge->dev, "pci%04x:%02x", pci_domain_nr(bus), bridge->busnr); //pci_domain_nr(bus) = bus->domain_nr; ?
			device_add(&bridge->dev);
			bus->dev.parent = bus->bridge; //bus->dev.parent = bridge->dev; bridge->dev.parent = &pdev/adev->dev
			dev_set_name(&bus->dev, "%04x:%02x", pci_domain_nr(bus), bus->number);
			device_register(&bus->dev); 
			pcibios_add_bus(bus); //ACPI
			list_add_tail(&bus->node, &pci_root_buses);
		pci_scan_child_bus(bridge->bus); -> pci_scan_child_bus_extend(bus, 0);
			for (devfn = 0; devfn < 256; devfn += 8)
				pci_scan_slot(bus, devfn);
					do {
						dev = pci_scan_single_device(bus, devfn + fn);
							dev = pci_scan_device(bus, devfn);
								dev = pci_alloc_dev(bus); // alloc pci dev: struct pci_dev
									dev = kzalloc(sizeof(struct pci_dev), GFP_KERNEL);
								pci_setup_device(dev);
									dev_set_name(&dev->dev, "%04x:%02x:%02x.%d", pci_domain_nr(dev->bus),
											dev->bus->number, PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn));
							pci_device_add(dev, bus); 
								list_add_tail(&dev->bus_list, &bus->devices);
								pci_set_msi_domain(dev); 
								dev->match_driver = false;
								device_add(&dev->dev); // pci/pcie device do not probe here
						fn = next_fn(bus, dev, fn);
					} while (fn >= 0);
			for_each_pci_bridge(dev, bus)
				pci_scan_bridge_extend(bus, dev, max, 0, 0);
			for_each_pci_bridge(dev, bus)
				pci_scan_bridge_extend(bus, dev, cmax, buses, 1); 
					// ACPI bios assign or else ...
					child = pci_add_new_bus(bus, dev:struct pci_dev *, secondary); //if (!pci_find_bus(pci_domain_nr(bus), secondary)
						pci_alloc_child_bus(parent, dev/bridge:struct pci_dev *, busnr); 
							child = pci_alloc_bus(parent); //alloc child bus: struct pci_bus
							dev_set_name(&child->dev, "%04x:%02x", pci_domain_nr(child), busnr);
							pci_set_bus_msi_domain(child);
							device_register(&child->dev);
							pcibios_add_bus(child);
								acpi_pci_add_bus(bus);
					child->primary = primary;
					pci_bus_insert_busn_res(child, secondary, subordinate); 
					child->bridge_ctl = bctl;
					cmax = pci_scan_child_bus_extend(child, subordinate - secondary);
						... scan child dev
	bus = bridge->bus;
	pci_bus_add_devices(bus);
		list_for_each_entry(dev, &bus->devices, bus_list)
			pci_bus_add_device(dev);
				dev->match_driver = !dn || of_device_is_available(dn); 
				device_attach(&dev->dev); // pci/pcie device probe
		list_for_each_entry(dev, &bus->devices, bus_list)
			pci_bus_add_devices(dev->subordinate)




/* pci cmdline control */
static int __init pci_setup(char *str)
{
	while (str) {
		char *k = strchr(str, ',');
		if (k)
			*k++ = 0;
		if (*str && (str = pcibios_setup(str)) && *str) {
			if (!strcmp(str, "nomsi")) {
				pci_no_msi();
			} else if (!strncmp(str, "noats", 5)) {
				pr_info("PCIe: ATS is disabled\n");
				pcie_ats_disabled = true;
			} else if (!strcmp(str, "noaer")) {
				pci_no_aer();
			} else if (!strcmp(str, "earlydump")) {
				pci_early_dump = true;
			} else if (!strncmp(str, "realloc=", 8)) {
				pci_realloc_get_opt(str + 8);
			} else if (!strncmp(str, "realloc", 7)) {
				pci_realloc_get_opt("on");
			} else if (!strcmp(str, "nodomains")) {
				pci_no_domains();
			} else if (!strncmp(str, "noari", 5)) {
				pcie_ari_disabled = true;
			} else if (!strncmp(str, "notph", 5)) {
				pci_no_tph();
			} else if (!strncmp(str, "cbiosize=", 9)) {
				pci_cardbus_io_size = memparse(str + 9, &str);
			} else if (!strncmp(str, "cbmemsize=", 10)) {
				pci_cardbus_mem_size = memparse(str + 10, &str);
			} else if (!strncmp(str, "resource_alignment=", 19)) {
				resource_alignment_param = str + 19;
			} else if (!strncmp(str, "ecrc=", 5)) {
				pcie_ecrc_get_policy(str + 5);
			} else if (!strncmp(str, "hpiosize=", 9)) {
				pci_hotplug_io_size = memparse(str + 9, &str);
			} else if (!strncmp(str, "hpmmiosize=", 11)) {
				pci_hotplug_mmio_size = memparse(str + 11, &str);
			} else if (!strncmp(str, "hpmmioprefsize=", 15)) {
				pci_hotplug_mmio_pref_size = memparse(str + 15, &str);
			} else if (!strncmp(str, "hpmemsize=", 10)) {
				pci_hotplug_mmio_size = memparse(str + 10, &str);
				pci_hotplug_mmio_pref_size = pci_hotplug_mmio_size;
			} else if (!strncmp(str, "hpbussize=", 10)) {
				pci_hotplug_bus_size =
					simple_strtoul(str + 10, &str, 0);
				if (pci_hotplug_bus_size > 0xff)
					pci_hotplug_bus_size = DEFAULT_HOTPLUG_BUS_SIZE;
			} else if (!strncmp(str, "pcie_bus_tune_off", 17)) {
				pcie_bus_config = PCIE_BUS_TUNE_OFF;
			} else if (!strncmp(str, "pcie_bus_safe", 13)) {
				pcie_bus_config = PCIE_BUS_SAFE;
			} else if (!strncmp(str, "pcie_bus_perf", 13)) {
				pcie_bus_config = PCIE_BUS_PERFORMANCE;
			} else if (!strncmp(str, "pcie_bus_peer2peer", 18)) {
				pcie_bus_config = PCIE_BUS_PEER2PEER;
			} else if (!strncmp(str, "pcie_scan_all", 13)) {
				pci_add_flags(PCI_SCAN_ALL_PCIE_DEVS);
			} else if (!strncmp(str, "disable_acs_redir=", 18)) {
				disable_acs_redir_param = str + 18;
			} else if (!strncmp(str, "config_acs=", 11)) {
				config_acs_param = str + 11;
			} else {
				pr_err("PCI: Unknown option `%s'\n", str);
			}
		}
		str = k;
	}
	return 0;
}
early_param("pci", pci_setup);


























	