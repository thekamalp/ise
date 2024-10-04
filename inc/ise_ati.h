// InfiniScroll Engine
// ise_ati.h
// ATI HW acceleration
//
// Kamal Pillai
// 4/8/2020

#ifndef __ISE_ATI_H
#define __ISE_ATI_H

#define ISE_PCI_VENDOR_ATI                     0x1002

#define ISE_PCI_DEVICE_ATI_MACH64GX            0x4758
#define ISE_PCI_DEVICE_ATI_MACH64CX            0x4358
#define ISE_PCI_DEVICE_ATI_MACH64CT            0x4354
#define ISE_PCI_DEVICE_ATI_MACH64VT            0x5654
#define ISE_PCI_DEVICE_ATI_MACH64VTB           0x5655
#define ISE_PCI_DEVICE_ATI_MACH64VT4           0x5656

#define ISE_PCI_DEVICE_ATI_3D_RAGE_GT          0x4754
#define ISE_PCI_DEVICE_ATI_3D_RAGEII_GTB       0x4755
#define ISE_PCI_DEVICE_ATI_3D_RAGEIIC_PQFB_AGP 0x475A
#define ISE_PCI_DEVICE_ATI_3D_RAGEIIC_BGA_AGP  0x4757
#define ISE_PCI_DEVICE_ATI_3D_RAGEIIC_PQFP_PCI 0x4756
#define ISE_PCI_DEVICE_ATI_3D_RAGE_PRO_BGA_AGP 0x4742
#define ISE_PCI_DEVICE_ATI_3D_RAGE_PRO_BGA_AGP1X 0x4744
#define ISE_PCI_DEVICE_ATI_3D_RAGE_PRO_BGA_PCI 0x4749
#define ISE_PCI_DEVICE_ATI_3D_RAGE_PRO_PQFP_PCI 0x4750
#define ISE_PCI_DEVICE_ATI_3D_RAGE_PRO_PQFP_LIMITED_3D 0x4751
#define ISE_PCI_DEVICE_ATI_3D_RAGE_LT_PRO_BGA_PCI 0x4C49
#define ISE_PCI_DEVICE_ATI_3D_RAGE_LT_PRO_BGA_AGP 0x4C42
#define ISE_PCI_DEVICE_ATI_3D_RAGE_LT_PRO      0x4C50
#define ISE_PCI_DEVICE_ATI_3D_RAGE_LT          0x4C47
#define ISE_PCI_DEVICE_ATI_RAGE_XL             0x474D
#define ISE_PCI_DEVICE_ATI_RAGE_MOBILITY       0x4C4D

#define ISE_ATI_M64_CRTC_VLINE_CRNT_VLINE     0x0010
#define ISE_ATI_M64_CRTC_OFF_PITCH            0x0014
#define ISE_ATI_M64_CRTC_INT_CNTL             0x0018
#define ISE_ATI_M64_BUS_CNTL                  0x00A0
#define ISE_ATI_M64_DAC_REGS                  0x00C0
#define ISE_ATI_M64_DAC_W_INDEX               0x00C0
#define ISE_ATI_M64_DAC_DATA                  0x00C1
#define ISE_ATI_M64_DAC_MASK                  0x00C2
#define ISE_ATI_M64_DAC_R_INDEX               0x00C3

#define ISE_ATI_M64_GEN_TEST_CNTL             0x00D0

#define ISE_ATI_M64_DST_OFF_PITCH             0x0100
#define ISE_ATI_M64_DST_X                     0x0104
#define ISE_ATI_M64_DST_Y                     0x0108
#define ISE_ATI_M64_DST_Y_X                   0x010C
#define ISE_ATI_M64_DST_WIDTH                 0x0110
#define ISE_ATI_M64_DST_HEIGHT                0x0114
#define ISE_ATI_M64_DST_HEIGHT_WIDTH          0x0118
#define ISE_ATI_M64_DST_X_WIDTH               0x011C

#define ISE_ATI_M64_DST_BRES_LNTH             0x0120
#define ISE_ATI_M64_DST_BRES_ERR              0x0124
#define ISE_ATI_M64_DST_BRES_INC              0x0128
#define ISE_ATI_M64_DST_BRES_DEC              0x012C
#define ISE_ATI_M64_DST_CNTL                  0x0130

#define ISE_ATI_M64_SRC_OFF_PITCH             0x0180
#define ISE_ATI_M64_SRC_X                     0x0184
#define ISE_ATI_M64_SRC_Y                     0x0188
#define ISE_ATI_M64_SRC_Y_X                   0x018C
#define ISE_ATI_M64_SRC_WIDTH1                0x0190
#define ISE_ATI_M64_SRC_HEIGHT1               0x0194
#define ISE_ATI_M64_SRC_HEIGHT1_WIDTH1        0x0198
#define ISE_ATI_M64_SRC_X_START               0x019C
#define ISE_ATI_M64_SRC_Y_START               0x01A0
#define ISE_ATI_M64_SRC_Y_X_START             0x01A4
#define ISE_ATI_M64_SRC_WIDTH2                0x01A8
#define ISE_ATI_M64_SRC_HEIGHT2               0x01AC
#define ISE_ATI_M64_SRC_HEIGHT2_WIDTH2        0x01B0
#define ISE_ATI_M64_SRC_CNTL                  0x01B4

#define ISE_ATI_M64_HOST_DATA                 0x0200
#define ISE_ATI_M64_HOST_CNTL                 0x0240

#define ISE_ATI_M64_PAT_REG0                  0x0280
#define ISE_ATI_M64_PAT_REG1                  0x0284
#define ISE_ATI_M64_PAT_CNTL                  0x0288

#define ISE_ATI_M64_SC_LEFT                   0x02A0
#define ISE_ATI_M64_SC_RIGHT                  0x02A4
#define ISE_ATI_M64_SC_LEFT_RIGHT             0x02A8
#define ISE_ATI_M64_SC_TOP                    0x02AC
#define ISE_ATI_M64_SC_BOTTOM                 0x02B0
#define ISE_ATI_M64_SC_TOP_BOTTOM             0x02B4

#define ISE_ATI_M64_DP_BKGD_CLR               0x02C0
#define ISE_ATI_M64_DP_FRGD_CLR               0x02C4
#define ISE_ATI_M64_DP_WRITE_MASK             0x02C8
#define ISE_ATI_M64_DP_CHAIN_MASK             0x02CC
#define ISE_ATI_M64_DP_PIX_WIDTH              0x02D0
#define ISE_ATI_M64_DP_MIX                    0x02D4
#define ISE_ATI_M64_DP_SRC                    0x02D8
#define ISE_ATI_M64_DP_FRGD_CLR_MIX           0x02DC
#define ISE_ATI_M64_DP_FRGD_BKGD_CLR          0x02E0
#define ISE_ATI_M64_DST_X_Y                   0x02E8
#define ISE_ATI_M64_DST_WIDTH_HEIGHT          0x02EC

#define ISE_ATI_M64_CLR_CMP_CLR               0x0300
#define ISE_ATI_M64_CLR_CMP_MSK               0x0304
#define ISE_ATI_M64_CLR_CMP_CNTL              0x0308
#define ISE_ATI_M64_FIFO_STAT                 0x0310
#define ISE_ATI_M64_CONTEXT_MASK              0x0320
#define ISE_ATI_M64_GUI_STAT                  0x0338


void ise_ati_install(int slot, int func);

#endif
