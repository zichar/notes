# Linux VFS：从文件系统注册到挂载的代码流程

本文整理 Linux 内核中文件系统从“注册”到“挂载生效”的主流程，并结合 `phram + jffs2` 给出一个适合 ARM QEMU 的实验思路。

---

## 1. 先分清两层：设备和文件系统

很多人第一次看文件系统代码时，容易把底层设备驱动和文件系统本身混在一起。

这里要先分两层：

1. `phram` 是 **MTD 设备驱动**
   它的作用是把一段物理 RAM 映射成一个 MTD 设备。

2. `jffs2`、`romfs`、`cramfs` 这类是 **文件系统**
   它们通过 VFS 注册，并挂载到某个底层设备上。

所以 `phram + jffs2` 的实际链路是：

`phram` 注册 MTD 设备 -> `jffs2` 注册到 VFS -> `mount -t jffs2 mtd:ph0 /mnt`

---

## 2. VFS 中最核心的对象

理解注册和挂载流程前，建议先记住这几个核心对象：

- `struct file_system_type`
  表示一种文件系统类型，例如 `ext2`、`ramfs`、`jffs2`

- `struct super_block`
  表示一个已经挂载起来的文件系统实例

- `struct dentry`
  目录项，连接“名字”和“inode”

- `struct inode`
  文件元数据对象

- `struct file`
  一个进程打开文件后的内核对象

在“注册”阶段最重要的是 `file_system_type`。
在“挂载”阶段最重要的是 `super_block` 和 `dentry`。

---

## 3. 文件系统注册：`register_filesystem()`

VFS 用一个全局链表维护当前已知的文件系统类型。

文件位置：

- `fs/filesystems.c`

核心逻辑：

- 检查 `fs->name`
- 检查参数描述是否合法
- 检查是否重复注册
- 把 `struct file_system_type` 挂到全局链表 `file_systems`

关键代码逻辑可以概括为：

```c
int register_filesystem(struct file_system_type *fs)
{
    if (fs->parameters && !fs_validate_description(...))
        return -EINVAL;

    if (fs->next)
        return -EBUSY;

    write_lock(&file_systems_lock);
    p = find_filesystem(fs->name, strlen(fs->name));
    if (*p)
        res = -EBUSY;
    else
        *p = fs;
    write_unlock(&file_systems_lock);

    return res;
}
```

### 3.1 `struct file_system_type` 的关键成员

定义位置：

- `include/linux/fs.h`

重点字段：

- `name`
  文件系统名字，比如 `"ext2"`、`"ramfs"`、`"jffs2"`

- `fs_flags`
  文件系统特性，比如是否要求块设备 `FS_REQUIRES_DEV`

- `init_fs_context`
  新挂载 API 的入口

- `mount`
  老挂载 API 的入口

- `kill_sb`
  卸载时释放 superblock 的方法

这意味着：

- 老式文件系统常实现 `.mount`
- 新式文件系统常实现 `.init_fs_context`

---

## 4. 注册示例 1：`ramfs`

`ramfs` 是很好的入门样板，因为代码很小，逻辑很清楚。

文件位置：

- `fs/ramfs/inode.c`

它的注册方式是：

```c
static struct file_system_type ramfs_fs_type = {
    .name            = "ramfs",
    .init_fs_context = ramfs_init_fs_context,
    .parameters      = ramfs_fs_parameters,
    .kill_sb         = ramfs_kill_sb,
    .fs_flags        = FS_USERNS_MOUNT,
};

static int __init init_ramfs_fs(void)
{
    return register_filesystem(&ramfs_fs_type);
}
fs_initcall(init_ramfs_fs);
```

说明：

- `ramfs` 走的是新挂载 API
- 它不依赖块设备
- 初始化时只是把 `ramfs_fs_type` 注册给 VFS

注册完后，你可以在 `/proc/filesystems` 中看到 `ramfs`

---

## 5. 注册示例 2：`ext2`

文件位置：

- `fs/ext2/super.c`

它还是经典写法：

```c
static struct file_system_type ext2_fs_type = {
    .owner      = THIS_MODULE,
    .name       = "ext2",
    .mount      = ext2_mount,
    .kill_sb    = kill_block_super,
    .fs_flags   = FS_REQUIRES_DEV,
};
```

这里可以看到：

- `ext2` 需要底层块设备，所以有 `FS_REQUIRES_DEV`
- 它实现的是 `.mount = ext2_mount`

初始化阶段：

```c
static int __init init_ext2_fs(void)
{
    int err;

    err = init_inodecache();
    if (err)
        return err;

    err = register_filesystem(&ext2_fs_type);
    if (err)
        goto out;

    return 0;
out:
    destroy_inodecache();
    return err;
}
```

这说明一个文件系统在注册前，往往还会先建立自己的缓存、slab、辅助结构。

---

## 6. 用户态执行 `mount` 后，VFS 先做什么

用户态执行：

```bash
mount -t ext2 /dev/vda /mnt
```

或：

```bash
mount -t jffs2 mtd:ph0 /mnt
```

内核主链路大致是：

1. `do_mount()`
2. `path_mount()`
3. `do_new_mount()`
4. `get_fs_type()`
5. `fs_context_for_mount()`
6. `vfs_get_tree()`
7. `do_new_mount_fc()`

主要文件：

- `fs/namespace.c`
- `fs/filesystems.c`
- `fs/super.c`

---

## 7. `get_fs_type()`：根据名字找到文件系统

文件位置：

- `fs/filesystems.c`

作用：

- 在 VFS 的全局文件系统链表里找指定名字
- 如果没找到，尝试自动加载模块 `request_module("fs-%s")`

逻辑简化如下：

```c
struct file_system_type *get_fs_type(const char *name)
{
    fs = __get_fs_type(name, len);
    if (!fs && request_module("fs-%.*s", len, name) == 0)
        fs = __get_fs_type(name, len);
    return fs;
}
```

这一步的意义非常关键：

- 用户传的是字符串，比如 `"ext2"`、`"jffs2"`
- VFS 把它转换成 `struct file_system_type *`

也就是从“名字”进入“类型对象”

---

## 8. `fs_context`：挂载过程的中间上下文

现代挂载框架里，VFS 不直接一把梭地调用具体文件系统，而是先构造一个 `struct fs_context`。

它的作用是：

- 保存 source
- 保存 mount 参数
- 保存 superblock 参数
- 保存文件系统私有挂载上下文
- 保存最终解析出来的 `fc->root`

这是一层很重要的解耦：

- 用户态 mount 参数先被 VFS 收集
- 再交给具体文件系统解析
- 最后由文件系统产出挂载根

---

## 9. `vfs_get_tree()`：让具体文件系统拿出可挂载的根

文件位置：

- `fs/super.c`

它的核心工作只有一句话：

> 调用该文件系统的 `get_tree()`，并要求它把挂载根放到 `fc->root`

逻辑可以理解为：

```c
int vfs_get_tree(struct fs_context *fc)
{
    if (fc->root)
        return -EBUSY;

    error = fc->ops->get_tree(fc);
    if (error < 0)
        return error;

    if (!fc->root)
        return -EINVAL;

    return 0;
}
```

也就是说：

- VFS 不关心你底下是块设备、匿名设备、MTD 还是网络文件系统
- 只要求你最终提供一个根 dentry

---

## 10. 三种常见挂载 helper

VFS 提供了三个非常常用的 helper：

- `mount_bdev()`
  基于块设备

- `mount_nodev()`
  不依赖设备

- `mount_single()`
  所有挂载共享同一个实例

新接口里对应常见的是：

- `get_tree_bdev()`
- `get_tree_nodev()`
- 其他特化 helper

---

## 11. `ramfs` 如何建立 superblock

`ramfs` 的核心在于：

```c
static int ramfs_get_tree(struct fs_context *fc)
{
    return get_tree_nodev(fc, ramfs_fill_super);
}
```

`ramfs_fill_super()` 是一个很标准的最小模型：

1. 填 `sb->s_magic`
2. 安装 `sb->s_op`
3. 创建根目录 inode
4. `d_make_root(inode)` 生成根 dentry
5. 放到 `sb->s_root`

简化理解：

```c
sb->s_magic = RAMFS_MAGIC;
sb->s_op    = &ramfs_ops;

inode = ramfs_get_inode(..., S_IFDIR | mode, ...);
sb->s_root = d_make_root(inode);
```

这段代码非常值得反复看，因为它展示了“文件系统挂载最小闭环”：

- superblock 建出来
- 根 inode 建出来
- 根 dentry 建出来
- 系统就有了一个可挂载树的起点

---

## 12. `ext2` 如何挂载块设备

`ext2` 是典型块设备文件系统。

挂载入口：

```c
static struct dentry *ext2_mount(struct file_system_type *fs_type,
    int flags, const char *dev_name, void *data)
{
    return mount_bdev(fs_type, flags, dev_name, data, ext2_fill_super);
}
```

`mount_bdev()` 主要做这些事：

1. `lookup_bdev(dev_name, &dev)`
   找到底层块设备

2. `sget(...)`
   查找已有 superblock，或者新建一个

3. 如果第一次挂载，则调用 `fill_super()`

4. `dget(s->s_root)`
   返回根 dentry

所以块设备文件系统的核心是：

- 底层设备由 VFS 帮你打开
- superblock 由 VFS 帮你管理生命周期
- 你只要把磁盘上的元数据解释出来并填到 `super_block`

---

## 13. 最后一步：挂到命名空间

就算文件系统已经通过 `get_tree()` 得到了 `fc->root`，它还没有真正出现在用户看到的目录树上。

真正让 mount 生效的是：

- `do_new_mount_fc()`
- `vfs_create_mount()`
- `do_add_mount()`

可以把它理解成：

1. 文件系统先提供一棵“可挂载树”
2. VFS 再把这棵树 graft 到目标挂载点

到这一步后，用户在 `/mnt` 才能真正看到这个文件系统

---

## 14. 结合 `phram + jffs2` 看完整路径

这个例子非常适合实验，因为它把“设备驱动”和“文件系统”两层都串起来了。

### 14.1 `phram` 做了什么

文件位置：

- `drivers/mtd/devices/phram.c`

它的作用是把某段物理 RAM 映射成一个 MTD 设备。

核心步骤：

1. `memremap()` 或 `ioremap()` 映射一段物理地址
2. 构造 `struct mtd_info`
3. 填 `_read`、`_write`、`_erase`
4. 调 `mtd_device_register()`

关键字段：

- `new->mtd.type = MTD_RAM`
- `new->mtd.flags = MTD_CAP_RAM`
- `new->mtd.erasesize = erasesize`

注册成功后：

- `/proc/mtd` 里会出现一个新的 MTD 设备

例如名字叫 `ph0`

### 14.2 `phram` 参数格式

若编进内核：

```text
phram.phram=<name>,<start>,<len>[,<erasesize>]
```

若编成模块：

```text
phram=<name>,<start>,<len>[,<erasesize>]
```

例如：

```text
phram.phram=ph0,0x4f800000,8Mi,64Ki
```

### 14.3 `jffs2` 做了什么

文件位置：

- `fs/jffs2/super.c`

`jffs2` 通过 VFS 注册：

```c
static struct file_system_type jffs2_fs_type = {
    .owner           = THIS_MODULE,
    .name            = "jffs2",
    .init_fs_context = jffs2_init_fs_context,
    .parameters      = jffs2_fs_parameters,
    .kill_sb         = jffs2_kill_sb,
};
```

挂载时走：

```c
static int jffs2_get_tree(struct fs_context *fc)
{
    return get_tree_mtd(fc, jffs2_fill_super);
}
```

这说明 `jffs2` 不是块设备文件系统，也不是 nodev，而是 **MTD-backed filesystem**

### 14.4 `get_tree_mtd()` 做了什么

文件位置：

- `drivers/mtd/mtdsuper.c`

它负责把 `source` 解析成一个 MTD 设备：

- `mtd:ph0`
- `mtd0`
- `/dev/mtdblock0`

解析到对应 MTD 后：

1. 找或建 superblock
2. 调 `jffs2_fill_super()`
3. 设置 `fc->root`

于是完整链路就是：

1. `phram` 把 RAM 注册为 MTD
2. `jffs2` 把自己注册到 VFS
3. `mount -t jffs2 mtd:ph0 /mnt`
4. VFS 找到 `jffs2_fs_type`
5. `jffs2_get_tree()`
6. `get_tree_mtd()`
7. `jffs2_fill_super()`
8. VFS 把根挂到 `/mnt`

---

## 15. 推荐实验：ARM QEMU 跑 `phram + jffs2`

下面给一个容易落地的实验思路。

### 15.1 实验目标

目标是验证：

1. `phram` 成功注册出 MTD 设备
2. `jffs2` 成功注册到 VFS
3. 可以把 `jffs2` 挂到 `phram` 提供的 MTD 上
4. 可以读写文件并重新挂载验证

### 15.2 建议内核配置

至少打开：

- `CONFIG_MTD=y`
- `CONFIG_MTD_PHRAM=y`
- `CONFIG_MTD_BLOCK=y`
- `CONFIG_JFFS2_FS=y`
- `CONFIG_DEVTMPFS=y`
- `CONFIG_DEVTMPFS_MOUNT=y`
- `CONFIG_PROC_FS=y`
- `CONFIG_SYSFS=y`
- `CONFIG_TMPFS=y`
- `CONFIG_SERIAL_AMBA_PL011=y`

建议：

- `phram` 和 `jffs2` 都直接编进内核
- 这样实验更简单，不依赖模块加载

### 15.3 预留一段 RAM 给 `phram`

假设 QEMU 使用：

- `-m 256M`

在 `arm virt` 机器上，RAM 常见从 `0x40000000` 开始。

可以让 Linux 只使用前 248 MiB：

```text
mem=248M
```

这样最后 8 MiB 保留下来，给 `phram`：

```text
phram.phram=ph0,0x4f800000,8Mi,64Ki
```

注意：

- `erasesize` 要和你准备用的 JFFS2 镜像设置一致
- `64Ki` 是常见选择

### 15.4 QEMU 启动命令示例

```bash
qemu-system-arm \
  -M virt \
  -cpu cortex-a15 \
  -m 256M \
  -nographic \
  -kernel arch/arm/boot/zImage \
  -initrd rootfs.cpio.gz \
  -append 'console=ttyAMA0 rdinit=/init mem=248M phram.phram=ph0,0x4f800000,8Mi,64Ki'
```

如果你跑的是 `arm64`，则改用：

- `qemu-system-aarch64`
- `Image`

### 15.5 最小 initramfs 脚本

`/init` 可以写成：

```sh
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev

echo "=== filesystems ==="
cat /proc/filesystems

echo "=== mtd devices ==="
cat /proc/mtd

mkdir -p /mnt

mount -t jffs2 mtd:ph0 /mnt || exec sh

echo hello > /mnt/hello.txt
sync
ls -l /mnt

umount /mnt
mount -t jffs2 mtd:ph0 /mnt || exec sh

echo "=== remount check ==="
cat /mnt/hello.txt

exec sh
```

### 15.6 预期现象

你应该观察到：

1. `/proc/filesystems` 中有 `jffs2`
2. `/proc/mtd` 中有 `ph0`
3. `mount -t jffs2 mtd:ph0 /mnt` 成功
4. 写入文件后，卸载再挂载，文件还在
5. 但整个 QEMU 重启后内容会丢失，因为底层仍是 RAM

---

## 16. 如果要做得更规范：用 DT 的 `reserved-memory`

如果你不想靠 `mem=` 粗暴预留内存，可以走设备树。

`phram` 已经有对应的绑定：

文件位置：

- `Documentation/devicetree/bindings/reserved-memory/phram.yaml`

示意：

```dts
reserved-memory {
    #address-cells = <1>;
    #size-cells = <1>;

    phram: flash@12340000 {
        compatible = "phram";
        label = "rootfs";
        reg = <0x12340000 0x00800000>;
    };
};
```

这样做的优点：

- 地址范围更明确
- 不会和普通内存管理混淆
- 更接近真实板级设计

---

## 17. 实验中建议重点观察的接口

建议每一步都用现象去对应代码。

### 17.1 看文件系统是否注册成功

```bash
cat /proc/filesystems
```

对应 VFS 层：

- `register_filesystem()`

### 17.2 看 MTD 设备是否注册成功

```bash
cat /proc/mtd
```

对应 MTD 驱动层：

- `mtd_device_register()`

### 17.3 挂载时看 dmesg

```bash
dmesg | grep -E 'phram|jffs2|MTD|VFS'
```

重点观察：

- `phram` 是否成功映射地址
- `jffs2` 是否识别底层设备
- 挂载失败时错误出在哪层

---

## 18. 推荐的源码阅读顺序

如果你准备继续深入，建议按这个顺序读：

1. `fs/filesystems.c`
   看文件系统注册、查找、自动加载

2. `fs/namespace.c`
   看 `do_mount()` 到 `do_new_mount()`

3. `fs/super.c`
   看 `vfs_get_tree()`、`mount_bdev()`、`mount_nodev()`

4. `fs/ramfs/inode.c`
   看最小文件系统模板

5. `drivers/mtd/devices/phram.c`
   看 MTD 设备如何注册

6. `drivers/mtd/mtdsuper.c`
   看 MTD 文件系统挂载桥接层

7. `fs/jffs2/super.c`
   看 MTD 文件系统如何接 VFS

---

## 19. 一句话总结

可以把整个流程记成一句话：

> 文件系统先把 `file_system_type` 注册给 VFS；挂载时 VFS 根据名字找到它，构造 `fs_context`，再让具体文件系统建立 `super_block` 和根 `dentry`，最后把这棵树挂到目标目录上。

而 `phram + jffs2` 的实验价值在于：

> 你能同时看到“设备注册”和“文件系统挂载”两层代码是如何衔接的。

---

## 20. 后续可扩展的实验

如果这个实验跑通了，下一步建议做这几个变体：

1. 把 `jffs2` 换成 `romfs` 或 `cramfs`
   对比只读文件系统与日志型闪存文件系统的差异

2. 把 `phram` 换成 `mtdram` 或 `nandsim`
   体验不同 MTD 模拟方式

3. 把实验切换到块设备文件系统
   如 `ext2/ext4`，对比 `mount_bdev()` 与 `get_tree_mtd()`

4. 给关键函数加 trace 或 printk
   例如：
   - `register_filesystem()`
   - `get_fs_type()`
   - `vfs_get_tree()`
   - `jffs2_get_tree()`
   - `get_tree_mtd()`

这样你会对挂载路径建立非常直观的感觉。
