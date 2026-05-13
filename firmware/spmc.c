/* spmc / hanfinum */
#Makefile
SPD=spmc
ifneq (${SPD},none)
	ifeq (${SPD},spmd)
		SPD_DIR := std_svc
	else
		SPD_DIR := spd //optee
		
#tfa
void bl31_setup(arg0, arg1, arg2, arg3)
	bl31_early_platform_setup2(arg0, arg1, arg2, arg3);
#ifdef BL32_BASE
		SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
		SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
		bl32_image_ep_info.pc = BL32_BASE;
#endif


void bl31_main(void)
	detect_arch_features(); 
	bl31_platform_setup();
	bl31_lib_init();
	ehf_init();
	runtime_svc_init();
		rt_svc_descs = (rt_svc_desc_t *) RT_SVC_DESCS_START;
		for (index = 0U; index < RT_SVC_DECS_NUM; index++) {
			rt_svc_desc_t *service = &rt_svc_descs[index];
			validate_rt_svc_desc(service);
			service->init(); //if !NULL
			for (; start_idx <= end_idx; start_idx++)
				rt_svc_descs_indices[start_idx] = index;
		}
	if (bl32_init != NULL)
		(*bl32_init)()
	if (rmm_init != NULL)
		(*rmm_init)();
	bl31_prepare_next_image_entry();
	bl31_plat_runtime_setup();

/* Register Standard Service Calls as runtime service */
DECLARE_RT_SVC(
		std_svc,

		OEN_STD_START,
		OEN_STD_END,
		SMC_TYPE_FAST,
		std_svc_setup,
		std_svc_smc_handler
);

##
std_svc_setup(void)
	psci_setup((const psci_lib_args_t *)svc_arg);
	spm_mm_setup(); #if SPM_MM
	spmd_setup(); #if defined(SPD_spmd)
		if (is_spmc_at_el3())
			spmc_setup();
			return 0;
		spmc_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
			return &bl32_image_ep_info; //if ((type == SECURE) && bl32_image_ep_info.pc)
		spmc_manifest = (void *)spmc_ep_info->args.arg0;
		spmd_spmc_init(spmc_manifest/pm_addr);
			plat_spm_core_manifest_load(&spmc_attrs/manifest/attr, pm_addr);
				... //TODO
				mmap_add_dynamic_region((unsigned long long)pm_base_align, pm_base_align, PAGE_SIZE, MT_RO_DATA | EL3_PAS);
				manifest_parse_attribute(manifest, fdt, node);
				fdt_check_header(pm_addr);
				fdt_node_offset_by_compatible(pm_addr, -1, "arm,ffa-core-manifest-1.0");
				manifest_parse_root(manifest, pm_addr, rc);
					fdt_subnode_offset_namelen(fdt, root, "attribute", sizeof("attribute") - 1);
					manifest_parse_attribute(manifest, fdt, node);
						...
						fdt_read_uint32(fdt, node, "spmc_id", &val32);
						fdt_read_uint64(fdt, node, "load_address", &attr->load_address);
						fdt_read_uint64(fdt, node, "entrypoint", &attr->entrypoint);
						...
			SET_PARAM_HEAD(spmc_ep_info, PARAM_EP, VERSION_1, ep_attr);
			for (core_id = 0U; core_id < PLATFORM_CORE_COUNT; core_id++)
				spm_core_context[core_id].state = SPMC_STATE_OFF;
				cpu_ctx = &spm_core_context[core_id].cpu_ctx;
				cm_setup_context(cpu_ctx, spmc_ep_info/ep); /**/
					setup_context_common(ctx, ep);
						//TODO:
						state = get_el3state_ctx(ctx); // (&((cpu_context_t *) h)->el3state_ctx)
						...
						write_ctx_reg(state, CTX_SCR_EL3, scr_el3);
						write_ctx_reg(state, CTX_ELR_EL3, ep->pc); /* ELR/pc */
						write_ctx_reg(state, CTX_SPSR_EL3, ep->spsr);
						...
					switch (GET_SECURITY_STATE(ep->h.attr)) {
					case SECURE: setup_secure_context(ctx, ep); break;
					case REALM: setup_realm_context(ctx, ep); break;
					case NON_SECURE: setup_ns_context(ctx, ep); break;
					}
				write_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X4, core_id);
			psci_register_spd_pm_hook(&spmd_pm);
			bl31_register_bl32_init(&spmd_init);
				bl32_init = spmd_init; 
			register_interrupt_type_handler(INTR_TYPE_S_EL1, spmd_secure_interrupt_handler, flags);
			if (plat_ic_has_interrupt_type(INTR_TYPE_EL3))
				register_interrupt_type_handler(INTR_TYPE_EL3, spmd_group0_interrupt_handler_nwd, flags);
	rmmd_setup(); #if ENABLE_RME
	sdei_init(); #if SDEI_SUPPORT
	trng_setup(); #if TRNG_SUPPORT
	drtm_setup(); #if DRTM_SUPPORT
	

spmd_init(void)
	spmd_spm_core_context_t *ctx = spmd_get_context();
		return &spm_core_context[plat_my_core_pos()];
	ctx->state = SPMC_STATE_ON_PENDING;
	spmd_spm_core_sync_entry(ctx/spmc_ctx);
		cm_set_context(&(spmc_ctx->cpu_ctx), SECURE);
			set_cpu_data(cpu_context[get_cpu_context_index(security_state)], context);
		cm_el2_sysregs_context_restore(SECURE); //or el1
			el2_sysregs_ctx = get_el2_sysregs_ctx(cm_get_context(security_state));
			el2_sysregs_context_restore_common(el2_sysregs_ctx);
				...
				write_elr_el2(read_el2_ctx_common(ctx, elr_el2)); //ELR
				...
			el2_sysregs_context_restore_gic(el2_sysregs_ctx);
			...//other regs
		cm_set_next_eret_context(SECURE);
			ctx = cm_get_context(security_state); //ctx = bl1_cpu_context_ptr[security_state]
			cm_set_next_context(ctx);
				__asm__ volatile("msr→  spsel, #1\n" "mov→  sp, %0\n" "msr→  spsel, #0\n" : : "r" (context));
		spmd_spm_core_enter(&spmc_ctx->c_rt_ctx);
			b→      el3_exit
		cm_el2_sysregs_context_save(SECURE); //or el1
	ctx->state = SPMC_STATE_ON; 
	spmd_logical_sp_set_spmc_initialized();
	spmd_logical_sp_init();
	
##irq:
runtime_exceptions //bl31/aarch64/runtime_exceptions.S
	.globl	runtime_exceptions
	...
	.globl	sync_exception_aarch64
	.globl	irq_aarch64
	.globl	fiq_aarch64
	.globl	serror_aarch64
	...
	
irq_aarch64/fiq_aarch64
	apply_at_speculative_wa
	sync_and_handle_pending_serror //ISR_EL1
	handle_interrupt_exception
		prepare_el3_entry
		type = plat_ic_get_pending_interrupt_type() //INTR_TYPE_S_EL1, INTR_TYPE_NS, INTR_TYPE_INVAL, INTR_TYPE_EL3
			read_icc_hppir0_el1() & HPPIR0_EL1_INTID_MASK
		get_interrupt_type_handler(type)
			return intr_type_descs[type].handler; //registered by register_interrupt_type_handler()
		bl x21 //bl intr_type_descs[type].handler
		el3_exit //goto CTX_ELR_EL3 


spmd_secure_interrupt_handler(id, flags, handle, cookie)
	spmd_spm_core_context_t *ctx = spmd_get_context();
	gp_regs_t *gpregs = get_gpregs_ctx(&ctx->cpu_ctx);
	cm_el2_sysregs_context_save(NON_SECURE); //or el1
	write_ctx_reg(gpregs, CTX_GPREG_X0, FFA_INTERRUPT);
	write_ctx_reg(gpregs, CTX_GPREG_X1, 0);
	...// X2, X3, X4, X5, X6, X7
	ctx->secure_interrupt_ongoing = true;
	spmd_spm_core_sync_entry(ctx);
	ctx->secure_interrupt_ongoing = false;
	cm_el2_sysregs_context_restore(NON_SECURE);
	cm_set_next_eret_context(NON_SECURE);
	SMC_RET0(&ctx->cpu_ctx);

spmd_group0_interrupt_handler_nwd(id, flags, handle, cookie)
	intid = plat_ic_acknowledge_interrupt();
		return gicv3_acknowledge_interrupt_sel1();
	plat_spmd_handle_group0_interrupt(intid);
	plat_ic_end_of_interrupt(intid);

##
std_svc_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags)
	if (is_psci_fid(smc_fid))
		psci_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags);
		SMC_RET1(handle, ret);
	...
	spmd_ffa_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags); //if (is_ffa_fid(smc_fid))
		spmd_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags);	
			switch (smc_fid) {
				case ...:
				...
				spmd_smc_forward(smc_fid, secure_origin, x1, x2, x3, x4, cookie, handle, flags);
					spmd_smc_switch_state(smc_fid, secure_origin, x1, x2, x3, x4,  handle, flags);
						unsigned int secure_state_out = (!secure_origin) ? SECURE : NON_SECURE;
						cm_el2_sysregs_context_save(secure_state_in); //or el1
						cm_set_next_eret_context(secure_state_out); 
						ctx_out = cm_get_context(secure_state_out);
						SMC_RET18(ctx_out, smc_fid, x1, x2, x3, x4,
									SMC_GET_GP(handle, CTX_GPREG_X5),
									...
									SMC_GET_GP(handle, CTX_GPREG_X17),
									);
				case FFA_MSG_WAIT: 
					if (secure_origin && (ctx->state == SPMC_STATE_ON_PENDING))
						spmd_spm_core_sync_exit(0ULL);
							spmd_spm_core_exit(ctx->c_rt_ctx, rc); //to spmd_spm_core_enter(&spmc_ctx->c_rt_ctx);
								...
								ret
							panic();
					...
				...
				break;
			}
	...


#hanfinum
##init
build/image/image.ld
src/arch/aarch64/entry.S
entry: //.section .init.entry, "ax"
	...
	b image_entry

image_entry: //src/arch/aarch64/hypervisor/hypervisor_entry.S
	bl plat_boot_flow_hook
	bl prepare_for_c
		/* Use SPx (instead of SP0). */
		msr spsel, #1
		/* Prepare the stack. */
		ldr x1, [x0, #CPU_STACK_BOTTOM]
		mov sp, x1
		/* Configure exception handlers. */
		msr vbar_el2, x2
		ret
	bl stack_protector_init
	bl one_time_init_mm
		plat_console_init();
		plat_ffa_log_init();
		mpool_init(&ppool, MM_PPOOL_ENTRY_SIZE);
		mpool_add_chunk(&ppool, ptable_buf, sizeof(ptable_buf));
		mm_init(&ppool);
	bl one_time_debug_print
	bl mm_enable
	bl one_time_init
		fdt_map(&fdt, mm_stage1_locked, plat_boot_flow_get_fdt_addr(), &ppool);
		boot_flow_get_params(params, &fdt); //initrd, cpus, memeory, ns-memory, device-memory, ns-device-memory
		manifest_init(mm_stage1_locked, &manifest, &manifest_it, params, &ppool); //fdt or initrd
		plat_iommu_init(&fdt, mm_stage1_locked, &ppool);
		cpu_module_init(params->cpu_ids, params->cpu_count);
		plat_interrupts_controller_driver_init(&fdt, mm_stage1_locked, &ppool);
		fdt_unmap(&fdt, mm_stage1_locked, &ppool);
		load_vms(mm_stage1_locked, manifest, &cpio, params, &update, &ppool);
			//TODO:
		boot_flow_update(mm_stage1_locked, manifest, &update, &cpio, &ppool);
		mpool_free(&ppool, params);
		manifest_deinit(&ppool);
		mm_vm_enable_invalidation();
		plat_ffa_init(&ppool);
		api_init(&ppool);
	b cpu_init
		bl cpu_main
		bl vcpu_restore_all_and_run
		0:→     wfi //not supposed to be here
		b 0b

vbar_el2:


cpu_main:


vcpu_restore_all_and_run:
	//TODO:
	...
	smc #0







#second cpu
psci
static void spmc_cpu_on_finish_handler(u_register_t unused) 
	spmc_sp_common_ep_commit(sp, &sec_ec_ep_info);
	
int psci_cpu_on_start(u_register_t target_cpu,
	cm_init_context_by_index(target_idx, ep);

https://static.linaro.org/connect/lvc21f/presentations/LVC21F-117.pdf



