# TF-A 加载与 FIP / IO

## 1. 调用链

```text
bl1_main()
  bl1_load_bl2()
    bl1_plat_get_image_desc(BL2_IMAGE_ID)
    bl1_plat_handle_pre_image_load(BL2_IMAGE_ID)
    load_auth_image(BL2_IMAGE_ID, &desc->image_info)
      load_auth_image_internal()
        load_image()                         // [!TBBR]  ; [TBBR] load_auth_image_recursive()->...
    bl1_plat_handle_post_image_load(BL2_IMAGE_ID)
  bl1_prepare_next_image(BL2_IMAGE_ID)
  // -> el3_exit / ERET -> BL2

bl2_main()
  bl2_load_images()
    bl2_load_info = plat_get_bl_image_load_info()   // -> §2
      [qemu] get_bl_load_info_from_mem_params_desc()
    bl2_node_info = bl2_load_info->head
    while (bl2_node_info)
      bl2_plat_handle_pre_image_load(id)
      load_auth_image(id, image_info)        // [SKIP_LOADING] 不进入 load_image 读盘
      bl2_plat_handle_post_image_load(id)
    plat_get_next_bl_params()
    plat_flush_next_bl_params()

load_image()                                   // §3
  plat_get_image_source()
  io_open() -> io_size() -> io_read() -> io_close()
  io_dev_close()
```

## 2. 获取 `bl_load_info`

```text
/* ---------- plat_get_bl_image_load_info() ---------- */
bl2_load_images()
  plat_get_bl_image_load_info()                // plat/qemu/common/qemu_image_load.c
    get_bl_load_info_from_mem_params_desc()    // common/desc_image_load.c
      SET_PARAM_HEAD(&bl_load_info, ...)
      bl_load_info.head = &bl_mem_params_desc_ptr[0].load_node_mem
      for (i = 0; i < bl_mem_params_desc_num; i++)
        load_node_mem[i] <- bl_mem_params_desc_ptr[i].{image_id, image_info}
        next_load_info -> &bl_mem_params_desc_ptr[i+1].load_node_mem
      return &bl_load_info

/* ---------- bl_mem_params_desc_ptr（数据源）---------- */
bl_mem_params_desc_ptr  ----->  bl_mem_params_node_t[0]
         |                      bl_mem_params_node_t[1]
         +--------------------> ...
         |
         +---> [i].image_id , [i].image_info , [i].load_node_mem --next--> [i+1].load_node_mem

/* ---------- REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs) @ qemu_bl2_mem_params_desc.c ---------- */
REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs)
  |==展开==>  bl_mem_params_desc_ptr = &bl2_mem_params_descs[0];
  |           bl_mem_params_desc_num = ARRAY_SIZE(bl2_mem_params_descs);
  // 附录：bl2_mem_params_descs[] 全文
```

## 3. IO（QEMU）

```text
/* ---------- plat_qemu_io_setup() @ qemu_io_storage.c ---------- */
plat_qemu_io_setup()
  register_io_dev_fip(&fip_dev_con)
    io_register_device(&dev_info_pool[0])   // io_fip.c
    fip_dev_con = &fip_dev_connector
  register_io_dev_memmap(&memmap_dev_con)
    io_register_device(&memmap_dev_info)    // io_memmap.c
    memmap_dev_con = &memmap_dev_connector
  io_dev_open(fip_dev_con, NULL, &fip_dev_handle)
    io_storage_dev_open()
      fip_dev_connector.dev_open()
        fip_dev_open()
          allocate_dev_info(&info)
          info->funcs = &fip_dev_funcs       // open=fip_file_open, dev_init=fip_dev_init, ...
          // 依后端三选一：&fip_dev_funcs / &block_dev_funcs / &mtd_dev_funcs；QEMU FIP 路径为 fip_dev_funcs
    // fip_dev_handle = (uintptr_t)info
  io_dev_open(memmap_dev_con, NULL, &memmap_dev_handle)
    io_storage_dev_open()
      memmap_dev_connector.dev_open()
        memmap_dev_open()
          *dev_info = &memmap_dev_info
    // memmap_dev_handle = (uintptr_t)&memmap_dev_info
  register_io_dev_sh(&sh_dev_con)
    io_register_device(&sh_dev_info)       // io_semihosting.c
    sh_dev_con = &sh_dev_connector
  io_dev_open(sh_dev_con, NULL, &sh_dev_handle)

/* ---------- plat_get_image_source()（Arm 通用：plat/arm/common/arm_io_storage.c；QEMU：qemu_io_storage.c）---------- */
plat_get_image_source(image_id, dev_handle, image_spec)
  policy = get_io_policy(image_id)        // policy 指向附录「struct plat_io_policy」；表项见附录「policies[]」
  result = policy->check(policy->image_spec) //->open_fip()/open_enc_fip()/open_memmap()...
  if (result == 0)
    *image_spec = policy->image_spec;
    *dev_handle = *(policy->dev_handle);
  else
    plat_arm_get_alt_image_source(image_id, dev_handle, image_spec);   // QEMU：get_alt_image_source()

open_fip(uuid_spec)
  io_dev_init(fip_dev_handle, FIP_IMAGE_ID)
    fip_dev_handle->funcs() //->fip_dev_init()
      plat_get_image_source(FIP_IMAGE_ID, &backend_dev_handle, &backend_image_spec)
      io_open(backend_dev_handle, backend_image_spec, &h)
      io_read(h, &fip_toc_header_t, ...)
      is_valid_header()
      io_close(h)
  io_open(fip_dev_handle, uuid_spec, &h)
    fip_file_open()
  io_close(h)

static const struct plat_io_policy policies[] = { // @ qemu_io_storage.c
    [FIP_IMAGE_ID] = {
        &memmap_dev_handle,
        (uintptr_t)&fip_block_spec,
        open_memmap //->backend_dev_handle
    },
    [BL31_IMAGE_ID] = {
        &fip_dev_handle,
        (uintptr_t)&bl31_uuid_spec,
        open_fip
    },
    ...
}

/* ---------- load_image() @ FIP ---------- */
load_image()
  plat_get_image_source()
  io_open(fip_dev_handle, uuid_spec, &entity)
    fip_file_open()
      io_open(backend, fip_block_spec, &bh)
      io_seek(bh, SET, sizeof(fip_toc_header_t))
      io_read(bh, &fip_toc_entry_t) ...      // 扫 UUID
      io_close(bh)
  io_size(entity)  -> fip_file_len()
  io_read(entity, image_base, size) -> fip_file_read()
    io_open(backend,...)
    io_seek(bh, SET, offset_address + file_pos)
    io_read(bh, ...) -> memmap_block_read() -> memcpy
    io_close(bh)
  io_close(entity) -> fip_file_close()
  io_dev_close(fip_dev_handle) -> fip_dev_close()

/* ---------- io_storage 分派 ---------- */
io_open()  -> allocate_entity() -> dev->funcs->open()
io_*()     -> entity->dev_handle->funcs->*()
```

**plat_get_image_source()** 是绑定io前后端的核心

## 4. FIP 布局与示例

```text
/* ---------- 线性文件布局 ---------- */
offset 0
  [ fip_toc_header_t | 16 B ]
  [ fip_toc_entry_t  | 40 B ] x N
  [ uuid=0 END       | 40 B ]
  [ payload A @ offset_address_A ]
  [ payload B @ offset_address_B ]
  ...

/* ---------- fip_toc_header_t ---------- */
struct {
  uint32_t name;            // TOC_HEADER_NAME 0xAA640001
  uint32_t serial_number;   // !=0
  uint64_t flags;
};

/* ---------- fip_toc_entry_t ---------- */
struct {
  uuid_t uuid;              // 全0 = ToC 结束
  uint64_t offset_address;
  uint64_t size;
  uint64_t flags;
};

/* ---------- fip_file_open() ---------- */
fip_file_open()
  io_seek(backend, SET, sizeof(fip_toc_header_t))
  do io_read(backend, &entry) while (!match(uuid_spec) && entry.uuid!=0)
  // miss -> ENOENT

/* ---------- fiptool info 示例（fip.bin）---------- */
// fip.bin
// $ tools/fiptool/fiptool info fip.bin
Trusted Boot Firmware BL2:    offset=0x150,    size=0x9B69,   --tb-fw
EL3 Runtime Firmware BL31:    offset=0x9CB9,   size=0x130D4,  --soc-fw
Secure Payload BL32:          offset=0x1CD8D,  size=0x2E1D0,  --tos-fw
Non-Trusted Firmware BL33:    offset=0x4AF5D,  size=0x300000, --nt-fw
TB_FW_CONFIG:                 offset=0x34AF5D, size=0xE9,     --tb-fw-config
TOS_FW_CONFIG:                offset=0x34B046, size=0x421,    --tos-fw-config
486178E0-... :                 offset=0x34B467, size=0xBB148, --blob
// entry <-> fip_toc_entry_t；offset/size 同 offset_address/size；load_image 按 image_id -> UUID
```

## 5. 小结

```text
REGISTER_BL_IMAGE_DESCS
  -> bl_mem_params_desc_ptr / bl_mem_params_desc_num
    -> get_bl_load_info_from_mem_params_desc()
      -> bl_load_info.head
        -> bl2_node_info -> next_load_info -> ...

plat_get_image_source -> io_open(fip,uuid) -> fip_file_open/read -> memmap -> memcpy
```

## 附录：`REGISTER_BL_IMAGE_DESCS` 与 `bl2_mem_params_descs`

### 宏定义（`include/common/desc_image_load.h`）

```c
#define REGISTER_BL_IMAGE_DESCS(_img_desc)				\
	bl_mem_params_node_t *bl_mem_params_desc_ptr = &_img_desc[0];	\
	unsigned int bl_mem_params_desc_num = ARRAY_SIZE(_img_desc);
```

### `plat/qemu/common/qemu_bl2_mem_params_desc.c`（与 TF-A 源码一致；`#ifdef` 随 defconfig 变化）

```c
/*
 * Copyright (c) 2017-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <common/desc_image_load.h>
#include <plat/common/platform.h>

#define SP_PKG_ENTRY(id) \
	{ \
		.image_id = (id), \
		SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY, VERSION_2, \
				      entry_point_info_t, \
				      SECURE | NON_EXECUTABLE), \
		SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY, \
				      VERSION_2, image_info_t, \
				      IMAGE_ATTRIB_SKIP_LOADING), \
		.next_handoff_image_id = INVALID_IMAGE_ID, \
	}

/*******************************************************************************
 * Following descriptor provides BL image/ep information that gets used
 * by BL2 to load the images and also subset of this information is
 * passed to next BL image. The image loading sequence is managed by
 * populating the images in required loading order. The image execution
 * sequence is managed by populating the `next_handoff_image_id` with
 * the next executable image id.
 ******************************************************************************/
static bl_mem_params_node_t bl2_mem_params_descs[] = {
#ifdef EL3_PAYLOAD_BASE
	/* Fill EL3 payload related information (BL31 is EL3 payload) */
	{ .image_id = BL31_IMAGE_ID,

	  SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				entry_point_info_t,
				SECURE | EXECUTABLE | EP_FIRST_EXE),
	  .ep_info.pc = EL3_PAYLOAD_BASE,
	  .ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
				  DISABLE_ALL_EXCEPTIONS),

	  SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2, image_info_t,
				IMAGE_ATTRIB_PLAT_SETUP | IMAGE_ATTRIB_SKIP_LOADING),

	  .next_handoff_image_id = INVALID_IMAGE_ID,
	},
#else /* EL3_PAYLOAD_BASE */
#ifdef __aarch64__
	/* Fill BL31 related information */
	{ .image_id = BL31_IMAGE_ID,

	  SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				entry_point_info_t,
				SECURE | EXECUTABLE | EP_FIRST_EXE),
	  .ep_info.pc = BL31_BASE,
	  .ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
				  DISABLE_ALL_EXCEPTIONS),
# if DEBUG
	  .ep_info.args.arg1 = QEMU_BL31_PLAT_PARAM_VAL,
# endif
	  SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2, image_info_t,
				IMAGE_ATTRIB_PLAT_SETUP),
	  .image_info.image_base = BL31_BASE,
	  .image_info.image_max_size = BL31_LIMIT - BL31_BASE,

# ifdef QEMU_LOAD_BL32
	  .next_handoff_image_id = BL32_IMAGE_ID,
# elif ENABLE_RME
	  .next_handoff_image_id = RMM_IMAGE_ID,
# else
	  .next_handoff_image_id = BL33_IMAGE_ID,
# endif
	},
#endif /* __aarch64__ */

#if ENABLE_RME
	/* Fill RMM related information */
	{ .image_id = RMM_IMAGE_ID,
	  SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
		VERSION_2, entry_point_info_t, EP_REALM | EXECUTABLE),
	  .ep_info.pc = RMM_BASE,
	  SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
		VERSION_2, image_info_t, 0),
	  .image_info.image_base = RMM_BASE,
	  .image_info.image_max_size = RMM_LIMIT - RMM_BASE,
	  .next_handoff_image_id = BL33_IMAGE_ID,
	},
#endif /* ENABLE_RME */

# ifdef QEMU_LOAD_BL32

#ifdef __aarch64__
#define BL32_EP_ATTRIBS		(SECURE | EXECUTABLE)
#define BL32_IMG_ATTRIBS	0
#else
#define BL32_EP_ATTRIBS		(SECURE | EXECUTABLE | EP_FIRST_EXE)
#define BL32_IMG_ATTRIBS	IMAGE_ATTRIB_PLAT_SETUP
#endif

	/* Fill BL32 related information */
	{ .image_id = BL32_IMAGE_ID,

	  SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				entry_point_info_t, BL32_EP_ATTRIBS),
	  .ep_info.pc = BL32_BASE,

	  SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				image_info_t, BL32_IMG_ATTRIBS),

	  .image_info.image_base = BL32_BASE,
	  .image_info.image_max_size = BL32_LIMIT - BL32_BASE,

#if ENABLE_RME
	  .next_handoff_image_id = RMM_IMAGE_ID,
#else
	  .next_handoff_image_id = BL33_IMAGE_ID,
#endif
	},

	/*
	 * Fill BL32 external 1 related information.
	 * A typical use for extra1 image is with OP-TEE where it is the
	 * pager image.
	 */
	{ .image_id = BL32_EXTRA1_IMAGE_ID,

	   SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				 entry_point_info_t, SECURE | NON_EXECUTABLE),

	   SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				 image_info_t, IMAGE_ATTRIB_SKIP_LOADING),
	   .image_info.image_base = BL32_BASE,
	   .image_info.image_max_size = BL32_LIMIT - BL32_BASE,

	   .next_handoff_image_id = INVALID_IMAGE_ID,
	},

	/*
	 * Fill BL32 external 2 related information.
	 * A typical use for extra2 image is with OP-TEE where it is the
	 * paged image.
	 */
	{ .image_id = BL32_EXTRA2_IMAGE_ID,

	   SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				 entry_point_info_t, SECURE | NON_EXECUTABLE),

	   SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2,
				 image_info_t, IMAGE_ATTRIB_SKIP_LOADING),
#if defined(SPD_opteed) || defined(AARCH32_SP_OPTEE) || defined(SPMC_OPTEE)
	   .image_info.image_base = QEMU_OPTEE_PAGEABLE_LOAD_BASE,
	   .image_info.image_max_size = QEMU_OPTEE_PAGEABLE_LOAD_SIZE,
#endif
	   .next_handoff_image_id = INVALID_IMAGE_ID,
	},

#if defined(SPD_spmd)
	/* Fill TOS_FW_CONFIG related information */
	{
	    .image_id = TOS_FW_CONFIG_ID,
	    SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
		    VERSION_2, entry_point_info_t, SECURE | NON_EXECUTABLE),
	    SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
		    VERSION_2, image_info_t, 0),
	    .image_info.image_base = TOS_FW_CONFIG_BASE,
	    .image_info.image_max_size = TOS_FW_CONFIG_LIMIT -
					 TOS_FW_CONFIG_BASE,
	    .next_handoff_image_id = INVALID_IMAGE_ID,
	},

#if SPMD_SPM_AT_SEL2
	/* Fill TB_FW_CONFIG related information */
	{
	    .image_id = TB_FW_CONFIG_ID,
	    SET_STATIC_PARAM_HEAD(ep_info, PARAM_IMAGE_BINARY,
		    VERSION_2, entry_point_info_t, SECURE | NON_EXECUTABLE),
	    SET_STATIC_PARAM_HEAD(image_info, PARAM_IMAGE_BINARY,
		    VERSION_2, image_info_t, 0),
	    .image_info.image_base = TB_FW_CONFIG_BASE,
	    .image_info.image_max_size = TB_FW_CONFIG_LIMIT - TB_FW_CONFIG_BASE,
	    .next_handoff_image_id = INVALID_IMAGE_ID,
	},

	/*
	 * Empty entries for SP packages to be filled in according to
	 * TB_FW_CONFIG.
	 */
	SP_PKG_ENTRY(SP_PKG1_ID),
	SP_PKG_ENTRY(SP_PKG2_ID),
	SP_PKG_ENTRY(SP_PKG3_ID),
	SP_PKG_ENTRY(SP_PKG4_ID),
	SP_PKG_ENTRY(SP_PKG5_ID),
	SP_PKG_ENTRY(SP_PKG6_ID),
	SP_PKG_ENTRY(SP_PKG7_ID),
	SP_PKG_ENTRY(SP_PKG8_ID),
#endif
#endif
# endif /* QEMU_LOAD_BL32 */

	/* Fill BL33 related information */
	{ .image_id = BL33_IMAGE_ID,
	  SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP, VERSION_2,
				entry_point_info_t, NON_SECURE | EXECUTABLE),
# ifdef PRELOADED_BL33_BASE
	  .ep_info.pc = PRELOADED_BL33_BASE,

	  SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2, image_info_t,
				IMAGE_ATTRIB_SKIP_LOADING),
# else /* PRELOADED_BL33_BASE */
	  .ep_info.pc = NS_IMAGE_OFFSET,

	  SET_STATIC_PARAM_HEAD(image_info, PARAM_EP, VERSION_2, image_info_t,
				0),
	  .image_info.image_base = NS_IMAGE_OFFSET,
	  .image_info.image_max_size = NS_IMAGE_MAX_SIZE,
# endif /* !PRELOADED_BL33_BASE */

	  .next_handoff_image_id = INVALID_IMAGE_ID,
	}
#endif /* !EL3_PAYLOAD_BASE */
};

REGISTER_BL_IMAGE_DESCS(bl2_mem_params_descs)
```

## 附录：`plat_io_policy` / `policies[]`（`plat/qemu/common/qemu_io_storage.c`）

### `struct plat_io_policy`

```c
struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};
```

### `static const struct plat_io_policy policies[]`

与 TF-A 源码一致；`#if` 随 `ENCRYPT_BL31` / `ENCRYPT_BL32` / `TRUSTED_BOARD_BOOT` 等变化。

```c
/* By default, ARM platforms load images from the FIP */
static const struct plat_io_policy policies[] = {
	[FIP_IMAGE_ID] = {
		&memmap_dev_handle,
		(uintptr_t)&fip_block_spec,
		open_memmap
	},
	[ENC_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)NULL,
		open_fip
	},
	[BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl2_uuid_spec,
		open_fip
	},
#if ENCRYPT_BL31 && !defined(DECRYPTION_SUPPORT_none)
	[BL31_IMAGE_ID] = {
		&enc_dev_handle,
		(uintptr_t)&bl31_uuid_spec,
		open_enc_fip
	},
#else
	[BL31_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl31_uuid_spec,
		open_fip
	},
#endif
#if ENCRYPT_BL32 && !defined(DECRYPTION_SUPPORT_none)
	[BL32_IMAGE_ID] = {
		&enc_dev_handle,
		(uintptr_t)&bl32_uuid_spec,
		open_enc_fip
	},
	[BL32_EXTRA1_IMAGE_ID] = {
		&enc_dev_handle,
		(uintptr_t)&bl32_extra1_uuid_spec,
		open_enc_fip
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		&enc_dev_handle,
		(uintptr_t)&bl32_extra2_uuid_spec,
		open_enc_fip
	},
#else
	[BL32_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_uuid_spec,
		open_fip
	},
	[BL32_EXTRA1_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_extra1_uuid_spec,
		open_fip
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_extra2_uuid_spec,
		open_fip
	},
#endif
	[TB_FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tb_fw_config_uuid_spec,
		open_fip
	},
	[TOS_FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tos_fw_config_uuid_spec,
		open_fip
	},
	[BL33_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_uuid_spec,
		open_fip
	},
	[RMM_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&rmm_uuid_spec,
		open_fip
	},

#if TRUSTED_BOARD_BOOT
	[TRUSTED_BOOT_FW_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tb_fw_cert_uuid_spec,
		open_fip
	},
	[TRUSTED_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&trusted_key_cert_uuid_spec,
		open_fip
	},
	[SOC_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&soc_fw_key_cert_uuid_spec,
		open_fip
	},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tos_fw_key_cert_uuid_spec,
		open_fip
	},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&nt_fw_key_cert_uuid_spec,
		open_fip
	},
	[SOC_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&soc_fw_cert_uuid_spec,
		open_fip
	},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tos_fw_cert_uuid_spec,
		open_fip
	},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&nt_fw_cert_uuid_spec,
		open_fip
	},
#endif /* TRUSTED_BOARD_BOOT */
};
```
