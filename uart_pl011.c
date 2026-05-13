


pl011_probe(struct amba_device *dev, const struct amba_id *id)
	uap = devm_kzalloc(&dev->dev, sizeof(struct uart_amba_port), GFP_KERNEL);
	uap->port.ops = &amba_pl011_pops;
	uap->port.rs485_config = pl011_rs485_config;
	pl011_setup_port(&dev->dev, uap, &dev->res, portnr);
		uap->port.dev = dev;
		uap->port.line = index;
		amba_ports[index] = uap;
	pl011_register_port(uap);
		if (!amba_reg.state)  //struct uart_driver drv = amba_reg
			uart_register_driver(&amba_reg);				
		uart_add_one_port(&amba_reg, &uap->port);
		

uart_register_driver(&amba_reg); //struct uart_driver drv = amba_reg
	drv->state = kcalloc(drv->nr, sizeof(struct uart_state), GFP_KERNEL);
	normal = tty_alloc_driver(drv->nr, TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV);
	drv->tty_driver = normal;
	normal->type→   →       = TTY_DRIVER_TYPE_SERIAL;
	normal->subtype→→       = SERIAL_TYPE_NORMAL;
	normal->driver_state    = drv;
	normal->init_termios→   = tty_std_termios;
	tty_set_operations(normal, &uart_ops);
	for (i = 0; i < drv->nr; i++)
		struct uart_state *state = drv->state + i;
		struct tty_port *port = &state->port;
		tty_port_init(port);
		port->ops = &uart_port_ops;
	tty_register_driver(normal);
		// chrdev
		list_add(&driver->tty_drivers, &tty_drivers);
		proc_tty_register_driver(driver);
	
uart_add_one_port(&amba_reg, &uap->port);
	serial_ctrl_register_port(drv, port);
		serial_core_register_port(drv, port);
			ctrl_dev = serial_core_ctrl_find(drv, port->dev, port->ctrl_id);
				if (!ctrl_dev) serial_core_ctrl_device_add(port);
					serial_base_ctrl_add(port, port->dev);
						ctrl_dev = kzalloc(sizeof(*ctrl_dev), GFP_KERNEL);
						ida_init(&ctrl_dev->port_ida);
						serial_base_device_init(port, &ctrl_dev->dev, parent,
								&serial_ctrl_type, serial_base_ctrl_release, port->ctrl_id, 0);
							device_initialize(dev);
							dev->type = type;
							dev->parent = parent;  // ctrl_dev->dev->parent = port->dev
							dev->bus = &serial_base_bus_type;
						device_add(&ctrl_dev->dev);
			serial_core_port_device_add(ctrl_dev, port);
				port->port_dev = serial_base_port_add(port, ctrl_dev); //struct serial_port_device
					port_dev = kzalloc(sizeof(*port_dev), GFP_KERNEL);
					serial_base_device_init(port, &port_dev->dev, &ctrl_dev->dev,  //port->port_dev->dev = ctrl_dev->dev
							&serial_port_type,  serial_base_port_release, port->ctrl_id, port->port_id); 
					device_add(&port_dev->dev);
			serial_core_add_one_port(drv, uport); //port == uport
				struct tty_port *port;
				state = drv->state + uport->line;
				port = &state->port;
				state->uart_port = uport;
				uport->state = state;
				uport->cons = drv->cons;
				uport->minor = drv->tty_driver->minor_start + uport->line;
				uport->name = kasprintf(GFP_KERNEL, "%s%d", drv->dev_name, drv->tty_driver->name_base + uport->line);
				tty_port_link_device(port, drv->tty_driver, uport->line);
					drv->driver->ports[index] = port;
				uart_configure_port(drv, state, uport); //console
					...
					register_console(port->cons);
				port->console = uart_console(uport); //((port)->cons && (port)->cons->index == (port)->line)
				uport->tty_groups = kcalloc(num_groups, sizeof(*uport->tty_groups), GFP_KERNEL);
				uport->tty_groups[0] = &tty_dev_attr_group;
				tty_dev = tty_port_register_device_attr_serdev(port, drv->tty_driver, uport->line, uport->dev, port, uport->tty_groups);
					tty_port_link_device(port, driver, index);
					serdev_tty_port_register(port, device, driver, index);
						ctrl = serdev_controller_alloc(parent, sizeof(struct serport));
						serdev_controller_add(ctrl);
					tty_register_device_attr(driver, index, device, drvdata, attr_grp);

void register_console(struct console *newcon)
	
	err = try_enable_preferred_console(newcon, true);
	if (err == -ENOENT)
		err = try_enable_preferred_console(newcon, false);
	console_init_seq(newcon, bootcon_registered);
	hlist_add_head_rcu(&newcon->node, &console_list);/hlist_add_behind_rcu(&newcon->node, console_list.first);
	console_sysfs_notify();
	con_printk(KERN_INFO, newcon, "enabled\n");





static int __init console_setup(char *str)			
__setup("console=", console_setup);



static struct uart_driver amba_reg;
static struct console amba_console = {
	.name		= "ttyAMA",
	.device		= uart_console_device,
	.setup		= pl011_console_setup,
	.match		= pl011_console_match,
	.write_atomic	= pl011_console_write_atomic,
	.write_thread	= pl011_console_write_thread,
	.device_lock	= pl011_console_device_lock,
	.device_unlock	= pl011_console_device_unlock,
	.flags		= CON_PRINTBUFFER | CON_ANYTIME | CON_NBCON,
	.index		= -1,
	.data		= &amba_reg,
};

#define AMBA_CONSOLE	(&amba_console)

static struct uart_driver amba_reg = {
	.owner			= THIS_MODULE,
	.driver_name		= "ttyAMA",
	.dev_name		= "ttyAMA",
	.major			= SERIAL_AMBA_MAJOR,
	.minor			= SERIAL_AMBA_MINOR,
	.nr			= UART_NR,
	.cons			= AMBA_CONSOLE,
};


static const struct uart_ops amba_pl011_pops = {
	.tx_empty	= pl011_tx_empty,
	.set_mctrl	= pl011_set_mctrl,
	.get_mctrl	= pl011_get_mctrl,
	.stop_tx	= pl011_stop_tx,
	.start_tx	= pl011_start_tx,
	.stop_rx	= pl011_stop_rx,
	.throttle	= pl011_throttle_rx,
	.unthrottle	= pl011_unthrottle_rx,
	.enable_ms	= pl011_enable_ms,
	.break_ctl	= pl011_break_ctl,
	.startup	= pl011_startup,
	.shutdown	= pl011_shutdown,
	.flush_buffer	= pl011_dma_flush_buffer,
	.set_termios	= pl011_set_termios,
	.type		= pl011_type,
	.config_port	= pl011_config_port,
	.verify_port	= pl011_verify_port,
#ifdef CONFIG_CONSOLE_POLL
	.poll_init     = pl011_hwinit,
	.poll_get_char = pl011_get_poll_char,
	.poll_put_char = pl011_put_poll_char,
#endif
};

static const struct uart_ops sbsa_uart_pops = {
	.tx_empty	= pl011_tx_empty,
	.set_mctrl	= sbsa_uart_set_mctrl,
	.get_mctrl	= sbsa_uart_get_mctrl,
	.stop_tx	= pl011_stop_tx,
	.start_tx	= pl011_start_tx,
	.stop_rx	= pl011_stop_rx,
	.startup	= sbsa_uart_startup,
	.shutdown	= sbsa_uart_shutdown,
	.set_termios	= sbsa_uart_set_termios,
	.type		= pl011_type,
	.config_port	= pl011_config_port,
	.verify_port	= pl011_verify_port,
#ifdef CONFIG_CONSOLE_POLL
	.poll_init     = pl011_hwinit,
	.poll_get_char = pl011_get_poll_char,
	.poll_put_char = pl011_put_poll_char,
#endif
};

static const struct tty_operations uart_ops = {
	.install	= uart_install,
	.open		= uart_open,
	.close		= uart_close,
	.write		= uart_write,
	.put_char	= uart_put_char,
	.flush_chars	= uart_flush_chars,
	.write_room	= uart_write_room,
	.chars_in_buffer= uart_chars_in_buffer,
	.flush_buffer	= uart_flush_buffer,
	.ioctl		= uart_ioctl,
	.throttle	= uart_throttle,
	.unthrottle	= uart_unthrottle,
	.send_xchar	= uart_send_xchar,
	.set_termios	= uart_set_termios,
	.set_ldisc	= uart_set_ldisc,
	.stop		= uart_stop,
	.start		= uart_start,
	.hangup		= uart_hangup,
	.break_ctl	= uart_break_ctl,
	.wait_until_sent= uart_wait_until_sent,
#ifdef CONFIG_PROC_FS
	.proc_show	= uart_proc_show,
#endif
	.tiocmget	= uart_tiocmget,
	.tiocmset	= uart_tiocmset,
	.set_serial	= uart_set_info_user,
	.get_serial	= uart_get_info_user,
	.get_icount	= uart_get_icount,
#ifdef CONFIG_CONSOLE_POLL
	.poll_init	= uart_poll_init,
	.poll_get_char	= uart_poll_get_char,
	.poll_put_char	= uart_poll_put_char,
#endif
};

static const struct tty_port_operations uart_port_ops = {
	.carrier_raised = uart_carrier_raised,
	.dtr_rts	= uart_dtr_rts,
	.activate	= uart_port_activate,
	.shutdown	= uart_tty_port_shutdown,
};


pics
https://www.cnblogs.com/yikoulinux/p/14507445.html