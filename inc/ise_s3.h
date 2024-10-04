// InfiniScroll Engine
// ise_s3.h
// S3 Trio32/64 HW acceleration
//
// Kamal Pillai
// 4/23/2020

#ifndef __ISE_S3_H
#define __ISE_S3_H

#define ISE_PCI_VENDOR_S3                      0x5333
#define ISE_PCI_DEVICE_S3_TRIO                 0x8811  // same for Trio32 and 64

// register set for Trio32/64
#define ISE_S3_TRIO_CUR_Y                      0x82E8
#define ISE_S3_TRIO_CUR_Y2                     0x82EA   // Trio64 only
#define ISE_S3_TRIO_CUR_X                      0x86E8
#define ISE_S3_TRIO_CUR_X2                     0x86EA   // Trio64 only
#define ISE_S3_TRIO_DESTY_AXSTP                0x8AE8
#define ISE_S3_TRIO_Y2_AXSTP2                  0x8AEA   // Trio64 only
#define ISE_S3_TRIO_DESTX_DIASTP               0x8EE8
#define ISE_S3_TRIO_X2                         0x8EEA   // Trio64 only
#define ISE_S3_TRIO_ERR_TERM                   0x92E8
#define ISE_S3_TRIO_ERR_TERM2                  0x92EA   // Trio64 only
#define ISE_S3_TRIO_MAJ_AXIS_PCNT              0x96E8
#define ISE_S3_TRIO_MAJ_AXIS_PCNT2             0x96EA   // Trio64 only
#define ISE_S3_TRIO_GP_STAT                    0x9AE8
#define ISE_S3_TRIO_CMD                        0x9AE8
#define ISE_S3_TRIO_CMD2                       0x9AEA   // Trio64 only
#define ISE_S3_TRIO_SHORT_STROKE               0x9EE8
#define ISE_S3_TRIO_BKGD_COLOR                 0xA2E8
#define ISE_S3_TRIO_FRGD_COLOR                 0xA6E8
#define ISE_S3_TRIO_WRT_MASK                   0xAAE8
#define ISE_S3_TRIO_RD_MASK                    0xAEE8
#define ISE_S3_TRIO_COLOR_CMP                  0xB2E8
#define ISE_S3_TRIO_BKGD_MIX                   0xB6E8
#define ISE_S3_TRIO_FRGD_MIX                   0xBAE8
#define ISE_S3_TRIO_MISC_REG                   0xBEE8
// following are indices into the MIS register (upper nibble defines the index)
#define ISE_S3_TRIO_MISC_MIN_AXIS_PCNT         0x0000
#define ISE_S3_TRIO_MISC_SCISSOR_T             0x1000
#define ISE_S3_TRIO_MISC_SCISSOR_L             0x2000
#define ISE_S3_TRIO_MISC_SCISSOR_B             0x3000
#define ISE_S3_TRIO_MISC_SCISSOR_R             0x4000
#define ISE_S3_TRIO_MISC_PIX_CNTL              0xA000
#define ISE_S3_TRIO_MISC_MULTI_MISC2           0xD000
#define ISE_S3_TRIO_MISC_MULTI_MISC            0xE000
#define ISE_S3_TRIO_MISC_READ_SEL              0xF000
// end MISC regs
#define ISE_S3_TRIO_PIX_TRANS                  0xE2E8
#define ISE_S3_TRIO_PIX_TRANS_EXT              0xE2EA
#define ISE_S3_TRIO_PAT_Y                      0xEAE8   // Trio64 only
#define ISE_S3_TRIO_PAT_X                      0xEAEA   // Trio64 only

// Packed registers
#define ISE_S3_TRIO_ALT_CURXY                  0x8100
#define ISE_S3_TRIO_ALT_CURXY2                 0x8104
#define ISE_S3_TRIO_ALT_STEP                   0x8108
#define ISE_S3_TRIO_ALT_STEP2                  0x810C
#define ISE_S3_TRIO_ALT_ERR                    0x8110
#define ISE_S3_TRIO_ALT_CMD                    0x8118
#define ISE_S3_TRIO_ALT_SHORT_STROKE           0x811C
#define ISE_S3_TRIO_ALT_BKGD_COLOR             0x8120
#define ISE_S3_TRIO_ALT_FRGD_COLOR             0x8124
#define ISE_S3_TRIO_ALT_WRT_MASK               0x8128
#define ISE_S3_TRIO_ALT_RD_MASK                0x812C
#define ISE_S3_TRIO_ALT_COLOR_CMP              0x8130
#define ISE_S3_TRIO_ALT_MIX                    0x8134
#define ISE_S3_TRIO_ALT_SCISSOR_TL             0x8138
#define ISE_S3_TRIO_ALT_SCISSOR_BR             0x813C
#define ISE_S3_TRIO_ALT_PIX_CNTL_MISC2         0x8140
#define ISE_S3_TRIO_ALT_MULTI_MISC             0x8144
#define ISE_S3_TRIO_ALT_PCNT                   0x8148
#define ISE_S3_TRIO_ALT_PCNT2                  0x814C
#define ISE_S3_TRIO_ALT_PAT                    0x8168

void ise_s3_install(int slot, int func);

#endif
