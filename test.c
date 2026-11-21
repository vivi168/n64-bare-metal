#include <stdint.h>
#include <stddef.h>

#define SCREEN_W 320
#define SCREEN_H 240

#define KSEG0_BASE 0x80000000
#define KSEG1_BASE 0xA0000000
#define PHYSADDR(a) ((uint32_t)(a) & 0x1FFFFFFF)
#define KSEG0ADDR(a) (PHYSADDR(a) | KSEG0_BASE)
#define KSEG1ADDR(a) (PHYSADDR(a) | KSEG1_BASE)

#define REG32(a) ((volatile uint32_t *)(a))

#define ISSOU 0xA00FFFF0

#define VI_BASE           0xA4400000
#define VI_STATUS         REG32(VI_BASE)
#define VI_ORIGIN         REG32(VI_BASE + 0x04)
#define VI_WIDTH          REG32(VI_BASE + 0x08)
#define VI_V_INTR         REG32(VI_BASE + 0x0C)
#define VI_V_CURRENT_LINE REG32(VI_BASE + 0x10)
#define VI_TIMING         REG32(VI_BASE + 0x14)
#define VI_V_TOTAL        REG32(VI_BASE + 0x18)
#define VI_H_TOTAL        REG32(VI_BASE + 0x1C)
#define VI_H_TOTAL_LEAP   REG32(VI_BASE + 0x20)
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

#define SI_BASE 0xA4800000
#define SI_DRAM_ADDR      REG32(SI_BASE)
#define SI_PIF_ADDR_RD64B REG32(SI_BASE + 0x04)
#define SI_PIF_ADDR_WR64B REG32(SI_BASE + 0x10)
#define SI_STATUS         REG32(SI_BASE + 0x18)

#define SI_DMA_BUSY  (1 << 0)
#define SI_IO_BUSY   (1 << 1)
#define SI_DMA_ERROR (1 << 3)

#define SI_CLEAR_INTERRUPT 0

#define PIF_BASE 0xBFC00000
#define PIF_RAM_START (PIF_BASE + 0x7C0)

#define MI_BASE 0xA4300000
#define MI_INTR REG32(MI_BASE + 0x08)
#define MI_MASK REG32(MI_BASE + 0x0C)

// Interrupt bits
#define MI_INTR_SP (1 << 0) // RSP Task done
#define MI_INTR_SI (1 << 1) // controller input available
#define MI_INTR_AI (1 << 2) // audio buffer swap
#define MI_INTR_VI (1 << 3) // vertical retrace
#define MI_INTR_PI (1 << 4) // ROM > RAM DMA done
#define MI_INTR_DP (1 << 5) // RDP done (Full_Sync)

// Cache related
#define I_CACHE          0
#define I_CACHE_SIZE     16384
#define I_CACHE_LINESIZE 32

#define D_CACHE          1
#define D_CACHE_SIZE     8192
#define D_CACHE_LINESIZE 16

#define INDEX_INVALIDATE         0
#define HIT_INVALIDATE           4
#define HIT_WRITEBACK_INVALIDATE 5
#define HIT_WRITEBACK            6

#define D_HIT_INVALIDATE (D_CACHE | (HIT_INVALIDATE << 2))
#define D_HIT_WRITEBACK (D_CACHE | (HIT_WRITEBACK << 2))
#define D_INDEX_WRITEBACK_INVALIDATE (D_CACHE | (INDEX_WRITEBACK_INVALIDATE << 2))
#define D_HIT_WRITEBACK_INVALIDATE (D_CACHE | (HIT_WRITEBACK_INVALIDATE << 2))

#define I_INDEX_INVALIDATE (I_CACHE | (INDEX_INVALIDATE << 2))
#define I_HIT_INVALIDATE (I_CACHE | (HIT_INVALIDATE << 2))

#define CACHE_OP(op, addr) asm volatile("cache %0, 0(%1)" :: "i"(op), "r"(addr) : "memory")

void invalid_i_cache(void *addr, uint32_t nbytes)
{
	if (nbytes == 0) {
		return;
	}

	// trash everything if nbytes is big
	if (nbytes >= I_CACHE_SIZE) {
		for (uint32_t a = KSEG0_BASE; a < KSEG0_BASE + I_CACHE_SIZE; a += I_CACHE_LINESIZE) {
			CACHE_OP(I_INDEX_INVALIDATE, a);
		}

		return;
	}

	uint32_t vaddr = KSEG0ADDR(addr);
	uint32_t start = vaddr & ~(I_CACHE_LINESIZE - 1); // align down to cache line size
	uint32_t end = (vaddr + nbytes + I_CACHE_LINESIZE - 1) & ~(I_CACHE_LINESIZE - 1); // align up to cache line size

	for (uint32_t a = start; a < end; a += I_CACHE_LINESIZE) {
		CACHE_OP(I_HIT_INVALIDATE, a);
	}
}

static _Alignas(8) uint32_t PIF_Init[] = {
	0xFF010401, 0, // controller 1
	0, 0,          // controller 2
	0, 0,          // controller 3
	0, 0,          // controller 4
	0xFE000000, 0, // controller end
	0, 0,
	0, 0,
	0, 1,          // end with 1 = status register
};

static _Alignas(8) uint32_t PIF_JoyState[16];

struct JoyState
{
    uint32_t a : 1;
    uint32_t b : 1;
    uint32_t z : 1;
    uint32_t start : 1;
    uint32_t d_up : 1;
    uint32_t d_down: 1;
    uint32_t d_left : 1;
    uint32_t d_right : 1;

    uint32_t reset : 1;
    uint32_t unused : 1;
    uint32_t l : 1;
    uint32_t r : 1;
    uint32_t c_up : 1;
    uint32_t c_down: 1;
    uint32_t c_left : 1;
    uint32_t c_right : 1;

    int32_t x_axis : 8;
    int32_t y_axis : 8;
};

void *memcpy(void *d, const void *s, size_t n)
{
	uint8_t *dest = (uint8_t *)d;
	const uint8_t *src = (const uint8_t *)s;

	while (n--) {
		*dest++ = *src++;
	}

	return dest;
}

extern void init_interrupts(void);
extern void enable_interrupts(void);
extern void disable_interrupts(void);

extern uint8_t __ivt_start[], __ivt_end[], __ivt_ram_start[];

static void install_interrupts(void)
{
	void *start = (void *)KSEG1ADDR(__ivt_ram_start);

	uint32_t ivt_size = __ivt_end - __ivt_start;

	memcpy(start, __ivt_start, ivt_size);

	invalid_i_cache(start, ivt_size);
}

static void si_wait(void)
{
    while (*SI_STATUS & (SI_DMA_BUSY | SI_IO_BUSY)) {}

    *SI_STATUS = SI_CLEAR_INTERRUPT;
}

void init_joypad(void)
{
	*SI_DRAM_ADDR = PHYSADDR(PIF_Init);
	*SI_PIF_ADDR_WR64B = PHYSADDR(PIF_RAM_START);

	si_wait();
}

void poll_joypad(void)
{
	*SI_DRAM_ADDR = PHYSADDR(PIF_JoyState);
	*SI_PIF_ADDR_RD64B = PHYSADDR(PIF_RAM_START);

	si_wait();
}

void wait_scanline(uint32_t v)
{
	while (*VI_V_CURRENT_LINE != v) {}
}

static void wait_vblank(void)
{
	while (!(*MI_INTR & MI_INTR_VI)) {}

	*VI_V_CURRENT_LINE = 0; // acknowledge by writing to it
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

	*VI_STATUS = status;
	*VI_ORIGIN = 0xA0100000;
	*VI_WIDTH = SCREEN_W;
	*VI_V_INTR = 2;
	*VI_V_CURRENT_LINE = 0;
	*VI_TIMING = 0x3E52239;
	*VI_V_TOTAL = 0x20D; // V_TOTAL=525, for non-interlaced NTSC
	*VI_H_TOTAL = 0xC15; // H_TOTAL=3093 for NTSC
	*VI_H_TOTAL_LEAP = 0xC150C15;
	*VI_H_VIDEO = 0x6C02EC; // H_START=108, H_END=748 (typical NTSC values)
	*VI_V_VIDEO = 0x2501FF; // V_START=37, V_END=511 (typical NTSC values)
	*VI_V_BURST = 0xE0204; // V_BURST_START=14, V_BURST_END=516 (typical NTSC values)
	*VI_X_SCALE = 512; // X_SCALE=0.5 (2.10 format)
	*VI_Y_SCALE = 1024; // Y_SCALE=1 (2.10 format)
}

void RDP_FillRectangle(uint32_t* cmd_list_dest, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	// top left corner, u10.2
	uint32_t xh = (x << 2) & 0xfff;
	uint32_t yh = (y << 2) & 0xfff;
	// top right corner, u10.2
	uint32_t xl = ((x + width) << 2) & 0xfff;
	uint32_t yl = ((y + height) << 2) & 0xfff;

	uint32_t hi = (0x36 << 24) | (xl << 12) | yl;
	uint32_t lo = (xh << 12) | yh;

	volatile uint32_t *p = (volatile uint32_t *)KSEG1ADDR(cmd_list_dest);

	p[0] = hi;
	p[1] = lo;
}

// FPU test
float ronre(float i, float j) {
	return i * j;
}

void main(void)
{
	install_interrupts();
	init_interrupts();

	si_wait(); // acknowledge the interrupt emited by PIF initialize in boot.S
	vi_init();
	init_joypad();

	*(volatile uint32_t*)ISSOU = sizeof(struct JoyState);

	int32_t rx = SCREEN_W/2 - 16;
	int32_t ry = SCREEN_H/2 - 16;
	const uint32_t rw = 32 - 1;
	const uint32_t rh = 32 - 1;

	float rire = ronre(13.37f, 26.52f);
	*(volatile uint32_t *)ISSOU = *(uint32_t *) &rire;

	{
		enable_interrupts();
	}

	for (;;) {
		wait_vblank();

		poll_joypad();

		// PIF_JoyState is written via DMA so we must read the uncached version (via KSEG1)
		volatile struct JoyState *js = (volatile struct JoyState *)KSEG1ADDR(&PIF_JoyState[1]);
		if (js->d_up) {
			ry--;
		}
		if (js->d_down) {
			ry++;
		}
		if (js->d_right) {
			rx++;
		}
		if (js->d_left) {
			rx--;
		}

		if (rx < 0) {
			rx = 0;
			// enable VBLANK Interrupt when we reach left border
			// should block the game and set $k0 to DEADC0DE
			*MI_MASK = MI_INTR_VI;
		}
		if (rx > (int32_t)(SCREEN_W - rw)) {
			rx = SCREEN_W - rw;
		}
		if (ry < 0) {
			ry = 0;
		}
		if (ry > (int32_t)(SCREEN_H - rh)) {
			ry = SCREEN_H - rh;
		}

		// hardcode some RDP commands
		{
			static _Alignas(8) uint32_t command_buffer[] = {
				0x2D000000, 0x005003c0, // Set_Scissor
				0x2f300000, 0x00000000, // Set_Other_Modes. bit 52-53 = cycle type
				0x3f10013f, 0x00100000, // Set_Color_Image
				0x37000000, 0xf83ff83f, // Set_Fill_Color
				0x364fc3bc, 0x00000000, // Fill_Rectangle
				0x37000000, 0x4bb54bb5, // Set_Fill_Color
				0x00000000, 0x00000000, // Fill_Rectangle placeholder (NOOP)
				0x29000000, 0x00000000, // Sync_Full
			};

			// in cycle_type = FILL and COPY modes, bot right corner coords are inclusive, so -1
			RDP_FillRectangle(&command_buffer[12], rx, ry, rw, rh);

			*DPC_START = PHYSADDR(command_buffer);
			*DPC_END = PHYSADDR(command_buffer) + sizeof(command_buffer);
		}
	}
}
