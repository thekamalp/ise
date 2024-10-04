// InfiniScroll Engine
// ise_ehci.h
// Enhanced Host Controller Interface
//
// Kamal Pillai
// 2/2/2019

#ifndef __ISE_EHCI_H
#define __ISE_EHCI_H

// full class code for EHCI compliant PCI host controllers
#define ISE_EHCI_PCI_CLASS ((ISE_PCI_CLASS_SERIAL_BUS << 16) | (ISE_PCI_SUBCLASS_SB_USB << 8) | (ISE_PCI_INTF_USB_EHCI))

// EHCI standard capability registers
// offset in dwords from BAR in pci config space
#define ISE_EHCI_CAP_HCIVERSION                0x00   // version[31:16], reserved[15:8], caplength[7:0]
#define ISE_EHCI_CAP_HCSPARAMS                 0x01
#define ISE_EHCI_CAP_HCCPARAMS                 0x02
#define ISE_EHCI_CAP_HCSP_PORTROUTE            0x03
#define ISE_EHCI_CAP_HCSP_PORTROUTE_1          0x04

// EHCI operational registers
// offset in dwords from end of capability registers (BAR + (HCIVERSION & 0xff))
#define ISE_EHCI_OP_USBCMD                     0x00
#define ISE_EHCI_OP_USBSTS                     0x01
#define ISE_EHCI_OP_USBINTR                    0x02
#define ISE_EHCI_OP_FRINDEX                    0x03
#define ISE_EHCI_OP_CTRLDSSEGMENT              0x04
#define ISE_EHCI_OP_PERIODICLISTBASE           0x05
#define ISE_EHCI_OP_ASYNCLISTADDR              0x06
// 0x07 - 0x0F reserved
#define ISE_EHCI_OP_CONFIGFLAG                 0x10
#define ISE_EHCI_OP_PORTSC                     0x11   // add 1 dword for each additional port on device

// maximum number of 8KB pools of memory that can be used by driver
#define ISE_EHCI_MAX_MEM_POOL                  8

#pragma pack(push, 1)
typedef struct {
    uint32_t next_qtd;
    uint32_t alt_next_qtd;
    uint32_t command;
    uint32_t buffer[5];
} ise_ehci_qtd_t;

typedef struct {
    ise_ehci_qtd_t qtd32;
    uint32_t high_buffer[5];  // upper 32 bits of the 64 bit address
} ise_ehci_qtd64_t;

typedef struct {
    uint32_t head_link;
    uint32_t device;
    uint32_t mask;
    uint32_t current_link;
    ise_ehci_qtd64_t qtd;
    uint32_t tail_link;
    uint32_t start_link;
    uint32_t end_link;
} ise_ehci_queue_t;

#pragma pack(pop)

typedef struct {
    volatile uint32_t* cap; // base address of capability registers
    volatile uint32_t* op;  // base address of operation registers

    uint8_t flags; // host controller flags from HCCPARAMS
    uint8_t num_ports; // number of ports in root hub
    uint8_t s_mask_offset;
    uint8_t reserved;

    // bit array to indicate a free 32B entry in memory pool
    ise_mem_pool_alloc_t mem_alloc[ISE_EHCI_MAX_MEM_POOL][8];
    // pointers to memory pool
    uint32_t* mem_pool[ISE_EHCI_MAX_MEM_POOL];
    // pointer to 4KB aligned periodic list base
    uint32_t* periodic_list;
    // pointer to bulk and control queues
    ise_ehci_queue_t* control_queue;
    ise_ehci_queue_t* bulk_queue;
    // dummy td - used as alt_next_qtd for short packet handling
    ise_ehci_qtd64_t* dummy_td;
} ise_ehci_t;

void ise_ehci_debug(FILE* file, ise_usb_hc_t* ise_usb);

// detects and install any ehci devices
// pci bus be pre-scanned
void ise_ehci_install();
void ise_ehci_uninstall();

#endif  //  #ifndef __ISE_EHCI_H
