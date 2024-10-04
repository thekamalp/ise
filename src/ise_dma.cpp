// InfiniScroll Engine
// ise_dma.cpp
// DMA setup
//
// Kamal Pillai
// 5/15/2019

#include "ise.h"

void ise_dma_setup(uint8_t mode, uint8_t channel, void* addr, uint16_t length)
{
    uint32_t laddr = (uint32_t) addr;
    // non-zero length, channel more than 8 or address greater than 16MB can't be done
    if(length == 0 || channel >= 8 || (laddr & ~0xFFFFFF)) return;

    uint8_t ch_off = channel & 0x3;  // channel offset
    if(channel >= 4) length = length >> 1;
    length--;

    int mask_port = (channel >= 4) ? ISE_DMA_SINGLE_MASK_1 : ISE_DMA_SINGLE_MASK_0;
    int mode_port = (channel >= 4) ? ISE_DMA_WRITE_MODE_1 : ISE_DMA_WRITE_MODE_0;
    int clear_byte_port = (channel >= 4) ? ISE_DMA_CLEAR_BYTE_1 : ISE_DMA_CLEAR_BYTE_0;
    int addr_port = (channel >= 4) ? ISE_DMA_ADDR_CH4 + 4*ch_off : ISE_DMA_ADDR_CH0 + 2*ch_off;
    int count_port = (channel >= 4) ? addr_port + 2 : addr_port + 1;
    int page_port;
    switch(channel) {
        case 0: page_port = ISE_DMA_PAGE_CH0; break;
        case 1: page_port = ISE_DMA_PAGE_CH1; break;
        case 2: page_port = ISE_DMA_PAGE_CH2; break;
        case 3: page_port = ISE_DMA_PAGE_CH3; break;
        case 4: page_port = ISE_DMA_PAGE_CH4; break;
        case 5: page_port = ISE_DMA_PAGE_CH5; break;
        case 6: page_port = ISE_DMA_PAGE_CH6; break;
        case 7: page_port = ISE_DMA_PAGE_CH7; break;
    }

    // disable interrupts
    _disable();  

    // disable DMA channel
    outp(mask_port, ISE_DMA_MASK_DISABLE | ch_off);
    
    // clear byte pointer
    outp(clear_byte_port, 0x0);
    
    // set the DMA mode
    outp(mode_port, mode | ch_off);

    // Set address and page
    uint16_t buff_offset = (uint16_t) (((channel >= 4) ? (laddr / 2) : laddr) & 0xFFFF);
    outp(addr_port, buff_offset & 0xFF);
    outp(addr_port, (buff_offset >> 8) & 0xFF);
    outp(page_port, (laddr >> 16) & 0xFF);
    
    // Set count
    outp(count_port, length & 0xFF);
    outp(count_port, (length >> 8) & 0xFF);

    // Enable DMA channel
    outp(mask_port, ISE_DMA_MASK_ENABLE | ch_off);
    
    // re-enable interrupts
    _enable();
}
