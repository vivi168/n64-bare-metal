.set noreorder
.section .text.entry, "ax", @progbits
.global _start
.type _start, @function

_start:
	/* PIF */
	lui $a0, 0xBFC0
	addiu $t0, $zero, 8
	sw $t0, 0x07FC($a0)

	/* set top of RDRAM as stack base */
	lui $sp, 0x8040
	addiu $sp, $sp, -16

	/* clear bss */
	lui $t0, %hi(__bss_start)
	addiu $t0, $t0, %lo(__bss_start)
	lui $t1, %hi(__bss_end)
	addiu $t1, $t1, %lo(__bss_end)
clear_bss:
	beq $t0, $t1, bss_cleared
	nop
	sw $zero, 0($t0)
	addiu $t0, $t0, 4
	b clear_bss
	nop

bss_cleared:
	jal main
	nop

infinite:
	b infinite
	nop

.size _start, . - _start
