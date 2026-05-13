#dpu

struct linlondp_kms_dev {
	/** @base: &drm_device */
	struct drm_device base;

	/** @n_crtcs: valid numbers of crtcs in &linlondp_kms_dev.crtcs */
	int n_crtcs;
	/** @crtcs: crtcs list */
	struct linlondp_crtc crtcs[LINLONDP_MAX_PIPELINES];
};


static const struct of_device_id linlondp_of_match[] = {
	{
		.compatible = "armchina,linlon-d8",
		.data = dp_identify,
	},
	{
		.compatible = "armchina,linlon-d6",
		.data = dp_identify,
	},
	{
		.compatible = "armchina,linlon-d2",
		.data = dp_identify,
	},
	{},
};

static const struct acpi_device_id linlondp_acpi_match[] = {
	{
		.id = "CIXH5010",
		.driver_data = (kernel_ulong_t)dp_identify,
	},
	{},
};

static const struct component_master_ops linlondp_master_ops = {
	.bind = linlondp_bind,
	.unbind = linlondp_unbind,
};

#define DEFINE_DRM_GEM_DMA_FOPS(name) \
	static const struct file_operations name = {\
		.owner		= THIS_MODULE,\
		.open		= drm_open,\
		.release	= drm_release,\
		.unlocked_ioctl	= drm_ioctl,\
		.compat_ioctl	= drm_compat_ioctl,\
		.poll		= drm_poll,\
		.read		= drm_read,\
		.llseek		= noop_llseek,\
		.mmap		= drm_gem_mmap,\
		DRM_GEM_DMA_UNMAPPED_AREA_FOPS \
	}
DEFINE_DRM_GEM_DMA_FOPS(linlondp_cma_fops);

static struct drm_driver linlondp_kms_driver = {
	.driver_features = DRIVER_GEM | DRIVER_MODESET | DRIVER_ATOMIC,
	.lastclose = drm_fb_helper_lastclose,
	DRM_GEM_DMA_DRIVER_OPS_WITH_DUMB_CREATE(linlondp_gem_dma_dumb_create),
	.fops = &linlondp_cma_fops,
	.name = "linlondp",
	.desc = "Linlon Display Processor driver",
	.date = "20230426",
	.major = 0,
	.minor = 0,
};

static const struct linlondp_dev_funcs dp_chip_funcs = {
	.init_format_table = dp_init_fmt_tbl,
	.enum_resources = dp_enum_resources,
	.cleanup = dp_cleanup,
	.irq_handler = dp_irq_handler,
	.enable_irq = dp_enable_irq,
	.disable_irq = dp_disable_irq,
	.on_off_vblank = dp_on_off_vblank,
	.change_opmode = dp_change_opmode,
	.flush = dp_flush,
	.connect_iommu = dp_connect_iommu,
	.disconnect_iommu = dp_disconnect_iommu,
	.init_hw = dp_init_hw,
	.dump_register = dp_dump,
	.close_gop = dp_close_gop,
	.gop_mode_changed = dp_gop_mode_changed,
	.dpu_reset = dpu_sw_reset,
};

linlondp_platform_probe(pdev)
	for_each_available_child_of_node(dev->of_node, of_child)
		linlondp_add_slave(dev, &match, of_child, LINLONDP_OF_PORT_OUTPUT, 0);
			remote = of_graph_get_remote_node(np, port, endpoint);
			drm_of_component_match_add(master, match/matchptr, compare_of/compare, remote/node);
		linlondp_add_slave(dev, &match, of_child, LINLONDP_OF_PORT_OUTPUT, 1);
	component_master_add_with_match(dev, &linlondp_master_ops, match);

linlondp_bind(dev) //simplified
	//init
	mdrv = devm_kzalloc(dev, sizeof(*mdrv), GFP_KERNEL);
	mdrv->mdev = linlondp_dev_create(dev);
	pipe->funcs = dp_pipeline_funcs;
	mdev->pipelines[mdev->n_pipelines] = pipe;
	mdrv->kms = devm_drm_dev_alloc(mdev->dev, &linlondp_kms_driver, struct linlondp_kms_dev, base);
	drm = &kms->base;
	drm->dev_private = mdev;
	crtc = &kms->crtcs[kms->n_crtcs];
	crtc->slave = linlondp_pipeline_get_slave(crtc->master);
	//mode config
	struct drm_mode_config *config = &kms->base.mode_config;
	config->funcs = &linlondp_mode_config_funcs;
	config->helper_private = &linlondp_mode_config_helpers;
	//planes for each
	plane->helper_private = linlondp_plane_helper_funcs;
	//crtcs for each
	crtc->funcs = linlondp_crtc_funcs;
	crtc->helper_private = linlondp_crtc_helper_funcs;
	//connectors
	wb_connector->encoder->helper_private = linlondp_wb_encoder_helper_funcs;
	wb_connector->encoder->funcs = drm_writeback_encoder_funcs;
	wb_connector->base->funcs = linlondp_wb_conn_helper_funcs; (connector->funcs)

linlondp_bind(dev)
	mdrv = devm_kzalloc(dev, sizeof(*mdrv), GFP_KERNEL);
	mdrv->mdev = linlondp_dev_create(dev);
		mdev = devm_kzalloc(dev, sizeof(*mdev), GFP_KERNEL);
		mdev->funcs = linlondp_identify(mdev->reg_base, &mdev->chip);
			mdev->funcs = &dp_chip_funcs;
		mdev->funcs->init_format_table(mdev);
		mdev->funcs->enum_resources(mdev);
			dp_enum_resources(mdev); //TODO:
				/*
				 * gcu_addr, periph_addr, num_blocks, num_pipelines ...
				 * GLB_CORE_INFO, GLB_BLOCK_INFO, ...
				 */
				...
				for (i = 0; i < dp->num_pipelines; i++) {
					pipe = linlondp_pipeline_add(mdev, sizeof(struct dp_pipeline), &dp_pipeline_funcs);
						pipe = devm_kzalloc(mdev->dev, size, GFP_KERNEL);
						pipe->mdev = mdev;
						pipe->id = mdev->n_pipelines;
						pipe->funcs = funcs; //dp_pipeline_funcs
						mdev->pipelines[mdev->n_pipelines] = pipe;
						mdev->n_pipelines++;
				...
		linlondp_parse_acpi(dev, mdev);/linlondp_parse_dt(dev, mdev);
		linlondp_assemble_pipelines(mdev);
		mdev->funcs->init_hw(mdev); //if (mdev->funcs->init_hw)
	if (!pm_runtime_enabled(dev))
		linlondp_dev_resume(mdrv->mdev);
			clk_prepare_enable(mdev->aclk);
			mdev->funcs->enable_irq(mdev);
			mdev->funcs->connect_iommu(mdev); //if mdev->iommu
			mdev->funcs->init_hw(mdev);
	mdrv->kms = linlondp_kms_attach(mdrv->mdev);
		if(enable_render)
			linlondp_kms_driver.driver_features |= DRIVER_RENDER;
		kms = devm_drm_dev_alloc(mdev->dev, &linlondp_kms_driver, struct linlondp_kms_dev, base);
		drm = &kms->base;
		drm->dev_private = mdev;
		linlondp_kms_mode_config_init(kms, mdev);
			struct drm_mode_config *config = &kms->base.mode_config;
			drm_mode_config_init(&kms->base);
			linlondp_kms_setup_crtcs(kms, mdev);
				for (i = 0; i < mdev->n_pipelines; i++) {
					crtc = &kms->crtcs[kms->n_crtcs];
					crtc->master = mdev->pipelines[i];
					crtc->slave = linlondp_pipeline_get_slave(crtc->master);
					crtc->side_by_side = mdev->side_by_side;
					kms->n_crtcs++;
				}
			config->funcs = &linlondp_mode_config_funcs;
			config->helper_private = &linlondp_mode_config_helpers;
		linlondp_kms_add_private_objs(kms, mdev);
		linlondp_kms_add_planes(kms, mdev);
			for (i = 0; i < mdev->n_pipelines; i++)
				pipe = mdev->pipelines[i];
				for (j = 0; j < pipe->n_layers; j++)
					linlondp_plane_add(kms, pipe->layers[j]);
						kplane = kzalloc(sizeof(*kplane), GFP_KERNEL);
						plane = &kplane->base;
						drm_universal_plane_init(&kms->base, plane, ..., &linlondp_plane_funcs, formats,...);
						drm_plane_helper_add(plane, &linlondp_plane_helper_funcs);
						... //plane properties
		drm_vblank_init(drm, kms->n_crtcs/num_crtcs);
			dev->vblank = drmm_kcalloc(dev, num_crtcs, sizeof(*dev->vblank), GFP_KERNEL);
			dev->num_crtcs = num_crtcs;
			for (i = 0; i < num_crtcs; i++) {
				struct drm_vblank_crtc *vblank = &dev->vblank[i];
				vblank->dev = dev;
				vblank->pipe = i;
				timer_setup(&vblank->disable_timer, vblank_disable_fn, 0);
				drmm_add_action_or_reset(dev, drm_vblank_init_release, vblank);
				drm_vblank_worker_init(vblank);
					vblank->worker = kthread_create_worker(0, "card%d-crtc%d", vblank->dev->primary->index, vblank->pipe);
			}
		linlondp_kms_add_crtcs(kms, mdev);
			for (i = 0; i < kms->n_crtcs; i++) {
				linlondp_crtc_add(kms, &kms->crtcs[i]);
					drm_crtc_init_with_planes(&kms->base, crtc, get_crtc_primary(kms, kcrtc), NULL, &linlondp_crtc_funcs, NULL);
					drm_crtc_helper_add(crtc, &linlondp_crtc_helper_funcs);
					err = drm_self_refresh_helper_init(crtc);
					drm_crtc_enable_color_mgmt(crtc, LINLONDP_COLOR_LUT_SIZE, true, LINLONDP_COLOR_LUT_SIZE);
					linlondp_crtc_create_ctm_ext_property(kcrtc);
					linlondp_crtc_create_protected_mode_property(kcrtc);
		linlondp_kms_add_wb_connectors(kms, mdev);
			for (i = 0; i < kms->n_crtcs; i++) {
				linlondp_wb_connector_add(kms, &kms->crtcs[i]);
					struct linlondp_wb_connector *kwb_conn;
					struct drm_writeback_connector *wb_conn;
					kwb_conn = kzalloc(sizeof(*kwb_conn), GFP_KERNEL);
					kwb_conn->wb_layer = kcrtc->master->wb_layer;
					kwb_conn->force_scaling_split = false;
					kwb_conn->expected_eow = BIT(kcrtc->master->id);
					if (kcrtc->side_by_side && kcrtc->slave)
						kwb_conn->expected_eow |= BIT(kcrtc->slave->id);
					wb_conn = &kwb_conn->base;
					formats = linlondp_get_layer_fourcc_list( &mdev->fmt_tbl, kwb_conn->wb_layer->layer_type, &n_formats);
					drm_writeback_connector_init(&kms->base, wb_conn, &linlondp_wb_connector_funcs, &linlondp_wb_encoder_helper_funcs, formats, n_formats, BIT(drm_crtc_index(&kcrtc->base)));
					linlondp_put_fourcc_list(formats);
					drm_connector_helper_add(&wb_conn->base, &linlondp_wb_conn_helper_funcs);
					kcrtc->wb_conn = kwb_conn;
					linlondp_wb_connector_create_color_prop(kwb_conn);
		component_bind_all(mdev->dev, kms);
		drm_mode_config_reset(drm);
		devm_request_irq(drm->dev, mdev->irq, linlondp_kms_irq_handler, IRQF_SHARED, dev_name(drm->dev), drm);
		drm_kms_helper_poll_init(drm);
		drm_dev_register(drm, 0);
		return kms;
	dev_set_drvdata(dev, mdrv);
	if (enable_fb)
		drm_fbdev_generic_setup(&mdrv->kms->base, 32);
	if (mdrv->mdev->enabled_by_gop)
		pm_runtime_set_active(dev);
	return 0;

linlondp_kms_irq_handler(irq, data)
	struct drm_device *drm = data;
	struct linlondp_dev *mdev = drm->dev_private;
	struct linlondp_kms_dev *kms = to_kdev(drm);
	struct linlondp_events evts;
	mdev->funcs->irq_handler(mdev, &evts);
	linlondp_print_events(&evts, drm);
	for (i = 0; i < kms->n_crtcs; i++)
		linlondp_crtc_handle_event(&kms->crtcs[i], &evts);




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


int linlondp_gem_dma_dumb_create(struct drm_file *file,
				struct drm_device *dev,
				struct drm_mode_create_dumb *args)
->drm_gem_dma_dumb_create_internal(file, dev, args);
-->drm_gem_dma_create_with_handle(file_priv, drm, args->size, &args->handle);
	dma_obj = drm_gem_dma_create(drm, size);
		dma_obj = __drm_gem_dma_create(drm, size, false);
			dma_obj->vaddr = dma_alloc_noncoherent/dma_alloc_wc(drm->dev, size, &dma_obj->dma_addr, ...); //if (dma_obj->map_noncoherent) or not
			gem_obj->funcs = &drm_gem_dma_default_funcs; //if (!gem_obj->funcs)
			if (private) {
				drm_gem_private_object_init(drm, gem_obj, size);
				dma_obj->map_noncoherent = false;
			} else
				ret = drm_gem_object_init(drm, gem_obj, size);
			drm_gem_create_mmap_offset(gem_obj);
			return dma_obj;
	gem_obj = &dma_obj->base;
	drm_gem_handle_create(file_priv, gem_obj, handle);
	drm_gem_object_put(gem_obj);















#DP
trilin_dptx_cix_probe(struct platform_device *pdev)
	return component_add(&pdev->dev, &trilin_dptx_cix_ops);

static const struct component_ops trilin_dptx_cix_ops = {
	.bind = trilin_dptx_cix_bind,
	.unbind = trilin_dptx_cix_unbind,
};


trilin_dptx_cix_bind(struct device *comp, struct device *master, void *master_data)
	struct trilin_dptx_cix_dev *cix_dptx = dev_get_drvdata(comp);
	cix_dptx->dev = comp;
	encoder = &cix_dptx->encoder;
	encoder->possible_crtcs = drm_acpi_find_possible_crtcs(drm, np);/drm_of_find_possible_crtcs(drm, np);
	match = acpi_device_get_match_data(comp); /of_match_node(trilin_dptx_dt_ids, np);
	dpsub = &cix_dptx->dpsub;
	dpsub->dev = comp;
	trilin_dp_probe(dpsub, drm);
		fwnode_edp_panel = fwnode_find_reference(dev->fwnode, "edp-panel", 0);
		dp = devm_kzalloc(dpsub->dev, sizeof(*dp), GFP_KERNEL);
		dp->dev = dev;
		dp->dpsub = dpsub;
		... //DT/ACPI resources
		dp->irq = platform_get_irq(pdev, 0);
		dptx_register_audio_device(dp);
	trilin_dp_drm_init(dpsub);
		struct trilin_dp *dp = dpsub->dp;
		struct trilin_encoder *enc = &dp->encoder;
		struct trilin_connector *conn = &dp->connector;
		struct drm_encoder *encoder = &enc->base;
		struct drm_connector *connector = &conn->base;
		int ret;
		int drm_mode_connector = DRM_MODE_CONNECTOR_DisplayPort;
		enc->dp = dp;
		conn->dp = dp;
		conn->type = TRILIN_OUTPUT_DP;
		/* Create the DRM encoder and connector. */
		encoder->possible_crtcs = TRILIN_DPTX_POSSIBLE_CRTCS_SST;
		drm_encoder_init(dp->drm[0], encoder, &trilin_dp_enc_funcs, DRM_MODE_ENCODER_TMDS, NULL);
			drm_mode_object_add(dev, &encoder->base, DRM_MODE_OBJECT_ENCODER);
			encoder->dev = dev;
			encoder->encoder_type = encoder_type;
			encoder->funcs = funcs;
			list_add_tail(&encoder->head, &dev->mode_config.encoder_list);
			encoder->index = dev->mode_config.num_encoder++;
		drm_encoder_helper_add(encoder, &trilin_dp_encoder_helper_funcs); //encoder->helper_private = trilin_dp_encoder_helper_funcs;
		drm_connector_init(encoder->dev, connector, &trilin_dp_connector_funcs, drm_mode_connector);
			__drm_mode_object_add(dev, &connector->base, DRM_MODE_OBJECT_CONNECTOR, false, drm_connector_free);
			connector->base.properties = &connector->properties;
			connector->dev = dev;
			connector->funcs = funcs;
			/* connector index is used with 32bit bitmasks */
			connector->index = ida_alloc_max(&config->connector_ida, 31, GFP_KERNEL);
			connector->connector_type = connector_type;
			connector->connector_type_id = ida_alloc_min(connector_ida, 1, GFP_KERNEL);
			connector->name = kasprintf(GFP_KERNEL, "%s-%d", drm_connector_enum_list[connector_type].name, connector->connector_type_id);
			drm_connector_get_cmdline_mode(connector);
			list_add_tail(&connector->head, &config->connector_list);
			config->num_connector++;
			drm_connector_attach_edid_property(connector); //if xxx
			drm_object_attach_property(&connector->base, config->dpms_property, 0);
			drm_object_attach_property(&connector->base, config->link_status_property, 0);
			drm_object_attach_property(&connector->base, config->non_desktop_property, 0);
			drm_object_attach_property(&connector->base, config->tile_property, 0);
			drm_object_attach_property(&connector->base, config->prop_crtc_id, 0); //if (drm_core_check_feature(dev, DRIVER_ATOMIC)) {
		drm_connector_helper_add(connector, &trilin_dp_connector_helper_funcs); //connector->helper_private = trilin_dp_connector_helper_funcs;
		drm_connector_register(connector);
			connector->funcs->late_register(connector); //if (connector->funcs->late_register) {
			drm_mode_object_register(connector->dev, &connector->base);
			connector->registration_state = DRM_CONNECTOR_REGISTERED;
			if (connector->privacy_screen)
				drm_privacy_screen_register_notifier(connector->privacy_screen, &connector->privacy_screen_notifier);
			list_add_tail(&connector->global_connector_list_entry, &connector_list);
		drm_connector_attach_encoder(connector, encoder);
			connector->possible_encoders |= drm_encoder_mask(encoder);
		trilin_drm_mst_encoder_init(dp, connector->base.id);
			/* create fake encoders */
			for (i = 0; i < dp->max_mst_encoders; i++) {
				mst_encoder = &dp->mst.private_info.mst_encoders[i];
				encoder = &mst_encoder->base;
				mst_encoder->id = i;
				mst_encoder->dp = dp;
				//fixme...
				mst_encoder->drm = dp->drm[0];
				if (dp->drm[1] != NULL && i >= 2) //fixme
					mst_encoder->drm = dp->drm[1];
				encoder->possible_crtcs = TRILIN_DPTX_POSSIBLE_CRTCS_MST; //1 << i; // crtc0 only for encoder0
				drm_encoder_init(mst_encoder->drm, encoder, &trilin_dp_mst_enc_funcs, DRM_MODE_ENCODER_DPMST, "DP-MST %c", pipe_name(i));
				drm_encoder_helper_add(encoder, &trilin_dp_encoder_helper_funcs);
			}
			drm_dp_mst_topology_mgr_init(&dp->mst.private_info.mst_mgr, dp->drm[0], &dp->aux, 16, dp->max_mst_encoders, conn_base_id);
			dp->mst_mgr = &dp->mst.private_info.mst_mgr;
		dp->dp_panel.connector = &dp->connector;
		dp->dp_panel.stream_id = 0;
		dp->connector.dp_panel = &dp->dp_panel;
		/* Some of the properties below require access to state, like bpc. */
		drm_atomic_helper_connector_reset(connector);
		trilin_dp_add_properties(dp, connector);
			if (!drm_mode_create_dp_colorspace_property(connector, 0))
				drm_connector_attach_colorspace_property(connector);
			drm_connector_attach_hdr_output_metadata_property(connector);
			drm_connector_attach_max_bpc_property(connector, 8, 10); //Fixme: max 10 for dpu, but not dp.
			drm_connector_attach_dp_subconnector_property(connector);
			drm_connector_attach_content_type_property(connector);
			drm_connector_attach_vrr_capable_property(connector);
		//dp hardware init now
		trilin_dp_init_config(dp);
			INIT_DELAYED_WORK(&dp->hpd_event_work, trilin_dp_hpd_event_work_func);
			INIT_DELAYED_WORK(&dp->hpd_irq_work, trilin_dp_hpd_irq_work_func);
			trilin_dp_aux_register(dp);
				dp->aux.dev = dp->dev;
				dp->aux.drm_dev = dp->drm[0];
				dp->aux.transfer = trilin_dp_aux_transfer;
				drm_dp_aux_register(&dp->aux);
					drm_dp_aux_register_devnode(aux);
					i2c_add_adapter(&aux->ddc);
			trilin_dp_hdcp_init(dp->dpsub);
			devm_request_threaded_irq(dp->dev, dp->irq, NULL, trilin_dp_irq_handler, IRQF_ONESHOT, dev_name(dp->dev), dp);

trilin_dp_irq_handler(int irq, void *data)
	struct trilin_dp *dp = (struct trilin_dp *)data;
	status = trilin_dp_read(dp, TRILIN_DPTX_INTERRUPT_CAUSE);
	mask = trilin_dp_read(dp, TRILIN_DPTX_INTERRUPT_MASK);
	if (!(status & ~mask))
		return IRQ_NONE;
	if (status & TRILIN_DPTX_INTERRUPT_HDCP_TIMER_IRQ)
		dev_dbg_ratelimited(dp->dev, "hdcp tmr\n");
		cix_hdcp_timer_process(&dp->hdcp);
	if (status & TRILIN_DPTX_INTERRUPT_GP_TIMER_IRQ) dev_dbg_ratelimited(dp->dev, "gp tmr\n");
	if (status & TRILIN_DPTX_INTERRUPT_REPLY_TIMEOUT) dev_dbg_ratelimited(dp->dev, "reply timeout\n");
	if (status & TRILIN_DPTX_INTERRUPT_REPLY_RECIEVED) dev_dbg_ratelimited(dp->dev, "reply received\n");
	if (status & TRILIN_DPTX_INTERRUPT_HPD_EVENT)
		schedule_delayed_work(&dp->hpd_event_work, msecs_to_jiffies(delay));
	if (status & TRILIN_DPTX_INTERRUPT_HPD_IRQ)
		schedule_delayed_work(&dp->hpd_irq_work, 0);
	return IRQ_HANDLED;


static void trilin_dp_hpd_event_work_func(struct work_struct *work)
{
	struct trilin_dp *dp;
	struct dptx_audio *dp_audio;
	struct trilin_phy_t *phy;
	int try;
	bool connected;
	enum drm_connector_status old_status;

	dp = container_of(work, struct trilin_dp, hpd_event_work.work);
	phy = &dp->phy;
	dp_audio = &dp->dp_audio;

	if (trilin_dp_get_hpd_state(dp) && !IS_ERR_OR_NULL(phy->base)) {
		volatile enum phy_mode mode;

		for (try = 0; try < DP_HPD_MAX_TRIES; try++) {
			mode = phy_get_mode(phy->base);
			if (mode == PHY_MODE_DP)
				break;
			DP_INFO("mode is not PHY_MODE_DP\n");
			msleep(100);
		}
		if (try >= DP_HPD_MAX_TRIES)
			DP_ERR("Wait too long for phy ready!\n");
	}

	/* add force to detect to sync call detect. */
	old_status = dp->status;
	drm_helper_probe_detect(&dp->connector.base, NULL, false);

	if(old_status == dp->status) {
		DP_INFO("dp status is same : %d", dp->status);
		return;
	}
	connected = (dp->status == connector_status_connected);

	if (connected)
		DP_INFO("dp hpd event received: Plugged\n");
	else
		DP_INFO("dp hpd event received: Unplugged\n");

	if (dp->drm[0])
		drm_helper_hpd_irq_event(dp->drm[0]);

	DP_DEBUG("dp audio plugin status = %d\n", connected);
	dptx_audio_handle_plugged_change(dp_audio, connected);

	cix_hdcp_hpd_event_process(&dp->hdcp, connected);
}

/* hdp irq handle other event */
static void trilin_dp_hpd_irq_work_func(struct work_struct *work)
{
	struct trilin_dp *dp;

	dp = container_of(work, struct trilin_dp, hpd_irq_work.work);
	DP_DEBUG("enter\n");

	if (!trilin_dp_get_hpd_state(dp)) {
		DP_DEBUG("hpd_high off, should update mst state\n");
		return;
	}

	mutex_lock(&dp->session_lock);
	if (!(dp->state & DPTX_STATE_INITIALIZED)) {
		mutex_unlock(&dp->session_lock);
		goto mst_attention;
	}

	mutex_unlock(&dp->session_lock);
	trilin_dp_link_process_request(dp);

	if (dp->link_request & DP_LINK_STATUS_UPDATED) {
		mutex_lock(&dp->session_lock);
		if (dp->state & DPTX_STATE_ENABLED)
			trilin_dp_train_loop(dp);
		mutex_unlock(&dp->session_lock);
	}
	trilin_dp_link_hdcp_request(dp);
mst_attention:
	trilin_dp_mst_display_hpd_irq(dp);
}




trilin_dptx_cix_bind
	struct drm_encoder *encoder = cix_dptx->dpsub->dp->encoder->base;
	struct drm_connector *connector = cix_dptx->dpsub->dp->connector->base;
	encoder->funcs = trilin_dp_enc_funcs;
	encoder->helper_private = trilin_dp_encoder_helper_funcs;
	connector->funcs = trilin_dp_connector_funcs;
	connector->helper_private = trilin_dp_connector_helper_funcs;
	drm_connector_register(connector);
	drm_connector_attach_encoder(connector, encoder);

