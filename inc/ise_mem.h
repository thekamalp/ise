// InfiniScroll Engine
// ise_mem.h
// Memory routines
//
// Kamal Pillai
// 1/31/2019

#ifndef __ISE_MEM_H
#define __ISE_MEM_H

#define ISE_MEM_MAP_IRQ                        0x31
#define ISE_MEM_MAP_FUNC                       0x0800
#define ISE_MEM_UNMAP_FUNC                     0x0801
#define ISE_MEM_DOS_ALLOC_FUNC                 0x0100
#define ISE_MEM_DOS_FREE_FUNC                  0x0101
#define ISE_MEM_DOS_REALLOC_FUNC               0x0102

#define ISE_MEM_DOS_IRQ                        0x21
#define ISE_MEM_VA_TO_PA_FUNC                  0x2509

#define ISE_MEM_REGION_EXTENDED                0x0
#define ISE_MEM_REGION_DOS                     0x1   // Conventional memory <640KB

// used to indicate a list of allocated or free entries
typedef union {
    uint8_t i8[4];
    uint16_t i16[2];
    uint32_t i32;
} ise_mem_pool_alloc_t;

// prints out memory for debug
void ise_mem_dump(FILE* file, void __far* addr, int length);

// Converts virtual to linear address
uint32_t ise_mem_va_to_linear(void __far* vaddr);

// Convents lienar to physical address
uint32_t ise_mem_linear_to_pa(uint32_t linear);

// convert virtual address to physical address
uint32_t ise_mem_va_to_pa(void __far* vaddr);

// allocates memory using malloc
// adds 2^log2_alignment padding so that the pointer
// returned is aligned to that amount
void* ise_mem_aligned_malloc( size_t size, size_t log2_alignment, int region );

// free pointer allocated with ise_mem_aligned_alloc
void ise_mem_aligned_free( void* ptr );

// maps a physical address into virtual address space
void* ise_mem_map( void* paddr, size_t size);

// unmaps physical address
// address passed in is an address returned by ise_mem_map
void ise_mem_unmap( void* vaddr );

// allocates entries from a memory pool
// returns -1 if no entry could be found
int ise_mem_alloc_entries(int num_entries, ise_mem_pool_alloc_t* pool, int array_entries);

// frees entries from a memory pool
// returns true if successfully freed
bool ise_mem_free_entries(int offset, int num_entries, ise_mem_pool_alloc_t* pool, int array_entries);

#endif  //  #ifndef __ISE_MEM_H
