// InfiniScroll Engine
// ise_pci.h
// PCI related header
//
// Kamal Pillai
// 1/30/2019

#include "ise.h"

ise_pci_t ise_pci;

/*
// previous ISR for interrupts 0x8 to 0xF
void (_interrupt FAR *ise_pci_prev_isr[8])() = {NULL};

uint32_t ise_pci_prev_isr_select[8];
uint32_t ise_pci_prev_isr_offset[8];

// for each interrupt (0x8 to 0xF), alist of interrupt routines
void (_interrupt FAR *ise_pci_isr[8][8])() = {NULL};

uint16_t ise_pci_interrupt_mask = 0xFFFF;

int pci_tick = 0;

void _interrupt FAR ise_pci_isr_handler()
{
    pci_tick++;
    //const int INTERRUPT = 10;
    //const int INTERRUPT_INDEX = INTERRUPT - 8;
    //int i;
    //for(i=0; i<8; i++) {
    //    // Call each register handler
    //    if(ise_pci_isr[INTERRUPT_INDEX][i]) (ise_pci_isr[INTERRUPT_INDEX][i])(INTERRUPT);
    //}
    //// Call previous handler, if one exists
    //if(ise_pci_prev_isr[INTERRUPT_INDEX]) {
    //    _chain_intr(ise_pci_prev_isr[INTERRUPT_INDEX]);
    //} else {
    //    // otherwise, clear the interrupt on PIC
    //    outp(ISE_IRQ_PIC0_COMMAND, 0x20);
    //    outp(ISE_IRQ_PIC1_COMMAND, 0x20);
    //}
    outp(ISE_IRQ_PIC1_COMMAND, 0x20);
    outp(ISE_IRQ_PIC0_COMMAND, 0x20);
}


bool ise_pci_register_isr(int interrupt_num, void (_interrupt FAR* isr)())
{
    if(interrupt_num < 0x8 || interrupt_num > 0xF) return false;

    // store the current interrupt mask, if we haven't already
    if(ise_pci_interrupt_mask == 0xFFFF) {
        ise_pci_interrupt_mask = (uint16_t) ((inp(ISE_IRQ_PIC1_DATA) << 8) | inp(ISE_IRQ_PIC0_DATA));
        printf("interrupt mask: 0x%x\n", ise_pci_interrupt_mask);
    }

    int i;
    int first_null = -1;
    bool all_null = true;
    for(i=0; i<8; i++) {
        if(ise_pci_isr[interrupt_num-8][i] == NULL) {
            if(first_null == -1) first_null = i;
        } else {
            all_null = false;
            if(ise_pci_isr[interrupt_num-8][i] == isr) {
                first_null = -2;
            }
        }
    }

    // isr already registered
    if(first_null == -2) return true;

    // empty slot not found
    if(first_null == -1) return false;

    ise_pci_isr[interrupt_num-8][first_null] = isr;
    if(all_null) {
        // check if this interrupt had been enabled (mask is clear)
        // need a bool to say it was valid
        //if(!(ise_pci_interrupt_mask & (1 << interrupt_num))) {
        //    
        //}

        // get old isr
        ise_pci_prev_isr[interrupt_num-8] = _dos_getvect(ISE_IRQ_INTERRUPT_VECTOR(interrupt_num));
        //union REGS regs;
        //regs.w.ax = 0x204;
        //regs.w.bx = ISE_IRQ_INTERRUPT_VECTOR(interrupt_num);
        //INT86(0x31, &regs, &regs);
        //ise_pci_prev_isr_select[interrupt_num-8] = regs.w.cx;
        //ise_pci_prev_isr_offset[interrupt_num-8] = regs.x.edx;
        //regs.w.ax = 0x205;
        //regs.w.bx = ISE_IRQ_INTERRUPT_VECTOR(interrupt_num);
        //regs.w.cx = (uint16_t) FP_SEG(ise_pci_isr_handler);
        //regs.x.edx = (uint32_t) FP_OFF(ise_pci_isr_handler);
        
        _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(interrupt_num), ise_pci_isr_handler);
        printf("pci isr handler: 0x%x:0x%x\n", _dos_getvect(ISE_IRQ_INTERRUPT_VECTOR(interrupt_num)));
        printf("pci isr pointer: 0x%x %x\n", ise_pci_isr_handler);
        //for(i=0; i<16; i++) {
        //    union REGS regs;
        //    regs.w.ax = 0x204;
        //    regs.w.bx = ISE_IRQ_INTERRUPT_VECTOR(i);
        //    INT86(0x31, &regs, &regs);
        //   printf("IRQ%d (vec 0x%x): 0x%x %x\n", i, ISE_IRQ_INTERRUPT_VECTOR(i), (uint32_t) regs.w.cx, (uint32_t) regs.x.edx);
        //    regs.w.ax = 0x200;
        //    regs.w.bx = ISE_IRQ_INTERRUPT_VECTOR(i);
        //    INT86(0x31, &regs, &regs);
        //    printf("IRQ%d (vec 0x%x): 0x%x %x (realmode)\n", i, ISE_IRQ_INTERRUPT_VECTOR(i), (uint32_t) regs.w.cx, (uint32_t) regs.w.dx);
        //}
        //switch(interrupt_num) {
        //    case 8:  _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(interrupt_num), ise_pci_isr8);  break;
        //    case 9:  _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(interrupt_num), ise_pci_isr9);  break;
        //    case 10: _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(interrupt_num), ise_pci_isr10); break;
        //    case 11: _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(interrupt_num), ise_pci_isr11); break;
        //    case 12: _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(interrupt_num), ise_pci_isr12); break;
        //    case 13: _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(interrupt_num), ise_pci_isr13); break;
        //    case 14: _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(interrupt_num), ise_pci_isr14); break;
        //    case 15: _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(interrupt_num), ise_pci_isr15); break;
        //}
        // clear the interrupt mask
        int new_interrupt_mask = inp(ISE_IRQ_PIC1_DATA) & ~(1 << (interrupt_num-8));
        printf("Setting interrupt handler for int 0x%x (vector 0x%x), new mask 0x%x\n", interrupt_num, ISE_IRQ_INTERRUPT_VECTOR(interrupt_num), new_interrupt_mask);
        outp(ISE_IRQ_PIC1_DATA, new_interrupt_mask);
        // test interrupt
        //INT86(ISE_IRQ_INTERRUPT_VECTOR(i), &regs, &regs);
    }
    return true;
}

void ise_pci_unregister_isr(int interrupt_num, void (_interrupt FAR* isr)())
{
    if(interrupt_num < 0x8 || interrupt_num > 0xF) return;

    printf("pci_tick: %d\n", pci_tick);
    int i;
    bool all_null = true;
    for(i=0; i<8; i++) {
        if(ise_pci_isr[interrupt_num-8][i] == isr) {
            ise_pci_isr[interrupt_num-8][i] = NULL;
        }
        if(ise_pci_isr[interrupt_num-8][i]) {
            all_null = false;
        }
    }
    if(all_null) {
        printf("Restoring interrupt handler for int 0x%x\n", interrupt_num);
        if(ise_pci_interrupt_mask != 0xFFFF && (ise_pci_interrupt_mask & (1 << interrupt_num))) {
            // need to set the mask again
            int new_interrupt_mask = inp(ISE_IRQ_PIC1_DATA) | (1 << (interrupt_num-8));
            outp(ISE_IRQ_PIC1_DATA, new_interrupt_mask);
        }
        _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(interrupt_num), ise_pci_prev_isr[interrupt_num-8]);
        //union REGS regs;
        //regs.w.ax = 0x205;
        //regs.w.bx = ISE_IRQ_INTERRUPT_VECTOR(interrupt_num);
        //regs.w.cx = (uint16_t) ise_pci_prev_isr_select[interrupt_num-8];
        //regs.x.edx = ise_pci_prev_isr_offset[interrupt_num-8];
        ////INT86(0x31, &regs, &regs);
        //ise_pci_prev_isr[interrupt_num-8] = NULL;
    }
}
*/

int ise_pci_detect()
{
    union REGS regs;

    regs.w.ax = ISE_PCI_CHECK_FUNCTION;
    INT86(ISE_PCI_CHECK_IRQ, &regs, &regs);
    return (regs.x.edx == ISE_PCI_CHECK_STRING) ? 0 : -1;
}

uint32_t ise_pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address = ISE_PCI_CONFIG_GEN_ADDR(bus, slot, func, offset);
    
    outpd(ISE_PCI_CONFIG_ADDRESS, address);
    return inpd(ISE_PCI_CONFIG_DATA);
}

void ise_pci_write_config16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t data)
{
    uint32_t address = ISE_PCI_CONFIG_GEN_ADDR(bus, slot, func, offset);
    
    outpd(ISE_PCI_CONFIG_ADDRESS, address);
    outpw(ISE_PCI_CONFIG_DATA, data);
}

void ise_pci_write_config32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t data)
{
    uint32_t address = ISE_PCI_CONFIG_GEN_ADDR(bus, slot, func, offset);
    
    outpd(ISE_PCI_CONFIG_ADDRESS, address);
    outpd(ISE_PCI_CONFIG_DATA, data);
}

uint32_t ise_pci_bar_size(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar)
{
    if(bar >= 6) return 0;

    uint8_t bar_offset = 0x10 + (bar << 2);
    uint32_t old_bar = ise_pci_read_config(bus, slot, func, bar_offset);
    ise_pci_write_config32(bus, slot, func, bar_offset, ~0L);
    uint32_t valid_bits = ise_pci_read_config(bus, slot, func, bar_offset);
    valid_bits &= ~3L;
    uint32_t bar_size = -valid_bits;

    ise_pci_write_config32(bus, slot, func, bar_offset, old_bar);
    return bar_size;
}

void ise_pci_scan()
{
    int s, f, maxfunc;
    uint32_t data;
    for(s=0; s<ISE_PCI_MAX_SLOTS; s++) {
        data = ise_pci_read_config(0, s, 0, 0xc);
        maxfunc = (data == 0xFFFFFFFF) ? 0 : ((data & 0x800000) ? ISE_PCI_MAX_FUNC : 1);
        for(f=0; f<ISE_PCI_MAX_FUNC; f++) {
            data = ise_pci_read_config(0, s, f, 0x0);
            ise_pci.slot[s][f].vendor_id = (uint16_t) (data & 0xffff);
            ise_pci.slot[s][f].priv = NULL;
            if(ise_pci.slot[s][f].vendor_id != ISE_PCI_VENDOR_INVALID) {
                ise_pci.slot[s][f].device_id = (uint16_t) (data >> 16);
                if(f >= maxfunc) {
                    // if func greater than the device supports, record as invalid
                    ise_pci.slot[s][f].vendor_id = ISE_PCI_VENDOR_INVALID;
                    ise_pci.slot[s][f].device_id = 0x0;
                } else {
                    data = ise_pci_read_config(0, s, f, 0x8);
                    ise_pci.slot[s][f].revision_id = (uint8_t) (data & 0xff);
                    ise_pci.slot[s][f].class_code = data >> 8;
                    data = ise_pci_read_config(0, s, f, 0x2c);
                    ise_pci.slot[s][f].subsystem_vendor_id = (uint16_t) (data & 0xffff);
                    ise_pci.slot[s][f].subsystem_id = (uint16_t) (data >> 16);
                }
            }
        }
    }
}
