// InfiniScroll Engine
// ise_s3.h
// S3 Trio32/64 HW acceleration
//
// Kamal Pillai
// 4/23/2020

#include "ise.h"

void ise_s3_fill(ise_framebuffer_t* fb, int x, int y, uint16_t width, uint16_t height, uint32_t color)
{
	uint32_t vmem_offset = fb->vmem_offset;
	uint32_t base = (vmem_offset >> 20) & 0x7;
	vmem_offset &= 0xFFFFF;
	uint32_t coord = (vmem_offset % ise_screen_mode.width) + x;
	coord <<= 16;
	coord |= ((vmem_offset / ise_screen_mode.width) + y) & 0xFFFF;
	uint32_t dimensions = (((uint32_t) width-1) << 16) | (height-1);
	ISE_SVGA_BASE_W(ISE_S3_TRIO_MISC_REG) = ISE_S3_TRIO_MISC_MULTI_MISC2 | (uint16_t) base;
	ISE_SVGA_BASE_W(ISE_S3_TRIO_MISC_REG) = ISE_S3_TRIO_MISC_MULTI_MISC;
	ISE_SVGA_BASE_W(ISE_S3_TRIO_FRGD_MIX) = 0x0027;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_FRGD_COLOR) = color;
	ISE_SVGA_BASE_W(ISE_S3_TRIO_MISC_PIX_CNTL) = 0xA000;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_ALT_CURXY) = coord;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_ALT_PCNT) = dimensions;
	ISE_SVGA_BASE_W(ISE_S3_TRIO_CMD) = 0x40B1;
}

void ise_s3_upload_fast_sprite_xform(ise_gfx_t* gfx, ise_fast_sprite_t* fsp)
{
	int max_xform = 0, k;
	uint32_t xforms = fsp->xforms;
	for(k=0; k<8; k++) {
		if(((xforms >> (4*k)) & 0xF) > max_xform) max_xform = ((xforms >> 4*k) & 0xF);
	}
	max_xform++;

    ise_framebuffer_t fb = {0};
    fb.width = fsp->sprite.width * max_xform;
    fb.height = fsp->sprite.height;
    fb.pitch = (fb.width + 7) & ~7;
    fb.hpan = 0;

	uint8_t bpp = ise_screen_mode.bpp;
	uint8_t index;
	ise_valloc(&gfx->assets, &fb, bpp);
    if(fb.vmem_offset) {
        fsp->aperture = &(gfx->assets);
        fsp->vmem_offset = fb.vmem_offset;
		fsp->bg_color = gfx->bg_color;
		for(k=0; k<8; k++) {
			index = (uint8_t) ((xforms >> (4*k)) & 0xF);
			if(k == 0 || index != 0) {
				ise_draw_sprite(&fsp->sprite, &fb, fsp->sprite.width * index, 0, k, gfx->bg_color, 0);
			}
		}
    } else {
        fsp->aperture = NULL;
        fsp->vmem_offset = ~0;
		fsp->bg_color = 0;
    }
}

void ise_s3_draw_fast_sprite(ise_fast_sprite_t* fsp, ise_framebuffer_t* fb, int x, int y)
{
	uint32_t vmem_offset = fb->vmem_offset;
	uint32_t base = vmem_offset >> 20;
	vmem_offset &= 0xFFFFF;
	uint32_t coord = (vmem_offset % ise_screen_mode.width) + x;
	coord <<= 16;
	coord |= ((vmem_offset / ise_screen_mode.width) + y) & 0xFFFF;
	vmem_offset = fsp->vmem_offset;
	base |= (vmem_offset >> 16) & 0x70;
	vmem_offset &= 0xFFFFF;
	uint32_t dimensions = ((fsp->sprite.width-1) << 16) | (fsp->sprite.height-1);
	uint32_t src_coord = (vmem_offset % ise_screen_mode.width);
	src_coord <<= 16;
	src_coord |= (vmem_offset / ise_screen_mode.width) & 0xFFFF;
	ISE_SVGA_BASE_W(ISE_S3_TRIO_MISC_REG) = ISE_S3_TRIO_MISC_MULTI_MISC2 | (uint16_t) base;
	ISE_SVGA_BASE_W(ISE_S3_TRIO_MISC_REG) = ISE_S3_TRIO_MISC_MULTI_MISC;
	ISE_SVGA_BASE_W(ISE_S3_TRIO_FRGD_MIX) = 0x0067;
	ISE_SVGA_BASE_W(ISE_S3_TRIO_MISC_PIX_CNTL) = 0xA000;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_ALT_CURXY) = src_coord;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_ALT_STEP) = coord;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_ALT_PCNT) = dimensions;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_FRGD_COLOR) = 0x07;
	ISE_SVGA_BASE_W(ISE_S3_TRIO_CMD) = 0xC0B1;
}

void ise_s3_draw_sprite(ise_fast_sprite_t* fsp, ise_framebuffer_t* fb, int x, int y, uint16_t xform)
{
	uint32_t vmem_offset = fb->vmem_offset;
	uint32_t base = vmem_offset >> 20;
	vmem_offset &= 0xFFFFF;
	uint32_t coord = (vmem_offset % ise_screen_mode.width) + x;
	coord <<= 16;
	coord |= ((vmem_offset / ise_screen_mode.width) + y) & 0xFFFF;
	vmem_offset = (fsp->xforms >> (4*xform)) & 0xF;
	vmem_offset *= fsp->sprite.width;
	vmem_offset += fsp->vmem_offset;
	base |= (vmem_offset >> 16) & 0x70;
	vmem_offset &= 0xFFFFF;
	uint32_t dimensions = ((fsp->sprite.width-1) << 16) | (fsp->sprite.height-1);
	uint32_t src_coord = (vmem_offset % ise_screen_mode.width);
	src_coord <<= 16;
	src_coord |= (vmem_offset / ise_screen_mode.width) & 0xFFFF;
	ISE_SVGA_BASE_W(ISE_S3_TRIO_MISC_REG) = ISE_S3_TRIO_MISC_MULTI_MISC2 | (uint16_t) base;
	ISE_SVGA_BASE_W(ISE_S3_TRIO_MISC_REG) = ISE_S3_TRIO_MISC_MULTI_MISC | 0x100;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_COLOR_CMP) = ((base >> 4) > 1) ? 0x0 : fsp->bg_color;
	ISE_SVGA_BASE_W(ISE_S3_TRIO_FRGD_MIX) = 0x0067;
	ISE_SVGA_BASE_W(ISE_S3_TRIO_MISC_PIX_CNTL) = 0xA000;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_ALT_CURXY) = src_coord;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_ALT_STEP) = coord;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_ALT_PCNT) = dimensions;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_FRGD_COLOR) = 0x07;
	ISE_SVGA_BASE_W(ISE_S3_TRIO_CMD) = 0xC0B1;
}

void ise_s3_set_scissor(ise_gfx_t* gfx)
{
	uint32_t vmem_offset = gfx->back->vmem_offset;
	uint32_t coord_tl = (vmem_offset % ise_screen_mode.width);
	uint32_t coord_br = coord_tl + ise_screen_mode.width - 1;
	coord_tl <<= 16;
	coord_br <<= 16;
	coord_tl |= (vmem_offset / ise_screen_mode.width) & 0xFFFF;
	coord_br |= (coord_tl + ise_screen_mode.height - 1) & 0xFFFF;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_ALT_SCISSOR_TL) = coord_tl;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_ALT_SCISSOR_BR) = coord_br;
}

void ise_s3_update_display(ise_gfx_t* gfx)
{
	uint32_t vmem_offset = gfx->front->vmem_offset;
	uint16_t vmem_higher_data = (uint16_t) ((0x69) | ((vmem_offset >> 10) & 0xf00));
	uint16_t vmem_high_data = (uint16_t) (ISE_VGA_CRTC_START_ADDR_HIGH | ((vmem_offset >> 2) & 0xff00));
	uint16_t vmem_low_data = (uint16_t) (ISE_VGA_CRTC_START_ADDR_LOW | ((vmem_offset << 6) & 0xff00));
	//while(!(inp(ISE_VGA_INPUT_STATUS1) & ISE_VGA_INPUT_STATUS1_VRETRACE)) ;
	while((inp(ISE_VGA_INPUT_STATUS1) & ISE_VGA_INPUT_STATUS1_VRETRACE)) ;
	outpw(ISE_VGA_CRTC1_ADDR, vmem_higher_data);
	outpw(ISE_VGA_CRTC1_ADDR, vmem_high_data);
	outpw(ISE_VGA_CRTC1_ADDR, vmem_low_data);
	//while((inp(ISE_VGA_INPUT_STATUS1) & ISE_VGA_INPUT_STATUS1_VRETRACE)) ;
	while(!(inp(ISE_VGA_INPUT_STATUS1) & ISE_VGA_INPUT_STATUS1_VRETRACE)) ;
	ise_s3_set_scissor(gfx);
}

void ise_s3_setup_mode(ise_gfx_t* gfx)
{
	uint32_t data;
	// Unlock S3 registers
	outp(ISE_VGA_CRTC1_ADDR, 0x38);
	outp(ISE_VGA_CRTC1_ADDR+1, 0x48);
	outp(ISE_VGA_CRTC1_ADDR, 0x39);
	outp(ISE_VGA_CRTC1_ADDR+1, 0xA5);
	outp(ISE_VGA_CRTC1_ADDR, 0x40);
	data = inp(ISE_VGA_CRTC1_ADDR+1);
	data |= 1;
	outp(ISE_VGA_CRTC1_ADDR+1, data);

	// enable enhanced mapping mode
	outp(ISE_VGA_CRTC1_ADDR, 0x31);
	data = inp(ISE_VGA_CRTC1_ADDR+1);
	data |= 0x08;
	outp(ISE_VGA_CRTC1_ADDR+1, data);

	// enable linear addressing
	outp(ISE_VGA_CRTC1_ADDR, 0x58);
	data = inp(ISE_VGA_CRTC1_ADDR+1);
	data |= 0x10;
	if(ise_vram_size_64k >= 0x40) data |= 0x3;
	else if(ise_vram_size_64k >= 0x20) data |= 0x2;
	else if(ise_vram_size_64k >= 0x10) data |= 0x1;
	outp(ISE_VGA_CRTC1_ADDR+1, data);
	
	// enable MMIO
	outp(ISE_VGA_CRTC1_ADDR, 0x53);
	outp(ISE_VGA_CRTC1_ADDR+1, 0x11);
	
	ISE_SVGA_BASE_D(ISE_S3_TRIO_WRT_MASK) = ~0x0;
	ISE_SVGA_BASE_D(ISE_S3_TRIO_RD_MASK) = ~0x0;
	ise_s3_set_scissor(gfx);
}

void ise_s3_uninstall()
{
	// Lock S3 registers
	uint32_t data;
	outp(ISE_VGA_CRTC1_ADDR, 0x40);
	data = inp(ISE_VGA_CRTC1_ADDR+1);
	data &= 0xFE;
	outp(ISE_VGA_CRTC1_ADDR+1, data);
	// disable enhanced mapping mode
	outp(ISE_VGA_CRTC1_ADDR, 0x31);
	data = inp(ISE_VGA_CRTC1_ADDR+1);
	data &= ~0x09;
	outp(ISE_VGA_CRTC1_ADDR+1, data);
	ise_svga_ctrl_base = NULL;
	ise_svga_uninstall = NULL;
	ise_svga_setup_mode = NULL;
	ise_svga_set_rgb8_palette = NULL;
	ise_svga_update_display = NULL;
	ise_svga_fill = NULL;
	ise_svga_draw_fast_sprite = NULL;
	ise_svga_upload_fast_sprite_xform = NULL;
	ise_svga_draw_sprite = NULL;
}

void ise_s3_install(int slot, int func)
{
	uint32_t paddr;
	uint16_t device_id = ise_pci.slot[slot][func].device_id;
	if(device_id == ISE_PCI_DEVICE_S3_TRIO) {
		paddr = ise_pci_read_config(0, slot, func, 0x10);
		paddr &= ~0xF;
		ise_screen_mode.framebuffer = paddr;
		ise_screen_mode.options |= ISE_VMODE_OPTION_NO_SRC_PITCH;

		ise_svga_ctrl_base = ISE_VGA_BASE(0x0);
		ise_svga_uninstall = ise_s3_uninstall;
		ise_svga_setup_mode = ise_s3_setup_mode;
		ise_svga_set_rgb8_palette = ise_vga_set_rgb8_palette;
		ise_svga_update_display = ise_s3_update_display;
		ise_svga_fill = ise_s3_fill;
		ise_svga_draw_fast_sprite = ise_s3_draw_fast_sprite;
		ise_svga_upload_fast_sprite_xform = ise_s3_upload_fast_sprite_xform;
		ise_svga_draw_sprite = ise_s3_draw_sprite;
	}
}
