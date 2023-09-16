# FIXME it's currently a really quick hack
.set noreorder
.globl __start
__start:
	j	retro_init	# 0x87000000
	nop	# branch delay slot
	j	retro_deinit	# 0x87000008
	nop
	j	retro_get_system_av_info	# 0x87000010
	nop
	j	retro_run	# 0x87000018
	nop
	j	retro_load_game	# 0x87000020
	nop
	j	retro_get_region	# 0x87000028
	nop
	j	retro_set_environment	# 0x87000030
	nop
	j	retro_set_video_refresh	# 0x87000038
	nop
	j	retro_set_audio_sample_batch	# 0x87000040
	nop
	j	retro_set_input_poll	# 0x87000048
	nop
	j	retro_set_input_state	# 0x87000050
	nop
# 0x87000058
	la	$a0, __bss_start
	la	$a1, _end
clear_bss:
	sw	$0, 0($a0)
	sw	$0, 4($a0)
	sw	$0, 8($a0)
	sw	$0, 12($a0)
	addiu	$a0, 16
	sltu	$v0, $a0, $a1
	bne	$v0, $0, clear_bss
	nop
	jr	$ra
	nop
