# reboot





SYSCALL_DEFINE4(reboot, int, magic1, int, magic2, unsigned int, cmd, void __user *, arg)
	switch (cmd) {
		case LINUX_REBOOT_CMD_RESTART:
			kernel_restart(NULL);
			break;
		case LINUX_REBOOT_CMD_HALT:
			kernel_halt();
			do_exit(0);
		case LINUX_REBOOT_CMD_POWER_OFF:
			kernel_power_off();
			do_exit(0)
			break;
		case LINUX_REBOOT_CMD_RESTART2:
			strncpy_from_user(&buffer[0], arg, sizeof(buffer) - 1);
			kernel_restart(buffer);
			break;
		
	}

kernel_restart() / kernel_halt() / kernel_power_off()
    ↓
kernel_restart_prepare()  // 准备重启
    ↓
migrate_to_reboot_cpu()   // 迁移到重启CPU
    ↓
syscore_shutdown()        // 关闭系统核心组件
    ↓
device_shutdown()         // 关闭所有设备
    ↓
kmsg_dump(KMSG_DUMP_SHUTDOWN)  // ★ 转储内核日志
    ↓
machine_restart() / machine_halt() / machine_power_off()  // 实际重启/关机

kernel_restart(char *cmd)
	kernel_restart_prepare(cmd);
		blocking_notifier_call_chain(&reboot_notifier_list, SYS_RESTART, cmd);
		system_state = SYSTEM_RESTART;
		usermodehelper_disable();
		device_shutdown();
	do_kernel_restart_prepare();
		ablocking_notifier_call_chain(&restart_prep_handler_list, 0, NULL);
	migrate_to_reboot_cpu();
	syscore_shutdown();
		list_for_each_entry_reverse(ops, &syscore_ops_list, node)
			ops->shutdown();
	kmsg_dump(KMSG_DUMP_SHUTDOWN);
	machine_restart(cmd);



machine_restart(cmd);
	local_irq_disable();
	smp_send_stop();
	if (efi_enabled(EFI_RUNTIME_SERVICES))
		efi_reboot(reboot_mode, NULL);
	do_kernel_restart(cmd);
		atomic_notifier_call_chain(&restart_handler_list, reboot_mode, cmd);
			psci_sys_reset(nb, action, data);//register_restart_handler(&psci_sys_reset_nb);
				invoke_psci_fn(PSCI_0_2_FN_SYSTEM_RESET, 0, 0, 0);
	printk("Reboot failed -- System halted\n");
	while (1);























