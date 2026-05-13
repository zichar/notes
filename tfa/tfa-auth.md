# TF-A 固件鉴权（Trusted Boot / TBBR）

与加载、IO、FIP 的关系见同目录 **`tfa-load.md`**（`load_image()`、`plat_get_image_source()`、FIP 等）。本文只描述 **带鉴权的加载路径**：`load_auth_image()` 与 **Chain of Trust（CoT）** 上的校验逻辑。

## 1. 总览：`TRUSTED_BOARD_BOOT` 开 / 关

```text
[!TRUSTED_BOARD_BOOT]
  load_auth_image()
    -> load_auth_image_internal()
         -> load_image()                    // 仅加载，不验签

[TRUSTED_BOARD_BOOT] 且 dyn_is_auth_disabled() == 0
  load_auth_image()
    -> load_auth_image_internal()
         -> load_auth_image_recursive()     // 先沿父链鉴权，再 load + verify 本镜像

[TRUSTED_BOARD_BOOT] 但鉴权被动态关闭
  -> 同 [!TBBR]，走 load_image() only
```

源码：`common/bl_common.c`（`load_auth_image` / `load_auth_image_internal` / `load_auth_image_recursive`）。

## 2. 入口：`load_auth_image()`

```text
load_auth_image(image_id, image_data)        // common/bl_common.c
  if (plat_try_img_ops 未配置多实例)
    load_auth_image_internal(image_id, image_data)
  else
    do {
      err = load_auth_image_internal(...)
      if (err && plat_try_img_ops->next_instance(image_id) == 0)
        重试下一实例
    } while (err != 0)

  if (err == 0)
    plat_mboot_measure_image(...)            // MEASURED_BOOT
    flush_dcache_range(image_base, image_size)
  return err
```

加载失败返回 IO 相关错误；**鉴权失败**在递归路径里返回 **`-EAUTH`**（并已对镜像缓冲区 `zero_normalmem` + `flush_dcache_range`）。

## 3. 递归：父镜像先于子镜像

```text
load_auth_image_recursive(image_id, image_data)   // TRUSTED_BOARD_BOOT
  rc = auth_mod_get_parent_id(image_id, &parent_id)
  if (rc == 0)                                    // 有未鉴权过的父节点
    load_auth_image_recursive(parent_id, image_data)
  rc = load_image(image_id, image_data)           // 同 tfa-load.md：IO + FIP 等
  rc = auth_mod_verify_img(image_id,
                           (void *)image_data->image_base,
                           image_data->image_size)
  if (rc != 0)
    zero_normalmem(...); flush_dcache_range(...); return -EAUTH
```

`auth_mod_get_parent_id()`（`drivers/auth/auth_mod.c`）：根据 **CoT** 里该 `image_id` 的 `auth_img_desc_t` 取父节点；若 **无父（根）** 或 **父已带 `IMG_FLAG_AUTHENTICATED`**，则不再向上递归。

## 4. 核心：`auth_mod_verify_img()`

```text
auth_mod_verify_img(img_id, img_ptr, img_len)     // drivers/auth/auth_mod.c
  img_desc = /* CoT 中该 img_id 的描述，如 FCONF_GET_PROPERTY(tbbr, cot, img_id) */
  img_parser_check_integrity(img_desc->img_type, img_ptr, img_len)

  for (i = 0; i < AUTH_METHOD_NUM; i++)
    switch (img_desc->img_auth_methods[i].type)
      AUTH_METHOD_HASH   -> auth_hash()          // 与父中提取的 hash 比对；crypto_mod_verify_hash
      AUTH_METHOD_SIG    -> auth_signature()     // 验签；crypto_mod；父或 ROTPK
      AUTH_METHOD_NV_CTR -> auth_nvctr()         // 证书 NV counter vs plat_get_nv_ctr；可触发 plat_set_nv_ctr2
      AUTH_METHOD_NONE   -> rc = 0

  /* 可选：NV counter 升级（与 SIG 等条件配合）*/

  /* 从本镜像解析 authenticated_data，拷入描述符供子节点使用 */
  for (authenticated_data[i])
    img_parser_get_auth_param(...)
    memcpy(-> data.ptr, ...)

  auth_img_flags[img_desc->img_id] |= IMG_FLAG_AUTHENTICATED
```

**要点**：每种镜像类型对应 **`img_type`**，由 **`img_parser_mod`** 从证书/固件二进制中取出待验字段；**`crypto_mod`** 完成哈希/签名运算；父镜像（或平台 ROTPK）提供公钥、哈希等 **认证参数**（见 `auth_img_desc_t::parent` 与 `authenticated_data`）。

## 5. 启动时注册：CoT 与 `auth_mod_init()`

```text
REGISTER_COT(_cot)                             // include/drivers/auth/auth_mod.h
  -> cot_desc_ptr / cot_desc_size / auth_img_flags[]

auth_mod_init()                                // TRUSTED_BOARD_BOOT
  assert(cot_desc_ptr)
  img_parser_init()
```

**`auth_img_desc_t`**（摘要）：`img_id`、`img_type`、`parent`、**`img_auth_methods[]`**（`AUTH_METHOD_HASH` / `SIG` / `NV_CTR`）、**`authenticated_data[]`**（供子镜像 `auth_get_param` 使用）。

具体 CoT 表由平台 **`cot_def.h` / `REGISTER_COT(...)`** 展开（与 `TRUSTED_BOARD_BOOT`、证书 UUID 等一致）。

## 6. 小结

```text
load_auth_image
  -> [TBB] load_auth_image_recursive
       auth_mod_get_parent_id   // 沿 CoT 向上直到根或已鉴权父
       load_image               // 见 tfa-load.md
       auth_mod_verify_img      // integrity + HASH/SIG/NV_CTR + 提取子用参数
  -> plat_mboot_measure_image（可选）
  -> flush_dcache_range
```

非 TBB 构建：**无** `load_auth_image_recursive` / `auth_mod_verify_img`，仅 **`load_image()`**。
