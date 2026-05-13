




gic_of_init()
	gic_enable_of_quirks(node, gic_quirks, &gic_data);
	gic_init_bases(dist_phys_base, dist_base, rdist_regs, nr_redist_regions, redist_stride, &node->fwnode);
		gic_data.fwnode = handle;
		gic_data.dist_phys_base = dist_phys_base;
		gic_data.dist_base = dist_base;
		gic_data.redist_regions = rdist_regs;
		gic_data.nr_redist_regions = nr_redist_regions;
		gic_data.redist_stride = redist_stride;
		gic_data.rdists.gicd_typer = readl_relaxed(gic_data.dist_base + GICD_TYPER);
		//
		gic_enable_quirks(readl_relaxed(gic_data.dist_base + GICD_IIDR), gic_quirks, &gic_data);
		gic_data.domain = irq_domain_create_tree(handle, &gic_irq_domain_ops, &gic_data);
		gic_data.rdists.rdist = alloc_percpu(typeof(*gic_data.rdists.rdist));
		//
		set_handle_irq(gic_handle_irq);
		//
		gic_update_rdist_properties();
			gic_iterate_rdists(__gic_update_rdist_properties);
				gic_data.rdists.has... =
				gic_data.ppi_nr = min(GICR_TYPER_NR_PPIS(typer), gic_data.ppi_nr);
		gic_dist_init();
			GICD_IGROUPR, GICD_ICENABLERnE, GICD_ICACTIVERnE, GICD_IGROUPRnE, GICD_ICFGRnE, GICD_IPRIORITYRnE, GICD_CTLR, GICD_IROUTER, GICD_IROUTERnE
		gic_cpu_init();
			gic_populate_rdist()
				gic_iterate_rdists(__gic_populate_rdist)
					if ((typer >> 32) == aff) gic_data_rdist()->phys_base = region->phys_base + offset;
			gic_enable_redist(true);
				writel_relaxed(val &= ~GICR_WAKER_ProcessorSleep, rbase + GICR_WAKER);
			SGI: GICR_IGROUPR0, GIC_DIST_ACTIVE_CLEAR, GIC_DIST_ENABLE_CLEAR, GIC_DIST_PRI
			gic_cpu_sys_reg_init();
				...
		gic_smp_init();
			cpuhp_setup_state_nocalls(CPUHP_AP_IRQ_GIC_STARTING, "irqchip/arm/gicv3:starting", gic_starting_cpu, NULL);
			base_sgi = irq_domain_alloc_irqs(gic_data.domain, 8, NUMA_NO_NODE, &sgi_fwspec);
			set_smp_ipi_range(base_sgi, 8);
				for (i = 0; i < nr_ipi; i++)
					request_percpu_irq(ipi_base + i, ipi_handler, "IPI", &cpu_number);
					ipi_desc[i] = irq_to_desc(ipi_base + i); 
					irq_set_status_flags(ipi_base + i, IRQ_HIDDEN);
			ipi_irq_base = ipi_base;
			ipi_setup(smp_processor_id()); 	
				for (i = 0; i < nr_ipi; i++)
					enable_percpu_irq(ipi_irq_base + i, 0);
		gic_cpu_pm_init();
			cpu_pm_register_notifier(&gic_cpu_pm_notifier_block);
		//
		its_init(handle, &gic_data.rdists, gic_data.domain);
		its_cpu_init();
		its_lpi_memreserve_init();
	gic_populate_ppi_partitions(node); 



static int gic_iterate_rdists(int (*fn)(struct redist_region *, void __iomem *))
	for (i = 0; i < gic_data.nr_redist_regions; i++)
		do {
			fn(gic_data.redist_regions + i, ptr);
			ptr += SZ_64K * 2;/ptr += SZ_64K * 2;/ptr += gic_data.redist_stride;
		} while (!(typer & GICR_TYPER_LAST));
