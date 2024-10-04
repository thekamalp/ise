// InfiniScroll Engine
// ise_vga.h
// VGA adapter related header
//
// Kamal Pillai
// 1/13/2019

#ifndef __ISE_VGA_H
#define __ISE_VGA_H

#include "ise_mga.h"
#include "ise_ati.h"
#include "ise_s3.h"
#include "ise_3dfx.h"

#ifdef DOS32
    #define ISE_VGA_BASE(o) ((uint8_t*) (0x0A0000 | (uint16_t) (o)))
#else
    #define ISE_VGA_BASE(o) ((uint8_t FAR *) (0xA0000000L | (uint16_t) (o)))
#endif

#define ISE_VGA_INPUT_STATUS1 0x3da
#define ISE_VGA_INPUT_STATUS1_DISPLAY_DISABLED 0x1
#define ISE_VGA_INPUT_STATUS1_VRETRACE 0x8

#define ISE_VGA_MISC_OUT_READ 0x3cc
#define ISE_VGA_MISC_OUT_WRITE 0x3c2

#define ISE_VGA_CRTC0_ADDR 0x3b4
#define ISE_VGA_CRTC0_DATA 0x3b5
#define ISE_VGA_CRTC1_ADDR 0x3d4
#define ISE_VGA_CRTC1_DATA 0x3d5
#define ISE_VGA_CRTC_VERTICAL_TOTAL 0x6
#define ISE_VGA_CRTC_OVERFLOW 0x7
#define ISE_VGA_CRTC_MAX_SCAN_LINE 0x9
#define ISE_VGA_CRTC_START_ADDR_HIGH 0xc
#define ISE_VGA_CRTC_START_ADDR_LOW 0xd
#define ISE_VGA_CRTC_VERT_RETRACE_START 0x10
#define ISE_VGA_CRTC_VERT_RETRACE_END 0x11
#define ISE_VGA_CRTC_VERT_DISPLAY_END 0x12
#define ISE_VGA_CRTC_OFFSET 0x13
#define ISE_VGA_CRTC_UNDERLINE_LOCATION 0x14
#define ISE_VGA_CRTC_START_VERT_BLANK 0x15
#define ISE_VGA_CRTC_END_VERT_BLANK 0x16
#define ISE_VGA_CRTC_MODE_CONTROL 0x17
#define ISE_VGA_CRTC_ATTRIB_STATUS 0x24

#define ISE_VGA_SEQ_ADDR 0x3c4
#define ISE_VGA_SEQ_DATA 0x3c5
#define ISE_VGA_SEQ_RESET 0x0
#define ISE_VGA_SEQ_MAP_MASK 0x2
#define ISE_VGA_SEQ_MEMORY_MODE 0x4

#define ISE_VGA_ATTR_ADDR 0x3c0
#define ISE_VGA_ATTR_DATA_WRITE 0x3c0
#define ISE_VGA_ATTR_DATA_READ 0x3c1
#define ISE_VGA_ATTR_OVERSCAN_COLOR 0x11
#define ISE_VGA_ATTR_HORIZ_PIX_PAN 0x13

#define ISE_VGA_COLOR_PALETTE_ADDR 0x3c8
#define ISE_VGA_COLOR_PALETTE_DATA 0x3c9

#define ISE_VGA_GRAPHICS_ADDR 0x3ce
#define ISE_VGA_GRAPHICS_DATA 0x3cf
#define ISE_VGA_GRAPHICS_SET_RESET 0x0
#define ISE_VGA_GRAPHICS_EN_SET_RESET 0x1
#define ISE_VGA_GRAPHICS_COLOR_COMPARE 0x2
#define ISE_VGA_GRAPHICS_DATA_ROTATE 0x3
#define ISE_VGA_GRAPHICS_READ_MAP_SELECT 0x4
#define ISE_VGA_GRAPHICS_MODE 0x5
#define ISE_VGA_GRAPHICS_MISC 0x6
#define ISE_VGA_GRAPHICS_COLOR_DONT_CARE 0x7
#define ISE_VGA_GRAPHICS_BIT_MASK 0x8

#define ISE_VGA_MODE_80X25 -1
#define ISE_VGA_MODE_320X200X8BPP -2
#define ISE_VGA_MODE_320X240X8BPP -3

#define ISE_VBE_SIG_VESA 0x41534556  // VESA
#define ISE_VBE_SIG_VBE2 0x32454256  // VBE2

#pragma pack(push, 1)
typedef struct {
	uint32_t signature;
	uint16_t version;
	uint32_t oem_string;
	uint32_t caps;
	uint32_t video_modes;
	uint16_t total_memory_64k;
	uint16_t oem_sw_rev;
	uint32_t oem_vendor;
	uint32_t oem_product_name;
	uint32_t oem_product_rev;
	char reserved[222];
	char oem_data[256];
} ise_vbe_info_t;

typedef struct {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;
 
	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;
 
	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} ise_vbe_mode_info_t;
#pragma pack(pop)

#define ISE_VMODE_OPTION_HW_ACCEL              0x0001
#define ISE_VMODE_OPTION_NO_SRC_PITCH          0x0002
#define ISE_VMODE_OPTION_3D_ACCEL              0x0004

typedef struct {
	int16_t mode;
	uint16_t attributes;
	uint16_t width;
	uint16_t height;
	uint8_t bpp;
	uint8_t memory_model;
	uint16_t options;
	uint32_t framebuffer;
} ise_vmode_t;

#define ISE_VGA_TIMING_INTERLACE               0x0001
#define ISE_VGA_TIMING_PRESERVE_ASPECT         0x0002

typedef struct {
	// input params
	uint32_t width;
	uint32_t height;
	uint32_t flags;
	float    refresh;
	float    display_aspect;   /// display aspect ratio in W:H
	uint32_t cell_gran;
	// calculated params
	uint32_t pixel_clk;
	float    actual_refresh;
	// horizontal params
	uint32_t h_front_porch;
	uint32_t h_sync_pulse;
	uint32_t h_back_porch;
	// vertical params
	uint32_t v_front_porch;
	uint32_t v_sync_pulse;
	uint32_t v_back_porch;
} ise_vga_timing_t;

#define ISE_SVGA_BASE_B(off) (* ((uint8_t*)  (ise_svga_ctrl_base + (off))))
#define ISE_SVGA_BASE_W(off) (* ((uint16_t*) (ise_svga_ctrl_base + (off))))
#define ISE_SVGA_BASE_D(off) (* ((uint32_t*) (ise_svga_ctrl_base + (off))))
extern uint8_t* ise_svga_ctrl_base;

extern uint16_t ise_vga_crtc_addr;

void ise_svga_install();

void ise_vga_get_timing();
void ise_vga_set_mode(int16_t mode);
void ise_vga_reset_attr_reg();
void ise_vga_set_rgb8_palette();
void ise_vga_calc_timing(ise_vga_timing_t* t);


#endif // #ifndef __ISE_VGA_H
