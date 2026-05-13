# QEMU + UEFI 启动 Linux 内核（fw_cfg 路径）

本文整理 **QEMU `virt` + `-bios`（EDK II / ArmVirtQemu）+ `-kernel`** 时，内核如何进入系统，以及 **UEFI 内部从 `fw_cfg` 取内核的代码流程**。

---

## 1. QEMU 侧：两种路径的分叉

源码参考：`hw/arm/boot.c` 中 `arm_load_kernel()`。

```c
/* Load the kernel.  */
if (!info->kernel_filename || info->firmware_loaded) {
    arm_setup_firmware_boot(cpu, info);
} else {
    arm_setup_direct_kernel_boot(cpu, info);
}
```

| 条件 | 行为 |
|------|------|
| **未指定 `-kernel`**，或逻辑上等价 | `arm_setup_firmware_boot`：纯固件启动（可从磁盘 ESP 等继续）。 |
| **已加载固件**（`firmware_loaded == true`）且 **指定了 `-kernel`** | **仍走** `arm_setup_firmware_boot`，**不会**走 `arm_setup_direct_kernel_boot`。 |
| **指定 `-kernel` 且未加载固件** | `arm_setup_direct_kernel_boot`：QEMU 在 RAM 里放引导桩，按 Linux arm64 约定直接进内核（**不跑 UEFI**）。 |

### 1.1 固件启动 + `-kernel` 时 QEMU 做什么

在 `arm_setup_firmware_boot()` 中，若存在 `kernel_filename` 且机器提供 `fw_cfg`，QEMU 将 **内核、initrd、命令行** 注册进 **fw_cfg**（如 `FW_CFG_KERNEL_SIZE` / `FW_CFG_KERNEL_DATA` 等），交给 **固件**消费，而不是在 QEMU 里解析 `Image` 并写 direct-boot 桩。

---

## 2. UEFI 内部总流程（ArmVirtQemu）

```text
QEMU 将 -kernel / -initrd / -append 写入 fw_cfg
    ↓
DXE：QemuKernelLoaderFsDxe
      通过 QemuFwCfgLib（MMIO）读 fw_cfg，
      实现 SimpleFileSystem：虚拟文件 kernel、initrd、cmdline
    ↓
BDS：PlatformBootManagerAfterConsole()
      → TryRunningQemuKernel()
    ↓
QemuLoadKernelImage()（GenericQemuLoadImageLib）
      gBS->LoadImage(..., 设备路径指向上述卷上的 L"kernel")
      打开同卷的 cmdline、initrd，构造 LoadedImage->LoadOptions
    ↓
QemuStartKernelImage()
      gBS->StartImage() → 进入带 EFI stub 的内核
```

**要点**：不是从磁盘分区读 `Image`，而是 **EFI `LoadImage` + 合成文件系统**，底层读 **fw_cfg**。

---

## 3. 平台与模块（EDK II）

- **平台 DSC**：`ArmVirtPkg/ArmVirtQemu.dsc`（或 `ArmVirtQemuKernel.dsc` 等变体）。
- **fw_cfg 访问库**：`QemuFwCfgLib` → 如 `OvmfPkg/Library/QemuFwCfgLib/QemuFwCfgMmioDxeLib.inf`。
- **虚拟文件系统驱动**：`OvmfPkg/QemuKernelLoaderFsDxe/QemuKernelLoaderFsDxe.inf`。
- **BDS / 平台引导**：`OvmfPkg/Library/PlatformBootManagerLibLight/PlatformBootManagerLib.inf`（含 `QemuKernel.c`、`PlatformBm.c`）。
- **加载内核逻辑库**：`OvmfPkg/Library/GenericQemuLoadImageLib/GenericQemuLoadImageLib.inf`（`QemuLoadKernelImage` / `QemuStartKernelImage`）。

---

## 4. `QemuKernelLoaderFsDxe`：fw_cfg → 文件名

`mKernelBlobItems[]` 将逻辑文件名映射到 fw_cfg 项（示意）：

| 虚拟文件 | fw_cfg 用途 |
|----------|-------------|
| `kernel` | `KernelSetup` / `Kernel` 的 size + data |
| `initrd` | `Initrd` size + data |
| `cmdline` | `CommandLine` size + data |

实现文件：`OvmfPkg/QemuKernelLoaderFsDxe/QemuKernelLoaderFsDxe.c`  
读数据：`QemuFwCfgLib`（`QemuFwCfgSelectItem`、`QemuFwCfgReadBytes` 等）。

---

## 5. BDS 入口：`TryRunningQemuKernel`

在 `PlatformBootManagerAfterConsole()` 中（`PlatformBm.c`），在连接设备、刷新启动项之前，**优先处理 QEMU `-kernel`**：

- 注释说明：此路径启动的内核可获得 **ACPI**（因在 `PlatformBootManagerBeforeConsole` 已连接 PCI 并触发 ACPI 平台驱动）。
- 调用：`TryRunningQemuKernel()`（`QemuKernel.c`）。

`TryRunningQemuKernel` 步骤简述：

1. `QemuLoadKernelImage(&KernelImageHandle)`
2. `EfiSignalEventReadyToBoot` / `REPORT_STATUS_CODE`
3. `QemuStartKernelImage(&KernelImageHandle)`
4. 失败则 `QemuUnloadKernelImage`

---

## 6. `QemuLoadKernelImage`：EFI 路径与 `LoadImage`

实现：`OvmfPkg/Library/GenericQemuLoadImageLib/GenericQemuLoadImageLib.c`。

1. 使用 **Vendor Media GUID** `QEMU_KERNEL_LOADER_FS_MEDIA_GUID` + 文件路径 **`L"kernel"`**（或先尝试 **`L"shim"`**）调用 **`gBS->LoadImage`**。
2. `LoadImage` 会触发 **QemuKernelLoaderFsDxe** 从 **fw_cfg** 读出内核字节并作为 PE/EFI 镜像加载。
3. 通过 **`LocateDevicePath` + `SimpleFileSystem`** 打开同一抽象卷，读取 **`cmdline`**、**`initrd`**，设置 **`EFI_LOADED_IMAGE_PROTOCOL`** 的 **`LoadOptions`**（含 `initrd=initrd` 等 UTF-16 前缀，视是否 shim 而定）。

---

## 7. 与「磁盘 ESP 启动」的对比

| 方式 | 数据从哪来 |
|------|------------|
| **`-kernel` + UEFI** | **`fw_cfg`** → **QemuKernelLoaderFsDxe** → **`TryRunningQemuKernel`**。 |
| **仅磁盘 / ISO** | **Boot Manager** → ESP 上 **`BOOTAA64.EFI`** 或 GRUB 等，**不经过**上述 `TryRunningQemuKernel`（除非额外配置）。 |

---

## 8. 前置条件（实现约定）

- **ArmVirt「轻量」路径**要求内核为 **带 EFI stub 的镜像**（如 Linux **`CONFIG_EFI_STUB`** 的 `Image`）。`QemuKernel.c` 头文件注释明确：**EFI stub 为硬前提**。
- 若仅 **`qemu-system-aarch64 -kernel Image`** **不带** `-bios`，则走 QEMU **direct kernel boot**，**不执行**上述 UEFI 流程。

---

## 9. 参考路径（可按本机检出位置调整）

- **QEMU 源码树**：`hw/arm/boot.c` — `arm_load_kernel`、`arm_setup_firmware_boot`、`arm_setup_direct_kernel_boot`
- EDK2：`ArmVirtPkg/ArmVirtQemu.dsc`
- EDK2：`OvmfPkg/QemuKernelLoaderFsDxe/QemuKernelLoaderFsDxe.c`
- EDK2：`OvmfPkg/Library/PlatformBootManagerLibLight/PlatformBm.c`
- EDK2：`OvmfPkg/Library/PlatformBootManagerLibLight/QemuKernel.c`
- EDK2：`OvmfPkg/Library/GenericQemuLoadImageLib/GenericQemuLoadImageLib.c`

---

## 10. 修订说明

- 文档内容来自对 **QEMU `arm_load_kernel` 逻辑**与 **EDK II ArmVirtQemu + OvmfPkg QemuKernelLoader 系列模块**的梳理。
- 不同 **EDK2 / QEMU 版本** 下 DSC 组件名、PCD 或注释可能略有差异，以当前检出源码为准。
