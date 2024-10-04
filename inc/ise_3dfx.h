// InfiniScroll Engine
// ise_3dfx.h
// 3dfx Voodoo/Voodoo2 HW acceleration
//
// Kamal Pillai
// 5/15/2020

#ifndef __ISE_3DFX_H
#define __ISE_3DFX_H

#define ISE_PCI_VENDOR_3DFX                      0x121A
#define ISE_PCI_DEVICE_3DFX_VOODOO               0x0001
#define ISE_PCI_DEVICE_3DFX_VOODOO2              0x0002

// CPI config space registers
#define ISE_3DFX_PCI_INIT_ENABLE                 0x40
#define ISE_3DFX_PCI_INIT_WRITE_EN               0x1
#define ISE_3DFX_PCI_INIT_FIFO_EN                0x2
#define ISE_3DFX_PCI_INIT_DAC_REG_EN             0x4

#define ISE_3DFX_PCI_VCLK_ENABLE                 0xC0     // write of 0 to this address enables vclk
#define ISE_3DFX_PCI_VCLK_DISABLE                0xE0     // write of 0 to this address disables vclk

// Register address fields for VooDoo1/2, not directly related to the register selection
#define ISE_3DFX_VOODOO_ALT_MAP                  0x200000
#define ISE_3DFX_VOODOO_BYTE_SWAP                0x100000
#define ISE_3DFX_VOODOO_WRAP_MASK                0x0FC000
#define ISE_3DFX_VOODOO_WRAP_START               0x004000

#define ISE_3DFX_VOODOO_CHIP_ALL                 0x000000
#define ISE_3DFX_VOODOO_CHIP_CHUCK               0x000400
#define ISE_3DFX_VOODOO_CHIP_BRUCE0              0x000800
#define ISE_3DFX_VOODOO_CHIP_BRUCE1              0x001000
#define ISE_3DFX_VOODOO_CHIP_BRUCE2              0x002000
#define ISE_3DFX_VOODOO_CHIP_ALL_BRUCE           (ISE_3DFX_VOODOO_CHIP_BRUCE2 | ISE_3DFX_VOODOO_CHIP_BRUCE1 | ISE_3DFX_VOODOO_CHIP_BRUCE0)

// register definitions for VooDoo1/2 chips
#define ISE_3DFX_VOODOO_STATUS                   0x000   //  31:0 Chuck R Yes / n/a Voodoo2 Graphics Status
#define ISE_3DFX_VOODOO_INTR_CTRL                0x004   //  31:0 Chuck R/W Yes / No Interrupt Status and Control
#define ISE_3DFX_VOODOO_VERTEX_AX                0x008   //  15:0 Chuck+Bruce% W Yes / Yes Vertex A x-coordinate location (12.4 format)
#define ISE_3DFX_VOODOO_VERTEX_AY                0x00C   //  15:0 Chuck+Bruce% W Yes / Yes Vertex A y-coordinate location (12.4 format)
#define ISE_3DFX_VOODOO_VERTEX_BX                0x010   //  15:0 Chuck+Bruce% W Yes / Yes Vertex B x-coordinate location (12.4 format)
#define ISE_3DFX_VOODOO_VERTEX_BY                0x014   //  15:0 Chuck+Bruce% W Yes / Yes Vertex B y-coordinate location (12.4 format)
#define ISE_3DFX_VOODOO_VERTEX_CX                0x018   //  15:0 Chuck+Bruce% W Yes / Yes Vertex C x-coordinate location (12.4 format)
#define ISE_3DFX_VOODOO_VERTEX_CY                0x01C   //  15:0 Chuck+Bruce% W Yes / Yes Vertex C y-coordinate location (12.4 format)

#define ISE_3DFX_VOODOO_START_R                  0x020   //  23:0 Chuck W Yes / Yes Starting Red parameter (12.12 format)
#define ISE_3DFX_VOODOO_START_G                  0x024   //  23:0 Chuck W Yes / Yes Starting Green parameter (12.12 format)
#define ISE_3DFX_VOODOO_START_B                  0x028   //  23:0 Chuck W Yes / Yes Starting Blue parameter (12.12 format)
#define ISE_3DFX_VOODOO_START_Z                  0x02C   //  31:0 Chuck W Yes / Yes Starting Z parameter (20.12 format)
#define ISE_3DFX_VOODOO_START_A                  0x030   //  23:0 Chuck W Yes / Yes Starting Alpha parameter (12.12 format)
#define ISE_3DFX_VOODOO_START_S                  0x034   //  31:0 Bruce* W Yes / Yes Starting S/W parameter (14.18 format)
#define ISE_3DFX_VOODOO_START_T                  0x038   //  31:0 Bruce* W Yes / Yes Starting T/W parameter (14.18 format)
#define ISE_3DFX_VOODOO_START_W                  0x03C   //  31:0 Chuck+Bruce* W Yes / Yes Starting 1/W parameter (2.30 format)

#define ISE_3DFX_VOODOO_D_RD_X                   0x040   //  23:0 Chuck W Yes / Yes Change in Red with respect to X (12.12 format)
#define ISE_3DFX_VOODOO_D_GD_X                   0x044   //  23:0 Chuck W Yes / Yes Change in Green with respect to X (12.12 format)
#define ISE_3DFX_VOODOO_D_BD_X                   0x048   //  23:0 Chuck W Yes / Yes Change in Blue with respect to X (12.12 format)
#define ISE_3DFX_VOODOO_D_ZD_X                   0x04C   //  31:0 Chuck W Yes / Yes Change in Z with respect to X (20.12 format)
#define ISE_3DFX_VOODOO_D_AD_X                   0x050   //  23:0 Chuck W Yes / Yes Change in Alpha with respect to X (12.12 format)
#define ISE_3DFX_VOODOO_D_SD_X                   0x054   //  31:0 Bruce* W Yes / Yes Change in S/W with respect to X (14.18 format)
#define ISE_3DFX_VOODOO_D_TD_X                   0x058   //  31:0 Bruce* W Yes / Yes Change in T/W with respect to X (14.18 format)
#define ISE_3DFX_VOODOO_D_WD_X                   0x05C   //  31:0 Chuck+Bruce* W Yes / Yes Change in 1/W with respect to X (2.30 format)

#define ISE_3DFX_VOODOO_D_RD_Y                   0x060   //  23:0 Chuck W Yes / Yes Change in Red with respect to Y (12.12 format)
#define ISE_3DFX_VOODOO_D_GD_Y                   0x064   //  23:0 Chuck W Yes / Yes Change in Green with respect to Y (12.12 format)
#define ISE_3DFX_VOODOO_D_BD_Y                   0x068   //  23:0 Chuck W Yes / Yes Change in Blue with respect to Y (12.12 format)
#define ISE_3DFX_VOODOO_D_ZD_Y                   0x06C   //  31:0 Chuck W Yes / Yes Change in Z with respect to Y (20.12 format)
#define ISE_3DFX_VOODOO_D_AD_Y                   0x070   //  23:0 Chuck W Yes / Yes Change in Alpha with respect to Y (12.12 format)
#define ISE_3DFX_VOODOO_D_SD_Y                   0x074   //  31:0 Bruce* W Yes / Yes Change in S/W with respect to Y (14.18 format)
#define ISE_3DFX_VOODOO_D_TD_Y                   0x078   //  31:0 Bruce* W Yes / Yes Change in T/W with respect to Y (14.18 format)
#define ISE_3DFX_VOODOO_D_WD_Y                   0x07C   //  31:0 Chuck+Bruce* W Yes / Yes Change in 1/W with respect to Y (2.30 format)

#define ISE_3DFX_VOODOO_TRIANGLE_CMD             0x080   //  31 Chuck+Bruce% W Yes / Yes Execute TRIANGLE command (floating point)
#define ISE_3DFX_VOODOO_RESERVED_0               0x084   //  n/a n/a W n/a
#define ISE_3DFX_VOODOO_FVERTEX_AX               0x088   //  31:0 Chuck+Bruce% W Yes / Yes Vertex A x-coordinate location (floating point)
#define ISE_3DFX_VOODOO_FVERTEX_AY               0x08C   //  31:0 Chuck+Bruce% W Yes / Yes Vertex A y-coordinate location (floating point)
#define ISE_3DFX_VOODOO_FVERTEX_BX               0x090   //  31:0 Chuck+Bruce% W Yes / Yes Vertex B x-coordinate location (floating point)
#define ISE_3DFX_VOODOO_FVERTEX_BY               0x094   //  31:0 Chuck+Bruce% W Yes / Yes Vertex B y-coordinate location (floating point)
#define ISE_3DFX_VOODOO_FVERTEX_CX               0x098   //  31:0 Chuck+Bruce% W Yes / Yes Vertex C x-coordinate location (floating point)
#define ISE_3DFX_VOODOO_FVERTEX_CY               0x09C   //  31:0 Chuck+Bruce% W Yes / Yes Vertex C y-coordinate location (floating point)

#define ISE_3DFX_VOODOO_FSTART_R                 0x0A0   //  31:0 Chuck W Yes / Yes Starting Red parameter (floating point)
#define ISE_3DFX_VOODOO_FSTART_G                 0x0A4   //  31:0 Chuck W Yes / Yes Starting Green parameter (floating point)
#define ISE_3DFX_VOODOO_FSTART_B                 0x0A8   //  31:0 Chuck W Yes / Yes Starting Blue parameter (floating point)
#define ISE_3DFX_VOODOO_FSTART_Z                 0x0AC   //  31:0 Chuck W Yes / Yes Starting Z parameter (floating point)
#define ISE_3DFX_VOODOO_FSTART_A                 0x0B0   //  31:0 Chuck W Yes / Yes Starting Alpha parameter (floating point)
#define ISE_3DFX_VOODOO_FSTART_S                 0x0B4   //  31:0 Bruce* W Yes / Yes Starting S/W parameter (floating point)
#define ISE_3DFX_VOODOO_FSTART_T                 0x0B8   //  31:0 Bruce* W Yes / Yes Starting T/W parameter (floating point)
#define ISE_3DFX_VOODOO_FSTART_W                 0x0BC   //  31:0 Chuck+Bruce* W Yes / Yes Starting 1/W parameter (floating point)

#define ISE_3DFX_VOODOO_FD_RD_X                  0x0C0   //  31:0 Chuck W Yes / Yes Change in Red with respect to X (floating point)
#define ISE_3DFX_VOODOO_FD_GD_X                  0x0C4   //  31:0 Chuck W Yes / Yes Change in Green with respect to X (floating point)
#define ISE_3DFX_VOODOO_FD_BD_X                  0x0C8   //  31:0 Chuck W Yes / Yes Change in Blue with respect to X (floating point)
#define ISE_3DFX_VOODOO_FD_ZD_X                  0x0CC   //  31:0 Chuck W Yes / Yes Change in Z with respect to X (floating point)
#define ISE_3DFX_VOODOO_FD_AD_X                  0x0D0   //  31:0 Chuck W Yes / Yes Change in Alpha with respect to X (floating point)
#define ISE_3DFX_VOODOO_FD_SD_X                  0x0D4   //  31:0 Bruce* W Yes / Yes Change in S/W with respect to X (floating point)
#define ISE_3DFX_VOODOO_FD_TD_X                  0x0D8   //  31:0 Bruce* W Yes / Yes Change in T/W with respect to X (floating point)
#define ISE_3DFX_VOODOO_FD_WD_X                  0x0DC   //  31:0 Chuck+Bruce* W Yes / Yes Change in 1/W with respect to X (floating point)
#define ISE_3DFX_VOODOO_FD_RD_Y                  0x0E0   //  31:0 Chuck W Yes / Yes Change in Red with respect to Y (floating point)
#define ISE_3DFX_VOODOO_FD_GD_Y                  0x0E4   //  31:0 Chuck W Yes / Yes Change in Green with respect to Y (floating point)
#define ISE_3DFX_VOODOO_FD_BD_Y                  0x0E8   //  31:0 Chuck W Yes / Yes Change in Blue with respect to Y (floating point)
#define ISE_3DFX_VOODOO_FD_ZD_Y                  0x0EC   //  31:0 Chuck W Yes / Yes Change in Z with respect to Y (floating point)
#define ISE_3DFX_VOODOO_FD_AD_Y                  0x0F0   //  31:0 Chuck W Yes / Yes Change in Alpha with respect to Y (floating point)
#define ISE_3DFX_VOODOO_FD_SD_Y                  0x0F4   //  31:0 Bruce* W Yes / Yes Change in S/W with respect to Y (floating point)
#define ISE_3DFX_VOODOO_FD_TD_Y                  0x0F8   //  31:0 Bruce* W Yes / Yes Change in T/W with respect to Y (floating point)
#define ISE_3DFX_VOODOO_FD_WD_Y                  0x0FC   //  31:0 Chuck+Bruce* W Yes / Yes Change in 1/W with respect to Y (floating point)

#define ISE_3DFX_VOODOO_FTRIANGLE_CMD            0x100   //  31 Chuck+Bruce% W Yes / Yes Execute TRIANGLE command (floating point)
#define ISE_3DFX_VOODOO_FBZ_COLOR_PATH           0x104   //  29:0 Chuck+Bruce% R/W Yes / Yes Chuck Color Path Control
#define ISE_3DFX_VOODOO_FOG_MODE                 0x108   //  7:0 Chuck R/W Yes / Yes Fog Mode Control
#define ISE_3DFX_VOODOO_ALPHA_MODE               0x10C   //  31:0 Chuck R/W Yes / Yes Alpha Mode Control
#define ISE_3DFX_VOODOO_FBZ_MODE                 0x110   //  21:0 Chuck R/W No / Yes RGB Buffer and Depth-Buffer Control
#define ISE_3DFX_VOODOO_LFB_MODE                 0x114   //  16:0 Chuck R/W No / Yes Linear Frame Buffer Mode Control
#define ISE_3DFX_VOODOO_CLIP_LEFT_RIGHT          0x118   //  31:0 Chuck R/W No / Yes Left and Right of Clipping Register
#define ISE_3DFX_VOODOO_CLIP_LOW_Y_HIGH_Y        0x11C   //  31:0 Chuck R/W No / Yes Top and Bottom of Clipping Register

#define ISE_3DFX_VOODOO_NOP_CMD                  0x120   //  1:0 Chuck+Bruce% W No / Yes Execute NOP command
#define ISE_3DFX_VOODOO_FASTFILL_CMD             0x124   //  n/a Chuck W No / Yes Execute FASTFILL command
#define ISE_3DFX_VOODOO_SWAPBUFFER_CMD           0x128   //  9:0 Chuck W No / Yes Execute SWAPBUFFER command
#define ISE_3DFX_VOODOO_FOG_COLOR                0x12C   //  23:0 Chuck W No / Yes Fog Color Value
#define ISE_3DFX_VOODOO_ZA_COLOR                 0x130   //  31:0 Chuck W No / Yes Constant Alpha/Depth Value
#define ISE_3DFX_VOODOO_CHROMA_KEY               0x134   //  23:0 Chuck+Bruce* W No / Yes Chroma Key Compare Value
#define ISE_3DFX_VOODOO_CHROMA_RANGE             0x138   //  27:0 Chuck+Bruce* W No / Yes Chroma Range Compare Values,modes,enable
#define ISE_3DFX_VOODOO_USER_INTR_CMD            0x13C   //  9:0 Chuck W No / Yes Execute USERINTERRUPT command

#define ISE_3DFX_VOODOO_STIPPLE                  0x140   //  31:0 Chuck R/W No / Yes Rendering Stipple Value
#define ISE_3DFX_VOODOO_COLOR_0                  0x144   //  31:0 Chuck R/W No / Yes Constant Color #0
#define ISE_3DFX_VOODOO_COLOR_1                  0x148   //  31:0 Chuck R/W No / Yes Constant Color #1
#define ISE_3DFX_VOODOO_FBI_PIXELS_IN            0x14C   //  23:0 Chuck R n/a Pixel Counter (Number pixels processed)
#define ISE_3DFX_VOODOO_FBI_CHROMA_FAIL          0x150   //  23:0 Chuck R n/a Pixel Counter (Number pixels failed Chroma test)
#define ISE_3DFX_VOODOO_FBI_ZFUNC_FAIL           0x154   //  23:0 Chuck R n/a Pixel Counter (Number pixels failed Z test)
#define ISE_3DFX_VOODOO_FBI_AFUNC_FAIL           0x158   //  23:0 Chuck R n/a Pixel Counter (Number pixels failed Alpha test)
#define ISE_3DFX_VOODOO_FBI_PIXELS_OUT           0x15C   //  23:0 Chuck R n/a Pixel Counter (Number pixels drawn)

#define ISE_3DFX_VOODOO_FOG_TABLE_0              0x160   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_1              0x164   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_2              0x168   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_3              0x16C   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_4              0x170   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_5              0x174   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_6              0x178   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_7              0x17C   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_8              0x180   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_9              0x184   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_10             0x188   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_11             0x18C   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_12             0x190   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_13             0x194   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_14             0x198   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_15             0x19C   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_16             0x1A0   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_17             0x1A4   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_18             0x1A8   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_19             0x1AC   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_20             0x1B0   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_21             0x1B4   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_22             0x1B8   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_23             0x1BC   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_24             0x1C0   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_25             0x1C4   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_26             0x1C8   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_27             0x1CC   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_28             0x1D0   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_29             0x1D4   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_30             0x1D8   //  31:0 Chuck W No / Yes Fog Table
#define ISE_3DFX_VOODOO_FOG_TABLE_31             0x1DC   //  31:0 Chuck W No / Yes Fog Table

#define ISE_3DFX_VOODOO_CMD_FIFO_BASE_ADDR       0x1E0   //  25:0 Chuck R/W (n/a) / No CMDFIFO base address and size
#define ISE_3DFX_VOODOO_CMD_FIFO_BUMP            0x1E4   //  15:0 Chuck R/W (n/a) / No CMDFIFO bump depth
#define ISE_3DFX_VOODOO_CMD_FIFO_RD_PTR          0x1E8   //  31:0 Chuck R/W (n/a) / No CMDFIFO current read pointer
#define ISE_3DFX_VOODOO_CMD_FIFO_A_MIN           0x1EC   //  31:0 Chuck R/W (n/a) / No CMDFIFO current minimum address
#define ISE_3DFX_VOODOO_CMD_FIFO_A_MAX           0x1F0   //  31:0 Chuck R/W (n/a) / No CMDFIFO current maximum address
#define ISE_3DFX_VOODOO_CMD_FIFO_DEPTH           0x1F4   //  15:0 Chuck R/W (n/a) / No CMDFIFO current depth
#define ISE_3DFX_VOODOO_CMD_FIFO_HOLES           0x1F8   //  15:0 Chuck R/W (n/a) / No CMDFIFO number of holes
#define ISE_3DFX_VOODOO_RESERVED_1               0x1FC   //  n/a n/a n/a n/a

#define ISE_3DFX_VOODOO_FBI_INIT_4               0x200   //  12:0 Chuck R/W (n/a) / No Chuck Hardware Initialization (register 4)
#define ISE_3DFX_VOODOO_V_RETRACE                0x204   //  12:0 Chuck R (n/a) / No Vertical Retrace Counter
#define ISE_3DFX_VOODOO_BACK_PORCH               0x208   //  24:0 Chuck R/W (n/a) / No Video Backporch Timing Generator
#define ISE_3DFX_VOODOO_VIDEO_DIMENSIONS         0x20C   //  26:0 Chuck R/W (n/a) / No Video Screen Dimensions
#define ISE_3DFX_VOODOO_FBI_INIT_0               0x210   //  31:0 Chuck R/W (n/a) / No Chuck Hardware Initialization (register 0)
#define ISE_3DFX_VOODOO_FBI_INIT_1               0x214   //  31:0 Chuck R/W (n/a) / No Chuck Hardware Initialization (register 1)
#define ISE_3DFX_VOODOO_FBI_INIT_2               0x218   //  31:0 Chuck R/W (n/a) / No Chuck Hardware Initialization (register 2)
#define ISE_3DFX_VOODOO_FBI_INIT_3               0x21C   //  31:0 Chuck R/W (n/a) / No Chuck Hardware Initialization (register 3)

#define ISE_3DFX_VOODOO_H_SYNC                   0x220   //  26:0 Chuck W (n/a) / No Horizontal Sync Timing Generator
#define ISE_3DFX_VOODOO_V_SYNC                   0x224   //  28:0 Chuck W (n/a) / No Vertical Sync Timing Generator
#define ISE_3DFX_VOODOO_CLUT_DATA                0x228   //  29:0 Chuck W No / Yes Video Color Lookup Table Initialization
#define ISE_3DFX_VOODOO_DAC_DATA                 0x22C   //  13:0 Chuck W (n/a) / No External DAC Initialization
#define ISE_3DFX_VOODOO_MAX_RGB_DELTA            0x230   //  23:0 Chuck W (n/a) / No Max. RGB difference for Video Filtering
#define ISE_3DFX_VOODOO_H_BORDER                 0x234   //  24:0 Chuck W (n/a) / No Horizontal Border Color Control
#define ISE_3DFX_VOODOO_V_BORDER                 0x238   //  24:0 Chuck W (n/a) / No Vertical Border Color Control
#define ISE_3DFX_VOODOO_BORDER_COLOR             0x23C   //  23:0 Chuck W (n/a) / No Video Border Color

#define ISE_3DFX_VOODOO_HV_RETRACE               0x240   //  26:0 Chuck R (n/a) / No Horizontal and Vertical Retrace Counters (synced)
#define ISE_3DFX_VOODOO_FBI_INIT_5               0x244   //  31:0 Chuck R/W (n/a) / No Chuck Hardware Initialization (register 5)
#define ISE_3DFX_VOODOO_FBI_INIT_6               0x248   //  31:0 Chuck R/W (n/a) / No Chuck Hardware Initialization (register 6)
#define ISE_3DFX_VOODOO_FBI_INIT_7               0x24C   //  31:0 Chuck R/W (n/a) / No Chuck Hardware Initialization (register 7)
#define ISE_3DFX_VOODOO_RESERVED_2               0x250   //  n/a n/a n/a n/a
#define ISE_3DFX_VOODOO_RESERVED_3               0x254   //  n/a n/a n/a n/a
#define ISE_3DFX_VOODOO_FBI_SWAP_HISTORY         0x258   //  31:0 Chuck R n/a Swap History Register
#define ISE_3DFX_VOODOO_FBI_TRIANGLES_OUT        0x25C   //  23:0 Chuck R n/a Triangle Counter (Number triangles drawn)

#define ISE_3DFX_VOODOO_S_SETUP_MODE             0x260   //  19:0 Chuck W Yes / Yes Triangle setup mode
#define ISE_3DFX_VOODOO_S_VX                     0x264   //  31:0 Chuck+Bruce* W Yes / Yes Triangle setup X
#define ISE_3DFX_VOODOO_S_VY                     0x268   //  31:0 Chuck+Bruce* W Yes / Yes Triangle setup Y
#define ISE_3DFX_VOODOO_S_ARGB                   0x26C   //  31:0 Chuck+Bruce* W Yes / Yes Triangle setup Alpha, Red, Green, Blue
#define ISE_3DFX_VOODOO_S_RED                    0x270   //  31:0 Chuck W Yes / Yes Triangle setup Red value
#define ISE_3DFX_VOODOO_S_GREEN                  0x274   //  31:0 Chuck W Yes / Yes Triangle setup Green value
#define ISE_3DFX_VOODOO_S_BLUE                   0x278   //  31:0 Chuck W Yes / Yes Triangle setup Blue value
#define ISE_3DFX_VOODOO_S_ALPHA                  0x27C   //  31:0 Chuck W Yes / Yes Triangle setup Alpha value

#define ISE_3DFX_VOODOO_S_VZ                     0x280   //  31:0 Chuck W Yes / Yes Triangle setup Z
#define ISE_3DFX_VOODOO_S_WB                     0x284   //  31:0 Chuck+Bruce* W Yes / Yes Triangle setup Global W
#define ISE_3DFX_VOODOO_S_WTMU_0                 0x288   //  31:0 Bruce* W Yes / Yes Triangle setup Tmu0 & Tmu1 W
#define ISE_3DFX_VOODOO_SS_W0                    0x28C   //  31:0 Bruce* W Yes / Yes Triangle setup Tmu0 & Tmu1 S/W
#define ISE_3DFX_VOODOO_ST_W0                    0x290   //  31:0 Bruce* W Yes / Yes Triangle setup Tmu0 & Tmu1 T/W
#define ISE_3DFX_VOODOO_S_WTMU_1                 0x294   //  31:0 Bruce-1 W Yes / Yes Triangle setup Tmu1 only W
#define ISE_3DFX_VOODOO_SS_WTMU_1                0x298   //  31:0 Bruce-1 W Yes / Yes Triangle setup Tmu1 only S/W
#define ISE_3DFX_VOODOO_ST_WTMU_1                0x29C   //  31:0 Bruce-1 W Yes / Yes Triangle setup Tmu1 only T/W

#define ISE_3DFX_VOODOO_S_DRAW_TRI_CMD           0x2A0   //  31:0 Chuck+Bruce* W Yes / Yes Triangle setup (Draw)
#define ISE_3DFX_VOODOO_S_BEGIN_TRI_CMD          0x2A4   //  31:0 Chuck W Yes / Yes Triangle setup Start New triangle
#define ISE_3DFX_VOODOO_RESERVED_4               0x2A8   //  n/a n/a n/a n/a
#define ISE_3DFX_VOODOO_RESERVED_5               0x2AC   //  n/a n/a n/a n/a
#define ISE_3DFX_VOODOO_RESERVED_6               0x2B0   //  n/a n/a n/a n/a
#define ISE_3DFX_VOODOO_RESERVED_7               0x2B4   //  n/a n/a n/a n/a
#define ISE_3DFX_VOODOO_RESERVED_8               0x2B8   //  n/a n/a n/a n/a
#define ISE_3DFX_VOODOO_RESERVED_9               0x2BC   //  n/a n/a n/a n/a

#define ISE_3DFX_VOODOO_BLT_SRC_BASE_ADDR        0x2C0   //  21:0 Chuck R/W Yes / Yes BitBLT Source base address
#define ISE_3DFX_VOODOO_BLT_DST_BASE_ADDR        0x2C4   //  21:0 Chuck R/W Yes / Yes BitBLT Destination base address
#define ISE_3DFX_VOODOO_BLT_XY_STRIDES           0x2C8   //  27:0 Chuck R/W Yes / Yes BitBLT Source and Destination strides
#define ISE_3DFX_VOODOO_BLT_SRC_CHROMA_RANGE     0x2CC   //  31:0 Chuck R/W Yes / Yes BiBLT Source Chroma key range
#define ISE_3DFX_VOODOO_BLT_DST_CHROMA_RANGE     0x2D0   //  31:0 Chuck R/W Yes / Yes BitBLT Destination Chroma key range
#define ISE_3DFX_VOODOO_BLT_CLIP_X               0x2D4   //  27:0 Chuck R/W Yes / Yes BitBLT Min/Max X clip values
#define ISE_3DFX_VOODOO_BLT_CLIP_Y               0x2D8   //  27:0 Chuck R/W Yes / Yes BitBLT Min/Max Y clip values
#define ISE_3DFX_VOODOO_RESERVED_10              0x2DC   // 

#define ISE_3DFX_VOODOO_BLT_SRC_XY               0x2E0   //  26:0 Chuck R/W Yes / Yes BitBLT Source starting XY coordinates
#define ISE_3DFX_VOODOO_BLT_DST_XY               0x2E4   //  31:0 Chuck R/W Yes / Yes BitBLT Destination starting XY coordinates
#define ISE_3DFX_VOODOO_BLT_SIZE                 0x2E8   //  31:0 Chuck R/W Yes / Yes BitBLT width and height
#define ISE_3DFX_VOODOO_BLT_ROP                  0x2EC   //  15:0 Chuck R/W Yes / Yes BitBLT Raster operations
#define ISE_3DFX_VOODOO_BLT_COLOR                0x2F0   //  31:0 Chuck R/W Yes / Yes BitBLT and foreground background colors
#define ISE_3DFX_VOODOO_RESERVED_11              0x2F4   // 
#define ISE_3DFX_VOODOO_BLT_COMMAND              0x2F8   //  31:0 Chuck R/W Yes / Yes BitBLT command mode
#define ISE_3DFX_VOODOO_BLT_DATA                 0x2FC   //  31:0 Chuck W Yes / Yes BitBLT data for CPU-to-Screen BitBLTs

#define ISE_3DFX_VOODOO_TEXTURE_MODE             0x300   //  30:0 Bruce* W Yes / Yes Texture Mode Control
#define ISE_3DFX_VOODOO_T_LOD                    0x304   //  27:0 Bruce* W Yes / Yes Texture LOD Settings
#define ISE_3DFX_VOODOO_T_DETAIL                 0x308   //  21:0 Bruce* W Yes / Yes Texture LOD Settings
#define ISE_3DFX_VOODOO_TEX_BASE_ADDR            0x30C   //  18:0 Bruce* W Yes / Yes Texture Base Address
#define ISE_3DFX_VOODOO_TEX_BASE_ADDR_1          0x310   //  18:0 Bruce* W Yes / Yes Texture Base Address (supplemental LOD 1)
#define ISE_3DFX_VOODOO_TEX_BASE_ADDR_2          0x314   //  18:0 Bruce* W Yes / Yes Texture Base Address (supplemental LOD 2)
#define ISE_3DFX_VOODOO_TEX_BASE_ADDR_3_8        0x318   //  18:0 Bruce* W Yes / Yes Texture Base Address (supplemental LOD 3-8)
#define ISE_3DFX_VOODOO_TREX_INIT_0              0x31C   //  31:0 Bruce* W No / Yes Bruce Hardware Initialization (register 0)
#define ISE_3DFX_VOODOO_TREX_INIT_1              0x320   //  31:0 Bruce* W No / Yes Bruce Hardware Initialization (register 1)

#define ISE_3DFX_VOODOO_NCC_TABLE0_0             0x324   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 0 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE0_1             0x328   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 0 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE0_2             0x32C   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 0 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE0_3             0x330   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 0 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE0_4             0x334   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 0 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE0_5             0x338   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 0 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE0_6             0x33C   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 0 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE0_7             0x340   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 0 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE0_8             0x344   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 0 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE0_9             0x348   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 0 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE0_10            0x34C   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 0 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE0_11            0x350   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 0 (12 entries)

#define ISE_3DFX_VOODOO_NCC_TABLE1_0             0x354   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 1 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE1_1             0x358   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 1 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE1_2             0x35C   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 1 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE1_3             0x360   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 1 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE1_4             0x364   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 1 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE1_5             0x368   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 1 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE1_6             0x36C   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 1 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE1_7             0x370   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 1 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE1_8             0x374   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 1 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE1_9             0x378   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 1 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE1_10            0x37C   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 1 (12 entries)
#define ISE_3DFX_VOODOO_NCC_TABLE1_11            0x380   //  31:0 or 26:0 Bruce* W No / Yes Narrow Channel Compression Table 1 (12 entries)

// Used to detect ICS
#define ISE_3DFX_ICS_PLL_CLK0_1_INI	             0x55
#define ISE_3DFX_ICS_PLL_CLK0_7_INI	             0x71
#define ISE_3DFX_ICS_PLL_CLK1_B_INI	             0x79

// Used to detect other DACs
#define ISE_3DFX_DAC_VENDOR_ATT                  0x84
#define ISE_3DFX_DAC_DEVICE_ATT20C409            0x09
#define ISE_3DFX_DAC_VENDOR_TI                   0x97
#define ISE_3DFX_DAC_DEVICE_TITVP3409            0x09

// Internal IDs for each DAC
#define ISE_3DFX_DAC_ID_UNKNOWN                  0x00
#define ISE_3DFX_DAC_ID_ATT                      0x01
#define ISE_3DFX_DAC_ID_TI                       0x02
#define ISE_3DFX_DAC_ID_ICS                      0x03

// 3dfx card flags
#define ISE_3DFX_CARD_USE_ALIAS_REG_MAPPING      0x00000001
#define ISE_3DFX_CARD_CMD_FIFO_EN                0x00000002

// Command fifo base starts at 2MB after base address
#define ISE_3DFX_CMD_BASE                        0x200000

#define ISE_3DFX_PRIM_TRIANGLES                  0x0
#define ISE_3DFX_PRIM_TRIANGLE_STRIP             0x1
#define ISE_3DFX_PRIM_TRIANGLE_STRIP_CONT        0x2

typedef struct {
	uint8_t device_id;
	uint8_t slot;
	uint8_t func;
	uint8_t dac_id;
	uint32_t flags;
	uint32_t cmd_fifo_base;
	uint32_t cmd_fifo_size;
	uint32_t cmd_fifO_wr_entry;
} ise_3dfx_card_t;

typedef struct {
	int m;
	int n;
	int p;
} ise_3dfx_pll_clk_t;

void ise_3dfx_install(int slot, int func);

#endif