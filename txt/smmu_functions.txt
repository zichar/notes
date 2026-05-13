
arm_smmu_device_probe(struct platform_device *pdev)
	iommu_device_register  //platform_bus_type->iommu_ops = &arm_smmu_ops;
		bus_iommu_probe
			bus_for_each_dev(bus, NULL, &group_list, probe_iommu_group);
				__iommu_probe_device(dev, group_list);


bind_store(struct device_driver *drv, const char *buf,
			  size_t count)
	device_driver_attach(struct device_driver *drv, struct device *dev)
	
	device_attach(struct device *dev)
		__device_attach(struct device *dev, bool allow_async)
			bus_for_each_drv(dev->bus, NULL, &data,
							__device_attach_driver);
				__device_attach_driver(struct device_driver *drv, void *_data)
					driver_match_device(drv, dev);
					driver_probe_device(struct device_driver *drv, struct device *dev)
						__driver_probe_device(struct device_driver *drv, struct device *dev)
							really_probe(struct device *dev, struct device_driver *drv)

really_probe(struct device *dev, struct device_driver *drv)
	bus_type platform_bus_type->dma_configure
		platform_dma_configure(struct device *dev)
			of_dma_configure(struct device *dev,
							   struct device_node *np,
							   bool force_dma)
				of_dma_configure_id(struct device *dev, struct device_node *np,
							bool force_dma, const u32 *id)			
					of_iommu_configure(struct device *dev,
										   struct device_node *master_np,
										   const u32 *id)
						iommu_probe_device(dev)
							__iommu_probe_device(dev, NULL);
								struct iommu_ops *ops = dev->bus->iommu_ops; //arm_smmu_ops
								iommu_dev = ops->probe_device(dev);//arm_smmu_ops->probe_device
									arm_smmu_probe_device(struct device *dev)
										smmu = arm_smmu_get_by_fwnode(fwspec->iommu_fwnode);
										master = kzalloc(sizeof(*master), GFP_KERNEL);
										master->dev = dev;
										master->smmu = smmu;
										dev_iommu_priv_set(dev, master);
										arm_smmu_insert_master(smmu, master);
										arm_smmu_enable_pasid(master);
										return &smmu->iommu;
								dev->iommu->iommu_dev = iommu_dev;
								group = iommu_group_get_for_dev(dev);
							__iommu_attach_device(group->default_domain, dev); 	// if (group->default_domain && !group->owner)
								domain->ops->attach_dev(domain, dev); //(domain->ops = bus->iommu_ops->default_domain_ops;)			
								
iommu_device_register(struct iommu_device *iommu,
			  const struct iommu_ops *ops, struct device *hwdev)
	iommu_buses[i]->iommu_ops = ops;

static struct bus_type * const iommu_buses[] = {
	&platform_bus_type,
#ifdef CONFIG_PCI
	&pci_bus_type,
#endif
#ifdef CONFIG_ARM_AMBA
	&amba_bustype,
#endif
#ifdef CONFIG_FSL_MC_BUS
	&fsl_mc_bus_type,
#endif
#ifdef CONFIG_TEGRA_HOST1X_CONTEXT_BUS
	&host1x_context_device_bus_type,
#endif
#ifdef CONFIG_CDX_BUS
	&cdx_bus_type,
#endif
};

iommu_device_register(struct iommu_device *iommu,
			  const struct iommu_ops *ops, struct device *hwdev)
	bus_iommu_probe(const struct bus_type *bus)
		probe_alloc_default_domain(const struct bus_type *bus,
							   struct iommu_group *group)
			iommu_group_alloc_default_domain(const struct bus_type *bus,
								struct iommu_group *group,
								unsigned int type)

iommu_group_alloc_default_domain(const struct bus_type *bus,
					    struct iommu_group *group,
					    unsigned int type)
/iommu_domain_alloc(const struct bus_type *bus)
	__iommu_domain_alloc(const struct bus_type *bus,
							 unsigned type)
		domain = bus->iommu_ops->domain_alloc(type);
		domain->pgsize_bitmap = bus->iommu_ops->pgsize_bitmap;
		domain->ops = bus->iommu_ops->default_domain_ops;

iommu_group_do_attach_device(struct device *dev, void *data) //...
	__iommu_attach_device(struct iommu_domain *domain,
					 struct device *dev)	
		domain->ops->attach_dev(domain, dev); //(domain->ops = bus->iommu_ops->default_domain_ops;)
			arm_smmu_ops.default_domain_ops->arm_smmu_attach_dev
				arm_smmu_attach_dev(struct iommu_domain *domain, struct device *dev)
					arm_smmu_domain_finalise(struct iommu_domain *domain,
										struct arm_smmu_master *master)
						pgtbl_ops = alloc_io_pgtable_ops(fmt, &pgtbl_cfg, smmu_domain);
						finalise_stage_fn(smmu_domain, master, &pgtbl_cfg)
						smmu_domain->pgtbl_ops = pgtbl_ops;
				

iommu_probe_device(struct device *dev)
/iommu_group_add_device(struct iommu_group *group, struct device *dev)
/__iommu_group_dma_first_attach(struct iommu_group *group)
	iommu_group_do_dma_first_attach(struct device *dev, void *data)
		__iommu_attach_device(struct iommu_domain *domain,
					 struct device *dev)

iommu_deferred_attach(struct device *dev, struct iommu_domain *domain)
	__iommu_attach_device(struct iommu_domain *domain,
					 struct device *dev)

iommu_attach_device(struct iommu_domain *domain, struct device *dev)//xx
/iommu_attach_group(struct iommu_domain *domain, struct iommu_group *group)//xx
	__iommu_attach_group(struct iommu_domain *domain,
					struct iommu_group *group)
		__iommu_group_for_each_dev(group, domain,
						 iommu_group_do_attach_device);
						 
						 
						 



 arm_smmu_attach_dev(struct iommu_domain *domain, struct device *dev)
	arm_smmu_domain_finalise(domain, master);