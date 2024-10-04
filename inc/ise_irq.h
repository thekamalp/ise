// InfiniScroll Engine
// ise_irq.h
// Interrupt related header
//
// Kamal Pillai
// 2/8/2019

#ifndef __ISE_IRQ_H
#define __ISE_IRQ_H

// Ports to master and slave programmable interrupt controllers
#define ISE_IRQ_PIC0_COMMAND                   0x20
#define ISE_IRQ_PIC0_DATA                      0x21
#define ISE_IRQ_PIC1_COMMAND                   0xA0
#define ISE_IRQ_PIC1_DATA                      0xA1

// No really PCI specific, but gets the vector from an IRQ number
#define ISE_IRQ_INTERRUPT_VECTOR(i)            (((i) < 8) ? 0x8+i : 0x68+i)

#pragma pack(push,1)
typedef struct {
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t reserved;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint16_t flags;
	uint16_t es;
	uint16_t ds;
	uint16_t fs;
	uint16_t gs;
	uint16_t ip;
	uint16_t cs;
	uint16_t sp;
	uint16_t ss;
} ise_regs_t;
#pragma pack(pop)

void ise_rm_interrupt(int int_num, ise_regs_t* rmi_regs);

#endif  //  #ifndef __ISE_IRQ_H
