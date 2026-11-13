#include <stdint.h>

#define SCREEN_W 320
#define SCREEN_H 240

#define ISSOU 0xA0001000

#define VI_BASE 0xA4400000
#define VI_STATUS (VI_BASE)
#define VI_ORIGIN (VI_BASE + 0x04)
#define VI_WIDTH  (VI_BASE + 0x08)
#define VI_V_INTR (VI_BASE + 0x0C)
#define VI_V_CURRENT_LINE (VI_BASE + 0x10)
#define VI_TIMING (VI_BASE + 0x14)
#define VI_V_SYNC (VI_BASE + 0x18)
#define VI_H_SYNC (VI_BASE + 0x1C)
#define VI_H_SYNC_LEAP (VI_BASE + 0x20)
#define VI_H_VIDEO (VI_BASE + 0x24)
#define VI_V_VIDEO (VI_BASE + 0x28)
#define VI_V_BURST (VI_BASE + 0x2C)
#define VI_X_SCALE (VI_BASE + 0x30)
#define VI_Y_SCALE (VI_BASE + 0x34)

#define VI_CTL_TYPE_32BPP (3 << 0)
#define VI_CTL_TYPE_16BPP (2 << 0)
#define VI_CTL_TYPE_NONE  (0 << 0)
#define VI_CTL_GAMMA_DITHER (1 << 2)
#define VI_CTL_GAMMA (1 << 3)
#define VI_CTL_DIVOT (1 << 4)
#define VI_CTL_SERRATE (1 << 6)
#define VI_CTL_AA_MODE_AA_ALWAYS (0 << 8)
#define VI_CTL_AA_MODE_AA_NEEDED (1 << 8)
#define VI_CTL_AA_MODE_RESAMPLE  (2 << 8)
#define VI_CTL_AA_MODE_REPLICATE (3 << 8)
#define VI_CTL_PIXEL_ADVANCE_DEFAULT (3 << 12)
#define VI_CTL_DEDITHER_ENABLE (1 << 16)

#define DPC_BASE 0xA4100000
#define DPC_START     (DPC_BASE)
#define DPC_END       (DPC_BASE + 0x04)
#define DPC_CURRENT   (DPC_BASE + 0x08)
#define DPC_STATUS    (DPC_BASE + 0x0C)
#define DPC_CLOCK     (DPC_BASE + 0x10)
#define DPC_BUF_BUSY  (DPC_BASE + 0x14)
#define DPC_PIPE_BUSY (DPC_BASE + 0x18)
#define DPC_TMEM_BUSY (DPC_BASE + 0x1C)

#define PHYSADDR(a) ((uint32_t)(a) & 0x1fffffff)

void wait_scanline(uint32_t v)
{
	while (*(volatile uint32_t*)VI_V_CURRENT_LINE != v) {}
}

static void vi_init(void)
{
	uint32_t status =
		VI_CTL_PIXEL_ADVANCE_DEFAULT |
		VI_CTL_AA_MODE_REPLICATE |
		VI_CTL_DIVOT |
		VI_CTL_GAMMA |
		VI_CTL_GAMMA_DITHER |
		VI_CTL_TYPE_16BPP;

	*(volatile uint32_t*)VI_STATUS = status;
	// FIXME: for now just blindly copied values from Peter Lemon HelloWorld example
	*(volatile uint32_t*)VI_ORIGIN = 0x00100000;
	*(volatile uint32_t*)VI_WIDTH = SCREEN_W;
	*(volatile uint32_t*)VI_V_INTR = 0x200;
	*(volatile uint32_t*)VI_V_CURRENT_LINE = 0x0;
	*(volatile uint32_t*)VI_TIMING = 0x3E52239;
	*(volatile uint32_t*)VI_V_SYNC = 0x20D;
	*(volatile uint32_t*)VI_H_SYNC = 0xC15;
	*(volatile uint32_t*)VI_H_SYNC_LEAP = 0xC150C15;
	*(volatile uint32_t*)VI_H_VIDEO = 0x6C02EC;
	*(volatile uint32_t*)VI_V_VIDEO = 0x2501FF;
	*(volatile uint32_t*)VI_V_BURST = 0xE0204;
	*(volatile uint32_t*)VI_X_SCALE = 256 * SCREEN_W / 160;
	*(volatile uint32_t*)VI_Y_SCALE = 256 * SCREEN_H / 60;

}

void main(void)
{
	vi_init();
	wait_scanline(0x200);

	// hardcode some RDP commands
	{
		static _Alignas(8) uint32_t command_buffer[] = {
			0x2D000000, 0x005003c0, // Set_Scissor
			0x2f300000, 0x00000000, // Set_Other_Modes
			0x3f10013f, 0x00100000, // Set_Color_Image
			0x37000000, 0xf83ff83f, // Set_Fill_Color
			0x364fc3bc, 0x00000000, // Fill_Rectangle
			0x29000000, 0x00000000, // Sync_Full
		};

		*(volatile uint32_t*)DPC_START = PHYSADDR(command_buffer);
		*(volatile uint32_t*)DPC_END = PHYSADDR(command_buffer) + sizeof(command_buffer);
	}

	*(volatile uint32_t*)ISSOU = 0x155E;

	for (;;) {}
}
