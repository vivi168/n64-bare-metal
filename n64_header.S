/* Adapted from Peter Lemon N64_HEADER.ASM */

.section .n64_header, "a", @progbits
.balign 4
.global n64_header
n64_header:
	.word 0x80371240             /* PI BSB domain 1 timing */
	.word 0x0000000F             /* clock rate */
	.word _start                 /* IPL3 entry point */
	.word 0x00000000             /* release */
	.word 0x00000000             /* CRC1; patched by romfix.py */
	.word 0x00000000             /* CRC2; patched by romfix.py */
	.space 8
	.ascii "issourire           " /* 20-byte internal name */
	.word 0x00000000              /* cartridge metadata */
	.word 0x00000000
	.word 0x00000000

.section .n64_boot, "a", @progbits
.incbin "BOOTCODE.BIN"
