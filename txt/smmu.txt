#iommu device init flow:

platform_dma_configure
	of_dma_configure_id
		of_iommu_configure
			dev_iommu_fwspec_get(dev);
			of_iommu_configure_device(master_np, dev, id); (not pcie)
			iommu_probe_device();
			{
				__iommu_probe_device
					iommu_init_device(dev, ops);
						iommu_dev = ops->probe_device(dev); // ops->probe_device() 					=> arm_smmu_probe_device()
					gdev = iommu_group_alloc_device(group, dev);
					if (group->default_domain)
						iommu_create_device_direct_mappings(group->default_domain, dev);
					if (group->domain) {
						__iommu_device_set_domain(group, dev, group->domain, 0);
					} else if (!group->default_domain && !group_list) { // <== *** first probe
						iommu_setup_default_domain
							dom = iommu_group_alloc_default_domain(group, req_type);
								__iommu_group_alloc_default_domain();
									__iommu_domain_alloc(bus, req_type);
										bus->iommu_ops->domain_alloc(alloc_type); // 			=> arm_smmu_domain_alloc()
							for_each_group_device(group, gdev)
								iommu_create_device_direct_mappings(dom, gdev->dev);
							group->default_domain = dom;
							if (!group->domain) {
								for_each_group_device(group, gdev)
									__iommu_group_set_domain_internal() {
										__iommu_device_set_domain
											__iommu_attach_device(new_domain, dev); //domain->ops->attach_dev() => arm_smmu_attach_dev()
										group->domain = new_domain;
									}								
							} else {
								__iommu_group_set_domain(group, dom);
									__iommu_group_set_domain_internal(group, new_domain, 0);
							}
							if (direct_failed)
								for_each_group_device(group, gdev) 
									iommu_create_device_direct_mappings(dom, gdev->dev);
					} else if (!group->default_domain) {
						...
					}
				dev->iommu->iommu_dev->ops->probe_finalize() // ops->probe_finalize 		=>arm_smmu_probe_finalize()
			}
	arch_setup_dma_ops(dev, dma_start, size, iommu, coherent);
		
platform_dma_configure
	acpi_dma_configure								
		acpi_dma_configure_id
			acpi_arch_dma_setup(dev); (dma mask and ranges)
			acpi_iommu_configure_id(dev, input_id);
				ops = acpi_iommu_fwspec_ops(dev);
				iort_iommu_configure_id(dev, id_in);
				iommu_probe_device(dev);
					...{}
		arch_setup_dma_ops(dev, 0, U64_MAX, iommu, attr == DEV_DMA_COHERENT);

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



							
iommu_bus_notifier


iommu_create_device_direct_mappings(dom, gdev->dev) {
	iommu_get_resv_regions(dev, &mappings);
		ops->get_resv_regions(dev, list); // 	=>arm_smmu_get_resv_regions()
	list_for_each_entry(entry, &mappings, list) { 
		for (addr = start; addr <= end; addr += pg_size) {
			iommu_map(domain, ...);
		}
	}
}

