

#aipu
armchina_aipu_probe(p_dev, &sky1, &sky1_ops);
-> aipu = devm_kzalloc(dev, sizeof(*aipu), GFP_KERNEL);
-> device_property_read_u32(dev, "core-id", &id);
-> init_aipu_priv(aipu, p_dev, &aipu_fops, soc, ops);
->	aipu->dev = &p_dev->dev;
->	aipu->aipu_fops = fops;
->	aipu->soc = soc;
->	aipu->soc_ops = soc_ops;
->	aipu->ops = get_v3_1_priv_ops(); // or get_v3_priv_ops();
->	init_misc_dev(aipu);
->		aipu->misc.fops = aipu->aipu_fops;
->		misc_register(&aipu->misc);
->	aipu_init_mm(&aipu->mm, p_dev, version);
->		sram_disable_head = devm_kzalloc(mm->dev, sizeof(*mm->sram_disable_head), GFP_KERNEL);
->		mm->importer_bufs = devm_kzalloc(mm->dev, sizeof(*mm->importer_bufs), GFP_KERNEL);
->		mm->default_asid_base = 0;
->		mm->default_asid_size = 0xC0000000;
->		mm->valid_asid_cnt = 0;
->		mm->obj_cache = kmem_cache_create("aipu_obj_cache", sizeof(struct aipu_mem_region_obj), 0, SLAB_PANIC, NULL);
->		mm->reg_cache = kmem_cache_create("aipu_reg_cache", sizeof(struct aipu_mem_region), 0, SLAB_PANIC, NULL);
->		mm->tbuf_cache = kmem_cache_create("aipu_tbuf_cache", sizeof(struct aipu_tcb_buf), 0, SLAB_PANIC, NULL);
->		mm->hold_tbuf_cache = kmem_cache_create("aipu_hold_tbuf_cache", sizeof(struct aipu_hold_tcb_buf), 0, SLAB_PANIC, NULL);
->		for (asid = AIPU_BUF_ASID_0; asid < ZHOUYI_ASID_COUNT; asid++)
->			ret = init_region_list(mm, &mm->ase[asid]);
->		ret = init_region_list(mm, &mm->mem);
->		if (!mm->has_iommu)
->			mm->res_cnt = aipu_mm_add_reserved_regions(mm);
->		else
->			mm->res_cnt = aipu_mm_add_iova_region(mm);
->	init_aipu_job_manager(&aipu->job_manager, &aipu->mm, aipu/priv);
->		manager->version = ((struct aipu_priv *)priv)->version;
->		manager->dev = ((struct aipu_priv *)priv)->dev;
->		manager->job_cache = kmem_cache_create("aipu_job_cache", sizeof(struct aipu_job), 0, SLAB_PANIC, NULL);
->		manager->prof_cache = kmem_cache_create("aipu_prof_cache", sizeof(struct profiler), 0, SLAB_PANIC, NULL);
->		manager->mm = mm;
->		manager->priv = priv;
->		if (manager->version == AIPU_ISA_VERSION_ZHOUYI_V3_1) {
->			manager->grid_id = 1;
->			manager->group_id_num = 0x7FFF;
->			manager->group_id_bmap =
->				devm_kzalloc(manager->dev,
->					     BITS_TO_LONGS(manager->group_id_num) * sizeof(long),
->					     GFP_KERNEL);
->		}
-> aipu->ops->create_partitions(aipu, id, p_dev);
--> v3_create_partitions(aipu, id, p_dev);
--> 	cluster_arr = devm_kzalloc(&p_dev->dev, cluster_cnt * 2 * sizeof(u32), GFP_KERNEL);
--> 	for (iter = 0; iter < cluster_cnt; iter++)
--> 		if (cluster_arr[2 * iter + 1] > (partition_cnt - 1))
--> 			partition_cnt = cluster_arr[2 * iter + 1] + 1;
--> 	partitions = devm_kzalloc(&p_dev->dev, sizeof(*partitions), GFP_KERNEL);
--> 	ret = aipu_common_init_reg_irq(p_dev, &partitions[0], &aipu->reg, &aipu->irq_obj);
--> 		platform_get_resource(p_dev, IORESOURCE_MEM, 0);
--> 		base = res->start;
--> 		size = res->end - res->start + 1;
--> 		init_aipu_ioregion(reg, base, size);
--> 		irqnum = platform_get_irq(p_dev, 0);
--> 		aipu_create_irq_object(&p_dev->dev, irqnum, partition, "aipu");
--> 			irq_obj = kzalloc(sizeof(*irq_obj), GFP_KERNEL);
--> 			irq_obj->aipu_wq = NULL;
--> 			irq_obj->irqnum = 0;
--> 			irq_obj->dev = dev;
--> 			irq_obj->aipu_wq = create_singlethread_workqueue(description);
--> 			INIT_WORK(&irq_obj->work, aipu_irq_handler_bottom_half);
--> 			ret = request_irq(irqnum, aipu_irq_handler_upper_half,
--> 					  IRQF_ONESHOT | IRQF_SHARED | IRQF_PROBE_SHARED,
--> 					  description, irq_obj->dev);
--> 			irq_obj->irqnum = irqnum;
--> 			irq_obj->partition = partition;
--> 	ret = aipu->ops->global_soft_reset(aipu);
--> 	build_info = aipu_read32(&aipu->reg, TSM_BUILD_INFO_REG);
--> 	aipu->max_partition_cnt = GET_MAX_PARTITION_NUM(build_info);
--> 	aipu->max_cmd_pool_cnt = GET_MAX_CMD_POOL_NUM(build_info);
--> 	for (iter = 0; iter < partition_cnt; iter++) {
--> 		partitions[iter].id = iter;
--> 		partitions[iter].priv = aipu;
--> 		partitions[iter].version = version;
--> 		init_aipu_partition(&partitions[iter], cluster_arr, cluster_cnt, p_dev);
--> 			partition->arch = AIPU_ARCH_ZHOUYI;
--> 			partition->dev = &p_dev->dev;
--> 			partition->reg = &partition->priv->reg;
--> 			partition->irq_obj = partition->priv->irq_obj;
--> 			mutex_init(&partition->reset_lock);
--> 			partition->ops = get_zhouyi_v3_ops();
--> 			for (iter = 0; iter < tot_cluster; iter++) {
--> 				if (partition->id == clusters[2 * iter + 1]) {
--> 					cluster_cnt++;
--> 					partition->clusters[cluster_cnt - 1].id = clusters[2 * iter];
--> 					val = aipu_read32(partition->reg, CLUSTER_CONFIG_REG(clusters[2 * iter]));
--> 					partition->clusters[cluster_cnt - 1].core_cnt = GET_AIPU_CORE_NUM(val);
--> 					atomic_set(&partition->clusters[cluster_cnt - 1].en_core_cnt,
--> 						   GET_AIPU_CORE_NUM(val));
--> 					partition->clusters[cluster_cnt - 1].tec_cnt = GET_TEC_NUM(val);
--> 					aipu_write32(partition->reg, DEBUG_PAGE_SELECTION_REG,
--> 						     SELECT_DEBUG_CORE(0, 0));
--> 					partition->clusters[cluster_cnt - 1].gm_bytes =
--> 						get_gm_size(aipu_read32(partition->reg, DEBUG_CLUSTER_GM_CONTROL));
--> 					aipu_write32(partition->reg, DEBUG_PAGE_SELECTION_REG, DISABLE_DEBUG);
--> 					ret = aipu_mm_init_gm(&partition->priv->mm,
--> 							      partition->clusters[cluster_cnt - 1].gm_bytes);
--> 				}
--> 			}
--> 			partition->cluster_cnt = cluster_cnt;
--> 			partition->ops->initialize(partition);
--> 	}
--> 	aipu->partitions = partitions;
--> 	aipu->partition_cnt = partition_cnt;
--> 	aipu->cluster_cnt = cluster_cnt;
--> 	aipu_job_manager_set_partitions_info(&aipu->job_manager, aipu->partition_cnt,
--> 					     aipu->partitions);
--> 	partitions[0].ops->print_hw_id_info(&partitions[0]);

init_region_list(struct aipu_memory_manager *mm, struct aipu_mem_region_list *list)
	aipu_mm_create_region_object(mm, AIPU_MEM_REGION_TYPE_MEMORY, 0, 0, 0, NULL, false);
		obj = kmem_cache_zalloc(mm->obj_cache, GFP_KERNEL);
		obj->reg = aipu_mm_create_region(mm, obj, type, size, offset, idx, reserved);
			if (reserved && !mm->has_iommu)
				reg->dev = aipu_mm_create_child_dev(mm->dev, idx);
			else
				reg->dev = mm->dev;
			reg->tcb_buf_head = create_tcb_buf(mm, reg);
			reg->tcb_buf_head = create_tcb_buf(mm, reg);
			va = dma_alloc_attrs(reg->dev, reg->bytes, &reg->base_pa, GFP_KERNEL, reg->attrs);
			if (reserved && !mm->has_iommu)
				of_reserved_mem_device_init_by_idx(reg->dev, mm->dev->of_node, idx);
			if (mm->has_iommu)
				dma_set_mask_and_coherent(mm->dev, DMA_BIT_MASK(mm->dma_mask));
			reg->base_iova = reg->base_pa - reg->host_aipu_offset;
			reg->base_va = va;
			if (reserved)
				aipu_mm_init_pages(mm, reg);
					reg->count = reg->bytes >> PAGE_SHIFT;
					reg->bitmap = devm_kzalloc(reg->dev, BITS_TO_LONGS(reg->count) * sizeof(long), GFP_KERNEL);
					reg->pages = vzalloc(reg->count * sizeof(struct aipu_virt_page *));
					reg->base_pfn = PFN_DOWN(reg->base_iova);

		obj->reg->filp = filp;

static irqreturn_t aipu_irq_handler_upper_half(int irq, void *dev_id)
-> partition->ops->upper_half(partition);
--> zhouyi_v3_upper_half(data/partition)
---> for (iter = 0; iter < aipu->partition_cnt; iter++)
--->	partition_upper_half(&aipu->partitions[iter]);
--->	...
--->	aipu_job_manager_irq_upper_half(partition, GET_INTR_TYPE(status), &info);
--->	aipu_irq_schedulework(partition->irq_obj);
--->		queue_work(irq_obj->aipu_wq, &irq_obj->work);

aipu_irq_handler_bottom_half(work)
->irq_obj = container_of(work, struct aipu_irq_object, work);
->partition = irq_obj->partition;
->partition->ops->bottom_half(partition);
-->zhouyi_v3_bottom_half(void *data)
--->aipu_job_manager_irq_bottom_half(data);
--->	manager = get_job_manager(core);
--->	list_for_each_entry_safe(curr, next, &manager->scheduled_head->node, node) {
--->			...
--->			if (curr->desc.aipu_version == AIPU_ISA_VERSION_ZHOUYI_V3)
--->				aipu_mm_unlink_tcb(manager->mm, curr->curr_hold_tcb, false);
--->		/* destroy the v3 command pool if all jobs are done */
--->		if (curr->state < AIPU_JOB_STATE_EXCEP || curr->desc.is_coredump_en)
--->			do_destroy = false;
--->	}
--->	if (do_destroy)
--->		aipu_job_manager_destroy_command_pool_no_lock(manager, core, true);
--->	list_for_each_entry_safe(curr, next, &manager->scheduled_head->node, node) {
--->		if (curr->state >= AIPU_JOB_STATE_EXCEP && !curr->wake_up &&
--->		    (curr->desc.aipu_version >= AIPU_ISA_VERSION_ZHOUYI_V3 ||
--->		     curr->core_id == core->id)) {
--->			wake_up_interruptible(curr->thread_queue);
--->			curr->wake_up = 1;
--->		}
--->	}


static const struct file_operations aipu_fops = {
	.owner = THIS_MODULE,
	.open = aipu_open,
	.poll = aipu_poll,
	.unlocked_ioctl = aipu_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = aipu_compat_ioctl,
#endif
	.mmap = aipu_mmap,
	.release = aipu_release,
};

static struct aipu_soc_operations sky1_ops = {
	.start_bw_profiling = NULL,
	.stop_bw_profiling = NULL,
	.read_profiling_reg = NULL,
	.enable_clk = NULL,
	.disable_clk = NULL,
	.is_clk_enabled = NULL,
	.is_aipu_irq = NULL,
};

static struct aipu_operations zhouyi_v3_ops = {
	.get_config = NULL,
	.enable_interrupt = zhouyi_v3_enable_interrupt,
	.disable_interrupt = zhouyi_v3_disable_interrupt,
	.trigger = zhouyi_v3_trigger,
	.reserve = zhouyi_v3_reserve,
	.is_idle = zhouyi_v3_is_idle,
	.print_hw_id_info = zhouyi_v3_print_hw_id_info,
	.io_rw = zhouyi_v3_io_rw,
	.upper_half = zhouyi_v3_upper_half,
	.bottom_half = zhouyi_v3_bottom_half,
#ifdef CONFIG_SYSFS
	.sysfs_show = zhouyi_v3_sysfs_show,
#endif
	.soft_reset = zhouyi_v3_soft_reset,
	.initialize = zhouyi_v3_initialize,
	.destroy_command_pool = zhouyi_v3_destroy_command_pool,
	.abort_command_pool = zhouyi_v3_abort_command_pool,
	.exit_dispatch = zhouyi_v3_exit_dispatch,
	.disable_tick_counter = zhouyi_v3_disable_tick_counter,
	.enable_tick_counter = zhouyi_v3_enable_tick_counter,
	.enable_core_cnt = zhouyi_v3_enable_core_cnt,
};

struct aipu_operations *get_zhouyi_v3_ops(void)
{
	return &zhouyi_v3_ops;
}
