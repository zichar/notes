# `module_mtd_blktrans(mtdblock_tr)` 调用流程分析

本文聚焦：

- `drivers/mtd/mtdblock.c`
- `drivers/mtd/mtd_blkdevs.c`

目标是解释：

```c
module_mtd_blktrans(mtdblock_tr);
```

到底会触发什么调用流程，以及它如何把一个 MTD 设备变成 `/dev/mtdblockX` 块设备。

---

## 1. 先说结论

`module_mtd_blktrans(mtdblock_tr);` 的本质作用是：

> 把 `mtdblock_tr` 注册成一个 MTD block translation driver，并自动生成模块加载和卸载入口。

加载后，它会：

1. 注册 block major
2. 注册 MTD notifier
3. 扫描系统中已有的 MTD 设备
4. 为每个合适的 MTD 设备调用 `mtdblock_add_mtd()`
5. 最终通过块层创建 `/dev/mtdblockX`

之后，只要系统里再新增 MTD 设备，也会自动收到 notifier 回调并继续创建设备节点。

---

## 2. `mtdblock_tr` 是什么

在 `drivers/mtd/mtdblock.c` 中：

```c
static struct mtd_blktrans_ops mtdblock_tr = {
	.name		= "mtdblock",
	.major		= MTD_BLOCK_MAJOR,
	.part_bits	= 0,
	.blksize 	= 512,
	.open		= mtdblock_open,
	.flush		= mtdblock_flush,
	.release	= mtdblock_release,
	.readsect	= mtdblock_readsect,
	.writesect	= mtdblock_writesect,
	.add_mtd	= mtdblock_add_mtd,
	.remove_dev	= mtdblock_remove_dev,
	.owner		= THIS_MODULE,
};
```

这是一个 `struct mtd_blktrans_ops`，表示：

- 名字叫 `mtdblock`
- 是一个块翻译层
- 块大小 512 字节
- 底层读写接口是按 sector 方式提供的
- 新增 MTD 时如何创建设备：`add_mtd`
- 删除 MTD 时如何删设备：`remove_dev`

这不是具体设备，而是“驱动描述符”。

---

## 3. `module_mtd_blktrans()` 宏展开

定义在：

- `include/linux/mtd/blktrans.h`

```c
#define module_mtd_blktrans(__mtd_blktrans) \
	module_driver(__mtd_blktrans, register_mtd_blktrans, \
					deregister_mtd_blktrans)
```

它又依赖：

- `include/linux/device/driver.h`

```c
#define module_driver(__driver, __register, __unregister, ...) \
static int __init __driver##_init(void) \
{ \
	return __register(&(__driver) , ##__VA_ARGS__); \
} \
module_init(__driver##_init); \
static void __exit __driver##_exit(void) \
{ \
	__unregister(&(__driver) , ##__VA_ARGS__); \
} \
module_exit(__driver##_exit);
```

因此：

```c
module_mtd_blktrans(mtdblock_tr);
```

实际效果等价于：

```c
static int __init mtdblock_tr_init(void)
{
	return register_mtd_blktrans(&mtdblock_tr);
}
module_init(mtdblock_tr_init);

static void __exit mtdblock_tr_exit(void)
{
	deregister_mtd_blktrans(&mtdblock_tr);
}
module_exit(mtdblock_tr_exit);
```

也就是说：

- 模块加载时会调用 `register_mtd_blktrans(&mtdblock_tr)`
- 模块卸载时会调用 `deregister_mtd_blktrans(&mtdblock_tr)`

---

## 4. 模块加载主流程：`register_mtd_blktrans()`

实现位于：

- `drivers/mtd/mtd_blkdevs.c`

核心代码逻辑：

```c
int register_mtd_blktrans(struct mtd_blktrans_ops *tr)
{
	struct mtd_info *mtd;
	int ret;

	if (!blktrans_notifier.list.next)
		register_mtd_user(&blktrans_notifier);

	ret = register_blkdev(tr->major, tr->name);
	if (ret < 0)
		return ret;

	if (ret)
		tr->major = ret;

	tr->blkshift = ffs(tr->blksize) - 1;
	INIT_LIST_HEAD(&tr->devs);

	mutex_lock(&mtd_table_mutex);
	list_add(&tr->list, &blktrans_majors);
	mtd_for_each_device(mtd)
		if (mtd->type != MTD_ABSENT && mtd->type != MTD_UBIVOLUME)
			tr->add_mtd(tr, mtd);
	mutex_unlock(&mtd_table_mutex);

	return 0;
}
```

这一步可以拆成 4 件事。

### 4.1 注册 notifier

如果这是第一个 blktrans 驱动，就先注册：

- `blktrans_notifier`

定义是：

```c
static struct mtd_notifier blktrans_notifier = {
	.add = blktrans_notify_add,
	.remove = blktrans_notify_remove,
};
```

作用：

- 以后有新的 MTD 设备加入时，blktrans 层能收到通知
- 以后有 MTD 设备移除时，blktrans 层也能删掉对应块设备

### 4.2 注册块设备 major

```c
ret = register_blkdev(tr->major, tr->name);
```

对 `mtdblock` 来说：

- 注册的是 `MTD_BLOCK_MAJOR`
- 名字是 `"mtdblock"`

### 4.3 记录块大小 shift

```c
tr->blkshift = ffs(tr->blksize) - 1;
```

对于 `mtdblock`：

- `blksize = 512`
- 所以 `blkshift = 9`

### 4.4 扫描系统中已有的 MTD

```c
mtd_for_each_device(mtd)
	if (mtd->type != MTD_ABSENT && mtd->type != MTD_UBIVOLUME)
		tr->add_mtd(tr, mtd);
```

这意味着：

> 如果 `mtdblock` 加载时系统里已经有 MTD 设备，它会立即为这些设备补建 `mtdblockX`。

不需要等将来新增设备。

---

## 5. 新 MTD 设备出现时的路径

如果 `mtdblock` 已经加载好，而这时系统里后来才新增一个 MTD 设备，例如：

- `phram`
- 某个 SPI NOR
- 某个 NAND

那么路径是：

1. MTD 核心调用 `add_mtd_device()`
2. `add_mtd_device()` 遍历 `mtd_notifiers`
3. `blktrans_notifier.add(mtd)` 被调用
4. 进入 `blktrans_notify_add()`
5. 遍历所有已注册 blktrans 驱动
6. 对每个驱动调用 `tr->add_mtd(tr, mtd)`

代码：

```c
static void blktrans_notify_add(struct mtd_info *mtd)
{
	struct mtd_blktrans_ops *tr;

	if (mtd->type == MTD_ABSENT || mtd->type == MTD_UBIVOLUME)
		return;

	list_for_each_entry(tr, &blktrans_majors, list)
		tr->add_mtd(tr, mtd);
}
```

所以：

> `mtdblock` 不是主动轮询 MTD，而是通过 notifier 被动接收新增事件。

---

## 6. `mtdblock_add_mtd()` 做了什么

在 `drivers/mtd/mtdblock.c`：

```c
static void mtdblock_add_mtd(struct mtd_blktrans_ops *tr, struct mtd_info *mtd)
{
	struct mtdblk_dev *dev = kzalloc(sizeof(*dev), GFP_KERNEL);

	if (!dev)
		return;

	dev->mbd.mtd = mtd;
	dev->mbd.devnum = mtd->index;
	dev->mbd.size = mtd->size >> 9;
	dev->mbd.tr = tr;

	if (!(mtd->flags & MTD_WRITEABLE))
		dev->mbd.readonly = 1;

	if (add_mtd_blktrans_dev(&dev->mbd))
		kfree(dev);
}
```

核心动作：

1. 分配 `mtdblk_dev`
2. 把底层 `mtd_info` 绑定进来
3. 用 `mtd->index` 作为 `devnum`
4. 把容量转成 512-byte sectors
5. 如不可写则标记只读
6. 调 `add_mtd_blktrans_dev()`

所以：

> `mtdblock_add_mtd()` 的作用是把一个 MTD 对象包装成一个可注册到块层的 blktrans 设备对象。

---

## 7. 真正创建 `/dev/mtdblockX`：`add_mtd_blktrans_dev()`

这个函数位于：

- `drivers/mtd/mtd_blkdevs.c`

它是整个块设备生成流程里最核心的实现。

### 7.1 分配和插入设备号

它会先在 `tr->devs` 链表中分配一个 `devnum`。

对于 `mtdblock` 来说，通常直接用：

- `dev->mbd.devnum = mtd->index`

所以：

- `mtd0` 往往对应 `mtdblock0`
- `mtd1` 对应 `mtdblock1`

### 7.2 分配 blk-mq tag set

```c
new->tag_set = kzalloc(sizeof(*new->tag_set), GFP_KERNEL);
blk_mq_alloc_sq_tag_set(new->tag_set, &mtd_mq_ops, 2,
			BLK_MQ_F_SHOULD_MERGE | BLK_MQ_F_BLOCKING);
```

说明：

- `mtdblock` 设备走的是 blk-mq

### 7.3 分配 `gendisk`

```c
gd = blk_mq_alloc_disk(new->tag_set, new);
new->disk = gd;
```

### 7.4 填块设备基本信息

```c
gd->private_data = new;
gd->major = tr->major;
gd->first_minor = (new->devnum) << tr->part_bits;
gd->minors = 1 << tr->part_bits;
gd->fops = &mtd_block_ops;
```

这里把它正式变成一个块设备实例。

### 7.5 设置磁盘名字

由于 `mtdblock_tr.part_bits = 0`，所以命名走：

```c
snprintf(gd->disk_name, sizeof(gd->disk_name),
	 "%s%d", tr->name, new->devnum);
```

也就是：

- `mtdblock0`
- `mtdblock1`

### 7.6 设置容量

```c
set_capacity(gd, ((u64)new->size * tr->blksize) >> 9);
```

### 7.7 注册到块层

最后一步：

```c
ret = device_add_disk(&new->mtd->dev, gd, NULL);
```

到这里块设备才真正进入系统，通常会对应看到：

- `/dev/mtdblockX`

所以最关键的一句话是：

> `/dev/mtdblockX` 不是 `mtdblock_add_mtd()` 直接创建出来的，而是 `add_mtd_blktrans_dev()` 中通过 `gendisk + device_add_disk()` 创建出来的。

---

## 8. 请求 I/O 时怎么走

生成块设备后，如果用户态去访问：

- `/dev/mtdblock0`

则后续走块层。

### 8.1 block ops

块设备操作是：

```c
static const struct block_device_operations mtd_block_ops = {
	.owner		= THIS_MODULE,
	.open		= blktrans_open,
	.release	= blktrans_release,
	.getgeo		= blktrans_getgeo,
};
```

### 8.2 request queue

请求队列处理是：

```c
static const struct blk_mq_ops mtd_mq_ops = {
	.queue_rq	= mtd_queue_rq,
};
```

### 8.3 进入 `do_blktrans_request()`

最终请求会转到：

- `do_blktrans_request()`

然后根据 request 类型调用 `tr->readsect` / `tr->writesect` / `tr->flush`。

对 `mtdblock_tr` 来说，对应的是：

- `mtdblock_readsect`
- `mtdblock_writesect`
- `mtdblock_flush`

因此从 I/O 角度可把路径记成：

```text
/dev/mtdblock0
  -> 块层 request
  -> mtd_queue_rq()
  -> do_blktrans_request()
  -> mtdblock_readsect()/mtdblock_writesect()
  -> 底层 MTD 读写
```

---

## 9. 卸载流程

`module_mtd_blktrans(mtdblock_tr)` 还自动生成了退出函数：

- `deregister_mtd_blktrans(&mtdblock_tr)`

实现逻辑：

```c
int deregister_mtd_blktrans(struct mtd_blktrans_ops *tr)
{
	mutex_lock(&mtd_table_mutex);

	list_del(&tr->list);

	list_for_each_entry_safe(dev, next, &tr->devs, list)
		tr->remove_dev(dev);

	mutex_unlock(&mtd_table_mutex);
	unregister_blkdev(tr->major, tr->name);

	return 0;
}
```

对 `mtdblock` 来说：

- `remove_dev = mtdblock_remove_dev`

而它只是：

```c
static void mtdblock_remove_dev(struct mtd_blktrans_dev *dev)
{
	del_mtd_blktrans_dev(dev);
}
```

这会把对应 `gendisk`、队列、sysfs、块设备节点都删掉。

---

## 10. 和 `phram` 的关系

如果系统中有 `phram`，那么两种时序都成立：

### 情况 A：先有 `phram`，后加载 `mtdblock`

路径：

1. `phram` 已经通过 `add_mtd_device()` 注册为 `mtd0`
2. `mtdblock` 加载，进入 `register_mtd_blktrans()`
3. `mtd_for_each_device(mtd)` 扫描到 `mtd0`
4. `mtdblock_add_mtd(tr, mtd0)`
5. `add_mtd_blktrans_dev()`
6. `/dev/mtdblock0`

### 情况 B：先有 `mtdblock`，后出现 `phram`

路径：

1. `mtdblock` 先加载并注册 notifier
2. `phram` 后续注册 `mtd0`
3. `add_mtd_device()` 通知 `blktrans_notifier`
4. `blktrans_notify_add(mtd0)`
5. `mtdblock_add_mtd(tr, mtd0)`
6. `add_mtd_blktrans_dev()`
7. `/dev/mtdblock0`

所以：

> `mtdblock` 既能处理“已有 MTD”，也能处理“未来新增 MTD”。

---

## 11. 总调用时序图

可以把 `module_mtd_blktrans(mtdblock_tr)` 对应的完整链路总结成下面这张图：

```text
module_mtd_blktrans(mtdblock_tr)
    |
    +--> 生成 module_init/module_exit
            |
            +--> register_mtd_blktrans(&mtdblock_tr)
                    |
                    +--> register_mtd_user(&blktrans_notifier)
                    +--> register_blkdev(MTD_BLOCK_MAJOR, "mtdblock")
                    +--> list_add(&mtdblock_tr.list, &blktrans_majors)
                    +--> 遍历已有 mtd_for_each_device(mtd)
                            |
                            +--> mtdblock_add_mtd(tr, mtd)
                                    |
                                    +--> add_mtd_blktrans_dev(&dev->mbd)
                                            |
                                            +--> blk_mq_alloc_disk()
                                            +--> 设置 disk_name = mtdblockX
                                            +--> device_add_disk()
                                                    |
                                                    +--> /dev/mtdblockX

后续若新 MTD 设备出现：

add_mtd_device(mtd)
    |
    +--> list_for_each_entry(not, &mtd_notifiers, list)
            |
            +--> blktrans_notify_add(mtd)
                    |
                    +--> list_for_each_entry(tr, &blktrans_majors, list)
                            |
                            +--> mtdblock_add_mtd(tr, mtd)
                                    |
                                    +--> add_mtd_blktrans_dev()
                                            |
                                            +--> /dev/mtdblockX
```

---

## 12. 一句话总结

`module_mtd_blktrans(mtdblock_tr);` 的本质不是“执行一个普通函数”，而是：

> 把 `mtdblock_tr` 注册为一个 MTD 块翻译层驱动，并自动把它接到 MTD notifier 和块层框架上，使它能够为所有现有和未来新增的 MTD 设备自动生成 `mtdblockX` 块设备。
