# TF-A QEMU `virt` 冷启动调用流程（简版）

适用：**`PLAT=qemu`**、**AArch64**、常见配置：`!RESET_TO_BL2`、`!BL2_RUNS_AT_EL3`、`!ENABLE_RME`。

汇编里**显式 `bl`** 已标出；`b` / `smc` / ERET 与 `bl` 不同，文中单独说明。

---

## 1. 三阶段串览

```text
BL1: bl1_entrypoint
       el3_entrypoint_common   // 宏，版本相关
       bl      bl1_main
       b       el3_exit        // 进入 BL2（非 bl）

BL2: bl2_entrypoint
       bl      inv_dcache_range
       bl      zeromem
       bl      zeromem         // USE_COHERENT_MEM
       bl      plat_set_my_stack
       bl      update_stack_protector_canary   // 若开启
       bl      bl2_main

BL2 C: bl2_main
       ... bl2_load_images ...
       smc(BL1_SMC_RUN_IMAGE, next_bl_ep_info, ...)   // AArch64 常见，非 bl

BL31: bl31_entrypoint
       el3_entrypoint_common
       bl      bl31_main
       bl      clean_dcache_range   // .data
       bl      clean_dcache_range   // .bss
       bl      clean_dcache_range   // per-cpu
       bl      plat_per_cpu_dcache_clean   // PLATFORM_NODE_COUNT>1
       b       el3_exit        // 进入 BL33（或经 BL32，由上下文决定）
```

---

## 2. BL1（C 主顺序 + QEMU I/O）

```text
bl1_main
  plat_setup_early_console()
  bl1_early_platform_setup()       // qemu: console 等
  bl1_plat_arch_setup()             // qemu: MMU/页表
  cm_manage_extensions_el3(...)
  cm_manage_extensions_per_world()  // 若 BL2 不在 EL3
  bl1_arch_setup()
  crypto_mod_init()
  auth_mod_init()
  bl1_plat_mboot_init()
  bl1_platform_setup()              // qemu -> plat_qemu_io_setup()
  bl1_plat_get_next_image_id()
  bl1_load_bl2()
    bl1_plat_handle_pre_image_load(BL2)
    load_auth_image(BL2)            // -> plat_get_image_source / io_*
    bl1_plat_handle_post_image_load(BL2)
  bl1_plat_mboot_finish()
  crypto_mod_finish()
  bl1_prepare_next_image(image_id)
  console_flush()
  pauth_disable_el3()               // 若支持
  return -> entrypoint -> b el3_exit
```

**QEMU I/O 注册（在 `bl1_platform_setup` → `plat_qemu_io_setup`）：**

```text
register_io_dev_fip -> io_dev_open(fip)
register_io_dev_memmap -> io_dev_open(memmap)
register_io_dev_sh -> io_dev_open(sh)
```

---

## 3. BL2（C 主顺序 + QEMU）

```text
bl2_main(arg0..arg3)
  plat_setup_early_console()
  bl2_early_platform_setup2()       // qemu: qemu_console_init + plat_qemu_io_setup()
  bl2_plat_arch_setup()             // qemu: qemu_bl2_setup.c
  pauth_init_enable_el1()           // 若支持
  bl2_arch_setup()
  crypto_mod_init()
  auth_mod_init()
  bl2_plat_mboot_init()
  bl2_plat_preload_setup()
  next_bl_ep_info = bl2_load_images()
  bl2_plat_mboot_finish()
  crypto_mod_finish()
  pauth_disable_el1()
  console_flush()
  smc(BL1_SMC_RUN_IMAGE, (unsigned long)next_bl_ep_info, ...)
```

**说明：** QEMU virt 上 BL2 通常在 **S-EL1**，交棒用 **`smc`**，不是 `bl`。

**若 `BL2_RUNS_AT_EL3`（非 virt 常见）：** `bl2_run_next_image` 内有 `bl disable_mmu_icache_el3`、`bl bl2_el3_plat_prepare_exit`，再 ERET。

---

## 4. BL31（C 主顺序）

```text
bl31_main(arg0..arg3)
  plat_setup_early_console()
  bl31_early_platform_setup2()
  bl31_plat_arch_setup()
  detect_arch_features()            // 若开启
  cm_manage_extensions_el3 / per_world
  bl31_platform_setup()
  gic_init / gic_pcpu_init / ...    // 若 USE_GIC_DRIVER
  bl31_lib_init()
  ehf_init()                        // 若 EL3_EXCEPTION_HANDLING
  runtime_svc_init()
  (*bl32_init)()                    // 若注册
  (*rmm_init)()                    // 若 RME
  bl31_prepare_next_image_entry()
  bl31_plat_runtime_setup()
  return -> entrypoint -> b el3_exit
```

**说明：** 上游 `bl31_entrypoint` 为 **`bl bl31_main`**，一般**无**单独 `bl bl31_setup`（其它 tree 可能不同）。

---

## 5. `bl` / `b` / `smc` 对照

| 形式 | 作用 |
|------|------|
| `bl func` | 子程序调用 |
| `b label` | 跳转，不保存返回地址（如 `el3_exit`） |
| `smc` | 进 EL3 处理路径，非 `bl` |
| ERET | 按 `elr_el3`/`spsr_el3` 返回低级 EL |

---

## 6. 关键源码位置（便于对照）

| 阶段 | 文件 |
|------|------|
| BL1 入口 | `bl1/aarch64/bl1_entrypoint.S` |
| BL1 C | `bl1/bl1_main.c` |
| QEMU BL1 平台 | `plat/qemu/common/qemu_bl1_setup.c` |
| BL2 入口 | `bl2/aarch64/bl2_entrypoint.S` |
| BL2 C | `bl2/bl2_main.c` |
| QEMU BL2 平台 | `plat/qemu/common/qemu_bl2_setup.c` |
| QEMU I/O | `plat/qemu/common/qemu_io_storage.c` |
| BL31 入口 | `bl31/aarch64/bl31_entrypoint.S` |
| BL31 C | `bl31/bl31_main.c` |
