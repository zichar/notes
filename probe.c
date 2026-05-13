/* probe */

platform_device_register()
	device_initialize()->device_pm_init()->pm_runtime_init()
	platform_device_add()->device_add()->bus_probe_device()->device_initial_probe()
	__device_attach()
		if (dev->parent) pm_runtime_get_sync()
		__device_attach_driver
			driver_probe_device()
				__driver_probe_device(drv, dev);
					pm_runtime_get_suppliers()
					if(dev->parent) pm_runtime_get_sync()
					pm_runtime_barrier()
						pm_runtime_getnoresume()
						__pm_runtime_barrier()
						pm_runtime_put_onidle()
					really_probe()
						device_links_check_suppliers(dev);
						pinctrl_bind_pins(dev);
						if (dev->bus->dma_configure) dev->bus->dma_configure(dev);
						driver_sysfs_add(dev);
						if (dev->pm_domain && dev->pm_domain->activate) dev->pm_domain->activate(dev);
						call_driver_probe(dev, drv);
							if (dev->bus->probe) dev->bus->probe(dev);
							dev_pm_domain_attach()
						device_add_groups(dev, drv->dev_groups);
						if (dev_has_sync_state(dev)) device_create_file(dev, &dev_attr_state_synced);
						pinctrl_init_done(dev);
						if (dev->pm_domain && dev->pm_domain->sync) dev->pm_domain->sync(dev);
						driver_bound(dev);
					pm_request_idle()
					if (dev->parent) pm_runtime_put()
					pm_runtime_put_suppliers()
		if (dev->parent) pm_runtime_put()


platform_driver_register()
	driver_register()
		bus_add_driver()
			__driver_attach()
				device_driver_attach()
					driver_probe_device()























