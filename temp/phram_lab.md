# phram + mtdparts + jffs2 实验手册

这份文档只保留实验相关内容，目标是快速验证下面这条链路：

```text
bootargs
  -> phram 创建 master MTD
  -> cmdlinepart 解析分区
  -> /proc/mtd 可见
  -> mtdblock 通过 notifier 自动生成
  -> jffs2 成功挂载分区
```

---

## 1. 实验目标

验证 5 件事：

1. `phram` 能把一段 RAM 注册成 MTD 设备
2. `mtdparts=` 能把这个 MTD 再切成多个分区
3. `/proc/mtd` 能正确显示结果
4. `mtdblock` 能自动生成 `/dev/mtdblockX`
5. `jffs2` 能挂载到 `rootfs` 分区

---

## 2. 推荐环境

- `qemu-system-arm` 或 `qemu-system-aarch64`
- 自编 Linux 内核
- 一个最小 initramfs

建议先做 QEMU 实验，再上真实板子。

---

## 3. 内核配置

至少打开：

- `CONFIG_MTD=y`
- `CONFIG_MTD_PHRAM=y`
- `CONFIG_MTD_BLOCK=y`
- `CONFIG_MTD_CMDLINE_PARTS=y`
- `CONFIG_JFFS2_FS=y`
- `CONFIG_DEVTMPFS=y`
- `CONFIG_DEVTMPFS_MOUNT=y`
- `CONFIG_PROC_FS=y`
- `CONFIG_SYSFS=y`
- `CONFIG_TMPFS=y`

可选但推荐：

- `CONFIG_MTD_PARTITIONED_MASTER=y`

说明：

- 打开 `CONFIG_MTD_PARTITIONED_MASTER` 时，master MTD 和分区可同时保留
- 关闭时，通常只看到分区

---

## 4. 启动参数设计

假设 QEMU 给虚拟机 256 MiB RAM：

- `-m 256M`

再通过：

- `mem=248M`

让 Linux 只使用前 248 MiB，这样最后 8 MiB 可以留给 `phram`。

推荐 bootargs：

```text
console=ttyAMA0 rdinit=/init mem=248M \
phram.phram=ph0,0x4f800000,8Mi,64Ki \
mtdparts=ph0:256k(boot)ro,2M(kernel),-(rootfs)
```

含义：

- `ph0`：一个 8 MiB 的 phram 设备
- 擦除块大小 `64KiB`
- 再切成 3 个分区：
  - `boot` = 256 KiB
  - `kernel` = 2 MiB
  - `rootfs` = 剩余空间

---

## 5. QEMU 启动示例

### ARM 32-bit

```bash
qemu-system-arm \
  -M virt \
  -cpu cortex-a15 \
  -m 256M \
  -nographic \
  -kernel arch/arm/boot/zImage \
  -initrd rootfs.cpio.gz \
  -append 'console=ttyAMA0 rdinit=/init mem=248M phram.phram=ph0,0x4f800000,8Mi,64Ki mtdparts=ph0:256k(boot)ro,2M(kernel),-(rootfs)'
```

### ARM64

```bash
qemu-system-aarch64 \
  -M virt \
  -cpu cortex-a57 \
  -m 256M \
  -nographic \
  -kernel arch/arm64/boot/Image \
  -initrd rootfs.cpio.gz \
  -append 'console=ttyAMA0 rdinit=/init mem=248M phram.phram=ph0,0x4f800000,8Mi,64Ki mtdparts=ph0:256k(boot)ro,2M(kernel),-(rootfs)'
```

---

## 6. 最小 initramfs 脚本

`/init` 可以这样写：

```sh
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev

echo "=== /proc/filesystems ==="
cat /proc/filesystems

echo "=== /proc/mtd ==="
cat /proc/mtd

echo "=== /dev/mtd* ==="
ls -l /dev/mtd* 2>/dev/null

echo "=== /dev/mtdblock* ==="
ls -l /dev/mtdblock* 2>/dev/null

mkdir -p /mnt

echo "=== try mount jffs2 ==="
mount -t jffs2 mtd:rootfs /mnt || exec sh

echo hello > /mnt/hello.txt
sync
ls -l /mnt
cat /mnt/hello.txt

umount /mnt

echo "=== remount ==="
mount -t jffs2 mtd:rootfs /mnt || exec sh
cat /mnt/hello.txt

exec sh
```

注意：

- 优先用 `mtd:rootfs`
- 不建议把 `jffs2` 挂在 `/dev/mtdblockX` 上做主路径验证

---

## 7. 预期结果

### 7.1 `/proc/mtd`

如果 **未启用** `CONFIG_MTD_PARTITIONED_MASTER`，更可能看到：

```text
dev:    size   erasesize  name
mtd0: 00040000 00010000 "boot"
mtd1: 00200000 00010000 "kernel"
mtd2: 005c0000 00010000 "rootfs"
```

如果 **启用了** `CONFIG_MTD_PARTITIONED_MASTER`，更可能看到：

```text
dev:    size   erasesize  name
mtd0: 00800000 00010000 "ph0"
mtd1: 00040000 00010000 "boot"
mtd2: 00200000 00010000 "kernel"
mtd3: 005c0000 00010000 "rootfs"
```

### 7.2 `/dev` 节点

若启用了 `CONFIG_MTD_BLOCK`，通常还能看到：

- `/dev/mtd0`
- `/dev/mtd0ro`
- `/dev/mtdblock0`

如果是分区场景，也可能有：

- `/dev/mtd1`
- `/dev/mtd1ro`
- `/dev/mtdblock1`

等等。

### 7.3 JFFS2 挂载

如果 `rootfs` 分区为空白 RAM 区域，JFFS2 很多情况下可以直接初始化并挂载成功。

验证点：

```bash
echo hello > /mnt/hello.txt
sync
cat /mnt/hello.txt
umount /mnt
mount -t jffs2 mtd:rootfs /mnt
cat /mnt/hello.txt
```

同一次运行里，卸载重挂载后文件还在，说明：

- MTD 层正常
- JFFS2 正常
- 分区路径正确

整机重启后数据消失是正常现象，因为底层仍是 RAM。

---

## 8. 推荐检查命令

### 看 phram 是否创建成功

```bash
dmesg | grep -i phram
```

### 看 MTD 设备和分区

```bash
cat /proc/mtd
```

### 看字符 / 块节点

```bash
ls -l /dev/mtd*
ls -l /dev/mtdblock*
```

### 看 JFFS2 相关日志

```bash
dmesg | grep -i jffs2
```

### 看整体 MTD 相关日志

```bash
dmesg | grep -Ei 'mtd|phram|jffs2'
```

---

## 9. 常见问题

### 9.1 `mtdparts=` 不生效

优先检查：

1. 是否启用了 `CONFIG_MTD_CMDLINE_PARTS`
2. `mtd-id` 是否和 `phram` 设备名一致
3. 驱动路径里是否会尝试 `cmdlinepart`

对于本实验，最常见写法应是：

```text
mtdparts=ph0:...
```

而不是：

```text
mtdparts=flash0:...
```

### 9.2 没有 `/dev/mtdblockX`

通常是：

- 没开 `CONFIG_MTD_BLOCK`

### 9.3 JFFS2 挂载失败

检查：

1. 是否挂到了正确分区
2. `erasesize` 是否设置合理
3. 目标分区是否真的存在
4. `cat /proc/mtd` 中的名字是否和挂载参数一致

推荐先试：

```bash
mount -t jffs2 mtd:rootfs /mnt
```

### 9.4 master 和分区为何有时同时出现

看：

- `CONFIG_MTD_PARTITIONED_MASTER`

---

## 10. 代码对应关系

这个实验每一步都能对应到代码：

### 创建 phram

- `phram_setup()`
- `register_device()`
- `phram_map()`

### 注册 MTD

- `mtd_device_parse_register()`
- `add_mtd_device()`

### 解析分区

- `parse_mtd_partitions()`
- `parse_cmdline_partitions()`

### 自动生成 mtdblock

- `blktrans_notify_add()`
- `mtdblock_add_mtd()`

### 挂载 JFFS2

- `jffs2_get_tree()`
- `get_tree_mtd()`
- `jffs2_fill_super()`

---

## 11. 建议的实验顺序

推荐按下面顺序跑：

1. 先只开 `phram`，不带 `mtdparts=`
   目标：确认单个 master MTD 能出来

2. 再加 `mtdparts=`
   目标：确认分区解析生效

3. 再确认 `/dev/mtdblockX`
   目标：确认 notifier 到 block translation 层生效

4. 最后挂 `jffs2`
   目标：完成全链路验证

---

## 12. 一句话总结

这套实验最重要的意义是：

> 用最简单的 RAM 模拟设备，把 `phram`、分区解析、MTD 核心注册、mtdblock 通知和 JFFS2 挂载串成一条可重复验证的完整链路。
