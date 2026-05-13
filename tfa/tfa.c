
/* compile */
echo @tools/fiptool/fiptool create 
--tb-fw-config /home/zichar/qemu/optee/trusted-firmware-a/build/qemu/debug/fdts/tb_fw_config.dtb 
--tos-fw-config /home/zichar/qemu/optee/trusted-firmware-a/build/qemu/debug/fdts/spmc_el2_manifest.dtb 
--blob uuid=486178e0-e7f8-11e3-bc5e-0002a5d5c51b,file=/home/zichar/qemu/optee/trusted-firmware-a/build/qemu/debug/op-tee.pkg 
--tb-fw /home/zichar/qemu/optee/trusted-firmware-a/build/qemu/debug/bl2.bin 
--soc-fw /home/zichar/qemu/optee/trusted-firmware-a/build/qemu/debug/bl31.bin 
--tos-fw /home/zichar/qemu/optee/build/../hafnium/out/reference/secure_qemu_aarch64_clang/hafnium.bin 
--nt-fw /home/zichar/qemu/uefi/edk2/Build/ArmVirtQemuKernel-AARCH64/DEBUG_GCC5/FV/QEMU_EFI.fd
/home/zichar/qemu/optee/trusted-firmware-a/build/qemu/debug/fip.bin       


Trusted Boot Firmware BL2: offset=0x150, size=0x9B69, cmdline="--tb-fw"
EL3 Runtime Firmware BL31: offset=0x9CB9, size=0x130D4, cmdline="--soc-fw"
Secure Payload BL32 (Trusted OS): offset=0x1CD8D, size=0x2D1D0, cmdline="--tos-fw"
Non-Trusted Firmware BL33: offset=0x49F5D, size=0x300000, cmdline="--nt-fw"
TB_FW_CONFIG: offset=0x349F5D, size=0xE9, cmdline="--tb-fw-config"
TOS_FW_CONFIG: offset=0x34A046, size=0x421, cmdline="--tos-fw-config"
486178E0-E7F8-11E3-BC5E-0002A5D5C51B: offset=0x34A467, size=0xBB148, cmdline="--blob"
  
Built /home/zichar/qemu/optee/trusted-firmware-a/build/qemu/debug/fip.bin successfully

/* BL1 */
func bl1_entrypoint
	el3_entrypoint_common
	bl→     bl1_setup
	bl→     pauth_init_enable_el3
	bl→     bl1_main
	bl→     pauth_disable_el3
	b→      el3_exit  //#if ENABLE_RME: b→      bl1_run_bl2_in_root


void bl1_setup(void)
	bl1_early_platform_setup();
	bl1_plat_arch_setup(); //such as mem,page,mmu,lib...
	
void bl1_main(void)
	NOTICE(...)
	bl1_arch_setup();
	crypto_mod_init();
	auth_mod_init();
	bl1_plat_mboot_init();
	bl1_platform_setup();
	image_id = bl1_plat_get_next_image_id();
	bl1_load_bl2(); //or FWU
	bl1_plat_mboot_finish();
	bl1_prepare_next_image(image_id);

/* BL2 */
func bl2_entrypoint
	el3_entrypoint_common
	bl→     bl2_el3_setup
	bl→     pauth_init_enable_el3
	bl→     bl2_main

void bl2_el3_setup(arg0, arg1, arg2, arg3)
	cix_boot_perf_init(PBL_PHASE);
	cix_set_boot_phase(PBL_PHASE, RECORD_START);
	bl2_el3_early_platform_setup(arg0, arg1, arg2, arg3);
		cix_bl2_el3_early_platform_setup();
			cix_console_boot_init();
			plat_cix_io_setup();
				cix_io_setup(); // plat/cix/common/cix_io_storage.c
		plat_watchdog_init();
		generic_delay_timer_init(); 
	bl2_el3_plat_arch_setup();
		cix_bl2_el3_plat_arch_setup(); // plat/cix/sky1/sky1_bl2_el3_setup.c

void bl2_main(void)
	rlog_init_printf((char*)RAM_LOG_BL2_ADDR, RAM_LOG_BL2_SIZE, &g_r_ops);
	bl2_arch_setup();
	fwu_init();
	if (meta->config[KM_CFG_ENC_ENABLE] == 1)
		crypto_mod_init(); // drivers/auth/cix_sec/csec_crypto.c & drivers/auth/mbedtls/mbedtls_crypto.c & plat/cix/sky1/sky1_plat.c
	auth_mod_init();
	bl2_plat_mboot_init();
	bl2_plat_preload_setup();
	bl2_load_images();
	bl2_plat_mboot_finish();
	console_flush();
	smc(BL1_SMC_RUN_IMAGE, (unsigned long)next_bl_ep_info, 0, 0, 0, 0, 0, 0);
	cix_set_boot_phase(PBL_PHASE, RECORD_END);
	cix_boot_perf_uninit(PBL_PHASE);
	bl2_run_next_image(next_bl_ep_info);


static bl_mem_params_node_t bl2_mem_params_descs[] = {
	...
}
REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs)

IO:         cix_io_setup(); // plat/cix/common/cix_io_storage.c
arch_setup: void bl2_el3_plat_arch_setup(void) // plat/cix/sky1/sky1_bl2_el3_setup.c
mailbox




#define REGISTER_CRYPTO_LIB(_name, _init, _verify_signature, _verify_hash, \
→       →       →           _calc_hash, _auth_decrypt) \
	const crypto_lib_desc_t crypto_lib_desc = { ... }


plat/cix/sky1/sky1_plat.c:
:#if IMAGE_BL2
:int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
:int sky1_set_cpu_boost_trigger(int set)



/* bl31 */
# bl31 init
func bl31_entrypoint
	el3_entrypoint_common //RESET_TO_BL31?? //_exception_vectors=runtime_exceptions
	bl→     bl31_setup
	bl→     pauth_init_enable_el3
	bl→     bl31_main
	bl→     cix_set_boot_tfa_end
	b→      el3_exit
	
runtime_exceptions //bl31/aarch64/runtime_exceptions.S
	.globl	runtime_exceptions

	.globl	sync_exception_sp_el0
	.globl	irq_sp_el0
	.globl	fiq_sp_el0
	.globl	serror_sp_el0

	.globl	sync_exception_sp_elx
	.globl	irq_sp_elx
	.globl	fiq_sp_elx
	.globl	serror_sp_elx

	.globl	sync_exception_aarch64
	.globl	irq_aarch64
	.globl	fiq_aarch64
	.globl	serror_aarch64

	.globl	sync_exception_aarch32
	.globl	irq_aarch32
	.globl	fiq_aarch32
	.globl	serror_aarch32
	
void bl31_setup(arg0, arg1, arg2, arg3)
	cix_boot_perf_init(TFA_PHASE);
	cix_set_boot_phase(TFA_PHASE, RECORD_START);
	bl31_early_platform_setup2(arg0, arg1, arg2, arg3);
		cix_bl31_early_platform_setup(arg0, arg1, arg2, arg3);
			cix_console_boot_init();
	bl31_plat_arch_setup(); 
		MemOutputBuffer = GetMemOutputBuffer();
		dram1_size = MemOutputBuffer->AvailableSize;
		dram1_size *= 0x100000; // size unit is MB
		mmap_add_region(SKY1_NS_DRAM1_BASE, SKY1_NS_DRAM1_BASE, dram1_size - SKY1_TZC_DRAM1_SIZE, MT_MEMORY | MT_RW | MT_NS)
		setup_page_tables(bl_regions, plat_sky1_mmap);
		enable_mmu_el3(0);
		sky1_init_scmi_server();
	
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


# bl31 irq handle
sync_exception_aarch64
	sync_and_handle_pending_serror
	handle_sync_exception
		smc_handler64/smc_handler32
			prepare_el3_entry
			/* Load descriptor index from array of indices */
			adrp→   x14, rt_svc_descs_indices
			add→    x14, x14, :lo12:rt_svc_descs_indices
			ldrb→   w15, [x14, x16]
			tbnz→   w15, 7, smc_unknown ///* Any index greater than 127 is invalid. Check bit 7. */
			/* handler = (base + off) + (index << log2(size)) */
			adr→    x11, (__RT_SVC_DESCS_START__ + RT_SVC_DESC_HANDLE)
			lsl→    w10, w15, #RT_SVC_SIZE_LOG2
			ldr→    x15, [x11, w10, uxtw]
			blr→    x15
			b→      el3_exit
		b→      imp_def_el3_handler
		b→      enter_lower_el_sync_ea
		b→      report_unhandled_exception

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
		el3_exit //goto CTX_ELR_EL3 -> entrypoint = optee_vector_table->fiq_entry

//SPD
register_opteed_interrupt_handler(void)
	register_interrupt_type_handler(INTR_TYPE_S_EL1, opteed_sel1_interrupt_handler, flags);

tspd_smc_handler()
	register_interrupt_type_handler(INTR_TYPE_NS, tspd_ns_interrupt_handler, flags);

//SPMD
spmd_spmc_init(void *pm_addr)
	register_interrupt_type_handler(INTR_TYPE_S_EL1, spmd_secure_interrupt_handler, flags);
	register_interrupt_type_handler(INTR_TYPE_EL3, spmd_group0_interrupt_handler_nwd, flags);
	
	

opteed_sel1_interrupt_handler //intr_type_descs[type].handler
	cm_el1_sysregs_context_save(NON_SECURE);
	linear_id = plat_my_core_pos();
	optee_ctx = &opteed_sp_context[linear_id];
	cm_set_elr_el3(SECURE, (uint64_t)&optee_vector_table->fiq_entry);
		ctx = cm_get_context(security_state);
		state = get_el3state_ctx(ctx);
		write_ctx_reg(state, CTX_ELR_EL3, entrypoint); /**/
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);




???
cix_qspi_init(); //bl31/bl31_main.c


//
typedef uintptr_t (*rt_svc_handle_t)(uint32_t smc_fid,
				  u_register_t x1,
				  u_register_t x2,
				  u_register_t x3,
				  u_register_t x4,
				  void *cookie,
				  void *handle,
				  u_register_t flags);
				  
typedef struct rt_svc_desc {
	uint8_t start_oen;
	uint8_t end_oen;
	uint8_t call_type;
	const char *name;
	rt_svc_init_t init;
	rt_svc_handle_t handle;
} rt_svc_desc_t;

#define DECLARE_RT_SVC(_name, _start, _end, _type, _setup, _smch)	\
	static const rt_svc_desc_t __svc_desc_ ## _name			\
		__section(".rt_svc_descs") __used = {			\
			.start_oen = (_start),				\
			.end_oen = (_end),				\
			.call_type = (_type),				\
			.name = #_name,					\
			.init = (_setup),				\
			.handle = (_smch)				\
		}



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





















