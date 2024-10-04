// InfiniScroll Engine
// ise_mem.cpp
// Memory routines
//
// Kamal Pillai
// 1/31/2019

#include "ise.h"

#define ISE_MEM_APTR(p, log2_align) (((((size_t) (p)) >> log2_align) + 1) << log2_align)

uint8_t ise_mem_dpmi_type = 0;
void __far (*ise_mem_dos32a_core)(uint32_t f) = NULL;

uint32_t __far* ise_mem_gdt = NULL;
uint32_t ise_mem_gdt_limit = 0;
uint32_t __far* ise_mem_pt0 = NULL;  // first level page table (aka Page diretory)
uint32_t __far* ise_mem_pt1 = NULL;  // second level page table (last in ia32)

void ise_mem_dos32a(ise_regs_t* regs)
{
	if(ise_mem_dos32a_core == NULL) {
		union REGS regs = {0};
		struct SREGS sregs = {0};
		const char vendor[16] = "SUNSYS DOS/32A";
		regs.w.ax = 0x0A00;
		sregs.ds = FP_SEG(vendor);
		regs.x.esi = FP_OFF(vendor);
		//printf("ds:esi= %x:%x\n", sregs.ds, regs.x.esi);
		INT86X(ISE_MEM_MAP_IRQ, &regs, &regs, &sregs);
		ise_mem_dpmi_type = regs.h.bh;
		//printf("bx=%x\n", regs.w.bx);
		//printf("es:edi=%x:%x\n", sregs.es, regs.x.edi);
		ise_mem_dos32a_core = (void __far (*)(uint32_t)) MK_FP(sregs.es, regs.x.edi);
	}
	ise_mem_dos32a_core(regs->eax);
	__asm {
		mov eax, [regs]
		mov [eax]+10h, ebx
		mov [eax]+18h, ecx
		mov [eax]+14h, edx
		mov [eax]+04h, esi
		mov [eax]+00h, edi
	}
}

void ise_mem_dump(FILE* file, void __far* addr, int length)
{
    uint32_t* a = (uint32_t*) addr;
    if(file == NULL) file = stdout;
    fprintf(file, "dumping 0x%x:\n", a);
    int i;
    for(i=0; i<length; i++) {
        fprintf(file, "0x%08x\n", *(a + i));
    }
}

void ise_mem_get_gdt()
{
	if(ise_mem_gdt == NULL) {
		ise_regs_t regs = {0};
		regs.eax = 0x0;
		ise_mem_dos32a(&regs);
		ise_mem_gdt_limit = regs.ecx;
		ise_mem_gdt = (uint32_t __far*) MK_FP((uint16_t) regs.ebx, regs.esi);
	}
}	

uint32_t ise_mem_va_to_linear(void __far* vaddr)
{
	// DOS32a should keep segments flat, so this should just return the original address offset
	// This function will check that the selector is valid
	ise_mem_get_gdt();
	uint16_t sel = FP_SEG(vaddr);
	uint32_t entry0 = ise_mem_gdt[sel/4];
	uint32_t entry1 = ise_mem_gdt[sel/4+1];
	uint32_t base = (entry0 >> 16) | ((entry1 & 0xFF) << 16) | (entry1 & 0xFF000000);
	uint32_t limit = (entry0 & 0xFFFF) | (entry1 & 0xF0000);
	uint32_t access = (entry1 >> 8) & 0xFF;
	uint32_t flags = (entry1 >> 20) & 0xF;
	uint32_t offset = FP_OFF(vaddr);
	if(flags & 0x8) {
		// 4KB granularity
		limit <<= 12;
	}
	if(!(access & 0x80) || offset > limit) return 0x0;
	
	return base + offset;
}

uint32_t ise_mem_linear_to_pa(uint32_t linear)
{
	uint32_t paddr = 0x0;
	ise_mem_get_gdt();
	if(ise_mem_dpmi_type == 0x2) {
		// In VCPI environments, read page tables, and translate the address
		if(ise_mem_pt0 == NULL || ise_mem_pt1 == NULL) {
			ise_regs_t regs = {0};
			regs.eax = 0x1;
			ise_mem_dos32a(&regs);
			ise_mem_pt0 = (uint32_t __far*) MK_FP((uint16_t) regs.ebx, regs.edi);
			ise_mem_pt1 = (uint32_t __far*) MK_FP((uint16_t) regs.ebx, regs.esi);
		}
		uint32_t pt0_index = ((uint32_t) linear) >> 22;
		uint32_t pt1_index = ((uint32_t) linear) >> 12;
		uint32_t pt0_entry = ise_mem_pt0[pt0_index];
		if(pt0_entry & 1) {
			// Make sure level 1 page table is mapped first, before reading it
			// if not mapped, return NULL (0x0)
			uint32_t pt1_entry = ise_mem_pt1[pt1_index];
			if(pt1_entry & 1) {
				// Make sure the page is mapped before returning
				paddr = (pt1_entry & ~0xFFF) | (linear & 0xFFF);
			}
		}
		return paddr;
	} else {
		// otherwise, return the linear address
		return linear;
	}
}

uint32_t ise_mem_va_to_pa(void __far* vaddr)
{
	uint32_t linear = ise_mem_va_to_linear(vaddr);
	uint32_t paddr = ise_mem_linear_to_pa(linear);
	return paddr;
}

void* ise_mem_aligned_malloc( size_t size, size_t log2_alignment, int region )
{
    void* ptr;
    size_t save_data;
    if(region == ISE_MEM_REGION_DOS) {
        union REGS regs;
        if(log2_alignment < 4) log2_alignment = 4;
        regs.w.ax = ISE_MEM_DOS_ALLOC_FUNC;
        regs.w.bx = (uint16_t) ((size + (1 << log2_alignment)) >> 4);
        INT86(ISE_MEM_MAP_IRQ, &regs, &regs);
        ptr = (regs.w.cflag & INTR_CF) ? NULL : (void*) ((uint32_t) regs.w.ax << 4);
        save_data = regs.w.dx;
    } else {
        ptr = malloc(size + (1 << log2_alignment));
        save_data = (size_t) ptr;
    }
    if(ptr == NULL) return ptr;

    void* aligned_ptr = (void*) ISE_MEM_APTR(ptr, log2_alignment);
    *(((size_t*) aligned_ptr) - 1) = save_data;

    return aligned_ptr;
}

void ise_mem_aligned_free( void* ptr )
{
    uint32_t ptr_val = (uint32_t) ptr;
    if(ptr_val < 0x100000) {
        // under 1MB, means this is DOS memory
        union REGS regs;
        regs.w.ax = ISE_MEM_DOS_FREE_FUNC;
        regs.w.dx = (uint16_t) *(((size_t*) ptr) - 1);
        INT86(ISE_MEM_MAP_IRQ, &regs, &regs);
    } else {
        // otherwise, extended memory
        void* orig_ptr = (void*) *(((size_t*) ptr) - 1);
        free(orig_ptr);
    }
}

int ise_mem_alloc_entries(int num_entries, ise_mem_pool_alloc_t* pool, int array_entries)
{
    // only support 32 entries or less
    if(num_entries <= 0 || num_entries > 32) return -1;
    uint32_t size_mask = (num_entries==32) ? ~0L : (1L << num_entries) - 1;
    int offset = -1; // invalid offset
    int i, j;

    // Do a hierarchical search through every bit that can be allocated
    // first 32 bits at a time
    for(i=0; i<array_entries && offset < 0; i++) {
        if(pool[i].i32 != ~0L) {
            uint32_t a = pool[i].i32;
            uint32_t b = (i<array_entries-1) ? pool[i+1].i32 : ~0L;
            uint32_t c;
            // hierarchically scan through every 8 bits
            // and look for something with a free entry
            for(j=0; j<4 && (pool[i].i8[j] == 0xFF); j++) ;
            for(j=j*8; j<32 && offset < 0; j++) {
                c = (a >>j) | ((j>0) ? (b << (32-j)) : 0x0);
                if((c & size_mask) == 0x0) {
                    offset = ((i << 5) | j);
                    pool[i].i32 |= (size_mask << j);
                    if(j>0 && i<15) pool[i+1].i32 |= (size_mask >> (32-j));
                }
            }
        }
    }
    return offset;
}

// returns true if successfully freed
bool ise_mem_free_entries(int offset, int num_entries, ise_mem_pool_alloc_t* pool, int array_entries)
{
    // only support 32 entries or less
    if(num_entries > 32) return false;
    // can't exceed size of pool
    if(offset+num_entries > array_entries*32) return false;
    uint32_t size_mask = (num_entries==32) ? ~0L : (1L << num_entries) - 1;

    int i = offset >> 5;
    int j = offset & 0x1f;
    pool[i].i32 &= ~(size_mask << j);
    if(j>0 && i<15) pool[i+1].i32 &= ~(size_mask >> (32-j));
    return true;
}

void* ise_mem_map( void* paddr, size_t size)
{
    union REGS regs;

    uint32_t addr32 = (uint32_t) paddr;
    uint32_t size32 = (uint32_t) size;

    regs.w.ax = ISE_MEM_MAP_FUNC;
    regs.w.bx = (uint16_t) (addr32 >> 16);
    regs.w.cx = (uint16_t) (addr32 & 0xFFFF);
    regs.w.si = (uint16_t) (size32 >> 16);
    regs.w.di = (uint16_t) (size32 & 0xFFFF);
    INT86(ISE_MEM_MAP_IRQ, &regs, &regs);

    addr32 = (((uint32_t) regs.w.bx) << 16) | regs.w.cx;
    
    // if carry flag is set, we encountered an error
    return (regs.w.cflag & INTR_CF) ? NULL : (void*) addr32;
}

void ise_mem_unmap( void* vaddr )
{
    union REGS regs;
    
    uint32_t addr32 = (uint32_t) vaddr;
    regs.w.ax = ISE_MEM_UNMAP_FUNC;
    regs.w.bx = (uint16_t) addr32 >> 16;
    regs.w.cx = (uint16_t) addr32 & 0xFFFF;
    INT86(ISE_MEM_MAP_IRQ, &regs, &regs);
}
