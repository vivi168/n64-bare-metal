.set noreorder
.section .text.entry, "ax", @progbits
.global _start
.type _start, @function

_start:
	/* PIF init */
	lui $a0, 0xBFC0
	addiu $t0, $zero, 8
	sw $t0, 0x07FC($a0)

	/* set top of RDRAM (0x80400000) as stack base */
	lui $sp, 0x8040
	addiu $sp, $sp, -16

	/* set  global pointer */
	la $gp, __gp

	/* clear bss */
	lui $t0, %hi(__bss_start)
	addiu $t0, $t0, %lo(__bss_start)
	lui $t1, %hi(__bss_end)
	addiu $t1, $t1, %lo(__bss_end)

	/* uncached addresses (via KSEG1) */
	lui $t2, 0xA000
	or  $t0, $t0, $t2
	or  $t1, $t1, $t2
clear_bss:
	beq $t0, $t1, bss_cleared
	addiu $t0, $t0, 4 /* delay slot */
	sw $zero, -4($t0)
	b clear_bss
	nop

bss_cleared:
	/*
	FIXME: BOOTCODE.BIN IPL3 already copies the first 1MiB from the ROM
	(so for now we an avoid copying .data/.sdata manually)
	*/

	jal main
	nop

infinite:
	b infinite
	nop

.size _start, . - _start
