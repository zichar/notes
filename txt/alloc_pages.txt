
# alloc_pages

//include/linux/gfp_types.h
#define GFP_ATOMIC	(__GFP_HIGH|__GFP_KSWAPD_RECLAIM)
#define GFP_KERNEL	(__GFP_RECLAIM | __GFP_IO | __GFP_FS)
#define GFP_KERNEL_ACCOUNT (GFP_KERNEL | __GFP_ACCOUNT)
#define GFP_NOWAIT	(__GFP_KSWAPD_RECLAIM | __GFP_NOWARN)
#define GFP_NOIO	(__GFP_RECLAIM)
#define GFP_NOFS	(__GFP_RECLAIM | __GFP_IO)
#define GFP_USER	(__GFP_RECLAIM | __GFP_IO | __GFP_FS | __GFP_HARDWALL)
#define GFP_DMA		__GFP_DMA
#define GFP_DMA32	__GFP_DMA32
#define GFP_HIGHUSER	(GFP_USER | __GFP_HIGHMEM)
#define GFP_HIGHUSER_MOVABLE	(GFP_HIGHUSER | __GFP_MOVABLE | __GFP_SKIP_KASAN)
#define GFP_TRANSHUGE_LIGHT	((GFP_HIGHUSER_MOVABLE | __GFP_COMP | \
			 __GFP_NOMEMALLOC | __GFP_NOWARN) & ~__GFP_RECLAIM)
#define GFP_TRANSHUGE	(GFP_TRANSHUGE_LIGHT | __GFP_DIRECT_RECLAIM)


#define __GFP_RECLAIM ((__force gfp_t)(___GFP_DIRECT_RECLAIM|___GFP_KSWAPD_RECLAIM))


alloc_pages(gfp_mask/gfp, order)
	...
	__alloc_pages_noprof(gfp_mask/gfp, order, nid, NULL); //__alloc_pages
		unsigned int alloc_flags = ALLOC_WMARK_LOW;
		gfp &= gfp_allowed_mask;
		gfp = current_gfp_context(gfp);
			pflags = READ_ONCE(current->flags);
			if (pflags & PF_MEMALLOC_NOIO) flags &= ~(__GFP_IO | __GFP_FS);
			else if (pflags & PF_MEMALLOC_NOFS) flags &= ~__GFP_FS;
			if (pflags & PF_MEMALLOC_PIN) flags &= ~__GFP_MOVABLE;
		alloc_gfp = gfp; 
		prepare_alloc_pages(gfp, order, preferred_nid, nodemask, &ac, &alloc_gfp, &alloc_flags))
			ac.xxx = 
			if (should_fail_alloc_page(gfp_mask, order)) return false;
			alloc_flags => check ALLOC_CPUSET, ALLOC_CMA;
		alloc_flags |= alloc_flags_nofragment(zonelist_zone(ac.preferred_zoneref), gfp);
		page = get_page_from_freelist(alloc_gfp, order, alloc_flags, &ac); { /**/
		  retry:
			no_fallback = alloc_flags & ALLOC_NOFRAGMENT; 
			z = ac->preferred_zoneref;
			for_next_zone_zonelist_nodemask(zone, z, ac->highest_zoneidx, ac->nodemask) {
				if (cpusets_enabled() && (alloc_flags & ALLOC_CPUSET) && !__cpuset_zone_allowed(zone, gfp_mask)) { continue; }
				if (ac->spread_dirty_pages) { ... }
				if (no_fallback && nr_online_nodes > 1 && zone != zonelist_zone(ac->preferred_zoneref)) {
					if (zone_to_nid(zone) != local_nid) // local_nid = zonelist_node_idx(ac->preferred_zoneref)
						alloc_flags &= ~ALLOC_NOFRAGMENT;
						goto retry;
				}
				mark = wmark_pages(zone, alloc_flags & ALLOC_WMARK_MASK);
				if (!zone_watermark_fast(zone, order, mark, ac->highest_zoneidx, alloc_flags, gfp_mask)) {
					... check check check
					ret = node_reclaim(zone->zone_pgdat, gfp_mask, order); //if allowed:  /**/
						... check check check, return NODE_RECLAIM_NOSCAN if not allowed
						__node_reclaim(pgdat, gfp_mask, order);
							struct scan_control sc = { ... };
							if (node_pagecache_reclaimable(pgdat) > pgdat->min_unmapped_pages ||
								node_page_state_pages(pgdat, NR_SLAB_RECLAIMABLE_B) > pgdat->min_slab_pages) {
								do {
									shrink_node(pgdat, &sc);
								} while (sc.nr_reclaimed < nr_pages && --sc.priority >= 0);
							}
							... balaba
					if (ret) //fail: NODE_RECLAIM_NOSCAN, NODE_RECLAIM_FULL
						continue;
					if (zone_watermark_ok(zone, order, mark, ac->highest_zoneidx, alloc_flags))
						goto try_this_zone;
				}	
			
			  try_this_zone:
				page = rmqueue(ac->preferred_zoneref->zone, zone, order, gfp_mask, alloc_flags, ac->migratetype);  /**/
					if (likely(pcp_allowed_order(order)))
						page = rmqueue_pcplist(preferred_zone, zone, order,  migratetype, alloc_flags);
							list = &pcp->lists[order_to_pindex(migratetype, order)];
							page = __rmqueue_pcplist(zone, order, migratetype, alloc_flags, pcp, list);
								do {
									if (list_empty(list))
										alloced = rmqueue_bulk(zone, order, pcp->batch, list, migratetype, alloc_flags);
										pcp->count += alloced << order; 
									page = list_first_entry(list, struct page, pcp_list);
									list_del(&page->pcp_list);
									pcp->count -= 1 << order;
								} while (check_new_pages(page, order));
					page = rmqueue_buddy(preferred_zone, zone, order, alloc_flags, migratetype);
						do {
							page = __rmqueue(zone, order, migratetype, alloc_flags);
								if (IS_ENABLED(CONFIG_CMA) && alloc_flags & ALLOC_CMA)
									if ( zone_page_state(zone, NR_FREE_CMA_PAGES) > zone_page_state(zone, NR_FREE_PAGES) / 2)
										page = __rmqueue_cma_fallback(zone, order);
							  retry:
								page = __rmqueue_smallest(zone, order, migratetype);
									for (current_order = order; current_order < NR_PAGE_ORDERS; ++current_order) {
										area = &(zone->free_area[current_order]);
										page = get_page_from_free_area(area, migratetype);
											list_first_entry_or_null(&area->free_list[migratetype], struct page, buddy_list);
										continue; // if (!page)
										del_page_from_free_list(page, zone, current_order);
										expand(zone, page, order, current_order, migratetype);
										set_pcppage_migratetype(page, migratetype);
										return page;
									}
								if (unlikely(!page))
									if (alloc_flags & ALLOC_CMA)
										page = __rmqueue_cma_fallback(zone, order);
									if (!page && __rmqueue_fallback(zone, order, migratetype, alloc_flags);
										goto retry;
								}
							if (!page && (alloc_flags & (ALLOC_OOM|ALLOC_NON_BLOCK)))
								page = __rmqueue_smallest(zone, order, MIGRATE_HIGHATOMIC);
								return NULL; //if (!page)
						} while (check_new_pages(page, order)); 
				if (page)
					prep_new_page(page, order, gfp_mask, alloc_flags);
					if (unlikely(alloc_flags & ALLOC_HIGHATOMIC))
						reserve_highatomic_pageblock(page, zone);
					return page;
			}
			if (no_fallback)
				alloc_flags &= ~ALLOC_NOFRAGMENT;
				goto retry;
			return NULL; // get_page_from_freelist
		}
		if (likely(page))
			goto out; // fast path succeed
		alloc_gfp = gfp;
		ac.spread_dirty_pages = false;
		ac.nodemask = nodemask;
		page = __alloc_pages_slowpath(alloc_gfp, order, &ac); { /**/
		  restart:
			alloc_flags = gfp_to_alloc_flags(gfp_mask, order);
			ac->preferred_zoneref = first_zones_zonelist(ac->zonelist, ac->highest_zoneidx, ac->nodemask); 
			if (alloc_flags & ALLOC_KSWAPD)
				wake_all_kswapds(order, gfp_mask, ac);
					for_each_zone_zonelist_nodemask(zone, z, ac->zonelist, highest_zoneidx, ac->nodemask)
							wakeup_kswapd(zone, gfp_mask, order, highest_zoneidx);
								wake_up_interruptible(&pgdat->kswapd_wait);  /**/
			page = get_page_from_freelist(gfp_mask, order, alloc_flags, ac);  /**/
			if (page)
				goto got_pg;
			if (can_direct_reclaim && can_compact 
				&& (costly_order || (order > 0 && ac->migratetype != MIGRATE_MOVABLE))
				&& !gfp_pfmemalloc_allowed(gfp_mask))
				page = __alloc_pages_direct_compact(gfp_mask, order, alloc_flags, ac, INIT_COMPACT_PRIORITY, &compact_result);  /**/
				if (page)
					goto got_pg;
		  retry:
			if (alloc_flags & ALLOC_KSWAPD)
				wake_all_kswapds(order, gfp_mask, ac);
			reserve_flags = __gfp_pfmemalloc_flags(gfp_mask);
			if (reserve_flags)
				alloc_flags = gfp_to_alloc_flags_cma(gfp_mask, reserve_flags) | (alloc_flags & ALLOC_KSWAPD);
			//trys /**/
			page = get_page_from_freelist(gfp_mask, order, alloc_flags, ac);
			page = __alloc_pages_direct_reclaim(gfp_mask, order, alloc_flags, ac, &did_some_progress); {
				*did_some_progress = __perform_reclaim(gfp_mask, order, ac); /**/
					progress = try_to_free_pages(ac->zonelist, order, gfp_mask, ac->nodemask);
						struct scan_control sc = { ... }
						do_try_to_free_pages(zonelist, &sc);
							do {
								shrink_zones(zonelist, sc);
									for_each_zone_zonelist_nodemask(zone, z, zonelist, sc->reclaim_idx, sc->nodemask)
										if (!cgroup_reclaim(sc)) { ... };
										shrink_node(zone->zone_pgdat, sc);
							} while (--sc->priority >= 0);
							for_each_zone_zonelist_nodemask(zone, z, zonelist, sc->reclaim_idx, sc->nodemask) {
								if (cgroup_reclaim(sc))
									lruvec = mem_cgroup_lruvec(sc->target_mem_cgroup, zone->zone_pgdat);
							}
				page = get_page_from_freelist(gfp_mask, order, alloc_flags, ac);
				if (!page && !drained)
					drain_all_pages(NULL);
					goto ...// try again;
			}
			page = __alloc_pages_direct_compact(gfp_mask, order, alloc_flags, ac, compact_priority, &compact_result); {
				*compact_result = try_to_compact_pages(gfp_mask, order, alloc_flags, ac, prio, &page);
					for_each_zone_zonelist_nodemask(zone, z, ac->zonelist, ac->highest_zoneidx, ac->nodemask)
						compact_zone_order(zone, order, gfp_mask, prio, alloc_flags, ac->highest_zoneidx, capture);
							compact_zone(&cc, &capc);
				page = get_page_from_freelist(gfp_mask, order, alloc_flags, ac);
			}
			if (page)
				goto got_pg;
			if (should_reclaim_retry(gfp_mask, order, ac, alloc_flags, did_some_progress > 0, &no_progress_loops))
				goto retry;
			if (did_some_progress > 0 && can_compact && should_compact_retry(ac, order, alloc_flags, compact_result, &compact_priority, &compaction_retries))
				goto retry;
			if (check_retry_cpuset(cpuset_mems_cookie, ac) || check_retry_zonelist(zonelist_iter_cookie))
				goto restart;
			page = __alloc_pages_may_oom(gfp_mask, order, ac, &did_some_progress); /**/
				page = get_page_from_freelist((gfp_mask | __GFP_HARDWALL) & ~__GFP_DIRECT_RECLAIM, order, ALLOC_WMARK_HIGH|ALLOC_CPUSET, ac);
				if (out_of_memory(&oc) || WARN_ON_ONCE_GFP(gfp_mask & __GFP_NOFAIL, gfp_mask)) {
					*did_some_progress = 1;
					if (gfp_mask & __GFP_NOFAIL) page = __alloc_pages_cpuset_fallback(gfp_mask, order, ALLOC_NO_WATERMARKS, ac);
				}
			if (page)
				goto got_pg;
			if (tsk_is_oom_victim(current) && (alloc_flags & ALLOC_OOM || (gfp_mask & __GFP_NOMEMALLOC)))
				goto nopage;
			if (did_some_progress) { no_progress_loops = 0; goto retry; }
		  nopage:
			if (check_retry_cpuset(cpuset_mems_cookie, ac) || check_retry_zonelist(zonelist_iter_cookie))
				goto restart;
			if (gfp_mask & __GFP_NOFAIL) {
				page = __alloc_pages_cpuset_fallback(gfp_mask, order, ALLOC_MIN_RESERVE, ac); /**/
				if (page)
					goto got_pg;
				goto retry;
			}
		  fail:
		  got_pg:
			return page;
		}
	out:
		kmsan_alloc_page(page, order, alloc_gfp);
		return page;
		
alloc_pages(gfp_mask/gfp, order)
	...
	__alloc_pages_noprof(gfp_mask/gfp, order, nid, NULL); //__alloc_pages
		prepare_alloc_pages(gfp, order, preferred_nid, nodemask, &ac, &alloc_gfp, &alloc_flags))
		page = get_page_from_freelist(alloc_gfp, order, alloc_flags, &ac);
		page = __alloc_pages_slowpath(alloc_gfp, order, &ac); { /**/
		  restart:
			wake_all_kswapds(order, gfp_mask, ac); // if (alloc_flags & ALLOC_KSWAPD)
			page = get_page_from_freelist(gfp_mask, order, alloc_flags, ac);  /**/
			if (can_direct_reclaim && can_compact 
				&& (costly_order || (order > 0 && ac->migratetype != MIGRATE_MOVABLE))
				&& !gfp_pfmemalloc_allowed(gfp_mask))
				page = __alloc_pages_direct_compact(gfp_mask, order, alloc_flags, ac, INIT_COMPACT_PRIORITY, &compact_result);  /**/
				if (page)
					goto got_pg;
		  retry:
			wake_all_kswapds(order, gfp_mask, ac); // if (alloc_flags & ALLOC_KSWAPD)
			//trys /**/
			page = get_page_from_freelist(gfp_mask, order, alloc_flags, ac);
			page = __alloc_pages_direct_reclaim(gfp_mask, order, alloc_flags, ac, &did_some_progress);
			page = __alloc_pages_direct_compact(gfp_mask, order, alloc_flags, ac, compact_priority, &compact_result); {
			//retrys and restarts
			page = __alloc_pages_may_oom(gfp_mask, order, ac, &did_some_progress); /**/
			//retrys and restarts
		  nopage:
			if (check_retry_cpuset(cpuset_mems_cookie, ac) || check_retry_zonelist(zonelist_iter_cookie))
				goto restart;
			if (gfp_mask & __GFP_NOFAIL) {
				page = __alloc_pages_cpuset_fallback(gfp_mask, order, ALLOC_MIN_RESERVE, ac); /**/
				if (page)
					goto got_pg;
				goto retry;
			}
		  fail:
		  got_pg:
			return page;
		}
	out:
		kmsan_alloc_page(page, order, alloc_gfp);
		return page;


key functions:
	get_page_from_freelist(alloc_gfp, order, alloc_flags, &ac); { /**/
	node_reclaim(zone->zone_pgdat, gfp_mask, order); 
	zone_watermark_ok(zone, order, mark, ac->highest_zoneidx, alloc_flags))	;
	__alloc_pages_direct_compact(gfp_mask, order, alloc_flags, ac, INIT_COMPACT_PRIORITY, &compact_result);
		compact_zone_order(zone, order, gfp_mask, prio, alloc_flags, ac->highest_zoneidx, capture);
	__alloc_pages_direct_reclaim(gfp_mask, order, alloc_flags, ac, &did_some_progress);
	__alloc_pages_may_oom(gfp_mask, order, ac, &did_some_progress);
		out_of_memory(&oc)

##	get_page_from_freelist
get_page_from_freelist(alloc_gfp, order, alloc_flags, &ac);
  retry:
	no_fallback = alloc_flags & ALLOC_NOFRAGMENT;
	z = ac->preferred_zoneref;
	for_next_zone_zonelist_nodemask(zone, z, ac->highest_zoneidx, ac->nodemask) {
		//ALLOC_CPUSET: cpu set check
		//ac->spread_dirty_pages
		mark = wmark_pages(zone, alloc_flags & ALLOC_WMARK_MASK);
	}

##watermark: details
zone_watermark_fast(zone, order, mark, ac->highest_zoneidx, alloc_flags, gfp_mask)
	free_pages = zone_page_state(z, NR_FREE_PAGES);
	__zone_watermark_ok(z, order, mark, highest_zoneidx, alloc_flags, free_pages))
	alloc_flags : ALLOC_RESERVES, ALLOC_MIN_RESERVE, ALLOC_NON_BLOCK, ALLOC_OOM
	for (o = order; o < NR_PAGE_ORDERS; o++)
		struct free_area *area = &z->free_area[o];
		for (mt = 0; mt < MIGRATE_PCPTYPES; mt++)
			return !free_area_empty(area, mt);
		if (alloc_flags & ALLOC_CMA) 
			return !free_area_empty(area, MIGRATE_CMA));
		if (alloc_flags & (ALLOC_HIGHATOMIC|ALLOC_OOM))
			return free_area_empty(area, MIGRATE_HIGHATOMIC);
				 list_empty(&area->free_list[migratetype]);
	...		

##cma: CONFIG_CMA
core_initcall(cma_init_reserved_areas);
cma_init_reserved_areas(void)
	for (i = 0; i < cma_area_count; i++)
		cma_activate_area(struct cma *cma)
			init_cma_reserved_pageblock(pfn_to_page(pfn)); //pageblock_nr_pages
				do {
					__ClearPageReserved(p);
					set_page_count(p, 0);
				} while (++p, --i)
				set_pageblock_migratetype(page, MIGRATE_CMA); /**/
				set_page_refcounted(page);
				__free_pages(page, pageblock_order);
				adjust_managed_page_count(page, pageblock_nr_pages);
				page_zone(page)->cma_pages += pageblock_nr_pages;
				
#define ___GFP_MOVABLE→ →       0x08u
#define ___GFP_RECLAIMABLE→     0x10u
#define GFP_HIGHUSER_MOVABLE→   (GFP_HIGHUSER | __GFP_MOVABLE | __GFP_SKIP_KASAN)
#define GFP_MOVABLE_MASK (__GFP_RECLAIMABLE|__GFP_MOVABLE)
if (gfp_migratetype(gfp_mask) == MIGRATE_MOVABLE)
	alloc_flags |= ALLOC_CMA;

###__GFP_MOVABLE？
1. 用户空间内存分配
2. 透明大页（THP）
3. tmpfs / shmem

## node_reclaim & shrink
node_reclaim(zone->zone_pgdat, gfp_mask, order);
	if (node_pagecache_reclaimable(pgdat) <= pgdat->min_unmapped_pages &&
		node_page_state_pages(pgdat, NR_SLAB_RECLAIMABLE_B) <= pgdat->min_slab_pages)
		return NODE_RECLAIM_FULL;
	if (!gfpflags_allow_blocking(gfp_mask) || (current->flags & PF_MEMALLOC)) // !!(gfp_flags & __GFP_DIRECT_RECLAIM);
		return NODE_RECLAIM_NOSCAN; 
	if (node_state(pgdat->node_id, N_CPU) && pgdat->node_id != numa_node_id())
		return NODE_RECLAIM_NOSCAN;
	if (test_and_set_bit(PGDAT_RECLAIM_LOCKED, &pgdat->flags))
		return NODE_RECLAIM_NOSCAN;
	__node_reclaim(pgdat, gfp_mask, order);	
		struct scan_control sc = {
			.nr_to_reclaim = max(nr_pages, SWAP_CLUSTER_MAX),
			.gfp_mask = current_gfp_context(gfp_mask),
			.order = order,
			.priority = NODE_RECLAIM_PRIORITY,
			.may_writepage = !!(node_reclaim_mode & RECLAIM_WRITE),
			.may_unmap = !!(node_reclaim_mode & RECLAIM_UNMAP),
			.may_swap = 1,
			.reclaim_idx = gfp_zone(gfp_mask),
		};

shrink_node(pgdat, &sc);
	again:
	...
	shrink_node_memcgs(pgdat, sc);
		do {
			shrink_lruvec(lruvec, sc);
				while (nr[LRU_INACTIVE_ANON] || nr[LRU_ACTIVE_FILE] || nr[LRU_INACTIVE_FILE]) {
					for_each_evictable_lru(lru) {
						if (nr[lru]) {
							nr_to_scan = min(nr[lru], SWAP_CLUSTER_MAX);
							nr[lru] -= nr_to_scan;
							shrink_list(lru, nr_to_scan, lruvec, sc);
								if (is_active_lru(lru))
									//if (sc->may_deactivate & (1 << is_file_lru(lru)))
									shrink_active_list(nr_to_scan, lruvec, sc, lru);
								shrink_inactive_list(nr_to_scan, lruvec, sc, lru);
									...
									shrink_folio_list(&folio_list, pgdat, sc, &stat, false);
										TODO:...
									...
						}
					}
					...
				}
				if (can_age_anon_pages(lruvec_pgdat(lruvec), sc) && inactive_is_low(lruvec, LRU_INACTIVE_ANON))
					shrink_active_list(SWAP_CLUSTER_MAX, lruvec, sc, LRU_ACTIVE_ANON);
			shrink_slab(sc->gfp_mask, pgdat->node_id, memcg, sc->priority);
				if (!mem_cgroup_disabled() && !mem_cgroup_is_root(memcg))
					return shrink_slab_memcg(gfp_mask, nid, memcg, priority);
				list_for_each_entry(shrinker, &shrinker_list, list) {
					struct shrink_control sc = { .gfp_mask = gfp_mask, .nid = nid, .memcg = memcg, }
					do_shrink_slab(&sc, shrinker, priority);
						freeable = shrinker->count_objects(shrinker, shrinkctl);
						while (total_scan >= batch_size || total_scan >= freeable) {
							ret = shrinker->scan_objects(shrinker, shrinkctl);
							freed += ret;
						}
				}
		} while ((memcg = mem_cgroup_iter(target_memcg, memcg, NULL)));
	...
	if (should_continue_reclaim(pgdat, nr_node_reclaimed, sc))
		goto again;
	
register_shrinker(&shrinker_name, "xxx")
	__register_shrinker(shrinker);
		__prealloc_shrinker(shrinker);
		register_shrinker_prepared(shrinker);
			list_add_tail(&shrinker->list, &shrinker_list);

## kswapd
wakeup_kswapd(zone, gfp_mask, order, highest_zoneidx);
	wake_up_interruptible(&pgdat->kswapd_wait);

##cgroup（memcg）



#???
alloc_pages cma first?
cgroup 内存限制（memcg）
正常流程和失败流程
cond_accept_memory(zone, order);