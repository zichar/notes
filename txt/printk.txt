//printk

_DEFINE_PRINTKRB(printk_rb_static, CONFIG_LOG_BUF_SHIFT - PRB_AVGBITS,
		PRB_AVGBITS, &__log_buf[0]); 


#define printk(fmt, ...) printk_index_wrap(_printk, fmt, ##__VA_ARGS__)

#define printk_index_wrap(_p_func, _fmt, ...)→  →       →       →       \ 
→       ({→     →       →       →       →       →       →       →       \
→       →       __printk_index_emit(_fmt, NULL, NULL);→ →       →       \
→       →       _p_func(_fmt, ##__VA_ARGS__);→  →       →       →       \
→       })  

asmlinkage __visible int _printk(const char *fmt, ...)
	va_start(args, fmt);
	vprintk(fmt, args);
		vprintk_default(fmt, args);
			vprintk_emit(0, LOGLEVEL_DEFAULT, NULL, fmt, args);
				vprintk_store(facility, level, dev_info, fmt, args);
					prb_reserve(&e, prb, &r);/prb_reserve_in_last(&e, prb, &r, caller_id, PRINTKRB_RECORD_MAX);
					prb_commit(&e);/prb_final_commit(&e); 				
				if (!in_sched)
					if (console_trylock_spining())
						console_unlock();
							do {
								console_flush_all(do_cond_resched, &next_seq, &handover);
							} while (prb_read_valid(prb, next_seq, NULL) && console_trylock()); 
	va_end(args);


static bool console_flush_all(bool do_cond_resched, u64 *next_seq, bool *handover)
	do {
		for_each_console_srcu(con) {
			progress = console_emit_next_record(con, handover, cookie);
				printk_get_next_message(&pmsg, con->seq, is_extended, true)
				con->write(con, outbuf, pmsg.outbuf_len);
		}
	} while (any_progress);