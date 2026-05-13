usb

/* usb init */
static int __init usb_init(void)
static void __exit usb_exit(void)
subsys_initcall(usb_init);

int usb_init(void)
	usb_init_pool_max();
	usb_debugfs_init(); -> /sys/kernel/debug/usb/devices
	usb_acpi_register();
	bus_register(&usb_bus_type);
		driver_register(&new_driver->driver);// probe = usb_probe_interface
	bus_register_notifier(&usb_bus_type, &usb_bus_nb);
	usb_major_init();
	class_register(&usbmisc_class);
	usb_register(&usbfs_driver);
	usb_devio_init();
	usb_hub_init();
		hub_wq = alloc_workqueue("usb_hub_wq", WQ_FREEZABLE, 0);
	usb_register_device_driver(&usb_generic_driver, THIS_MODULE);
		driver_register(&usb_generic_driver->driver); // probe = usb_probe_device

usb_alloc_dev() will register dev.bus_type = usb_bus_type	


    static inline int is_usb_device(const struct device *dev)
    {
        return dev->type == &usb_device_type;
    }

    static inline int is_usb_interface(const struct device *dev)
    {
        return dev->type == &usb_if_device_type;
    }

    static inline int is_usb_endpoint(const struct device *dev)
    {
    return dev->type == &usb_ep_device_type;
    }

    static inline int is_usb_port(const struct device *dev)
    {
        return dev->type == &usb_port_device_type;
    }

/* init */
#ifdef CONFIG_ACPI
static const struct acpi_device_id usb_xhci_acpi_match[] = {
	/* XHCI-compliant USB Controller */
	{ "PNP0D10", },
	{ "PNP0D15", },
	{ }
};
MODULE_DEVICE_TABLE(acpi, usb_xhci_acpi_match);
#endif

static struct platform_driver usb_generic_xhci_driver = {
	.probe	= xhci_generic_plat_probe,
	.remove = xhci_plat_remove,
	.shutdown = usb_hcd_platform_shutdown,
	.driver	= {
		.name = "xhci-hcd",
		.pm = &xhci_plat_pm_ops,
		.of_match_table = of_match_ptr(usb_xhci_of_match),
		.acpi_match_table = ACPI_PTR(usb_xhci_acpi_match),
	},
};
MODULE_ALIAS("platform:xhci-hcd");


static int xhci_generic_plat_probe(struct platform_device *pdev)
	priv_match = dev_get_platdata(&pdev->dev); 
	xhci_plat_probe(pdev, sysdev, priv_match);

int xhci_plat_probe(struct platform_device *pdev, struct device *sysdev, const struct xhci_plat_priv *priv_match)
	driver = &xhci_plat_hc_driver;
	irq = platform_get_irq(pdev, 0);
	hcd = __usb_create_hcd(driver, sysdev, &pdev->dev, dev_name(&pdev->dev), NULL);
	hcd->regs = devm_platform_get_and_ioremap_resource(pdev, 0, &res);
	xhci = hcd_to_xhci(hcd);
	xhci->main_hcd = hcd;
	usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (!xhci_has_one_roothub(xhci))
		xhci->shared_hcd = __usb_create_hcd(driver, sysdev, &pdev->dev, dev_name(&pdev->dev), hcd);
	if (xhci->shared_hcd)
		usb_add_hcd(xhci->shared_hcd, irq, IRQF_SHARED);
	
int usb_add_hcd(struct usb_hcd *hcd, unsigned int irqnum, unsigned long irqflags)
	hcd_buffer_create(hcd);
	usb_register_bus(&hcd->self);
	rhdev = usb_alloc_dev(NULL, &hcd->self, 0);
	hcd->self.root_hub = rhdev;
	if (usb_hcd_is_primary_hcd(hcd) && irqnum)
		usb_hcd_request_irqs(hcd, irqnum, irqflags); 
	hcd->state = HC_STATE_RUNNING; 
	retval = hcd->driver->start(hcd);
	if (!usb_hcd_is_primary_hcd(hcd) && shared_hcd && HCD_DEFER_RH_REGISTER(shared_hcd))
		register_root_hub(hcd->shared_hcd);
	if (!HCD_DEFER_RH_REGISTER(hcd))
		register_root_hub(hcd);
	
static int register_root_hub(struct usb_hcd *hcd)
	struct usb_device *usb_dev = hcd->self.root_hub;
	usb_dev->devnum = devnum;
	usb_dev->bus->devnum_next = devnum + 1;
	set_bit (devnum, usb_dev->bus->devmap.devicemap);
	usb_set_device_state(usb_dev, USB_STATE_ADDRESS);
	descr = usb_get_device_descriptor(usb_dev);        /**/
	usb_dev->descriptor = *descr;
	if (le16_to_cpu(usb_dev->descriptor.bcdUSB) >= 0x0201)
		usb_get_bos_descriptor(usb_dev);
	usb_new_device (usb_dev);

struct usb_device_descriptor *usb_get_device_descriptor(struct usb_device *udev)
	usb_get_descriptor(udev, USB_DT_DEVICE, 0, desc, sizeof(*desc));
		for (i = 0; i < 3; ++i)
			usb_control_msg(dev, usb_rcvctrlpipe(dev, 0), USB_REQ_GET_DESCRIPTOR, USB_DIR_IN, (type << 8) + index, 0, buf, size, USB_CTRL_GET_TIMEOUT);
				usb_internal_control_msg(dev, pipe, dr, data, size, timeout);
					urb = usb_alloc_urb(0, GFP_NOIO);
					usb_fill_control_urb(urb, usb_dev, pipe, (unsigned char *)cmd, data, len, usb_api_blocking_completion, NULL);
					usb_start_wait_urb(urb, timeout, &length);
						usb_submit_urb(urb, GFP_NOIO);
							urb->ep = usb_pipe_endpoint(urb->dev, urb->pipe);
							usb_hcd_submit_urb(urb, mem_flags);
								hcd = bus_to_hcd(urb->dev->bus)
								if (is_root_hub(urb->dev))
									rh_urb_enqueue(hcd, urb);
										if (usb_endpoint_xfer_int(&urb->ep->desc))
											rh_queue_status (hcd, urb);
												usb_hcd_link_urb_to_ep(hcd, urb);
													list_add_tail(&urb->urb_list, &urb->ep->urb_list); 
												mod_timer(&hcd->rh_timer, jiffies);/mod_timer(&hcd->rh_timer, (jiffies/(HZ/4) + 1) * (HZ/4));
										if (usb_endpoint_xfer_control(&urb->ep->desc))
											rh_call_control(hcd, urb);
												*ubuf = urb->transfer_buffer;
												usb_hcd_link_urb_to_ep(hcd, urb);
													list_add_tail(&urb->urb_list, &urb->ep->urb_list); 
												memcpy (ubuf, bufp, len);
								else
									map_urb_for_dma(hcd, urb, mem_flags);
									hcd->driver->urb_enqueue(hcd, urb, mem_flags);
						wait_for_completion_timeout(&ctx.done, expire);

int usb_new_device(struct usb_device *udev)
	usb_enumerate_device(udev);
		if (udev->config == NULL)
			usb_get_configuration(udev);
		udev->product = usb_cache_string(udev, udev->descriptor.iProduct);
		udev->manufacturer = usb_cache_string(udev, udev->descriptor.iManufacturer);
		udev->serial = usb_cache_string(udev, udev->descriptor.iSerialNumber);
		usb_enumerate_device_otg(udev); 
		usb_detect_interface_quirks(udev);
	udev->dev.devt = MKDEV(USB_DEVICE_MAJOR, (((udev->bus->busnum-1) * 128) + (udev->devnum-1)));
	announce_device(udev); //just print
	device_add(&udev->dev); 
	usb_create_ep_devs(&udev->dev, &udev->ep0, udev);	
/* init done */	
	
	

usb_hcd_flush_endpoint(struct usb_device *udev, struct usb_host_endpoint *ep)
	
	
/* phy */	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	








































