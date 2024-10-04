// InfiniScroll Engine
// ise_mga.h
// Matrox HW acceleration
//
// Kamal Pillai
// 4/5/2020

#ifndef __ISE_MGA_H
#define __ISE_MGA_H

#define ISE_PCI_VENDOR_MATROX                  0x102B
#define ISE_PCI_DEVICE_MGA2064W                0x0519
#define ISE_PCI_DEVICE_MGA1064SG               0x051A

// register set for MGA1064SG, MGA2064W
#define ISE_MGA_REG0_DRAW_START                0x0100  // OR'd in with another register write to trigger draw command
#define ISE_MGA_REG0_DWGCTL                    0x1C00
#define ISE_MGA_REG0_MACCESS                   0x1C04
#define ISE_MGA_REG0_ZORG                      0x1C0C
#define ISE_MGA_REG0_PLNWT                     0x1C1C
#define ISE_MGA_REG0_BCOL                      0x1C20
#define ISE_MGA_REG0_FCOL                      0x1C24
#define ISE_MGA_REG0_DMAPAD                    0x1C54
#define ISE_MGA_REG0_SGN                       0x1C58
#define ISE_MGA_REG0_AR0                       0x1C60
#define ISE_MGA_REG0_AR1                       0x1C64
#define ISE_MGA_REG0_AR2                       0x1C68
#define ISE_MGA_REG0_AR3                       0x1C6C
#define ISE_MGA_REG0_AR4                       0x1C70
#define ISE_MGA_REG0_AR5                       0x1C74
#define ISE_MGA_REG0_AR6                       0x1C78
#define ISE_MGA_REG0_CXBNDRY                   0x1C80
#define ISE_MGA_REG0_FXBNDRY                   0x1C84
#define ISE_MGA_REG0_YDSTLEN                   0x1C88
#define ISE_MGA_REG0_PITCH                     0x1C8C
#define ISE_MGA_REG0_YDSTORG                   0x1C94
#define ISE_MGA_REG0_YTOP                      0x1C98
#define ISE_MGA_REG0_YBOT                      0x1C9C



void ise_mga_install(int slot, int func);

#endif
