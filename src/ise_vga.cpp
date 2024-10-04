// InfiniScroll Engine
// ise_vga.cpp
// VGA adapter related code
//
// Kamal Pillai
// 1/13/2019

#include "ise.h"

uint8_t* ise_svga_ctrl_base = NULL;

uint16_t ise_vga_crtc_addr = ISE_VGA_CRTC0_ADDR;
uint16_t ise_vga_mode = ISE_VGA_MODE_80X25;

void (*ise_svga_set_rgb8_palette)() = NULL;
int (*ise_svga_set_mode)(ise_gfx_t* gfx) = NULL;
void (*ise_svga_setup_mode)(ise_gfx_t* gfx) = NULL;
void (*ise_svga_update_display)(ise_gfx_t* gfx) = NULL;
void (*ise_svga_upload_fast_sprite_xform)(ise_gfx_t* gfx, ise_fast_sprite_t* fsp) = NULL;
void (*ise_svga_fill)(ise_framebuffer_t* fb, int x, int y, uint16_t width, uint16_t height, uint32_t color) = NULL;
void (*ise_svga_draw_fast_sprite)(ise_fast_sprite_t* fsp, ise_framebuffer_t* fb, int x, int y) = NULL;
void (*ise_svga_draw_sprite)(ise_fast_sprite_t* fsp, ise_framebuffer_t* fb, int x, int y, uint16_t xform) = NULL;
void (*ise_svga_wait_idle)() = NULL;
void (*ise_svga_uninstall)() = NULL;

void ise_svga_install()
{
    int s, f;
    for(s=0; s<ISE_PCI_MAX_SLOTS; s++) {
        for(f=0; f<ISE_PCI_MAX_FUNC; f++) {
			if(ise_svga_ctrl_base) return;

            // Look for 3dfx cards we support
			if(ise_pci.slot[s][f].vendor_id == ISE_PCI_VENDOR_3DFX) {
				ise_3dfx_install(s, f);
			}
		}
	}

    for(s=0; s<ISE_PCI_MAX_SLOTS; s++) {
        for(f=0; f<ISE_PCI_MAX_FUNC; f++) {
			if(ise_svga_ctrl_base) return;

            // Look for vga cards we support
			switch(ise_pci.slot[s][f].vendor_id) {
			case ISE_PCI_VENDOR_MATROX:
				ise_mga_install(s, f);
				break;
			case ISE_PCI_VENDOR_ATI:
				ise_ati_install(s, f);
				break;
			case ISE_PCI_VENDOR_S3:
				ise_s3_install(s, f);
				break;
			}
		}
	}
}

void ise_vga_get_timing()
{
	// First, try to get EDID from monitor, if supported
	ise_regs_t regs = {0};
	regs.eax = 0x4F15;
	ise_rm_interrupt(0x10, &regs);
	if((regs.eax & 0xFFFF) == 0x004F) {
		// Call is successful
		uint32_t* edid_info;
		edid_info = (uint32_t*) ise_mem_aligned_malloc(128, 0, ISE_MEM_REGION_DOS);
		uint32_t addr = (uint32_t) edid_info;
		regs.eax = 0x4F15;
		regs.ebx = 0x1;
		regs.edi = addr &0xF;
		regs.es = (uint16_t) (addr >> 4);
		ise_rm_interrupt(0x10, &regs);
		ise_mem_dump(NULL, edid_info, 16);
		ise_mem_aligned_free(edid_info);
	} else {
		// Could not get edid info...use calculate GTF
		printf("EDID info not supported: 0x%x\n", regs.eax);
	}
		
}

void ise_vga_set_mode(int16_t mode)
{
    union REGS regs;

    // determine crtc register
    ise_vga_crtc_addr = (inp(ISE_VGA_MISC_OUT_READ) & 1) ? ISE_VGA_CRTC1_ADDR : ISE_VGA_CRTC0_ADDR;

    if(mode == ISE_VGA_MODE_80X25) {
        regs.h.ah = 0x00;
        regs.h.al = 0x03;
        INT86(0x10, &regs, &regs);
        //ise_vga_reset_attr_reg();
    } else {
        regs.h.ah = 0x00;
        regs.h.al = 0x13;
        INT86(0x10, &regs, &regs);

        // get out of chain 4 mode
        outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MEMORY_MODE | 0x0600);
        
        // synchronous reset
        outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_RESET | 0x0100);
        // select 25 MHz dot clock & 60 Hz scanning rate
        outp(ISE_VGA_MISC_OUT_WRITE, 0xe3);
        // restart sequencer
        outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_RESET | 0x0300);

        if(mode == ISE_VGA_MODE_320X240X8BPP) {
            // Remove write protection
            outp(ise_vga_crtc_addr, ISE_VGA_CRTC_VERT_RETRACE_END);
            uint16_t crtc_data = ise_vga_crtc_addr+1;
            uint16_t vert_retrace_data = (uint16_t) inp(crtc_data);
            outp(crtc_data, vert_retrace_data & 0x7f);

            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_VERTICAL_TOTAL | 0x0d00);
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_OVERFLOW | 0x3e00);
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_MAX_SCAN_LINE | 0x4100);
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_VERT_RETRACE_START | 0xea00);
            // re-enables protection
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_VERT_RETRACE_END | 0xac00);
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_VERT_DISPLAY_END | 0xdf00);
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_START_VERT_BLANK | 0xe700);
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_END_VERT_BLANK | 0x0600);
        }

        // turn off dword mode
        outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_UNDERLINE_LOCATION | 0x0000);
        outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_MODE_CONTROL | 0xe300);

        // get out of chain 4 mode
        /*outpw(ISE_VGA_SEQ_ADDR, ISE_VGA_SEQ_MEMORY_MODE | 0x0600);

        outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_UNDERLINE_LOCATION | 0x0000);
        outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_MODE_CONTROL | 0xe300);

        if(mode == ISE_VGA_MODE_320X240X8BPP) {
            // get to 320x240 mode
            // turn off write protection
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_VERT_RETRACE_END | 0x2c00);
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_VERTICAL_TOTAL | 0x0d00);
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_OVERFLOW | 0x3e00);
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_VERT_RETRACE_START | 0xea00);
            // turn on write protection
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_VERT_RETRACE_END | 0xac00);
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_VERT_DISPLAY_END | 0xdf00);
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_START_VERT_BLANK | 0xe700);
            outpw(ise_vga_crtc_addr, ISE_VGA_CRTC_END_VERT_BLANK | 0x0600);
        }*/
        //ise_vga_reset_attr_reg();
    }
    ise_vga_mode = mode;
}

void ise_vga_reset_attr_reg()
{
    uint16_t addr = (uint16_t) inp(ISE_VGA_ATTR_ADDR);
    uint16_t data = (uint16_t) inp(ISE_VGA_ATTR_DATA_READ);
    uint16_t new_addr = (addr == ISE_VGA_ATTR_OVERSCAN_COLOR) ? ISE_VGA_ATTR_HORIZ_PIX_PAN : ISE_VGA_ATTR_OVERSCAN_COLOR;

    outp(ISE_VGA_ATTR_ADDR, new_addr);
    if(inp(ISE_VGA_ATTR_ADDR) == new_addr) {
        outp(ISE_VGA_ATTR_DATA_WRITE, 0);
        *(ISE_VGA_BASE(0)) = 0x04;
    } else {
        outp(ISE_VGA_ATTR_ADDR, addr);
        outp(ISE_VGA_ATTR_DATA_WRITE, data);
        *(ISE_VGA_BASE(0)) = 0x0f;
    }
}

void ise_vga_set_rgb8_palette()
{
    uint16_t color_index, color_value;
    outp(ISE_VGA_COLOR_PALETTE_ADDR, 0x0);
    for(color_index=0; color_index<256; color_index++) {
        // red
        color_value = ((color_index >> 0) & 0x7);
        color_value |= color_value << 3;
        outp(ISE_VGA_COLOR_PALETTE_DATA, color_value);
        // green
        color_value = ((color_index >> 3) & 0x7);
        color_value |= color_value << 3;
        outp(ISE_VGA_COLOR_PALETTE_DATA, color_value);
        // blue
        color_value = ((color_index >> 6) & 0x3);
        color_value |= (color_value << 2) | (color_value << 4);
        outp(ISE_VGA_COLOR_PALETTE_DATA, color_value);
    }
}

void ise_vga_calc_timing(ise_vga_timing_t* t)
{
	if(t == NULL) return;
	
	// refresh not specified, use 60Hz
	if(t->refresh < 50) t->refresh = 60;
	
	if(t->width == 640 && t->height == 480 && t->refresh == 60) {
		t->pixel_clk = 25175;
		t->h_front_porch = 16;
		t->h_sync_pulse = 96;
		t->h_back_porch = 48;
		t->v_front_porch = 10;
		t->v_sync_pulse = 2;
		t->v_back_porch = 33;
		t->actual_refresh = (t->pixel_clk * 1000.0f) / ((t->width + t->h_front_porch + t->h_sync_pulse + t->h_back_porch) * (t->height + t->v_front_porch + t->v_sync_pulse + t->v_back_porch));
		return;
	}

	// all params are in fixed 24.8 format
	float v_field_rate_rqd = (t->flags & ISE_VGA_TIMING_INTERLACE) ? 2 * t->refresh : t->refresh;
	
	// divide by cell gran (assume 8, if not specifies)
	if(t->cell_gran == 0) t->cell_gran = 8;
	uint32_t h_pixels_rnd = (t->width / t->cell_gran) * t->cell_gran;

	uint32_t v_lines_rnd = (t->flags & ISE_VGA_TIMING_INTERLACE) ? t->height / 2 : t->height;

	uint32_t left_margin = 0, right_margin = 0;
	uint32_t top_margin = 0, bottom_margin = 0;
	float vis_aspect = (float) t->width / (float) t->height;
	if((t->flags & ISE_VGA_TIMING_PRESERVE_ASPECT) && t->display_aspect != 0.0f) {
		if(vis_aspect < t->display_aspect) {
			float h_pad_total = (t->height * t->display_aspect) - t->width;
			if(h_pad_total < 0.0f) h_pad_total = 0.0f;
			right_margin = ((uint32_t) h_pad_total) / 2;
			left_margin = left_margin + ((uint32_t) h_pad_total & 0x1);
		} else {
			float v_pad_total = (t->width / t->display_aspect) - t->height;
			if(t->flags & ISE_VGA_TIMING_INTERLACE) v_pad_total *= 0.5f;
			if(v_pad_total < 0.0f) v_pad_total = 0.0f;
			bottom_margin = ((uint32_t) v_pad_total) / 2;
			top_margin = bottom_margin + ((uint32_t) v_pad_total & 0x1);
		}
	}
	uint32_t total_active_pixels = h_pixels_rnd + left_margin + right_margin;
	float interlace = (t->flags & ISE_VGA_TIMING_INTERLACE) ? 0.5f : 0.0f;
	
	const float MIN_VSYNC_BP = 550.0f;
	const uint32_t MIN_V_PORCH_RND = 3.0f;
	float h_period_est = (((1.0f / v_field_rate_rqd) - MIN_VSYNC_BP / 1000000.0f) / (v_lines_rnd + (2.0f*top_margin) + MIN_V_PORCH_RND + interlace)) * 1000000.0f;

	uint32_t v_sync_rnd = 10;   // default for non-standard aspect ratios
	if(vis_aspect == 4.0f/3.0f) v_sync_rnd = 4;
	else if(vis_aspect == 16.0f/9.0f) v_sync_rnd = 5;
	else if(vis_aspect == 16.0f/10.0f) v_sync_rnd = 6;
	else if(vis_aspect == 5.0f/4.0f || vis_aspect == 15.0f/9.0f) v_sync_rnd = 7;

	const uint32_t MIN_V_BPORCH = 6;
	uint32_t v_sync_bp = ((uint32_t) (MIN_VSYNC_BP / h_period_est)) + 1;
	if(v_sync_bp < v_sync_rnd + MIN_V_BPORCH) v_sync_bp = v_sync_rnd + MIN_V_BPORCH;
	
	uint32_t total_v_lines = v_lines_rnd + top_margin + bottom_margin + v_sync_bp + MIN_V_PORCH_RND;
	
	const float C_PRIME = 30.0f;
	const float CLOCK_STEP = 0.25f;
	const float M_PRIME = 300.0f;
	float ideal_duty_cycle = C_PRIME - (M_PRIME * h_period_est / 1000.0f);
	
	uint32_t h_blank;
	if(ideal_duty_cycle < 20.0f) {
		h_blank = ((total_active_pixels * 20 / (100 - 20)) / (2 * t->cell_gran)) * (2* t->cell_gran);
	} else {
		h_blank = (((uint32_t) (total_active_pixels * ideal_duty_cycle / (100 - ideal_duty_cycle))) / (2 * t->cell_gran)) * (2* t->cell_gran);
	}

	uint32_t total_pixels = total_active_pixels + h_blank;
	
	float act_pixel_freq = CLOCK_STEP * (uint32_t) ((total_pixels / h_period_est) / CLOCK_STEP);
	float act_h_freq = 1000.0f * act_pixel_freq / total_pixels;
	
	float act_field_rate = 1000.0f * act_h_freq / total_v_lines;

	const uint32_t H_SYNC_PER = 8;
	t->pixel_clk = (uint32_t) (act_pixel_freq * 1000.0f);
	t->actual_refresh = (t->flags & ISE_VGA_TIMING_INTERLACE) ? act_field_rate / 2.0f : act_field_rate;
	t->h_sync_pulse = ( (H_SYNC_PER * total_pixels) / (100*t->cell_gran)) * t->cell_gran;
	t->h_front_porch = h_blank / 2 - t->h_sync_pulse;
	t->h_back_porch = t->h_front_porch + t->h_sync_pulse;
	t->v_front_porch = total_v_lines - v_lines_rnd - v_sync_bp;
	t->v_sync_pulse = v_sync_rnd;
	t->v_back_porch = v_sync_bp - v_sync_rnd;
}