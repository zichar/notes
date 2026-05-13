#console




[    0.906618] =============[489][drivers/tty/serial/amba-pl011.c]
[    0.906759] sbsa-uart ARMH0011:02: no DMA platform data
[    0.906881] CPU: 0 PID: 1 Comm: swapper/0 Not tainted 6.6.89-cix-build-generic #23
[    0.907046] Hardware name:  , BIOS
[    0.907127] Call trace:
[    0.907184]  dump_backtrace+0x98/0x118
[    0.907295]  show_stack+0x18/0x24
[    0.907374]  dump_stack_lvl+0x48/0x60
[    0.907469]  dump_stack+0x18/0x24
[    0.907544]  pl011_dma_probe+0x420/0x4bc
[    0.907643]  sbsa_uart_startup+0x16c/0x178
[    0.907737]  uart_startup+0x130/0x2f8
[    0.907822]  uart_port_activate+0x34/0x9c
[    0.907911]  tty_port_open+0x8c/0x13c
[    0.907998]  uart_open+0x1c/0x30
[    0.908069]  tty_open+0x130/0x6f4
[    0.908142]  chrdev_open+0xc0/0x20c
[    0.908230]  do_dentry_open+0x1b8/0x534
[    0.908314]  vfs_open+0x2c/0x38
[    0.908383]  path_openat+0xb3c/0xf04
[    0.908464]  do_filp_open+0x9c/0x14c
[    0.908544]  filp_open+0x110/0x1b0
[    0.908618]  console_on_rootfs+0x20/0x74
[    0.908711]  kernel_init_freeable+0x21c/0x3c4
[    0.908805]  kernel_init+0x5c/0x2a4
[    0.908887]  ret_from_fork+0x10/0x20
[    0.909023] =============[492][drivers/tty/serial/amba-pl011.c]
























