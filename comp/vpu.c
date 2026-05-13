

#mvx
## init
// no power, clk, freq
mvx_pdev_probe(struct platform_device *pdev)
	mvx_dev_probe(&pdev->dev, rcsu_res, res, irq);
		ctx = devm_kzalloc(dev, sizeof(struct mvx_dev_ctx *), GFP_KERNEL);
		ctx->client_ops.xxx = yyy
		mvx_if_create(dev, &ctx->client_ops, ctx);
		mvx_hwreg_construct(&ctx->hwreg, dev, rcsu_res, res, ctx->dentry);
		ctx->irq = irq;
		irq_set_status_flags(ctx->irq, IRQ_DISABLE_UNLAZY);
		request_irq(ctx->irq, irq_top, IRQF_SHARED, dev_name(dev), ctx);
		mvx_sched_construct(&ctx->scheduler, dev, ctx->if_ops, &ctx->hwreg, ctx->dentry);
		INIT_WORK(&ctx->work, irq_bottom);

mvx_if_create(dev, &ctx->client_ops, ctx);
	dev->dma_mask = &mvx_if_dma_mask;
	dev->coherent_dma_mask = mvx_if_dma_mask;
	dev->dma_parms = devm_kzalloc(dev, sizeof(*dev->dma_parms), GFP_KERNEL);
	dma_set_max_seg_size(dev, SZ_2G);
	ctx = devm_kzalloc(dev, sizeof(struct mvx_if_ctx *), GFP_KERNEL);
	debugfs_create_dir("amvx_if", NULL);
	ctx->dev = dev;
	ctx->client_ops = client_ops;
	ctx->if_ops.irq = mvx_session_irq;
	kobject_init_and_add(&ctx->kobj, &if_ktype, kernel_kobj, "amvx%u", dev->id);
	mvx_secure_construct(&ctx->secure, dev);
		secure->dev = dev;
		secure->kset = kset_create_and_add("securevideo", NULL, &dev->kobj);
		secure->workqueue = alloc_workqueue("mvx_securevideo", WQ_UNBOUND | WQ_MEM_RECLAIM, 1);
	mvx_fw_cache_construct(&ctx->firmware, dev, &ctx->secure, &ctx->kobj);
		cache->dev = dev;
		cache->secure = secure;
		cache->kobj_parent = kobject_get(kobj_parent);
		kobject_init_and_add(&cache->kobj, &cache_ktype, kobj_parent, "fw_cache");
		cache->cache_thread = kthread_run(cache_thread, cache, "fw_cache");
	mvx_ext_if_construct(ctx->ext, dev, &ctx->firmware, ctx->client_ops, ctx->dentry);
		ext[MVX_EXT_IF_DECODER].dev = dev;
		ext[MVX_EXT_IF_DECODER].cache = cache;
		ext[MVX_EXT_IF_DECODER].client_ops = client_ops;
		ext[MVX_EXT_IF_ENCODER].dev = dev;
		ext[MVX_EXT_IF_ENCODER].cache = cache;
		ext[MVX_EXT_IF_ENCODER].client_ops = client_ops;
		dsessions = debugfs_create_dir("session", parent);
		ext[MVX_EXT_IF_DECODER].dsessions = dsessions;
		ext[MVX_EXT_IF_ENCODER].dsessions = dsessions;
		v4l2_dev = devm_kzalloc(dev, sizeof(*v4l2_dev), GFP_KERNEL);
		v4l2_device_register(dev, v4l2_dev);
		ext[MVX_EXT_IF_DECODER].v4l2_dev = v4l2_dev;
		ext[MVX_EXT_IF_ENCODER].v4l2_dev = v4l2_dev;
		mvx_ext_if_register_device(&ext[MVX_EXT_IF_DECODER], "mvxdec", 0/is_encoder);
		mvx_ext_if_register_device(&ext[MVX_EXT_IF_ENCODER], "mvxenc", 1/is_encoder);
			ext->is_encoder = is_encoder;
			ext->vdev.fops = &mvx_v4l2_fops;
			ext->vdev.ioctl_ops = &mvx_v4l2_ioctl_ops;
			ext->vdev.release = video_device_release_empty;
			ext->vdev.vfl_dir = VFL_DIR_M2M;
			ext->vdev.v4l2_dev = ext->v4l2_dev;
			ext->vdev.device_caps = V4L2_CAP_VIDEO_M2M | V4L2_CAP_VIDEO_M2M_MPLANE | V4L2_CAP_EXT_PIX_FORMAT | V4L2_CAP_STREAMING;
			video_set_drvdata(&ext->vdev, ext);
			video_register_device(&ext->vdev, VFL_TYPE_VIDEO, -1);

mvx_hwreg_construct(&ctx->hwreg, dev, rcsu_res, res, ctx->dentry);
	hwreg->dev = dev;
	hwreg->rcsu_res = request_mem_region(rcsu_res->start, resource_size(rcsu_res), name);
	hwreg->rcsu_registers = ioremap(rcsu_res->start, resource_size(rcsu_res));
	hwreg->res = request_mem_region(res->start, resource_size(res), name);
	hwreg->registers = ioremap(res->start, resource_size(res));
	for (lsid = 0; lsid < MVX_LSID_MAX; ++lsid)
		hwreg->lsid_hwreg[lsid].hwreg = hwreg;
		hwreg->lsid_hwreg[lsid].lsid = lsid;
	debugfs_init(hwreg, parent);

mvx_sched_construct(&ctx->scheduler, dev, ctx->if_ops, &ctx->hwreg, ctx->dentry);
	sched->dev = dev;
	sched->hwreg = hwreg;
	sched->if_ops = if_ops;
	sched->state = MVX_SCHED_STATE_IDLE;
	...
	sched->sched_queue = create_singlethread_workqueue("mvx_sched");
	sched->nlsid = mvx_hwreg_get_nlsid(hwreg); //hwreg->nlsid;
	for (lsid = 0; lsid < sched->nlsid; lsid++)
		mvx_lsid_construct(&sched->lsid[lsid], dev, hwreg, lsid);
			lsid->dev = dev;
			lsid->hwreg = hwreg;
			lsid->session = NULL;
			lsid->lsid = id;
		sched_debugfs_init(sched, parent);

irqreturn_t irq_top(int irq, void *dev_id)
	struct mvx_dev_ctx *ctx = dev_id;
	nlsid = mvx_hwreg_get_nlsid(&ctx->hwreg);
	irqve = mvx_hwreg_read(&ctx->hwreg, MVX_HWREG_IRQVE);
	while (nlsid-- > 0)
		if ((irqve >> nlsid) & 0x1) {
			mvx_hwreg_write_lsid(&ctx->hwreg, nlsid, MVX_HWREG_LIRQVE);
			mb();
			set_bit(nlsid, &ctx->irqve);
			ret = IRQ_HANDLED;
		}
	queue_work(ctx->work_queue, &ctx->work);

irq_bottom(struct work_struct *work)
	nlsid = mvx_hwreg_get_nlsid(&ctx->hwreg);
	for (i = 0; i < nlsid; i++)
		if (test_and_clear_bit(i, &ctx->irqve))
			mvx_sched_handle_irq(&ctx->scheduler, i);
				session = sched->lsid[lsid].session
				if (kref_get_unless_zero(&session->isession->kref) != 0) //if (session)
					isession = session->isession;
				sched->if_ops->irq(isession);
				queue_work(sched->sched_queue, &sched->sched_task);

//sched->if_ops->irq
void mvx_session_irq(struct mvx_if_session *isession)
	struct mvx_session *session = mvx_if_session_to_session(isession);
	if (is_fw_loaded(session) == false)
		return;
	session->fw.ops.handle_rpc(&session->fw);
	do {
		watchdog_update(session, timeout_ms);
		session->fw.ops.get_message(&session->fw, &msg);
		handle_fw_message(session, &msg);
			for (i = 0; i < ARRAY_SIZE(handlers); i++)
				if (handlers[i].code == msg->code)
					handler = &handlers[i]; // real handle messages
					break;
			handler->done(session, msg); //if (handler && handler->done)
	} while (ret > 0 && session->error == 0);

void sched_task(struct work_struct *ws)
	if (sched->state == MVX_SCHED_STATE_IDLE) {
		if (list_empty_careful(&sched->pending) == false)
			set_sched_state(sched, MVX_SCHED_STATE_RUNNING);
	} else if (sched->state == MVX_SCHED_STATE_SUSPEND) {
		return;
	}
	list_for_each_entry_safe(pending, tmp, &sched->pending, pending) {
		if (pending->lsid != NULL) {
			mvx_lsid_jobqueue_add(pending->lsid, pending->isession->ncores, sort_jobs);
			pending->in_pending = false;
			list_del(&pending->pending);
		}
		lsid = find_free_lsid(sched);
		lsid = find_idle_lsid(sched); //or
		if (lsid->session != NULL) {
			struct mvx_sched_session *unmapped = lsid->session;
			unmap_session(sched, unmapped);
			ret = kref_get_unless_zero(&unmapped->isession->kref);
			if (ret != 0) {
				if (list_find_node(&notify_list, &unmapped->notify))
					kref_put(&unmapped->isession->kref, unmapped->isession->release);
				else
					list_add_tail(&unmapped->notify, &notify_list);
			}
		}
		map_session(sched, pending, lsid);
		mvx_lsid_jobqueue_add(lsid, pending->isession->ncores, sort_jobs);
		pending->in_pending = false;
		list_del(&pending->pending);
	}
	list_for_each_entry_safe(unmapped, tmp, &notify_list, notify) {
		struct mvx_if_session *iunmapped = unmapped->isession;
		list_del(&unmapped->notify);
		sched->if_ops->irq(iunmapped);
		kref_put(&iunmapped->kref, iunmapped->release);
	}



static struct mvx_fw_msg_handler handlers[] = {
    {MVX_FW_CODE_ALLOC_PARAM,     mvx_handle_alloc_param},
    {MVX_FW_CODE_BUFFER_GENERAL,  mvx_handle_buffer_general},
    {MVX_FW_CODE_BUFFER,          mvx_handle_buffer},
    {MVX_FW_CODE_DISPLAY_SIZE,    mvx_handle_display_size},
    {MVX_FW_CODE_COLOR_DESC,      mvx_handle_color_desc},
    {MVX_FW_CODE_ERROR,           mvx_handle_error},
    {MVX_FW_CODE_FLUSH,           mvx_handle_flush},
    {MVX_FW_CODE_IDLE,            mvx_handle_idle},
    {MVX_FW_CODE_JOB,             mvx_handle_job},
    {MVX_FW_CODE_PONG,            mvx_handle_pong},
    {MVX_FW_CODE_SEQ_PARAM,       mvx_handle_seq_param},
    {MVX_FW_CODE_SET_OPTION,      mvx_handle_set_option},
    {MVX_FW_CODE_STATE_CHANGE,    mvx_handle_state_change},
    {MVX_FW_CODE_SWITCH_IN,       mvx_handle_switch_in},
    {MVX_FW_CODE_SWITCH_OUT,      mvx_handle_switch_out},
    {MVX_FW_CODE_DUMP,            mvx_handle_dump},
    {MVX_FW_CODE_DEBUG,           mvx_handle_debug},
    {MVX_FW_CODE_UNKNOWN,         mvx_handle_unknown},
};


static const struct v4l2_file_operations mvx_v4l2_fops = {
    .owner          = THIS_MODULE,
    .open           = mvx_v4l2_open,
    .release        = mvx_v4l2_release,
    .poll           = mvx_v4l2_poll,
    .unlocked_ioctl = video_ioctl2,
    .mmap           = mvx_v4l2_mmap
};

static const struct v4l2_ioctl_ops mvx_v4l2_ioctl_ops = {
    .vidioc_querycap                = mvx_v4l2_vidioc_querycap,
    .vidioc_enum_fmt_vid_cap        = mvx_v4l2_vidioc_enum_fmt_vid_cap,
    .vidioc_enum_fmt_vid_out        = mvx_v4l2_vidioc_enum_fmt_vid_out,
    .vidioc_enum_framesizes         = mvx_v4l2_vidioc_enum_framesizes,
    .vidioc_g_fmt_vid_cap           = mvx_v4l2_vidioc_g_fmt_vid_cap,
    .vidioc_g_fmt_vid_cap_mplane    = mvx_v4l2_vidioc_g_fmt_vid_cap,
    .vidioc_g_fmt_vid_out           = mvx_v4l2_vidioc_g_fmt_vid_out,
    .vidioc_g_fmt_vid_out_mplane    = mvx_v4l2_vidioc_g_fmt_vid_out,
    .vidioc_s_fmt_vid_cap           = mvx_v4l2_vidioc_s_fmt_vid_cap,
    .vidioc_s_fmt_vid_cap_mplane    = mvx_v4l2_vidioc_s_fmt_vid_cap,
    .vidioc_s_fmt_vid_out           = mvx_v4l2_vidioc_s_fmt_vid_out,
    .vidioc_s_fmt_vid_out_mplane    = mvx_v4l2_vidioc_s_fmt_vid_out,
    .vidioc_try_fmt_vid_cap         = mvx_v4l2_vidioc_try_fmt_vid_cap,
    .vidioc_try_fmt_vid_cap_mplane  = mvx_v4l2_vidioc_try_fmt_vid_cap,
    .vidioc_try_fmt_vid_out         = mvx_v4l2_vidioc_try_fmt_vid_out,
    .vidioc_try_fmt_vid_out_mplane  = mvx_v4l2_vidioc_try_fmt_vid_out,
    .vidioc_g_selection             = mvx_v4l2_vidioc_g_selection,
    .vidioc_s_selection             = mvx_v4l2_vidioc_s_selection,
    .vidioc_g_parm                  = mvx_v4l2_vidioc_g_parm,
    .vidioc_s_parm                  = mvx_v4l2_vidioc_s_parm,
    .vidioc_streamon                = mvx_v4l2_vidioc_streamon,
    .vidioc_streamoff               = mvx_v4l2_vidioc_streamoff,
    .vidioc_encoder_cmd             = mvx_v4l2_vidioc_encoder_cmd,
    .vidioc_try_encoder_cmd         = mvx_v4l2_vidioc_try_encoder_cmd,
    .vidioc_decoder_cmd             = mvx_v4l2_vidioc_decoder_cmd,
    .vidioc_try_decoder_cmd         = mvx_v4l2_vidioc_try_decoder_cmd,
    .vidioc_reqbufs                 = mvx_v4l2_vidioc_reqbufs,
    .vidioc_create_bufs             = mvx_v4l2_vidioc_create_bufs,
    .vidioc_querybuf                = mvx_v4l2_vidioc_querybuf,
    .vidioc_qbuf                    = mvx_v4l2_vidioc_qbuf,
    .vidioc_dqbuf                   = mvx_v4l2_vidioc_dqbuf,
    .vidioc_expbuf                  = mvx_v4l2_vidioc_expbuf,
    .vidioc_subscribe_event         = mvx_v4l2_vidioc_subscribe_event,
    .vidioc_unsubscribe_event       = v4l2_event_unsubscribe,
    .vidioc_default                 = mvx_v4l2_vidioc_default
};

const struct vb2_ops mvx_vb2_ops = {
    .queue_setup     = queue_setup,
    .buf_init        = buf_init,
    .buf_finish      = buf_finish,
    .buf_cleanup     = buf_cleanup,
    .start_streaming = start_streaming, mvx_session_streamon(&vsession->session, vport->dir);
    .stop_streaming  = stop_streaming,
    .buf_queue       = buf_queue,
    .wait_prepare    = wait_prepare,
    .wait_finish     = wait_finish
};

const struct vb2_mem_ops vb2_dma_sg_memops = {
	.alloc		= vb2_dma_sg_alloc,
	.put		= vb2_dma_sg_put,
	.get_userptr	= vb2_dma_sg_get_userptr,
	.put_userptr	= vb2_dma_sg_put_userptr,
	.prepare	= vb2_dma_sg_prepare,
	.finish		= vb2_dma_sg_finish,
	.vaddr		= vb2_dma_sg_vaddr,
	.mmap		= vb2_dma_sg_mmap,
	.num_users	= vb2_dma_sg_num_users,
	.get_dmabuf	= vb2_dma_sg_get_dmabuf,
	.map_dmabuf	= vb2_dma_sg_map_dmabuf,
	.unmap_dmabuf	= vb2_dma_sg_unmap_dmabuf,
	.attach_dmabuf	= vb2_dma_sg_attach_dmabuf,
	.detach_dmabuf	= vb2_dma_sg_detach_dmabuf,
	.cookie		= vb2_dma_sg_cookie,
};

static const struct vb2_buf_ops v4l2_buf_ops = {
	.verify_planes_array	= __verify_planes_array_core,
	.init_buffer		= __init_vb2_v4l2_buffer,
	.fill_user_buffer	= __fill_v4l2_buffer,
	.fill_vb2_buffer	= __fill_vb2_buffer,
	.copy_timestamp		= __copy_timestamp,
};



static int cache_thread(void *v)

/**
 * enum vb2_buffer_state - current video buffer state.
 * @VB2_BUF_STATE_DEQUEUED:	buffer under userspace control.
 * @VB2_BUF_STATE_IN_REQUEST:	buffer is queued in media request.
 * @VB2_BUF_STATE_PREPARING:	buffer is being prepared in videobuf2.
 * @VB2_BUF_STATE_QUEUED:	buffer queued in videobuf2, but not in driver.
 * @VB2_BUF_STATE_ACTIVE:	buffer queued in driver and possibly used
 *				in a hardware operation.
 * @VB2_BUF_STATE_DONE:		buffer returned from driver to videobuf2, but
 *				not yet dequeued to userspace.
 * @VB2_BUF_STATE_ERROR:	same as above, but the operation on the buffer
 *				has ended with an error, which will be reported
 *				to the userspace when it is dequeued.
 */
enum vb2_buffer_state {
	VB2_BUF_STATE_DEQUEUED,
	VB2_BUF_STATE_IN_REQUEST,
	VB2_BUF_STATE_PREPARING,
	VB2_BUF_STATE_QUEUED,
	VB2_BUF_STATE_ACTIVE,
	VB2_BUF_STATE_DONE,
	VB2_BUF_STATE_ERROR,
};

#v4l2
/* --- v4l2 flows --- */
int fd = open("/dev/video0", O_RDWR | O_NONBLOCK);
ioctl(fd, VIDIOC_S_FMT, &fmt);
ioctl(fd, VIDIOC_QUERYCAP, &cap);
ioctl(fd, VIDIOC_REQBUFS, &req); //alloc
for (int i = 0; i < req.count; i++)
	ioctl(fd, VIDIOC_QUERYBUF, &buf);
	buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
ioctl(fd, VIDIOC_QBUF, &buf);
ioctl(fd, VIDIOC_STREAMON, &type);
ioctl(fd, VIDIOC_DQBUF, &buf);
fwrite(buffers[buf.index].start, buf.bytesused, 1, fp);
ioctl(fd, VIDIOC_STREAMOFF, &type);

#
总结图示
[用户空间]                     [内核驱动]
   |                               |
   |-- open() -------------------->|
   |-- VIDIOC_S_FMT -------------->| (设置格式)
   |                               |
   |-- VIDIOC_REQBUFS ------------>| (申请内核buf)
   |-- VIDIOC_QUERYBUF ----------->| (查询buf地址)
   |-- mmap() ---------------------| (映射到用户空间)
   |                               |
   |-- VIDIOC_QBUF --------------->| (buf入队，等待填充)
   |                               |
   |-- VIDIOC_STREAMON ---------->| (开始采集)
   |                               |
   |<-- [数据采集中...] -----------|
   |                               |
   |-- VIDIOC_DQBUF -------------->| (取出已填满的buf)
   |    (处理数据...)              |
   |-- VIDIOC_QBUF --------------->| (放回buf继续循环)
   |                               |
   |-- VIDIOC_STREAMOFF ---------->| (停止)
   |-- close() -------------------->|

static const struct file_operations v4l2_fops = {
	.owner = THIS_MODULE,
	.read = v4l2_read,
	.write = v4l2_write,
	.open = v4l2_open,
	.get_unmapped_area = v4l2_get_unmapped_area,
	.mmap = v4l2_mmap,
	.unlocked_ioctl = v4l2_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = v4l2_compat_ioctl32,
#endif
	.release = v4l2_release,
	.poll = v4l2_poll,
	.llseek = no_llseek,
};

static const struct v4l2_file_operations mvx_v4l2_fops = {
    .owner          = THIS_MODULE,
    .open           = mvx_v4l2_open,
    .release        = mvx_v4l2_release,
    .poll           = mvx_v4l2_poll,
    .unlocked_ioctl = video_ioctl2,
    .mmap           = mvx_v4l2_mmap
};

int v4l2_open(struct inode *inode, struct file *filp)
if (vdev->fops->open && video_is_registered(vdev))
	ret = vdev->fops->open(filp);

long v4l2_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
	vdev->fops->unlocked_ioctl(filp, cmd, arg);

long video_ioctl2(struct file *file, unsigned int cmd, unsigned long arg)
	video_usercopy(file, cmd, arg, __video_do_ioctl);
		__video_do_ioctl(file, cmd, parg);
			info = &v4l2_ioctls[_IOC_NR(cmd)];
			info->func(ops, file, fh, arg);

__poll_t v4l2_poll(struct file *filp, struct poll_table_struct *poll)
	vdev->fops->poll(filp, poll);

### |-- open() -------------------->|
fd = open("/dev/video0", O_RDWR | O_NONBLOCK);

int mvx_v4l2_open(struct file *file) //TODO:
	struct mvx_v4l2_session *session;
	session = devm_kzalloc(ctx->dev, sizeof(*session), GFP_KERNEL);
	mvx_v4l2_session_construct(session, ctx);
		mvx_session_construct(&vsession->session, ctx->dev, ctx->client_ops, ctx->cache,
			                  &vsession->mutex, free_session, handle_event, vsession->dentry, ctx->is_encoder);
			...
			session->destructor = destructor; //free_session
			session->event = event; //handle_event
			...
	mvx_v4l2_vidioc_s_fmt_vid_cap(file, NULL, &fmt);
	if (ctx->is_encoder)
		mvx_v4l2_ctrls_init_enc(&session->v4l2_ctrl);
	else
		mvx_v4l2_ctrls_init_dec(&session->v4l2_ctrl);
	session->fh.ctrl_handler = &session->v4l2_ctrl;

### |-- VIDIOC_S_FMT -------------->|
ioctl(fd, VIDIOC_S_FMT, &fmt);

mvx_v4l2_vidioc_s_fmt_vid(file, f, dir)
	from_v4l2_format(vsession, f, &pix_mp, &format, stride, size, &interlaced);
	mvx_session_set_format(&vsession->session, dir, format, pix_mp.pixelformat, ...)
	mvx_v4l2_session_set_color_info(vsession, &pix_mp);
	/mvx_v4l2_copy_color_desc(&pix_mp, &vsession->port[MVX_DIR_INPUT].pix_mp);
	to_v4l2_format(f, f->type, &pix_mp, stride, size, interlaced);

### |-- VIDIOC_REQBUFS ------------>|
ioctl(fd, VIDIOC_REQBUFS, &req); //alloc

mvx_v4l2_vidioc_reqbufs(struct file *file, fh, b/req)
-> setup_vb2_queue(struct mvx_v4l2_port *vport)
	struct vb2_queue *q = &vport->vb2_queue;
	/* q->ops, q->mem_ops, q->buf_ops */
	q->ops = &mvx_vb2_ops;
	q->mem_ops = &vb2_dma_sg_memops;
	q->buf_struct_size = sizeof(struct mvx_v4l2_buffer);
	vb2_queue_init(q);
		vb2_queue_init_name(q, NULL);
			q->buf_ops = &v4l2_buf_ops;
-> vb2_reqbufs(&vport->vb2_queue, b/req);
--> vb2_core_reqbufs(q, req->memory, req->flags, &req->count);
---> call_qop(q, queue_setup, q, &num_buffers, &num_planes, plane_sizes, q->alloc_devs);
		vb2_ops->queue_setup(q, unused, buf_cnt, plane_cnt, plane_size, alloc_devs); //mvx_vb2_ops->mvx_v4l2_vidioc.c
---> allocated_buffers = __vb2_queue_alloc(q, memory, num_buffers, num_planes, plane_sizes);
		for (buffer = 0; buffer < num_buffers; ++buffer) {
			vb = kzalloc(q->buf_struct_size, GFP_KERNEL);
			vb->vb2_queue = q;
			vb->num_planes = num_planes;
			vb->index = q->num_buffers + buffer;
			...
			call_void_bufop(q, init_buffer, vb);
			q->bufs[vb->index] = vb;
			if (memory == VB2_MEMORY_MMAP) {
				__vb2_buf_mem_alloc(vb);
					for (plane = 0; plane < vb->num_planes; ++plane) {
						mem_priv = call_ptr_memop(alloc, vb, q->alloc_devs[plane] ? : q->dev, size);
							vb2_dma_sg_alloc(vb, q->dev, size);
								struct vb2_dma_sg_buf *buf;
								buf = kzalloc(sizeof *buf, GFP_KERNEL);
								buf->dma_dir = vb->vb2_queue->dma_dir;
								buf->size = size;
								buf->num_pages = size >> PAGE_SHIFT;
								buf->dma_sgt = &buf->sg_table;
								buf->pages = kvcalloc(buf->num_pages, sizeof(struct page *), GFP_KERNEL);
								vb2_dma_sg_alloc_compacted(buf, vb->vb2_queue->gfp_flags);
								sg_alloc_table_from_pages(buf->dma_sgt, buf->pages, buf->num_pages, 0, size, GFP_KERNEL);
								sgt = &buf->sg_table;
								dma_map_sgtable(buf->dev, sgt, buf->dma_dir, DMA_ATTR_SKIP_CPU_SYNC);
								buf->handler.refcount = &buf->refcount;
								buf->handler.put = vb2_dma_sg_put;
								buf->handler.arg = buf;
								buf->vb = vb;
								return buf;
						vb->planes[plane].mem_priv = mem_priv;
					}
				call_vb_qop(vb, buf_init, vb);
			}
		}
	q->num_buffers = allocated_buffers;
	*count = allocated_buffers;
result: fd-> vb2_queue-> n * mvx_v4l2_buffer[vb2_v4l2_buffer[vb2_buffer]

### |-- VIDIOC_QUERYBUF ----------->|
for (int i = 0; i < req.count; i++)
	ioctl(fd, VIDIOC_QUERYBUF, &buf);
	buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

mvx_v4l2_vidioc_querybuf(file, fh, b);
-> vb2_querybuf(&vport->vb2_queue, b);
--> vb2_core_querybuf(q, b->index, pb/b);
---> call_void_bufop(q, fill_user_buffer, q->bufs[index], pb);
----> __fill_v4l2_buffer(struct vb2_buffer *vb, void *pb)
		b->index = vb->index;
		b->type = vb->type;
		b->memory = vb->memory;
		b->flags = vbuf->flags;
		...
		if (q->is_multiplanar) {
			b->length = vb->num_planes;
			...
		} else {
			b->length = vb->planes[0].length;
			b->bytesused = vb->planes[0].bytesused;
			if (q->memory == VB2_MEMORY_MMAP)
				b->m.offset = vb->planes[0].m.offset;
			else if (q->memory == VB2_MEMORY_USERPTR)
				b->m.userptr = vb->planes[0].m.userptr;
			else if (q->memory == VB2_MEMORY_DMABUF)
				b->m.fd = vb->planes[0].m.fd;
		}
		...

v4l2_mmap(struct file *filp, struct vm_area_struct *vm)
-> vdev->fops->mmap(filp, vm);
--> mvx_v4l2_mmap(file, vma);
---> vb2_mmap(q, vma);
		ret = __find_plane_by_offset(q, off, &buffer, &plane);
		vb = q->bufs[buffer];
		call_memop(vb, mmap, vb->planes[plane].mem_priv, vma);
			vb2_dma_sg_mmap(buf, vma);
				vm_map_pages(vma, buf->pages, buf->num_pages);
				vma->vm_private_data	= &buf->handler;
				vma->vm_ops		= &vb2_common_vm_ops;
				vma->vm_ops->open(vma);


### |-- VIDIOC_QBUF --------------->|
ioctl(fd, VIDIOC_QBUF, &buf);
mvx_v4l2_vidioc_qbuf(file, fh, struct v4l2_buffer *b)
->...
-> vb2_qbuf(&vport->vb2_queue, NULL, b);
--> vb2_queue_or_prepare_buf(q, mdev, b, false, &req);
--> vb2_core_qbuf(q, b->index, b, req);
	if (req) {
		... // TODO:?
		return 0;
	}
	__buf_prepare(vb); //if (!vb->prepared) && vb->state = VB2_BUF_STATE_DEQUEUED or VB2_BUF_STATE_IN_REQUEST:
		switch (q->memory) {
		case VB2_MEMORY_MMAP: __prepare_mmap(vb); break;
			call_bufop(vb->vb2_queue, fill_vb2_buffer, vb, vb->planes);
			call_vb_qop(vb, buf_prepare, vb);
		case VB2_MEMORY_USERPTR: __prepare_userptr(vb); break;
			call_bufop(vb->vb2_queue, fill_vb2_buffer, vb, planes);
			for (plane = 0; plane < vb->num_planes; ++plane) {
				mem_priv = call_ptr_memop(get_userptr, vb,
							  q->alloc_devs[plane] ? : q->dev,
							  planes[plane].m.userptr,
							  planes[plane].length);
				vb->planes[plane].mem_priv = mem_priv;
			}
			call_vb_qop(vb, buf_prepare, vb);
		case VB2_MEMORY_DMABUF: __prepare_dmabuf(vb); break;
			call_bufop(vb->vb2_queue, fill_vb2_buffer, vb, planes);
			for (plane = 0; plane < vb->num_planes; ++plane) {
				struct dma_buf *dbuf = dma_buf_get(planes[plane].m.fd);
				mem_priv = call_ptr_memop(attach_dmabuf, vb,
							  q->alloc_devs[plane] ? : q->dev,
							  dbuf,
							  planes[plane].length);
				vb->planes[plane].dbuf = dbuf;
				vb->planes[plane].mem_priv = mem_priv;
			}
			for (plane = 0; plane < vb->num_planes; ++plane) {
				call_memop(vb, map_dmabuf, vb->planes[plane].mem_priv);
				vb->planes[plane].dbuf_mapped = 1;
			}
		}
		__vb2_buf_mem_prepare(vb);
	list_add_tail(&vb->queued_entry, &q->queued_list);
	q->queued_count++;
	q->waiting_for_buffers = false;
	vb->state = VB2_BUF_STATE_QUEUED;
	if (q->start_streaming_called)
		__enqueue_in_driver(vb);
			call_void_vb_qop(vb, buf_queue, vb);
				mvx_v4l2_buffer_set(vbuf, b);
				mvx_session_qbuf(&vsession->session, dir, &vbuf->buf);
					queue_buffer(session, dir, buf);
					switch_in(session);
	if (pb)
		call_void_bufop(q, fill_user_buffer, vb, pb);
	if (q->streaming && !q->start_streaming_called && q->queued_count >= q->min_buffers_needed)
		vb2_start_streaming(q);


### |-- VIDIOC_STREAMON --------------->|
ioctl(fd, VIDIOC_STREAMON, &type);
//not here

### |<-- [数据采集中...] -----------|
### |-- VIDIOC_DQBUF --------------->|
ioctl(fd, VIDIOC_DQBUF, &buf);
mvx_v4l2_vidioc_dqbuf(struct file *file, void *fh, struct v4l2_buffer *b)
	struct mvx_v4l2_session *vsession = file_to_session(file);
	struct mvx_v4l2_port *vport = &vsession->port[dir];
	vb2_dqbuf(&vport->vb2_queue/q, b, file->f_flags & O_NONBLOCK);
		vb2_core_dqbuf(q, NULL, b/pb, nonblocking);
			__vb2_get_done_vb(q, &vb, pb, nonblocking);
				__vb2_wait_for_done_vb(q, nonblocking);
					for (;;) {
						if (!list_empty(&q->done_list))
							break;
						call_void_qop(q, wait_prepare, q);
						wait_event_interruptible(q->done_wq, !list_empty(&q->done_list) || !q->streaming || q->error);
						call_void_qop(q, wait_finish, q);
					}
				*vb = list_first_entry(&q->done_list, struct vb2_buffer, done_entry);
				call_bufop(q, verify_planes_array, *vb, pb); // if (pb)
					v4l2_buf_ops->__verify_planes_array_core();
			call_void_vb_qop(vb, buf_finish, vb);
				mvx_vb2_ops->buf_finish;
			call_void_bufop(q, fill_user_buffer, vb, pb); // if (pb)
				v4l2_buf_ops->fill_user_buffer();
			list_del(&vb->queued_entry);
			__vb2_dqbuf(vb);
				call_void_bufop(q, init_buffer, vb);
					v4l2_buf_ops->init_buffer();
		b->flags &= ~V4L2_BUF_FLAG_DONE;
	... //b->bytesused, b->m.offset, b->m.userptr, b->reserved2, b->m.planes[i].reserved[0]
	v4l2_event_queue_fh(&vsession->fh, &event); // if last
	...

fwrite(buffers[buf.index].start, buf.bytesused, 1, fp);

// how to get buffer readly => q->done_list]
mvx_handle_buffer_general(struct mvx_session *session, struct mvx_fw_msg *msg) 
	session->event(session, MVX_SESSION_EVENT_BUFFER, buf);

mvx_handle_buffer(struct mvx_session *session, struct mvx_fw_msg *msg)
	session->event(session, MVX_SESSION_EVENT_BUFFER, buf);

return_done_buffers(session, dir);
	list_for_each_entry_safe(buf, tmp, &session->port[MVX_DIR_OUTPUT].buffer_done_queue, head)
		session->event(session, MVX_SESSION_EVENT_BUFFER, buf);
... //lots of calls

session->event = handle_event;
handle_event(struct mvx_session *session, enum mvx_session_event event, void *arg);
void vb2_buffer_done(struct vb2_buffer *vb, enum vb2_buffer_state state)
	list_add_tail(&vb->done_entry, &q->done_list);
	wake_up(&q->done_wq);

### |-- VIDIOC_STREAMOFF ----------->|
ioctl(fd, VIDIOC_STREAMOFF, &type);
mvx_v4l2_vidioc_streamoff(struct file *file, void *priv, enum v4l2_buf_type type)
	vb2_streamoff(&vsession->port[dir].vb2_queue, type);
		vb2_core_streamoff(q, type);
			__vb2_queue_cancel(q);
				if (q->start_streaming_called)
					call_void_qop(q, stop_streaming, q);
				if (q->streaming)
					call_void_qop(q, unprepare_streaming, q);
				wake_up_all(&q->done_wq);
				for (i = 0; i < q->num_buffers; ++i) {
					...
					__vb2_buf_mem_finish(vb);
					__vb2_dqbuf(vb);
					...
				}


### |-- VIDIOC_STREAMON --------------->|
ioctl(fd, VIDIOC_STREAMON, &type);
mvx_vb2_ops->start_streaming
start_streaming(struct vb2_queue *q, unsigned int cnt)
->mvx_session_streamon(&vsession->session, vport->dir);
-->bdir = get_bitstream_port(session);
-->session->client_ops->get_hw_ver(session->client_ops, &hw_ver);
-->mvx_fw_cache_get(session->cache, session->port[bdir].format, bdir, &session->fw_event/event, &hw_ver, session->isession.securevideo);
	//load fw_bin
	fw_bin = fw_bin_get(cache, format, dir, hw_ver, securevideo);
		 list_for_each_entry(tmp, &cache->fw_bin_list, cache_head) {
			if (tmp->format == format
				&& tmp->dir == dir
				&& hwvercmp(&tmp->hw_ver, hw_ver) == 0
				&& tmp->securevideo == securevideo
				&& atomic_read(&tmp->flush_cnt) == atomic_read(&cache->flush_cnt)) {
				fw_bin = tmp;
				break;
			}
		 }
	/* If firmware was not found, then try to request firmware. */
	if (fw_bin == NULL) {
		fw_bin = fw_bin_create(cache, format, dir, hw_ver, securevideo);
			get_fw_name(fw_bin->filename, sizeof(fw_bin->filename), format, dir, &fw_bin->hw_ver); //eg: mpeg4dec.fwb
			if (securevideo != false) {
				mvx_secure_request_firmware_nowait(cache->secure, fw_bin->filename, MVX_SECURE_NUMCORES, fw_bin, secure_request_firmware_done);
			} else {
				request_firmware_nowait(THIS_MODULE, true, fw_bin->filename, fw_bin->dev, GFP_KERNEL, fw_bin, request_firmware_done);
			}
		list_add(&fw_bin->cache_head, &cache->fw_bin_list);
	} else {
		kobject_get(&fw_bin->kobj);
	}
	event->fw_bin_ready(fw_bin, event->arg, true);

void request_firmware_done(const struct firmware *fw, void *arg)
	fw_bin_validate(fw, fw_bin->dev);
	header = (struct mvx_fw_header *)fw->data;
	fw_bin->nonsecure.header = header;
	fw_bin->nonsecure.text_cnt = (header->text_length + MVE_PAGE_SIZE - 1) >> MVE_PAGE_SHIFT;
	...
	fw_bin->nonsecure.fw = fw;
	fw_bin_callback(fw_bin);
		list_for_each_entry_safe(event, tmp, &fw_bin->event_list, head) {
			list_del(&event->head);
			event->fw_bin_ready(fw_bin, event->arg, false);
		}

event->fw_bin_ready(fw_bin, event->arg, true);
void fw_bin_ready(struct mvx_fw_bin *bin, void *arg, bool same_thread)
	/* Create client session. */
	session->isession.core_mask = session->client_ops->get_core_mask(session->client_ops);
	session->isession.ncores = hweight32(session->isession.core_mask);
	session->isession.l0_pte = mvx_mmu_set_pte(MVX_ATTR_PRIVATE, virt_to_phys(session->mmu.page_table), MVX_ACCESS_READ_WRITE);
	session->csession = session->client_ops->register_session(session->client_ops, &session->isession);
	/* Construct the firmware instance. */
	mvx_fw_factory(&session->fw, bin, &session->mmu,
					session, session->client_ops, session->csession,
					session->isession.core_mask, session->dentry);
		switch (major) {
		case 2:
			mvx_fw_construct_v2(fw, fw_bin, mmu, session, client_ops, csession, core_mask, major, minor);
				fw->ops.map_protocol = map_protocol_v2;
				fw->ops.unmap_protocol = unmap_protocol_v2;
				fw->ops.get_message = get_message_v2;
				fw->ops.put_message = put_message_v2;
				fw->ops.handle_rpc = handle_rpc_v2;
				fw->ops.print_stat = print_stat_v2;
				fw->ops_priv.xxx = yyy;
				...
		case 3:
			mvx_fw_construct_v3(fw, fw_bin, mmu, session, client_ops, csession, core_mask, major, minor);
				mvx_fw_construct_v2(fw, fw_bin, mmu, session, client_ops,...);
				fw->ops.get_region = get_region_v3;
				fw->ops_priv.to_mve_profile = to_mve_profile_v3;
		}
		fw_map(fw); //mmu: alloc text, bss, mmu table for each core, copy firmware binary,
			if (fw->fw_bin->securevideo != false) {
				mvx_mmu_map_l2(fw->mmu, fw_base, l2);
			} else {
				/* allocs */
				fw->text = mvx_mmu_alloc_pages(fw->dev, fw_bin->nonsecure.text_cnt, 0, GFP_KERNEL);
				fw->bss = mvx_mmu_alloc_pages( fw->dev, fw_bin->nonsecure.bss_cnt * fw->ncores, 0, GFP_KERNEL | __GFP_ZERO);
				fw->bss_shared = mvx_mmu_alloc_pages(fw->dev, fw_bin->nonsecure.sbss_cnt, 0, GFP_KERNEL | __GFP_ZERO);
				for (i = 0; i < fw->ncores; i++)
					fw_map_core(fw, i);
						bss_cnt = core * fw->fw_bin->nonsecure.bss_cnt;
						fw->ops.get_region(MVX_FW_REGION_CORE_0 + core, &fw_base, &end);
						mvx_mmu_map_pages(fw->mmu, fw_base + FW_TEXT_BASE_ADDR, fw->text, MVX_ATTR_PRIVATE, MVX_ACCESS_EXECUTABLE, NULL);
						va = header->bss_start_address;
						for (i = 0; i < header->bss_bitmap_size; i++) {
							mvx_mmu_map_pa(fw->mmu, fw_base + va, fw->bss->pages[bss_cnt++], MVE_PAGE_SIZE, MVX_ATTR_PRIVATE, MVX_ACCESS_READ_WRITE);
							va += MVE_PAGE_SIZE;
						}
				/* Copy firmware binary. */
				fw->ops.get_region(MVX_FW_REGION_CORE_0 + __ffs(mask), &fw_base, &end);
				mvx_mmu_write(fw->mmu, fw_base + FW_TEXT_BASE_ADDR, fw_bin->nonsecure.fw->data, header->text_length);
			}
			fw->ops.map_protocol(fw);//map_protocol_v2
				map_msq(fw, &fw->msg_host, MVX_FW_REGION_MSG_HOST);
				map_msq(fw, &fw->msg_mve, MVX_FW_REGION_MSG_MVE);
				map_msq(fw, &fw->buf_in_host, MVX_FW_REGION_BUF_IN_HOST);
				map_msq(fw, &fw->buf_in_mve, MVX_FW_REGION_BUF_IN_MVE);
				map_msq(fw, &fw->buf_out_host, MVX_FW_REGION_BUF_OUT_HOST);
				map_msq(fw, &fw->buf_out_mve, MVX_FW_REGION_BUF_OUT_MVE);
				map_msq(fw, &fw->rpc, MVX_FW_REGION_RPC);
				map_fw_print_ram(fw, &fw->fw_print_ram, MVX_FW_REGION_PRINT_RAM);
		fw_debugfs_init(fw, parent);
	session->fw_bin = bin;
	complete(&session->fw_loaded);
	fw_initial_setup(session);
		fw_set_option(session, &option); //watchdog
		fw_job(session, session->job_frames);
		fw_encoder_setup(session);/fw_decoder_setup(session);
		fw_state_change(session, MVX_FW_STATE_RUNNING);
		fw_ping(session);
	queue_pending_buffers(session, MVX_DIR_INPUT);
	queue_pending_buffers(session, MVX_DIR_OUTPUT);
		list_for_each_entry_safe(buf, tmp, &session->port[dir].buffer_queue, head) {
			queue_buffer(session, dir, buf);
				if (mvx_buffer_is_mapped(buf) == false)
					map_buffer(session, dir, buf);
				msg.code = MVX_FW_CODE_BUFFER;
				msg.buf = buf;
				session->fw.ops.put_message(&session->fw, &msg);
				send_irq(session);
			pending_buf_idx++;
			list_del(&buf->head);
		}

enum mvx_fw_region {
    MVX_FW_REGION_CORE_0,
    MVX_FW_REGION_CORE_1,
    MVX_FW_REGION_CORE_2,
    MVX_FW_REGION_CORE_3,
    MVX_FW_REGION_CORE_4,
    MVX_FW_REGION_CORE_5,
    MVX_FW_REGION_CORE_6,
    MVX_FW_REGION_CORE_7,
    MVX_FW_REGION_PROTECTED,
    MVX_FW_REGION_FRAMEBUF,
    MVX_FW_REGION_MSG_HOST,
    MVX_FW_REGION_MSG_MVE,
    MVX_FW_REGION_BUF_IN_HOST,
    MVX_FW_REGION_BUF_IN_MVE,
    MVX_FW_REGION_BUF_OUT_HOST,
    MVX_FW_REGION_BUF_OUT_MVE,
    MVX_FW_REGION_RPC,
    MVX_FW_REGION_PRINT_RAM
};
eg: component/cix_proprietary/cix_proprietary-debs/cix-vpu-umd/usr/lib/firmware/mpeg4dec.fwb


ref:
https://baron-z.cn/2025/03/10/linux%E9%A9%B1%E5%8A%A8-V4L2%E9%A9%B1%E5%8A%A8%E6%A1%86%E6%9E%B6/
https://www.cnblogs.com/fuzidage/p/18462450

/**
 * enum vb2_buffer_state - current video buffer state.
 * @VB2_BUF_STATE_DEQUEUED:	buffer under userspace control.
 * @VB2_BUF_STATE_IN_REQUEST:	buffer is queued in media request.
 * @VB2_BUF_STATE_PREPARING:	buffer is being prepared in videobuf2.
 * @VB2_BUF_STATE_QUEUED:	buffer queued in videobuf2, but not in driver.
 * @VB2_BUF_STATE_ACTIVE:	buffer queued in driver and possibly used
 *				in a hardware operation.
 * @VB2_BUF_STATE_DONE:		buffer returned from driver to videobuf2, but
 *				not yet dequeued to userspace.
 * @VB2_BUF_STATE_ERROR:	same as above, but the operation on the buffer
 *				has ended with an error, which will be reported
 *				to the userspace when it is dequeued.
 */
enum vb2_buffer_state {
	VB2_BUF_STATE_DEQUEUED,
	VB2_BUF_STATE_IN_REQUEST,
	VB2_BUF_STATE_PREPARING,
	VB2_BUF_STATE_QUEUED,
	VB2_BUF_STATE_ACTIVE,
	VB2_BUF_STATE_DONE,
	VB2_BUF_STATE_ERROR,
};

/**
 * struct vb2_plane - plane information.
 * @mem_priv:	private data with this plane.
 * @dbuf:	dma_buf - shared buffer object.
 * @dbuf_mapped:	flag to show whether dbuf is mapped or not
 * @bytesused:	number of bytes occupied by data in the plane (payload).
 * @length:	size of this plane (NOT the payload) in bytes. The maximum
 *		valid size is MAX_UINT - PAGE_SIZE.
 * @min_length:	minimum required size of this plane (NOT the payload) in bytes.
 *		@length is always greater or equal to @min_length, and like
 *		@length, it is limited to MAX_UINT - PAGE_SIZE.
 * @m:		Union with memtype-specific data.
 * @m.offset:	when memory in the associated struct vb2_buffer is
 *		%VB2_MEMORY_MMAP, equals the offset from the start of
 *		the device memory for this plane (or is a "cookie" that
 *		should be passed to mmap() called on the video node).
 * @m.userptr:	when memory is %VB2_MEMORY_USERPTR, a userspace pointer
 *		pointing to this plane.
 * @m.fd:	when memory is %VB2_MEMORY_DMABUF, a userspace file
 *		descriptor associated with this plane.
 * @data_offset:	offset in the plane to the start of data; usually 0,
 *		unless there is a header in front of the data.
 *
 * Should contain enough information to be able to cover all the fields
 * of &struct v4l2_plane at videodev2.h.
 */
struct vb2_plane {
	void			*mem_priv;
	struct dma_buf		*dbuf;
	unsigned int		dbuf_mapped;
	unsigned int		bytesused;
	unsigned int		length;
	unsigned int		min_length;
	union {
		unsigned int	offset;
		unsigned long	userptr;
		int		fd;
	} m;
	unsigned int		data_offset;
};

/**
 * struct vb2_buffer - represents a video buffer.
 * @vb2_queue:		pointer to &struct vb2_queue with the queue to
 *			which this driver belongs.
 * @index:		id number of the buffer.
 * @type:		buffer type.
 * @memory:		the method, in which the actual data is passed.
 * @num_planes:		number of planes in the buffer
 *			on an internal driver queue.
 * @timestamp:		frame timestamp in ns.
 * @request:		the request this buffer is associated with.
 * @req_obj:		used to bind this buffer to a request. This
 *			request object has a refcount.
 */
struct vb2_buffer {
	struct vb2_queue	*vb2_queue;
	unsigned int		index;
	unsigned int		type;
	unsigned int		memory;
	unsigned int		num_planes;
	u64			timestamp;
	struct media_request	*request;
	struct media_request_object	req_obj;

	/* private: internal use only
	 *
	 * state:		current buffer state; do not change
	 * synced:		this buffer has been synced for DMA, i.e. the
	 *			'prepare' memop was called. It is cleared again
	 *			after the 'finish' memop is called.
	 * prepared:		this buffer has been prepared, i.e. the
	 *			buf_prepare op was called. It is cleared again
	 *			after the 'buf_finish' op is called.
	 * copied_timestamp:	the timestamp of this capture buffer was copied
	 *			from an output buffer.
	 * skip_cache_sync_on_prepare: when set buffer's ->prepare() function
	 *			skips cache sync/invalidation.
	 * skip_cache_sync_on_finish: when set buffer's ->finish() function
	 *			skips cache sync/invalidation.
	 * queued_entry:	entry on the queued buffers list, which holds
	 *			all buffers queued from userspace
	 * done_entry:		entry on the list that stores all buffers ready
	 *			to be dequeued to userspace
	 * vb2_plane:		per-plane information; do not change
	 */
	enum vb2_buffer_state	state;
	unsigned int		synced:1;
	unsigned int		prepared:1;
	unsigned int		copied_timestamp:1;
	unsigned int		skip_cache_sync_on_prepare:1;
	unsigned int		skip_cache_sync_on_finish:1;

	struct vb2_plane	planes[VB2_MAX_PLANES];
	struct list_head	queued_entry;
	struct list_head	done_entry;
#ifdef CONFIG_VIDEO_ADV_DEBUG
	/*
	 * Counters for how often these buffer-related ops are
	 * called. Used to check for unbalanced ops.
	 */
	u32		cnt_mem_alloc;
	u32		cnt_mem_put;
	u32		cnt_mem_get_dmabuf;
	u32		cnt_mem_get_userptr;
	u32		cnt_mem_put_userptr;
	u32		cnt_mem_prepare;
	u32		cnt_mem_finish;
	u32		cnt_mem_attach_dmabuf;
	u32		cnt_mem_detach_dmabuf;
	u32		cnt_mem_map_dmabuf;
	u32		cnt_mem_unmap_dmabuf;
	u32		cnt_mem_vaddr;
	u32		cnt_mem_cookie;
	u32		cnt_mem_num_users;
	u32		cnt_mem_mmap;

	u32		cnt_buf_out_validate;
	u32		cnt_buf_init;
	u32		cnt_buf_prepare;
	u32		cnt_buf_finish;
	u32		cnt_buf_cleanup;
	u32		cnt_buf_queue;
	u32		cnt_buf_request_complete;

	/* This counts the number of calls to vb2_buffer_done() */
	u32		cnt_buf_done;
#endif
};
