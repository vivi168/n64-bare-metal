.set noreorder

.section .ivt, "ax", @progbits
    .org 0x0   /* TLB miss */
    j inthandler

    .org 0x80   /* XTLB miss */
    j inthandler

    .org 0x100   /* ECC exception */
    j inthandler

    .org 0x180   /* General exception */
    j inthandler

    .balign 16

#define CP0_STATUS $12

.section .text, "ax", @progbits

.global init_interrupts
.type init_interrupts, @function

/* doesn't enable interrupt, must call enable_interrupts */
init_interrupts:
    mfc0 $t0, CP0_STATUS
    ori $t0, $t0, 0x0400 /* set C0_INTERRUPT_RCP */
    ori $t0, $t0, 0x1000 /* set C0_INTERRUPT_PRENMI (PRENMI is triggered when pressing RESET) */
    jr $ra
    mtc0 $t0, CP0_STATUS /* delay slot */

.global enable_interrupts
.type enable_interrupts, @function

enable_interrupts:
    mfc0 $t0, CP0_STATUS
    ori $t0, $t0, 1 /* set C0_STATUS_IE */
    jr $ra
    mtc0 $t0, CP0_STATUS /* delay slot */

.global disable_interrupts
.type disable_interrupts, @function

disable_interrupts:
    mfc0 $t0, CP0_STATUS
    srl $t0, 1
    sll $t0, 1 /* unset C0_STATUS_IE */
    jr $ra
    mtc0 $t0, CP0_STATUS /* delay slot */

.global inthandler
.type inthandler, @function

inthandler:
    /* FIXME: just a infinite loop for now */
    li $k0, 0xdeadc0de
infinite:
	b infinite
	nop
