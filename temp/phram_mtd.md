# phram 注册 MTD 设备及 `mtd_device_register()` 后续流程分析

本文聚焦 `phram` 驱动，梳理它如何把一段物理 RAM 注册成 MTD 设备，以及调用 `mtd_device_register()` 之后，MTD 核心还会继续做哪些事情。

---

## 1. `phram` 是什么

`phram` 的全称可以理解成：

- **MTD in PHysical RAM**

它的作用是：

> 把一段物理内存映射成一个标准的 MTD 设备。

这样上层就可以把这段 RAM 当成 flash 风格设备来使用，例如：

- 通过 `/dev/mtdX` 读写
- 通过 `/dev/mtdblockX` 走块设备接口
- 在其上挂载 `jffs2`、`romfs`、`cramfs` 等 MTD 文件系统

---

## 2. `phram` 的两种入口

`phram` 支持两种创建设备的方式：

1. 通过内核参数 / 模块参数
2. 通过设备树平台设备

这两条路径最终都会汇聚到同一个核心函数：

- `register_device()`

---

## 3. 模块初始化入口

文件：

- `drivers/mtd/devices/phram.c`

初始化函数：

```c
static int __init init_phram(void)
{
	int ret;

	ret = platform_driver_register(&phram_driver);
	if (ret)
		return ret;

#ifndef MODULE
	if (phram_paramline[0])
		ret = phram_setup(phram_paramline);
	phram_init_called = 1;
#endif

	if (ret)
		platform_driver_unregister(&phram_driver);

	return ret;
}
```

这里说明：

- `phram` 是一个 platform driver
- 如果是 built-in，并且命令行中带了 `phram.phram=...`，那么初始化时会继续调用 `phram_setup()`

---

## 4. 参数方式注册流程

### 4.1 参数格式

支持格式：

```text
phram=<name>,<start>,<len>[,<erasesize>]
phram.phram=<name>,<start>,<len>[,<erasesize>]
```

例如：

```text
phram.phram=ph0,0x4f800000,8Mi,64Ki
```

### 4.2 调用链

参数方式大致调用链：

1. `phram_param_call()`
2. `phram_setup()`
3. `register_device(NULL, name, start, len, erasesize)`

`phram_setup()` 主要做：

- 解析名字
- 解析起始地址
- 解析长度
- 解析擦除块大小
- 检查长度是否为 `erasesize` 的整数倍
- 调用 `register_device()`

也就是说：

> 参数入口本质上只是把字符串转成 `register_device()` 所需的结构化参数。

---

## 5. DT 方式注册流程

如果设备树节点 `compatible = "phram"`，则会匹配到：

```c
static struct platform_driver phram_driver = {
	.probe		= phram_probe,
	.remove		= phram_remove,
	.driver		= {
		.name		= "phram",
		.of_match_table	= of_match_ptr(phram_of_match),
	},
};
```

进入 `phram_probe()`：

```c
static int phram_probe(struct platform_device *pdev)
{
	struct resource *res;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENOMEM;

	return register_device(pdev, NULL, res->start, resource_size(res),
			       PAGE_SIZE);
}
```

也就是说：

- 从 `reg` 取物理地址范围
- 从 DT 的 `label` 提取名字
- 再调用 `register_device()`

---

## 6. 核心函数：`register_device()`

这一步才是真正把“物理 RAM”变成“MTD 设备”。

它主要做 4 件事：

1. 分配私有结构
2. 映射物理内存
3. 填充 `struct mtd_info`
4. 调用 `mtd_device_register()`

### 6.1 分配 `phram_mtd_list`

`phram` 用一个私有结构保存：

- `struct mtd_info mtd`
- 链表节点
- `cached` 标志

### 6.2 映射内存：`phram_map()`

如果 `cached`：

- `memremap(start, len, MEMREMAP_WB)`

否则：

- `ioremap(start, len)`

映射结果保存在：

- `mtd.priv`

这一步非常关键，因为后面的 `_read/_write/_erase` 都直接基于 `mtd->priv` 进行内存访问。

### 6.3 填充 `mtd_info`

`phram` 把这段 RAM 描述成一个标准的 MTD RAM 设备：

- `mtd.name`
- `mtd.size`
- `mtd.flags = MTD_CAP_RAM`
- `mtd.type = MTD_RAM`
- `mtd.erasesize`
- `mtd.writesize = 1`

并绑定操作函数：

- `_erase = phram_erase`
- `_point = phram_point`
- `_unpoint = phram_unpoint`
- `_read = phram_read`
- `_write = phram_write`

这些函数本质上就是：

- `erase` -> `memset(..., 0xff, ...)`
- `read` -> `memcpy(...)`
- `write` -> `memcpy(...)`

### 6.4 注册进 MTD 核心

最后调用：

```c
mtd_device_register(&new->mtd, NULL, 0)
```

这里参数表示：

- 不提供 parser
- 不提供静态分区
- 注册整个 master MTD

---

## 7. `mtd_device_register()` 实际走的是谁

`mtd_device_register()` 并不是独立函数，而是宏：

```c
#define mtd_device_register(master, parts, nr_parts) \
	mtd_device_parse_register(master, NULL, NULL, parts, nr_parts)
```

因此 `phram` 实际调用的是：

```c
mtd_device_parse_register(&new->mtd, NULL, NULL, NULL, 0)
```

由于：

- 没有 parser
- 没有静态分区

所以最终会走到：

- `add_mtd_device(mtd)`

---

## 8. `mtd_device_parse_register()` 在 `phram` 场景下的行为

它的通用逻辑是：

1. 设置 device 默认值
2. 注册 OTP/nvmem（如果支持）
3. 尝试解析分区
4. 如果分区找不到，就注册整个 master MTD

对于 `phram` 而言：

- `parse_mtd_partitions()` 通常不会找到任何分区
- `parts == NULL`
- `nr_parts == 0`

于是最终注册整个设备本身。

也就是说：

> `phram` 默认是“整片设备直接注册”，不是“注册一个 master 再带若干 partitions”。

---

## 9. 真正关键的后续函数：`add_mtd_device()`

这才是 `mtd_device_register()` 后续主流程的核心。

### 9.1 基本合法性检查

`add_mtd_device()` 会先检查：

- 是否重复注册
- `writesize` 是否有效
- `_read/_write` 与 `_read_oob/_write_oob` 是否冲突
- `erasesize` 是否合理

对 `phram` 来说，这些条件通常都满足。

### 9.2 分配 MTD 设备编号

通过 `idr_alloc(&mtd_idr, ...)` 分配：

- `mtd->index`

例如：

- `mtd0`
- `mtd1`

这就是后续：

- `/proc/mtd`
- `/dev/mtd0`
- `/dev/mtd0ro`
- `/dev/mtdblock0`

这些编号的来源。

### 9.3 填派生字段

会根据：

- `erasesize`
- `writesize`

计算：

- `erasesize_shift`
- `writesize_shift`
- `erasesize_mask`
- `writesize_mask`

这些是 MTD 核心后续访问优化和对齐计算用的。

### 9.4 注册到 Linux 设备模型

这一步最重要：

- 设置 `mtd->dev.type`
- 设置 `mtd->dev.class = &mtd_class`
- 设置 `mtd->dev.devt`
- 设置设备名为 `mtdX`
- 调 `device_register(&mtd->dev)`

效果是：

- `phram` 设备变成一个真正的 Linux device
- 能出现在 sysfs / devtmpfs 中

### 9.5 创建 `mtdXro`

随后 MTD 核心还会额外创建一个只读设备：

- `mtd0ro`

因此通常可见：

- `/dev/mtd0`
- `/dev/mtd0ro`

### 9.6 注册 nvmem provider

`add_mtd_device()` 还会调用：

- `mtd_nvmem_add(mtd)`

所以某些 MTD 设备还能通过 nvmem 子系统使用。

### 9.7 创建 debugfs

还会调用：

- `mtd_debugfs_populate(mtd)`

用于调试。

### 9.8 通知所有 MTD 用户

这是后续扩散的关键。

`add_mtd_device()` 会遍历：

- `mtd_notifiers`

对每个 notifier 调用：

- `not->add(mtd)`

这一步会让：

- `mtdblock`
- 一些 FTL/翻译层
- 其他 MTD 用户

知道“系统里新增了一个 MTD 设备”。

---

## 10. `/proc/mtd` 为什么能看到 `phram`

`/proc/mtd` 不是注册时主动写进去的，而是在读取时遍历全局 MTD 表。

显示函数会遍历：

- 所有已注册 MTD 设备

并打印：

- `mtd->index`
- `mtd->size`
- `mtd->erasesize`
- `mtd->name`

因此一旦 `add_mtd_device()` 成功、`phram` 被插入全局 MTD 表，
`/proc/mtd` 中就会立即可见。

---

## 11. 为什么 `phram` 后面还会影响 `mtdblock`

这是很多人最关心的一步：

> 为什么 `phram` 注册成 MTD 后，我还能看到 `/dev/mtdblock0`？

原因是：

- `mtdblock` 并不是 `phram` 自己创建的
- 而是 MTD block translation layer 通过 notifier 自动感知后创建的

### 11.1 `mtd_blkdevs` 注册 notifier

`mtd_blkdevs.c` 里定义了：

- `blktrans_notifier`

其 `.add` 回调为：

- `blktrans_notify_add`

当块翻译层初始化时，它会通过：

- `register_mtd_user(&blktrans_notifier)`

注册到 MTD 通知链。

### 11.2 新 MTD 到来时，block translation 层收到通知

当 `add_mtd_device()` 遍历 `mtd_notifiers` 时，`blktrans_notifier.add(mtd)` 会被调用。

在 `blktrans_notify_add()` 中：

- 遍历所有已注册的 `mtd_blktrans_ops`
- 逐个调用 `tr->add_mtd(tr, mtd)`

### 11.3 `mtdblock` 的 `add_mtd`

如果系统启用了 `CONFIG_MTD_BLOCK`，`mtdblock` 会注册自己的 `mtd_blktrans_ops`。

它的核心回调是：

- `mtdblock_add_mtd()`

它会：

1. 分配一个 `mtdblk_dev`
2. 绑定到底层 `mtd`
3. 设置 `devnum = mtd->index`
4. 调用 `add_mtd_blktrans_dev()`

最终会生成：

- `/dev/mtdblock0`

所以：

> `phram -> add_mtd_device() -> notifier -> mtdblock_add_mtd() -> /dev/mtdblock0`

这就是自动出现 block 节点的根本原因。

---

## 12. `phram` 之后设备节点的整体结果

如果：

- `CONFIG_MTD=y`
- `CONFIG_MTD_BLOCK=y`
- devtmpfs/udev 正常

那么 `phram` 成功注册之后，通常可以看到：

### 字符设备节点

- `/dev/mtd0`
- `/dev/mtd0ro`

### 块设备节点

- `/dev/mtdblock0`

### proc/sysfs/debugfs

- `/proc/mtd`
- `/sys/class/mtd/mtd0/`
- `debugfs` 下的 mtd 目录

---

## 13. `phram` 后续流程时序图

可以把整条链简化成下面这样：

```text
phram_setup()/phram_probe()
        |
        v
register_device()
        |
        +--> phram_map()
        |      |
        |      +--> memremap/ioremap
        |
        +--> fill mtd_info
        |
        +--> mtd_device_register()
               |
               v
        mtd_device_parse_register()
               |
               +--> parse_mtd_partitions() -> 没有分区
               |
               +--> add_mtd_device()
                       |
                       +--> 分配 mtd->index
                       +--> device_register(&mtd->dev)
                       +--> 创建 mtdXro
                       +--> mtd_nvmem_add()
                       +--> mtd_debugfs_populate()
                       +--> 通知 mtd_notifiers
                               |
                               +--> blktrans_notify_add()
                                       |
                                       +--> mtdblock_add_mtd()
                                               |
                                               +--> add_mtd_blktrans_dev()
                                                       |
                                                       +--> /dev/mtdblockX
```

---

## 14. 注销流程

### 参数方式注册

走：

- `unregister_devices()`

逐个做：

1. `mtd_device_unregister(&this->mtd)`
2. `phram_unmap(this)`
3. 释放名字和结构体

### DT / platform 方式

走：

- `phram_remove()`

做：

1. `mtd_device_unregister(&phram->mtd)`
2. `phram_unmap(phram)`
3. `kfree(phram)`

也就是说：

注册和注销都是很标准的 MTD 生命周期管理。

---

## 15. 一句话总结

针对 `phram`，`mtd_device_register()` 后续最核心的事情可以概括为三条：

1. `phram` 被正式注册为一个 master MTD 设备 `mtdX`
2. MTD 核心为它创建设备模型、字符节点、`/proc/mtd` 可见项
3. 通过 notifier 机制自动触发 `mtdblock` 等上层，进一步生成 `/dev/mtdblockX`

所以从系统视角看：

> `phram` 并不只是“驱动内部多了个 `mtd_info`”，而是完整接入了整个 MTD 栈。

---

## 16. 补充：`mtd_device_parse_register()` 详细分析

前面已经提到，`phram` 实际调用的并不是一个独立实现的
`mtd_device_register()`，而是：

```c
mtd_device_parse_register(mtd, NULL, NULL, NULL, 0)
```

因此，如果想真正理解 `phram` 的注册路径，必须把
`mtd_device_parse_register()` 也看清楚。

### 16.1 函数原型

```c
int mtd_device_parse_register(struct mtd_info *mtd,
			      const char * const *part_probe_types,
			      struct mtd_part_parser_data *parser_data,
			      const struct mtd_partition *defparts,
			      int defnr_parts);
```

参数含义：

- `mtd`
  要注册的 master MTD

- `part_probe_types`
  要尝试的分区解析器列表，例如：
  - `cmdlinepart`
  - `ofpart`
  - `RedBoot`

- `parser_data`
  传给分区解析器的私有数据

- `defparts`
  驱动提供的静态分区表

- `defnr_parts`
  静态分区表项数量

### 16.2 它的设计目标

这个函数的定位不是“简单注册一个 MTD 设备”，而是：

> 用统一策略完成“分区解析 + 设备注册 + 分区注册 + 清理回滚”。

它把大量 MTD 驱动里都重复出现的逻辑统一收口了。

### 16.3 核心流程

可以概括成下面这几步：

1. `mtd_set_dev_defaults(mtd)`
2. `mtd_otp_nvmem_add(mtd)`
3. 如果启用 `CONFIG_MTD_PARTITIONED_MASTER`，先注册整个 master
4. 调 `parse_mtd_partitions(mtd, types, parser_data)`
5. 如果找到分区，则注册分区
6. 否则回退到驱动静态分区
7. 若静态分区也没有，则注册整个 MTD
8. 注册 reboot notifier
9. 失败时统一回滚

### 16.4 在 `phram` 里的具体表现

`phram` 调用：

```c
mtd_device_register(&new->mtd, NULL, 0)
```

展开后等价于：

```c
mtd_device_parse_register(&new->mtd, NULL, NULL, NULL, 0)
```

此时：

- 没有 parser
- 没有 parser data
- 没有驱动静态分区

所以对于 `phram` 而言，最常见的实际结果是：

1. `parse_mtd_partitions()` 返回 0
2. 没有 fallback 分区表
3. 走 `add_mtd_device(mtd)`
4. 整片 `phram` RAM 区域注册成一个 master MTD

也就是说：

> `phram` 的默认语义不是“带分区的 flash”，而是“整片 RAM 作为一个完整的 MTD 设备”。

### 16.5 为什么很多驱动都用它

因为很多 MTD 驱动并不知道分区信息到底来自哪：

- 设备树
- 启动参数 `mtdparts=`
- RedBoot
- 板级静态分区表
- 或根本没有分区

所以驱动最通用的写法就是：

1. 先把 `mtd_info` 填好
2. 调 `mtd_device_parse_register()`
3. 把“分区是否存在、如何解析、如何 fallback”交给 MTD 核心

### 16.6 和 `phram` 的实验联系

如果你只是想做 `phram + jffs2` 实验，通常并不需要分区。

这时最常见的结果是：

- `/proc/mtd` 中出现 `mtd0`
- 直接挂载：

```bash
mount -t jffs2 mtd:ph0 /mnt
```

但如果你后续想把 `phram` 模拟成“一个带多个分区的 flash”，那么就会涉及：

- `mtdparts=`
- `cmdlinepart`
- `parse_cmdline_partitions()`

下面继续分析这个 parser。

---

## 17. 补充：`parse_cmdline_partitions()` 详细分析

`parse_cmdline_partitions()` 是 MTD 的 `cmdlinepart` 分区解析器核心函数。

它的作用可以概括为：

> 把启动参数里的 `mtdparts=` 字符串解析成某个 `master MTD` 对应的 `struct mtd_partition[]`。

它本身不直接注册设备，而是把解析结果返回给 MTD parser 框架。

### 17.1 它在调用链里的位置

完整关系是：

1. 驱动调用 `mtd_device_parse_register()`
2. 里面调用 `parse_mtd_partitions()`
3. `parse_mtd_partitions()` 依次尝试 parser
4. 当 parser 名称为 `"cmdlinepart"` 时
5. 最终调用 `parse_cmdline_partitions()`

所以：

- `parse_cmdline_partitions()` 是具体 parser
- `parse_mtd_partitions()` 是 parser 调度器

### 17.2 启动参数语法

源码注释给出的格式是：

```text
mtdparts=<mtddef>[;<mtddef]
<mtddef>  := <mtd-id>:<partdef>[,<partdef>]
<partdef> := <size>[@<offset>][<name>][ro][lk][slc]
```

字段含义：

- `<mtd-id>`
  设备名字，对应 `mtd->name`

- `<size>`
  分区大小，例如 `256k`、`4M`

- `@<offset>`
  显式偏移

- `<name>`
  分区名字，格式 `(name)`

- `ro`
  只读

- `lk`
  上电保持锁定

- `slc`
  使用 SLC 模拟标志

例如：

```text
mtdparts=ph0:256k(boot)ro,2M(kernel),-(rootfs)
```

### 17.3 两阶段设计

这个 parser 不是在最早期启动参数解析时就把所有内容完整建好。

它采用“两阶段”：

#### 第一阶段：启动早期只保存字符串

通过：

- `__setup("mtdparts=", mtdpart_setup);`

只把原始字符串记录到全局变量 `cmdline`

#### 第二阶段：真正需要时再解析

当某个 MTD 设备真的走到 `cmdlinepart` parser 时，
`parse_cmdline_partitions()` 才会调用：

- `mtdpart_setup_real(cmdline)`

做真正的解析。

这样做的目的很简单：

- 启动早期不适合大量动态内存分配
- 所以先记字符串，后续再展开成结构体

### 17.4 `mtdpart_setup_real()` 做什么

这个函数负责把整条 `mtdparts=` 字符串拆成多个：

- `mtd_id -> partition array`

例如：

```text
mtdparts=ph0:256k(boot),-(root);ph1:1M(cfg),-(data)
```

会被拆成两组：

- `ph0`
- `ph1`

每组都保存为一条 `cmdline_mtd_partition`

### 17.5 `newpart()`：真正解析单个分区项

`newpart()` 是递归解析函数。

它会依次提取：

1. 大小
2. 偏移
3. 名字
4. 标志位
5. 是否还有下一个分区

#### 大小

- 普通大小：`memparse()`
- 特殊值 `-`：表示剩余所有空间

#### 偏移

- 若写了 `@offset`，则显式使用
- 若没写，则记成连续模式，后续再补

#### 名字

- `(boot)` 这种格式
- 如果没写，会自动生成：
  - `Partition_000`
  - `Partition_001`

#### 标志

- `ro` -> mask 掉 `MTD_WRITEABLE`
- `lk` -> 加 `MTD_POWERUP_LOCK`
- `slc` -> 加 `MTD_SLC_ON_MLC_EMULATION`

#### 递归约束

若某个分区用了 `-` 吃掉剩余空间，那么后面不能再跟分区。

### 17.6 `parse_cmdline_partitions()` 自己的主要工作

它拿到已经解析好的全局链表后，主要做 4 件事：

1. 找到和 `master->name` 匹配的那组 `mtd-id`
2. 给没显式写 offset 的分区补连续偏移
3. 把 `-` 转成“剩余空间”
4. 如果越界则截断，如果最终变成 0 长度则跳过

这里一个关键点是：

> `mtd-id` 必须和 `master->name` 匹配，否则这个 parser 对该设备就返回 0。

这也是很多 `mtdparts=` 不生效的根本原因：

- 启动参数写的设备名
- 和驱动里 `mtd->name`
- 根本不是同一个字符串

### 17.7 越界和零长度行为

它不会因为某个分区越界就直接整体失败，而是：

- 先警告
- 再截断到设备边界

如果截断后大小变成 0，则：

- 跳过该分区

这说明 `cmdlinepart` 的风格比较“尽量容忍”。

### 17.8 返回值语义

`parse_cmdline_partitions()` 返回：

- `> 0`：解析到多少个分区
- `0`：当前这个 MTD 没有匹配的 `mtd-id`
- `< 0`：语法或内存错误

后续 `parse_mtd_partitions()` 会根据这个结果决定是否调用：

- `add_mtd_partitions()`

### 17.9 和 `phram` 的关系

默认情况下，`phram` 自己并不会自动带分区。

如果你只是：

```text
phram.phram=ph0,0x4f800000,8Mi,64Ki
```

通常只会得到：

- 一个完整的 `mtd ph0`

但如果你再配合：

```text
mtdparts=ph0:256k(boot),2M(kernel),-(rootfs)
```

并且你的驱动路径/配置允许 `cmdlinepart` parser 被尝试，
那么最终就可能看到：

- `mtd0` 整片 master
- 或多个分区 `mtd0` / `mtd1` / `mtd2`

具体是否保留 master，还取决于：

- `CONFIG_MTD_PARTITIONED_MASTER`

### 17.10 实验建议

如果你想专门验证 `cmdlinepart`，推荐这样做：

1. 先让 `phram` 正常出来：

```text
phram.phram=ph0,0x4f800000,8Mi,64Ki
```

2. 再增加：

```text
mtdparts=ph0:256k(boot)ro,2M(kernel),-(rootfs)
```

3. 启动后观察：

- `/proc/mtd`
- `dmesg | grep -i mtd`

4. 对照看：

- 是整片设备
- 还是已经拆分成多个分区

这样最容易理解：

- `phram` 负责“提供 master MTD”
- `cmdlinepart` 负责“把 master 再切成 partitions”

---

## 18. 把三者串起来理解

如果把整条链路串起来，可以记成这样：

```text
phram
  -> register_device()
  -> mtd_device_register()
  -> mtd_device_parse_register()
       -> parse_mtd_partitions()
            -> cmdlinepart / ofpart / RedBoot ...
       -> 如果有分区，则 add_mtd_partitions()
       -> 如果没分区，则 add_mtd_device()
```

所以三者职责很清楚：

- `phram`
  负责把 RAM 变成一个 master MTD

- `mtd_device_parse_register()`
  负责“按策略决定注册 master 还是注册 partitions”

- `parse_cmdline_partitions()`
  负责解释 `mtdparts=` 文本，把它转换成 `struct mtd_partition[]`

---

## 19. 最终总结

如果只看 `phram`，容易以为它只是“注册一个 MTD 设备”。

但把后续链路补全后，可以更完整地理解为：

1. `phram` 提供底层 master MTD
2. `mtd_device_parse_register()` 负责统一的注册策略
3. `parse_cmdline_partitions()` 提供命令行分区能力
4. `add_mtd_device()` 把设备接入整个 MTD 栈
5. notifier 机制又把这个新设备传播给 `mtdblock` 等翻译层

所以：

> `phram` 不只是一个“RAM 模拟 flash”的小驱动，它其实是进入整个 MTD 子系统学习路径的一个非常好的入口。

---

## 20. 实战补充：`phram + mtdparts + jffs2`

前面的内容主要是代码路径分析，这一节给一个更贴近实际验证的例子，把：

- `phram`
- `mtdparts=`
- `jffs2`

三者串起来。

目标是验证下面这条链：

```text
bootargs
  -> phram 创建设备
  -> cmdlinepart 解析分区
  -> /proc/mtd 出现分区
  -> jffs2 挂载到指定 MTD 分区
```

### 20.1 适用场景

这个实验适合：

- QEMU ARM / ARM64
- 内核把 `phram`、`MTD`、`JFFS2` 编进内核
- 用 initramfs 做最小用户空间

如果只是本机分析代码，也可以不跑 QEMU，但建议至少跑一遍，这样最容易把现象和代码对上。

---

## 21. 推荐内核配置

至少建议打开：

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

如果你希望“整片设备”和“分区设备”同时可见，还可以打开：

- `CONFIG_MTD_PARTITIONED_MASTER=y`

这个选项的影响是：

- 关闭时：通常只看见分区
- 打开时：master MTD 和 partitions 都保留

做实验时这个选项很有助于观察 MTD 核心行为差异。

---

## 22. 一个推荐的 QEMU 参数设计

假设你用：

- `-m 256M`

并通过：

- `mem=248M`

让 Linux 只使用前 248 MiB，那么最后 8 MiB RAM 可以留给 `phram`。

例如：

```text
phram.phram=ph0,0x4f800000,8Mi,64Ki
```

再配合：

```text
mtdparts=ph0:256k(boot)ro,2M(kernel),-(rootfs)
```

完整 bootargs 形态可以是：

```text
console=ttyAMA0 rdinit=/init mem=248M \
phram.phram=ph0,0x4f800000,8Mi,64Ki \
mtdparts=ph0:256k(boot)ro,2M(kernel),-(rootfs)
```

这个参数组合表达的意思是：

1. 在物理地址 `0x4f800000` 开始，取 8 MiB RAM 作为 `ph0`
2. `ph0` 的擦除块大小是 `64KiB`
3. 然后用 `cmdlinepart` 再把 `ph0` 切成三段：
   - `boot`
   - `kernel`
   - `rootfs`

---

## 23. 预期的 `/proc/mtd`

### 情况 A：未启用 `CONFIG_MTD_PARTITIONED_MASTER`

通常更可能看到的是：

```text
dev:    size   erasesize  name
mtd0: 00040000 00010000 "boot"
mtd1: 00200000 00010000 "kernel"
mtd2: 005c0000 00010000 "rootfs"
```

也就是：

- 只看到分区
- 看不到完整的 `ph0`

### 情况 B：启用 `CONFIG_MTD_PARTITIONED_MASTER`

则更可能看到：

```text
dev:    size   erasesize  name
mtd0: 00800000 00010000 "ph0"
mtd1: 00040000 00010000 "boot"
mtd2: 00200000 00010000 "kernel"
mtd3: 005c0000 00010000 "rootfs"
```

也就是：

- `ph0` master 还保留着
- 分区也同时存在

这个现象正好能帮助理解：

- `mtd_device_parse_register()`
- `add_mtd_partitions()`
- `CONFIG_MTD_PARTITIONED_MASTER`

三者之间的关系。

---

## 24. 最小验证脚本

在 initramfs 的 `/init` 中，可以先放这样一个脚本：

```sh
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev

echo "=== filesystems ==="
cat /proc/filesystems

echo "=== mtd table ==="
cat /proc/mtd

echo "=== device nodes ==="
ls /dev/mtd* /dev/mtdblock* 2>/dev/null

mkdir -p /mnt

mount -t jffs2 /dev/mtdblock2 /mnt 2>/dev/null || \
mount -t jffs2 mtd:rootfs /mnt 2>/dev/null || \
mount -t jffs2 /dev/mtd2 /mnt 2>/dev/null || \
echo "mount failed"

echo hello > /mnt/hello.txt 2>/dev/null
sync
ls -l /mnt 2>/dev/null

exec sh
```

注意：

- 对 JFFS2 来说，更推荐直接使用 MTD 设备语义，而不是 block 语义
- 所以更推荐：

```bash
mount -t jffs2 mtd:rootfs /mnt
```

或：

```bash
mount -t jffs2 mtd2 /mnt
```

不同用户空间工具支持格式略有不同，可以按实际环境调整。

---

## 25. 更推荐的挂载方式

如果系统里 `rootfs` 是第三个分区，推荐优先尝试：

```bash
mount -t jffs2 mtd:rootfs /mnt
```

原因是：

- 语义更清晰
- 直接走 MTD 文件系统路径
- 不依赖 `mtdblock`

而不是：

```bash
mount -t jffs2 /dev/mtdblock2 /mnt
```

后者更多是历史兼容路径，不是学习 MTD 文件系统最理想的方式。

---

## 26. 你应该观察哪些现象

这个实验里最值得观察的是：

### 26.1 `phram` 是否成功出来

看：

```bash
dmesg | grep -i phram
```

预期应能看到类似：

- 创建设备名
- 起始地址
- 长度
- erasesize

### 26.2 `cmdlinepart` 是否生效

看：

```bash
cat /proc/mtd
```

如果 `mtdparts=` 生效，你看到的就不只是单个 `ph0`，而是按分区切出来的设备。

### 26.3 `mtdblock` 是否被 notifier 自动创建

看：

```bash
ls /dev/mtdblock*
```

如果内核启用了 `CONFIG_MTD_BLOCK`，则说明：

- `add_mtd_device()`
- `mtd_notifiers`
- `blktrans_notify_add()`
- `mtdblock_add_mtd()`

这一整套路径都生效了。

### 26.4 `jffs2` 是否真正工作

成功挂载后，可以做：

```bash
echo hello > /mnt/hello.txt
sync
cat /mnt/hello.txt
umount /mnt
mount -t jffs2 mtd:rootfs /mnt
cat /mnt/hello.txt
```

如果同一次运行中卸载重挂载后文件还在，就说明：

- MTD 设备正常
- JFFS2 正常
- 分区与挂载链路都是通的

如果整机重启后数据消失，也很正常，因为底层仍然只是 RAM。

---

## 27. 这个实验能帮助你验证哪些代码点

### 验证 `phram`

对应：

- `phram_setup()`
- `register_device()`
- `phram_map()`

### 验证 MTD 核心

对应：

- `mtd_device_parse_register()`
- `parse_mtd_partitions()`
- `add_mtd_device()`
- `add_mtd_partitions()`

### 验证 `cmdlinepart`

对应：

- `parse_cmdline_partitions()`
- `mtdpart_setup_real()`
- `newpart()`

### 验证 block translation 层

对应：

- `blktrans_notify_add()`
- `mtdblock_add_mtd()`

### 验证文件系统层

对应：

- `jffs2_get_tree()`
- `get_tree_mtd()`
- `jffs2_fill_super()`

所以这其实是一个很完整的“小型全链路实验”。

---

## 28. 常见问题

### 28.1 `mtdparts=` 不生效

最常见原因：

1. 没开 `CONFIG_MTD_CMDLINE_PARTS`
2. 驱动路径里没有尝试 `"cmdlinepart"` parser
3. `mtd-id` 和 `mtd->name` 不匹配

对 `phram` 来说，最常见就是第三种：

- 你写的是 `mtdparts=flash0:...`
- 实际 `phram` 名字是 `ph0`

那当然不会匹配。

### 28.2 只有 `/dev/mtd0`，没有 `/dev/mtdblock0`

一般是因为：

- 没开 `CONFIG_MTD_BLOCK`

### 28.3 能看到分区，但 `mount -t jffs2` 失败

常见原因：

1. 目标分区还不是合法 JFFS2 内容
2. 擦除块大小和实际期望不匹配
3. 挂载目标用错了设备名

### 28.4 master 和分区为什么有时同时出现，有时不同时出现

因为这取决于：

- `CONFIG_MTD_PARTITIONED_MASTER`

---

## 29. 最推荐的一组学习顺序

如果你打算一边实验一边读代码，建议用下面顺序：

1. 先读 `drivers/mtd/devices/phram.c`
2. 再读 `drivers/mtd/mtdcore.c`
3. 再读 `drivers/mtd/mtdpart.c`
4. 再读 `drivers/mtd/parsers/cmdlinepart.c`
5. 再读 `drivers/mtd/mtd_blkdevs.c`
6. 最后读 `fs/jffs2/super.c` 与 `drivers/mtd/mtdsuper.c`

这样最符合实验观察顺序。

---

## 30. 一句话收尾

`phram + mtdparts + jffs2` 这个实验最大的价值在于：

> 它把“底层设备创建”、“分区解析”、“MTD 核心注册”、“block translation 通知”、“文件系统挂载”五层逻辑串成了一条你可以亲手验证的完整链路。
