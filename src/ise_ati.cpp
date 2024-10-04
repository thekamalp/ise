// InfiniScroll Engine
// ise_ati.cpp
// ATI HW acceleration
//
// Kamal Pillai
// 4/8/2020

#include "ise.h"

void ise_ati_wait_fifo(uint16_t entries)
{
	while( (ISE_SVGA_BASE_D(ISE_ATI_M64_FIFO_STAT) & 0xFFFF) > (0x8000 >> entries) ) ;
}

void ise_ati_reset_engine()
{
	ISE_SVGA_BASE_D(ISE_ATI_M64_GEN_TEST_CNTL) = ISE_SVGA_BASE_D(ISE_ATI_M64_GEN_TEST_CNTL) & ~0x100;
	ISE_SVGA_BASE_D(ISE_ATI_M64_GEN_TEST_CNTL) = ISE_SVGA_BASE_D(ISE_ATI_M64_GEN_TEST_CNTL) |  0x100;
	ISE_SVGA_BASE_D(ISE_ATI_M64_BUS_CNTL) = ISE_SVGA_BASE_D(ISE_ATI_M64_BUS_CNTL) | 0x00A00000;
}

void ise_ati_wait_idle()
{
	ise_ati_wait_fifo(16);
	while( (ISE_SVGA_BASE_D(ISE_ATI_M64_GUI_STAT) & 0x1) != 0x0 ) ;
}

void ise_ati_setup_mode(ise_gfx_t* gfx)
{
	uint32_t pitch = gfx->back->pitch / ise_screen_mode.bpp;
	ise_ati_reset_engine();
	
	ise_ati_wait_fifo(13);
	//ISE_SVGA_BASE_D(ISE_ATI_M64_CONTEXT_MASK) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_OFF_PITCH) = pitch << 22;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_Y_X) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_HEIGHT) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_BRES_ERR) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_BRES_INC) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_BRES_DEC) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_CNTL) = 0x23;
	
	ISE_SVGA_BASE_D(ISE_ATI_M64_SRC_OFF_PITCH) = pitch << 22;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SRC_Y_X) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SRC_HEIGHT1_WIDTH1) = 0x1;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SRC_Y_X_START) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SRC_HEIGHT2_WIDTH2) = 0x1;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SRC_CNTL) = 0x10;

	ise_ati_wait_fifo(13);
	ISE_SVGA_BASE_D(ISE_ATI_M64_HOST_CNTL) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_PAT_REG0) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_PAT_REG1) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_PAT_CNTL) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SC_LEFT) = 0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SC_TOP) = 0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SC_BOTTOM) = ise_screen_mode.height-1;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SC_RIGHT) = ise_screen_mode.width-1;
	
	ISE_SVGA_BASE_D(ISE_ATI_M64_DP_BKGD_CLR) = gfx->bg_color;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DP_FRGD_CLR) = gfx->fg_color;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DP_WRITE_MASK) = ~0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DP_MIX) = 0x70003;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DP_SRC) = 0x100;
	
	ise_ati_wait_fifo(5);
	ISE_SVGA_BASE_D(ISE_ATI_M64_CLR_CMP_CLR) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_CLR_CMP_MSK) = ~0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_CLR_CMP_CNTL) = 0x0;
	switch(ise_screen_mode.bpp) {
	case 4:
		ISE_SVGA_BASE_D(ISE_ATI_M64_DP_PIX_WIDTH) = 0x00010101;
		ISE_SVGA_BASE_D(ISE_ATI_M64_DP_CHAIN_MASK) = 0x8888;
		break;
	case 8:
		ISE_SVGA_BASE_D(ISE_ATI_M64_DP_PIX_WIDTH) = 0x01020202;
		ISE_SVGA_BASE_D(ISE_ATI_M64_DP_CHAIN_MASK) = 0x8080;
		break;
	case 15:
		ISE_SVGA_BASE_D(ISE_ATI_M64_DP_PIX_WIDTH) = 0x01030303;
		ISE_SVGA_BASE_D(ISE_ATI_M64_DP_CHAIN_MASK) = 0x4210;
		break;
	case 16:
		ISE_SVGA_BASE_D(ISE_ATI_M64_DP_PIX_WIDTH) = 0x01040404;
		ISE_SVGA_BASE_D(ISE_ATI_M64_DP_CHAIN_MASK) = 0x8410;
		break;
	case 24:
		ISE_SVGA_BASE_D(ISE_ATI_M64_DP_PIX_WIDTH) = 0x01050505;
		ISE_SVGA_BASE_D(ISE_ATI_M64_DP_CHAIN_MASK) = 0x8080;
		break;
	case 32:
		ISE_SVGA_BASE_D(ISE_ATI_M64_DP_PIX_WIDTH) = 0x01060606;
		ISE_SVGA_BASE_D(ISE_ATI_M64_DP_CHAIN_MASK) = 0x8080;
		break;
	}
	ise_ati_wait_idle();
}

int ise_ati_set_mode(ise_gfx_t* gfx)
{
	union REGS regs = {0};
	gfx->buffers[0].pitch = ise_screen_mode.width;
	regs.h.cl = 0x90;
	switch(ise_screen_mode.bpp) {
	case 4: regs.h.cl |= 0x1; break;
	case 8: regs.h.cl |= 0x2; break;
	case 15: regs.h.cl |= 0x3;
	case 16: regs.h.cl |= 0x4;
	case 24: regs.h.cl |= 0x5;
	case 32: regs.h.cl |= 0x6;
	default: return -1;
	}
	if(ise_screen_mode.width == 640 && ise_screen_mode.height == 400) regs.h.ch = 0xe1;
	else if(ise_screen_mode.width == 320 && ise_screen_mode.height == 200) regs.h.ch = 0xe2;
	else if(ise_screen_mode.width == 320 && ise_screen_mode.height == 240) regs.h.ch = 0xe3;
	else if(ise_screen_mode.width == 640 && ise_screen_mode.height == 480) regs.h.ch = 0x12;
	else if(ise_screen_mode.width == 800 && ise_screen_mode.height == 600) regs.h.ch = 0x6A;
	else if(ise_screen_mode.width == 1024 && ise_screen_mode.height == 768) regs.h.ch = 0x55;
	else if(ise_screen_mode.width == 1280 && ise_screen_mode.height == 1024) regs.h.ch = 0x83;
	else if(ise_screen_mode.width == 1600 && ise_screen_mode.height == 1200) regs.h.ch = 0x84;
	else return -1;

	regs.w.ax = 0xA002;
	INT86(0x10, &regs, &regs);
	return 0;
}

void ise_ati_set_rgb8_palette()
{
    uint16_t color_index;
	uint8_t color_value;
	ISE_SVGA_BASE_B(ISE_ATI_M64_DAC_W_INDEX) = 0x0;
    for(color_index=0; color_index<256; color_index++) {
        // red
        color_value = (uint8_t) ((color_index >> 0) & 0x7);
        color_value |= color_value << 3;
		ISE_SVGA_BASE_B(ISE_ATI_M64_DAC_DATA) = color_value;
        // green
        color_value = (uint8_t) ((color_index >> 3) & 0x7);
        color_value |= color_value << 3;
		ISE_SVGA_BASE_B(ISE_ATI_M64_DAC_DATA) = color_value;
        // blue
        color_value = (uint8_t) ((color_index >> 6) & 0x3);
        color_value |= (color_value << 2) | (color_value << 4);
		ISE_SVGA_BASE_B(ISE_ATI_M64_DAC_DATA) = color_value;
    }
}

void ise_ati_update_display(ise_gfx_t* gfx)
{
	// Need to get pitch in 8 pixel unit...so just divide by bits per pixel
	uint32_t off_pitch = (gfx->front->pitch) / ise_screen_mode.bpp;
	off_pitch <<= 22;  // shift it into the right position
	// OR in the front buffer offset, in 8B units
	off_pitch |= (gfx->front->vmem_offset >> 3);
	ise_ati_wait_idle();
	//while(ISE_SVGA_BASE_W(ISE_ATI_M64_CRTC_VLINE_CRNT_VLINE+2) != 0) ;
	//while(ISE_SVGA_BASE_W(ISE_ATI_M64_CRTC_VLINE_CRNT_VLINE+2) < ise_screen_mode.height-1) ;
	while((ISE_SVGA_BASE_D(ISE_ATI_M64_CRTC_INT_CNTL) & 1)) ;
	ISE_SVGA_BASE_D(ISE_ATI_M64_CRTC_OFF_PITCH) = off_pitch;
	//ise_time_wait(5);
	//uint16_t vline = ISE_SVGA_BASE_W(ISE_ATI_M64_CRTC_VLINE_CRNT_VLINE+2);
	//while(ISE_SVGA_BASE_W(ISE_ATI_M64_CRTC_VLINE_CRNT_VLINE+2) == vline) ;
	//while(ISE_SVGA_BASE_W(ISE_ATI_M64_CRTC_VLINE_CRNT_VLINE+2) > vline) ;
	//while((inp(ISE_VGA_INPUT_STATUS1) & ISE_VGA_INPUT_STATUS1_VRETRACE)) ;
	//while(!(inp(ISE_VGA_INPUT_STATUS1) & ISE_VGA_INPUT_STATUS1_VRETRACE)) ;
	while(!(ISE_SVGA_BASE_D(ISE_ATI_M64_CRTC_INT_CNTL) & 1)) ;
	//while(ISE_SVGA_BASE_W(ISE_ATI_M64_CRTC_VLINE_CRNT_VLINE+2) != 0) ;
	//ise_time_wait(10);
	//while(ISE_SVGA_BASE_W(ISE_ATI_M64_CRTC_VLINE_CRNT_VLINE+2) < 200) ;
}

void ise_ati_upload_fast_sprite_xform(ise_gfx_t* gfx, ise_fast_sprite_t* fsp)
{
	int max_xform = 0, k;
	uint32_t xforms = fsp->xforms;
	for(k=0; k<8; k++) {
		if(((xforms >> (4*k)) & 0xF) > max_xform) max_xform = ((xforms >> 4*k) & 0xF);
	}
	max_xform++;

    ise_framebuffer_t fb = {0};
    fb.width = fsp->sprite.width;
    fb.height = fsp->sprite.height * max_xform;
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
				ise_draw_sprite(&fsp->sprite, &fb, 0, fsp->sprite.height * index, k, gfx->bg_color, 0);
			}
		}
    } else {
        fsp->aperture = NULL;
        fsp->vmem_offset = ~0;
		fsp->bg_color = 0;
    }
}
	
void ise_ati_draw_rect(int x, int y, uint16_t width, uint16_t height)
{
	ise_ati_wait_fifo(4);
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_X) = x & 0x7FFF;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_Y) = y & 0x7FFF;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_HEIGHT) = height;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_WIDTH) = width;
}

void ise_ati_fill(ise_framebuffer_t* fb, int x, int y, uint16_t width, uint16_t height, uint32_t color)
{
	uint32_t off_pitch = fb->pitch / ise_screen_mode.bpp;
	off_pitch <<= 22;
	off_pitch |= fb->vmem_offset >> 3;
	x += (8*(fb->vmem_offset & 0x7)) / ise_screen_mode.bpp;
	ise_ati_wait_fifo(4);
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_OFF_PITCH) = off_pitch;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DP_FRGD_CLR) = color;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_CNTL) = 0x03;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DP_SRC) = 0x101;
	ise_ati_draw_rect(x, y, width, height);
}

void ise_ati_draw_fast_sprite(ise_fast_sprite_t* fsp, ise_framebuffer_t* fb, int x, int y)
{
	uint32_t off_pitch;
	ise_ati_wait_fifo(7);
	ISE_SVGA_BASE_D(ISE_ATI_M64_DP_SRC) = 0x301;
	off_pitch = (fsp->sprite.width + 7) >> 3; // need to make sure this is padded to 8 pixels
	off_pitch <<= 22;
	off_pitch |= fsp->vmem_offset >> 3;  // need to make sure alignment is on 8 pixel granularity
	ISE_SVGA_BASE_D(ISE_ATI_M64_SRC_OFF_PITCH) = off_pitch;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SRC_CNTL) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SRC_Y_X) = 0x0;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SRC_WIDTH1) = fsp->sprite.width;
	off_pitch = fb->pitch / ise_screen_mode.bpp;
	off_pitch <<= 22;
	off_pitch |= fb->vmem_offset >> 3;
	x += (8*(fb->vmem_offset & 0x7)) / ise_screen_mode.bpp;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_OFF_PITCH) = off_pitch;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_CNTL) = 0x03;
	ise_ati_draw_rect(x, y, fsp->sprite.width, fsp->sprite.height);
}

#define ISE_ATI_USE_COLOR_MASK 1

void ise_ati_draw_sprite(ise_fast_sprite_t* fsp, ise_framebuffer_t* fb, int x, int y, uint16_t xform)
{
	int32_t off_pitch;
	uint32_t temp = 0;
#ifdef ISE_ATI_USE_COLOR_MASK
	if(fsp->sprite.mask) {
		ise_ati_wait_fifo(3);
		ISE_SVGA_BASE_D(ISE_ATI_M64_CLR_CMP_CLR) = fsp->bg_color;
		ISE_SVGA_BASE_D(ISE_ATI_M64_CLR_CMP_MSK) = ~0;
		ISE_SVGA_BASE_D(ISE_ATI_M64_CLR_CMP_CNTL) = 0x01000005;
	}
#else
	temp |= (fsp->sprite.mask) ? 0x020000 : 0x0;
#endif
	temp |= (fsp->sprite.color) ? 0x0301 : 0x101;
	ise_ati_wait_fifo(10);
	ISE_SVGA_BASE_D(ISE_ATI_M64_DP_SRC) = temp;
		
	off_pitch = (fsp->sprite.width + 7) >> 3; // need to make sure this is padded to 8 pixels
	off_pitch <<= 22;
	off_pitch |= fsp->vmem_offset >> 3;  // need to make sure alignment is on 8 pixel granularity
	ISE_SVGA_BASE_D(ISE_ATI_M64_SRC_OFF_PITCH) = off_pitch;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SRC_CNTL) = 0x0;
	temp = (fsp->xforms >> (4*xform)) & 0xF;
	temp *= fsp->sprite.height;
	if(xform & ISE_XFORM_VFLIP) temp += (fsp->sprite.height - 1);
	if(xform & ISE_XFORM_HFLIP) temp |= (fsp->sprite.width - 1) << 16;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SRC_Y_X) = temp;
	ISE_SVGA_BASE_D(ISE_ATI_M64_SRC_WIDTH1) = fsp->sprite.width;
	ISE_SVGA_BASE_D(ISE_ATI_M64_HOST_CNTL) = 0x0;
	switch(ise_screen_mode.bpp) {
	case 4:  temp = 0x00000101; break;
	case 8:  temp = 0x00000202; break;
	case 15: temp = 0x00000303; break;
	case 16: temp = 0x00000404; break;
	case 24: temp = 0x00000505; break;
	case 32: temp = 0x00000606; break;
	}
	temp |= (xform & ISE_XFORM_HFLIP) ? 0x0 : 0x01000000;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DP_PIX_WIDTH) = temp;
	off_pitch = fb->pitch / ise_screen_mode.bpp;
	off_pitch <<= 22;
	off_pitch |= fb->vmem_offset >> 3;
	if(xform & ISE_XFORM_HFLIP) x += fsp->sprite.width - 1;
	if(xform & ISE_XFORM_VFLIP) y += fsp->sprite.height - 1;
	x += (8*(fb->vmem_offset & 0x7)) / ise_screen_mode.bpp;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_OFF_PITCH) = off_pitch;
	temp = 0x0;
	temp |= (xform & ISE_XFORM_HFLIP) ? 0x0 : 0x1;
	temp |= (xform & ISE_XFORM_VFLIP) ? 0x0 : 0x2;
	ISE_SVGA_BASE_D(ISE_ATI_M64_DST_CNTL) = temp;
	ise_ati_draw_rect(x, y, fsp->sprite.width, fsp->sprite.height);

#ifdef ISE_ATI_USE_COLOR_MASK
	ise_ati_wait_fifo(1);
	ISE_SVGA_BASE_D(ISE_ATI_M64_CLR_CMP_CNTL) = 0x0;
#else
	if(fsp->sprite.mask) {
		int i, j, sp_mask_size_dwords = ((fsp->sprite.height * fsp->sprite.width + 31) / 32);
		for(i=0; i<sp_mask_size_dwords; ) {
			ise_ati_wait_fifo(4);
			for(j=0; j<4 && i<sp_mask_size_dwords; j++, i++) {
				temp = fsp->sprite.mask[i];
				if(xform & ISE_XFORM_HFLIP) {
					temp = ((temp & 0xFFFF0000) >> 16) | ((temp & 0x0000FFFF) << 16);
					temp = ((temp & 0xFF00FF00) >> 8)  | ((temp & 0x00FF00FF) << 8);
				}
				ISE_SVGA_BASE_D(ISE_ATI_M64_HOST_DATA + 4*(i & 0xF)) = temp;
			}
		}
	}
#endif
}

void ise_ati_uninstall()
{
	if(ise_svga_ctrl_base) {
		union REGS regs;
		ise_mem_unmap( ise_svga_ctrl_base );
		ise_svga_ctrl_base = NULL;
		ise_svga_setup_mode = NULL;
		ise_svga_set_rgb8_palette = NULL;
		ise_svga_update_display = NULL;
		ise_svga_upload_fast_sprite_xform = NULL;
		ise_svga_fill = NULL;
		ise_svga_draw_fast_sprite = NULL;
		ise_svga_draw_sprite = NULL;
		ise_svga_wait_idle = NULL;
		ise_svga_uninstall = NULL;
		// Disable linear framebuffer
		regs.w.ax = 0xA005;
		regs.w.cx = 0x4;
		INT86(0x10, &regs, &regs);
	}
}

void ise_ati_install(int slot, int func)
{
	uint32_t paddr;
	union REGS regs;
	switch(ise_pci.slot[slot][func].device_id) {
	case ISE_PCI_DEVICE_ATI_MACH64GX:
	case ISE_PCI_DEVICE_ATI_MACH64CX:
	case ISE_PCI_DEVICE_ATI_MACH64CT:
	case ISE_PCI_DEVICE_ATI_MACH64VT:
	case ISE_PCI_DEVICE_ATI_MACH64VTB:
	case ISE_PCI_DEVICE_ATI_MACH64VT4:
	case ISE_PCI_DEVICE_ATI_3D_RAGE_GT:
	case ISE_PCI_DEVICE_ATI_3D_RAGEII_GTB:
	case ISE_PCI_DEVICE_ATI_3D_RAGEIIC_PQFB_AGP:
	case ISE_PCI_DEVICE_ATI_3D_RAGEIIC_BGA_AGP:
	case ISE_PCI_DEVICE_ATI_3D_RAGEIIC_PQFP_PCI:
	case ISE_PCI_DEVICE_ATI_3D_RAGE_PRO_BGA_AGP:
	case ISE_PCI_DEVICE_ATI_3D_RAGE_PRO_BGA_AGP1X:
	case ISE_PCI_DEVICE_ATI_3D_RAGE_PRO_BGA_PCI:
	case ISE_PCI_DEVICE_ATI_3D_RAGE_PRO_PQFP_PCI:
	case ISE_PCI_DEVICE_ATI_3D_RAGE_PRO_PQFP_LIMITED_3D:
	case ISE_PCI_DEVICE_ATI_3D_RAGE_LT_PRO_BGA_PCI:
	case ISE_PCI_DEVICE_ATI_3D_RAGE_LT_PRO_BGA_AGP:
	case ISE_PCI_DEVICE_ATI_3D_RAGE_LT_PRO:
	case ISE_PCI_DEVICE_ATI_3D_RAGE_LT:
	case ISE_PCI_DEVICE_ATI_RAGE_XL:
	case ISE_PCI_DEVICE_ATI_RAGE_MOBILITY:
		//// Set refresh to factory defaults
		////regs.w.ax = 0xA015;
		////regs.h.bl = 0x5;
		////INT86(0x10, &regs, &regs);
		//// Query Bios for linear aperture base and size
		//regs.w.ax = 0xA006;
		//INT86(0x10, &regs, &regs);
		//paddr = ((uint32_t) regs.w.bx) << 20;
		//psize = ((uint32_t) regs.w.si) << 20;
		//ise_screen_mode.framebuffer = paddr;
		// Enable linear framebuffer
		regs.w.ax = 0xA005;
		regs.w.cx = 0x1;
		INT86(0x10, &regs, &regs);
		
		paddr = ise_pci_read_config(0, slot, func, 0x10);
		paddr &= ~0xF;
		ise_screen_mode.framebuffer = paddr;
		//paddr = ise_pci_read_config(0, slot, func, 0x18);
		//paddr &= ~0xF;
		paddr += 0x800000 - 0x400;

		ise_svga_ctrl_base = (uint8_t*) ise_mem_map((void*) paddr, 0x400);
		//printf("ATI mach64/RAGE card found base=0x%x vram: 0x%x ctrl: 0x%x\n", paddr, ise_vram_base, ise_svga_ctrl_base);
		ise_ati_reset_engine();
		ise_svga_set_rgb8_palette = ise_ati_set_rgb8_palette;
		ise_svga_update_display = ise_ati_update_display;
		ise_svga_upload_fast_sprite_xform = ise_ati_upload_fast_sprite_xform;
		//ise_svga_set_mode = ise_ati_set_mode;
		ise_svga_setup_mode = ise_ati_setup_mode;
		ise_svga_fill = ise_ati_fill;
		ise_svga_draw_fast_sprite = ise_ati_draw_fast_sprite;
		ise_svga_draw_sprite = ise_ati_draw_sprite;
		ise_svga_wait_idle = ise_ati_wait_idle;
		ise_svga_uninstall = ise_ati_uninstall;
		break;
	}
}
