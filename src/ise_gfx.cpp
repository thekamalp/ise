// InfiniScroll Engine
// ise_gfx.cpp
// Graphics routines
//
// Kamal Pillai
// 1/13/2019

#include "ise.h"

void* ise_vram_base = NULL;
uint32_t ise_vram_size_64k = 0;
ise_vmode_t ise_screen_mode = {0};
ise_font_t ise_default_font = {0};
uint32_t ise_default_font_data[384] = {0};
ise_font_t* ise_current_font = NULL;

uint32_t ise_reverse_byte(uint32_t a)
{
	uint32_t o = 0;
	int i, shift = 7;
	uint32_t mask0 = 0x01010101, mask1 = 0x80808080;
	for(i=0; i<4; i++) {
		o |= (a & mask0) << shift;
		o |= (a & mask1) >> shift;
		mask0 <<= 1;
		mask1 >>= 1;
		shift -= 2;
	}
	return o;
}

#define ISE_FONT_FROM_BIOS

void ise_load_default_font()
{
	uint32_t* font_base;
	int c, i, offset, char_size = 4;

#ifdef ISE_FONT_FROM_BIOS
	ise_regs_t regs = {0};
	//font_base = (uint32_t*) ise_mem_aligned_malloc(4096, 0, ISE_MEM_REGION_DOS);
	regs.eax = 0x1130;
	regs.ebx = 0x0600;
	//uint32_t addr = (uint32_t) font_base;
	//regs.ebp = addr &0xF;
	//regs.es = (uint16_t) (addr >> 4);
	ise_rm_interrupt(0x10, &regs);
	font_base = (uint32_t*) ((((uint32_t) regs.es) << 4) + (regs.ebp & 0xFFFF));
#else
	// Font must be loaded before switching modes
	outpw(ISE_VGA_GRAPHICS_ADDR, ISE_VGA_GRAPHICS_MODE | 0x0000);
	outpw(ISE_VGA_GRAPHICS_ADDR, ISE_VGA_GRAPHICS_MISC | 0x0400);
	outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MAP_MASK | 0x0400);
	outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MEMORY_MODE | 0x0600);
	font_base = (uint32_t*) ISE_VGA_BASE(0);
	char_size = 8;
#endif

	// copy font data
	ise_default_font.width = 8;
	ise_default_font.height = 16;
	for(c=32, i=0, offset=32*char_size; c<128; c++, i+=4, offset += char_size) {
		ise_default_font_data[i+0] = ise_reverse_byte(*(font_base + offset + 0));
		ise_default_font_data[i+1] = ise_reverse_byte(*(font_base + offset + 1));
		ise_default_font_data[i+2] = ise_reverse_byte(*(font_base + offset + 2));
		ise_default_font_data[i+3] = ise_reverse_byte(*(font_base + offset + 3));
		ise_default_font.mask[c] = &(ise_default_font_data[i]);
	}
	if(ise_current_font == NULL) ise_current_font = &ise_default_font;

#ifdef ISE_FONT_FROM_BIOS
	//ise_mem_aligned_free(font_base);
#else
	// restore registers
	outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MAP_MASK | 0x0300);
	outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MEMORY_MODE | 0x0200);
	outpw(ISE_VGA_GRAPHICS_ADDR, ISE_VGA_GRAPHICS_MODE | 0x1000);
	outpw(ISE_VGA_GRAPHICS_ADDR, ISE_VGA_GRAPHICS_MISC | 0x0E00);
#endif
}

void ise_valloc(ise_aperture_t* ap, ise_framebuffer_t* fb, uint8_t bpp)
{
	uint16_t pix_pitch = (fb->pitch + 7) & ~7;
    uint32_t pitch = (pix_pitch * bpp) / 8;
    uint32_t size = fb->height*pitch;
	if((ise_screen_mode.options & ISE_VMODE_OPTION_NO_SRC_PITCH) && (pix_pitch < ise_screen_mode.width)) {
		// if HW has no independent source pitch, it must use the screen pitch for all offsreen sprites
		int row_id, best_row_id = -1, best_row_height = 0x7FFFFFFF;
		for(row_id=0; row_id < ISE_MAX_APERTURE_ROWS; row_id++) {
			if(fb->height < ap->rows[row_id].height && ap->rows[row_id].height < best_row_height && pix_pitch < ap->rows[row_id].remaining) {
				best_row_id = row_id;
				best_row_height = ap->rows[row_id].height;
			}
		}
		uint32_t height = (fb->height + 15) & ~0xF;
		pix_pitch = (ise_screen_mode.width + 7) & ~0x7;
		pitch = (pix_pitch * bpp) / 8;
		size = height*pitch;
		if(best_row_id < 0) {
			if(ap->base + size >= ap->size) {
				fb->vmem_offset = 0x0;
				return;
			}
			best_row_id = ap->next_row;
			ap->next_row++;
			if(ap->next_row >= ISE_MAX_APERTURE_ROWS) ap->next_row = 0;
			ap->rows[best_row_id].vmem_offset = ap->start + ap->base;
			ap->rows[best_row_id].remaining = ise_screen_mode.width;
			ap->rows[best_row_id].width = ise_screen_mode.width;
			ap->rows[best_row_id].height = height;
			ap->base += size;
		}
		fb->vmem_offset = ap->rows[best_row_id].vmem_offset;
		ap->rows[best_row_id].vmem_offset += (fb->pitch * bpp) / 8;
		ap->rows[best_row_id].remaining -= fb->pitch;
		fb->pitch = (uint16_t) pitch;
	} else {
		if(ap->base + size >= ap->size) {
			fb->vmem_offset = 0x0;
			return;
		}
		fb->vmem_offset = ap->start + ap->base;
		fb->pitch = (bpp < 8) ? pix_pitch : (uint16_t) pitch;
		ap->base += size;
	}
}

void ise_vfree_all(ise_aperture_t* ap)
{
    ap->base = 0x0;
}

void ise_gfx_get_mode(ise_vmode_t* vmode)
{
	vmode->mode = 0x7FFF;  // invalid;

	if(vmode->width == 80 && vmode->height == 25) vmode->mode = ISE_VGA_MODE_80X25;
	else if(vmode->width == 320 && vmode->height == 200 && vmode->bpp == 8) vmode->mode = ISE_VGA_MODE_320X200X8BPP;
	else if(vmode->width == 320 && vmode->height == 240 && vmode->bpp == 8) vmode->mode = ISE_VGA_MODE_320X240X8BPP;
	else {
		const uint16_t MAX_VMODES = 256;
		ise_vmode_t enum_modes[MAX_VMODES];
		uint16_t num_vmodes = ise_gfx_get_supported_modes(MAX_VMODES, enum_modes);
		uint16_t i;
		for(i=0; i<num_vmodes; i++) {
			if(vmode->width == enum_modes[i].width && vmode->height == enum_modes[i].height && vmode->bpp == enum_modes[i].bpp) {
				*vmode = enum_modes[i];
			}
		}
	}
}
	
uint16_t ise_gfx_get_supported_modes(uint16_t max_modes, ise_vmode_t* vmode)
{
	ise_vbe_info_t* vbe_info;
	ise_vbe_mode_info_t* vbe_mode_info;
	ise_regs_t regs = {0};

	vbe_info = (ise_vbe_info_t*) ise_mem_aligned_malloc(512, 0, ISE_MEM_REGION_DOS);
	vbe_info->signature = ISE_VBE_SIG_VBE2;
	vbe_info->version = 0xFF10;
	uint32_t addr = (uint32_t) vbe_info;
	regs.eax = 0x4f00;
	regs.edi = addr &0xF;
	regs.es = (uint16_t) (addr >> 4);
	ise_rm_interrupt(0x10, &regs);
	uint16_t num_vmodes = 0;
	uint16_t vbe_version = vbe_info->version;
	if(vbe_info->signature == ISE_VBE_SIG_VESA && vbe_version >= 0x100) {
		ise_vram_size_64k = vbe_info->total_memory_64k;
		uint16_t* vmode_num = (uint16_t*) (((vbe_info->video_modes >> 12) & 0xFFFF0) + (vbe_info->video_modes & 0xFFFF));
		for(num_vmodes=0; num_vmodes<max_modes && *vmode_num != 0xFFFF; num_vmodes++) {
			vmode[num_vmodes].mode = *vmode_num;
			vmode_num++;
		}
		int i;
		vbe_mode_info = (ise_vbe_mode_info_t*) addr;
		for(i=0; i<num_vmodes; i++) {
			regs.eax = 0x4f01;
			regs.ecx = vmode[i].mode;
			regs.edi = addr &0xF;
			regs.es = (uint16_t) (addr >> 4);
			ise_rm_interrupt(0x10, &regs);
			vmode[i].attributes = vbe_mode_info->attributes;
			if(vbe_version < 0x200) vmode[i].attributes &= 0x7F;
			vmode[i].width = vbe_mode_info->width;
			vmode[i].height = vbe_mode_info->height;
			vmode[i].bpp = vbe_mode_info->bpp;
			vmode[i].memory_model = vbe_mode_info->memory_model;
			vmode[i].framebuffer = (vbe_version >= 0x200) ? vbe_mode_info->framebuffer : 0;
		}
	}
	ise_mem_aligned_free(vbe_info);
	return num_vmodes;
}

void ise_vbe_set_rgb8_palette()
{
	uint32_t* palette = (uint32_t*) ise_mem_aligned_malloc(256*sizeof(uint32_t), 4, ISE_MEM_REGION_DOS);
	uint32_t addr = (uint32_t) palette;
	ise_regs_t regs = {0};
	uint32_t color_index;
	uint32_t red, green, blue, color_value;
	for(color_index=0; color_index<256; color_index++) {
		red = (color_index >> 0) & 0x7;
		green = (color_index >> 3) & 0x7;
		blue = (color_index >> 6) & 0x3;
		color_value = (red << 16) | (red << 19) | (green << 8) | (green << 11) | (blue << 0) | (blue << 2) | (blue << 4);
		*(palette + color_index) = color_value;
	}
	regs.eax = 0x4f09;
	regs.ebx = 0x0000;
	regs.ecx = 256;
	regs.edx = 0;
	regs.edi = addr & 0xF;
	regs.es = (uint16_t) (addr >> 4);
	ise_rm_interrupt(0x10, &regs);
	ise_mem_aligned_free(palette);
}

void ise_gfx_init(ise_gfx_t* gfx, ise_vmode_t* vmode, uint16_t max_vscroll, uint16_t padding_request)
{
    ise_aperture_t vram;
    vram.start = 0x0;
    vram.base = 0x0;
    vram.size = 0x10000;

	if(ise_current_font == NULL) ise_load_default_font();

	if(ise_svga_uninstall) ise_svga_uninstall();

	if(ise_vram_base != NULL) {
		ise_mem_unmap(ise_vram_base);
	}

	ise_screen_mode = *vmode;
	ise_vram_base = NULL;

	if(ise_screen_mode.mode < 0) {

		// VGA mode
		// add 1 for horizontal scrolling
		max_vscroll++;

		ise_vga_set_mode(vmode->mode);
		if(vmode->mode == ISE_VGA_MODE_80X25) return;

		if(padding_request < 4) padding_request = 4;
		gfx->buffers[0].width = 320;
		gfx->buffers[0].height = (vmode->mode == ISE_VGA_MODE_320X240X8BPP) ? 240 : 200;
		gfx->buffers[0].height += max_vscroll;
		gfx->buffers[0].pitch = (320 + padding_request + 7) & ~7;
		gfx->buffers[0].hpan = 0;
		gfx->buffers[1] = gfx->buffers[0];
		ise_valloc(&vram, &(gfx->buffers[0]), 2);
		ise_valloc(&vram, &(gfx->buffers[1]), 2);
		gfx->buffers[0].height -= max_vscroll;
		gfx->buffers[1].height -= max_vscroll;

		uint16_t vscroll_padding = max_vscroll * gfx->buffers[0].pitch;
    
		// set screen pitch
		outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_OFFSET | ((gfx->buffers[0].pitch / 8) << 8));

		gfx->front = &(gfx->buffers[0]);
		gfx->back = &(gfx->buffers[1]);
    
		gfx->assets.start = vram.base;
		gfx->assets.base = 0x0;
		gfx->assets.size = vram.size - vram.base - vscroll_padding + 1;
    
		gfx->fg_color = 0xff;
		gfx->bg_color = 0x0;

		uint16_t front_addr = (uint16_t) gfx->front->vmem_offset;
		outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_START_ADDR_HIGH | (front_addr & 0xff00));
		outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_START_ADDR_LOW | ((front_addr << 8) & 0xff00));

		ise_vga_set_rgb8_palette();
	
		//ise_gfx_clear(gfx->front, 0);
		//ise_gfx_clear(gfx->back, 0);

		gfx->need_prev_scroll = false;
		gfx->prev_scroll_x = 0;
		gfx->prev_scroll_y = 0;

		gfx->num_front_sprites = 0;
		gfx->num_back_sprites = 0;
		gfx->front_sprite_bbox = &(gfx->sprite_bbox[0]);
		gfx->back_sprite_bbox = &(gfx->sprite_bbox[MAX_SPRITES]);
	} else {

		ise_svga_install();

		vram.size = ise_vram_size_64k << 16;
		if(ise_vram_base == NULL && ise_screen_mode.framebuffer != 0) ise_vram_base = ise_mem_map((void*) ise_screen_mode.framebuffer, vram.size);
		if(ise_vram_base == NULL) {
			// no linear buffer available
			printf("No linear buffer found - 0x%x\n", ise_screen_mode.framebuffer);
			return;
		}

		// SVGA mode
		union REGS regs = {0};

		gfx->buffers[0].width = ise_screen_mode.width;
		gfx->buffers[0].height = ise_screen_mode.height;
		gfx->buffers[0].pitch = (ise_screen_mode.width + padding_request + 7) & ~0x7;
		gfx->buffers[0].hpan = 0;

		if(ise_svga_set_mode) ise_svga_set_mode(gfx);
		else {
			regs.x.eax = 0x4f02;
			regs.x.ebx = ise_screen_mode.mode;
			if(ise_screen_mode.attributes & 0x80) {
				// Check for linear buffer support; if so enable it my sett bit 14 in ebx
				regs.x.ebx |= 0x4000;
			}
			INT86(0x10, &regs, &regs);

			if(ise_svga_setup_mode) ise_svga_setup_mode(gfx);

			if(gfx->buffers[0].pitch != gfx->buffers[0].width) {
				// set pitch
				regs.x.eax = 0x4f06;
				regs.x.ebx = 0x0000;
				regs.x.ecx = gfx->buffers[0].pitch;
				INT86(0x10, &regs, &regs);
	
				// get actual pitch
				regs.x.eax = 0x4f06;
				regs.x.ebx = 0x0001;
				INT86(0x10, &regs, &regs);
				gfx->buffers[0].pitch = regs.w.bx;
			}
		}

		gfx->buffers[1] = gfx->buffers[0];

		ise_valloc(&vram, &(gfx->buffers[0]), ise_screen_mode.bpp);
		ise_valloc(&vram, &(gfx->buffers[1]), ise_screen_mode.bpp);

		gfx->front = &(gfx->buffers[0]);
		gfx->back = &(gfx->buffers[1]);
    
		gfx->assets.start = vram.base;
		gfx->assets.base = 0x0;
		gfx->assets.size = vram.size - vram.base;
    
		gfx->fg_color = 0xff;
		gfx->bg_color = 0x0;

		// Set display start
		regs.x.eax = 0x4f07;
		regs.x.ebx = 0x0080;
		regs.x.ecx = 0x0000;
		regs.x.edx = 0;
		INT86(0x10, &regs, &regs);
		
		if(ise_svga_set_rgb8_palette) ise_svga_set_rgb8_palette();
		else ise_vbe_set_rgb8_palette();

		ise_gfx_clear(gfx->front, 0x0);
		ise_gfx_clear(gfx->back, 0x0);

		gfx->map_drawn = false;
		gfx->need_prev_scroll = false;
		gfx->prev_scroll_x = 0;
		gfx->prev_scroll_y = 0;

		gfx->num_front_sprites = 0;
		gfx->num_back_sprites = 0;
		gfx->front_sprite_bbox = &(gfx->sprite_bbox[0]);
		gfx->back_sprite_bbox = &(gfx->sprite_bbox[MAX_SPRITES]);
	}
}

void ise_gfx_clear(ise_framebuffer_t* fb, uint32_t clear_color)
{
	if(ise_screen_mode.mode < 0) {
		uint16_t FAR * vmem = (uint16_t FAR *) (ISE_VGA_BASE(fb->vmem_offset));
		// write to all planes
		outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MAP_MASK | 0xf00);
		uint16_t offset;
		uint16_t size = fb->height * (fb->pitch / 8);
		uint16_t color = (uint16_t) (clear_color & 0xFF);
		color |= (color << 8);
		for(offset = 0; offset < size; offset++) {
			*(vmem + offset) = color;
		}
	} else {
		if(ise_svga_fill) ise_svga_fill(fb, 0, 0, fb->width, fb->height, clear_color);
		else {
			uint32_t* vmem = (uint32_t*) ((uint8_t*) ise_vram_base + fb->vmem_offset);
			uint32_t offset;
			uint32_t size = (fb->height * fb->pitch * ise_screen_mode.bpp) / 32;  // size in dwords
			uint32_t color = clear_color;
			if(ise_screen_mode.bpp == 8) {
				color &= 0xFF;
				color |= (color << 8);
				color |= (color << 16);
			} else if(ise_screen_mode.bpp == 16) {
				color &= 0xFFFF;
				color |= (color << 16);
			}
			for(offset = 0; offset < size; offset++ ) {
				*(vmem + offset) = color;
			}
		}
	}
}

void ise_gfx_upload_fast_sprite(ise_gfx_t* gfx, ise_fast_sprite_t* fsp)
{
    ise_framebuffer_t fb = {0};
    fb.width = fsp->sprite.width;
    fb.height = fsp->sprite.height;
    fb.pitch = (fb.width + 7) & ~7;
    fb.hpan = 0;
	// only doing in vga modes or if driver support fast sprites
    if((ise_screen_mode.mode < 0) || (ise_svga_draw_fast_sprite)) {
		uint8_t bpp = (ise_screen_mode.mode < 0) ? 2 : ise_screen_mode.bpp;
		ise_valloc(&gfx->assets, &fb, bpp);
	}
    if(fb.vmem_offset) {
        fsp->aperture = &(gfx->assets);
        fsp->vmem_offset = (ise_screen_mode.mode < 0) ? (fb.vmem_offset - fsp->aperture->start) : fb.vmem_offset;
		fsp->xforms = 0;
		fsp->bg_color = gfx->bg_color;
        uint16_t cur_plane;
		uint16_t num_planes = (ise_screen_mode.mode < 0) ? 4 : 1;
        for(cur_plane = 0; cur_plane<num_planes; cur_plane++) {
            ise_set_plane(cur_plane);
            ise_draw_sprite(&fsp->sprite, &fb, 0, 0, ISE_XFORM_NONE, gfx->bg_color, cur_plane);
        }
    } else {
        fsp->aperture = NULL;
        fsp->vmem_offset = ~0;
		fsp->xforms = 0;
		fsp->bg_color = 0;
    }
}

void ise_gfx_set_map_origin(ise_gfx_t* gfx, uint32_t x, uint32_t y)
{
	if(ise_screen_mode.mode < 0) ise_gfx_scroll(gfx, (x & 3) - (gfx->map.origin_x & 3), 0);
	gfx->map.origin_x = x;
	gfx->map.origin_y = y;
	if(ise_screen_mode.mode < 0) {
		ise_draw_map(&(gfx->map), gfx->front, 0, 0, gfx->bg_color, ISE_MASK_NONE);
		ise_draw_map(&(gfx->map), gfx->back,  0, 0, gfx->bg_color, ISE_MASK_NONE);
	} else {
		gfx->map_drawn = false;
	}
}

void ise_gfx_scroll(ise_gfx_t* gfx, int x, int y)
{
    if(x < 0) {
        if(gfx->map.origin_x < -x) x = (int) (-gfx->map.origin_x);
    } else {
        uint32_t origin_end_x = gfx->map.map_width;
        origin_end_x <<= gfx->map.log2_tile_width;
        origin_end_x -= gfx->back->width;
        if(gfx->map.origin_x + x >= origin_end_x) x = (int) (origin_end_x - gfx->map.origin_x);
    }

    if(y < 0) {
        if(gfx->map.origin_y < -y) y = (int) (-gfx->map.origin_y);
    } else {
        uint32_t origin_end_y = gfx->map.map_height;
        origin_end_y <<= gfx->map.log2_tile_height;
        origin_end_y -= gfx->back->height;
        if(gfx->map.origin_y + y >= origin_end_y) y = (int) (origin_end_y - gfx->map.origin_y);
    }

	if(ise_screen_mode.mode < 0) {
		int x_offset = x + gfx->front->hpan;
		x_offset = (x_offset < 0) ? (x_offset - 3) : x_offset;
		x_offset /= 4;
		// determine how many bytes is vmem space to move
		int move_offset = y*(gfx->front->pitch / 4) + x_offset;

		// move horizontal pan state
		gfx->front->hpan += x & 3;
		gfx->front->hpan &= 3;
		gfx->back->hpan = gfx->front->hpan;

		// copy data in assets
		uint16_t asset_source, asset_dest, copy_size;
		if(move_offset < 0) {
			asset_source = (uint16_t) (gfx->assets.start + gfx->assets.size + move_offset);
			asset_dest = (uint16_t) (gfx->assets.start + move_offset);
			copy_size = -move_offset;
		} else {
			asset_source = (uint16_t) gfx->assets.start;
			asset_dest = (uint16_t) (gfx->assets.start + gfx->assets.size);
			copy_size = move_offset;
		}
		// OR source data with latched data
		outpw(ISE_VGA_GRAPHICS_ADDR, ISE_VGA_GRAPHICS_DATA_ROTATE | 0x1000);
		// write to all planes
		outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MAP_MASK | 0xf00);

		uint8_t data;
		uint16_t i;
		for(i=0; i<copy_size; i++, asset_source++, asset_dest++) {
			data = *(ISE_VGA_BASE(asset_source));
			*(ISE_VGA_BASE(asset_dest)) = 0x0;
		}
		// Go back to regular write mode
		outpw(ISE_VGA_GRAPHICS_ADDR, ISE_VGA_GRAPHICS_DATA_ROTATE | 0x0000);
		gfx->assets.start += move_offset;
		gfx->assets.start &= 0xFFFF;
		if(move_offset < 0) {
			if(gfx->assets.base - move_offset > gfx->assets.size) {
				gfx->assets.base -= gfx->assets.size;
			}
		} else {
			if(move_offset > gfx->assets.base) {
				gfx->assets.base += gfx->assets.size;
			}
		}
		gfx->assets.base -= move_offset;
		gfx->front->vmem_offset += move_offset;
		gfx->back->vmem_offset += move_offset;
		gfx->assets.base &= 0xFFFF;
		gfx->front->vmem_offset &= 0xFFFF;
		gfx->back->vmem_offset &= 0xFFFF;
	}

    gfx->map.origin_x += x;
    gfx->map.origin_y += y;

	if(ise_screen_mode.mode < 0) {
		ise_framebuffer_t fb;
		fb.pitch = gfx->back->pitch;

		if(gfx->need_prev_scroll) {
			gfx->need_prev_scroll = false;
			x += gfx->prev_scroll_x;
			gfx->prev_scroll_x = x - gfx->prev_scroll_x;
			y += gfx->prev_scroll_y;
			gfx->prev_scroll_y = y - gfx->prev_scroll_y;
		} else {
			gfx->prev_scroll_x += x;
			gfx->prev_scroll_y += y;
		}
		int offset_x, offset_y;
		uint16_t mask_flags = (gfx->back->width+3 < gfx->back->pitch) ? ISE_MASK_NONE : ISE_MASK_EDGE;
		if(x != 0) {
			offset_x = (x < 0) ? 0 : (gfx->back->width - x) & ~3;
			offset_y = (y < 0) ? -y : 0;
			fb.width = (x < 0) ? -x : x + 7;
			fb.height = gfx->back->height - ((y < 0) ? -y : y);
			fb.vmem_offset = (gfx->back->vmem_offset + (offset_y * (fb.pitch/4)) + ((offset_x+3)/4)) & 0xFFFF;
			fb.hpan = (gfx->back->hpan + offset_x) & 3;
			ise_draw_map(&gfx->map, &fb, offset_x, offset_y, gfx->bg_color, mask_flags);
		}
		if(y != 0) {
			offset_x = 0;
			offset_y = (y < 0) ? 0 : gfx->back->height - y;
			fb.width = gfx->back->width;
			fb.height = (y < 0) ? -y : y;
			fb.vmem_offset = (gfx->back->vmem_offset + (offset_y * (fb.pitch/4)) + (offset_x/4)) & 0xFFFF;
			fb.hpan = gfx->back->hpan;
			ise_draw_map(&gfx->map, &fb, offset_x, offset_y, gfx->bg_color, mask_flags);
		}
	} else {
		gfx->map_drawn = false;
	}
}

void ise_gfx_clear_map(ise_gfx_t* gfx, ise_bbox32_t* bbox)
{
    if(gfx->need_prev_scroll) {
        bbox->min_x -= gfx->prev_scroll_x;
        bbox->min_y -= gfx->prev_scroll_y;
	}

	// Check if the rect is out of the screen
	if(bbox->min_x > gfx->back->width || bbox->min_x + bbox->width < 0 || bbox->min_y > gfx->back->height || bbox->min_y + bbox->height < 0) return;
	
	// adjust if region partially outside screen
	if(bbox->min_x < 0) {
		bbox->width += bbox->min_x;
		bbox->min_x = 0;
	}
	if(bbox->min_x + bbox->width > gfx->back->width) {
		bbox->width = gfx->back->width - bbox->min_x;
	}
	if(bbox->min_y < 0) {
		bbox->height += bbox->min_y;
		bbox->min_y = 0;
	}
	if(bbox->min_y + bbox->height > gfx->back->height) {
		bbox->height = gfx->back->height - bbox->min_y;
	}

    ise_framebuffer_t fb;
    fb.pitch = gfx->back->pitch;
    int offset_x, offset_y;
    uint16_t mask_flags = (gfx->back->width+3 < gfx->back->pitch) ? ISE_MASK_NONE : ISE_MASK_EDGE;
	offset_x = (bbox->min_x < 0) ? 0 : bbox->min_x & ~3;
	offset_y = (bbox->min_y < 0) ? 0 : bbox->min_y;
	fb.width = bbox->width + (bbox->min_x - offset_x);
	fb.height = bbox->height + (bbox->min_y - offset_y);
	fb.vmem_offset = (gfx->back->vmem_offset + (offset_y * (fb.pitch/4)) + (offset_x/4)) & 0xFFFF;
	fb.hpan = gfx->back->hpan;
	ise_draw_map(&gfx->map, &fb, offset_x, offset_y, gfx->bg_color, mask_flags);
}

void ise_gfx_swap_buffers(ise_gfx_t* gfx)
{
    ise_framebuffer_t* swap;
    swap = gfx->front;
    gfx->front = gfx->back;
    gfx->back = swap;
    gfx->need_prev_scroll = true;
    ise_gfx_update_display(gfx);
	if(ise_screen_mode.mode < 0) {
		int i;
		for(i=0; i<gfx->num_front_sprites; i++) {
			ise_gfx_clear_map(gfx, &(gfx->front_sprite_bbox[i]));
		}
		gfx->num_front_sprites = gfx->num_back_sprites;
		gfx->num_back_sprites = 0;
		ise_bbox32_t* swap_bbox;
		swap_bbox = gfx->back_sprite_bbox;
		gfx->back_sprite_bbox = gfx->front_sprite_bbox;
		gfx->front_sprite_bbox = swap_bbox;
	}
}

void ise_gfx_update_display(ise_gfx_t* gfx)
{
	if(ise_screen_mode.mode < 0) {
		uint16_t vmem_offset = (uint16_t) gfx->front->vmem_offset;
		uint16_t vmem_high_data = ISE_VGA_CRTC_START_ADDR_HIGH | (vmem_offset & 0xff00);
		uint16_t vmem_low_data = ISE_VGA_CRTC_START_ADDR_LOW | ((vmem_offset << 8) & 0xff00);
		//while(!(inp(ISE_VGA_INPUT_STATUS1) & ISE_VGA_INPUT_STATUS1_VRETRACE)) ;
		while((inp(ISE_VGA_INPUT_STATUS1) & ISE_VGA_INPUT_STATUS1_VRETRACE)) ;
		outpw(ise_vga_crtc_addr, vmem_high_data);
		outpw(ise_vga_crtc_addr, vmem_low_data);
		//while((inp(ISE_VGA_INPUT_STATUS1) & ISE_VGA_INPUT_STATUS1_VRETRACE)) ;
		while(!(inp(ISE_VGA_INPUT_STATUS1) & ISE_VGA_INPUT_STATUS1_VRETRACE)) ;
		_disable();
		outp(ISE_VGA_ATTR_ADDR, ISE_VGA_ATTR_HORIZ_PIX_PAN | 0x20);
		outp(ISE_VGA_ATTR_DATA_WRITE, gfx->front->hpan * 2);
		_enable();
	} else {
		if(ise_svga_update_display) ise_svga_update_display(gfx);
		else {
			union REGS regs = {0};
			// Set display start
			regs.x.eax = 0x4f07;
			regs.x.ebx = 0x0080;
			regs.x.ecx = 0x0000;
			regs.x.edx = (gfx->front == &(gfx->buffers[1])) ? gfx->front->height : 0;
			INT86(0x10, &regs, &regs);
			while(!(inp(ISE_VGA_INPUT_STATUS1) & ISE_VGA_INPUT_STATUS1_VRETRACE)) ;
		}
	}
}

void ise_gfx_load_map(ise_gfx_t* gfx, const char* filename)
{
    gfx->map.sprites = NULL;
	gfx->map.tile_type = NULL;
    gfx->map.tiles = NULL;

    FILE* fp = fopen( filename, "rb" );
    if( fp == NULL ) return;

    uint16_t i;
    fread(&gfx->map, 1, 20, fp);
    if(gfx->map.id[0] != 'M' && gfx->map.id[1] != 'P') {
        fclose(fp);
        return;
    }

    fseek(fp, 32, SEEK_SET);

    char sprite_file[16];
    sprite_file[15] = '\0'; // Make sure we end with a 0
    gfx->map.sprites = (ise_fast_sprite_t*) malloc(sizeof(ise_fast_sprite_t)*gfx->map.num_sprites);  //new ise_fast_sprite_t[gfx->map.num_sprites];
	gfx->map.tile_type = (uint8_t*) malloc(sizeof(uint8_t)*gfx->map.num_sprites);
    gfx->map.num_fast_sprites = 0;
    bool upload_fast_sprite = ((ise_screen_mode.mode < 0) || (ise_svga_draw_fast_sprite)) ? true : false;
    for(i=0; i<gfx->map.num_sprites; i++) {
        fread(sprite_file, 1, 15, fp);
		fread(&(gfx->map.tile_type[i]), sizeof(uint8_t), 1, fp);
        ise_load_sprite(&(gfx->map.sprites[i].sprite), sprite_file);
        if(upload_fast_sprite) {
            ise_gfx_upload_fast_sprite(gfx, &(gfx->map.sprites[i]));
            if(gfx->map.sprites[i].vmem_offset != ~0) {
                gfx->map.num_fast_sprites = i + 1;
            } else {
                upload_fast_sprite = false;
            }
		} else {
			gfx->map.sprites[i].aperture = NULL;
			gfx->map.sprites[i].vmem_offset = ~0;
			gfx->map.sprites[i].xforms = 0;
			gfx->map.sprites[i].bg_color = 0;
        }
    }
    
    gfx->map.tiles = (uint8_t*) malloc(gfx->map.map_width * gfx->map.map_height); //new uint8_t[gfx->map.map_width * gfx->map.map_height];
    fread(gfx->map.tiles, 1, gfx->map.map_width * gfx->map.map_height, fp);
    
    fclose(fp);
}

void ise_gfx_free_map(ise_gfx_t* gfx)
{
    if(gfx->map.sprites) {
        free(gfx->map.sprites);  //delete [] gfx->map.sprites;
        gfx->map.sprites = NULL;
    }
	if(gfx->map.tile_type) {
		free(gfx->map.tile_type);
		gfx->map.tile_type = NULL;
	}
    if(gfx->map.tiles) {
        free(gfx->map.tiles); //delete [] gfx->map.tiles;
        gfx->map.tiles = NULL;
    }
}

void ise_draw_map(ise_map_t* map, ise_framebuffer_t* fb, int offset_x, int offset_y, uint8_t fill_color, uint16_t mask_flags)
{
    uint16_t tile_width = 1 << (map->log2_tile_width);
    uint16_t tile_height = 1 << (map->log2_tile_height);
    uint16_t tile_x_mask = tile_width - 1;
    uint16_t tile_y_mask = tile_height - 1;
    uint32_t pixel_start_x = map->origin_x + offset_x;
    uint32_t pixel_start_y = map->origin_y + offset_y;
    uint32_t pixel_end_x = pixel_start_x + fb->width;
    uint32_t pixel_end_y = pixel_start_y + fb->height;
    uint16_t tile_start_x = (uint16_t) (pixel_start_x >> map->log2_tile_width);
    uint16_t tile_start_y = (uint16_t) (pixel_start_y >> map->log2_tile_height);
    uint16_t tile_end_x = (uint16_t) ((pixel_end_x + tile_width) >> map->log2_tile_width);
    uint16_t tile_end_y = (uint16_t) ((pixel_end_y + tile_height) >> map->log2_tile_height);
    uint16_t tile_x_offset = (uint16_t) (pixel_start_x & tile_x_mask);
    uint16_t tile_y_offset = (uint16_t) (pixel_start_y & tile_y_mask);
    int pixel_x = -tile_x_offset;
    int pixel_y = -tile_y_offset;
    uint16_t num_tiles = map->map_width * map->map_height;
    uint16_t tile_offset;
    uint8_t sprite_id;
    uint16_t tile_x, tile_y;

	if(ise_screen_mode.mode < 0) {
		// write to all bit planes
		outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MAP_MASK | 0xf00);

		for(tile_y = tile_start_y; tile_y < tile_end_y; tile_y++, pixel_y += tile_height) {
			pixel_x = -tile_x_offset;
			for(tile_x = tile_start_x; tile_x < tile_end_x; tile_x++, pixel_x += tile_width) {
				tile_offset = (tile_y * map->map_width) + tile_x;
				sprite_id = (tile_offset < num_tiles) ? map->tiles[tile_offset] : 0;
				if(sprite_id < map->sprite_id_start || sprite_id >= (map->sprite_id_start + map->num_sprites)) {
					ise_fast_fill(fb, pixel_x, pixel_y, tile_width, tile_height, fill_color, mask_flags);
				}
			}
		}

		// OR source data with latched data
		outpw(ISE_VGA_GRAPHICS_ADDR, ISE_VGA_GRAPHICS_DATA_ROTATE | 0x1000);

		pixel_y = -tile_y_offset;
		for(tile_y = tile_start_y; tile_y < tile_end_y; tile_y++, pixel_y += tile_height) {
			pixel_x = -tile_x_offset;
			for(tile_x = tile_start_x; tile_x < tile_end_x; tile_x++, pixel_x += tile_width) {
				tile_offset = (tile_y * map->map_width) + tile_x;
				sprite_id = (tile_offset < num_tiles) ? map->tiles[tile_offset] : 0;
				if(sprite_id >= map->sprite_id_start && sprite_id < (map->sprite_id_start + map->num_sprites)) {
					ise_draw_fast_sprite(&(map->sprites[sprite_id - map->sprite_id_start]), fb, pixel_x, pixel_y, mask_flags);
				}
			}
		}
		// Go back to regular write mode
		outpw(ISE_VGA_GRAPHICS_ADDR, ISE_VGA_GRAPHICS_DATA_ROTATE | 0x0000);
	} else {

		for(tile_y = tile_start_y; tile_y < tile_end_y; tile_y++, pixel_y += tile_height) {
			pixel_x = -tile_x_offset;
			for(tile_x = tile_start_x; tile_x < tile_end_x; tile_x++, pixel_x += tile_width) {
				tile_offset = (tile_y * map->map_width) + tile_x;
				sprite_id = (tile_offset < num_tiles) ? map->tiles[tile_offset] : 0;
				if(sprite_id < map->sprite_id_start || sprite_id >= (map->sprite_id_start + map->num_sprites)) {
					ise_fast_fill(fb, pixel_x, pixel_y, tile_width, tile_height, fill_color, mask_flags);
				} else {
					ise_fast_sprite_t* fsp = &(map->sprites[sprite_id - map->sprite_id_start]);
					if(ise_svga_draw_fast_sprite && (fsp->vmem_offset != ~0)) ise_svga_draw_fast_sprite(fsp, fb, pixel_x, pixel_y);
					else ise_draw_sprite(&(fsp->sprite), fb, pixel_x, pixel_y, ISE_XFORM_NONE, fill_color, 0);
				}
			}
		}
		
	}
}

uint8_t ise_get_tile_type(ise_map_t* map, int x, int y)
{
	if(x < 0 || x >= map->map_width || y < 0 || y >= map->map_height) return 1;

	int map_index = y * map->map_width + x;
	int tile = map->tiles[map_index];
	if(tile < map->sprite_id_start || tile >= (map->sprite_id_start + map->num_sprites)) return 0;

	int tile_type = map->tile_type[tile - map->sprite_id_start];
	return tile_type;
}

uint32_t ise_check_map_collision(ise_map_t* map, ise_anim_object_t* obj, uint16_t ledge_depth)
{
	uint32_t collision_type = 0;
	ise_bbox_t* bbox = &(obj->anim_sprite[obj->anim].frames[obj->frame].hitbox);
	int x, y;
	int x_start, x_inc, x_end;
	int y_start, y_inc, y_end;
	int32_t pix_vec_x = (obj->frac_pos_x + obj->vec_x) >> 16;
	int32_t pix_vec_y = (obj->frac_pos_y + obj->vec_y) >> 16;
	int32_t collide_vec_x, collide_vec_y;
	uint8_t tile_type;
	bool collision;
	if(pix_vec_x != 0) {
		y_start = (obj->pos_y + bbox->min_y) >> map->log2_tile_height;
		y_inc = 1;
		y_end = ((obj->pos_y + bbox->min_y + bbox->height) >> map->log2_tile_height) + 1;
		if(pix_vec_x > 0) {
			x_start = ((obj->pos_x + bbox->min_x + bbox->width) >> map->log2_tile_width);
			x_inc = 1;
			x_end = ((obj->pos_x + bbox->min_x + bbox->width + pix_vec_x) >> map->log2_tile_width) + 1;
		} else {
			x_start = ((obj->pos_x + bbox->min_x) >> map->log2_tile_width);
			x_inc = -1;
			x_end = ((obj->pos_x + bbox->min_x + pix_vec_x) >> map->log2_tile_width) - 1;
		}
		collision = false;
		for(x = x_start; x != x_end && !collision; x += x_inc) {
			for(y = y_start; y != y_end && !collision; y += y_inc) {
				tile_type = ise_get_tile_type(map, x, y);
				if(tile_type == 1) {
					collision = true;
					collide_vec_x = ((x + (pix_vec_x < 0)) << map->log2_tile_width) - (obj->pos_x + bbox->min_x + ((pix_vec_x > 0) ? bbox->width : 0)) - (pix_vec_x > 0);
				}
			}
		}
		if(collision && abs(pix_vec_x) > ((pix_vec_x < 0) ? -collide_vec_x : collide_vec_x)) {
			collision_type |= 1 << (pix_vec_x < 0);
			obj->frac_pos_x = (pix_vec_x > 0) ? ~0 : 0;
			obj->vec_x = (collide_vec_x << 16);// + ((pix_vec_x > 0) ? ~(obj->frac_pos_x) : -(obj->frac_pos_x));
			//obj->pos_x += vec_x;//(vec_x << 16) / delta_time;
			//obj->hspeed = 0;
		}
		if(ledge_depth > 0) {
			// check for ledge
			bool ledge = true;
			x = (obj->pos_x + bbox->min_x + pix_vec_x + ((pix_vec_x > 0) ? bbox->width-1 : 0)) >> map->log2_tile_width;
			y_start = ((obj->pos_y + bbox->min_y + bbox->height) >> map->log2_tile_height) + 1;
			y_end = y_start + ledge_depth;
			for(y=y_start; y<y_end; y++) {
				tile_type = ise_get_tile_type(map, x, y);
				if(tile_type == 1) ledge = false;
			}
			if(ledge) collision_type |= 0x40;
		}
	}
	if(pix_vec_y != 0) {
		x_start = (obj->pos_x + bbox->min_x /*+ pix_vec_x*/) >> map->log2_tile_width;
		x_inc = 1;
		x_end = ((obj->pos_x + bbox->min_x /*+ pix_vec_x*/ + bbox->width) >> map->log2_tile_width) + 1;
		if(pix_vec_y > 0) {
			y_start = ((obj->pos_y + bbox->min_y + bbox->height) >> map->log2_tile_height) + 1;
			y_inc = 1;
			y_end = ((obj->pos_y + bbox->min_y + bbox->height + pix_vec_y) >> map->log2_tile_height) + 1;
		} else {
			y_start = ((obj->pos_y + bbox->min_y) >> map->log2_tile_height) - 1;
			y_inc = -1;
			y_end = ((obj->pos_y + bbox->min_y + pix_vec_y) >> map->log2_tile_height) - 1;
		}
		collision = false;
		for(y = y_start; y != y_end && !collision; y += y_inc) {
			for(x = x_start; x != x_end && !collision; x += x_inc) {
				tile_type = ise_get_tile_type(map, x, y);
				if(tile_type == 1) {
					collision = true;
					collide_vec_y = ((y + (pix_vec_y < 0)) << map->log2_tile_height) - (obj->pos_y + bbox->min_y + ((pix_vec_y > 0) ? bbox->height : 0)) - (pix_vec_y > 0);
				}
			}
		}
		if(collision && abs(pix_vec_y) > ((pix_vec_y < 0) ? -collide_vec_y : collide_vec_y)) {
			collision_type |= 4 << (pix_vec_y < 0);
			obj->frac_pos_y = (pix_vec_y > 0) ? ~0 : 0;
			obj->vec_y = (collide_vec_y << 16);// + ((pix_vec_y > 0) ? ~(obj->frac_pos_y) : -(obj->frac_pos_y));
			//(y - y_start - y_inc) << map->log2_tile_height;
			//obj->pos_y += vec_y;
			//obj->vspeed = 0;//(vec_y << 16) / delta_time;
		}
	}
	return collision_type;
}


uint8_t ise_get_closest_rgb8_color(ise_color_t* c)
{
    uint8_t color = ((c->red >> 5) & 0x07) | ((c->green >> 2) & 0x38) | (c->blue & 0xc0);
    return color;
}

void ise_load_sprite(ise_sprite_t* sp, const char* filename)
{
    sp->width = 0;
    sp->height = 0;
    sp->color = NULL;
    sp->mask = NULL;
    
    FILE* fp = fopen( filename, "rb" );
    if( fp == NULL ) return;

    ise_bmp_header_t bmp_header;
    fread(&bmp_header, 1, sizeof(bmp_header), fp);
    
    ise_bmp_mask_header_t bmp_mask_header;
    if(bmp_header.compression == ISE_BMP_BI_BITFIELDS) {
        fread(&bmp_mask_header, 1, sizeof(bmp_mask_header), fp);
    }
    
    if(bmp_header.width > 0x10000L || bmp_header.height > 0x10000L ||
        !(bmp_header.compression == ISE_BMP_BI_RGB || bmp_header.compression == ISE_BMP_BI_BITFIELDS) ||
        !(bmp_header.bits_per_pixel == 16 || bmp_header.bits_per_pixel == 24 || bmp_header.bits_per_pixel == 32) ) {
            fclose(fp);
            return;
    }
    
    sp->width = (uint16_t) bmp_header.width;
    sp->height = (uint16_t) bmp_header.height;
    
    uint16_t sp_pitch = (sp->width + 7) & ~7;
    uint16_t sp_mask_pitch = sp->width;//(sp->width + 7) / 8;
	uint16_t sp_mask_size_dwords = ((sp->height * sp_mask_pitch + 31) / 32);
    sp->color = (uint8_t*) malloc(sp->height * sp_pitch);  //new uint8_t[sp->height * sp_pitch];  //[4 * sp_plane_pitch];
    if(bmp_header.compression == ISE_BMP_BI_BITFIELDS && bmp_mask_header.alpha_mask != 0) {
		int i;
        sp->mask = (uint32_t*) malloc(4*sp_mask_size_dwords); // new uint8_t[sp->height * sp_mask_pitch];
		for(i=0; i<sp_mask_size_dwords; i++) sp->mask[i] = 0;
    } else {
        sp->mask = NULL;
    }

    uint16_t pitch = 4*((bmp_header.bits_per_pixel * sp->width + 24) / 32);
    uint16_t pad = pitch - ((bmp_header.bits_per_pixel * sp->width) / 8);

    int r_shift, g_shift, b_shift, a_shift;
    if(bmp_header.compression == ISE_BMP_BI_BITFIELDS) {
        int i;
        for(i=0; i<31; i++)
            if(bmp_mask_header.red_mask & (1L << i)) r_shift = i - 7;
        for(i=0; i<31; i++)
            if(bmp_mask_header.green_mask & (1L << i)) g_shift = i - 7;
        for(i=0; i<31; i++)
            if(bmp_mask_header.blue_mask & (1L << i)) b_shift = i - 7;
        for(i=0; i<31; i++)
            if(bmp_mask_header.alpha_mask & (1L << i)) a_shift = i - 7;
    } else {
		if(bmp_header.bits_per_pixel == 16) {
			bmp_mask_header.red_mask = 0x00007c00L;
			bmp_mask_header.green_mask = 0x000003e0L;
			bmp_mask_header.blue_mask = 0x0000001fL;
			bmp_mask_header.alpha_mask = 0x00008000L;
			r_shift = 7;
			g_shift = 2;
			b_shift = -3;
			a_shift = 8;
		} else {
			bmp_mask_header.red_mask = 0x00ff0000L;
			bmp_mask_header.green_mask = 0x0000ff00L;
			bmp_mask_header.blue_mask = 0x000000ffL;
			bmp_mask_header.alpha_mask = 0x00000000L;
			r_shift = 16;
			g_shift = 8;
			b_shift = 0;
			a_shift = 0;
		}
    }

    uint32_t raw_color;
    ise_color_t current_color;
    uint8_t norm_color;

    // move file pointer to beginning of bitmap_offset
    fseek(fp, bmp_header.bitmap_offset, SEEK_SET);
    
    uint16_t x, y, offset, index;
    // start from the bottom to top
    y=sp->height;
    do {
        y--;
        x = 0;
        do {
            fread(&raw_color, 1, bmp_header.bits_per_pixel / 8, fp);
            if(r_shift >= 0)
                current_color.red = (uint8_t) ((raw_color & bmp_mask_header.red_mask) >> r_shift);
            else
                current_color.red = (uint8_t) ((raw_color & bmp_mask_header.red_mask) << -r_shift);
            if(g_shift >= 0)
                current_color.green = (uint8_t) ((raw_color & bmp_mask_header.green_mask) >> g_shift);
            else
                current_color.green = (uint8_t) ((raw_color & bmp_mask_header.green_mask) << -g_shift);
            if(b_shift >= 0)
                current_color.blue = (uint8_t) ((raw_color & bmp_mask_header.blue_mask) >> b_shift);
            else
                current_color.blue = (uint8_t) ((raw_color & bmp_mask_header.blue_mask) << -b_shift);
            if(a_shift >= 0)
                current_color.alpha = (uint8_t) ((raw_color & bmp_mask_header.alpha_mask) >> a_shift);
            else
                current_color.alpha = (uint8_t) ((raw_color & bmp_mask_header.alpha_mask) << -a_shift);
            norm_color = ise_get_closest_rgb8_color(&current_color);
            index = x + (y * sp_pitch);
            sp->color[index] = norm_color;
            if(sp->mask) {
				offset = x + (y * sp_mask_pitch);
                index = offset / 32;//(x / 8) + (y * sp_mask_pitch);
				offset &= 0x1F;
                sp->mask[index] |= ((uint32_t) ((current_color.alpha >> 7) & 0x1)) << offset;
            }
            x++;
        } while(x != sp->width);
        if(pad) fread(&raw_color, 1, pad, fp);
    } while(y != 0x0);
    fclose(fp);
}

void ise_free_sprite_color(ise_sprite_t* sp)
{
    if(sp && sp->color) {
        free(sp->color); // delete [] sp->color;
        sp->color = NULL;
    }
}

void ise_free_sprite_mask(ise_sprite_t* sp)
{
    if(sp && sp->mask) {
        free(sp->mask); // delete [] sp->mask;
        sp->mask = NULL;
    }
}

void ise_free_sprite(ise_sprite_t* sp)
{
    ise_free_sprite_color(sp);
    ise_free_sprite_mask(sp);
}

void ise_set_plane(uint16_t cur_plane)
{
    outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MAP_MASK | (0x100 << cur_plane));
}

void ise_draw_text(ise_gfx_t* gfx, int x, int y, const char* str, uint16_t xform)
{
	if(ise_current_font == NULL) return;

	ise_sprite_t ch_sprite;
	ch_sprite.width = ise_current_font->width;
	ch_sprite.height = ise_current_font->height;
	ch_sprite.color = NULL;
	int cur_x, start_x;
	int cur_y, start_y;
	int width = 0, height = ch_sprite.height;
	int plane, num_planes = (ise_screen_mode.mode < 0) ? 4 : 1;
	int pass;
	const char* c;
	ise_framebuffer_t* fb = gfx->back;
	for(plane = 0; plane < num_planes; plane++) {
		pass = (xform & ISE_XFORM_SHADOW) ? 0 : 1;
		if(num_planes > 1) ise_set_plane(plane);
		for( ; pass < 2; pass++) {
			int color = (pass == 0) ? gfx->bg_color : gfx->fg_color;
			cur_x = start_x = x + !pass;
			cur_y = start_y = y + !pass;
			for(c = str; *c != '\0'; c++) {
				ch_sprite.mask = ise_current_font->mask[*c];
				if(ch_sprite.mask != NULL && cur_x < fb->width) {
					ise_draw_sprite(&ch_sprite, fb, cur_x, cur_y, ISE_XFORM_NONE, color, plane);
				}
				if(*c == '\n') {
					cur_x = start_x;
					cur_y += ch_sprite.height;
					if(plane == 0) height += ch_sprite.height;
				} else {
					cur_x += ch_sprite.width;
					if(plane == 0 && ch_sprite.mask != NULL && cur_x - x > width) width = cur_x - start_x;
				}
			}
		}
	}
	if(num_planes > 1) {
		ise_bbox32_t* bbox = &(gfx->back_sprite_bbox[gfx->num_back_sprites]);
		bbox->min_x = x;
		bbox->min_y = y;
		bbox->width = width;
		bbox->height = height;
		gfx->num_back_sprites++;
	}
}

int ise_draw_sprite(ise_sprite_t* sp, ise_framebuffer_t* fb, int x, int y, uint16_t xform, int fill_color, uint16_t cur_plane)
{
    uint16_t fb_start_y = (y < 0) ? 0 : y;
    uint16_t sp_start_y = (y < 0) ? -y : 0;
    uint16_t fb_start_x = (x < 0) ? 0 : x;
    uint16_t sp_start_x = (x < 0) ? -x : 0;

    uint16_t sp_width = (xform & ISE_XFORM_FLIP_XY) ? sp->height : sp->width;
    uint16_t sp_height = (xform & ISE_XFORM_FLIP_XY) ? sp->width : sp->height;

    // check if entire sprite is out of framebuffer
    if((fb_start_y > fb->height) || (sp_start_y > sp_height) || (fb_start_x > fb->width) || (sp_start_x > sp_width))
        return 0;

    uint16_t fb_end_y = fb_start_y + (sp_height - sp_start_y);
    if(fb_end_y > fb->height) fb_end_y = fb->height;
    if(xform & ISE_XFORM_VFLIP) sp_start_y = sp_height - sp_start_y - 1;//(fb_end_y - fb_start_y) - 1;

    uint16_t fb_end_x = fb_start_x + (sp_width - sp_start_x);
    if(fb_end_x > fb->width) fb_end_x = fb->width;
    if(xform & ISE_XFORM_HFLIP) sp_start_x = sp_width - sp_start_x - 1;//(fb_end_x - fb_start_x) - 1;

    int16_t sp_x_inc = 1;
    int16_t sp_y_inc = (sp->width + 7) & ~7;
    int16_t sp_mask_x_inc = 1;
    int16_t sp_mask_y_inc = sp->width; //(sp->width + 7) & ~7;

    uint16_t sp_pitch = sp_y_inc;
    uint16_t sp_mask_pitch = sp_mask_y_inc;

    if(xform & ISE_XFORM_FLIP_XY) {
        uint16_t uswap_temp;
        int16_t swap_temp;
        uswap_temp = sp_start_x;
        sp_start_x = sp_start_y;
        sp_start_y = uswap_temp;
        swap_temp = sp_x_inc;
        sp_x_inc = sp_y_inc;
        sp_y_inc = swap_temp;
        swap_temp = sp_mask_x_inc;
        sp_mask_x_inc = sp_mask_y_inc;
        sp_mask_y_inc = swap_temp;
    }

    if(xform & ISE_XFORM_HFLIP) {
        sp_x_inc = -sp_x_inc;
        sp_mask_x_inc = -sp_mask_x_inc;
    }
    if(xform & ISE_XFORM_VFLIP) {
        sp_y_inc = -sp_y_inc;
        sp_mask_y_inc = -sp_mask_y_inc;
    }

    uint32_t fb_x, fb_y;
    uint32_t fb_offset, fb_row_offset;
    uint32_t sp_offset, sp_row_offset;
    uint32_t sp_mask_offset, sp_mask_row_offset;
	uint16_t num_planes = 1;

	if(ise_screen_mode.mode < 0) {
		num_planes = 4;
		sp_x_inc *= num_planes;
		sp_mask_x_inc *= num_planes;
		uint16_t sp_plane = (cur_plane - fb->hpan - fb_start_x) & 0x3;
    
		fb_start_x += sp_plane;
		if(xform & ISE_XFORM_FLIP_XY) {
			sp_start_y += (xform & ISE_XFORM_HFLIP) ? -sp_plane : sp_plane;
		} else {
			sp_start_x += (xform & ISE_XFORM_HFLIP) ? -sp_plane : sp_plane;
		}
	}
    
    bool pixel_lit = true;

    fb_row_offset = (fb_start_y * (fb->pitch / num_planes)) + (fb_start_x + fb->hpan) / num_planes;
    sp_row_offset = (sp_start_y * sp_pitch) + sp_start_x;
    sp_mask_row_offset = (sp_start_y * sp_mask_pitch) + sp_start_x;
    for(fb_y= fb_start_y; fb_y < fb_end_y; fb_y++) {
        fb_offset = fb_row_offset;
        sp_offset = sp_row_offset;
        sp_mask_offset = sp_mask_row_offset;
        for(fb_x=fb_start_x; fb_x < fb_end_x; fb_x+=num_planes) {
            if(sp->mask) pixel_lit = (sp->mask[sp_mask_offset / 32] & (0x1 << (sp_mask_offset & 0x1F))) != 0;
            if(pixel_lit || ((fill_color >= 0) && sp->color)) {
				if(ise_screen_mode.mode < 0) {
					*(ISE_VGA_BASE(fb->vmem_offset + fb_offset)) = (pixel_lit && sp->color) ? sp->color[sp_offset] : fill_color & 0xff;
				} else {
					*((uint8_t*) ise_vram_base + fb->vmem_offset + fb_offset) = (pixel_lit && sp->color) ? sp->color[sp_offset] : fill_color & 0xff;
				}
            }
            fb_offset ++;
            sp_offset += sp_x_inc;
            sp_mask_offset += sp_mask_x_inc;
        }
        fb_row_offset += (fb->pitch / num_planes);
        sp_row_offset += sp_y_inc;
        sp_mask_row_offset += sp_mask_y_inc;
    }
	return 1;
}

void ise_fast_fill(ise_framebuffer_t* fb, int x, int y, uint16_t width, uint16_t height, uint8_t fill_color, uint16_t mask_flags)
{
	if(ise_screen_mode.mode < 0) {
		int aligned_x = (x & ~3) | ((fb->hpan ^ (fb->hpan << 1)) & 3);
    
		uint16_t fb_y = (y < 0) ? 0 : y;
		uint16_t sp_y = (y < 0) ? -y : 0;
		uint16_t fb_end_y = fb_y + (height - sp_y);
		if(fb_end_y > fb->height) fb_end_y = fb->height;
		uint16_t fb_x = (aligned_x < 0) ? 0 : aligned_x;
		uint16_t sp_x = (aligned_x < 0) ? -aligned_x : 0;
		uint16_t fb_end_x = fb_x + (width - sp_x);
		if(fb_end_x > fb->width) fb_end_x = fb->width;

		// check if entire sprite is out of framebuffer
		if((fb_y > fb->height) || (fb_x > fb->width) || (sp_y > height) || (sp_x > width)) return;

		uint32_t fb_offset;
		uint16_t edge_mask, mask, last_mask;
		edge_mask = 0xf00;
		last_mask = 0xf00;
		bool edge_mask_needed = (mask_flags & ISE_MASK_EDGE);
		if(fb->pitch - fb->width < 4) edge_mask_needed = true;

		uint32_t fb_col_offset;
		fb_col_offset = (fb_y * (fb->pitch/4)) + ((fb_x + fb->hpan) / 4);
    
		for(; fb_x < fb_end_x; fb_x += 4, sp_x += 4) {
			if(edge_mask_needed) {
				// take care of right edge of screen
				edge_mask = ((fb_x & ~3) < (fb_end_x & ~3)) ? 0xf00 : ~(0xf00 << (fb_end_x - fb_x)) & 0xf00;
				// take care of left edge of screen
				edge_mask &= ((fb_x & ~3) > 0) ? 0xf00 : (0xf00 << fb->hpan);


				// take care of right edge of screen
				//edge_mask = ((fb->width - fb_x) >= 4) ? 0xf00 : ~(0xf00 << fb->hpan) & 0xf00;
				// take care of left edge of screen
				//edge_mask &= (fb_x >= 4) ? 0xf00 : (0xf00 << fb->hpan);
			}
			fb_y = (y < 0) ? 0 : y;

			fb_offset = fb_col_offset;
			for(; fb_y < fb_end_y; fb_y++) {
				mask = edge_mask;
				if(mask) {
					if(mask != last_mask) {
						outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MAP_MASK | mask);
						last_mask = mask;
					}
					if(ise_screen_mode.mode < 0) {
						*(ISE_VGA_BASE(fb->vmem_offset + fb_offset)) = (edge_mask == 0xf00) ? fill_color : 0xff;
					} else {
					}
				}
				fb_offset += fb->pitch/4;
			}
			fb_col_offset++;
		}
		if(last_mask != 0xf00) {
			// write to all bit planes
			outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MAP_MASK | 0xf00);
		}
    } else if(ise_svga_fill) {
		ise_svga_fill(fb, x, y, width, height, fill_color);
	} else {
		uint32_t fill_color32 = fill_color;
		fill_color32 |= (fill_color32 << 8);
		fill_color32 |= (fill_color32 << 8);
		fill_color32 |= (fill_color32 << 8);

		uint16_t fb_y = (y < 0) ? 0 : y;
		uint16_t sp_y = (y < 0) ? -y : 0;
		uint16_t fb_end_y = fb_y + (height - sp_y);
		if(fb_end_y > fb->height) fb_end_y = fb->height;
		uint16_t fb_x = (x < 0) ? 0 : x;
		uint16_t sp_x = (x < 0) ? -x : 0;
		uint16_t fb_end_x = fb_x + (width - sp_x);
		if(fb_end_x > fb->width) fb_end_x = fb->width;

		// check if entire sprite is out of framebuffer
		if((fb_y > fb->height) || (fb_x > fb->width) || (sp_y > height) || (sp_x > width)) return;

		uint32_t fb_offset;
		uint32_t fb_col_offset;
		fb_col_offset = ((fb_y * fb->pitch) + fb_x);
		uint16_t sub_x, sub_x_start, sub_x_end;

		for(; fb_x < fb_end_x; fb_x = (fb_x + 4) & ~0x3) {
			fb_y = (y < 0) ? 0 : y;
			sub_x_start = fb_x & 0x3;
			sub_x_end = (((fb_x + 4) & ~0x3) > fb_end_x) ? (fb_end_x & 0x3) + 1 : 4;

			fb_offset = fb_col_offset & ~0x3;
			for(; fb_y < fb_end_y; fb_y++) {
				uint8_t* vmem = (uint8_t*) ise_vram_base + fb->vmem_offset;
				if(sub_x_start == 0 && sub_x_end == 4) {
					*((uint32_t*) (vmem + fb_offset)) = fill_color32;
				} else {
					for(sub_x = sub_x_start; sub_x < sub_x_end; sub_x++) {
						*(vmem + fb_offset + sub_x) = fill_color;
					}
				}
				fb_offset += fb->pitch;
			}
			fb_col_offset = (fb_col_offset + 4) & ~0x3;
		}
	}
    
}

void ise_load_anim_set(ise_gfx_t* gfx, ise_anim_set_t* anim_set, const char* filename)
{
	anim_set->num_anim_sprites = 0;
	anim_set->num_sprites = 0;
	anim_set->anim_sprite = NULL;
	anim_set->fsp = NULL;

	uint16_t i, j;
	uint32_t sp_index;
	char sprite_file[17];
	sprite_file[16] = '\0';

    FILE* fp = fopen( filename, "rb" );
    if( fp == NULL ) return;

	// read header
	uint32_t header[2];
	fread(header, sizeof(uint32_t), 2, fp);
	if(header[0] != 0x5F455349 || header[1] != 0x4D494E41) {
		fclose(fp);
		return;
	}

	fread(&(anim_set->num_sprites), sizeof(anim_set->num_sprites), 1, fp);
	fread(&(anim_set->num_anim_sprites), sizeof(anim_set->num_anim_sprites), 1, fp);
	anim_set->fsp = (ise_fast_sprite_t*) malloc(anim_set->num_sprites*sizeof(ise_fast_sprite_t));
	anim_set->anim_sprite = (ise_anim_sprite_t*) malloc(anim_set->num_anim_sprites*sizeof(ise_anim_sprite_t));

	for(i=0; i<anim_set->num_sprites; i++) {
		fread(sprite_file, 16, 1, fp);
		ise_load_sprite(&(anim_set->fsp[i].sprite), sprite_file);
		if(ise_svga_draw_sprite && (ise_svga_upload_fast_sprite_xform == NULL) && (ise_screen_mode.mode >= 0)) {
			ise_gfx_upload_fast_sprite(gfx, &(anim_set->fsp[i]));
		} else {
			anim_set->fsp[i].vmem_offset = ~0x0;
			anim_set->fsp[i].aperture = NULL;
			anim_set->fsp[i].xforms = 0;
			anim_set->fsp[i].bg_color = 0;
		}
	}

	ise_anim_sprite_t* anim_sprite;
	ise_anim_frame_t* frame;
	for(i=0; i<anim_set->num_anim_sprites; i++) {
		anim_sprite = &(anim_set->anim_sprite[i]);
		fread(&(anim_sprite->num_frames), sizeof(anim_sprite->num_frames), 1, fp);
		fread(&(anim_sprite->frame_restart), sizeof(anim_sprite->frame_restart), 1, fp);
		anim_sprite->frames = (ise_anim_frame_t*) malloc(anim_sprite->num_frames*sizeof(ise_anim_frame_t));
		for(j=0; j<anim_sprite->num_frames; j++) {
			frame = &(anim_sprite->frames[j]);
			fread(frame, sizeof(uint16_t), 8, fp);
			fread(&sp_index, sizeof(sp_index), 1, fp);
			frame->fsp = &(anim_set->fsp[sp_index]);
			if(frame->xform != ISE_XFORM_NONE && ((frame->fsp->xforms >> (frame->xform*4)) & 0xF) == 0x0) {
				int max_xform = 0, k;
				uint32_t xforms = frame->fsp->xforms;
				for(k=0; k<8; k++) {
					if(((xforms >> (4*k)) & 0xF) > max_xform) max_xform = ((xforms >> (4*k)) & 0xF);
				}
				max_xform++;
				frame->fsp->xforms |= max_xform << (frame->xform*4);
			}
		}
	}

	fclose(fp);

	if(ise_svga_draw_sprite && ise_svga_upload_fast_sprite_xform && (ise_screen_mode.mode >= 0)) {
		for(i=0; i<anim_set->num_sprites; i++) {
			ise_svga_upload_fast_sprite_xform(gfx, &(anim_set->fsp[i]));
		}
	}
}

void ise_free_anim_set(ise_anim_set_t* anim_set)
{
	uint16_t i;
	ise_anim_sprite_t* anim_sprite;
	for(i=0; i<anim_set->num_anim_sprites; i++) {
		anim_sprite = &(anim_set->anim_sprite[i]);
		free(anim_sprite->frames);
	}
	for(i=0; i<anim_set->num_sprites; i++) {
		ise_free_sprite(&(anim_set->fsp[i].sprite));
	}
	free(anim_set->anim_sprite);
	free(anim_set->fsp);
	anim_set->num_sprites = 0;
	anim_set->num_anim_sprites = 0;
}

void ise_clear_anim_objects(ise_gfx_t* gfx, uint32_t num_objs, ise_anim_object_t** objs)
{
	int i;
	ise_bbox32_t bbox;
	for(i=num_objs-1; i>=0; i--) {
		ise_anim_sprite_t* asprite = &(objs[i]->anim_sprite[objs[i]->anim]);
		ise_anim_frame_t* frame = &(asprite->frames[objs[i]->frame]);
		bbox.min_x = objs[i]->pos_x - gfx->map.origin_x - frame->origin_x;
		bbox.min_y = objs[i]->pos_y - gfx->map.origin_y - frame->origin_y;
		bbox.width = frame->fsp->sprite.width;
		bbox.height = frame->fsp->sprite.height;
		ise_gfx_clear_map(gfx, &bbox);
	}
}

void ise_draw_anim_objects(ise_gfx_t* gfx, uint32_t num_objs, ise_anim_object_t** objs)
{
	int x, y, i, plane, drawn;
	int num_planes = (ise_screen_mode.mode < 0) ? 4 : 1;
	if(ise_screen_mode.mode >= 0 && !gfx->map_drawn) {
		ise_draw_map(&(gfx->map), gfx->back,  0, 0, gfx->bg_color, ISE_MASK_NONE);
		gfx->map_drawn = true;
		if(ise_svga_wait_idle) ise_svga_wait_idle();
	}
	for(plane=0; plane<num_planes; plane++) {
		if(num_planes > 1) ise_set_plane(plane);
		for(i=num_objs-1; i>=0; i--) {
			if((objs[i]->flags & 0x1) && gfx->num_back_sprites < MAX_SPRITES) {
				ise_anim_sprite_t* asprite = &(objs[i]->anim_sprite[objs[i]->anim]);
				ise_anim_frame_t* frame = &(asprite->frames[objs[i]->frame]);
				x = objs[i]->pos_x - gfx->map.origin_x - frame->origin_x;
				y = objs[i]->pos_y - gfx->map.origin_y - frame->origin_y;
				if(num_planes == 1 && ise_svga_draw_sprite && frame->fsp->vmem_offset != ~0x0) {
					ise_svga_draw_sprite(frame->fsp, gfx->back, x, y, frame->xform);
				} else {
					drawn = ise_draw_sprite(&(frame->fsp->sprite), gfx->back, x, y, frame->xform, -1, plane);
					if(num_planes > 1 && plane == 0 && drawn) {
						ise_bbox32_t* bbox = &(gfx->back_sprite_bbox[gfx->num_back_sprites]);
						bbox->min_x = x;
						bbox->min_y = y;
						bbox->width = frame->fsp->sprite.width;
						bbox->height = frame->fsp->sprite.height;
						gfx->num_back_sprites++;
					}
				}
			}
		}
	}
}

bool ise_animate_object(ise_anim_object_t* obj, uint16_t t)
{
	bool done = false;
	ise_anim_sprite_t* asprite = &(obj->anim_sprite[obj->anim]);
	if(obj->frame >= asprite->num_frames) {
		obj->frame = asprite->frame_restart;
		done = true;
	}

	ise_anim_frame_t* frame = &(asprite->frames[obj->frame]);
	obj->frame_time += t;
	while(obj->frame_time > frame->duration) {
		obj->frame_time -= frame->duration;
		obj->frame++;
		if(obj->frame >= asprite->num_frames) {
			obj->frame = asprite->frame_restart;
			done = true;
		}
		frame = &(asprite->frames[obj->frame]);
	}
	return done;
}

void ise_set_animation(ise_anim_object_t* obj, uint16_t a, bool restart)
{
	if(a != obj->anim) {
		obj->anim = a;
		if(restart) {
			obj->frame = 0;
			obj->frame_time = 0;
		} else {
			ise_animate_object(obj, 0);
		}
	}
}

void ise_move_obj(ise_anim_object_t* obj)
{
	int32_t vec_x = obj->frac_pos_x + obj->vec_x;
	int32_t vec_y = obj->frac_pos_y + obj->vec_y;
	obj->frac_pos_x += vec_x & 0xFFFF;
	obj->frac_pos_y += vec_y & 0xFFFF;
	obj->pos_x += vec_x >> 16;
	obj->pos_y += vec_y >> 16;
}

uint32_t ise_check_obj_collision(ise_anim_object_t* src, ise_anim_object_t* dst)
{
	uint32_t collision_type = 0;
	int32_t vec_x, vec_y;
	ise_bbox_t* src_bbox = &(src->anim_sprite[src->anim].frames[src->frame].hitbox);
	ise_bbox_t* dst_bbox = &(dst->anim_sprite[dst->anim].frames[dst->frame].hitbox);
	vec_x = src->frac_pos_x + src->vec_x;
	vec_y = src->frac_pos_y + src->vec_y;
	uint32_t src_min_x = src->pos_x + src_bbox->min_x + (vec_x >> 16);
	uint32_t src_min_y = src->pos_y + src_bbox->min_y + (vec_y >> 16);
	uint32_t src_max_x = src_min_x + src_bbox->width - 1;
	uint32_t src_max_y = src_min_y + src_bbox->height - 1;
	vec_x = dst->frac_pos_x + dst->vec_x;
	vec_y = dst->frac_pos_y + dst->vec_y;
	uint32_t dst_min_x = dst->pos_x + dst_bbox->min_x + (vec_x >> 16);
	uint32_t dst_min_y = dst->pos_y + dst_bbox->min_y + (vec_y >> 16);
	uint32_t dst_max_x = dst_min_x + dst_bbox->width - 1;
	uint32_t dst_max_y = dst_min_y + dst_bbox->height - 1;
	//if(src_max_x >= dst_min_x && src_max_x <= dst_max_x) collision_type |= 0x1;
	//if(src_min_x >= dst_min_x && src_min_x <= dst_max_x) collision_type |= 0x2;
	//if(src_max_y >= dst_min_y && src_max_y <= dst_max_y) collision_type |= 0x4;
	//if(src_min_y >= dst_min_y && src_min_y <= dst_max_y) collision_type |= 0x8;
	//if(((collision_type & 0x3) == 0) || ((collision_type & 0xc) == 0)) collision_type = 0x0;
	if(src_max_x >= dst_min_x && src_min_x <= dst_max_x && src_max_y >= dst_min_y && src_min_y <= dst_max_y) {
		collision_type = 0xf;
		if(src_max_x > dst_max_x) collision_type &= ~0x1;
		if(src_min_x < dst_min_x) collision_type &= ~0x2;
		if(src_max_y > dst_max_y) collision_type &= ~0x4;
		if(src_min_y < dst_min_y) collision_type &= ~0x8;
	}
	return collision_type;
}


void ise_setup_fast_sprite_draw()
{
    // OR source data with latched data
    outpw(ISE_VGA_GRAPHICS_ADDR, ISE_VGA_GRAPHICS_DATA_ROTATE | 0x1000);
    // write to all bit planes
    outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MAP_MASK | 0xf00);
}

void ise_draw_fast_sprite(ise_fast_sprite_t* fsp, ise_framebuffer_t* fb, int x, int y, uint16_t mask_flags)
{
    uint8_t data;

    int aligned_x = (x & ~3) | ((fb->hpan ^ (fb->hpan << 1)) & 3);
    
    uint16_t fb_y = (y < 0) ? 0 : y;
    uint16_t sp_y = (y < 0) ? -y : 0;
    uint16_t fb_end_y = fb_y + (fsp->sprite.height - sp_y);
    if(fb_end_y > fb->height) fb_end_y = fb->height;
    uint16_t fb_x = (aligned_x < 0) ? 0 : aligned_x;
    uint16_t sp_x = (aligned_x < 0) ? -aligned_x : 0;
    uint16_t fb_end_x = fb_x + (fsp->sprite.width - sp_x);
    if(fb_end_x > fb->width) fb_end_x = fb->width;

    // check if entire sprite is out of framebuffer
    if((fb_y > fb->height) || (fb_x > fb->width) || (sp_y > fsp->sprite.height) || (sp_x > fsp->sprite.width)) return;

    uint16_t fb_offset, sp_offset, sp_mask_offset;
    uint16_t sp_mask_pitch = fsp->sprite.width;//(fsp->sprite.width + 7) / 8;
    uint16_t edge_mask, mask, sp_mask, last_mask;
    edge_mask = 0xf00;
    last_mask = 0xf00;
    bool sprite_mask_needed = (mask_flags & ISE_MASK_SPRITE);
    bool edge_mask_needed = (mask_flags & ISE_MASK_EDGE);
    if(fb->pitch - fb->width < 4) edge_mask_needed = true;

    uint16_t fb_col_offset, sp_col_offset, sp_mask_col_offset;
    fb_col_offset = (fb_y * (fb->pitch/4)) + ((fb_x + fb->hpan) / 4);
    sp_col_offset = (sp_y * (fsp->sprite.width/4)) + (sp_x / 4);
    sp_mask_col_offset = sp_x + (sp_y * sp_mask_pitch); //((sp_y * sp_mask_pitch) + (sp_x / 8));
    
    sp_col_offset += (uint16_t) (fsp->aperture->base + fsp->vmem_offset);
    uint16_t sp_aperture_offset;
    for(; fb_x < fb_end_x; fb_x += 4, sp_x += 4) {
        if(edge_mask_needed) {
            // take care of right edge of screen
            edge_mask = ((fb_x & ~3) < (fb_end_x & ~3)) ? 0xf00 : ~(0xf00 << (fb_end_x - fb_x)) & 0xf00;
            // take care of left edge of screen
            edge_mask &= ((fb_x & ~3) > 0) ? 0xf00 : (0xf00 << fb->hpan);
        }
        fb_y = (y < 0) ? 0 : y;
        sp_y = (y < 0) ? -y : 0;

        fb_offset = fb_col_offset;
        sp_offset = sp_col_offset;
        sp_mask_offset = sp_mask_col_offset;
        for(; fb_y < fb_end_y; fb_y++, sp_y++) {
            mask = edge_mask;
            if(sprite_mask_needed && fsp->sprite.mask) {
                //sp_mask_offset = ((sp_y * sp_mask_pitch) + (sp_x / 8));
                sp_mask = (uint8_t) ((fsp->sprite.mask[sp_mask_offset / 32]) >> (sp_mask_offset & 0x18));
                sp_mask = (sp_mask << (8 - (sp_mask_offset & 4)));
                mask &= sp_mask;
            }
            if(mask) {
                if(mask != last_mask) {
                    outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MAP_MASK | mask);
                    last_mask = mask;
                }
                //sp_offset = (sp_y * (fsp->sprite.width/4)) + (sp_x / 4);
                //fb_offset = (fb_y * (fb->pitch/4)) + ((fb_x + fb->hpan) / 4);
                sp_aperture_offset = sp_offset;
                if(sp_aperture_offset >= fsp->aperture->size) sp_aperture_offset = (uint16_t) (sp_aperture_offset - fsp->aperture->size);
                data = *(ISE_VGA_BASE(fsp->aperture->start + sp_aperture_offset));
                *(ISE_VGA_BASE(fb->vmem_offset + fb_offset)) = 0x0;
            }
            fb_offset += fb->pitch/4;
            sp_offset += fsp->sprite.width / 4;
            sp_mask_offset += sp_mask_pitch;
        }
        fb_col_offset++;
        sp_col_offset++;
        if(sp_x & 4) sp_mask_col_offset++;
    }
    if(last_mask != 0xf00) {
        // write to all bit planes
        outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MAP_MASK | 0xf00);
    }

}

void ise_shutdown_fast_sprite_draw()
{
    // Go back to regular write mode
    outpw(ISE_VGA_GRAPHICS_ADDR, ISE_VGA_GRAPHICS_DATA_ROTATE | 0x0000);
}


