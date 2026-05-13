# `phram -> MTD -> JFFS2 -> VFS -> inode/file` 流程解析

## 1. 总体结论

`phram` 不是文件系统驱动，而是一个 **MTD RAM 驱动**。  
它把一段物理内存映射成 `struct mtd_info`，向上提供 `erase/read/write` 等 MTD 操作。

真正的文件系统路径是：

```text
用户态系统调用
  -> VFS
  -> JFFS2
  -> MTD core
  -> phram
  -> 物理内存
```

如果按模块来理解，可以写成：

```text
phram
  -> 注册为 struct mtd_info
  -> MTD core
  -> JFFS2
  -> VFS
  -> inode / file / dentry
  -> open/read/write/mount
```

## 2. `phram` 层在做什么

`phram` 的核心工作是：

1. 把一段物理地址通过 `memremap()` 或 `ioremap()` 映射进内核
2. 构造 `struct mtd_info`
3. 注册为一个 `MTD_RAM` 设备

关键字段如下：

```c
new->mtd.name = name;
new->mtd.size = len;
new->mtd.flags = MTD_CAP_RAM;
new->mtd._erase = phram_erase;
new->mtd._point = phram_point;
new->mtd._unpoint = phram_unpoint;
new->mtd._read = phram_read;
new->mtd._write = phram_write;
new->mtd.type = MTD_RAM;
new->mtd.erasesize = erasesize;
new->mtd.writesize = 1;
```

它的底层行为非常直接：

- `phram_erase()`：`memset(..., 0xff, len)`
- `phram_read()`：`memcpy()`
- `phram_write()`：`memcpy()`

也就是说，`phram` 本质上是在模拟一块具有 MTD 接口语义的 RAM 设备。

## 3. 整体调用关系图

### 3.1 `mount`

```text
mount -t jffs2 mtd:<name> /mnt
  -> jffs2_get_tree()
  -> get_tree_mtd()
  -> get_mtd_device_nm() / get_mtd_device()
  -> mtd_get_sb()
  -> jffs2_fill_super()
  -> 建立 super_block
  -> 挂载完成
```

### 3.2 `open`

```text
open("/mnt/file")
  -> path_openat()
  -> link_path_walk()
  -> jffs2_lookup()
  -> jffs2_iget()
  -> d_splice_alias()
  -> do_open()
  -> generic_file_open()
```

### 3.3 `read`

```text
read(fd, ...)
  -> vfs_read()
  -> new_sync_read()
  -> generic_file_read_iter()
  -> page cache miss
  -> jffs2_read_folio()
  -> jffs2_read_inode_range()
  -> jffs2_read_dnode()
  -> jffs2_flash_read()
  -> mtd_read()
  -> phram_read()
```

### 3.4 `write`

```text
write(fd, ...)
  -> vfs_write()
  -> new_sync_write()
  -> generic_file_write_iter()
  -> jffs2_write_begin()
  -> jffs2_write_end()
  -> jffs2_write_inode_range()
  -> jffs2_write_dnode()
  -> jffs2_flash_writev() / mtd_write()
  -> phram_write()
```

## 4. `mount` 路径详细说明

### 4.1 JFFS2 的挂载入口

JFFS2 的挂载入口是 `jffs2_get_tree()`：

```c
static int jffs2_get_tree(struct fs_context *fc)
{
    return get_tree_mtd(fc, jffs2_fill_super);
}
```

这说明 JFFS2 不是挂块设备，而是直接挂 MTD 设备。

### 4.2 `get_tree_mtd()` 的作用

`get_tree_mtd()` 做两件事：

1. 解析挂载源，比如 `mtd:rootfs` 或 `mtd0`
2. 找到对应的 `struct mtd_info`

逻辑可以理解为：

```text
fc->source = "mtd:rootfs"
  -> get_mtd_device_nm("rootfs")
  -> 拿到 phram 注册出的 mtd_info
  -> mtd_get_sb()
```

### 4.3 `mtd_get_sb()` 的作用

`mtd_get_sb()` 会把 `mtd_info` 绑定到 `super_block`：

```text
super_block
  -> s_mtd = mtd
  -> 调用 fill_super()
  -> 建立根目录 dentry 和 inode
```

之后，这个文件系统就正式进入 VFS 管理。

### 4.4 `jffs2_fill_super()` 的作用

`jffs2_fill_super()` 会初始化：

- `sb->s_op`
- `sb->s_export_op`
- `sb->s_root`
- `jffs2_sb_info`

也就是建立一个完整的 JFFS2 超级块实例。

## 5. `open` 路径详细说明

### 5.1 VFS 入口

打开文件的主入口是：

```text
do_file_open()
  -> path_openat()
```

`path_openat()` 会完成：

1. 路径遍历
2. 最后一级目录项查找
3. 调用具体文件系统的打开逻辑

### 5.2 JFFS2 的目录查找

JFFS2 在目录 inode 上挂了自己的 `inode_operations`：

```text
jffs2_dir_inode_operations
  -> .lookup = jffs2_lookup
  -> .create = jffs2_create
  -> .mkdir  = jffs2_mkdir
  -> .unlink = jffs2_unlink
  ...
```

当 VFS 解析路径走到目录时，会调用 `jffs2_lookup()`。

### 5.3 `jffs2_lookup()` 做了什么

`jffs2_lookup()` 会：

1. 在目录的 dirent 链表中查找目标名字
2. 找到 inode 号
3. 调用 `jffs2_iget()` 取回 inode
4. 返回 dentry 结果

即：

```text
文件名
  -> 目录项(dirent)
  -> inode number
  -> inode
```

### 5.4 打开后绑定的文件操作表

普通文件 inode 会绑定：

```text
inode->i_fop = &jffs2_file_operations
inode->i_mapping->a_ops = &jffs2_file_address_operations
```

因此后续的 `read/write` 会继续进入 JFFS2。

## 6. `read` 路径详细说明

### 6.1 VFS 到 `read_iter`

用户态执行 `read()` 后，进入：

```text
vfs_read()
  -> new_sync_read()
  -> file->f_op->read_iter()
```

对 JFFS2 普通文件来说：

```text
file->f_op->read_iter = generic_file_read_iter
```

也就是说，JFFS2 复用了通用页缓存读逻辑。

### 6.2 页缓存缺页时进入 `jffs2_read_folio()`

当页缓存里没有对应数据时，VFS 会通过地址空间操作表回调：

```text
address_space_operations
  -> .read_folio = jffs2_read_folio
```

`jffs2_read_folio()` 继续调用：

```text
__jffs2_read_folio()
  -> jffs2_do_readpage_nolock()
  -> jffs2_read_inode_range()
```

### 6.3 `jffs2_read_inode_range()` 的作用

`jffs2_read_inode_range()` 会根据 JFFS2 的 fragment 树，找到文件某个逻辑区间对应的物理节点，然后逐段读取。

也就是：

```text
逻辑文件偏移
  -> frag tree
  -> 对应 dnode
  -> 逐节点读取
```

### 6.4 `jffs2_read_dnode()` 的作用

`jffs2_read_dnode()` 会：

1. 先读出 raw inode/node header
2. 校验 CRC
3. 读出数据区
4. 如果压缩过则解压
5. 拷贝到页缓存

### 6.5 落到 MTD 和 phram

JFFS2 读 flash 的统一入口是：

```text
jffs2_flash_read()
  -> mtd_read(c->mtd, ...)
```

而对 `phram` 来说：

```text
mtd_read()
  -> phram_read()
  -> memcpy()
```

所以 `read()` 的最后一跳已经进入 `phram` 驱动。

## 7. `write` 路径详细说明

### 7.1 VFS 到 `write_iter`

用户态执行 `write()` 后：

```text
vfs_write()
  -> new_sync_write()
  -> file->f_op->write_iter()
```

对 JFFS2 普通文件来说：

```text
file->f_op->write_iter = generic_file_write_iter
```

也就是说，写路径同样先走通用页缓存。

### 7.2 `jffs2_write_begin()`

写页缓存前，VFS 会通过地址空间操作表调用：

```text
.write_begin = jffs2_write_begin
```

它主要负责：

1. 找到或分配 folio
2. 必要时先把原有内容读入 folio
3. 处理文件扩展和 hole
4. 为真正写入做准备

### 7.3 `jffs2_write_end()`

`.write_end = jffs2_write_end` 是真正提交写入的关键点。

它会：

1. 根据当前写入范围构造 `jffs2_raw_inode`
2. 从 folio 中取出数据
3. 调用 `jffs2_write_inode_range()`

### 7.4 `jffs2_write_inode_range()` 和 `jffs2_write_dnode()`

这一步是 JFFS2 文件数据写入 flash 介质的核心：

```text
jffs2_write_inode_range()
  -> jffs2_write_dnode()
  -> jffs2_flash_writev()
```

`jffs2_write_dnode()` 会：

1. 准备 node header
2. 拼装 `kvec`
3. 调用底层写接口
4. 建立新的物理节点引用
5. 更新 inode 的节点/fragment 关系

这也体现了 JFFS2 的日志式写入思想：

- 不是原地覆盖旧数据
- 而是写入新 node
- 再更新元数据指向

### 7.5 落到 MTD 和 phram

JFFS2 最终会调用：

```text
jffs2_flash_direct_writev()
  -> mtd_writev()
```

或者：

```text
jffs2_flash_direct_write()
  -> mtd_write()
```

对 `phram` 来说，最终就是：

```text
mtd_write()
  -> phram_write()
  -> memcpy()
```

所以 `write()` 的最后一跳同样进入 `phram`。

## 8. 哪一步开始从“文件系统”进入“设备驱动”

这是理解整个层次最关键的一条分界线。

### 8.1 文件系统层

以下函数仍然属于 JFFS2 或 VFS 语义：

- `jffs2_get_tree()`
- `jffs2_fill_super()`
- `jffs2_lookup()`
- `jffs2_read_folio()`
- `jffs2_read_inode_range()`
- `jffs2_write_begin()`
- `jffs2_write_end()`
- `jffs2_write_inode_range()`
- `jffs2_write_dnode()`

### 8.2 MTD 设备层

从这里开始进入设备抽象层：

- `mtd_read()`
- `mtd_write()`
- `mtd_erase()`

### 8.3 phram 驱动层

对 `phram` 设备而言，最终落点是：

- `phram_read()`
- `phram_write()`
- `phram_erase()`

因此可以简单记成：

```text
JFFS2 的最后一跳 = mtd_read/mtd_write
phram 的第一跳   = phram_read/phram_write
```

## 9. 和普通块文件系统的区别

如果是 `ext4/xfs` 这类普通文件系统，路径通常是：

```text
VFS
  -> ext4/xfs
  -> block layer
  -> block device driver
```

而这里是：

```text
VFS
  -> JFFS2
  -> MTD
  -> phram
```

区别在于：

- 块文件系统假设底层是块设备
- JFFS2 假设底层是 flash/MTD 设备
- `phram` 虽然底层是 RAM，但对上层暴露的是 MTD 语义

所以它很适合拿来学习 Linux 存储抽象分层。

## 10. 最终总结

整个 `phram -> MTD -> JFFS2 -> VFS -> inode/file` 流程可以压缩为一句话：

> `phram` 把一段 RAM 伪装成 MTD 设备，JFFS2 把这个 MTD 设备组织成文件系统，VFS 再把这个文件系统统一包装成 Linux 通用的 inode/file/dentry 接口，最终为用户提供 `mount/open/read/write` 能力。

从调用路径上看：

- `mount`：建立 `super_block`
- `open`：通过目录查找拿到 inode 和 file
- `read`：页缓存缺页后下钻到 `mtd_read -> phram_read`
- `write`：日志式写 node，最终下钻到 `mtd_write -> phram_write`

因此，`phram` 最适合用来理解 Linux 文件系统和底层介质之间是如何通过抽象层解耦的。
