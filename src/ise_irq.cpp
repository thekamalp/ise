// InfiniScroll Engine
// ise_irq.cpp
// Interrupt related source
//
// Kamal Pillai
// 4/1/2020

#include "ise.h"

void ise_rm_interrupt(int int_num, ise_regs_t* rmi_regs)
{
    union REGS regs = {0};
	struct SREGS sregs = {0};

	regs.w.ax = 0x300;
	regs.w.bx = int_num & 0xFF;
	
	sregs.es = FP_SEG(rmi_regs);
	regs.x.edi = FP_OFF(rmi_regs);
	
	INT86X(0x31, &regs, &regs, &sregs);
}
