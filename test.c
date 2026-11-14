#include <stdint.h>

#define SCREEN_W 320
#define SCREEN_H 240

#define PHYSADDR(a) ((uint32_t)(a) & 0X1FFFFFFF)

#define REG32(a) (*(volatile uint32_t *)(a))

#define ISSOU 0xA0001000

#define VI_BASE           0xA4400000
#define VI_STATUS         REG32(VI_BASE)
#define VI_ORIGIN         REG32(VI_BASE + 0x04)
#define VI_WIDTH          REG32(VI_BASE + 0x08)
#define VI_V_INTR         REG32(VI_BASE + 0x0C)
#define VI_V_CURRENT_LINE REG32(VI_BASE + 0x10)
#define VI_TIMING         REG32(VI_BASE + 0x14)
#define VI_V_SYNC         REG32(VI_BASE + 0x18)
#define VI_H_SYNC         REG32(VI_BASE + 0x1C)
#define VI_H_SYNC_LEAP    REG32(VI_BASE + 0x20)
#define VI_H_VIDEO        REG32(VI_BASE + 0x24)
#define VI_V_VIDEO        REG32(VI_BASE + 0x28)
#define VI_V_BURST        REG32(VI_BASE + 0x2C)
#define VI_X_SCALE        REG32(VI_BASE + 0x30)
#define VI_Y_SCALE        REG32(VI_BASE + 0x34)

#define VI_CTL_TYPE_32BPP            (3 << 0)
#define VI_CTL_TYPE_16BPP            (2 << 0)
#define VI_CTL_TYPE_NONE             (0 << 0)
#define VI_CTL_GAMMA_DITHER          (1 << 2)
#define VI_CTL_GAMMA                 (1 << 3)
#define VI_CTL_DIVOT                 (1 << 4)
#define VI_CTL_SERRATE               (1 << 6)
#define VI_CTL_AA_MODE_AA_ALWAYS     (0 << 8)
#define VI_CTL_AA_MODE_AA_NEEDED     (1 << 8)
#define VI_CTL_AA_MODE_RESAMPLE      (2 << 8)
#define VI_CTL_AA_MODE_REPLICATE     (3 << 8)
#define VI_CTL_PIXEL_ADVANCE_DEFAULT (3 << 12)
#define VI_CTL_DEDITHER_ENABLE       (1 << 16)

#define DPC_BASE      0xA4100000
#define DPC_START     REG32(DPC_BASE)
#define DPC_END       REG32(DPC_BASE + 0x04)
#define DPC_CURRENT   REG32(DPC_BASE + 0x08)
#define DPC_STATUS    REG32(DPC_BASE + 0x0C)
#define DPC_CLOCK     REG32(DPC_BASE + 0x10)
#define DPC_BUF_BUSY  REG32(DPC_BASE + 0x14)
#define DPC_PIPE_BUSY REG32(DPC_BASE + 0x18)
#define DPC_TMEM_BUSY REG32(DPC_BASE + 0x1C)

void wait_scanline(uint32_t v)
{
	while (VI_V_CURRENT_LINE != v) {}
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

	VI_STATUS = status;
	// FIXME: for now just blindly copied values from Peter Lemon HelloWorld example
	VI_ORIGIN = 0x00100000;
	VI_WIDTH = SCREEN_W;
	VI_V_INTR = 0x200;
	VI_V_CURRENT_LINE = 0x0;
	VI_TIMING = 0x3E52239;
	VI_V_SYNC = 0x20D;
	VI_H_SYNC = 0xC15;
	VI_H_SYNC_LEAP = 0xC150C15;
	VI_H_VIDEO = 0x6C02EC;
	VI_V_VIDEO = 0x2501FF;
	VI_V_BURST = 0xE0204;
	VI_X_SCALE = 256 * SCREEN_W / 160;
	VI_Y_SCALE = 256 * SCREEN_H / 60;
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

		DPC_START = PHYSADDR(command_buffer);
		DPC_END = PHYSADDR(command_buffer) + sizeof(command_buffer);
	}

	*(volatile uint32_t*)ISSOU = 0x155E;

	for (;;) {}
}
