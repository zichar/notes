[    0.000000] Booting Linux on physical CPU 0x0000000000 [0x410fd083]
[    0.000000] Linux version 6.6.6-g41c772dcde14-dirty (zichar@ZicharPC) (aarch64-none-linux-gnu-gcc (Arm GNU Toolchain 13.2.rel1 (Build arm-13.7)) 13.2.1 20231009, GNU ld (Arm GNU Toolchain 13.2.rel1 (Build arm-13.7)) 2.41.0.20231009) #96 SMP PREEMPT Wed Nov  6 17:06:39 CST 2024
[    0.000000] KASLR disabled on command line
[    0.000000] random: crng init done
[    0.000000] Machine model: linux,dummy-virt
[    0.000000] efi: UEFI not found.
[    0.000000] earlycon: pl11 at MMIO 0x0000000009000000 (options '')
[    0.000000] printk: bootconsole [pl11] enabled
[    0.000000] NUMA: No NUMA configuration found
[    0.000000] NUMA: Faking a node at [mem 0x0000000040000000-0x00000000bfffffff]
[    0.000000] NUMA: NODE_DATA [mem 0xbfbf29c0-0xbfbf4fff]
[    0.000000] Zone ranges:
[    0.000000]   DMA      [mem 0x0000000040000000-0x00000000bfffffff]
[    0.000000]   DMA32    empty
[    0.000000]   Normal   empty
[    0.000000] Movable zone start for each node
[    0.000000] Early memory node ranges
[    0.000000]   node   0: [mem 0x0000000040000000-0x00000000bfffffff]
[    0.000000] Initmem setup node 0 [mem 0x0000000040000000-0x00000000bfffffff]
[    0.000000] cma: Reserved 32 MiB at 0x00000000bba00000 on node -1
[    0.000000] psci: probing for conduit method from DT.
[    0.000000] psci: PSCIv1.1 detected in firmware.
[    0.000000] psci: Using standard PSCI v0.2 function IDs
[    0.000000] psci: Trusted OS migration not required
[    0.000000] psci: SMC Calling Convention v1.0
[    0.000000] percpu: Embedded 31 pages/cpu s88552 r8192 d30232 u126976
[    0.000000] pcpu-alloc: s88552 r8192 d30232 u126976 alloc=31*4096
[    0.000000] pcpu-alloc: [0] 0 [0] 1 
[    0.000000] Detected PIPT I-cache on CPU0
[    0.000000] CPU features: detected: GIC system register CPU interface
[    0.000000] CPU features: detected: Spectre-v2
[    0.000000] CPU features: detected: Spectre-v3a
[    0.000000] CPU features: detected: Spectre-v4
[    0.000000] CPU features: detected: Spectre-BHB
[    0.000000] CPU features: detected: ARM erratum 1742098
[    0.000000] CPU features: detected: ARM errata 1165522, 1319367, or 1530923
[    0.000000] alternatives: applying boot alternatives
[    0.000000] Kernel command line: rootwait root=/dev/nfs console=ttyAMA0 earlycon nfsroot=10.0.2.2:/home/zichar/qemu/rootfs/buildroot-2023.02.3/output/target,nfsvers=4 rw ip=dhcp nokaslr
[    0.000000] Dentry cache hash table entries: 262144 (order: 9, 2097152 bytes, linear)
[    0.000000] Inode-cache hash table entries: 131072 (order: 8, 1048576 bytes, linear)
[    0.000000] Fallback order for Node 0: 0 
[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 516096
[    0.000000] Policy zone: DMA
[    0.000000] mem auto-init: stack:all(zero), heap alloc:off, heap free:off
[    0.000000] software IO TLB: area num 2.
[    0.000000] software IO TLB: mapped [mem 0x00000000b7800000-0x00000000bb800000] (64MB)
[    0.000000] Memory: 1906836K/2097152K available (19200K kernel code, 5502K rwdata, 11668K rodata, 12736K init, 628K bss, 157548K reserved, 32768K cma-reserved)
[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=2, Nodes=1
[    0.000000] ftrace: allocating 65329 entries in 256 pages
[    0.000000] ftrace: allocated 256 pages with 1 groups
[    0.000000] trace event string verifier disabled
[    0.000000] rcu: Preemptible hierarchical RCU implementation.
[    0.000000] rcu: 	RCU event tracing is enabled.
[    0.000000] rcu: 	RCU restricting CPUs from NR_CPUS=256 to nr_cpu_ids=2.
[    0.000000] 	Trampoline variant of Tasks RCU enabled.
[    0.000000] 	Rude variant of Tasks RCU enabled.
[    0.000000] 	Tracing variant of Tasks RCU enabled.
[    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 25 jiffies.
[    0.000000] rcu: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=2
[    0.000000] NR_IRQS: 64, nr_irqs: 64, preallocated irqs: 0
[    0.000000] GICv3: 256 SPIs implemented
[    0.000000] GICv3: 0 Extended SPIs implemented
[    0.000000] Root IRQ handler: gic_handle_irq
[    0.000000] GICv3: GICv3 features: 16 PPIs
[    0.000000] GICv3: CPU0: found redistributor 0 region 0:0x00000000080a0000
[    0.000000] ITS [mem 0x08080000-0x0809ffff]
[    0.000000] ITS@0x0000000008080000: allocated 8192 Devices @43620000 (indirect, esz 8, psz 64K, shr 1)
[    0.000000] ITS@0x0000000008080000: allocated 8192 Interrupt Collections @43630000 (flat, esz 8, psz 64K, shr 1)
[    0.000000] GICv3: using LPI property table @0x0000000043640000
[    0.000000] GICv3: CPU0: using allocated LPI pending table @0x0000000043650000
[    0.000000] rcu: srcu_init: Setting srcu_struct sizes based on contention.
[    0.000000] ======[179][drivers/clocksource/timer-sp804.c] reload[375]
[    0.000000] ============[116][drivers/clocksource/timer-sp804.c] rate[0x0000000000016e36]
[    0.000000] clocksource: arm,sp804: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 20386778099061 ns
[    0.000000] sched_clock: 32 bits at 94kHz, resolution 10666ns, wraps every 22906492242730ns
[    0.000010] ========[328][drivers/clocksource/timer-sp804.c]
[    0.000096] arch_timer: cp15 timer(s) running at 62.50MHz (virt).
[    0.000096] clocksource: arch_sys_counter: mask: 0x1ffffffffffffff max_cycles: 0x1cd42e208c, max_idle_ns: 881590405314 ns
[    0.000117] sched_clock: 57 bits at 63MHz, resolution 16ns, wraps every 4398046511096ns
[    0.008087] Console: colour dummy device 80x25
[    0.010155] Calibrating delay loop (skipped), value calculated using timer frequency.. 125.00 BogoMIPS (lpj=250000)
[    0.010443] pid_max: default: 32768 minimum: 301
[    0.011562] LSM: initializing lsm=capability,integrity
[    0.013146] Mount-cache hash table entries: 4096 (order: 3, 32768 bytes, linear)
[    0.013275] Mountpoint-cache hash table entries: 4096 (order: 3, 32768 bytes, linear)
[    0.037373] cacheinfo: Unable to detect cache hierarchy for CPU 0
[    0.045227] RCU Tasks: Setting shift to 1 and lim to 1 rcu_task_cb_adjust=1.
[    0.045590] RCU Tasks Rude: Setting shift to 1 and lim to 1 rcu_task_cb_adjust=1.
[    0.046201] RCU Tasks Trace: Setting shift to 1 and lim to 1 rcu_task_cb_adjust=1.
[    0.047775] rcu: Hierarchical SRCU implementation.
[    0.047875] rcu: 	Max phase no-delay instances is 1000.
[    0.050757] Platform MSI: its@8080000 domain created
[    0.051796] PCI/MSI: /intc@8000000/its@8080000 domain created
[    0.052295] fsl-mc MSI: its@8080000 domain created
[    0.053575] EFI services will not be available.
[    0.055684] smp: Bringing up secondary CPUs ...
[    0.059204] Detected PIPT I-cache on CPU1
[    0.059965] GICv3: CPU1: found redistributor 1 region 0:0x00000000080c0000
[    0.060104] GICv3: CPU1: using allocated LPI pending table @0x0000000043660000
[    0.060391] CPU1: Booted secondary processor 0x0000000001 [0x410fd083]
[    0.064005] smp: Brought up 1 node, 2 CPUs
[    0.064352] SMP: Total of 2 processors activated.
[    0.064518] CPU features: detected: 32-bit EL0 Support
[    0.064600] CPU features: detected: 32-bit EL1 Support
[    0.064714] CPU features: detected: CRC32 instructions
[    0.066909] CPU: All CPU(s) started at EL1
[    0.067139] alternatives: applying system-wide alternatives
[    0.083162] devtmpfs: initialized
[    0.098419] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645041785100000 ns
[    0.098679] futex hash table entries: 512 (order: 3, 32768 bytes, linear)
[    0.102382] pinctrl core: initialized pinctrl subsystem
[    0.107776] DMI not present or invalid.
[    0.109532] ---[576][drivers/base/dd.c] dev[reg-dummy] probe start
[    0.111754] ===-[584][drivers/base/dd.c] dev[reg-dummy] probe ok
[    0.115860] NET: Registered PF_NETLINK/PF_ROUTE protocol family
[    0.124535] DMA: preallocated 256 KiB GFP_KERNEL pool for atomic allocations
[    0.125118] DMA: preallocated 256 KiB GFP_KERNEL|GFP_DMA pool for atomic allocations
[    0.125618] DMA: preallocated 256 KiB GFP_KERNEL|GFP_DMA32 pool for atomic allocations
[    0.125949] audit: initializing netlink subsys (disabled)
[    0.127489] audit: type=2000 audit(0.120:1): state=initialized audit_enabled=0 res=1
[    0.131258] thermal_sys: Registered thermal governor 'step_wise'
[    0.131305] thermal_sys: Registered thermal governor 'power_allocator'
[    0.132108] cpuidle: using governor menu
[    0.133417] hw-breakpoint: found 6 breakpoint and 4 watchpoint registers.
[    0.134227] ASID allocator initialised with 65536 entries
[    0.139751] Serial: AMBA PL011 UART driver
[    0.196661] ---[576][drivers/base/dd.c] dev[9000000.pl011] probe start
[    0.199490] ---[576][drivers/base/dd.c] dev[9000000.pl011:0] probe start
[    0.199608] ===-[584][drivers/base/dd.c] dev[9000000.pl011:0] probe ok
[    0.200496] ---[576][drivers/base/dd.c] dev[9000000.pl011:0.0] probe start
[    0.200713] ===-[584][drivers/base/dd.c] dev[9000000.pl011:0.0] probe ok
[    0.201567] 9000000.pl011: ttyAMA0 at MMIO 0x9000000 (irq = 14, base_baud = 0) is a PL011 rev1
[    0.202777] printk: console [ttyAMA0] enabled
[    0.203135] printk: bootconsole [pl11] disabled
[    0.214404] ===-[584][drivers/base/dd.c] dev[9000000.pl011] probe ok
[    0.220226] Modules: 20272 pages in range for non-PLT usage
[    0.220255] Modules: 511792 pages in range for PLT usage
[    0.224388] HugeTLB: registered 1.00 GiB page size, pre-allocated 0 pages
[    0.224544] HugeTLB: 0 KiB vmemmap can be freed for a 1.00 GiB page
[    0.224630] HugeTLB: registered 32.0 MiB page size, pre-allocated 0 pages
[    0.224694] HugeTLB: 0 KiB vmemmap can be freed for a 32.0 MiB page
[    0.224761] HugeTLB: registered 2.00 MiB page size, pre-allocated 0 pages
[    0.224822] HugeTLB: 0 KiB vmemmap can be freed for a 2.00 MiB page
[    0.224891] HugeTLB: registered 64.0 KiB page size, pre-allocated 0 pages
[    0.224952] HugeTLB: 0 KiB vmemmap can be freed for a 64.0 KiB page
[    0.233480] ACPI: Interpreter disabled.
[    0.242731] iommu: Default domain type: Translated
[    0.242826] iommu: DMA domain TLB invalidation policy: strict mode
[    0.243916] SCSI subsystem initialized
[    0.244731] libata version 3.00 loaded.
[    0.246096] usbcore: registered new interface driver usbfs
[    0.246347] usbcore: registered new interface driver hub
[    0.246545] usbcore: registered new device driver usb
[    0.249285] pps_core: LinuxPPS API ver. 1 registered
[    0.249356] pps_core: Software ver. 5.3.6 - Copyright 2005-2007 Rodolfo Giometti <giometti@linux.it>
[    0.249507] PTP clock support registered
[    0.250139] EDAC MC: Ver: 3.0.0
[    0.251204] ---[576][drivers/base/dd.c] dev[psci] probe start
[    0.251353] ===-[584][drivers/base/dd.c] dev[psci] probe ok
[    0.252679] scmi_core: SCMI protocol bus registered
[    0.255672] FPGA manager framework
[    0.256335] Advanced Linux Sound Architecture Driver Initialized.
[    0.265123] vgaarb: loaded
[    0.267638] clocksource: Switched to clocksource arch_sys_counter
[    0.270727] VFS: Disk quotas dquot_6.6.0
[    0.270930] VFS: Dquot-cache hash table entries: 512 (order 0, 4096 bytes)
[    0.273038] pnp: PnP ACPI: disabled
[    0.289076] NET: Registered PF_INET protocol family
[    0.290307] IP idents hash table entries: 32768 (order: 6, 262144 bytes, linear)
[    0.294115] tcp_listen_portaddr_hash hash table entries: 1024 (order: 2, 16384 bytes, linear)
[    0.294292] Table-perturb hash table entries: 65536 (order: 6, 262144 bytes, linear)
[    0.294424] TCP established hash table entries: 16384 (order: 5, 131072 bytes, linear)
[    0.294680] TCP bind hash table entries: 16384 (order: 7, 524288 bytes, linear)
[    0.294993] TCP: Hash tables configured (established 16384 bind 16384)
[    0.296199] UDP hash table entries: 1024 (order: 3, 32768 bytes, linear)
[    0.296497] UDP-Lite hash table entries: 1024 (order: 3, 32768 bytes, linear)
[    0.297490] NET: Registered PF_UNIX/PF_LOCAL protocol family
[    0.300723] RPC: Registered named UNIX socket transport module.
[    0.300843] RPC: Registered udp transport module.
[    0.300906] RPC: Registered tcp transport module.
[    0.300963] RPC: Registered tcp-with-tls transport module.
[    0.301025] RPC: Registered tcp NFSv4.1 backchannel transport module.
[    0.301194] PCI: CLS 0 bytes, default 64
[    0.307964] kvm [1]: HYP mode not available
[    0.311169] Initialise system trusted keyrings
[    0.312115] ===============[7925][mm/vmscan.c]
[    0.312209] ===============[7928][mm/vmscan.c]nid[0]
[    0.312990] ===============[7931][mm/vmscan.c]
[    0.313261] workingset: timestamp_bits=42 max_order=19 bucket_order=0
[    0.314855] squashfs: version 4.0 (2009/01/31) Phillip Lougher
[    0.316385] NFS: Registering the id_resolver key type
[    0.316805] Key type id_resolver registered
[    0.316892] Key type id_legacy registered
[    0.317201] nfs4filelayout_init: NFSv4 File Layout Driver Registering...
[    0.317374] nfs4flexfilelayout_init: NFSv4 Flexfile Layout Driver Registering...
[    0.318038] 9p: Installing v9fs 9p2000 file system support
[    0.340570] Key type asymmetric registered
[    0.340696] Asymmetric key parser 'x509' registered
[    0.341167] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 245)
[    0.341349] io scheduler mq-deadline registered
[    0.341450] io scheduler kyber registered
[    0.341786] io scheduler bfq registered
[    0.350086] ---[576][drivers/base/dd.c] dev[platform-bus@c000000] probe start
[    0.373789] ---[576][drivers/base/dd.c] dev[9030000.pl061] probe start
[    0.378095] pl061_gpio 9030000.pl061: PL061 GPIO chip registered
[    0.378321] ===-[584][drivers/base/dd.c] dev[9030000.pl061] probe ok
[    0.386028] ---[576][drivers/base/dd.c] dev[4010000000.pcie] probe start
[    0.386350] pci-host-generic 4010000000.pcie: host bridge /pcie@10000000 ranges:
[    0.387000] pci-host-generic 4010000000.pcie:       IO 0x003eff0000..0x003effffff -> 0x0000000000
[    0.387568] pci-host-generic 4010000000.pcie:      MEM 0x0010000000..0x003efeffff -> 0x0010000000
[    0.387740] pci-host-generic 4010000000.pcie:      MEM 0x8000000000..0xffffffffff -> 0x8000000000
[    0.388438] pci-host-generic 4010000000.pcie: Memory resource size exceeds max for 32 bits
[    0.388963] pci-host-generic 4010000000.pcie: ECAM at [mem 0x4010000000-0x401fffffff] for [bus 00-ff]
[    0.390075] pci-host-generic 4010000000.pcie: PCI host bridge to bus 0000:00
[    0.390330] pci_bus 0000:00: root bus resource [bus 00-ff]
[    0.390447] pci_bus 0000:00: root bus resource [io  0x0000-0xffff]
[    0.390546] pci_bus 0000:00: root bus resource [mem 0x10000000-0x3efeffff]
[    0.390635] pci_bus 0000:00: root bus resource [mem 0x8000000000-0xffffffffff]
[    0.392123] pci 0000:00:00.0: [1b36:0008] type 00 class 0x060000
[    0.395386] pci 0000:00:01.0: [1af4:1050] type 00 class 0x038000
[    0.395851] pci 0000:00:01.0: reg 0x14: [mem 0x00000000-0x00000fff]
[    0.396030] pci 0000:00:01.0: reg 0x20: [mem 0x00000000-0x00003fff 64bit pref]
[    0.398343] pci 0000:00:01.0: BAR 4: assigned [mem 0x8000000000-0x8000003fff 64bit pref]
[    0.398674] pci 0000:00:01.0: BAR 1: assigned [mem 0x10000000-0x10000fff]
[    0.399416] ===-[584][drivers/base/dd.c] dev[4010000000.pcie] probe ok
[    0.408449] EINJ: ACPI disabled.
[    0.477326] ---[576][drivers/base/dd.c] dev[a000000.virtio_mmio] probe start
[    0.478011] ---[576][drivers/base/dd.c] dev[a000200.virtio_mmio] probe start
[    0.478265] ---[576][drivers/base/dd.c] dev[a000400.virtio_mmio] probe start
[    0.478506] ---[576][drivers/base/dd.c] dev[a000600.virtio_mmio] probe start
[    0.478739] ---[576][drivers/base/dd.c] dev[a000800.virtio_mmio] probe start
[    0.478951] ---[576][drivers/base/dd.c] dev[a000a00.virtio_mmio] probe start
[    0.479169] ---[576][drivers/base/dd.c] dev[a000c00.virtio_mmio] probe start
[    0.479378] ---[576][drivers/base/dd.c] dev[a000e00.virtio_mmio] probe start
[    0.479683] ---[576][drivers/base/dd.c] dev[a001000.virtio_mmio] probe start
[    0.479921] ---[576][drivers/base/dd.c] dev[a001200.virtio_mmio] probe start
[    0.480143] ---[576][drivers/base/dd.c] dev[a001400.virtio_mmio] probe start
[    0.480387] ---[576][drivers/base/dd.c] dev[a001600.virtio_mmio] probe start
[    0.480619] ---[576][drivers/base/dd.c] dev[a001800.virtio_mmio] probe start
[    0.480852] ---[576][drivers/base/dd.c] dev[a001a00.virtio_mmio] probe start
[    0.481093] ---[576][drivers/base/dd.c] dev[a001c00.virtio_mmio] probe start
[    0.481315] ---[576][drivers/base/dd.c] dev[a001e00.virtio_mmio] probe start
[    0.481535] ---[576][drivers/base/dd.c] dev[a002000.virtio_mmio] probe start
[    0.481766] ---[576][drivers/base/dd.c] dev[a002200.virtio_mmio] probe start
[    0.481998] ---[576][drivers/base/dd.c] dev[a002400.virtio_mmio] probe start
[    0.482244] ---[576][drivers/base/dd.c] dev[a002600.virtio_mmio] probe start
[    0.482479] ---[576][drivers/base/dd.c] dev[a002800.virtio_mmio] probe start
[    0.482710] ---[576][drivers/base/dd.c] dev[a002a00.virtio_mmio] probe start
[    0.482948] ---[576][drivers/base/dd.c] dev[a002c00.virtio_mmio] probe start
[    0.483174] ---[576][drivers/base/dd.c] dev[a002e00.virtio_mmio] probe start
[    0.483394] ---[576][drivers/base/dd.c] dev[a003000.virtio_mmio] probe start
[    0.483690] ---[576][drivers/base/dd.c] dev[a003200.virtio_mmio] probe start
[    0.483936] ---[576][drivers/base/dd.c] dev[a003400.virtio_mmio] probe start
[    0.484165] ---[576][drivers/base/dd.c] dev[a003600.virtio_mmio] probe start
[    0.484394] ---[576][drivers/base/dd.c] dev[a003800.virtio_mmio] probe start
[    0.484631] ---[576][drivers/base/dd.c] dev[a003a00.virtio_mmio] probe start
[    0.484853] ---[576][drivers/base/dd.c] dev[a003c00.virtio_mmio] probe start
[    0.485133] ---[576][drivers/base/dd.c] dev[a003e00.virtio_mmio] probe start
[    0.485833] ===-[584][drivers/base/dd.c] dev[a003e00.virtio_mmio] probe ok
[    0.501916] Serial: 8250/16550 driver, 4 ports, IRQ sharing enabled
[    0.504319] ---[576][drivers/base/dd.c] dev[serial8250:0] probe start
[    0.504400] ===-[584][drivers/base/dd.c] dev[serial8250:0] probe ok
[    0.504642] ---[576][drivers/base/dd.c] dev[serial8250:0.0] probe start
[    0.504741] ===-[584][drivers/base/dd.c] dev[serial8250:0.0] probe ok
[    0.505697] ---[576][drivers/base/dd.c] dev[serial8250:0.1] probe start
[    0.505797] ===-[584][drivers/base/dd.c] dev[serial8250:0.1] probe ok
[    0.506569] ---[576][drivers/base/dd.c] dev[serial8250:0.2] probe start
[    0.506664] ===-[584][drivers/base/dd.c] dev[serial8250:0.2] probe ok
[    0.507432] ---[576][drivers/base/dd.c] dev[serial8250:0.3] probe start
[    0.507523] ===-[584][drivers/base/dd.c] dev[serial8250:0.3] probe ok
[    0.508429] ---[576][drivers/base/dd.c] dev[serial8250] probe start
[    0.508557] ===-[584][drivers/base/dd.c] dev[serial8250] probe ok
[    0.512710] SuperH (H)SCI(F) driver initialized
[    0.514147] msm_serial: driver initialized
[    0.515976] STM32 USART driver initialized
[    0.521727] ---[576][drivers/base/dd.c] dev[9050000.smmuv3] probe start
[    0.523912] arm-smmu-v3 9050000.smmuv3: ias 44-bit, oas 44-bit (features 0x00008305)
[    0.525243] arm-smmu-v3 9050000.smmuv3: allocated 65536 entries for cmdq
[    0.526852] arm-smmu-v3 9050000.smmuv3: allocated 32768 entries for evtq
[    0.531868] ===-[584][drivers/base/dd.c] dev[9050000.smmuv3] probe ok
[    0.549187] loop: module loaded
[    0.552341] megasas: 07.725.01.00-rc1
[    0.557313] ---[576][drivers/base/dd.c] dev[0.flash] probe start
[    0.557828] ====[47][drivers/base/power/generic_ops.c]  resume 1 dev[0.flash]
[    0.557949] ====[49][drivers/base/power/generic_ops.c]  resume 2 dev[0.flash]
[    0.558388] physmap-flash 0.flash: physmap platform flash device: [mem 0x00000000-0x03ffffff]
[    0.559733] 0.flash: Found 2 x16 devices at 0x0 in 32-bit bank. Manufacturer ID 0x000000 Chip ID 0x000000
[    0.560240] Intel/Sharp Extended Query Table at 0x0031
[    0.560796] Using buffer write method
[    0.561148] erase region 0: offset=0x0,size=0x40000,blocks=256
[    0.561337] physmap-flash 0.flash: physmap platform flash device: [mem 0x04000000-0x07ffffff]
[    0.561726] 0.flash: Found 2 x16 devices at 0x0 in 32-bit bank. Manufacturer ID 0x000000 Chip ID 0x000000
[    0.561870] Intel/Sharp Extended Query Table at 0x0031
[    0.562208] Using buffer write method
[    0.562279] erase region 0: offset=0x0,size=0x40000,blocks=256
[    0.562345] Concatenating MTD devices:
[    0.562413] (0): "0.flash"
[    0.562465] (1): "0.flash"
[    0.562507] into device "0.flash"
[    0.582299] ===-[584][drivers/base/dd.c] dev[0.flash] probe ok
[    0.597925] tun: Universal TUN/TAP device driver, 1.6
[    0.598696] ---[576][drivers/base/dd.c] dev[virtio0] probe start
[    0.604811] ===-[584][drivers/base/dd.c] dev[virtio0] probe ok
[    0.607117] thunder_xcv, ver 1.0
[    0.607255] thunder_bgx, ver 1.0
[    0.607389] nicpf, ver 1.0
[    0.611976] hns3: Hisilicon Ethernet Network Driver for Hip08 Family - version
[    0.612079] hns3: Copyright (c) 2017 Huawei Corporation.
[    0.612364] hclge is initializing
[    0.612597] e1000: Intel(R) PRO/1000 Network Driver
[    0.612664] e1000: Copyright (c) 1999-2006 Intel Corporation.
[    0.612844] e1000e: Intel(R) PRO/1000 Network Driver
[    0.612916] e1000e: Copyright(c) 1999 - 2015 Intel Corporation.
[    0.613067] igb: Intel(R) Gigabit Ethernet Network Driver
[    0.613144] igb: Copyright (c) 2007-2014 Intel Corporation.
[    0.613318] igbvf: Intel(R) Gigabit Virtual Function Network Driver
[    0.613409] igbvf: Copyright (c) 2009 - 2012 Intel Corporation.
[    0.614513] sky2: driver version 1.30
[    0.618357] VFIO - User Level meta-driver version: 0.3
[    0.627709] usbcore: registered new interface driver usb-storage
[    0.635171] ---[576][drivers/base/dd.c] dev[9010000.pl031] probe start
[    0.638349] ---[576][drivers/base/dd.c] dev[alarmtimer.0.auto] probe start
[    0.638434] ===-[584][drivers/base/dd.c] dev[alarmtimer.0.auto] probe ok
[    0.638932] rtc-pl031 9010000.pl031: registered as rtc0
[    0.639761] rtc-pl031 9010000.pl031: setting system clock to 2024-11-06T09:08:23 UTC (1730884103)
[    0.640363] ===-[584][drivers/base/dd.c] dev[9010000.pl031] probe ok
[    0.642320] i2c_dev: i2c /dev entries driver
[    0.663411] ---[576][drivers/base/dd.c] dev[psci-cpuidle] probe start
[    0.664861] sdhci: Secure Digital Host Controller Interface driver
[    0.664937] sdhci: Copyright(c) Pierre Ossman
[    0.666956] Synopsys Designware Multimedia Card Interface Driver
[    0.669836] sdhci-pltfm: SDHCI platform and OF driver helper
[    0.675457] ledtrig-cpu: registered to indicate activity on CPUs
[    0.681480] usbcore: registered new interface driver usbhid
[    0.681559] usbhid: USB HID core driver
[    0.689642] ---[576][drivers/base/dd.c] dev[pmu] probe start
[    0.691690] hw perfevents: enabled with armv8_pmuv3 PMU driver, 7 counters available
[    0.692070] ===-[584][drivers/base/dd.c] dev[pmu] probe ok
[    0.704557] ---[576][drivers/base/dd.c] dev[snd-soc-dummy] probe start
[    0.705246] ===-[584][drivers/base/dd.c] dev[snd-soc-dummy] probe ok
[    0.706853] NET: Registered PF_PACKET protocol family
[    0.708046] 9pnet: Installing 9P2000 support
[    0.708348] Key type dns_resolver registered
[    0.727711] registered taskstats version 1
[    0.728885] Loading compiled-in X.509 certificates
[    0.750093] virtio-pci 0000:00:01.0: Adding to iommu group 0
[    0.754158] ---[576][drivers/base/dd.c] dev[0000:00:01.0] probe start
[    0.755455] virtio-pci 0000:00:01.0: enabling device (0000 -> 0002)
[    0.758245] ===-[584][drivers/base/dd.c] dev[0000:00:01.0] probe ok
[    0.760091] ---[576][drivers/base/dd.c] dev[gpio-keys] probe start
[    0.764693] input: gpio-keys as /devices/platform/gpio-keys/input/input0
[    0.766265] ===-[584][drivers/base/dd.c] dev[gpio-keys] probe ok
[    0.795804] Sending DHCP requests ., OK
[    0.807978] IP-Config: Got DHCP answer from 10.0.2.2, my address is 10.0.2.15
[    0.808368] IP-Config: Complete:
[    0.808418]      device=eth0, hwaddr=52:54:00:12:34:56, ipaddr=10.0.2.15, mask=255.255.255.0, gw=10.0.2.2
[    0.808573]      host=10.0.2.15, domain=, nis-domain=(none)
[    0.808636]      bootserver=10.0.2.2, rootserver=10.0.2.2, rootpath=
[    0.808663]      nameserver0=10.0.2.3
[    1.218218] clk: Disabling unused clocks
[    1.218683] ALSA device list:
[    1.218776]   No soundcards found.
[    1.222444] uart-pl011 9000000.pl011: no DMA platform data
[    1.313840] VFS: Mounted root (nfs4 filesystem) on device 0:29.
[    1.314699] devtmpfs: mounted
[    1.362541] Freeing unused kernel memory: 12736K
[    1.363430] Run /sbin/init as init process
[    1.363606]   with arguments:
[    1.363632]     /sbin/init
[    1.363650]   with environment:
[    1.363682]     HOME=/
[    1.363699]     TERM=linux
[    7.161197] data_breakpoint: loading out-of-tree module taints kernel.
[    7.165642] failing symbol_get of non-GPLONLY symbol jiffies.
[    7.165769] ==================[50]hw_break_module_init]
[    7.166006] CPU: 0 PID: 133 Comm: insmod Tainted: G           O       6.6.6-g41c772dcde14-dirty #96
[    7.166107] Hardware name: linux,dummy-virt (DT)
[    7.166300] Call trace:
[    7.166339]  dump_backtrace+0x98/0xf8
[    7.166431]  show_stack+0x20/0x38
[    7.166485]  dump_stack_lvl+0x90/0xb0
[    7.166525]  dump_stack+0x18/0x28
[    7.166561]  hw_break_module_init+0x74/0xff8 [data_breakpoint]
[    7.166972]  do_one_initcall+0x60/0x2c0
[    7.167018]  do_init_module+0x60/0x210
[    7.167060]  load_module+0x1e2c/0x1f48
[    7.167099]  init_module_from_file+0x90/0xe0
[    7.167142]  __arm64_sys_finit_module+0x1e4/0x2f8
[    7.167191]  invoke_syscall+0x50/0x120
[    7.167235]  el0_svc_common.constprop.0+0x48/0xf0
[    7.167289]  do_el0_svc+0x24/0x38
[    7.167329]  el0_svc+0x48/0xf8
[    7.167365]  el0t_64_sync_handler+0x120/0x130
[    7.167412]  el0t_64_sync+0x190/0x198
[    7.167725] ==================[54]hw_break_module_init]
[    7.168259] ==============[948][hw_breakpoint_event_init]
[    7.168349] CPU: 0 PID: 133 Comm: insmod Tainted: G           O       6.6.6-g41c772dcde14-dirty #96
[    7.168465] Hardware name: linux,dummy-virt (DT)
[    7.168527] Call trace:
[    7.168561]  dump_backtrace+0x98/0xf8
[    7.168612]  show_stack+0x20/0x38
[    7.168650]  dump_stack_lvl+0x90/0xb0
[    7.168700]  dump_stack+0x18/0x28
[    7.168745]  hw_breakpoint_event_init+0x48/0xb0
[    7.168796]  perf_try_init_event+0x58/0x168
[    7.168844]  perf_event_alloc+0x3b8/0xf08
[    7.168924]  perf_event_create_kernel_counter+0x58/0x1f8
[    7.169010]  register_wide_hw_breakpoint+0xb0/0x130
[    7.169085]  hw_break_module_init+0xd8/0xff8 [data_breakpoint]
[    7.169166]  do_one_initcall+0x60/0x2c0
[    7.169208]  do_init_module+0x60/0x210
[    7.169256]  load_module+0x1e2c/0x1f48
[    7.169303]  init_module_from_file+0x90/0xe0
[    7.169376]  __arm64_sys_finit_module+0x1e4/0x2f8
[    7.169479]  invoke_syscall+0x50/0x120
[    7.169563]  el0_svc_common.constprop.0+0x48/0xf0
[    7.169647]  do_el0_svc+0x24/0x38
[    7.169694]  el0_svc+0x48/0xf8
[    7.169735]  el0t_64_sync_handler+0x120/0x130
[    7.169802]  el0t_64_sync+0x190/0x198
[    7.169875] ==============[950][hw_breakpoint_event_init]
[    7.191390] ==============[972][hw_breakpoint_add]
[    7.191654] CPU: 0 PID: 133 Comm: insmod Tainted: G           O       6.6.6-g41c772dcde14-dirty #96
[    7.191915] Hardware name: linux,dummy-virt (DT)
[    7.192084] Call trace:
[    7.192166]  dump_backtrace+0x98/0xf8
[    7.192315]  show_stack+0x20/0x38
[    7.192469]  dump_stack_lvl+0x60/0xb0
[    7.192635]  dump_stack+0x18/0x28
[    7.192752]  hw_breakpoint_add+0x4c/0xb8
[    7.192893]  event_sched_in+0xa4/0x158
[    7.193041]  merge_sched_in+0x1d4/0x338
[    7.193188]  visit_groups_merge.constprop.0.isra.0+0x1b4/0x4f8
[    7.193382]  ctx_groups_sched_in+0x9c/0x110
[    7.193552]  ctx_sched_in+0xb0/0x160
[    7.193687]  ctx_resched+0x100/0x210
[    7.193818]  __perf_install_in_context+0x194/0x210
[    7.193962]  remote_function+0x6c/0x88
[    7.194096]  generic_exec_single+0xdc/0x120
[    7.194248]  smp_call_function_single+0x188/0x1e0
[    7.194407]  perf_install_in_context+0x1ec/0x210
[    7.194549]  perf_event_create_kernel_counter+0x180/0x1f8
[    7.194756]  register_wide_hw_breakpoint+0xb0/0x130
[    7.194920]  hw_break_module_init+0xd8/0xff8 [data_breakpoint]
[    7.195128]  do_one_initcall+0x60/0x2c0
[    7.195248]  do_init_module+0x60/0x210
[    7.195367]  load_module+0x1e2c/0x1f48
[    7.195468]  init_module_from_file+0x90/0xe0
[    7.195599]  __arm64_sys_finit_module+0x1e4/0x2f8
[    7.195759]  invoke_syscall+0x50/0x120
[    7.195878]  el0_svc_common.constprop.0+0x48/0xf0
[    7.196020]  do_el0_svc+0x24/0x38
[    7.196114]  el0_svc+0x48/0xf8
[    7.196221]  el0t_64_sync_handler+0x120/0x130
[    7.196346]  el0t_64_sync+0x190/0x198
[    7.196500] ==============[974][hw_breakpoint_add]
[    7.198157] ==============[948][hw_breakpoint_event_init]
[    7.198303] CPU: 0 PID: 133 Comm: insmod Tainted: G           O       6.6.6-g41c772dcde14-dirty #96
[    7.198488] Hardware name: linux,dummy-virt (DT)
[    7.198598] Call trace:
[    7.198665]  dump_backtrace+0x98/0xf8
[    7.198751]  show_stack+0x20/0x38
[    7.198837]  dump_stack_lvl+0x90/0xb0
[    7.198915]  dump_stack+0x18/0x28
[    7.198987]  hw_breakpoint_event_init+0x48/0xb0
[    7.199116]  perf_try_init_event+0x58/0x168
[    7.199228]  perf_event_alloc+0x3b8/0xf08
[    7.199331]  perf_event_create_kernel_counter+0x58/0x1f8
[    7.199471]  register_wide_hw_breakpoint+0xb0/0x130
[    7.199692]  hw_break_module_init+0xd8/0xff8 [data_breakpoint]
[    7.199861]  do_one_initcall+0x60/0x2c0
[    7.199947]  do_init_module+0x60/0x210
[    7.200033]  load_module+0x1e2c/0x1f48
[    7.200125]  init_module_from_file+0x90/0xe0
[    7.200221]  __arm64_sys_finit_module+0x1e4/0x2f8
[    7.200317]  invoke_syscall+0x50/0x120
[    7.200402]  el0_svc_common.constprop.0+0x48/0xf0
[    7.200508]  do_el0_svc+0x24/0x38
[    7.200581]  el0_svc+0x48/0xf8
[    7.200660]  el0t_64_sync_handler+0x120/0x130
[    7.200758]  el0t_64_sync+0x190/0x198
[    7.200932] ==============[950][hw_breakpoint_event_init]
[    7.201147] ==============[972][hw_breakpoint_add]
[    7.201333] CPU: 1 PID: 79 Comm: syslogd Tainted: G           O       6.6.6-g41c772dcde14-dirty #96
[    7.201476] Hardware name: linux,dummy-virt (DT)
[    7.201553] Call trace:
[    7.201603]  dump_backtrace+0x98/0xf8
[    7.201680]  show_stack+0x20/0x38
[    7.201740]  dump_stack_lvl+0x60/0xb0
[    7.201804]  dump_stack+0x18/0x28
[    7.201861]  hw_breakpoint_add+0x4c/0xb8
[    7.201930]  event_sched_in+0xa4/0x158
[    7.201999]  merge_sched_in+0x1d4/0x338
[    7.202063]  visit_groups_merge.constprop.0.isra.0+0x1b4/0x4f8
[    7.202153]  ctx_groups_sched_in+0x9c/0x110
[    7.202224]  ctx_sched_in+0xb0/0x160
[    7.202287]  ctx_resched+0x100/0x210
[    7.202350]  __perf_install_in_context+0x194/0x210
[    7.202429]  remote_function+0x6c/0x88
[    7.202494]  __flush_smp_call_function_queue+0x20c/0x4f8
[    7.202582]  generic_smp_call_function_single_interrupt+0x1c/0x30
[    7.202672]  ipi_handler+0x1c8/0x278
[    7.202727]  handle_percpu_devid_irq+0x90/0x238
[    7.202794]  generic_handle_domain_irq+0x34/0x58
[    7.202859]  gic_handle_irq+0x58/0x140
[    7.202914]  call_on_irq_stack+0x24/0x58
[    7.202974]  do_interrupt_handler+0x88/0x98
[    7.203036]  el0_interrupt+0x50/0xe8
[    7.203089]  __el0_irq_handler_common+0x18/0x28
[    7.203152]  el0t_64_irq_handler+0x10/0x20
[    7.203211]  el0t_64_irq+0x190/0x198
[    7.203266] ==============[974][hw_breakpoint_add]
[    7.203409] HW Breakpoint for jiffies write installed
[   11.238370] amba 90d0000.sp804: deferred probe pending
[   11.239000] amba 90c0000.sp804: deferred probe pending

# rmmod data_breakpoint.ko 
[   15.486680] ==============[988][hw_breakpoint_del]                                                  
[   15.486799] CPU: 0 PID: 133 Comm: rmmod Tainted: G           O       6.6.6-g41c772dcde14-dirty #97  
[   15.486900] Hardware name: linux,dummy-virt (DT)                                                    
[   15.486949] Call trace:                                                                             
[   15.486977]  dump_backtrace+0x98/0xf8                                                               
[   15.487035]  show_stack+0x20/0x38                                                                   
[   15.487074]  dump_stack_lvl+0x60/0xb0                                                               
[   15.487118]  dump_stack+0x18/0x28                                                                   
[   15.487155]  hw_breakpoint_del+0x48/0x80                                                            
[   15.487199]  event_sched_out+0x9c/0x230                                                             
[   15.487243]  __perf_remove_from_context+0x58/0x2d0                                                  
[   15.487293]  event_function+0xdc/0x148                                                              
[   15.487337]  remote_function+0x6c/0x88                                                              
[   15.487378]  generic_exec_single+0xdc/0x120                                                         
[   15.487423]  smp_call_function_single+0x188/0x1e0                                                   
[   15.487495]  event_function_call+0x16c/0x180                                                        
[   15.487540]  perf_remove_from_context+0x54/0xb0                                                     
[   15.487598]  perf_event_release_kernel+0x80/0x2f8                                                   
[   15.487666]  unregister_wide_hw_breakpoint+0x78/0xc8                                                
[   15.487718]  hw_break_module_exit+0x24/0xfb0 [data_breakpoint]                                      
[   15.487862]  __arm64_sys_delete_module+0x1ac/0x2a8                                                  
[   15.487918]  invoke_syscall+0x50/0x120                                                              
[   15.487970]  el0_svc_common.constprop.0+0x48/0xf0                                                   
[   15.488028]  do_el0_svc+0x24/0x38                                                                   
[   15.488093]  el0_svc+0x48/0xf8                                                                      
[   15.488180]  el0t_64_sync_handler+0x120/0x130                                                       
[   15.488259]  el0t_64_sync+0x190/0x198                                                               
[   15.488328] ==============[990][hw_breakpoint_del]                                                  
[   15.504793] ==============[988][hw_breakpoint_del]                                                  
[   15.505010] CPU: 1 PID: 0 Comm: swapper/1 Tainted: G           O       6.6.6-g41c772dcde14-dirty #97
[   15.505196] Hardware name: linux,dummy-virt (DT)                                                    
[   15.505290] Call trace:                                                                             
[   15.505344]  dump_backtrace+0x98/0xf8                                                               
[   15.505452]  show_stack+0x20/0x38                                                                   
[   15.505540]  dump_stack_lvl+0x60/0xb0                                                               
[   15.505631]  dump_stack+0x18/0x28                                                                   
[   15.505718]  hw_breakpoint_del+0x48/0x80                                                            
[   15.505804]  event_sched_out+0x9c/0x230                                                             
[   15.505900]  __perf_remove_from_context+0x58/0x2d0                                                  
[   15.505999]  event_function+0xdc/0x148                                                              
[   15.506095]  remote_function+0x6c/0x88                                                              
[   15.506178]  __flush_smp_call_function_queue+0x20c/0x4f8                                            
[   15.506305]  generic_smp_call_function_single_interrupt+0x1c/0x30                                   
[   15.506427]  ipi_handler+0x1c8/0x278                                                                
[   15.506514]  handle_percpu_devid_irq+0x90/0x238                                                     
[   15.506616]  generic_handle_domain_irq+0x34/0x58                                                    
[   15.506714]  gic_handle_irq+0x58/0x140                                                              
[   15.506774]  call_on_irq_stack+0x24/0x58                                                            
[   15.506827]  do_interrupt_handler+0x88/0x98                                                         
[   15.506882]  el1_interrupt+0x34/0x68                                                                
[   15.506933]  el1h_64_irq_handler+0x18/0x28                                                          
[   15.506981]  el1h_64_irq+0x64/0x68                                                                  
[   15.507024]  default_idle_call+0x70/0x178                                                           
[   15.507073]  do_idle+0x234/0x2a0                                                                    
[   15.507118]  cpu_startup_entry+0x3c/0x50                                                            
[   15.507166]  secondary_start_kernel+0x138/0x160                                                     
[   15.507221]  __secondary_switched+0xb8/0xc0                                                         
[   15.507275] ==============[990][hw_breakpoint_del]                                                  
[   15.507471] HW Breakpoint for jiffies write uninstalled     




[  123.021405] Dump stack from sample_hbp_handler
[  123.021407] CPU: 1 PID: 134 Comm: insmod Tainted: G           O       6.6.6-g41c772dcde14-dirty #97
[  123.021481] jiffies value is changed
[  123.021550] Hardware name: linux,dummy-virt (DT)
[  123.018097] Call trace: 
[  123.018129]  dump_backtrace+0x98/0xf8
[  123.018176]  show_stack+0x20/0x38
[  123.018215]  dump_stack_lvl+0x60/0xb0
[  123.018256]  dump_stack+0x18/0x28
[  123.018295]  sample_hbp_handler+0x2c/0x48 [data_breakpoint]
[  123.018362]  __perf_event_overflow+0xd0/0x268
[  123.018410]  perf_swevent_event+0x144/0x150
[  123.018456]  perf_bp_event+0xcc/0xe0
[  123.018496]  watchpoint_report+0x6c/0xb8
[  123.018540]  watchpoint_handler+0xc0/0x268
[  123.018584]  do_debug_exception+0x74/0x118
[  123.018632]  el1_dbg+0x70/0x90
[  123.018668]  el1h_64_sync_handler+0xc4/0xe8
[  123.018713]  el1h_64_sync+0x64/0x68
[  123.018753]  tick_sched_do_timer+0x54/0xc8
[  123.018797]  tick_sched_timer+0x48/0xb8
[  123.018840]  __hrtimer_run_queues+0x2e4/0x380
[  123.018887]  hrtimer_interrupt+0xf0/0x258
[  123.018931]  arch_timer_handler_virt+0x34/0x58
[  123.018978]  handle_percpu_devid_irq+0x90/0x238
[  123.019028]  generic_handle_domain_irq+0x34/0x58
[  123.019077]  gic_handle_irq+0x58/0x140
[  123.019118]  call_on_irq_stack+0x24/0x58
[  123.019160]  do_interrupt_handler+0x88/0x98
[  123.019206]  el1_interrupt+0x34/0x68
[  123.019246]  el1h_64_irq_handler+0x18/0x28
[  123.019290]  el1h_64_irq+0x64/0x68
[  123.019331]  generic_exec_single+0xb8/0x120
[  123.019375]  smp_call_function_single+0x188/0x1e0
[  123.019425]  perf_install_in_context+0x1ec/0x210
[  123.019473]  perf_event_create_kernel_counter+0x180/0x1f8
[  123.019527]  register_wide_hw_breakpoint+0xb0/0x130
[  123.019579]  hw_break_module_init+0xd0/0xff8 [data_breakpoint]
[  123.019645]  do_one_initcall+0x60/0x2c0
[  123.019688]  do_init_module+0x60/0x210
[  123.019730]  load_module+0x1e2c/0x1f48
[  123.019771]  init_module_from_file+0x90/0xe0
[  123.019816]  __arm64_sys_finit_module+0x1e4/0x2f8
[  123.019865]  invoke_syscall+0x50/0x120
[  123.019908]  el0_svc_common.constprop.0+0x48/0xf0
[  123.019958]  do_el0_svc+0x24/0x38
[  123.019998]  el0_svc+0x48/0xf8
[  123.020034]  el0t_64_sync_handler+0x120/0x130
[  123.020081]  el0t_64_sync+0x190/0x198
[  123.020123] Dump stack from sample_hbp_handler
[  123.020124] CPU: 0 PID: 0 Comm: swapper/0 Tainted: G           O       6.6.6-g41c772dcde14-dirty #97
[  123.020210] jiffies value is changed
[  123.020267] Hardware name: linux,dummy-virt (DT)
[  123.020352] Call trace:
[  123.020382]  dump_backtrace+0x98/0xf8
[  123.020426]  show_stack+0x20/0x38
[  123.020467]  dump_stack_lvl+0x60/0xb0
[  123.020510]  dump_stack+0x18/0x28
[  123.020551]  sample_hbp_handler+0x2c/0x48 [data_breakpoint]
[  123.020618]  __perf_event_overflow+0xd0/0x268
[  123.020671]  perf_swevent_event+0x144/0x150
[  123.020720]  perf_bp_event+0xcc/0xe0
[  123.020762]  watchpoint_report+0x6c/0xb8
[  123.020809]  watchpoint_handler+0xc0/0x268
[  123.020856]  do_debug_exception+0x74/0x118
[  123.020905]  el1_dbg+0x70/0x90
[  123.020943]  el1h_64_sync_handler+0xc4/0xe8
[  123.020991]  el1h_64_sync+0x64/0x68
[  123.021033]  tick_nohz_next_event+0x50/0x168
[  123.021082]  tick_nohz_idle_stop_tick+0x17c/0x2f8
[  123.021135]  do_idle+0x230/0x2a0
[  123.021177]  cpu_startup_entry+0x3c/0x50
[  123.021224]  rest_init+0xf0/0xf8
[  123.021264]  arch_call_rest_init+0x18/0x20
[  123.021312]  start_kernel+0x688/0x8d8
[  123.021358]  __primary_switched+0xbc/0xd0


# cat 2[   11.245902] amba 90d0000.sp804: deferred probe pending                                     
[   11.246572] amba 90c0000.sp804: deferred probe pending                                            
                                                                                                     
[   11.530113] jiffies value is changed                                                              
[   11.530283] CPU: 1 PID: 135 Comm: cat Tainted: G           O       6.6.6-g41c772dcde14-dirty #97  
[   11.530449] Hardware name: linux,dummy-virt (DT)                                                  
[   11.530541] Call trace:                                                                           
[   11.530594]  dump_backtrace+0x98/0xf8                                                             
[   11.530681]  show_stack+0x20/0x38                                                                 
[   11.530791]  dump_stack_lvl+0x60/0xb0                                                             
[   11.530862]  dump_stack+0x18/0x28                                                                 
[   11.530964]  sample_hbp_handler+0x2c/0x48 [data_breakpoint]                                       
[   11.531198]  __perf_event_overflow+0xd0/0x268                                                     
[   11.531279]  perf_swevent_event+0x144/0x150                                                       
[   11.531356]  perf_bp_event+0xcc/0xe0                                                              
[   11.531422]  breakpoint_handler+0xf4/0x250                                                        
[   11.531515]  do_debug_exception+0x74/0x118                                                        
[   11.531615]  el1_dbg+0x70/0x90                                                                    
[   11.531685]  el1h_64_sync_handler+0xc4/0xe8                                                       
[   11.531763]  el1h_64_sync+0x64/0x68                                                               
[   11.531843]  find_mergeable_anon_vma+0xd8/0x1e8                                                   
[   11.531941]  __anon_vma_prepare+0x4c/0x1a0                                                        
[   11.532012]  __handle_mm_fault+0xff0/0x1160                                                       
[   11.532077]  handle_mm_fault+0x70/0x2b8                                                           
[   11.532159]  do_page_fault+0x1b4/0x4a0                                                            
[   11.532239]  do_translation_fault+0xa4/0xb8                                                       
[   11.532318]  do_mem_abort+0x4c/0xa8                                                               
[   11.532383]  el0_da+0x48/0xb8                                                                     
[   11.532435]  el0t_64_sync_handler+0xb4/0x130                                                      
[   11.532511]  el0t_64_sync+0x190/0x198                                                             
[   11.532587] Dump stack from sample_hbp_handler                                                    
[   11.533020] jiffies value is changed                                                              
[   11.533090] CPU: 1 PID: 135 Comm: cat Tainted: G           O       6.6.6-g41c772dcde14-dirty #97  
[   11.533217] Hardware name: linux,dummy-virt (DT)                                                  
[   11.533285] Call trace:                                                                           
[   11.533346]  dump_backtrace+0x98/0xf8                                                             
[   11.533432]  show_stack+0x20/0x38                                                                 
[   11.533493]  dump_stack_lvl+0x60/0xb0                                                             
[   11.533553]  dump_stack+0x18/0x28                                                                 
[   11.533596]  sample_hbp_handler+0x2c/0x48 [data_breakpoint]                                       
[   11.533669]  __perf_event_overflow+0xd0/0x268                                                     
[   11.533762]  perf_swevent_event+0x144/0x150                                                       
[   11.533829]  perf_bp_event+0xcc/0xe0                                                              
[   11.533900]  breakpoint_handler+0xf4/0x250                                                        
[   11.533975]  do_debug_exception+0x74/0x118                                                        
[   11.534038]  el1_dbg+0x70/0x90                                                                    
[   11.534109]  el1h_64_sync_handler+0xc4/0xe8                                                       
[   11.534179]  el1h_64_sync+0x64/0x68                                                               
[   11.534234]  find_mergeable_anon_vma+0xd8/0x1e8                                                   
[   11.534311]  __anon_vma_prepare+0x4c/0x1a0                                                        
[   11.534387]  __handle_mm_fault+0xff0/0x1160                                                       
[   11.534454]  handle_mm_fault+0x70/0x2b8                                                           
[   11.534519]  do_page_fault+0x1b4/0x4a0                                                            
[   11.534571]  do_translation_fault+0xa4/0xb8                                                       
[   11.534634]  do_mem_abort+0x4c/0xa8                                                               
[   11.534693]  el0_da+0x48/0xb8                                                                     
[   11.534754]  el0t_64_sync_handler+0xb4/0x130                                                      
[   11.534825]  el0t_64_sync+0x190/0x198                                                             
[   11.534899] Dump stack from sample_hbp_handler                                                    
[   11.534993] jiffies value is changed                                                              
[   11.535056] CPU: 1 PID: 135 Comm: cat Tainted: G           O       6.6.6-g41c772dcde14-dirty #97  
[   11.535167] Hardware name: linux,dummy-virt (DT)                                                  
[   11.535235] Call trace:                                                                           

static struct pmu perf_breakpoint = {
	.task_ctx_nr	= perf_sw_context, /* could eventually get its own */

	.event_init	= hw_breakpoint_event_init,
	.add		= hw_breakpoint_add,
	.del		= hw_breakpoint_del,
	.start		= hw_breakpoint_start,
	.stop		= hw_breakpoint_stop,
	.read		= hw_breakpoint_pmu_read,
};


## System init
void start_kernel(void)
	perf_event_init();
		perf_event_init_all_cpus();
		init_srcu_struct(&pmus_srcu);
		perf_pmu_register(&perf_swevent, "software", PERF_TYPE_SOFTWARE);
		perf_pmu_register(&perf_cpu_clock, "cpu_clock", -1);
		perf_pmu_register(&perf_task_clock, "task_clock", -1);
		perf_tp_register();
		perf_event_init_cpu(smp_processor_id()); 
		register_reboot_notifier(&perf_reboot_notifier);
		init_hw_breakpoint(); 
			ret = rhltable_init(&task_bps_ht, &task_bps_ht_params);
			ret = init_breakpoint_slots();
			perf_pmu_register(&perf_breakpoint, "breakpoint", PERF_TYPE_BREAKPOINT);
				ret = idr_alloc(&pmu_idr, pmu, max, 0, GFP_KERNEL);
				pmu->type = type = ret;
				pmu->cpu_pmu_context = alloc_percpu(struct perf_cpu_pmu_context);
				for_each_possible_cpu(cpu)
					cpc = per_cpu_ptr(pmu->cpu_pmu_context, cpu);
					__perf_init_event_pmu_context(&cpc->epc, pmu);
					__perf_mux_hrtimer_init(cpc, cpu);
				list_add_rcu(&pmu->entry, &pmus);
				atomic_set(&pmu->exclusive_cnt, 0);
		perf_event_cache = KMEM_CACHE(perf_event, SLAB_PANIC);
		
		
## user usage	
static int __init hw_break_module_init(void) //module_init
	hw_breakpoint_init(&attr);
	register_wide_hw_breakpoint(&attr, sample_hbp_handler, NULL);
		for_each_online_cpu(cpu)
			bp = perf_event_create_kernel_counter(attr, cpu, NULL, triggered, context);
				event = perf_event_alloc(attr, cpu, task, NULL, NULL, overflow_handler, context, -1);
					event = kmem_cache_alloc_node(perf_event_cache, GFP_KERNEL | __GFP_ZERO, node);
					pmu = perf_init_event(event);
						pmu = idr_find(&pmu_idr, type);
						perf_try_init_event(pmu, event);
							event->pmu->event_init(event); //ops->event_init()
				ctx = find_get_context(task, event);
				pmu_ctx = find_get_pmu_context(pmu, ctx, event);
				perf_install_in_context(ctx, event, event->cpu);
					if (!task)
						cpu_function_call(cpu, __perf_install_in_context, event);
						return;
					if (!task_function_call(task, __perf_install_in_context, event))
						return;
					add_event_to_ctx(event, ctx); 


static int  __perf_install_in_context(void *info)  //irq context
	if (ctx->task)
		reprogram = (ctx->task == current);
	if (event->state > PERF_EVENT_STATE_OFF && is_cgroup_event(event))
		reprogram = cgroup_is_descendant(cgrp->css.cgroup, event->cgrp->css.cgroup);
	if (reprogram)
		ctx_sched_out(ctx, EVENT_TIME);
		add_event_to_ctx(event, ctx);
			list_add_event(event, ctx);
				perf_group_attach(event);
					event->attach_state |= PERF_ATTACH_GROUP;
						list_add_tail(&event->sibling_list, &group_leader->sibling_list); 
		ctx_resched(cpuctx, task_ctx, get_event_type(event));
			perf_event_sched_in(cpuctx, task_ctx);
				ctx_sched_in(&cpuctx->ctx, EVENT_PINNED);
				if (ctx) ctx_sched_in(ctx, EVENT_PINNED);
				ctx_sched_in(&cpuctx->ctx, EVENT_FLEXIBLE);
				if (ctx) ctx_sched_in(ctx, EVENT_FLEXIBLE);
				(ctx_sched_in)==>
					event_type &= ~EVENT_CGROUP;
					ctx->is_active |= (event_type | EVENT_TIME);
					is_active ^= ctx->is_active; /* changed bits */
					if (is_active & EVENT_PINNED)
						ctx_groups_sched_in(ctx, &ctx->pinned_groups, cgroup);
					if (is_active & EVENT_FLEXIBLE)
						ctx_groups_sched_in(ctx, &ctx->flexible_groups, cgroup);
					ctx_groups_sched_in()==>
						list_for_each_entry(pmu_ctx, &ctx->pmu_ctx_list, pmu_ctx_entry)
							pmu_groups_sched_in(ctx, groups, pmu_ctx->pmu);
								visit_groups_merge(ctx, groups, smp_processor_id(), pmu, merge_sched_in, &can_add_hw);
									merge_sched_in(struct perf_event *event, void *data);
										if (group_can_go_on(event, *can_add_hw))
											if (!group_sched_in(event, ctx)) list_add_tail(&event->active_list, get_event_list(event));
												event_sched_in(group_event, ctx);
												for_each_sibling_event(event, group_event);
													event_sched_in(event, ctx);
												(event_sched_in)==>
													perf_pmu_disable(event->pmu);
													event->pmu->add(event, PERF_EF_START); //ops->add
													perf_pmu_enable(event->pmu);
			perf_ctx_enable(&cpuctx->ctx, false);
			if (task_ctx) perf_ctx_enable(task_ctx, false);
	else
		add_event_to_ctx(event, ctx);



el1h_64_sync_handler <- ESR_ELx_EC_BRK64
	el1_dbg(regs, esr);/el0_dbg(regs, esr);
		do_debug_exception(far, esr, regs);
			inf->fn(addr_if_watchpoint, esr, regs)
			==>breakpoint_handler()/watchpoint_handler()
				perf_bp_event(bp, regs);
					perf_sample_data_init(&sample, bp->attr.bp_addr, 0);
						perf_swevent_event(bp, 1, &sample, regs);
							perf_swevent_overflow(event, 0, data, regs);
								__perf_event_overflow(event, throttle, data, regs));
									READ_ONCE(event->overflow_handler)(event, data, regs);
										=>sample_hbp_handler(event, data, regs);

[   11.533346]  dump_backtrace+0x98/0xf8                                                             
[   11.533432]  show_stack+0x20/0x38                                                                 
[   11.533493]  dump_stack_lvl+0x60/0xb0                                                             
[   11.533553]  dump_stack+0x18/0x28                                                                 
[   11.533596]  sample_hbp_handler+0x2c/0x48 [data_breakpoint]                                       
[   11.533669]  __perf_event_overflow+0xd0/0x268                                                     
[   11.533762]  perf_swevent_event+0x144/0x150                                                       
[   11.533829]  perf_bp_event+0xcc/0xe0                                                              
[   11.533900]  breakpoint_handler+0xf4/0x250                                                        
[   11.533975]  do_debug_exception+0x74/0x118                                                        
[   11.534038]  el1_dbg+0x70/0x90                                                                    
[   11.534109]  el1h_64_sync_handler+0xc4/0xe8                                                       
[   11.534179]  el1h_64_sync+0x64/0x68
