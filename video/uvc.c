#uvc


static struct usb_driver uvc_driver = {
	.name		= "uvcvideo",
	.probe		= uvc_probe,
	.disconnect	= uvc_disconnect,
	.suspend	= uvc_suspend,
	.resume		= uvc_resume,
	.reset_resume	= uvc_reset_resume,
	.id_table	= uvc_ids,
	.supports_autosuspend = 1,
};

static int __init uvc_init(void)
{
	uvc_debugfs_init();
	ret = usb_register(&uvc_driver);
}


uvc_probe(struct usb_interface *intf, const struct usb_device_id *id)
	struct usb_device *udev = interface_to_usbdev(intf);
	struct uvc_device *dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	uvc_parse_control(dev);
		while (buflen > 2) {
			uvc_parse_vendor_control(dev, buffer, buflen);
			uvc_parse_standard_control(dev, buffer, buflen);
				switch (buffer[2]) {
					case UVC_VC_HEADER:
						for (i = 0; i < n; ++i) {
							intf = usb_ifnum_to_if(udev, buffer[12+i]);
							uvc_parse_streaming(dev, intf);
								...
								list_add_tail(&streaming->list, &dev->streams);
						}
						break;
					case UVC_VC_INPUT_TERMINAL:
						term = uvc_alloc_entity(type | UVC_TERM_INPUT, buffer[3], 1, n + p);
						...
						list_add_tail(&term->list, &dev->entities);
					case UVC_VC_OUTPUT_TERMINAL:
						term = uvc_alloc_entity(type | UVC_TERM_OUTPUT, buffer[3], 1, 0);
						list_add_tail(&term->list, &dev->entities);
					case UVC_VC_SELECTOR_UNIT:
						unit = uvc_alloc_entity(buffer[2], buffer[3], p + 1, 0);
						list_add_tail(&unit->list, &dev->entities);
					case UVC_VC_PROCESSING_UNIT:
						unit = uvc_alloc_entity(buffer[2], buffer[3], 2, n);
						list_add_tail(&unit->list, &dev->entities);
					case UVC_VC_EXTENSION_UNIT:
						unit = uvc_alloc_entity(buffer[2], buffer[3], p + 1, n);
						list_add_tail(&unit->list, &dev->entities);
				}
		}
	uvc_gpio_parse(dev);
	v4l2_device_register(&intf->dev, &dev->vdev) < 0);
	uvc_scan_device(dev);
		list_for_each_entry(term, &dev->entities, list) {
			uvc_scan_chain(chain, term);
				while (entity != NULL)
					uvc_scan_chain_entity(chain, entity);
						switch (UVC_ENTITY_TYPE(entity)) { case ... }
						list_add_tail(&entity->chain, &chain->entities);
					uvc_scan_chain_forward(chain, entity, prev);
						switch (UVC_ENTITY_TYPE(entity)) {
							...
							case UVC_VC_SELECTOR_UNIT:
								for (i = 0; i < entity->bNrInPins; ++i) {
									term = uvc_entity_by_id(chain->dev, id);
									list_add_tail(&term->chain, &chain->entities);
									uvc_scan_chain_forward(chain, term, entity);
								}
							...
						}
					prev = entity;
					uvc_scan_chain_backward(chain, &entity);
						...
			list_add_tail(&chain->list, &dev->chains);
		}
	uvc_ctrl_init_device(dev);
		list_for_each_entry(chain, &dev->chains, list)
			uvc_ctrl_init_chain(chain);
				list_for_each_entry(entity, &chain->entities, chain) {
					entity->controls = kcalloc(ncontrols, sizeof(*ctrl), GFP_KERNEL);
					entity->ncontrols = ncontrols;
					for (i = 0; i < bControlSize * 8; ++i) {
						uvc_ctrl_init_ctrl(chain, ctrl);
						ctrl++;
					}
				}
	uvc_register_chains(dev);
		list_for_each_entry(chain, &dev->chains, list)
			uvc_register_terms(dev, chain);
				list_for_each_entry(term, &chain->entities, chain) 
					stream = uvc_stream_by_id(dev, term->id);
					stream->chain = chain;
					uvc_register_video(dev, stream);
						uvc_video_init(stream);
						uvc_register_video_device(dev, stream, &stream->vdev, &stream->queue, stream->type, &uvc_fops, &uvc_ioctl_ops);
							uvc_queue_init(queue, type, !uvc_no_drop_param);
							vdev->v4l2_dev = &dev->vdev;
							vdev->fops = fops;
							vdev->ioctl_ops = ioctl_ops;
							vdev->release = uvc_release;
							video_set_drvdata(vdev, stream);
							video_register_device(vdev, VFL_TYPE_VIDEO, -1);
					uvc_meta_register(stream);
					term->vdev = &stream->vdev;
			uvc_mc_register_entities(chain);
	media_device_register(&dev->mdev);
	usb_set_intfdata(intf, dev);
	uvc_gpio_init_irq(dev);



















