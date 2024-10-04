// InfiniScroll Engine
// ise_mga.h
// Matrox HW acceleration
//
// Kamal Pillai
// 4/5/2020

#include "ise.h"

uint32_t ise_mga_dma_size_mask = 0x0;
uint32_t ise_mga_dma_offset = 0x0;
uint32_t ise_mga_dma_reg_addr = 0x0;
uint32_t ise_mga_dma_reg_data[4] = {0x0};
uint8_t ise_mga_dma_reg_count = 0x0;

#define ISE_MGA_DMA_REG(reg) ((uint32_t) ((((reg) >> 2) & 0x7F) | (((reg) >> 6) & 0x80)))

void ise_mga_reg_write(uint16_t reg, uint32_t data)
{
	if(ise_mga_dma_size_mask) {
		ise_mga_dma_reg_addr |= ISE_MGA_DMA_REG(reg) << (8*ise_mga_dma_reg_count);
		ise_mga_dma_reg_data[ise_mga_dma_reg_count] = data;
		if(ise_mga_dma_reg_count == 3 || reg & ISE_MGA_REG0_DRAW_START) {
			for(ise_mga_dma_reg_count++; ise_mga_dma_reg_count < 4; ise_mga_dma_reg_count++) {
				ise_mga_dma_reg_addr |= ISE_MGA_DMA_REG(ISE_MGA_REG0_DMAPAD) << (8*ise_mga_dma_reg_count);
				ise_mga_dma_reg_data[ise_mga_dma_reg_count] = 0x0;
			}
			ISE_SVGA_BASE_D(ise_mga_dma_offset) = ise_mga_dma_reg_addr;
			ise_mga_dma_offset++;
			ise_mga_dma_offset &= ise_mga_dma_size_mask;
			for(ise_mga_dma_reg_count = 0; ise_mga_dma_reg_count < 4; ise_mga_dma_reg_count++) {
				ISE_SVGA_BASE_D(ise_mga_dma_offset) = ise_mga_dma_reg_data[ise_mga_dma_reg_count];
				ise_mga_dma_offset++;
				ise_mga_dma_offset &= ise_mga_dma_size_mask;
			}
			ise_mga_dma_reg_addr = 0;
			ise_mga_dma_reg_count = 0;
		}
	} else {
		ISE_SVGA_BASE_D(reg) = data;
	}
}

void ise_mga_setup_mode(ise_gfx_t* gfx)
{
	int pitch_pad = (ise_screen_mode.bpp == 8 || ise_screen_mode.bpp == 24) ? 64 : 32;
	int pwidth = 0;
	switch(ise_screen_mode.bpp) {
	case 8: pwidth = 0; break;
	case 16: pwidth = 1; break;
	case 24: pwidth = 3; break;
	case 32: pwidth = 2; break;
	}
	gfx->buffers[0].pitch += pitch_pad - 1;
	gfx->buffers[0].pitch &= ~(pitch_pad - 1);
	ise_mga_reg_write(ISE_MGA_REG0_PITCH, (gfx->buffers[0].pitch) | 0x8000);
	ise_mga_reg_write(ISE_MGA_REG0_MACCESS, pwidth | 0x40000000);
	ise_mga_reg_write(ISE_MGA_REG0_CXBNDRY, (gfx->buffers[0].width - 1) << 16);
	ise_mga_reg_write(ISE_MGA_REG0_PLNWT, ~0x0);
	ise_mga_reg_write(ISE_MGA_REG0_ZORG, 0x0);
}

void ise_mga_upload_fast_sprite_xform(ise_gfx_t* gfx, ise_fast_sprite_t* fsp)
{
	int max_xform = 0, k;
	uint32_t xforms = 0;
	// first remove all the VFLIP xforms
	for(k=0; k<8; k++) {
		if(!(k & ISE_XFORM_VFLIP) && (((fsp->xforms >> (4*k)) & 0xF) != 0)) {
			max_xform++;
			xforms |= max_xform << (4*k);
		}
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
		fsp->xforms = xforms;
    } else {
        fsp->aperture = NULL;
        fsp->vmem_offset = ~0;
		fsp->bg_color = 0;
    }
}

void ise_mga_fill(ise_framebuffer_t* fb, int x, int y, uint16_t width, uint16_t height, uint32_t color)
{
	uint32_t color32 = color;
	if(ise_screen_mode.bpp == 8) {
		color32 &= 0xFF;
		color32 |= (color32 << 8);
		color32 |= (color32 << 16);
	} else if(ise_screen_mode.bpp == 16) {
		color32 &= 0xFFFF;
		color32 |= (color32 << 16);
	}
	uint32_t base = (fb->vmem_offset << 3) / ise_screen_mode.bpp;
	ise_mga_reg_write(ISE_MGA_REG0_YDSTORG, base);
	ise_mga_reg_write(ISE_MGA_REG0_YTOP, base);
	ise_mga_reg_write(ISE_MGA_REG0_YBOT, base + ((fb->height-1) * fb->pitch));
	ise_mga_reg_write(ISE_MGA_REG0_FXBNDRY, (((x + width) & 0xFFFF) << 16) | (x & 0xFFFF));
	ise_mga_reg_write(ISE_MGA_REG0_YDSTLEN, (height & 0xFFFF) | ((y * (fb->pitch >> 5)) << 16));
	ise_mga_reg_write(ISE_MGA_REG0_FCOL, color32);
	ise_mga_reg_write((ISE_MGA_REG0_DWGCTL | ISE_MGA_REG0_DRAW_START), 0xC7804);
}

void ise_mga_draw_fast_sprite(ise_fast_sprite_t* fsp, ise_framebuffer_t* fb, int x, int y)
{
	uint32_t base = (fb->vmem_offset << 3) / ise_screen_mode.bpp;
	ise_mga_reg_write(ISE_MGA_REG0_YDSTORG, base);
	ise_mga_reg_write(ISE_MGA_REG0_YTOP, base);
	ise_mga_reg_write(ISE_MGA_REG0_YBOT, base + ((fb->height-1) * fb->pitch));
	ise_mga_reg_write(ISE_MGA_REG0_FXBNDRY, (((x + fsp->sprite.width - 1) & 0xFFFF) << 16) | (x & 0xFFFF));
	ise_mga_reg_write(ISE_MGA_REG0_YDSTLEN, (fsp->sprite.height & 0xFFFF) | ((y * (fb->pitch >> 5)) << 16));
	base = (fsp->vmem_offset << 3) / ise_screen_mode.bpp;
	ise_mga_reg_write(ISE_MGA_REG0_AR3, base);
	ise_mga_reg_write(ISE_MGA_REG0_AR0, (base + fsp->sprite.width-1));  // what does this really do?
	ise_mga_reg_write(ISE_MGA_REG0_AR5, (fsp->sprite.width + 7) & ~7);
	ise_mga_reg_write((ISE_MGA_REG0_DWGCTL | ISE_MGA_REG0_DRAW_START), 0x040C6008);
}

void ise_mga_draw_sprite(ise_fast_sprite_t* fsp, ise_framebuffer_t* fb, int x, int y, uint16_t xform)
{
	uint32_t color32 = fsp->bg_color;
	if(ise_screen_mode.bpp == 8) {
		color32 &= 0xFF;
		color32 |= (color32 << 8);
		color32 |= (color32 << 16);
	} else if(ise_screen_mode.bpp == 16) {
		color32 &= 0xFFFF;
		color32 |= (color32 << 16);
	}	
	uint32_t base = (fb->vmem_offset << 3) / ise_screen_mode.bpp;
	uint32_t temp;
	ise_mga_reg_write(ISE_MGA_REG0_YDSTORG, base);
	ise_mga_reg_write(ISE_MGA_REG0_YTOP, base);
	ise_mga_reg_write(ISE_MGA_REG0_YBOT, base + ((fb->height-1) * fb->pitch));
	ise_mga_reg_write(ISE_MGA_REG0_FXBNDRY, (((x + fsp->sprite.width - 1) & 0xFFFF) << 16) | (x & 0xFFFF));
	ise_mga_reg_write(ISE_MGA_REG0_YDSTLEN, (fsp->sprite.height & 0xFFFF) | ((y * (fb->pitch >> 5)) << 16));
	ise_mga_reg_write(ISE_MGA_REG0_FCOL, color32);
	ise_mga_reg_write(ISE_MGA_REG0_BCOL, ~0x0);
	base = (fsp->vmem_offset << 3) / ise_screen_mode.bpp;
	int32_t pitch = (fsp->sprite.width + 7) & ~7;
	temp = (fsp->xforms >> (4*xform)) & 0xF;
	temp *= pitch * fsp->sprite.height * (ise_screen_mode.bpp >> 3);
	base += temp;
	if(xform & ISE_XFORM_VFLIP) {
		base += (fsp->sprite.height-1) * pitch;
		pitch = -pitch;
	}
	ise_mga_reg_write(ISE_MGA_REG0_AR3, base);
	ise_mga_reg_write(ISE_MGA_REG0_AR0, (base + fsp->sprite.width-1));  // what does this really do?
	ise_mga_reg_write(ISE_MGA_REG0_AR5, pitch);
	ise_mga_reg_write((ISE_MGA_REG0_DWGCTL | ISE_MGA_REG0_DRAW_START), 0x440C6008);
}

void ise_mga_uninstall()
{
	if(ise_svga_ctrl_base) {
		ise_mem_unmap( (void*) ise_svga_ctrl_base );
		ise_svga_ctrl_base = NULL;
		ise_svga_setup_mode = NULL;
		ise_svga_upload_fast_sprite_xform = NULL;
		ise_svga_fill = NULL;
		ise_svga_draw_fast_sprite = NULL;
		ise_svga_draw_sprite = NULL;
		ise_svga_uninstall = NULL;
	}
}

void ise_mga_install(int slot, int func)
{
	uint32_t paddr;
	uint16_t device_id = ise_pci.slot[slot][func].device_id;
	if(device_id == ISE_PCI_DEVICE_MGA1064SG || 
	   device_id == ISE_PCI_DEVICE_MGA2064W) {

		
		//if(device_id == ISE_PCI_DEVICE_MGA1064SG) ise_mga_dma_size_mask = 0xFFF;//0x7FFFFF;
		//else ise_mga_dma_size_mask = 0x0;

		uint32_t map_size;
		if(ise_mga_dma_size_mask > 0x4000) {
			paddr = ise_pci_read_config(0, slot, func, 0x18); // get MEMBASE3
			map_size = ise_mga_dma_size_mask+1;
		} else {
			paddr = ise_pci_read_config(0, slot, func, 0x10); // get MEMBASE1
			map_size = 0x4000;
		}
		if(paddr & 1) {
			// indicate I/O space, which is not legal
			return;
		}
		paddr &= ~0xF;
		ise_svga_ctrl_base = (uint8_t*) ise_mem_map((void*) paddr, map_size);
		ise_svga_setup_mode = ise_mga_setup_mode;
		ise_svga_upload_fast_sprite_xform = ise_mga_upload_fast_sprite_xform;
		ise_svga_fill = ise_mga_fill;
		ise_svga_draw_fast_sprite = ise_mga_draw_fast_sprite;
		ise_svga_draw_sprite = ise_mga_draw_sprite;
		ise_svga_uninstall = ise_mga_uninstall;
		ise_mga_dma_offset = 0x0;
		ise_mga_dma_reg_addr = 0x0;
		ise_mga_dma_reg_count = 0x0;
	}
}

