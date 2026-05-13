# Linux Block Device 调用链速查

这份文档聚焦 Linux block device 的两条主线：

1. 初始化时序
2. runtime I/O 时序

适合做源码速查，不追求大段背景解释。

---

## 1. 初始化主线

现代 MQ 块驱动的典型初始化顺序：

```text
驱动 probe/init
    |
    +--> 填 struct blk_mq_tag_set
    |
    +--> blk_mq_alloc_tag_set()
    |
    +--> blk_mq_alloc_disk()
    |       |
    |       +--> blk_mq_init_queue_data()
    |       |       |
    |       |       +--> 创建 request_queue
    |       |
    |       +--> __alloc_disk_node()
    |               |
    |               +--> 创建 gendisk
    |
    +--> 驱动填写 gendisk
    |       |
    |       +--> disk_name
    |       +--> major / first_minor / minors
    |       +--> fops
    |       +--> private_data
    |       +--> set_capacity()
    |
    +--> 设置 queue limits
    |       |
    |       +--> blk_queue_logical_block_size()
    |       +--> blk_queue_max_hw_sectors()
    |       +--> blk_queue_max_segments()
    |       +--> discard / write_zeroes / io_opt 等
    |
    +--> device_add_disk()
            |
            +--> elevator_init_mq()
            +--> device_add()
            +--> blk_register_queue()
            +--> bdi_register()
            +--> bdev_add(part0)
            +--> disk_scan_partitions()
            +--> disk_uevent(KOBJ_ADD)
```

---

## 2. `blk_mq_alloc_disk()` 速记

可以直接记成：

```text
blk_mq_alloc_disk()
    = 创建 request_queue + 创建 gendisk + 二者绑定
```

等价理解：

- 之前：只有 `tag_set`
- 之后：你拿到了 `disk`，且 `disk->queue` 已准备好

但它还没有进入设备模型。

---

## 3. `device_add_disk()` 速记

可以直接记成：

```text
device_add_disk()
    = 把已经准备好的 gendisk 正式注册到系统
```

执行后你通常能观察到：

- `/sys/block/<disk>`
- `/dev/<disk>`
- 自动分区扫描结果
- uevent 通知用户空间

---

## 4. runtime I/O 主线

块设备运行时 I/O 主线可以记成：

```text
用户 read/write
    |
    +--> 文件系统 / 页缓存 / direct IO
    |
    +--> submit_bio()
    |
    +--> blk_mq_submit_bio()
    |       |
    |       +--> bio_queue_enter()
    |       +--> split / bounce / integrity
    |       +--> merge
    |       +--> blk_mq_get_new_requests()
    |       +--> blk_mq_bio_to_request()
    |       +--> insert scheduler / direct issue
    |
    +--> 驱动 queue_rq()
    |       |
    |       +--> 组装硬件命令
    |       +--> 提交到设备
    |
    +--> 中断/完成回调
    |
    +--> blk_mq_end_request()
    |
    +--> bio_endio()
```

---

## 5. 关键对象在 runtime 里的位置

### 5.1 `bio`

代表块层输入 I/O。

关心：

- `bio->bi_bdev`
- `bio->bi_opf`
- `bio->bi_iter`
- `bio->bi_end_io`

### 5.2 `request`

代表驱动执行单元。

关心：

- `rq->q`
- `rq->mq_hctx`
- `rq->bio`
- `rq->tag`

### 5.3 `blk_mq_hw_ctx`

代表一个硬件队列上下文。

驱动的 `queue_rq()` 第一个参数就是它。

### 5.4 `request_queue`

所有 runtime I/O 都会围绕 queue 转。

驱动通常通过：

- `hctx->queue`
- `rq->q`

拿到它。

---

## 6. `virtio-blk` 初始化时序

典型路径：

```text
virtblk_probe()
    |
    +--> 填 vblk->tag_set
    |
    +--> blk_mq_alloc_tag_set(&vblk->tag_set)
    |
    +--> blk_mq_alloc_disk(&vblk->tag_set, vblk)
    |       |
    |       +--> queue->queuedata = vblk
    |
    +--> disk->major / first_minor / minors / fops / private_data
    +--> virtblk_update_capacity()
    +--> 各种 blk_queue_* limits
    +--> virtio_device_ready()
    +--> device_add_disk(&vdev->dev, vblk->disk, ...)
```

这是一条非常标准的现代块驱动初始化链路。

---

## 7. `virtio-blk` runtime 时序

```text
submit_bio()
    |
    +--> blk_mq_submit_bio()
    |
    +--> 生成 request
    |
    +--> virtio_queue_rq(hctx, bd)
            |
            +--> virtblk_prep_rq()
            +--> virtblk_add_req(vq, vbr)
            +--> virtqueue_notify()

设备完成后：

virtqueue completion
    |
    +--> virtblk_request_done()
    |
    +--> blk_mq_end_request(rq, status)
    |
    +--> bio_endio()
```

所以 `virtio-blk` 是理解：

- `queue_rq`
- request 提交
- completion

最好的真实驱动案例之一。

---

## 8. `mtdblock` 初始化时序

`mtdblock` 不是原生块控制器驱动，而是 MTD 翻译层。

其路径可以记成：

```text
module_mtd_blktrans(mtdblock_tr)
    |
    +--> register_mtd_blktrans(&mtdblock_tr)
            |
            +--> register_mtd_user(&blktrans_notifier)
            +--> register_blkdev(MTD_BLOCK_MAJOR, "mtdblock")
            +--> 遍历已有 MTD
                    |
                    +--> mtdblock_add_mtd(tr, mtd)
                            |
                            +--> add_mtd_blktrans_dev(&dev->mbd)
                                    |
                                    +--> blk_mq_alloc_disk()
                                    +--> device_add_disk(&mtd->dev, gd, NULL)
                                    +--> /dev/mtdblockX
```

如果以后新增 MTD 设备：

```text
add_mtd_device(mtd)
    |
    +--> blktrans_notify_add(mtd)
            |
            +--> mtdblock_add_mtd()
                    |
                    +--> add_mtd_blktrans_dev()
```

---

## 9. `mtdblock` runtime 时序

```text
/dev/mtdblock0
    |
    +--> 块层 request
    |
    +--> mtd_queue_rq()
    |
    +--> mtd_blktrans_work()
    |
    +--> do_blktrans_request()
            |
            +--> mtdblock_readsect()
            +--> mtdblock_writesect()
            +--> mtdblock_flush()
```

它不是发硬件命令，而是转成对下层 MTD 的访问。

所以它适合理解：

- `blk_mq_alloc_disk()`
- `device_add_disk()`
- `queue_rq()`

这套共性流程。

---

## 10. 重点函数速查

### 初始化相关

- `blk_mq_alloc_tag_set()`
- `blk_mq_alloc_disk()`
- `device_add_disk()`
- `register_blkdev()`
- `set_capacity()`

### runtime 相关

- `submit_bio()`
- `blk_mq_submit_bio()`
- `blk_mq_get_new_requests()`
- `blk_mq_bio_to_request()`
- `queue_rq()`
- `blk_mq_end_request()`
- `bio_endio()`

### 常见对象

- `struct blk_mq_tag_set`
- `struct request_queue`
- `struct gendisk`
- `struct bio`
- `struct request`

---

## 11. ARM QEMU 实验推荐

最推荐用：

- `virtio-blk`

### 启动示例

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

### 建议观察

```bash
cat /proc/partitions
ls -l /dev/vd*
cat /sys/block/vda/queue/logical_block_size
cat /sys/block/vda/queue/max_hw_sectors_kb
```

### I/O 验证

```bash
dd if=/dev/zero of=/tmp/test.bin bs=1M count=32 conv=fsync
dd if=/tmp/test.bin of=/dev/null bs=1M
```

### 配合 tracepoints

```bash
echo 1 > /sys/kernel/debug/tracing/events/block/block_rq_issue/enable
echo 1 > /sys/kernel/debug/tracing/events/block/block_rq_complete/enable
cat /sys/kernel/debug/tracing/trace_pipe
```

---

## 12. 最推荐的阅读顺序

1. `block/blk-mq.c`
   先看 `blk_mq_alloc_disk()`、`blk_mq_submit_bio()`

2. `block/genhd.c`
   看 `device_add_disk()`

3. `drivers/block/virtio_blk.c`
   看真实块设备驱动

4. `drivers/mtd/mtd_blkdevs.c`
   看 block 翻译层

5. `drivers/mtd/mtdblock.c`
   看具体实现

---

## 13. 一句话总结

初始化角度：

> `blk_mq_alloc_disk()` 创建 queue 和 disk，`device_add_disk()` 让这个磁盘正式进入系统。

runtime 角度：

> `bio` 经由 `blk_mq_submit_bio()` 变成 `request`，再由驱动 `queue_rq()` 提交到底层设备。
