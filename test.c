#include <stdint.h>

#define SCREEN_W 320
#define SCREEN_H 240

#define ISSOU 0xA0001000

#define VI_BASE 0xA4400000 // Video Interface (VI) Base Register
#define VI_STATUS (VI_BASE + 0x00)         // VI: Status/Control Register
#define VI_ORIGIN (VI_BASE + 0x04)         // VI: Origin Register
#define VI_WIDTH  (VI_BASE + 0x08)         // VI: Width Register
#define VI_V_INTR (VI_BASE + 0x0C)         // VI: Vertical Interrupt Register
#define VI_V_CURRENT_LINE (VI_BASE + 0x10) // VI: Current Vertical Line Register
#define VI_TIMING (VI_BASE + 0x14)         // VI: Video Timing Register
#define VI_V_SYNC (VI_BASE + 0x18)         // VI: Vertical Sync Register
#define VI_H_SYNC (VI_BASE + 0x1C)         // VI: Horizontal Sync Register
#define VI_H_SYNC_LEAP (VI_BASE + 0x20)    // VI: Horizontal Sync Leap Register
#define VI_H_VIDEO (VI_BASE + 0x24)        // VI: Horizontal Video Register
#define VI_V_VIDEO (VI_BASE + 0x28)        // VI: Vertical Video Register
#define VI_V_BURST (VI_BASE + 0x2C)        // VI: Vertical Burst Register
#define VI_X_SCALE (VI_BASE + 0x30)        // VI: X-Scale Register
#define VI_Y_SCALE (VI_BASE + 0x34)        // VI: Y-Scale Register

#define VI_CTL_TYPE_16BPP (3 << 0)
#define VI_CTL_TYPE_32BPP (2 << 0)
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
	*(volatile uint32_t*)VI_ORIGIN = 0xA0100000;
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
	*(volatile uint32_t*)ISSOU = 0x155E;

	vi_init();

	for (;;) {}
}
