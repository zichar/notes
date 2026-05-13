# Linux Block Driver 分析

本文从两个主线来分析 Linux block device / block driver：

1. 初始化流程
2. runtime 使用流程

重点围绕：

- `blk_mq_alloc_disk()`
- `device_add_disk()`

并结合实际案例：

- `virtio-blk`
- `mtdblock`

最后给出适合 ARM QEMU 平台的实验方法。

---

## 1. Linux block 驱动的核心对象

理解块驱动之前，先记住 5 个核心对象：

### 1.1 `struct blk_mq_tag_set`

这是驱动交给 blk-mq 的“硬件队列模板”。

它描述：

- `ops`
- 硬件队列数
- 队列深度
- NUMA 位置
- 每个 request 的私有数据大小

其中最重要的是：

- `ops->queue_rq`

这是 runtime 下发请求最核心的驱动回调。

### 1.2 `struct request_queue`

这是块层请求队列，是运行时 I/O 的核心入口。

它负责：

- 队列限制
- merge/split
- elevator
- blk-mq queueing
- 调度到底层驱动

### 1.3 `struct gendisk`

这是内核中的“磁盘对象”。

它包含：

- 设备名
- major/minor
- `fops`
- 分区信息
- request queue
- 容量

用户在 `/dev` 下看到的磁盘设备，最终都对应一个 `gendisk`。

### 1.4 `struct bio`

`bio` 是块层输入对象，表示一次块 I/O。

它描述：

- 对哪个块设备
- 做什么操作
- 起始扇区
- 长度
- 页向量

### 1.5 `struct request`

这是 blk-mq 调度和派发给驱动的执行对象。

可以理解为：

- `bio` 是块层提交单元
- `request` 是驱动执行单元

---

## 2. 初始化流程总览

一个现代 MQ 块驱动的初始化流程，通常是：

1. 准备 `blk_mq_tag_set`
2. `blk_mq_alloc_tag_set()`
3. `blk_mq_alloc_disk()`
4. 填 `gendisk`
5. 设置 queue limits
6. `device_add_disk()`

关键理解：

- `blk_mq_alloc_disk()` 创建对象
- `device_add_disk()` 让对象正式生效

---

## 3. `blk_mq_alloc_disk()` 详细分析

它实际落到：

- `__blk_mq_alloc_disk()`

实现位于：

- `block/blk-mq.c`

```c
struct gendisk *__blk_mq_alloc_disk(struct blk_mq_tag_set *set, void *queuedata,
		struct lock_class_key *lkclass)
{
	struct request_queue *q;
	struct gendisk *disk;

	q = blk_mq_init_queue_data(set, queuedata);
	if (IS_ERR(q))
		return ERR_CAST(q);

	disk = __alloc_disk_node(q, set->numa_node, lkclass);
	if (!disk) {
		blk_mq_destroy_queue(q);
		blk_put_queue(q);
		return ERR_PTR(-ENOMEM);
	}
	set_bit(GD_OWNS_QUEUE, &disk->state);
	return disk;
}
```

### 3.1 它做了什么

它做了两件大事：

1. 创建 `request_queue`
2. 创建 `gendisk`

更具体地说：

- `blk_mq_init_queue_data(set, queuedata)`
  创建 queue，并把 `queuedata` 绑定进去

- `__alloc_disk_node(q, ...)`
  创建 `gendisk`，并把 queue 和 disk 关联起来

### 3.2 它的本质

所以 `blk_mq_alloc_disk()` 的本质可以概括为：

> 一次性帮驱动建立 request_queue 和 gendisk，并把二者绑定起来。

但要注意：

> 这一步之后设备还没有真正对系统可见。

此时它只是一个“已经准备好的内核对象”。

---

## 4. 驱动拿到 `disk` 后还要做什么

驱动在调用 `blk_mq_alloc_disk()` 之后，通常还要继续填写：

- `disk->disk_name`
- `disk->major`
- `disk->first_minor`
- `disk->minors`
- `disk->fops`
- `disk->private_data`
- `set_capacity(disk, ...)`
- queue 限制参数

例如：

- 块大小
- 最大 segment 数
- discard / flush / write zeroes 能力
- 只读标志

这一步的意义是：

> `device_add_disk()` 之前，驱动必须把磁盘的“身份”和“能力”描述完整。

---

## 5. `device_add_disk()` 详细分析

实现位于：

- `block/genhd.c`

```c
int __must_check device_add_disk(struct device *parent, struct gendisk *disk,
				 const struct attribute_group **groups)
{
	/* 初始化 elevator */
	elevator_init_mq(disk->queue);

	/* 设置 submit_bio 能力标志 */
	disk->part0->bd_has_submit_bio = disk->fops->submit_bio != NULL;

	/* 分配或检查 major/minor */
	/* 注册 device */
	/* 注册 queue */
	/* 注册 bdi */
	/* 注册 bdev */
	/* 分区扫描 */
	/* 发 KOBJ_ADD uevent */
}
```

### 5.1 关键动作

它可以拆成 6 件大事：

1. 初始化 I/O scheduler / elevator
2. 分配或校验 major/minor
3. `device_add(ddev)`，把 `gendisk` 对应 device 注册到 Linux device model
4. `blk_register_queue(disk)`，把 request queue 挂到块层和 sysfs
5. `bdev_add(disk->part0, ...)`，把主块设备注册进去
6. `disk_scan_partitions()`，自动扫描分区
7. 最后发 `KOBJ_ADD` uevent

### 5.2 为什么它非常关键

`device_add_disk()` 是块驱动初始化里最关键的一步，因为：

> 它是块设备真正“活起来”的时刻。

在这之前：

- 只是内核对象

在这之后：

- `/dev` 下可能出现节点
- sysfs 下出现 `/sys/block/...`
- 分区可能被自动发现
- 用户空间工具开始能看到磁盘

### 5.3 和 `blk_mq_alloc_disk()` 的区别

一句话区分：

- `blk_mq_alloc_disk()`：创建 queue + disk
- `device_add_disk()`：注册 queue + disk 到系统

---

## 6. runtime 使用流程总览

块设备运行时的主线可以总结成：

```text
用户态 read/write
    -> 页缓存 / 文件系统 / 直接块层
    -> submit_bio()
    -> blk_mq_submit_bio()
    -> 生成 request
    -> 调度 / merge / split
    -> 驱动 queue_rq()
    -> 访问设备
    -> blk_mq_end_request()
    -> bio_endio()
```

---

## 7. `bio`：块层的输入对象

`bio` 定义在：

- `include/linux/blk_types.h`

它至少包含：

- `bi_bdev`
- `bi_opf`
- `bi_status`
- `bi_iter`
- `bi_end_io`
- 页向量数组

可以把它理解成：

> 块层用来描述一次 I/O 的对象。

它回答的是：

- 给谁做 I/O
- 读还是写
- 从哪个扇区开始
- 多大
- 数据在哪些页里

---

## 8. `submit_bio()` 到 `blk_mq_submit_bio()`

核心路径在：

- `block/blk-core.c`

```c
static void __submit_bio(struct bio *bio)
{
	if (!bio->bi_bdev->bd_has_submit_bio) {
		blk_mq_submit_bio(bio);
	} else if (likely(bio_queue_enter(bio) == 0)) {
		struct gendisk *disk = bio->bi_bdev->bd_disk;

		disk->fops->submit_bio(bio);
		blk_queue_exit(disk->queue);
	}
}
```

这说明块设备有两类：

1. 大多数普通 MQ 块设备
   - 走 `blk_mq_submit_bio()`

2. 少数 bio-based 驱动
   - 直接走 `disk->fops->submit_bio()`

我们分析重点放在第一类。

---

## 9. `blk_mq_submit_bio()`：从 bio 到 request

实现位于：

- `block/blk-mq.c`

这个函数做的事情可以概括为：

1. `bio_queue_enter()`
2. bounce / split / integrity 检查
3. 尝试 merge
4. 分配 request
5. 把 bio 填到 request
6. 选择“插 scheduler”还是“直接 issue”

关键点：

- `blk_mq_get_new_requests()`
- `blk_mq_bio_to_request()`
- `blk_mq_insert_request()`
- `blk_mq_try_issue_directly()`

所以可以理解为：

> `blk_mq_submit_bio()` 的核心任务是把块层输入 `bio` 转成驱动能执行的 `request`。

---

## 10. `request`：驱动真正看到的对象

`request` 定义在：

- `include/linux/blk-mq.h`

关键字段：

- `q`
- `mq_ctx`
- `mq_hctx`
- `cmd_flags`
- `rq_flags`
- `tag`
- `bio`
- `biotail`

可以简单理解为：

> `request` 是块层调度和驱动执行的最核心对象。

和 `bio` 的区别：

- `bio` 更像“原始 I/O 描述”
- `request` 更像“经过块层整理后的执行任务”

---

## 11. 驱动 runtime 最关键的回调：`queue_rq()`

所有 MQ 驱动最关键的运行时回调都是：

- `blk_mq_ops.queue_rq`

驱动典型工作：

1. 从 `request` 中提取扇区、长度、buffer/scatterlist
2. 转换为设备命令
3. 写 doorbell 或放进队列
4. 等设备完成
5. 最终调用 `blk_mq_end_request()`

所以：

> `queue_rq()` 是块驱动最值得重点读的 runtime 函数。

---

## 12. 完成路径

请求完成后，驱动通常调用：

- `blk_mq_end_request(rq, status)`

块层接下来会：

1. 结束 request
2. 回收 request 相关资源
3. 逐个完成其挂着的 bio
4. 最终触发 `bio_endio()`

所以 runtime 闭环是：

```text
bio -> request -> queue_rq -> hardware -> blk_mq_end_request -> bio_endio
```

---

## 13. 实际案例 1：`virtio-blk`

如果要选一个最适合 ARM QEMU 平台的真实块驱动，推荐：

- `drivers/block/virtio_blk.c`

原因：

- QEMU ARM / ARM64 virt 平台天然支持
- 是标准 blk-mq 驱动
- 初始化和 runtime 路径都很典型

### 13.1 初始化流程

`virtblk_probe()` 的典型片段：

```c
vblk->tag_set.ops = &virtio_mq_ops;
vblk->tag_set.queue_depth = queue_depth;
vblk->tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
vblk->tag_set.driver_data = vblk;
vblk->tag_set.nr_hw_queues = vblk->num_vqs;

err = blk_mq_alloc_tag_set(&vblk->tag_set);
if (err)
	goto out_free_vq;

vblk->disk = blk_mq_alloc_disk(&vblk->tag_set, vblk);
if (IS_ERR(vblk->disk)) {
	err = PTR_ERR(vblk->disk);
	goto out_free_tags;
}
```

然后继续填：

```c
vblk->disk->major = major;
vblk->disk->first_minor = index_to_minor(index);
vblk->disk->minors = 1 << PART_BITS;
vblk->disk->private_data = vblk;
vblk->disk->fops = &virtblk_fops;
```

最后：

```c
err = device_add_disk(&vdev->dev, vblk->disk, virtblk_attr_groups);
```

这几乎就是现代块驱动初始化的模板。

### 13.2 runtime 路径

`virtio-blk` 的 MQ ops：

```c
static const struct blk_mq_ops virtio_mq_ops = {
	.queue_rq	= virtio_queue_rq,
	.queue_rqs	= virtio_queue_rqs,
	.commit_rqs	= virtio_commit_rqs,
	.complete	= virtblk_request_done,
	.map_queues	= virtblk_map_queues,
	.poll		= virtblk_poll,
};
```

最关键的是：

- `virtio_queue_rq()`

它会：

1. 从 `hctx->queue->queuedata` 取到 `vblk`
2. 从 `bd->rq` 取到 `request`
3. 准备 virtio 请求头和 scatterlist
4. 塞进 virtqueue
5. 通知 host

这就是非常典型的真实块设备驱动。

---

## 14. 实际案例 2：`mtdblock`

`mtdblock` 不是原生硬件块驱动，而是 MTD 到 block 的翻译层。

它非常适合理解 block 框架本身。

### 14.1 初始化

在 `mtd_blkdevs.c` 中：

```c
gd = blk_mq_alloc_disk(new->tag_set, new);
/* ... */
gd->private_data = new;
gd->major = tr->major;
gd->first_minor = (new->devnum) << tr->part_bits;
gd->minors = 1 << tr->part_bits;
gd->fops = &mtd_block_ops;
/* ... */
ret = device_add_disk(&new->mtd->dev, gd, NULL);
```

这里和真实块驱动非常像：

- 也是 `blk_mq_alloc_disk()`
- 也是 `device_add_disk()`

### 14.2 runtime

运行时入口：

```c
static blk_status_t mtd_queue_rq(struct blk_mq_hw_ctx *hctx,
				 const struct blk_mq_queue_data *bd)
{
	struct mtd_blktrans_dev *dev;

	dev = hctx->queue->queuedata;
	/* ... */
	list_add_tail(&bd->rq->queuelist, &dev->rq_list);
	mtd_blktrans_work(dev);
	return BLK_STS_OK;
}
```

它不是发硬件命令，而是：

- 取出 request
- 再调用下层 MTD read/write

所以 `mtdblock` 更像一个“软件块设备驱动”。

---

## 15. ARM QEMU 平台实验：推荐 `virtio-blk`

如果你要在 ARM QEMU 上学习块驱动，最推荐的实验对象是：

- `virtio-blk`

### 15.1 推荐内核配置

至少打开：

- `CONFIG_BLOCK=y`
- `CONFIG_BLK_MQ=y`
- `CONFIG_VIRTIO=y`
- `CONFIG_VIRTIO_MMIO=y`
- `CONFIG_VIRTIO_BLK=y`
- `CONFIG_EXT4_FS=y`
- `CONFIG_DEVTMPFS=y`
- `CONFIG_DEVTMPFS_MOUNT=y`
- `CONFIG_PARTITION_ADVANCED=y`
- `CONFIG_EFI_PARTITION=y`
- `CONFIG_PROC_FS=y`
- `CONFIG_SYSFS=y`

### 15.2 QEMU 启动示例

#### ARM64

```bash
qemu-system-aarch64 \
  -M virt \
  -cpu cortex-a57 \
  -m 1024M \
  -nographic \
  -kernel arch/arm64/boot/Image \
  -initrd rootfs.cpio.gz \
  -append 'console=ttyAMA0 rdinit=/init' \
  -drive if=none,file=rootdisk.img,format=raw,id=vd0 \
  -device virtio-blk-device,drive=vd0
```

#### ARM32

```bash
qemu-system-arm \
  -M virt \
  -cpu cortex-a15 \
  -m 1024M \
  -nographic \
  -kernel arch/arm/boot/zImage \
  -initrd rootfs.cpio.gz \
  -append 'console=ttyAMA0 rdinit=/init' \
  -drive if=none,file=rootdisk.img,format=raw,id=vd0 \
  -device virtio-blk-device,drive=vd0
```

---

## 16. 实验步骤

### 16.1 启动后确认设备存在

```bash
cat /proc/partitions
ls -l /dev/vd*
```

预期能看到：

- `vda`

### 16.2 看 sysfs queue 参数

```bash
cat /sys/block/vda/queue/logical_block_size
cat /sys/block/vda/queue/physical_block_size
cat /sys/block/vda/queue/max_hw_sectors_kb
cat /sys/block/vda/queue/nr_requests
```

这能帮助你理解：

- queue limits 是如何从驱动设置出去的

### 16.3 分区 + 文件系统实验

```bash
fdisk /dev/vda
mkfs.ext4 /dev/vda1
mount /dev/vda1 /mnt
echo hello > /mnt/hello.txt
sync
cat /mnt/hello.txt
```

### 16.4 运行时 I/O 实验

```bash
dd if=/dev/zero of=/mnt/test.bin bs=1M count=64 conv=fsync
dd if=/mnt/test.bin of=/dev/null bs=1M
```

### 16.5 观察 dmesg

```bash
dmesg | grep -i virtio
```

---

## 17. 如果你想看 runtime 更细

可以开启 block tracepoints：

```bash
echo 1 > /sys/kernel/debug/tracing/events/block/block_rq_issue/enable
echo 1 > /sys/kernel/debug/tracing/events/block/block_rq_complete/enable
cat /sys/kernel/debug/tracing/trace_pipe
```

再执行：

```bash
dd if=/dev/zero of=/mnt/test.bin bs=4K count=16 conv=fsync
```

你就能看到 request 的 issue / complete 路径。

---

## 18. 推荐阅读顺序

如果你准备继续深入源码，推荐按这个顺序：

1. `include/linux/blk-mq.h`
   看 `struct request`

2. `include/linux/blk_types.h`
   看 `struct bio`

3. `block/blk-mq.c`
   看：
   - `blk_mq_alloc_disk()`
   - `blk_mq_submit_bio()`

4. `block/genhd.c`
   看：
   - `device_add_disk()`

5. `drivers/block/virtio_blk.c`
   看真实块驱动

6. `drivers/mtd/mtd_blkdevs.c`
   看软件块设备翻译层

---

## 19. 一句话总结

Linux block 驱动初始化的关键是：

> 用 `blk_mq_alloc_disk()` 建立 queue 和 disk，再用 `device_add_disk()` 把它正式接入系统。

runtime 的关键是：

> `bio` 经过 `blk_mq_submit_bio()` 变成 `request`，最终由驱动的 `queue_rq()` 处理。

如果你要在 ARM QEMU 上做实验，最推荐从：

- `virtio-blk`

开始，因为它最接近标准的现代 Linux block 驱动。
