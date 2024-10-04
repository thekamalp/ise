// InfiniScroll Engine
// ise_ohci.h
// Open Host Controller Interface
//
// Kamal Pillai
// 2/1//2019

#ifndef __ISE_OHCI_H
#define __ISE_OHCI_H

// full class code for OHCI compliant PCI host controllers
#define ISE_OHCI_PCI_CLASS ((ISE_PCI_CLASS_SERIAL_BUS << 16) | (ISE_PCI_SUBCLASS_SB_USB << 8) | (ISE_PCI_INTF_USB_OHCI))

// OHCI operational registers
// offset from base address in dwords
#define ISE_OHCI_HC_REVISION                   0x0
#define ISE_OHCI_HC_CONTROL                    0x1
#define ISE_OHCI_HC_COMMAND_STATUS             0x2
#define ISE_OHCI_HC_INTERRUPT_STATUS           0x3
#define ISE_OHCI_HC_INTERRUPT_ENABLE           0x4
#define ISE_OHCI_HC_INTERRUPT_DISABLE          0x5

#define ISE_OHCI_HC_HCCA                       0x6
#define ISE_OHCI_HC_PERIOD_CURRENT_ED          0x7
#define ISE_OHCI_HC_CONTROL_HEAD_ED            0x8
#define ISE_OHCI_HC_CONTROL_CURRENT_ED         0x9
#define ISE_OHCI_HC_BULK_HEAD_ED               0xA
#define ISE_OHCI_HC_BULK_CURRENT_ED            0xB
#define ISE_OHCI_HC_DONE_HEAD                  0xC

#define ISE_OHCI_HC_FM_INTERVAL                0xD
#define ISE_OHCI_HC_FM_REMAINING               0xE
#define ISE_OHCI_HC_FM_NUMBER                  0xF
#define ISE_OHCI_HC_PERIODIC_START             0x10
#define ISE_OHCI_HC_LS_THRESHOLD               0x11
#define ISE_OHCI_HC_RH_DESCRIPTOR_A            0x12
#define ISE_OHCI_HC_RH_DESCRIPTOR_B            0x13
#define ISE_OHCI_HC_RH_STATUS                  0x14
#define ISE_OHCI_HC_RH_PORT_STATUS             0x15  // array up to number of ports

// completion codes
#define ISE_OHCI_CC_NO_ERROR                   0x0
#define ISE_OHCI_CC_CRC                        0x1
#define ISE_OHCI_CC_BIT_STUFF                  0x2
#define ISE_OHCI_CC_DATA_TOGGLE_MISMATCH       0x3
#define ISE_OHCI_CC_STALL                      0x4
#define ISE_OHCI_CC_DEVICE_NOT_RESPONDING      0x5
#define ISE_OHCI_CC_PID_CHECK_FAILURE          0x6
#define ISE_OHCI_CC_UNEXPECTED_PID             0x7
#define ISE_OHCI_CC_DATA_OVERRUN               0x8
#define ISE_OHCI_CC_DATA_UNDERRUN              0x9
#define ISE_OHCI_CC_BUFFER_OVERRUN             0xC
#define ISE_OHCI_CC_BUFFER_UNDERRUN            0xD
#define ISE_OHCI_CC_NOT_ACCESSED               0xE

// Host Controller Communications Area

// number of dwords in host controller communications area
// this region must be 256B aligned
#define ISE_OHCI_HCCA_REGS                     0x40

// offset from base address in dwords
#define ISE_OHCI_HCCA_INTERRUPT_TABLE          0x0  // 32 dwords in interrupt table
#define ISE_OHCI_HCCA_FRAME_NUMBER             0x20 // byte offset 0x80
#define ISE_OHCI_HCCA_DONE_HEAD                0x21 // byte offset 0x84

// maximum number of 8KB pools of memory that can be used by driver
#define ISE_OHCI_MAX_MEM_POOL                  8

#pragma pack(push, 1)
// OHCI transfer descriptors
typedef struct {
    uint32_t command;
    uint32_t buffer;
    uint32_t next;
    uint32_t buffer_end;
    uint32_t sw_next;
    uint32_t reserved[3];
} ise_ohci_td_t;

// OHCI calls queues end point descriptors.  We'll use queue nomenclature
// to be consistent with other host controller types
// First 16 Bytes are read by host controller, and last 16 bytes are used by the driver
typedef struct {
    uint32_t command;
    uint32_t null_pointer;  // should be 0; current pointer matches null to denote end of queue
    uint32_t current_pointer;
    uint32_t next_queue;
    uint32_t head_pointer;
    uint32_t tail_pointer;
    uint32_t end_queue;
    uint32_t qtype;
} ise_ohci_queue_t;

#pragma pack(pop)


typedef struct {
    // base address
    volatile uint32_t* reg;
    // hcca memory
    volatile uint32_t* hcca;
    // number of ports
    int num_ports;
    // bit array to indicate a free 16B entry in memory pool
    ise_mem_pool_alloc_t mem_alloc[ISE_OHCI_MAX_MEM_POOL][16];
    // pointers to memory pools
    uint32_t* mem_pool[ISE_OHCI_MAX_MEM_POOL];
    // a dummy td used for tail of all queues
    uint32_t dummy_tail_td;
    // pointer to bulk and control queues
    ise_ohci_queue_t* control_queue;
    ise_ohci_queue_t* bulk_queue;
} ise_ohci_t;

// detects and install any ohci devices
// pci bus be pre-scanned
void ise_ohci_install();
void ise_ohci_uninstall();

#endif  //  #ifndef __ISE_OHCI_H