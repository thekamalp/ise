// InfiniScroll Engine
// ise_3dfx.h
// 3dfx Voodoo/Voodoo2 HW acceleration
//
// Kamal Pillai
// 5/15/2020

#include "ise.h"

uint32_t ise_3dfx_reg_addr_wrap = 0x0;
ise_3dfx_card_t ise_3dfx_card = {0};

// ----------------------------
// MMIO access
inline uint32_t ise_3dfx_mmio_read(uint32_t reg_addr)
{
	return ISE_SVGA_BASE_D(reg_addr);
}

inline void ise_3dfx_wait_fifo(uint32_t entries)
{
	uint32_t capped_entries = (entries > 63) ? 63 : entries;
	while((ise_3dfx_mmio_read(ISE_3DFX_VOODOO_STATUS) & 0x3F) < capped_entries) ;
}

uint32_t ise_3dfx_count_bits(uint32_t* v, uint32_t max_count)
{
	int i, count = 0;
	for(i=0; i<32 && count < max_count; i++) {
		if(*v & (0x1 << i)) {
			count++;
			*v &= ~(0x1 << i);
		}
	}
	return count;
}

inline uint32_t ise_3dfx_get_read_offset()
{
	return ISE_3DFX_CMD_BASE + ISE_SVGA_BASE_D(ISE_3DFX_VOODOO_CMD_FIFO_RD_PTR) - ise_3dfx_card.cmd_fifo_base;
}

// Send a command packet to command fifo
// Ensures packet fits to end of command fifo, and if not, breaks it apart
// Ensures that writes to command fifo do not over run the hardware read pointer
void ise_3dfx_send_packet(uint32_t pkt_hdr, uint32_t* pkt_data)
{
	uint32_t pkt_hdr0, pkt_hdr1 = pkt_hdr;
	uint32_t pkt_data_size;
	uint32_t vec;
	uint32_t addr = ~0;

	// Only applies for Type 3 packet
	uint32_t num_verts, vertex_size;
	if((pkt_hdr & 0x7) == 0x3) {
		vertex_size = 2; // X,Y
		if(pkt_hdr & 0x1000000) {
			if(pkt_hdr & 0xC00) vertex_size++;     // Packed RGBA
		} else {
			if(pkt_hdr & 0x400) vertex_size += 3;  // RGB
			if(pkt_hdr & 0x800) vertex_size++;     // Alpha
		}
		if(pkt_hdr & 0x1000) vertex_size++;        // Z
		if(pkt_hdr & 0x2000) vertex_size++;        // Wb
		if(pkt_hdr & 0x4000) vertex_size++;        // W0
		if(pkt_hdr & 0x8000) vertex_size += 2;     // S0, T0
		if(pkt_hdr & 0x10000) vertex_size++;       // W1
		if(pkt_hdr & 0x20000) vertex_size += 2;    // S1, T1
	}

	while(pkt_hdr1) {
		pkt_hdr0 = pkt_hdr1;
		pkt_hdr1 = 0;
		int max_pkt_data_size = ise_3dfx_card.cmd_fifo_size - ise_3dfx_card.cmd_fifO_wr_entry - 1;
		switch(pkt_hdr0 & 0x7) {
		case 0:  // Type 0 packets have no payload, except JMP_AGP instruction
			if(((pkt_hdr0 >> 3) & 0x7) == 0x4) {  // JUMP_AGP
				pkt_data_size = 1;
			}
			if(pkt_data_size > max_pkt_data_size) {
				pkt_hdr1 = pkt_hdr0;
				pkt_hdr0 = 0;
				pkt_data_size = 0;
			}
			break;
		case 1:
			pkt_data_size = pkt_hdr0 >> 16;
			if(pkt_data_size > max_pkt_data_size) {
				pkt_hdr0 &= 0xFFFF;
				pkt_hdr0 |= max_pkt_data_size << 16;
				if(pkt_hdr0 & 0x8000) {
					pkt_hdr1 = 0x8001;
					pkt_hdr1 |= (((pkt_hdr0 >> 3) & 0xFFF) + max_pkt_data_size) << 3;
					pkt_hdr1 |= (pkt_data_size - max_pkt_data_size) << 16;
				}
				pkt_data_size = max_pkt_data_size;
			}
			break;
		case 2:
			vec = pkt_hdr0 >> 3;
			pkt_data_size = ise_3dfx_count_bits(&vec, max_pkt_data_size);
			if(vec) {
				pkt_hdr0 &= ~(vec << 3);
				pkt_hdr1 = 0x2 | (vec << 3);
			}
			break;
		case 3:
			num_verts = (pkt_hdr0 >> 6) & 0xF;
			pkt_data_size = num_verts * vertex_size;
			if(pkt_data_size > max_pkt_data_size) {
				uint32_t new_num_verts = max_pkt_data_size / vertex_size;
				if(((pkt_hdr0 >> 3) & 0x7) == ISE_3DFX_PRIM_TRIANGLES) new_num_verts = 3*(new_num_verts/3);
				pkt_hdr1 = pkt_hdr0;
				if(((pkt_hdr1 >> 3) & 0x7) == ISE_3DFX_PRIM_TRIANGLE_STRIP) {
					pkt_hdr1 &= ~0x38;
					pkt_hdr1 |=  ISE_3DFX_PRIM_TRIANGLE_STRIP_CONT << 3;
				}
				pkt_hdr1 &= ~0x3C0;
				pkt_hdr1 |= (num_verts - new_num_verts) << 6;
				if(new_num_verts == 0) {
					pkt_hdr0 = 0;
				} else {
					pkt_hdr0 &= 0x1FFFC3F;
					pkt_hdr0 |= new_num_verts << 6;
				}
				pkt_data_size = new_num_verts * vertex_size;
			} else {
				uint32_t pad = pkt_hdr0 >> 29;
				if(pkt_data_size + pad > max_pkt_data_size) {
					pad = max_pkt_data_size - pkt_data_size;
					pkt_hdr0 &= 0x1FFFFFF;
					pkt_hdr0 |= pad << 29;
				}
				pkt_data_size += pad;
			}
			break;
		case 4:
			vec = (pkt_hdr0 >> 15) & 0x3FFF;
			pkt_data_size = ise_3dfx_count_bits(&vec, max_pkt_data_size);
			if(vec == 0) {
				uint32_t pad = pkt_hdr0 >> 29;
				if(pkt_data_size + pad > max_pkt_data_size) {
					pad = max_pkt_data_size - pkt_data_size;
					pkt_hdr0 &= 0x1FFFFFFF;
					pkt_hdr0 |= pad << 29;
				}
				pkt_data_size += pad;
			} else {
				pkt_hdr1 = (pkt_hdr0 & 0xE0007FFF) | (vec << 15);
				pkt_hdr0 &= 0x1FFFFFFF;
				pkt_hdr0 &= ~(vec << 15);
			}
			break;
		case 5:
			if(addr == ~0) {
				addr = *pkt_data;
				pkt_data++;
			}
			pkt_data_size = ((pkt_hdr0 >> 3) & 0x7FFFF);
			if(max_pkt_data_size <= 1) {
				pkt_hdr1 = pkt_hdr0;
				pkt_hdr0 = 0;
				pkt_data_size = 0;
			} else if(pkt_data_size > max_pkt_data_size-1) {
				pkt_hdr0 &= 0xFFC00007;
				pkt_hdr1 = pkt_hdr0;
				pkt_hdr0 |= (max_pkt_data_size-1) << 3;
				pkt_hdr1 |= (pkt_data_size - max_pkt_data_size+1) << 3;
				pkt_data_size = max_pkt_data_size-1;
			}
			break;
		}
		uint32_t end_offset = ise_3dfx_get_read_offset() - 4;
		uint32_t offset = ISE_3DFX_CMD_BASE + (ise_3dfx_card.cmd_fifO_wr_entry << 2);
		if(pkt_hdr0) {
			while(offset == end_offset) end_offset = ise_3dfx_get_read_offset() - 4;
			ISE_SVGA_BASE_D(offset) = pkt_hdr0;
			offset += 4;
			if(addr != ~0) {
				while(offset == end_offset) end_offset = ise_3dfx_get_read_offset() - 4;
				ISE_SVGA_BASE_D(offset) = addr;
				offset += 4;
				addr += pkt_data_size * 4;
			}
			uint32_t i;
			for(i=0; i<pkt_data_size; i++) {
				while(offset == end_offset) end_offset = ise_3dfx_get_read_offset() - 4;
				ISE_SVGA_BASE_D(offset) = *pkt_data;
				offset += 4;
				pkt_data++;
			}
			ise_3dfx_card.cmd_fifO_wr_entry += pkt_data_size+1 + ((addr != ~0) ? 1 : 0);
		}
		if(ise_3dfx_card.cmd_fifO_wr_entry + 2 >= ise_3dfx_card.cmd_fifo_size || pkt_hdr0 == 0) {
			uint32_t jmp_pkt = (ise_3dfx_card.cmd_fifo_base << 4) | 0x18;
			uint32_t read_offset = end_offset + 4;
			while(read_offset == ISE_3DFX_CMD_BASE) read_offset = ise_3dfx_get_read_offset();
			ISE_SVGA_BASE_D(offset) = jmp_pkt;
			//__asm {
			//	mfence
			//}
			ise_3dfx_card.cmd_fifO_wr_entry = 0;
		}
	}
}

void ise_3dfx_mmio_write(uint32_t reg_addr, uint32_t val, uint32_t chip_sel = ISE_3DFX_VOODOO_CHIP_ALL)
{
	uint32_t addr = reg_addr;
	uint32_t pkt_hdr;
	if(ise_3dfx_card.flags & ISE_3DFX_CARD_CMD_FIFO_EN) {
		switch(reg_addr) {
		case ISE_3DFX_VOODOO_FBI_INIT_0:
		case ISE_3DFX_VOODOO_FBI_INIT_1:
		case ISE_3DFX_VOODOO_FBI_INIT_2:
		case ISE_3DFX_VOODOO_FBI_INIT_3:
		case ISE_3DFX_VOODOO_FBI_INIT_4:
		case ISE_3DFX_VOODOO_FBI_INIT_5:
		case ISE_3DFX_VOODOO_FBI_INIT_6:
		case ISE_3DFX_VOODOO_FBI_INIT_7:
		case ISE_3DFX_VOODOO_INTR_CTRL:
		case ISE_3DFX_VOODOO_BACK_PORCH:
		case ISE_3DFX_VOODOO_VIDEO_DIMENSIONS:
		case ISE_3DFX_VOODOO_DAC_DATA:
		case ISE_3DFX_VOODOO_H_SYNC:
		case ISE_3DFX_VOODOO_V_SYNC:
		case ISE_3DFX_VOODOO_MAX_RGB_DELTA:
		case ISE_3DFX_VOODOO_H_BORDER:
		case ISE_3DFX_VOODOO_V_BORDER:
		case ISE_3DFX_VOODOO_BORDER_COLOR:
		case ISE_3DFX_VOODOO_CMD_FIFO_BASE_ADDR:
		case ISE_3DFX_VOODOO_CMD_FIFO_BUMP:
		case ISE_3DFX_VOODOO_CMD_FIFO_RD_PTR:
		case ISE_3DFX_VOODOO_CMD_FIFO_A_MIN:
		case ISE_3DFX_VOODOO_CMD_FIFO_A_MAX:
		case ISE_3DFX_VOODOO_CMD_FIFO_DEPTH:
		case ISE_3DFX_VOODOO_CMD_FIFO_HOLES:
			break;
		default:
			pkt_hdr = 0x1 | (reg_addr << 1 ) | (chip_sel << 1) | (0x1 << 16);
			ise_3dfx_send_packet(pkt_hdr, &val);
			return;
		}
	} else {
		addr |= ise_3dfx_reg_addr_wrap | chip_sel;
		switch(reg_addr) {
		case ISE_3DFX_VOODOO_FBI_INIT_0:
		case ISE_3DFX_VOODOO_FBI_INIT_1:
		case ISE_3DFX_VOODOO_FBI_INIT_2:
		case ISE_3DFX_VOODOO_FBI_INIT_3:
		case ISE_3DFX_VOODOO_FBI_INIT_4:
		case ISE_3DFX_VOODOO_FBI_INIT_5:
		case ISE_3DFX_VOODOO_FBI_INIT_6:
		case ISE_3DFX_VOODOO_FBI_INIT_7:
		case ISE_3DFX_VOODOO_INTR_CTRL:
		case ISE_3DFX_VOODOO_BACK_PORCH:
		case ISE_3DFX_VOODOO_VIDEO_DIMENSIONS:
		case ISE_3DFX_VOODOO_DAC_DATA:
		case ISE_3DFX_VOODOO_H_SYNC:
		case ISE_3DFX_VOODOO_V_SYNC:
		case ISE_3DFX_VOODOO_MAX_RGB_DELTA:
		case ISE_3DFX_VOODOO_H_BORDER:
		case ISE_3DFX_VOODOO_V_BORDER:
		case ISE_3DFX_VOODOO_BORDER_COLOR:
		case ISE_3DFX_VOODOO_CMD_FIFO_BASE_ADDR:
		case ISE_3DFX_VOODOO_CMD_FIFO_BUMP:
		case ISE_3DFX_VOODOO_CMD_FIFO_RD_PTR:
		case ISE_3DFX_VOODOO_CMD_FIFO_A_MIN:
		case ISE_3DFX_VOODOO_CMD_FIFO_A_MAX:
		case ISE_3DFX_VOODOO_CMD_FIFO_DEPTH:
		case ISE_3DFX_VOODOO_CMD_FIFO_HOLES:
		case ISE_3DFX_VOODOO_TRIANGLE_CMD:
		case ISE_3DFX_VOODOO_FTRIANGLE_CMD:
		case ISE_3DFX_VOODOO_NOP_CMD:
		case ISE_3DFX_VOODOO_FASTFILL_CMD:
		case ISE_3DFX_VOODOO_SWAPBUFFER_CMD:
		case ISE_3DFX_VOODOO_S_BEGIN_TRI_CMD:
		case ISE_3DFX_VOODOO_S_DRAW_TRI_CMD:
			ise_3dfx_reg_addr_wrap += ISE_3DFX_VOODOO_WRAP_START;
			ise_3dfx_reg_addr_wrap &= ISE_3DFX_VOODOO_WRAP_MASK;
			break;
		}
	}
	ISE_SVGA_BASE_D(addr) = val;
}

// ----------------------------
// GFX idle
void ise_3dfx_wait_idle()
{
	int i=0;
	while(i<0) {
		if((ise_3dfx_mmio_read(ISE_3DFX_VOODOO_STATUS) & 0x80) == 0) {
			i++;
		}
	}
}

// ----------------------------
// initializtion enables in PCI config space
void ise_3dfx_pci_init_en(uint32_t init_flags)
{
	uint32_t v = ise_pci_read_config(0, ise_3dfx_card.slot, ise_3dfx_card.func, ISE_3DFX_PCI_INIT_ENABLE);
	v &= ~0x7;
	v |= (init_flags & 0x7);
	ise_pci_write_config32(0, ise_3dfx_card.slot, ise_3dfx_card.func, ISE_3DFX_PCI_INIT_ENABLE, v);
}

void ise_3dfx_vclk_en(bool enable)
{
	uint8_t addr = (enable) ? ISE_3DFX_PCI_VCLK_ENABLE : ISE_3DFX_PCI_VCLK_DISABLE;
	ise_pci_write_config32(0, ise_3dfx_card.slot, ise_3dfx_card.func, addr, 0);
}

void ise_3dfx_blank()
{
	uint32_t data;
	ise_3dfx_vclk_en(true);
	ise_3dfx_pci_init_en(ISE_3DFX_PCI_INIT_WRITE_EN);
	
	data = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_1);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_1, data | 0x100);
	ise_3dfx_wait_idle();

	data = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_0);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_0, data | 0x6);
	ise_3dfx_wait_idle();
	
	data = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_2);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_2, data & ~0x400000);
	ise_3dfx_wait_idle();
}

// ----------------------------
// DAC access
void ise_3dfx_dac_write(uint32_t reg_addr, uint32_t val)
{
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_DAC_DATA, (reg_addr << 8) | (val & 0xFF));
	ise_3dfx_wait_idle();
}

uint32_t ise_3dfx_dac_read(uint32_t reg_addr)
{
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_DAC_DATA, (reg_addr << 8) | 0x800);
	ise_3dfx_wait_idle();
	return ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_2) & 0xFF;
}

void ise_3dfx_dac_idx_write(uint32_t reg_addr, uint32_t val)
{
	ise_3dfx_dac_write(0, reg_addr);
	ise_3dfx_dac_write(2, val);
}

uint32_t ise_3dfx_dac_idx_read(uint32_t reg_addr)
{
	ise_3dfx_dac_write(0, reg_addr);
	return ise_3dfx_dac_read(2);
}

// Activate DAC backdoor
void ise_3dfx_dac_backdoor_en()
{
	ise_3dfx_dac_write(0, 0);
	ise_3dfx_dac_read(2);
	ise_3dfx_dac_read(2);
	ise_3dfx_dac_read(2);
	ise_3dfx_dac_read(2);
}

// Probe ICS
uint32_t ise_3dfx_probe_ics5432()
{
	uint32_t i, m01, m07, m11;
	
	for(i = 0; i < 5; i++) {
		ise_3dfx_dac_write(7, 1);
		m01 = ise_3dfx_dac_read(5);
		ise_3dfx_dac_read(5);
		ise_3dfx_dac_write(7, 7);
		m07 = ise_3dfx_dac_read(5);
		ise_3dfx_dac_read(5);
		ise_3dfx_dac_write(7, 11);
		m11 = ise_3dfx_dac_read(5);
		ise_3dfx_dac_read(5);

		if(m01 == ISE_3DFX_ICS_PLL_CLK0_1_INI &&
		   m07 == ISE_3DFX_ICS_PLL_CLK0_7_INI &&
		   m11 == ISE_3DFX_ICS_PLL_CLK1_B_INI)
		   return 1;
	}
	return 0;
}

uint8_t ise_3dfx_find_dac()
{
	uint32_t vendor_id, device_id;
	
	ise_3dfx_dac_backdoor_en();	
	ise_3dfx_dac_read(2);
	vendor_id = ise_3dfx_dac_read(2);
	device_id = ise_3dfx_dac_read(2);
	
	// AT&T 20C409 and clones
	if(vendor_id == ISE_3DFX_DAC_VENDOR_ATT && device_id == ISE_3DFX_DAC_DEVICE_ATT20C409)
		return ISE_3DFX_DAC_ID_ATT;
		
	if(vendor_id == ISE_3DFX_DAC_VENDOR_TI && device_id == ISE_3DFX_DAC_DEVICE_TITVP3409)
		return ISE_3DFX_DAC_ID_TI;
		
	// ICS5432 doesn't implement the back door. Glide does some
	// quick tests to see if it is an ICS5432 just in case.
	if(ise_3dfx_probe_ics5432())
		return ISE_3DFX_DAC_ID_ICS;
	
	// Shouldn't be any boards that get this far
	return ISE_3DFX_DAC_ID_UNKNOWN;
}


//  Compute the PLL clock values. This is directly based on the
//  technique used by sstfb. 

 
//  compute the m,n,p  , returns the real freq
//  (ics datasheet :  N <-> N1 , P <-> N2)
//  
//  Fout= Fref * (M+2)/( 2^P * (N+2))
//   we try to get close to the asked freq
//   with P as high, and M as low as possible
//   range:
//  ti/att : 0 <= M <= 255; 0 <= P <= 3; 0<= N <= 63
//  ics    : 1 <= M <= 127; 0 <= P <= 3; 1<= N <= 31
//  
//   We will use the lowest limitation, should be precise enough
//  

int ise_3dfx_sst_calc_pll(int freq, ise_3dfx_pll_clk_t* pll_clk)
{
	int m, m2, n, p, best_err, fout;
	int best_n=-1;
	int best_m=-1;
	int err;

	best_err = freq;
	p=3;
	// f * 2^P = vco should be less than VCOmax ~ 250 MHz for ics
	while (((1 << p) * freq > 260000) && (p >= 0))
		p--;
	if (p == -1)
		return 0;
	for (n = 1; n < 32; n++) {
		// Calc 2 * m so we can round it later
		m2 = (2 * freq * (1 << p) * (n + 2) ) / 14318 - 4 ;

		m = (m2 % 2) ? m2/2+1 : m2/2 ;
		if (m >= 128)
			break;
		fout = (14318 * (m + 2)) / ((1 << p) * (n + 2));
		err = fout - freq;
		if(err < 0) err = -err;
		if (err < best_err && m > 0) {	
			best_n = n;
			best_m = m;
			best_err = err;
			// We get the lowest m , allowing 0.5% error in freq
			if (200*best_err < freq)
				break;
		}
	}
	if (best_n == -1)  // Unlikely, but who knows ? 
		return 0;
	pll_clk->p=p;
	pll_clk->n=best_n;
	pll_clk->m=best_m;
	return (14318 * (pll_clk->m + 2)) / ((1 << pll_clk->p) * (pll_clk->n + 2));
}

void ise_3dfx_set_pll(int pllnum, ise_3dfx_pll_clk_t* pll_clk)
{
	uint32_t cr0;
	if(ise_3dfx_card.dac_id == ISE_3DFX_DAC_ID_ATT || ise_3dfx_card.dac_id == ISE_3DFX_DAC_ID_TI)
	{
		uint32_t cc;
		ise_3dfx_dac_backdoor_en();
		cr0 = ise_3dfx_dac_read(2);
		
		ise_3dfx_dac_backdoor_en();
		ise_3dfx_dac_write(2, (cr0 & 0xF0) | 0xB);	// Switch to index mode
		
		// Sleep a bit
		ise_time_uwait(300);
		
		cc = ise_3dfx_dac_idx_read(6);
		if(pllnum == 0)	{
			ise_3dfx_dac_idx_write(0x48, pll_clk->m);
			ise_3dfx_dac_idx_write(0x49, (pll_clk->p << 6) | pll_clk->n);
			ise_3dfx_dac_idx_write(6, (cc & 0x0F) | 0xA0 );
		} else {
			ise_3dfx_dac_idx_write(0x6C, pll_clk->m);
			ise_3dfx_dac_idx_write(0x6D, (pll_clk->p << 6) | pll_clk->n);
			ise_3dfx_dac_idx_write(6, (cc & 0x0F) | 0x0B);
		}
		return;
	}
	// ICS5432
	ise_3dfx_dac_write(7, 14);
	cr0 = ise_3dfx_dac_read(5);
	if(pllnum == 0) {
		ise_3dfx_dac_write(4, 0);
		ise_3dfx_dac_write(5, pll_clk->m);
		ise_3dfx_dac_write(5, (pll_clk->p << 5) | pll_clk->n);
		ise_3dfx_dac_write(4, 14);
		ise_3dfx_dac_write(5, (cr0 & 0xD8) | 0x20);
	} else {
		ise_3dfx_dac_write(4, 10);
		ise_3dfx_dac_write(5, pll_clk->m);
		ise_3dfx_dac_write(5, (pll_clk->p << 5) | pll_clk->n);
		ise_3dfx_dac_write(4, 14);
		ise_3dfx_dac_write(5, (cr0 & 0xEF));
	}
}

//  
//  Set the depth in the DAC. This must match the frame
//  buffer format. Right now we could hard code 16, in fact
//  it may be correct to always do so.. ?
//  
 
void ise_3dfx_set_dac_depth(int depth)
{
	uint32_t cr0;
	
	if(ise_3dfx_card.dac_id == ISE_3DFX_DAC_ID_ATT || ise_3dfx_card.dac_id == ISE_3DFX_DAC_ID_TI)
	{
		ise_3dfx_dac_backdoor_en();
		cr0 = ise_3dfx_dac_read(2);
		
		ise_3dfx_dac_backdoor_en();
		
		if(depth == 16)
			ise_3dfx_dac_write(2, (cr0 & 0x0F) | 0x50);
		else if(depth ==24 || depth == 32)
			ise_3dfx_dac_write(2, (cr0 & 0x0F) | 0x70);
	} else if(ise_3dfx_card.dac_id== ISE_3DFX_DAC_ID_ICS) {
		if(depth == 16)
			ise_3dfx_dac_write(6, 0x50);
		else
			ise_3dfx_dac_write(6, 0x70);
	}
}


// ----------------------------
// Main setup functions
void ise_3dfx_calc_tri_deriv_mat(float x0, float y0, float x1, float y1, float x2, float y2, float* mat)
{
	mat[0] = x1 - x0; mat[1] = y1 - y0;
	mat[2] = x2 - x0; mat[3] = y2 - y0;
	ise_m2_inverse(mat);
}

void ise_3dfx_calc_tri_deriv(float p0, float p1, float p2, float* mat, float* deriv)
{
	deriv[0] = p1 - p0;
	deriv[1] = p2 - p0;
	ise_mv2_mul(deriv, mat, deriv);
}

void ise_3dfx_draw_prim(uint32_t prim_type, uint32_t setup, uint32_t num_verts, float* data)
{
	uint32_t vert_size = 2;
	if(setup & 0x80000000) {
		if(setup & 0x3) vert_size++;     // Packed RGBA
	} else {
		if(setup & 0x1) vert_size += 3;  // RGB
		if(setup & 0x2) vert_size++;     // Alpha
	}
	if(setup & 0x4) vert_size++;         // Z
	if(setup & 0x8) vert_size++;         // Wb
	if(setup & 0x10) vert_size++;        // W0
	if(setup & 0x20) vert_size += 2;     // S0, T0
	if(setup & 0x40) vert_size++;        // W1
	if(setup & 0x80) vert_size += 2;     // S1, T1

	if(ise_3dfx_card.flags & ISE_3DFX_CARD_CMD_FIFO_EN) {
		uint32_t pkt_hdr = 0x3;  // type 3 packet
		pkt_hdr |= prim_type << 3;
		pkt_hdr |= (setup & 0xFF) << 10;
		pkt_hdr |= (setup & 0xF0000) << 6;
		pkt_hdr |= (setup & 0x80000000) >> 3;
		uint32_t n;
		while(num_verts > 0) {
			n = (num_verts > 15) ? 15 : num_verts;
			ise_3dfx_send_packet(pkt_hdr | (n << 6), (uint32_t*) data);
			data += n*vert_size;
			num_verts -= n;
		}
	} else {
		// TODO: check for triangles, strips or fan
		// TODO: check for culling
		// TODO: check for which params are enabled
		uint32_t n;//, a, b, c, t;
		//float mat[4];
		//float deriv[2];
		ise_3dfx_wait_fifo(11);
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_S_SETUP_MODE, setup);
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_S_ARGB, 0);
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_S_ALPHA, 0);
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_S_VZ, 0);
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_S_WB, 0);
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_S_WTMU_0, 0);
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_SS_W0, 0);
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_ST_W0, 0);
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_S_WTMU_1, 0);
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_SS_WTMU_1, 0);
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_ST_WTMU_1, 0);
		for(n=0; n<num_verts; n++) {
			ise_3dfx_wait_fifo(6);
			ise_3dfx_mmio_write(ISE_3DFX_VOODOO_S_VX, data[n*vert_size+0]);
			ise_3dfx_mmio_write(ISE_3DFX_VOODOO_S_VY, data[n*vert_size+1]);
			ise_3dfx_mmio_write(ISE_3DFX_VOODOO_S_RED, data[n*vert_size+2]);
			ise_3dfx_mmio_write(ISE_3DFX_VOODOO_S_GREEN, data[n*vert_size+3]);
			ise_3dfx_mmio_write(ISE_3DFX_VOODOO_S_BLUE, data[n*vert_size+4]);
			if(n==0 || (prim_type == ISE_3DFX_PRIM_TRIANGLES && (n % 3 == 0)))
				ise_3dfx_mmio_write(ISE_3DFX_VOODOO_S_BEGIN_TRI_CMD, 1);
			else
				ise_3dfx_mmio_write(ISE_3DFX_VOODOO_S_DRAW_TRI_CMD, 1);
		}
		//for(n=2; n<num_verts; n++) {
		//	a = n-2; b = n-1; c=n;
		//	if(data[c*vert_size+1] < data[b*vert_size+1]) {
		//		t = b;
		//		b = c;
		//		c = t;
		//	}
		//	if(data[b*vert_size+1] < data[a*vert_size+1]) {
		//		t = a;
		//		a = b;
		//		b = t;
		//	}
		//	if(data[c*vert_size+1] < data[b*vert_size+1]) {
		//		t = b;
		//		b = c;
		//		c = t;
		//	}
		//	ise_3dfx_calc_tri_deriv_mat(data[a*vert_size+0], data[a*vert_size+1], data[b*vert_size+0], data[b*vert_size+1], data[c*vert_size+0], data[c*vert_size+1], mat);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FVERTEX_AX, data[a*vert_size+0]);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FVERTEX_AY, data[a*vert_size+1]);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FVERTEX_BX, data[b*vert_size+0]);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FVERTEX_BY, data[b*vert_size+1]);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FVERTEX_CX, data[c*vert_size+0]);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FVERTEX_CY, data[c*vert_size+1]);
		//	ise_3dfx_calc_tri_deriv(data[a*vert_size+2], data[b*vert_size+2], data[c*vert_size+2], mat, deriv);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FSTART_R, data[a*vert_size+2]);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FD_RD_X, deriv[0]);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FD_RD_Y, deriv[1]);
		//	ise_3dfx_calc_tri_deriv(data[a*vert_size+3], data[b*vert_size+3], data[c*vert_size+3], mat, deriv);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FSTART_G, data[a*vert_size+3]);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FD_GD_X, deriv[0]);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FD_GD_Y, deriv[1]);
		//	ise_3dfx_calc_tri_deriv(data[a*vert_size+4], data[b*vert_size+4], data[c*vert_size+4], mat, deriv);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FSTART_B, data[a*vert_size+4]);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FD_BD_X, deriv[0]);
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FD_BD_Y, deriv[1]);
		//	float area_x2 = ((data[a*vert_size+0] - data[b*vert_size+0])*(data[b*vert_size+1] - data[c*vert_size+1])) - ((data[b*vert_size+0] - data[c*vert_size+0])*(data[a*vert_size+1] - data[b*vert_size+1]));
		//	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FTRIANGLE_CMD, (area_x2 < 0) ? 0x80000000 : 0x0);
		//}
	}
}

int ise_3dfx_set_mode(ise_gfx_t* gfx)
{
	uint32_t f1, f2, f3;
	ise_vga_timing_t vga_timing = {0};
	vga_timing.width = ise_screen_mode.width;
	vga_timing.height = ise_screen_mode.height;
	ise_vga_calc_timing(&vga_timing);
	
	// TODO: check monitor capabilities, and make sure it can support timing
	
	ise_3dfx_pll_clk_t vclk;
	ise_3dfx_sst_calc_pll(vga_timing.pixel_clk, &vclk);

	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_NOP_CMD, 0);
	ise_3dfx_wait_idle();

	uint32_t data;
	ise_3dfx_pci_init_en(ISE_3DFX_PCI_INIT_WRITE_EN);
	data = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_1);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_1, data | 0x100);
	data = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_0);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_0, data | 0x6);
	data = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_2);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_2, data & ~0x400000);
	ise_3dfx_wait_idle();

	// set timing registers
	// Write back porch values
	data = vga_timing.v_back_porch << 16;
	data |= vga_timing.h_back_porch - 2;
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_BACK_PORCH, data);
	// Write displayed area
	data = vga_timing.height << 16;
	data |= vga_timing.width;
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_VIDEO_DIMENSIONS, data);
	// Horizontal and vertical sync
	data = (vga_timing.width + vga_timing.h_front_porch + vga_timing.h_back_porch - 1) << 16;
	data |= vga_timing.h_sync_pulse - 1;
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_H_SYNC, data);
	data = (vga_timing.height + vga_timing.v_front_porch + vga_timing.v_back_porch) << 16;
	data |= vga_timing.v_sync_pulse;
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_V_SYNC, data);
	
	f2 = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_2);
	f3 = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_3);

	ise_3dfx_pci_init_en(ISE_3DFX_PCI_INIT_WRITE_EN | ISE_3DFX_PCI_INIT_DAC_REG_EN);
	
	ise_3dfx_set_dac_depth(ise_screen_mode.bpp); // Hardware output is always 16bpp - check that it is
	ise_3dfx_set_pll(0, &vclk);
	
	ise_3dfx_pci_init_en(ISE_3DFX_PCI_INIT_WRITE_EN);

	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_2, f2);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_3, f3);
	
	f1 = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_1);
	
	f1 &= 0x8080010F;		// Mask off video bits
	f1 |= 0x21E000;			// Enable blanking, data, vsync, dclock 2x sel
	
	// Number of 64 pixel tiles
	uint32_t tiles = (vga_timing.width + 63) / 64;
	
	if(ise_3dfx_card.device_id == ISE_PCI_DEVICE_3DFX_VOODOO2)
		f1|= ((tiles & 0x10) ? 1<<24 : 0) | (tiles & 0x0f) << 4;
	else
		f1|= tiles << 4;

	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_1, f1);
	
	// Voodoo 2 support
	if(ise_3dfx_card.device_id == ISE_PCI_DEVICE_3DFX_VOODOO2) {
		uint32_t f5 = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_5);
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_6, 0);
		
		f5 &= ~0x05BF0000;
		
		f5 &= ~(1<<22);	// For now

		//if(mode->Flags & V_INTERLACE)
		//	f5 |= 1<<26;
		// FIXME: is this H, V or both doublescan ??
		//if(mode->Flags & V_DBLSCAN)
		//	f5 |= (1<<21) | (1<<20);
		//if(mode->Flags & V_PHSYNC)
		//	f5 |= (1<<23);
		//if(mode->Flags & V_PVSYNC)
		//	f5 |= (1<<24);
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_5, f5);
	}
	ise_3dfx_wait_idle();
	data = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_1);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_1, data & ~0x100);
	data = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_0);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_0, (data & ~0x6) | 0x1);
	uint16_t vid_buffer_size = ((ise_screen_mode.width + 63) / 64)*((ise_screen_mode.height + 31) / 32);
	vid_buffer_size &= 0x1FF;
	data = vid_buffer_size;
	data <<= 11;
	data |= ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_2);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_2, data | 0x400000);

	// enable command fifo for voodoo2
	const bool CMD_FIFO_EN = true;
	if(CMD_FIFO_EN && ise_3dfx_card.device_id == ISE_PCI_DEVICE_3DFX_VOODOO2) {
		// multiply by 3 for the 3 screen buffers (front, back and depth)
		uint16_t cmd_fifo_start = 3*vid_buffer_size;
		uint16_t mem_rows = 0x400000 >> 12; // Need to get real memory size - shouldn't assume 4MB
		uint16_t cmd_fifo_size = (cmd_fifo_start < mem_rows) ? mem_rows - cmd_fifo_start : 0;
		if(cmd_fifo_size > 64) cmd_fifo_size = 64;
		if(cmd_fifo_size > 0) {
			uint32_t f7 = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_7);
			f7 |= 0x7F38300;
			data = (cmd_fifo_start + cmd_fifo_size-1) << 16;
			data |= cmd_fifo_start;
			ise_3dfx_mmio_write(ISE_3DFX_VOODOO_CMD_FIFO_BASE_ADDR, data);
			data = cmd_fifo_start << 12;
			ise_3dfx_card.cmd_fifo_base = data;
			ise_3dfx_mmio_write(ISE_3DFX_VOODOO_CMD_FIFO_BUMP, 0);
			ise_3dfx_mmio_write(ISE_3DFX_VOODOO_CMD_FIFO_RD_PTR, data);
			ise_3dfx_mmio_write(ISE_3DFX_VOODOO_CMD_FIFO_A_MIN, data - 4);
			ise_3dfx_mmio_write(ISE_3DFX_VOODOO_CMD_FIFO_A_MAX, data - 4);
			ise_3dfx_mmio_write(ISE_3DFX_VOODOO_CMD_FIFO_DEPTH, 0);
			ise_3dfx_mmio_write(ISE_3DFX_VOODOO_CMD_FIFO_HOLES, 0);
			ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_7, f7);
			ise_3dfx_card.flags |= ISE_3DFX_CARD_CMD_FIFO_EN;
			ise_3dfx_card.cmd_fifo_size = cmd_fifo_size * 1024;  // 1K words per page
		}
	} else if(ise_3dfx_card.device_id == ISE_PCI_DEVICE_3DFX_VOODOO2) {
		uint32_t f7 = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_7);
		f7 &= ~0xFFFFFF00;
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_7, f7);
		ise_3dfx_card.flags &= ~(ISE_3DFX_CARD_CMD_FIFO_EN);
	}

	ise_3dfx_pci_init_en(ISE_3DFX_PCI_INIT_FIFO_EN);

	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_LFB_MODE, 0x100);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBZ_COLOR_PATH, 0x4000000);

	//  
	//  Set a clipping rectangle. We really deeply emphatically
	//  don't want to write off screen otherwise.
	//  
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBZ_MODE, 0x46F1);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_CLIP_LEFT_RIGHT, vga_timing.width);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_CLIP_LOW_Y_HIGH_Y, vga_timing.height);

	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_COLOR_1, 0x000000FF);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_ZA_COLOR, 0x0000FFFF);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FASTFILL_CMD, 0x1);

	float prim_data[] = {  0.0f,   0.0f, 255.0f, 255.0f,   0.0f,
						   0.0f, 100.0f, 255.0f, 255.0f,   0.0f,
						 100.0f,   0.0f,   0.0f, 255.0f, 255.0f,
						 100.0f, 100.0f,   0.0f, 255.0f, 255.0f,
						 200.0f,   0.0f, 255.0f,   0.0f, 255.0f,
						 200.0f, 100.0f, 255.0f,   0.0f, 255.0f };
	
	ise_3dfx_draw_prim(ISE_3DFX_PRIM_TRIANGLE_STRIP, 0x1, 6, prim_data);

	ise_3dfx_wait_fifo(5);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_CLIP_LEFT_RIGHT, (100 << 16) | 200);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_CLIP_LOW_Y_HIGH_Y, (100 << 16) | 200);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_COLOR_1, 0x0000FF00);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FASTFILL_CMD, 0x1);
	


	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_SWAPBUFFER_CMD, 0x1);
	//ise_3dfx_mmio_write(ISE_3DFX_VOODOO_CMD_FIFO_DEPTH, 14);
	//printf("depth: 0x%x\na min: 0x%x\na max: 0x%x\nrd ptr: 0x%x\n", 
	//	ise_3dfx_mmio_read(ISE_3DFX_VOODOO_CMD_FIFO_DEPTH),
	//	ise_3dfx_mmio_read(ISE_3DFX_VOODOO_CMD_FIFO_A_MIN),
	//	ise_3dfx_mmio_read(ISE_3DFX_VOODOO_CMD_FIFO_A_MAX),
	//	ise_3dfx_mmio_read(ISE_3DFX_VOODOO_CMD_FIFO_RD_PTR));
	ise_3dfx_wait_idle();

	gfx->buffers[0].pitch = 1024;

	return 0;
}

void ise_3dfx_hw_init()
{
	uint32_t data;
	ise_3dfx_vclk_en(false);
	ise_3dfx_pci_init_en(ISE_3DFX_PCI_INIT_WRITE_EN);

	data = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_7);
	data &= ~0xFFFFFF00;
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_7, data);
	ise_3dfx_card.flags &= ~(ISE_3DFX_CARD_CMD_FIFO_EN);

	data = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_1);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_1, data | 0x100);
	ise_3dfx_wait_idle();

	data = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_0);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_0, data | 0x7);
	ise_3dfx_wait_idle();

	data = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_2);
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_2, data & ~(0x400000));
	ise_3dfx_wait_idle();
	
	// At this point we are basically shut down

	ise_3dfx_pci_init_en(ISE_3DFX_PCI_INIT_WRITE_EN | ISE_3DFX_PCI_INIT_DAC_REG_EN);
	ise_3dfx_card.dac_id = ise_3dfx_find_dac();
	
	// Graphics clock depends on the board
	uint32_t max_clock = 50000;
	if(ise_3dfx_card.device_id == ISE_PCI_DEVICE_3DFX_VOODOO2) {
		// VooDoo2
		max_clock = 75000;
	}

	ise_3dfx_pll_clk_t gclk;
	ise_3dfx_sst_calc_pll(max_clock, &gclk);
	ise_3dfx_set_pll(1, &gclk);

	ise_3dfx_pci_init_en(ISE_3DFX_PCI_INIT_WRITE_EN | ISE_3DFX_PCI_INIT_FIFO_EN);

	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_0, 0x0);
	ise_3dfx_wait_idle();
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_1, 0x2001A8);
	ise_3dfx_wait_idle();
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_2, 0x186000E0);  // RAM setup with fast ras, oe, fifo, refresh on and a 16mS refresh
	ise_3dfx_wait_idle();
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_3, 0x40);
	ise_3dfx_wait_idle();
	ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_4, 0x2);
	ise_3dfx_wait_idle();

	if(ise_3dfx_card.device_id == ISE_PCI_DEVICE_3DFX_VOODOO2) {
		// VooDoo2
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_6, 0x0);
		ise_3dfx_wait_idle();
		uint32_t f7 = ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_7);
		f7 &= ~0xFFFFFF00;
		ise_3dfx_mmio_write(ISE_3DFX_VOODOO_FBI_INIT_7, f7);
		ise_3dfx_card.flags &= ~(ISE_3DFX_CARD_CMD_FIFO_EN);
	}

	ise_3dfx_pci_init_en(ISE_3DFX_PCI_INIT_FIFO_EN);
	ise_3dfx_vclk_en(true);
}

void ise_3dfx_uninstall()
{
	if(ise_svga_ctrl_base) {
		ise_3dfx_hw_init();
		ise_mem_unmap( ise_svga_ctrl_base );
		ise_svga_ctrl_base = NULL;
		ise_svga_uninstall = NULL;
		ise_svga_set_mode = NULL;
	}
}
//   will become           { 0x000, 0x2001A8, 0x186000e0, 0x000240, 0x2, 0x128, 0x1e6000, 0xaa}
//uint32_t ise_3dfx_f[8] = { 0x410, 0x201102, 0x80000040, 0x1e4200, 0x1, 0x128, 0x1e6000, 0xaa};
void ise_3dfx_install(int slot, int func)
{
	if(ise_screen_mode.bpp != 16) return;
	uint32_t paddr;
	uint16_t device_id = ise_pci.slot[slot][func].device_id;
	if(device_id == ISE_PCI_DEVICE_3DFX_VOODOO || device_id == ISE_PCI_DEVICE_3DFX_VOODOO2) {
		ise_3dfx_card.device_id = (uint8_t) device_id;
		ise_3dfx_card.slot = slot;
		ise_3dfx_card.func = func;
		paddr = ise_pci_read_config(0, ise_3dfx_card.slot, ise_3dfx_card.func, 0x10);
		paddr &= ~0xF;
		ise_svga_ctrl_base = (uint8_t*) ise_mem_map((void*) paddr, 0x400000);
		ise_screen_mode.framebuffer = paddr + 0x400000;
		ise_vram_base = ise_mem_map((void*) ise_screen_mode.framebuffer, 0x400000);
		ise_svga_uninstall = ise_3dfx_uninstall;
		ise_svga_set_mode = ise_3dfx_set_mode;
		ise_3dfx_hw_init();

		//printf("base: 0x%x\nstatus: 0x%x\nfbiInit0: 0x%x\nfbiInit1: 0x%x\nfbiInit2: 0x%x\nfbiInit3: 0x%x\n",
		//	ise_svga_ctrl_base,
		//	ise_3dfx_mmio_read(ISE_3DFX_VOODOO_STATUS),
		//	ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_0),
		//	ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_1),
		//	ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_2),
		//	ise_3dfx_mmio_read(ISE_3DFX_VOODOO_FBI_INIT_3));
	}
}