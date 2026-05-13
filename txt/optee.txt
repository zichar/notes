/* optee */

# tfa setup
/* Define an OPTEED runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	opteed_fast,

	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_FAST,
	opteed_setup,
	opteed_smc_handler
);
/* Define an OPTEED runtime service descriptor for yielding SMC calls */
DECLARE_RT_SVC(
	opteed_std,

	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_YIELD,
	NULL,
	opteed_smc_handler
);

opteed_setup()
	optee_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	opteed_init_optee_ep_state(optee_ep_info, opteed_rw, optee_ep_info->pc, arg0, arg1, arg2, arg3, &opteed_sp_context[linear_id]);
		optee_ctx->mpidr = read_mpidr_el1();
		optee_ctx->state = 0;
		set_optee_pstate(optee_ctx->state, OPTEE_PSTATE_OFF);
		cm_set_context(&optee_ctx->cpu_ctx, SECURE);
		optee_entry_point->pc = pc;
		optee_entry_point->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS); 
		optee_entry_point->args.arg0 = arg0; //arg0, arg1, arg2, arg3
	bl31_register_bl32_init(&opteed_init);
		bl32_init = func;

opteed_init(void)
	optee_entry_point = bl31_plat_get_next_image_ep_info(SECURE);
	opteed_init_with_entry_point(optee_entry_point); //optee_entry_point = _start
		uint32_t linear_id = plat_my_core_pos();
		optee_context_t *optee_ctx = &opteed_sp_context[linear_id];
		cm_init_my_context(optee_entry_point/ep);
			ctx = cm_get_context(GET_SECURITY_STATE(ep->h.attr)); 
			cm_setup_context(ctx, ep);
				reg_ctx = get_regs_ctx(ctx);
				write_ctx_reg(reg_ctx, CTX_SCR, scr);
				write_ctx_reg(reg_ctx, CTX_LR, ep->pc);
				write_ctx_reg(reg_ctx, CTX_SPSR, ep->spsr);
				memcpy((void *)reg_ctx, (void *)&ep->args, sizeof(aapcs32_params_t));
		opteed_synchronous_sp_entry(optee_ctx);
			cm_el1_sysregs_context_restore(SECURE);
			cm_set_next_eret_context(SECURE);
			opteed_enter_sp(&optee_ctx->c_rt_ctx);
				... //regs
				b→      el3_exit //=> _start

## optee_vector_table = thread_vector_table


# init
core/arch/arm/kernel/entry_a64.S
FUNC _start
	set_sctlr_el1
	isb
	...
	b.gt→   copy_init
	b.lt→   clear_bss
	b.lt→   clear_nex_bss
	bl→     relocate
	... //Setup SP_EL0 and SP_EL1, SP will be set to SP_EL0
	bl→     console_init
	bl→     boot_save_args
	bl→     boot_mem_init
	bl→     boot_init_memtag
	bl→     core_init_mmu_map
	bl→     __get_core_pos
	bl→     enable_mmu
	bl→     console_init
	bl→     boot_clear_memtag
	bl→     core_mmu_set_default_prtn_tbl
	bl→     boot_init_primary_early
	init_memtag_per_cpu
	bl→     boot_init_primary_late
	bl→     boot_init_primary_runtime
	bl→     boot_init_primary_final
				call_finalcalls();
	bl →    thread_clr_boot_thread
#ifdef CFG_CORE_FFA
	adr→    x0, cpu_on_handler
	ldr→    x1, boot_mmu_config + CORE_MMU_CONFIG_MAP_OFFSET
	sub→    x0, x0, x1
	bl→     thread_spmc_register_secondary_ep
	b→      thread_ffa_msg_wait
#else
	ldr→    x0, boot_mmu_config + CORE_MMU_CONFIG_MAP_OFFSET
	adr→    x1, thread_vector_table
	sub→    x1, x1, x0
	mov→    x0, #TEESMC_OPTEED_RETURN_ENTRY_DONE //smc ->TEESMC_OPTEED_RETURN_ENTRY_DONE
	smc→    #0 // => tfa => opteed_smc_handler => optee_vector_table = (optee_vectors_t *) x1;
	/* SMC should not return */
	panic_at_smc_return
#endif

FUNC thread_vector_table , : , .identity_map, , nobti
	b	vector_std_smc_entry
	b	vector_fast_smc_entry
	b	vector_cpu_on_entry
	b	vector_cpu_off_entry
	b	vector_cpu_resume_entry
	b	vector_cpu_suspend_entry
	b	vector_fiq_entry
	b	vector_system_off_entry
	b	vector_system_reset_entry
END_FUNC thread_vector_table

sync_exception_aarch64
	sync_and_handle_pending_serror
	handle_sync_exception
		smc_handler64/smc_handler32
			prepare_el3_entry
			rt_svc_descs_indices[optee_index].handler() // = opteed_smc_handler;

opteed_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags)
	if (is_caller_non_secure(flags)) {
		...
		SMC_RETx()
	}
	/* Returning from OPTEE	*/
	switch (smc_fid) { 
	...
	case TEESMC_OPTEED_RETURN_ENTRY_DONE:
		optee_vector_table = (optee_vectors_t *) x1;
		set_optee_pstate(optee_ctx->state, OPTEE_PSTATE_ON);
		psci_register_spd_pm_hook(&opteed_pm);
		register_opteed_interrupt_handler();
		opteed_synchronous_sp_exit(optee_ctx, x1); 
			cm_el1_sysregs_context_save(SECURE);
			opteed_exit_sp(optee_ctx->c_rt_ctx, ret);
			/* Should never reach here */
			assert(0);
		break;
	...
	}
	...

func opteed_exit_sp
	/* Restore the previous stack */
	mov	sp, x0

	/* Restore callee-saved registers on to the stack */
	ldp	x19, x20, [x0, #(OPTEED_C_RT_CTX_X19 - OPTEED_C_RT_CTX_SIZE)]
	ldp	x21, x22, [x0, #(OPTEED_C_RT_CTX_X21 - OPTEED_C_RT_CTX_SIZE)]
	ldp	x23, x24, [x0, #(OPTEED_C_RT_CTX_X23 - OPTEED_C_RT_CTX_SIZE)]
	ldp	x25, x26, [x0, #(OPTEED_C_RT_CTX_X25 - OPTEED_C_RT_CTX_SIZE)]
	ldp	x27, x28, [x0, #(OPTEED_C_RT_CTX_X27 - OPTEED_C_RT_CTX_SIZE)]
	ldp	x29, x30, [x0, #(OPTEED_C_RT_CTX_X29 - OPTEED_C_RT_CTX_SIZE)]

	/* ---------------------------------------------
	 * This should take us back to the instruction
	 * after the call to the last opteed_enter_sp().
	 * Place the second parameter to x0 so that the
	 * caller will see it as a return value from the
	 * original entry call
	 * ---------------------------------------------
	 */
	mov	x0, x1
	ret
endfunc opteed_exit_sp





# app call
opteed_smc_handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags)	
	if (is_caller_non_secure(flags)) {
		if (GET_SMC_TYPE(smc_fid) == SMC_TYPE_FAST)
			cm_set_elr_el3(SECURE, (uint64_t)&optee_vector_table->fast_smc_entry);
		else
			cm_set_elr_el3(SECURE, (uint64_t)&optee_vector_table->yield_smc_entry);
		cm_el1_sysregs_context_restore(SECURE);
		cm_set_next_eret_context(SECURE);
		... //restore CTX_GPREG_X4, X5, X6, X7
		SMC_RET4(&optee_ctx->cpu_ctx, smc_fid, x1, x2, x3);
	}

FUNC vector_std_smc_entry
	bl→     thread_handle_std_smc
	ldr→    r0, =TEESMC_OPTEED_RETURN_CALL_DONE
	smc→    #0
	/* SMC should not return */
	panic_at_smc_return
END_FUNC vector_std_smc_entry

FUNC vector_fast_smc_entry
	bl→     thread_handle_fast_smc
	ldr→    r0, =TEESMC_OPTEED_RETURN_CALL_DONE
	smc→    #0
	/* SMC should not return */
	panic_at_smc_return
END_FUNC vector_fast_smc_entry

a0 = OPTEE_SMC_CALL_WITH_REGD_ARG, OPTEE_SMC_CALL_WITH_RPC_ARG, OPTEE_SMC_CALL_WITH_ARG

uint32_t thread_handle_std_smc(a0, a1, a2, a3, a4, a5, a6, a7)
	/*
	 * thread_resume_from_rpc() and thread_alloc_and_run() only return
	 * on error. Successful return is done via thread_exit() or
	 * thread_rpc().
	 */
	if (a0 == OPTEE_SMC_CALL_RETURN_FROM_RPC) {
		thread_resume_from_rpc(a3/thread_id, a1, a2, a4, a5);
			l->curr_thread = thread_id;
			l->flags &= ~THREAD_CLF_TMP;
			thread_resume(&threads[thread_id].regs);
		rv = OPTEE_SMC_RETURN_ERESUME;
	} else {
		thread_alloc_and_run(a0, a1, a2, a3, 0, 0);
			__thread_alloc_and_run(a0, a1, a2, a3, a4, a5, 0, 0, thread_std_smc_entry, 0);
				for (n = 0; n < CFG_NUM_THREADS; n++) {
					if (threads[n].state == THREAD_STATE_FREE)
						threads[n].state = THREAD_STATE_ACTIVE;
						found_thread = true;
						break;
				}
				init_regs(threads + n, a0, a1, a2, a3, a4, a5, a6, a7, pc);
				l->flags &= ~THREAD_CLF_TMP;
				thread_resume(&threads[n].regs);
					thread_std_smc_entry
						__thread_std_smc_entry
							std_smc_entry(a0, a1, a2, a3);
								switch (a0) {
								case  OPTEE_SMC_CALL_WITH_ARG/OPTEE_SMC_CALL_WITH_RPC_ARG:
									std_entry_with_parg(reg_pair_to_64(a1, a2), !with_rpc_arg/with_rpc_arg);
										//if (core_pbuf_is(CORE_MEM_NSEC_SHM, parg, sz))
										call_entry_std(arg, num_params, rpc_arg);
											tee_entry_std(arg, num_params)
												__tee_entry_std(arg, num_params);
								case OPTEE_SMC_CALL_WITH_REGD_ARG:
									std_entry_with_regd_arg(reg_pair_to_64(a1, a2), a3);
								default: error;
								}
						thread_state_free
							tee_pager_release_phys((void *)(threads[ct].stack_va_end - STACK_THREAD_SIZE), STACK_THREAD_SIZE);
							threads[ct].state = THREAD_STATE_FREE;
							threads[ct].flags = 0;
							l->curr_thread = THREAD_ID_INVALID;
						ffa_msg_send_direct_resp
		rv = OPTEE_SMC_RETURN_ETHREAD_LIMIT;
	}
	return rv

__tee_entry_std(arg, num_params);
	//TODO:
	//eg: case OPTEE_MSG_CMD_OPEN_SESSION: ts_ctx->ops->enter_open_session(&s->ts_sess); 

void thread_handle_fast_smc(struct thread_smc_args *args) 
	tee_entry_fast(args);
		__tee_entry_fast(args); //core/arch/arm/tee/entry_fast.c
			//TODO
			
	
//others
opteed_setup
#if OPTEE_ALLOW_SMC_LOAD
	register_opteed_interrupt_handler();
		register_interrupt_type_handler(INTR_TYPE_S_EL1, opteed_sel1_interrupt_handler, flags);
#endif 

irq:
opteed_sel1_interrupt_handler
	linear_id = plat_my_core_pos();
	optee_ctx = &opteed_sp_context[linear_id];
	cm_set_elr_el3(SECURE, (uint64_t)&optee_vector_table->fiq_entry); 
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);
	SMC_RET1(&optee_ctx->cpu_ctx, read_elr_el3());





#second cpu
psci

#linux kernel driver
drivers/tee/optee/smc_abi.c
optee->ops = &optee_ops;
optee->smc.invoke_fn = invoke_fn;



??

psci power on second cpu
https://zhuanlan.zhihu.com/p/556039631


https://zhuanlan.zhihu.com/p/559219659