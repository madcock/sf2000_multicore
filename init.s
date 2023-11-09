.set noreorder
.globl __start
__start:
	j	load_and_run_core	# 0x800016d0
	nop
	j	hook_sys_watchdog_reboot	# 0x800016d8
	nop
# 0x800016e0
	mfc0	$ra, $14	# EPC
# curiously enough this core actually supports ehb but we won't use it atm
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	j	hook_exception_handler
	srl	$a0, $k1, 2	# masked Cause from INT_General_Exception_Hdlr