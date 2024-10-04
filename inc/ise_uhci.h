// InfiniScroll Engine
// ise_uhci.h
// Universal Host Controller Interface
//
// Kamal Pillai
// 2/4/2019

#ifndef __ISE_UHCI_H
#define __ISE_UHCI_H

// full class code for UHCI compliant PCI host controllers
#define ISE_UHCI_PCI_CLASS ((ISE_PCI_CLASS_SERIAL_BUS << 16) | (ISE_PCI_SUBCLASS_SB_USB << 8) | (ISE_PCI_INTF_USB_UHCI))

// maximum root hub ports
#define ISE_UHCI_RH_MAX_PORTS                  8

// UHCI standard registers
// offset in bytes from BAR4 (0x20) in pci config space
#define ISE_UHCI_REG_USBCMD                    0x00
#define ISE_UHCI_REG_USBSTS                    0x02
#define ISE_UHCI_REG_USBINTR                   0x04
#define ISE_UHCI_REG_FRNUM                     0x06
#define ISE_UHCI_REG_FRBASEADD                 0x08
#define ISE_UHCI_REG_SOFMOD                    0x0C
#define ISE_UHCI_REG_PORTSC                    0x10   // add 1 word for each additional port on device

// maximum number of 8KB pools of memory that can be used by driver
#define ISE_UHCI_MAX_MEM_POOL                  8

#pragma pack(push, 1)
// A queue is 2 Dwords, defined by UHCI
// an additional 2 Dwords are used for the driver
// (Queues must be 16B aligned, so this doesn't waste space)
typedef struct {
    uint32_t head_link;
    uint32_t current_element_link;
    uint32_t start_element_link;
    uint32_t end_element_link;
} ise_uhci_queue_t;

// A transfer descriptor is 4 Bytes, as defined by UHCI
typedef struct {
    uint32_t link;
    uint32_t command;
    uint32_t device;
    uint32_t buffer;
} ise_uhci_td_t;
#pragma pack(pop)

typedef struct {
    // Base I/O address
    uint32_t usb_base;
    // number of ports attached UHCI root hub
    int num_ports;
    // bit array to indicate a free 16B entry in memory pool
    ise_mem_pool_alloc_t mem_alloc[ISE_UHCI_MAX_MEM_POOL][16];
    // pointers to memory pools
    uint32_t* mem_pool[ISE_UHCI_MAX_MEM_POOL];
    // pointer to 4KB aligned frame list
    uint32_t* frame_list;
    // pointer to bulk and control queues
    ise_uhci_queue_t* control_queue;
    ise_uhci_queue_t* bulk_queue;
} ise_uhci_t;

// detects and install any uhci devices
// pci bus must be pre-scanned
void ise_uhci_install();
void ise_uhci_uninstall();

void ise_uhci_debug(FILE* file, ise_usb_hc_t* ise_usb);

#endif  //  #ifndef __ISE_UHCI_H
