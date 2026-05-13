


setup_arch(&command_line);
	kaslr_init();
	early_fixmap_init();
	early_ioremap_init();
	parse_early_param();
	cpu_uninstall_idmap();
	arm64_memblock_init();
	paging_init();
	bootmem_init();
	kasan_init();
	psci_dt_init();/psci_acpi_init();
...	
mm_init();
...
kthread_start	
	
	
Linux Reserve Memory简述
Linux系统reserve内存主要分成两个部分：

系统reserve内存： 不进入buddy系统 
memblock.memory 中设置”nomap“ flag。
加入memblock.reserved 中。
设备驱动使用：
dev→dma_mem: memblock.memory + "nomap" flag
dev→ cma_area: memblock.reserved

## kernel boot memory (original)
## memblock & reserved memory
start_kernel()
	setup_arch()
		setup_machine_fdt(__fdt_pointer);
			early_init_dt_scan(dt_virt);
				early_init_dt_scan_nodes();
					early_init_dt_scan_memory();
						early_init_dt_add_memory_arch(base, size);
							memblock_add(base, size); 		/* DeviceTree: system memory add */
		efi_init()
			reserve_regions() 								/** ACPI memory_reserve **/
				memblock_remove(0, PHYS_ADDR_MAX); // clean all memblock.memory
				early_init_dt_add_memory_arch(paddr, size);
 					memblock_add(base, size); 				/* ACPI: system memory add */
 				if (!is_usable_memory(md))
					memblock_mark_nomap(paddr, size);
				if (md->type == EFI_ACPI_RECLAIM_MEMORY)
					memblock_reserve(paddr, size);
		arm64_memblock_init()
			...
			memblock_add(__pa_symbol(_text), (u64)(_end - _text));
			memblock_reserve(__pa_symbol(_stext), _end - _stext);
			early_init_fdt_scan_reserved_mem()				/** DeviceTree: memory_reserve **/
				fdt_scan_reserved_mem()
					fdt_for_each_subnode(child, fdt, node) {
						if (!of_fdt_device_is_available(fdt, child)); //status = "ok"/"okay"
							continue;
						__reserved_mem_reserve_reg(child, uname);
							early_init_dt_reserve_memory(base, size, nomap) == 0)
								memblock_mark_nomap(base, size); || memblock_reserve(base, size);
						fdt_reserved_mem_save_node(child, uname, 0, 0);
							reserved_mem[reserved_mem_count++];
					}
				for (n = 0; ; n++) {
					fdt_get_mem_rsv(initial_boot_params, n, &base, &size); //fdt_get_header(fdt, off_mem_rsvmap)
					memblock_reserve(base, size); 
				}
				fdt_init_reserved_mem();
					for (i = 0; i < reserved_mem_count; i++) {
						__reserved_mem_alloc_size(node, rmem->name, &rmem->base, &rmem->size);
						__reserved_mem_init_node(rmem);
							for (i = __reservedmem_of_table; i < &__rmem_of_table_sentinel; i++) {
								if (of_flat_dt_is_compatible(rmem->fdt_node, compat))
									initfn(rmem); //eg: RESERVEDMEM_OF_DECLARE(cma, "shared-dma-pool", rmem_cma_setup);
							}
					}
		paging_init()
			map_kernel(pgdp);
			map_mem(pgdp);
			cpu_replace_ttbr1(lm_alias(swapper_pg_dir), init_idmap_pg_dir);
			init_mm.pgd = swapper_pg_dir;
			memblock_phys_free(__pa_symbol(init_pg_dir)...)
			create_idmap();
		
	/* should reserve before here */
	mm_init()
		mem_init()
			memblock_free_all()
				free_low_memory_core_early()
					for_each_free_mem_range() {
						free... ->buddy
					}
	/* should bind reserve memory to dma_mem, cma_mem and other purpose after here */
		
	kthread_start..
			init_calls... acpi device create, driver probe


## bootmem_init :sparse, zone
start_kernel()
	setup_arch()
		bootmem_init();
			sparse_init();
				for_each_present_section_nr(pnum_begin + 1, pnum_end)
					sparse_init_nid(nid_begin, pnum_begin, pnum_end, map_count);
			zone_sizes_init();
				free_area_init(max_zone_pfns);
					for_each_mem_pfn_range(i, MAX_NUMNODES, &start_pfn, &end_pfn, &nid)
						subsection_map_init(start_pfn, end_pfn - start_pfn);
					for_each_node(nid)
						free_area_init_node(nid);
							pg_data_t *pgdat = NODE_DATA(nid);
							pgdat->... = ...
							calculate_node_totalpages(pgdat, start_pfn, end_pfn);
							alloc_node_mem_map(pgdat);
								pgdat->node_mem_map = map + offset;
							free_area_init_core(pgdat);
								for (j = 0; j < MAX_NR_ZONES; j++) {
									nr_kernel_pages += zone->present_pages;
									nr_all_pages += zone->present_pages;
									zone_init_internals(zone, j, nid, zone->present_pages);
										atomic_long_set(&zone->managed_pages, zone->present_pages);
										zone_set_nid(zone, nid);
										zone->zone_pgdat = NODE_DATA(nid);
										...
								}
						
## mm_init: buddy system
/* buddy */
start_kernel()
	mm_core_init();
		mem_init();
			memblock_free_all();
				pages = free_low_memory_core_early();
				totalram_pages_add(pages);
					atomic_long_add(count, &_totalram_pages); 
	rest_init()
		kernel_init(void) //thread
			kernel_init_freeable();
			free_initmem();
				free_reserved_area(lm_alias(__init_begin), lm_alias(__init_end), ...);
				
				
memblock_free_all();
	pages = free_low_memory_core_early();
		memmap_init_reserved_pages();
		for_each_free_mem_range(i, NUMA_NO_NODE, MEMBLOCK_NONE, &start, &end, NULL);
			__free_memory_core(start, end);
				__free_pages_memory(start_pfn, end_pfn);
					while (start < end)
						memblock_free_pages(pfn_to_page(start), start, order);
							while (start + (1UL << order) > end) order--;
							__free_pages_core(page, order);				
	totalram_pages_add(pages);
		atomic_long_add(count, &_totalram_pages); 
		

__free_pages_core(page, order);			
	for (loop = 0; loop < (nr_pages - 1); loop++, p++)
		__ClearPageReserved(p);
		set_page_count(p, 0);
	__ClearPageReserved(p);
	set_page_count(p, 0); 
	atomic_long_add(nr_pages, &page_zone(page)->managed_pages);
	__free_pages_ok(page, order, FPI_TO_TAIL);
		migratetype = get_pfnblock_migratetype(page, pfn); // => area->free_list[migratetype]
		__free_one_page(page, pfn, zone, order, migratetype, fpi_flags); 
			while (order < MAX_ORDER)
				merging...
			set_buddy_order(page, order);
			add_to_free_list_tail(page, zone, order, migratetype);

## /* alloc_pages */

struct page *alloc_pages(gfp_t gfp, unsigned order) 
	pol = get_task_policy(current); //=> MPOL_INTERLEAVE/MPOL_PREFERRED_MANY/others
	__alloc_pages(gfp, order, policy_node(gfp, pol, numa_node_id()), policy_nodemask(gfp, pol));
		gfp = current_gfp_context(gfp);
		prepare_alloc_pages(gfp, order, preferred_nid, nodemask, &ac, &alloc_gfp, &alloc_flags))
		alloc_flags |= alloc_flags_nofragment(ac.preferred_zoneref->zone, gfp); 
		page = get_page_from_freelist(alloc_gfp, order, alloc_flags, &ac);
		if (!page)
			__alloc_pages_slowpath(gfp, order, &ac);

prepare_alloc_pages(gfp, order, preferred_nid, nodemask, &ac, &alloc_gfp, &alloc_flags))
	

__alloc_pages_slowpath(alloc_gfp, order, &ac);
	


#define GFP_KERNEL→     (__GFP_RECLAIM | __GFP_IO | __GFP_FS)
	(___GFP_DIRECT_RECLAIM|___GFP_KSWAPD_RECLAIM) | ___GFP_IO | ___GFP_FS
	
	
	
/* dma_alloc_coherent */

/* /proc/meminfo */


```
Buddy 初始化 bypass reserved memory（nomap & reserved）


/**
 * for_each_free_mem_range - iterate through free memblock areas
 * @i: u64 used as loop variable
 * @nid: node selector, %NUMA_NO_NODE for all nodes
 * @flags: pick from blocks based on memory attributes
 * @p_start: ptr to phys_addr_t for start address of the range, can be %NULL
 * @p_end: ptr to phys_addr_t for end address of the range, can be %NULL
 * @p_nid: ptr to int for nid of the range, can be %NULL
 *
 * Walks over free (memory && !reserved) areas of memblock.  Available as
 * soon as memblock is initialized.
 */
#define for_each_free_mem_range(i, nid, flags, p_start, p_end, p_nid)	\
	__for_each_mem_range(i, &memblock.memory, &memblock.reserved,	\   /* bypass reserved */
			     nid, flags, p_start, p_end, p_nid)

#define __for_each_mem_range(i, type_a, type_b, nid, flags,		\
			   p_start, p_end, p_nid)			\
	for (i = 0, __next_mem_range(&i, nid, flags, type_a, type_b,	\
				     p_start, p_end, p_nid);		\
	     i != (u64)ULLONG_MAX;					\
	     __next_mem_range(&i, nid, flags, type_a, type_b,		\
			      p_start, p_end, p_nid))
				  
void __next_mem_range(u64 *idx, int nid, enum memblock_flags flags,
		      struct memblock_type *type_a,
		      struct memblock_type *type_b, phys_addr_t *out_start,
		      phys_addr_t *out_end, int *out_nid)
{
	...

	for (; idx_a < type_a->cnt; idx_a++) {
		struct memblock_region *m = &type_a->regions[idx_a];


		if (should_skip_region(type_a, m, nid, flags)) /* bypass "nomap" */
			continue;

		/* scan areas before each reservation */
		for (; idx_b < type_b->cnt + 1; idx_b++) {
			...	
			}
		}
	}
	...
}

static bool should_skip_region(struct memblock_type *type,
			       struct memblock_region *m,
			       int nid, int flags)
{
	...	
	/* skip nomap memory unless we were asked for it explicitly */
	if (!(flags & MEMBLOCK_NOMAP) && memblock_is_nomap(m))
		return true;
	...

	return false;
}



其他：
uefi memory boot services 获取内存信息（only for ACPI）
efi function


efi_exit_boot_services()
	efi_get_memory_map(&map, true);
	priv_func(map, priv);
	efi_bs_call(exit_boot_services, handle, map->map_key);
ioremap reserve memory should mark ”no-map“


ioremap()
	ioremap_prot()
		if (!ioremap_allowed())
			return NULL;

ioremap_allowed()
	if (WARN_ON(pfn_is_map_memory(__phys_to_pfn(phys_addr))))
		 return false;

pfn_is_map_memory()
	return memblock_is_map_memory(addr);
	
	
驱动 reserved memory 使用 - DMA/CMA (Devicetree)
DMA/CMA
DeviceTree "reserved-memory" 定义reserved memory节点
DMA 使用 “no-map”属性
CMA 使用 ”reuseable“ 属性
初始化struct reserved_mem 结构 (kernel/dma/coherent.c - RESERVEDMEM_OF_DECLARE)
驱动调用 reserved_mem->ops→device_init()
platform bus probe的时候调用platform_bus_type→dma_configure() → platfrom_dma_configure()
platform_dma_configure()→ of_dma_configure()→ of_dma_configure_id()→ if(!iommu) of_dma_set_restricted_buffer(dev, np)→ of_reserved_mem_device_init_by_idx()
驱动主动调用of_reserved_mem_device_init_xxx接口
```



enum zone_stat_item {
    // 活跃和非活跃页面统计
    NR_FREE_PAGES,          // 空闲页面数量
    NR_ZONE_LRU_BASE,       // LRU链表基础统计
    NR_ZONE_INACTIVE_ANON,  // 非活跃匿名页
    NR_ZONE_ACTIVE_ANON,    // 活跃匿名页  
    NR_ZONE_INACTIVE_FILE,  // 非活跃文件页
    NR_ZONE_ACTIVE_FILE,    // 活跃文件页
    NR_ZONE_UNEVICTABLE,    // 不可回收页面
    // ...
};

/* include/linux/mmzone.h */
enum migratetype {
    MIGRATE_UNMOVABLE,      // 不可移动页面
    MIGRATE_MOVABLE,        // 可移动页面  
    MIGRATE_RECLAIMABLE,    // 可回收页面
    MIGRATE_PCPTYPES,       // per-cpu pageset 类型数量
    MIGRATE_HIGHATOMIC = MIGRATE_PCPTYPES, // 高阶原子分配
    MIGRATE_CMA,            // 连续内存分配器
    MIGRATE_ISOLATE,        // 不能从这里分配
    MIGRATE_TYPES           // 类型总数
};





































































