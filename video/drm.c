
#vkms
#define DRM_GEM_FOPS \
	.open		= drm_open,\
	.release	= drm_release,\
	.unlocked_ioctl	= drm_ioctl,\
	.compat_ioctl	= drm_compat_ioctl,\
	.poll		= drm_poll,\
	.read		= drm_read,\
	.llseek		= noop_llseek,\
	.mmap		= drm_gem_mmap
#define DEFINE_DRM_GEM_FOPS(name) \
	static const struct file_operations name = {\
		.owner		= THIS_MODULE,\
		DRM_GEM_FOPS,\
	}
DEFINE_DRM_GEM_FOPS(vkms_driver_fops);

#define DRM_GEM_SHMEM_DRIVER_OPS \
	.gem_prime_import_sg_table = drm_gem_shmem_prime_import_sg_table, \
	.dumb_create		   = drm_gem_shmem_dumb_create

static const struct drm_driver vkms_driver = {
	.driver_features	= DRIVER_MODESET | DRIVER_ATOMIC | DRIVER_GEM,
	.release		= vkms_release,
	.fops			= &vkms_driver_fops,
	DRM_GEM_SHMEM_DRIVER_OPS,
	.name			= DRIVER_NAME,
	.desc			= DRIVER_DESC,
	.date			= DRIVER_DATE,
	.major			= DRIVER_MAJOR,
	.minor			= DRIVER_MINOR,
};

static const struct drm_mode_config_helper_funcs vkms_mode_config_helpers = {
	.atomic_commit_tail = vkms_atomic_commit_tail,
};

static const struct drm_fb_helper_funcs drm_fbdev_generic_helper_funcs = {
	.fb_probe = drm_fbdev_generic_helper_fb_probe,
	.fb_dirty = drm_fbdev_generic_helper_fb_dirty,
};

static const struct drm_mode_config_funcs vkms_mode_funcs = {
	.fb_create = drm_gem_fb_create,
	.atomic_check = vkms_atomic_check,
	.atomic_commit = drm_atomic_helper_commit,
};

static const struct drm_crtc_funcs vkms_crtc_funcs = {
	.set_config             = drm_atomic_helper_set_config,
	.page_flip              = drm_atomic_helper_page_flip,
	.reset                  = vkms_atomic_crtc_reset,
	.atomic_duplicate_state = vkms_atomic_crtc_duplicate_state,
	.atomic_destroy_state   = vkms_atomic_crtc_destroy_state,
	.enable_vblank		= vkms_enable_vblank,
	.disable_vblank		= vkms_disable_vblank,
	.get_vblank_timestamp	= vkms_get_vblank_timestamp,
	.get_crc_sources	= vkms_get_crc_sources,
	.set_crc_source		= vkms_set_crc_source,
	.verify_crc_source	= vkms_verify_crc_source,
};

static const struct drm_crtc_helper_funcs vkms_crtc_helper_funcs = {
	.atomic_check	= vkms_crtc_atomic_check,
	.atomic_begin	= vkms_crtc_atomic_begin,
	.atomic_flush	= vkms_crtc_atomic_flush,
	.atomic_enable	= vkms_crtc_atomic_enable,
	.atomic_disable	= vkms_crtc_atomic_disable,
};

vkms_init(void)
->struct vkms_config *config;
->config = kmalloc(sizeof(*config), GFP_KERNEL);
->config->cursor = enable_cursor;
->config->writeback = enable_writeback;
->config->overlay = enable_overlay;
->vkms_create(config);
->	pdev = platform_device_register_simple(DRIVER_NAME, -1, NULL, 0);
->	devres_open_group(&pdev->dev, NULL, GFP_KERNEL)
->		grp = kmalloc(sizeof(*grp), gfp);
->		grp->node[0].release = &group_open_release;
->		grp->node[1].release = &group_close_release;
->		grp->id = grp; //or grp->id = id;
->		grp->color = 0;
->		add_dr(dev, &grp->node[0]);
->	vkms_device = devm_drm_dev_alloc(&pdev->dev, &vkms_driver, struct vkms_device, drm);
->	vkms_device->platform = pdev;
->	vkms_device->config = config;
->	config->dev = vkms_device;
->	dma_coerce_mask_and_coherent(vkms_device->drm.dev, DMA_BIT_MASK(64));
->	drm_vblank_init(&vkms_device->drm, 1/num_crtcs);
->		dev->vblank = drmm_kcalloc(dev, num_crtcs, sizeof(*dev->vblank), GFP_KERNEL);
->		dev->num_crtcs = num_crtcs;
->		for (i = 0; i < num_crtcs; i++) {
->			struct drm_vblank_crtc *vblank = &dev->vblank[i];
->			vblank->dev = dev;
->			vblank->pipe = i;
->			init_waitqueue_head(&vblank->queue);
->			timer_setup(&vblank->disable_timer, vblank_disable_fn, 0);
->			seqlock_init(&vblank->seqlock);
->			drmm_add_action_or_reset(dev, drm_vblank_init_release, vblank);
->			drm_vblank_worker_init(vblank);
->				struct kthread_worker *worker;
->				INIT_LIST_HEAD(&vblank->pending_work);
->				init_waitqueue_head(&vblank->work_wait_queue);
->				worker = kthread_create_worker(0, "card%d-crtc%d",
->							       vblank->dev->primary->index,
->							       vblank->pipe);
->				vblank->worker = worker;
->				sched_set_fifo(worker->task);
->		}
->	vkms_modeset_init(vkms_device);
->		struct drm_device *dev = &vkmsdev->drm;
->		ret = drmm_mode_config_init(dev);
->		dev->mode_config.funcs = &vkms_mode_funcs;
->		...
->		dev->mode_config.helper_private = &vkms_mode_config_helpers;
->		vkms_output_init(vkmsdev, 0);
->			struct vkms_output *output = &vkmsdev->output;
->			struct drm_device *dev = &vkmsdev->drm;
->			struct drm_connector *connector = &output->connector;
->			struct drm_encoder *encoder = &output->encoder;
->			struct drm_crtc *crtc = &output->crtc;
->			primary = vkms_plane_init(vkmsdev, DRM_PLANE_TYPE_PRIMARY, index);
->			if (vkmsdev->config->overlay)
->				for (n = 0; n < NUM_OVERLAY_PLANES; n++)
->					ret = vkms_add_overlay_plane(vkmsdev, index, crtc);
->			if (vkmsdev->config->cursor)
->				cursor = vkms_plane_init(vkmsdev, DRM_PLANE_TYPE_CURSOR, index);
->			vkms_crtc_init(dev, crtc, &primary->base, &cursor->base);
->				drmm_crtc_init_with_planes(dev, crtc, primary, cursor, &vkms_crtc_funcs, NULL);
->				drm_crtc_helper_add(crtc, &vkms_crtc_helper_funcs);
->				drm_mode_crtc_set_gamma_size(crtc, VKMS_LUT_SIZE);
->				drm_crtc_enable_color_mgmt(crtc, 0, false, VKMS_LUT_SIZE);
->			drm_connector_init(dev, connector, &vkms_connector_funcs, DRM_MODE_CONNECTOR_VIRTUAL);
->			drm_connector_helper_add(connector, &vkms_conn_helper_funcs);
->			drm_encoder_init(dev, encoder, &vkms_encoder_funcs, DRM_MODE_ENCODER_VIRTUAL, NULL);
->			encoder->possible_crtcs = 1;
->			drm_connector_attach_encoder(connector, encoder);
->			if (vkmsdev->config->writeback)
->				vkms_enable_writeback_connector(vkmsdev);
->			drm_mode_config_reset(dev);
->	drm_dev_register(&vkms_device->drm, 0);
->	drm_fbdev_generic_setup(&vkms_device->drm, 0);
->		fb_helper = kzalloc(sizeof(*fb_helper), GFP_KERNEL);
->		drm_fb_helper_prepare(dev, fb_helper, preferred_bpp, &drm_fbdev_generic_helper_funcs/funcs);
->			helper->funcs = funcs;
->			helper->dev = dev;
->			helper->preferred_bpp = preferred_bpp;
->		drm_client_init(dev, &fb_helper->client, "fbdev", &drm_fbdev_generic_client_funcs);
->			client->dev = dev;
->			client->name = name;
->			client->funcs = funcs;
->			drm_client_modeset_create(client);
->				client->modesets = kcalloc(num_crtc + 1, sizeof(*client->modesets), GFP_KERNEL);
->				drm_for_each_crtc(crtc, dev)
->					client->modesets[i++].crtc = crtc;
->				if (num_crtc == 1)
->					max_connector_count = DRM_CLIENT_MAX_CLONED_CONNECTORS;
->				for (modeset = client->modesets; modeset->crtc; modeset++) {
->					modeset->connectors = kcalloc(max_connector_count, sizeof(*modeset->connectors), GFP_KERNEL);
->			drm_client_open(client);
->				file = drm_file_alloc(dev->primary);
->				list_add(&file->lhead, &dev->filelist_internal);
->			drm_dev_get(dev);
->		drm_client_register(&fb_helper->client);
->			list_add(&client->list, &dev->clientlist);
->			if (client->funcs && client->funcs->hotplug)
->				client->funcs->hotplug(client);
->default_config = config;

#drm

#define devm_drm_dev_alloc(parent, driver, type, member) \
	((type *) __devm_drm_dev_alloc(parent, driver, sizeof(type), \
				       offsetof(type, member)))

void *__devm_drm_dev_alloc(struct device *parent,
			   const struct drm_driver *driver,
			   size_t size, size_t offset)
	container = kzalloc(size, GFP_KERNEL);
	drm = container + offset;
	devm_drm_dev_init(parent, drm, driver);
		drm_dev_init(dev, driver, parent);
			dev->dev = get_device(parent);
			dev->driver = driver;
			drmm_add_action_or_reset(dev, drm_dev_init_release, NULL);
			inode = drm_fs_inode_new();
			dev->anon_inode = inode;
			if (drm_core_check_feature(dev, DRIVER_COMPUTE_ACCEL)) {
				drm_minor_alloc(dev, DRM_MINOR_ACCEL);
			} else {
				if (drm_core_check_feature(dev, DRIVER_RENDER))
					ret = drm_minor_alloc(dev, DRM_MINOR_RENDER);
				drm_minor_alloc(dev, DRM_MINOR_PRIMARY);
			}
			drm_legacy_create_map_hash(dev);
			drm_legacy_ctxbitmap_init(dev);
			if (drm_core_check_feature(dev, DRIVER_GEM))
				ret = drm_gem_init(dev);
			dev->unique = drmm_kstrdup(dev, dev_name(parent), GFP_KERNEL);
		devm_add_action_or_reset(parent, devm_drm_dev_init_release, dev);
	drmm_add_final_kfree(drm, container);
		dev->managed.final_kfree = container;
	return container;

drm_minor_alloc(struct drm_device *dev, enum drm_minor_type type)
	xa_alloc(drm_minor_get_xa(type), &minor->index, NULL, DRM_MINOR_LIMIT(type), GFP_KERNEL);
	drmm_add_action_or_reset(dev, drm_minor_alloc_release, minor);
	minor->kdev = drm_sysfs_minor_alloc(minor);
		kdev = kzalloc(sizeof(*kdev), GFP_KERNEL);
		device_initialize(kdev);
		if (minor->type == DRM_MINOR_ACCEL) {
			minor_str = "accel%d";
			accel_set_device_instance_params(kdev, minor->index);
		} else {
			if (minor->type == DRM_MINOR_RENDER)
				minor_str = "renderD%d";
			else
				minor_str = "card%d";
			kdev->devt = MKDEV(DRM_MAJOR, minor->index);
			kdev->class = drm_class;
			kdev->type = &drm_sysfs_device_minor;
		}
		kdev->parent = minor->dev->dev;
		kdev->release = drm_sysfs_release;
		dev_set_drvdata(kdev, minor);
		dev_set_name(kdev, minor_str, minor->index);
	*drm_minor_get_slot(dev, type) = minor;

drm_dev_register(struct drm_device *dev, flags);
	const struct drm_driver *driver = dev->driver;
	drm_minor_register(dev, DRM_MINOR_RENDER);
		minor = *drm_minor_get_slot(dev, type);
			case DRM_MINOR_PRIMARY: return &dev->primary;
			case DRM_MINOR_RENDER: return &dev->render;
			case DRM_MINOR_ACCEL: return &dev->accel;
		accel_debugfs_init(minor, minor->index);/ drm_debugfs_init(minor, minor->index, drm_debugfs_root);
		entry = xa_store(drm_minor_get_xa(type), minor->index, minor, GFP_KERNEL);
	drm_minor_register(dev, DRM_MINOR_PRIMARY);
	drm_minor_register(dev, DRM_MINOR_ACCEL);
	create_compat_control_link(dev);
	dev->registered = true;
	driver->load(dev, flags); //if (driver->load)
	drm_modeset_register_all(dev); //if (drm_core_check_feature(dev, DRIVER_MODESET))
		drm_plane_register_all(dev);
			drm_for_each_plane(plane, dev) //list_for_each_entry(plane, &(dev)->mode_config.plane_list, head)
				plane->funcs->late_register(plane); //if (plane->funcs->late_register)
				num_planes++;
		drm_crtc_register_all(dev);
			drm_for_each_crtc(crtc, dev) //list_for_each_entry(crtc, &(dev)->mode_config.crtc_list, head)
				drm_debugfs_crtc_add(crtc);
				crtc->funcs->late_register(crtc); //if (crtc->funcs->late_register)
		drm_encoder_register_all(dev);
			drm_for_each_encoder(encoder, dev) //list_for_each_entry(encoder, &(dev)->mode_config.encoder_list, head)
				encoder->funcs->late_register(encoder); //if (encoder->funcs && encoder->funcs->late_register)
		drm_connector_register_all(dev);
			drm_for_each_connector_iter(connector, &conn_iter) //while ((connector = drm_connector_list_iter_next(iter)))
				drm_connector_register(connector);
		drm_debugfs_late_register(dev);
			list_for_each_entry_safe(entry, tmp, &dev->debugfs_list, list)
				debugfs_create_file(entry->file.name, 0444, minor->debugfs_root, entry, &drm_debugfs_entry_fops);
				list_del(&entry->list);
	DRM_INFO("Initialized %s %d.%d.%d %s for %s on minor %d\n",
		 driver->name, driver->major, driver->minor,
		 driver->patchlevel, driver->date,
		 dev->dev ? dev_name(dev->dev) : "virtual device",
		 dev->primary ? dev->primary->index : dev->accel->index);






enum drm_minor_type {
	DRM_MINOR_PRIMARY = 0,
	DRM_MINOR_CONTROL = 1,
	DRM_MINOR_RENDER = 2,
	DRM_MINOR_ACCEL = 32,
};



#define DRM_IOCTL_BASE			'd'
#define DRM_IO(nr)			_IO(DRM_IOCTL_BASE,nr)
#define DRM_IOR(nr,type)		_IOR(DRM_IOCTL_BASE,nr,type)
#define DRM_IOW(nr,type)		_IOW(DRM_IOCTL_BASE,nr,type)
#define DRM_IOWR(nr,type)		_IOWR(DRM_IOCTL_BASE,nr,type)

#define DRM_IOCTL_VERSION		DRM_IOWR(0x00, struct drm_version)
#define DRM_IOCTL_GET_UNIQUE		DRM_IOWR(0x01, struct drm_unique)
#define DRM_IOCTL_GET_MAGIC		DRM_IOR( 0x02, struct drm_auth)
#define DRM_IOCTL_IRQ_BUSID		DRM_IOWR(0x03, struct drm_irq_busid)
#define DRM_IOCTL_GET_MAP               DRM_IOWR(0x04, struct drm_map)
#define DRM_IOCTL_GET_CLIENT            DRM_IOWR(0x05, struct drm_client)
#define DRM_IOCTL_GET_STATS             DRM_IOR( 0x06, struct drm_stats)
#define DRM_IOCTL_SET_VERSION		DRM_IOWR(0x07, struct drm_set_version)
#define DRM_IOCTL_MODESET_CTL           DRM_IOW(0x08, struct drm_modeset_ctl)
/**
 * DRM_IOCTL_GEM_CLOSE - Close a GEM handle.
 *
 * GEM handles are not reference-counted by the kernel. User-space is
 * responsible for managing their lifetime. For example, if user-space imports
 * the same memory object twice on the same DRM file description, the same GEM
 * handle is returned by both imports, and user-space needs to ensure
 * &DRM_IOCTL_GEM_CLOSE is performed once only. The same situation can happen
 * when a memory object is allocated, then exported and imported again on the
 * same DRM file description. The &DRM_IOCTL_MODE_GETFB2 IOCTL is an exception
 * and always returns fresh new GEM handles even if an existing GEM handle
 * already refers to the same memory object before the IOCTL is performed.
 */
#define DRM_IOCTL_GEM_CLOSE		DRM_IOW (0x09, struct drm_gem_close)
#define DRM_IOCTL_GEM_FLINK		DRM_IOWR(0x0a, struct drm_gem_flink)
#define DRM_IOCTL_GEM_OPEN		DRM_IOWR(0x0b, struct drm_gem_open)
#define DRM_IOCTL_GET_CAP		DRM_IOWR(0x0c, struct drm_get_cap)
#define DRM_IOCTL_SET_CLIENT_CAP	DRM_IOW( 0x0d, struct drm_set_client_cap)

#define DRM_IOCTL_SET_UNIQUE		DRM_IOW( 0x10, struct drm_unique)
#define DRM_IOCTL_AUTH_MAGIC		DRM_IOW( 0x11, struct drm_auth)
#define DRM_IOCTL_BLOCK			DRM_IOWR(0x12, struct drm_block)
#define DRM_IOCTL_UNBLOCK		DRM_IOWR(0x13, struct drm_block)
#define DRM_IOCTL_CONTROL		DRM_IOW( 0x14, struct drm_control)
#define DRM_IOCTL_ADD_MAP		DRM_IOWR(0x15, struct drm_map)
#define DRM_IOCTL_ADD_BUFS		DRM_IOWR(0x16, struct drm_buf_desc)
#define DRM_IOCTL_MARK_BUFS		DRM_IOW( 0x17, struct drm_buf_desc)
#define DRM_IOCTL_INFO_BUFS		DRM_IOWR(0x18, struct drm_buf_info)
#define DRM_IOCTL_MAP_BUFS		DRM_IOWR(0x19, struct drm_buf_map)
#define DRM_IOCTL_FREE_BUFS		DRM_IOW( 0x1a, struct drm_buf_free)

#define DRM_IOCTL_RM_MAP		DRM_IOW( 0x1b, struct drm_map)

#define DRM_IOCTL_SET_SAREA_CTX		DRM_IOW( 0x1c, struct drm_ctx_priv_map)
#define DRM_IOCTL_GET_SAREA_CTX 	DRM_IOWR(0x1d, struct drm_ctx_priv_map)

#define DRM_IOCTL_SET_MASTER            DRM_IO(0x1e)
#define DRM_IOCTL_DROP_MASTER           DRM_IO(0x1f)

#define DRM_IOCTL_ADD_CTX		DRM_IOWR(0x20, struct drm_ctx)
#define DRM_IOCTL_RM_CTX		DRM_IOWR(0x21, struct drm_ctx)
#define DRM_IOCTL_MOD_CTX		DRM_IOW( 0x22, struct drm_ctx)
#define DRM_IOCTL_GET_CTX		DRM_IOWR(0x23, struct drm_ctx)
#define DRM_IOCTL_SWITCH_CTX		DRM_IOW( 0x24, struct drm_ctx)
#define DRM_IOCTL_NEW_CTX		DRM_IOW( 0x25, struct drm_ctx)
#define DRM_IOCTL_RES_CTX		DRM_IOWR(0x26, struct drm_ctx_res)
#define DRM_IOCTL_ADD_DRAW		DRM_IOWR(0x27, struct drm_draw)
#define DRM_IOCTL_RM_DRAW		DRM_IOWR(0x28, struct drm_draw)
#define DRM_IOCTL_DMA			DRM_IOWR(0x29, struct drm_dma)
#define DRM_IOCTL_LOCK			DRM_IOW( 0x2a, struct drm_lock)
#define DRM_IOCTL_UNLOCK		DRM_IOW( 0x2b, struct drm_lock)
#define DRM_IOCTL_FINISH		DRM_IOW( 0x2c, struct drm_lock)

/**
 * DRM_IOCTL_PRIME_HANDLE_TO_FD - Convert a GEM handle to a DMA-BUF FD.
 *
 * User-space sets &drm_prime_handle.handle with the GEM handle to export and
 * &drm_prime_handle.flags, and gets back a DMA-BUF file descriptor in
 * &drm_prime_handle.fd.
 *
 * The export can fail for any driver-specific reason, e.g. because export is
 * not supported for this specific GEM handle (but might be for others).
 *
 * Support for exporting DMA-BUFs is advertised via &DRM_PRIME_CAP_EXPORT.
 */
#define DRM_IOCTL_PRIME_HANDLE_TO_FD    DRM_IOWR(0x2d, struct drm_prime_handle)
/**
 * DRM_IOCTL_PRIME_FD_TO_HANDLE - Convert a DMA-BUF FD to a GEM handle.
 *
 * User-space sets &drm_prime_handle.fd with a DMA-BUF file descriptor to
 * import, and gets back a GEM handle in &drm_prime_handle.handle.
 * &drm_prime_handle.flags is unused.
 *
 * If an existing GEM handle refers to the memory object backing the DMA-BUF,
 * that GEM handle is returned. Therefore user-space which needs to handle
 * arbitrary DMA-BUFs must have a user-space lookup data structure to manually
 * reference-count duplicated GEM handles. For more information see
 * &DRM_IOCTL_GEM_CLOSE.
 *
 * The import can fail for any driver-specific reason, e.g. because import is
 * only supported for DMA-BUFs allocated on this DRM device.
 *
 * Support for importing DMA-BUFs is advertised via &DRM_PRIME_CAP_IMPORT.
 */
#define DRM_IOCTL_PRIME_FD_TO_HANDLE    DRM_IOWR(0x2e, struct drm_prime_handle)

#define DRM_IOCTL_AGP_ACQUIRE		DRM_IO(  0x30)
#define DRM_IOCTL_AGP_RELEASE		DRM_IO(  0x31)
#define DRM_IOCTL_AGP_ENABLE		DRM_IOW( 0x32, struct drm_agp_mode)
#define DRM_IOCTL_AGP_INFO		DRM_IOR( 0x33, struct drm_agp_info)
#define DRM_IOCTL_AGP_ALLOC		DRM_IOWR(0x34, struct drm_agp_buffer)
#define DRM_IOCTL_AGP_FREE		DRM_IOW( 0x35, struct drm_agp_buffer)
#define DRM_IOCTL_AGP_BIND		DRM_IOW( 0x36, struct drm_agp_binding)
#define DRM_IOCTL_AGP_UNBIND		DRM_IOW( 0x37, struct drm_agp_binding)

#define DRM_IOCTL_SG_ALLOC		DRM_IOWR(0x38, struct drm_scatter_gather)
#define DRM_IOCTL_SG_FREE		DRM_IOW( 0x39, struct drm_scatter_gather)

#define DRM_IOCTL_WAIT_VBLANK		DRM_IOWR(0x3a, union drm_wait_vblank)

#define DRM_IOCTL_CRTC_GET_SEQUENCE	DRM_IOWR(0x3b, struct drm_crtc_get_sequence)
#define DRM_IOCTL_CRTC_QUEUE_SEQUENCE	DRM_IOWR(0x3c, struct drm_crtc_queue_sequence)

#define DRM_IOCTL_UPDATE_DRAW		DRM_IOW(0x3f, struct drm_update_draw)

#define DRM_IOCTL_MODE_GETRESOURCES	DRM_IOWR(0xA0, struct drm_mode_card_res)
#define DRM_IOCTL_MODE_GETCRTC		DRM_IOWR(0xA1, struct drm_mode_crtc)
#define DRM_IOCTL_MODE_SETCRTC		DRM_IOWR(0xA2, struct drm_mode_crtc)
#define DRM_IOCTL_MODE_CURSOR		DRM_IOWR(0xA3, struct drm_mode_cursor)
#define DRM_IOCTL_MODE_GETGAMMA		DRM_IOWR(0xA4, struct drm_mode_crtc_lut)
#define DRM_IOCTL_MODE_SETGAMMA		DRM_IOWR(0xA5, struct drm_mode_crtc_lut)
#define DRM_IOCTL_MODE_GETENCODER	DRM_IOWR(0xA6, struct drm_mode_get_encoder)
#define DRM_IOCTL_MODE_GETCONNECTOR	DRM_IOWR(0xA7, struct drm_mode_get_connector)
#define DRM_IOCTL_MODE_ATTACHMODE	DRM_IOWR(0xA8, struct drm_mode_mode_cmd) /* deprecated (never worked) */
#define DRM_IOCTL_MODE_DETACHMODE	DRM_IOWR(0xA9, struct drm_mode_mode_cmd) /* deprecated (never worked) */

#define DRM_IOCTL_MODE_GETPROPERTY	DRM_IOWR(0xAA, struct drm_mode_get_property)
#define DRM_IOCTL_MODE_SETPROPERTY	DRM_IOWR(0xAB, struct drm_mode_connector_set_property)
#define DRM_IOCTL_MODE_GETPROPBLOB	DRM_IOWR(0xAC, struct drm_mode_get_blob)
#define DRM_IOCTL_MODE_GETFB		DRM_IOWR(0xAD, struct drm_mode_fb_cmd)
#define DRM_IOCTL_MODE_ADDFB		DRM_IOWR(0xAE, struct drm_mode_fb_cmd)
/**
 * DRM_IOCTL_MODE_RMFB - Remove a framebuffer.
 *
 * This removes a framebuffer previously added via ADDFB/ADDFB2. The IOCTL
 * argument is a framebuffer object ID.
 *
 * Warning: removing a framebuffer currently in-use on an enabled plane will
 * disable that plane. The CRTC the plane is linked to may also be disabled
 * (depending on driver capabilities).
 */
#define DRM_IOCTL_MODE_RMFB		DRM_IOWR(0xAF, unsigned int)
#define DRM_IOCTL_MODE_PAGE_FLIP	DRM_IOWR(0xB0, struct drm_mode_crtc_page_flip)
#define DRM_IOCTL_MODE_DIRTYFB		DRM_IOWR(0xB1, struct drm_mode_fb_dirty_cmd)

#define DRM_IOCTL_MODE_CREATE_DUMB DRM_IOWR(0xB2, struct drm_mode_create_dumb)
#define DRM_IOCTL_MODE_MAP_DUMB    DRM_IOWR(0xB3, struct drm_mode_map_dumb)
#define DRM_IOCTL_MODE_DESTROY_DUMB    DRM_IOWR(0xB4, struct drm_mode_destroy_dumb)
#define DRM_IOCTL_MODE_GETPLANERESOURCES DRM_IOWR(0xB5, struct drm_mode_get_plane_res)
#define DRM_IOCTL_MODE_GETPLANE	DRM_IOWR(0xB6, struct drm_mode_get_plane)
#define DRM_IOCTL_MODE_SETPLANE	DRM_IOWR(0xB7, struct drm_mode_set_plane)
#define DRM_IOCTL_MODE_ADDFB2		DRM_IOWR(0xB8, struct drm_mode_fb_cmd2)
#define DRM_IOCTL_MODE_OBJ_GETPROPERTIES	DRM_IOWR(0xB9, struct drm_mode_obj_get_properties)
#define DRM_IOCTL_MODE_OBJ_SETPROPERTY	DRM_IOWR(0xBA, struct drm_mode_obj_set_property)
#define DRM_IOCTL_MODE_CURSOR2		DRM_IOWR(0xBB, struct drm_mode_cursor2)
#define DRM_IOCTL_MODE_ATOMIC		DRM_IOWR(0xBC, struct drm_mode_atomic)
#define DRM_IOCTL_MODE_CREATEPROPBLOB	DRM_IOWR(0xBD, struct drm_mode_create_blob)
#define DRM_IOCTL_MODE_DESTROYPROPBLOB	DRM_IOWR(0xBE, struct drm_mode_destroy_blob)

#define DRM_IOCTL_SYNCOBJ_CREATE	DRM_IOWR(0xBF, struct drm_syncobj_create)
#define DRM_IOCTL_SYNCOBJ_DESTROY	DRM_IOWR(0xC0, struct drm_syncobj_destroy)
#define DRM_IOCTL_SYNCOBJ_HANDLE_TO_FD	DRM_IOWR(0xC1, struct drm_syncobj_handle)
#define DRM_IOCTL_SYNCOBJ_FD_TO_HANDLE	DRM_IOWR(0xC2, struct drm_syncobj_handle)
#define DRM_IOCTL_SYNCOBJ_WAIT		DRM_IOWR(0xC3, struct drm_syncobj_wait)
#define DRM_IOCTL_SYNCOBJ_RESET		DRM_IOWR(0xC4, struct drm_syncobj_array)
#define DRM_IOCTL_SYNCOBJ_SIGNAL	DRM_IOWR(0xC5, struct drm_syncobj_array)

#define DRM_IOCTL_MODE_CREATE_LEASE	DRM_IOWR(0xC6, struct drm_mode_create_lease)
#define DRM_IOCTL_MODE_LIST_LESSEES	DRM_IOWR(0xC7, struct drm_mode_list_lessees)
#define DRM_IOCTL_MODE_GET_LEASE	DRM_IOWR(0xC8, struct drm_mode_get_lease)
#define DRM_IOCTL_MODE_REVOKE_LEASE	DRM_IOWR(0xC9, struct drm_mode_revoke_lease)

#define DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT	DRM_IOWR(0xCA, struct drm_syncobj_timeline_wait)
#define DRM_IOCTL_SYNCOBJ_QUERY		DRM_IOWR(0xCB, struct drm_syncobj_timeline_array)
#define DRM_IOCTL_SYNCOBJ_TRANSFER	DRM_IOWR(0xCC, struct drm_syncobj_transfer)
#define DRM_IOCTL_SYNCOBJ_TIMELINE_SIGNAL	DRM_IOWR(0xCD, struct drm_syncobj_timeline_array)

/**
 * DRM_IOCTL_MODE_GETFB2 - Get framebuffer metadata.
 *
 * This queries metadata about a framebuffer. User-space fills
 * &drm_mode_fb_cmd2.fb_id as the input, and the kernels fills the rest of the
 * struct as the output.
 *
 * If the client is DRM master or has &CAP_SYS_ADMIN, &drm_mode_fb_cmd2.handles
 * will be filled with GEM buffer handles. Fresh new GEM handles are always
 * returned, even if another GEM handle referring to the same memory object
 * already exists on the DRM file description. The caller is responsible for
 * removing the new handles, e.g. via the &DRM_IOCTL_GEM_CLOSE IOCTL. The same
 * new handle will be returned for multiple planes in case they use the same
 * memory object. Planes are valid until one has a zero handle -- this can be
 * used to compute the number of planes.
 *
 * Otherwise, &drm_mode_fb_cmd2.handles will be zeroed and planes are valid
 * until one has a zero &drm_mode_fb_cmd2.pitches.
 *
 * If the framebuffer has a format modifier, &DRM_MODE_FB_MODIFIERS will be set
 * in &drm_mode_fb_cmd2.flags and &drm_mode_fb_cmd2.modifier will contain the
 * modifier. Otherwise, user-space must ignore &drm_mode_fb_cmd2.modifier.
 *
 * To obtain DMA-BUF FDs for each plane without leaking GEM handles, user-space
 * can export each handle via &DRM_IOCTL_PRIME_HANDLE_TO_FD, then immediately
 * close each unique handle via &DRM_IOCTL_GEM_CLOSE, making sure to not
 * double-close handles which are specified multiple times in the array.
 */
#define DRM_IOCTL_MODE_GETFB2		DRM_IOWR(0xCE, struct drm_mode_fb_cmd2)

#define DRM_IOCTL_SYNCOBJ_EVENTFD	DRM_IOWR(0xCF, struct drm_syncobj_eventfd)

#flow:
1. resources & init
fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC)
	sys_open -> drm_open
drmModeGetResources(fd); //DRM_IOCTL_MODE_GETRESOURCES
drmModeGetConnector(fd, connector_id); //DRM_IOCTL_MODE_GETCONNECTOR

2. memory & map
drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_req); //DRM_IOCTL_MODE_CREATE_DUMB
	drm_mode_create_dumb -> driver->dumb_create (eg: drm_gem_shmem_dumb_create)
drmModeAddFB(fd, ...); // DRM_IOCTL_MODE_ADDFB
drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map_req) -> mmap(...) //DRM_IOCTL_MODE_MAP_DUMB
mmap();

3. screen setting
drmModeSetCrtc(fd, fb_id, conn_id, mode); //DRM_IOCTL_MODE_SETCRTC
	drm_atomic_commit -> drm_atomic_helper_commit
	driver->atomic_check
	drm_atomic_helper_commit_modeset_enables
	drm_atomic_helper_commit_planes

4. page filp
drmModePageFlip(fd, crtc_id, new_fb_id, flags, user_data) //DRM_IOCTL_MODE_PAGE_FLIP
	drm_mode_page_flip_ioctl -> drm_atomic_commit 异步处理: 内核不会立即切换画面，而是将请求挂起，等待下一个 VBlank 事件。
VBlank Interrupt (硬件中断)
    Hardware: 显示器扫描完一帧，发出中断。
    Kernel ISR: drm_vblank_handler
    Tasklet/Worker: 唤醒等待的线程，执行实际的硬件寄存器更新（切换 FB 指针）。
Event Notification (事件通知)
    Kernel: 向 fd 的文件描述符写入可读事件（poll 返回）。
    User: App 通过 read 或 poll 感知到 Flip 完成。
    Loop: App 准备下一帧画面，回到步骤二（或者复用 Buffer），继续调用 Page Flip。


## DRM 应用程序 VBlank 流程
这是实现高帧率、无撕裂动画的标准流程（通常称为 SwapBuffers 流程）。
Step 1: 请求 Page Flip
应用程序准备好下一帧图像后，调用 drmModePageFlip。
    参数: 指定新的 Framebuffer，并设置 DRM_MODE_PAGE_FLIP_EVENT 标志。
    状态: 此时调用立即返回，不会阻塞等待 VBlank。这允许 CPU 在 GPU 等待 VBlank 的同时去准备下一帧（Pipeline 并行）。
Step 2: 内核排队
    内核将这个 Flip 请求挂起，等待下一次 VBlank 中断到来。
    此时，旧的 Framebuffer 依然在显示。
Step 3: VBlank 中断触发
    硬件触发 VBlank IRQ。
    内核处理中断，发现有一个挂起的 Page Flip 请求。
    执行: 内核瞬间修改 CRTC 寄存器，将显示源指向新的 Framebuffer。
Step 4: 发送事件通知
    因为设置了 DRM_MODE_PAGE_FLIP_EVENT，内核会发送一个事件（Event）到用户空间。
    这个事件可以通过 read() 文件描述符读取到。
Step 5: 应用程序循环
    应用程序通过 poll() 或 select() 监听 fd。
    当收到 Flip 完成事件后，应用程序知道上一帧已经上屏了。
    应用程序开始计算下一帧逻辑，并再次调用 drmModePageFlip。

##fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC)
	drm_open_helper(filp, minor);
		priv = drm_file_alloc(minor);
			...
			drm_gem_open(dev, file); //if (drm_core_check_feature(dev, DRIVER_GEM))
			drm_syncobj_open(file); //if (drm_core_check_feature(dev, DRIVER_SYNCOBJ))
			drm_prime_init_file_private(&file->prime);
			dev->driver->open(dev, file); //if (dev->driver->open)
		drm_master_open(priv); //if (drm_is_primary_client(priv))
		list_add(&priv->lhead, &dev->filelist);


## DRM_IOCTL_MODE_GETRESOURCES
drmModeGetResources(fd);
drm_mode_getresources(struct drm_device *dev, void *data/card_res, struct drm_file *file_priv)
	fb_id = u64_to_user_ptr(card_res->fb_id_ptr);
	crtc_id = u64_to_user_ptr(card_res->crtc_id_ptr);
	encoder_id = u64_to_user_ptr(card_res->encoder_id_ptr);
	connector_id = u64_to_user_ptr(card_res->connector_id_ptr);
	list_for_each_entry(fb, &file_priv->fbs, filp_head) count++;
		put_user(fb->base.id, fb_id + count)
	drm_for_each_crtc(crtc, dev) count++;
		put_user(crtc->base.id, crtc_id + count)
	drm_for_each_encoder(encoder, dev) count++;
		put_user(encoder->base.id, encoder_id + count)
	drm_for_each_connector_iter(connector, &conn_iter) count++;
		if (drm_lease_held(file_priv, connector->base.id))
			put_user(connector->base.id, connector_id + count)
	card_res->count_fbs = count_fbs; //count++
	card_res->count_crtcs = count_crtcs;
	card_res->count_encoders = count_encoders;
	card_res->count_connectors = count_connectors;
	card_res->max_height = dev->mode_config.max_height;
	card_res->min_height = dev->mode_config.min_height;
	card_res->max_width = dev->mode_config.max_width;
	card_res->min_width = dev->mode_config.min_width;

## DRM_IOCTL_MODE_GETCONNECTOR
drmModeGetConnector(fd, conn_id); //conn_id = res->connectors[0]
DRM_IOCTL_DEF(DRM_IOCTL_MODE_GETCONNECTOR, drm_mode_getconnector, 0),

drm_mode_getconnector(struct drm_device *dev, void *data, struct drm_file *file_priv)
	connector = drm_connector_lookup(dev, file_priv, out_resp->connector_id);
	encoders_count = hweight32(connector->possible_encoders);
	if ((out_resp->count_encoders >= encoders_count) && encoders_count)
		encoder_ptr = (uint32_t __user *)(unsigned long)(out_resp->encoders_ptr);
		drm_connector_for_each_possible_encoder(connector, encoder) copied++;
			if (put_user(encoder->base.id, encoder_ptr + copied)
	out_resp->count_encoders = encoders_count;
	out_resp->connector_id = connector->base.id;
	out_resp->connector_type = connector->connector_type;
	out_resp->connector_type_id = connector->connector_type_id;
	if (out_resp->count_modes == 0 && if (drm_is_current_master(file_priv)))
			connector->funcs->fill_modes(connector, dev->mode_config.max_width, dev->mode_config.max_height);
	out_resp->mm_width = connector->display_info.width_mm;
	out_resp->mm_height = connector->display_info.height_mm;
	out_resp->subpixel = connector->display_info.subpixel_order;
	out_resp->connection = connector->status;
	/* delayed so we get modes regardless of pre-fill_modes state */
	list_for_each_entry(mode, &connector->modes, head)
		if (drm_mode_expose_to_userspace(mode, &connector->modes, file_priv))
			mode->expose_to_userspace = true; mode_count++;
	/*
	 * This ioctl is called twice, once to determine how much space is
	 * needed, and the 2nd time to fill it.
	 */
	if ((out_resp->count_modes >= mode_count) && mode_count) {
		copied = 0;
		mode_ptr = (struct drm_mode_modeinfo __user *)(unsigned long)out_resp->modes_ptr;
		list_for_each_entry(mode, &connector->modes, head) {
			mode->expose_to_userspace = false;
			drm_mode_convert_to_umode(&u_mode, mode);
			copy_to_user(mode_ptr + copied, &u_mode, sizeof(u_mode));
			copied++;
		}
	} else
		list_for_each_entry(mode, &connector->modes, head)
			mode->expose_to_userspace = false;
	out_resp->count_modes = mode_count;

	encoder = drm_connector_get_encoder(connector);
	if (encoder)
		out_resp->encoder_id = encoder->base.id;
	else
		out_resp->encoder_id = 0;
	/* Only grab properties after probing, to make sure EDID and other
	 * properties reflect the latest status.
	 */
	ret = drm_mode_object_get_properties(&connector->base, file_priv->atomic,
			(uint32_t __user *)(unsigned long)(out_resp->props_ptr),
			(uint64_t __user *)(unsigned long)(out_resp->prop_values_ptr),
			&out_resp->count_props);


## DRM_IOCTL_MODE_CREATE_DUMB
drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create);
DRM_IOCTL_DEF(DRM_IOCTL_MODE_CREATE_DUMB, drm_mode_create_dumb_ioctl, 0),

static const struct drm_gem_object_funcs drm_gem_shmem_funcs = {
	.free = drm_gem_shmem_object_free,
	.print_info = drm_gem_shmem_object_print_info,
	.pin = drm_gem_shmem_object_pin,
	.unpin = drm_gem_shmem_object_unpin,
	.get_sg_table = drm_gem_shmem_object_get_sg_table,
	.vmap = drm_gem_shmem_object_vmap,
	.vunmap = drm_gem_shmem_object_vunmap,
	.mmap = drm_gem_shmem_object_mmap,
	.vm_ops = &drm_gem_shmem_vm_ops,
};

drm_mode_create_dumb_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv)
	drm_mode_create_dumb(dev, data, file_priv);
		args->handle = 0;
		args->pitch = 0;
		args->size = 0;
		dev->driver->dumb_create(file_priv, dev, args);

drm_gem_shmem_dumb_create(file_priv, dev, args)
->drm_gem_shmem_create_with_handle(file, dev, args->size, &args->handle);
	shmem = drm_gem_shmem_create(dev, size);
		__drm_gem_shmem_create(dev, size, false/private);
			struct drm_gem_shmem_object *shmem;
			struct drm_gem_object *obj;
			if (dev->driver->gem_create_object) {
				obj = dev->driver->gem_create_object(dev, size);
				shmem = to_drm_gem_shmem_obj(obj);
			} else {
				shmem = kzalloc(sizeof(*shmem), GFP_KERNEL);
				obj = &shmem->base;
			}
			if (!obj->funcs)
				obj->funcs = &drm_gem_shmem_funcs;
			if (private)
				drm_gem_private_object_init(dev, obj, size);
			else
				drm_gem_object_init(dev, obj, size);
					drm_gem_private_object_init(dev, obj, size);
					filp = shmem_file_setup("drm mm object", size, VM_NORESERVE);
					obj->filp = filp;
			drm_gem_create_mmap_offset(obj);
	drm_gem_handle_create(file_priv, &shmem->base, handle/handlep);
		drm_gem_handle_create_tail(file_priv, obj, handlep);
			handle  = idr_alloc(&file_priv->object_idr, obj, 1, 0, GFP_NOWAIT);
			drm_vma_node_allow(&obj->vma_node, file_priv);
				vma_node_allow(node, tag, true);
			obj->funcs->open(obj, file_priv); //if (obj->funcs->open)
			*handlep = handle;
	drm_gem_object_put(&shmem->base);

int xxx_gem_dma_dumb_create(struct drm_file *file,
				struct drm_device *dev,
				struct drm_mode_create_dumb *args)
->drm_gem_dma_dumb_create_internal(file, dev, args);
-->drm_gem_dma_create_with_handle(file_priv, drm, args->size, &args->handle);
	dma_obj = drm_gem_dma_create(drm, size);
		dma_obj = __drm_gem_dma_create(drm, size, false);
			if (dma_obj->map_noncoherent)
				dma_obj->vaddr = dma_alloc_noncoherent(drm->dev, size, &dma_obj->dma_addr, DMA_TO_DEVICE, GFP_KERNEL | __GFP_NOWARN);
			else
				dma_obj->vaddr = dma_alloc_wc(drm->dev, size, &dma_obj->dma_addr, GFP_KERNEL | __GFP_NOWARN);
			gem_obj->funcs = &drm_gem_dma_default_funcs; //if (!gem_obj->funcs)
			if (private) {
				drm_gem_private_object_init(drm, gem_obj, size);
				dma_obj->map_noncoherent = false;
			} else
				drm_gem_object_init(drm, gem_obj, size);
			drm_gem_create_mmap_offset(gem_obj);
			return dma_obj;
	gem_obj = &dma_obj->base;
	drm_gem_handle_create(file_priv, gem_obj, handle);
	drm_gem_object_put(gem_obj);

drm_gem_create_mmap_offset(struct drm_gem_object *obj)
->drm_gem_create_mmap_offset_size(obj, obj->size);
-->drm_vma_offset_add(obj->dev->vma_offset_manager, &obj->vma_node, size / PAGE_SIZE);
--->drm_mm_insert_node(&mgr->vm_addr_space_mm, &node->vm_node, pages); //if (!drm_mm_node_allocated(&node->vm_node))
---->drm_mm_insert_node_generic(mm, node, size, 0, 0, 0);
----->drm_mm_insert_node_in_range(mm, node, size, alignment, color, 0, U64_MAX, mode);
		for (hole = first_hole(mm, range_start, range_end, size, mode);
		     hole;
		     hole = once ? NULL : next_hole(mm, hole, size, mode)) {
			...
			__set_bit(DRM_MM_NODE_ALLOCATED_BIT, &node->flags);
			list_add(&node->node_list, &hole->node_list);
			drm_mm_interval_tree_add_node(hole, node);

			rm_hole(hole);
			if (adj_start > hole_start)
				add_hole(hole);
			if (adj_start + size < hole_end)
				add_hole(node);
		}

//mmap

##DRM_IOCTL_MODE_ADDFB
drmModeAddFB(fd, bo->width, bo->height, 24, 32, bo->pitch, bo->handle, bo->fb_id);
DRM_IOCTL_DEF(DRM_IOCTL_MODE_ADDFB, drm_mode_addfb_ioctl, 0),

int drm_mode_addfb_ioctl(struct drm_device *dev,
			 void *data, struct drm_file *file_priv)
	drm_mode_addfb(dev, data, file_priv);
		r.pixel_format = drm_driver_legacy_fb_format(dev, or->bpp, or->depth);
		/* convert to new format and call new ioctl */
		r.fb_id = or->fb_id;
		r.width = or->width;
		r.height = or->height;
		r.pitches[0] = or->pitch;
		r.handles[0] = or->handle;
		drm_mode_addfb2(dev, &r, file_priv);
			drm_internal_framebuffer_create(dev, r, file_priv);
				framebuffer_check(dev, r);
				dev->mode_config.funcs->fb_create(dev, file_priv, r);
			list_add(&fb->filp_head, &file_priv->fbs);
		or->fb_id = r.fb_id;

struct drm_framebuffer *
drm_gem_fb_create(struct drm_device *dev, struct drm_file *file, const struct drm_mode_fb_cmd2 *mode_cmd)
	drm_gem_fb_create_with_funcs(dev, file, mode_cmd, &drm_gem_fb_funcs);
		fb = kzalloc(sizeof(*fb), GFP_KERNEL);
		drm_gem_fb_init_with_funcs(dev, fb, file, mode_cmd, funcs);
			info = drm_get_format_info(dev, mode_cmd);
					info = dev->mode_config.funcs->get_format_info(mode_cmd);
					info = drm_format_info(mode_cmd->pixel_format);
			for (i = 0; i < info->num_planes; i++)
				objs[i] = drm_gem_object_lookup(file, mode_cmd->handles[i]);
					objects_lookup(filp, &handle, 1, &obj);
			drm_gem_fb_init(dev, fb, mode_cmd, objs, i, funcs);
				drm_helper_mode_fill_fb_struct(dev, fb, mode_cmd);
				for (i = 0; i < num_planes; i++)
					fb->obj[i] = obj[i];
				drm_framebuffer_init(dev, fb, funcs);
					fb->funcs = funcs;
					strcpy(fb->comm, current->comm);
					__drm_mode_object_add(dev, &fb->base, DRM_MODE_OBJECT_FB, false, drm_framebuffer_free);
					dev->mode_config.num_fb++;
					list_add(&fb->head, &dev->mode_config.fb_list);
					drm_mode_object_register(dev, &fb->base);

##DRM_IOCTL_MODE_MAP_DUMB
drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMP, &map);
DRM_IOCTL_DEF(DRM_IOCTL_MODE_MAP_DUMB, drm_mode_mmap_dumb_ioctl, 0),

drm_mode_mmap_dumb_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv)
	if (dev->driver->dumb_map_offset)
		dev->driver->dumb_map_offset(file_priv, dev, args->handle, &args->offset);
	else
		drm_gem_dumb_map_offset(file_priv, dev, args->handle, &args->offset);
			obj = drm_gem_object_lookup(file, handle);
			drm_gem_create_mmap_offset(obj);
			*offset = drm_vma_node_offset_addr(&obj->vma_node);
				return ((__u64)node->vm_node.start) << PAGE_SHIFT;
			drm_gem_object_put(obj);

##DRM_IOCTL_MODE_SETCRTC
drmModeSetCrtc(fd, crtc_id, buf.fb_id, 0, 0, &conn_id, 1, &conn->modes[0]);
DRM_IOCTL_DEF(DRM_IOCTL_MODE_SETCRTC, drm_mode_setcrtc, DRM_MASTER),

drm_mode_setcrtc(struct drm_device *dev, void *data, struct drm_file *file_priv)
	struct drm_mode_config *config = &dev->mode_config;
	struct drm_mode_crtc *crtc_req = data;
	crtc = drm_crtc_find(dev, file_priv, crtc_req->crtc_id);
	...
	set.crtc = crtc;
	set.x = crtc_req->x;
	set.y = crtc_req->y;
	set.mode = mode;
	set.connectors = connector_set;
	set.num_connectors = num_connectors;
	set.fb = fb;
	ret = crtc->funcs->set_config(&set, &ctx); //atomic ? __drm_mode_set_config_internal(&set, &ctx);
						   //
drm_atomic_helper_set_config(struct drm_mode_set *set, struct drm_modeset_acquire_ctx *ctx)
	state = drm_atomic_state_alloc(crtc->dev);
		if (config->funcs->atomic_state_alloc)
			config->funcs->atomic_state_alloc(dev);
		else
			state = kzalloc(sizeof(*state), GFP_KERNEL);
			drm_atomic_state_init(dev, state);
				state->crtcs = kcalloc(dev->mode_config.num_crtc, sizeof(*state->crtcs), GFP_KERNEL);
				state->planes = kcalloc(dev->mode_config.num_total_plane, sizeof(*state->planes), GFP_KERNEL);
				state->dev = dev;
	state->acquire_ctx = ctx;
	__drm_atomic_helper_set_config(set, state);
		...
	handle_conflicting_encoders(state, true);
		...
	drm_atomic_commit(state);
		config->funcs->atomic_commit(state->dev, state, false);

drm_atomic_helper_commit(struct drm_device *dev, struct drm_atomic_state *state, bool nonblock)
	if (state->async_update) {
		ret = drm_atomic_helper_prepare_planes(dev, state);
		drm_atomic_helper_async_commit(dev, state);
		drm_atomic_helper_unprepare_planes(dev, state);
		return 0;
	}
	ret = drm_atomic_helper_setup_commit(state, nonblock);
	INIT_WORK(&state->commit_work, commit_work);
	ret = drm_atomic_helper_prepare_planes(dev, state);
	if (!nonblock)
		drm_atomic_helper_wait_for_fences(dev, state, true);
	drm_atomic_helper_swap_state(state, true);
	commit_tail(state); // or if (nonblock) queue_work(system_unbound_wq, &state->commit_work);
		drm_atomic_helper_wait_for_fences(dev, old_state, false);
		drm_atomic_helper_wait_for_dependencies(old_state);
		if (funcs && funcs->atomic_commit_tail)
			funcs->atomic_commit_tail(old_state);
		else
			drm_atomic_helper_commit_tail(old_state);
				struct drm_device *dev = old_state->dev;
				drm_atomic_helper_commit_modeset_disables(dev, old_state);
				drm_atomic_helper_commit_planes(dev, old_state, 0);
					for_each_oldnew_crtc_in_state(old_state, crtc, old_crtc_state, new_crtc_state, i)
						funcs->atomic_begin(crtc, old_state); // funcs = crtc->helper_private;
					for_each_oldnew_plane_in_state(old_state, plane, old_plane_state, new_plane_state, i)
						...
					for_each_oldnew_crtc_in_state(old_state, crtc, old_crtc_state, new_crtc_state, i)
						funcs->atomic_flush(crtc, old_state); //funcs = crtc->helper_private;
					for_each_old_plane_in_state(old_state, plane, old_plane_state, i)
						funcs->end_fb_access(plane, old_plane_state); //if (funcs->end_fb_access)
				drm_atomic_helper_commit_modeset_enables(dev, old_state);
					for_each_oldnew_crtc_in_state(old_state, crtc, old_crtc_state, new_crtc_state, i) {
						funcs = crtc->helper_private;
						if (new_crtc_state->enable) {
							if (funcs->atomic_enable) funcs->atomic_enable(crtc, old_state);
							else if (funcs->commit) funcs->commit(crtc);
						}
					}
					for_each_new_connector_in_state(old_state, connector, new_conn_state, i) {
						...
						if (funcs) { //funcs = encoder->helper_private;
							if (funcs->atomic_enable) funcs->atomic_enable(encoder, old_state);
							else if (funcs->enable) funcs->enable(encoder);
							else if (funcs->commit) funcs->commit(encoder);
						}
						drm_atomic_bridge_chain_enable(bridge, old_state);
					}
					drm_atomic_helper_commit_writebacks(dev, old_state);
						for_each_new_connector_in_state(old_state, connector, new_conn_state, i)
							funcs->atomic_commit(connector, old_state);
				drm_atomic_helper_fake_vblank(old_state);
				drm_atomic_helper_commit_hw_done(old_state);
				drm_atomic_helper_wait_for_vblanks(dev, old_state);
				drm_atomic_helper_cleanup_planes(dev, old_state);


##DRM_IOCTL_MODE_PAGE_FLIP
drmModePageFlip(fd, crtc_id, new_fb_id, flags, user_data)
DRM_IOCTL_DEF(DRM_IOCTL_MODE_PAGE_FLIP, drm_mode_page_flip_ioctl, DRM_MASTER),

##DRM_IOCTL_MODE_RMFB
drmModeRmFB(fd, bo->fb_id);
DRM_IOCTL_DEF(DRM_IOCTL_MODE_RMFB, drm_mode_rmfb_ioctl, 0),

##dRM_IOCTL_MODE_DESTROY_DUMB
drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
DRM_IOCTL_DEF(DRM_IOCTL_MODE_DESTROY_DUMB, drm_mode_destroy_dumb_ioctl, 0),


ref:
https://blog.csdn.net/hexiaolong2009/article/details/83720940
https://blog.csdn.net/qq_41709234/article/details/129472180
https://github.com/tantan580/drm-demo/tree/master
https://github.com/tantan580/drm-demo.git

