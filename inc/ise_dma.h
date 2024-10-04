// InfiniScroll Engine
// ise_dma.h
// DMA header
//
// Kamal Pillai
// 5/15/2019

#ifndef __ISE_DMA_H
#define __ISE_DMA_H

// DMA I/O ports
// 8 bit DMA channels
#define ISE_DMA_ADDR_CH0                       0x00
#define ISE_DMA_COUNT_CH0                      0x01
#define ISE_DMA_ADDR_CH1                       0x02
#define ISE_DMA_COUNT_CH1                      0x03
#define ISE_DMA_ADDR_CH2                       0x04
#define ISE_DMA_COUNT_CH2                      0x05
#define ISE_DMA_ADDR_CH3                       0x06
#define ISE_DMA_COUNT_CH3                      0x07

// 16 bit DMA channels
#define ISE_DMA_ADDR_CH4                       0xC0
#define ISE_DMA_COUNT_CH4                      0xC2
#define ISE_DMA_ADDR_CH5                       0xC4
#define ISE_DMA_COUNT_CH5                      0xC6
#define ISE_DMA_ADDR_CH6                       0xC8
#define ISE_DMA_COUNT_CH6                      0xCA
#define ISE_DMA_ADDR_CH7                       0xCC
#define ISE_DMA_COUNT_CH7                      0xCE

#define ISE_DMA_PAGE_CH0                       0x87
#define ISE_DMA_PAGE_CH1                       0x83
#define ISE_DMA_PAGE_CH2                       0x81
#define ISE_DMA_PAGE_CH3                       0x82
#define ISE_DMA_PAGE_CH4                       0x8F
#define ISE_DMA_PAGE_CH5                       0x8B
#define ISE_DMA_PAGE_CH6                       0x89
#define ISE_DMA_PAGE_CH7                       0x8A

#define ISE_DMA_WRITE_MODE_0                   0x0B  // 8 bit DMAC
#define ISE_DMA_WRITE_MODE_1                   0xD6  // 16 bit DMAC

#define ISE_DMA_SINGLE_MASK_0                  0x0A  // 8 bit DMAC
#define ISE_DMA_SINGLE_MASK_1                  0xD4  // 16 bit DMAC

#define ISE_DMA_CLEAR_BYTE_0                   0x0C  // 8 bit DMAC
#define ISE_DMA_CLEAR_BYTE_1                   0xD8  // 16 bit DMAC

// Write mode enums
#define ISE_DMA_MODE_DEMAND                    0x00
#define ISE_DMA_MODE_SINGLE                    0x40
#define ISE_DMA_MODE_BLOCK                     0x80
#define ISE_DMA_MODE_CASCADE                   0xC0

#define ISE_DMA_MODE_ADDR_INC                  0x00
#define ISE_DMA_MODE_ADDR_DEC                  0x20

#define ISE_DMA_MODE_ONE_CYCLE                 0x00
#define ISE_DMA_MODE_AUTO_INIT                 0x10

#define ISE_DMA_MODE_XFER_VERIFY               0x00
#define ISE_DMA_MODE_XFER_WRITE                0x04
#define ISE_DMA_MODE_XFER_READ                 0x08

#define ISE_DMA_MASK_ENABLE                    0x00
#define ISE_DMA_MASK_DISABLE                   0x04

#define ISE_DMA_CH0                            0x00
#define ISE_DMA_CH1                            0x01
#define ISE_DMA_CH2                            0x02
#define ISE_DMA_CH3                            0x03
#define ISE_DMA_CH4                            0x00
#define ISE_DMA_CH5                            0x01
#define ISE_DMA_CH6                            0x02
#define ISE_DMA_CH7                            0x03

void ise_dma_setup(uint8_t mode, uint8_t channel, void* addr, uint16_t length);

#endif  //  #ifndef __ISE_DMA_H