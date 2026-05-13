/* power domain */


platform_probe()
	of_clk_set_defaults(_dev->of_node, false)
	dev_pm_domain_attach(_dev, true)
	if (drv->probe) {
		drv->probe(dev);
	}