# TF-A QEMU `virt` 冷启动调用流程

适用：**`PLAT=qemu`**、**AArch64**、常见配置：`!RESET_TO_BL2`、`!BL2_RUNS_AT_EL3`、`!ENABLE_RME`。

- 说明用行尾 `//` 注释。
- 汇编 **`bl`** 标在目标前；**`b` / `smc` / exception_return** 非 `bl`。
- **`el3_entrypoint_common`** 为宏，随版本与 defconfig 变化；**`bl31_plat_arch_setup` / `gic_*`** 同理。

---

## 1. 从 `bl1_entrypoint` 到 BL33 的调用树

```
bl1_entrypoint                                    // bl1/aarch64/bl1_entrypoint.S
  el3_entrypoint_common ...                       // el3_common_macros.S：栈/BSS/C 运行时/向量等
  bl bl1_main

bl1_main                                          // bl1/bl1_main.c
  plat_setup_early_console()
  bl1_early_platform_setup()                      // qemu_bl1_setup.c：qemu_console_init；TZRAM
  bl1_plat_arch_setup()                           // setup_page_tables；enable_mmu_el3(0)
  cm_manage_extensions_el3(plat_my_core_pos())
  cm_manage_extensions_per_world()
  bl1_arch_setup()                                // bl1/aarch64/bl1_arch_setup.c：SCR_RW_BIT
  crypto_mod_init()                               // crypto_lib_desc.init()
  auth_mod_init()                                 // img_parser_init()
  bl1_plat_mboot_init()
  bl1_platform_setup()
    plat_qemu_io_setup()                          // qemu_io_storage.c
      register_io_dev_fip(&fip_dev_con)
        io_register_device(&dev_info_pool[0])
      register_io_dev_memmap(&memmap_dev_con)
        io_register_device(&memmap_dev_info)
      io_dev_open(fip_dev_con, NULL, &fip_dev_handle)
        fip_dev_open -> allocate_dev_info
      io_dev_open(memmap_dev_con, NULL, &memmap_dev_handle)
        memmap_dev_open -> &memmap_dev_info
      register_io_dev_sh(&sh_dev_con)
      io_dev_open(sh_dev_con, NULL, &sh_dev_handle)
  bl1_plat_get_next_image_id()                    // 通常 BL2_IMAGE_ID
  bl1_load_bl2()
    bl1_plat_get_image_desc(BL2_IMAGE_ID)
    bl1_plat_handle_pre_image_load(BL2_IMAGE_ID)
    load_auth_image(BL2_IMAGE_ID, &desc->image_info)
      load_auth_image_internal
        load_image                                  // common/bl_common.c
          plat_get_image_source(BL2, &dev, &spec)   // open_fip 或 semihosting
            [FIP] io_dev_init(fip_dev_handle, FIP_IMAGE_ID)
              fip_dev_init -> plat_get_image_source(FIP) -> io_open/io_read FIP 头
            io_open(fip_dev_handle, uuid_spec, ...)
              fip_file_open -> 扫 ToC 匹配 UUID
            io_size / io_read
              fip_file_read -> io_open(backend) -> io_seek -> io_read -> memmap_block_read
          io_close / io_dev_close
      [可选 TBBR] load_auth_image_recursive + auth_mod_verify_img
      plat_mboot_measure_image(...)
      flush_dcache_range(...)
    bl1_plat_handle_post_image_load(BL2_IMAGE_ID)
      bl1_plat_calc_bl2_layout；ep_info.args.arg1 = meminfo 指针
  bl1_plat_mboot_finish()
  crypto_mod_finish()
  bl1_prepare_next_image(BL2_IMAGE_ID)
    cm_init_my_context(next_bl_ep) -> cm_setup_context
    cm_prepare_el3_exit(SECURE)
  console_flush()
  pauth_disable_el3()
  return
  b el3_exit                                        // context.S -> ERET 进 BL2

bl2_entrypoint                                    // bl2/aarch64/bl2_entrypoint.S
  bl inv_dcache_range
  bl zeromem
  [USE_COHERENT_MEM] bl zeromem
  bl plat_set_my_stack
  [STACK_PROTECTOR] bl update_stack_protector_canary
  mov x0..x3 <- x20..x23
  bl bl2_main

bl2_main                                          // bl2/bl2_main.c
  plat_setup_early_console()
  bl2_early_platform_setup2()                     // qemu：qemu_console_init；plat_qemu_io_setup
  bl2_plat_arch_setup()                           // qemu：页表；enable_mmu_el1(0)
  pauth_init_enable_el1()
  bl2_arch_setup()                                // write_cpacr（FP/SIMD）
  crypto_mod_init()
  auth_mod_init()
  bl2_plat_mboot_init()
  bl2_plat_preload_setup()
  bl2_load_images()                                 // bl2/bl2_image_load_v2.c
    plat_get_bl_image_load_info()                   // bl_load_info_t：待加载镜像链表
      get_bl_load_info_from_mem_params_desc()       // 由 REGISTER_BL_IMAGE_DESCS 等生成
    while (bl2_node_info):
      [IMAGE_ATTRIB_PLAT_SETUP] bl2_platform_setup() // 至多一次；重复则 WARN
      bl2_plat_handle_pre_image_load(image_id)
      [非 IMAGE_ATTRIB_SKIP_LOADING] load_auth_image(image_id, image_info)
      [SKIP_LOADING] 仅打日志，不读 Flash/FIP
      bl2_plat_handle_post_image_load(image_id)     // 填 ep、布局等
      bl2_node_info = next_load_info
    plat_get_next_bl_params()                       // bl_params_t：交给下一级的执行链
      get_next_bl_params_from_mem_params_desc()
    [若 ep_info->args.arg0 == 0] arg0 = bl2_to_next_bl_params  // 下一镜像可取整链指针
    plat_flush_next_bl_params()                     // 缓存一致性等
    return head->ep_info                            // AArch64 多为 BL31
  bl2_plat_mboot_finish()
  crypto_mod_finish()
  pauth_disable_el1()
  console_flush()
  smc(BL1_SMC_RUN_IMAGE, next_bl_ep_info, ...)    // 进 EL3

smc_handler64                                     // bl1/aarch64/bl1_exceptions.S（RUN_IMAGE）
  bl bl1_print_next_bl_ep_info
  // elr_el3/spsr_el3 <- entry_point_info（目标 EL3 = BL31）
  bl disable_mmu_icache_el3
  tlbi alle3
  bl bl1_plat_prepare_exit
  exception_return                                // 进 BL31

bl31_entrypoint                                   // bl31/aarch64/bl31_entrypoint.S
  el3_entrypoint_common ...
  mov x0..x3 <- x20..x23
  bl bl31_main

bl31_main                                         // bl31/bl31_main.c
  plat_setup_early_console()
  bl31_early_platform_setup2()                    // qemu：解析 bl_params，拷贝 BL32/BL33 ep
  bl31_plat_arch_setup()
  detect_arch_features(...)                       // 若 FEATURE_DETECTION
  report_ctx_memory_usage()
  cm_manage_extensions_el3 / per_world
  bl31_platform_setup()
  [USE_GIC_DRIVER] gic_init / gic_pcpu_init / gic_cpuif_enable
  bl31_lib_init() -> cm_init()
  [EL3_EXCEPTION_HANDLING] ehf_init()
  runtime_svc_init()
    for each rt_svc_desc: validate_rt_svc_desc；service->init()；填 rt_svc_descs_indices[]
  if (bl32_init) (*bl32_init)()
  bl31_prepare_next_image_entry()
    bl31_plat_get_next_image_ep_info(...)
    cm_init_my_context -> cm_setup_context
    cm_prepare_el3_exit_ns() 或 cm_prepare_el3_exit(SECURE)
  bl31_plat_runtime_setup()
  console_switch_state(CONSOLE_FLAG_RUNTIME)
  return
  bl clean_dcache_range  (×3 段)
  [PLATFORM_NODE_COUNT>1] bl plat_per_cpu_dcache_clean
  b el3_exit
    bl restore_gp_pmcr_pauth_regs
    exception_return                              // ERET -> BL33（或经 BL32，由策略决定）
```

### `bl2_load_images` 说明

- **职责**：按平台给出的「加载描述」依次处理 SCP_BL2、BL31、BL32、BL33 等条目（具体列表由 **`REGISTER_BL_IMAGE_DESCS`** 与平台内存布局决定），把镜像读入 RAM（或跳过加载），最后返回 **下一跳** 的 **`entry_point_info_t *`**（AArch64 上通常是 **BL31**）。
- **输入来源**：**`plat_get_bl_image_load_info()`** 返回带 **`PARAM_BL_LOAD_INFO`** 头的 **`bl_load_info_t`**，其 **`head`** 为 **`bl_load_info_node_t`** 链表；每个节点含 **`image_id`**、**`image_info`**（加载地址、大小、属性位等）。
- **循环内顺序**：若节点带 **`IMAGE_ATTRIB_PLAT_SETUP`**，则在本阶段首次调用 **`bl2_platform_setup()`**；然后 **`bl2_plat_handle_pre_image_load`** →（可选）**`load_auth_image`**（与 BL1 加载 BL2 同一路径，可含 TBBR 认证）→ **`bl2_plat_handle_post_image_load`**。带 **`IMAGE_ATTRIB_SKIP_LOADING`** 的镜像只走 pre/post，不执行 **`load_auth_image`**。
- **输出与 handoff**：**`plat_get_next_bl_params()`** 得到 **`bl_params_t`**（**`PARAM_BL_PARAMS`**），其 **`head`** 链起各阶段的 **`ep_info`**。若 **`head->ep_info->args.arg0`** 仍为 0，则填入 **`bl_params` 指针**，便于 BL31 解析整条链（如 BL32/BL33）。**`plat_flush_next_bl_params()`** 后返回 **`head->ep_info`**，供 **`bl2_main`** 里 **`smc(BL1_SMC_RUN_IMAGE, …)`** 交给 EL3 再进 BL31。

---

## 2. 运行时（EL3 常驻，概要）

```
lower EL: smc
  -> runtime_exceptions -> handle_sync_exception -> smc_handler64
       -> handle_runtime_svc -> rt_svc_descs[index].handle(...)

lower EL: irq/fiq
  -> handle_interrupt_exception -> plat_ic_get_pending_interrupt_type
       -> registered handler
```

---

## 3. 变体说明

- **`TRUSTED_BOARD_BOOT`**：`load_auth_image` 走递归认证 + `auth_mod_verify_img`。
- **`BL2_RUNS_AT_EL3`**：`bl2_main` 末尾 `bl2_run_next_image`（内有 `bl disable_mmu_icache_el3` 等），不经 `smc(BL1_SMC_RUN_IMAGE)`。
- **`RESET_TO_BL31`**：`bl31_entrypoint` 前半与 `el3_entrypoint_common` 参数不同。
- **`ENABLE_RME`**：`bl1_entrypoint` 可能 `b bl1_run_bl2_in_root`；另有 `rmm_init` 等分支。
