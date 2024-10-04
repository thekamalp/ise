// InfiniScroll Engine
// ise_pci.h
// PCI related header
//
// Kamal Pillai
// 1/30/2019

#ifndef __ISE_PCI_H
#define __ISE_PCI_H

#define ISE_PCI_CHECK_IRQ                      0x1a
#define ISE_PCI_CHECK_FUNCTION                 0xb101
#define ISE_PCI_CHECK_STRING                   0x20494350  // " ICP"

#define ISE_PCI_CONFIG_ADDRESS                 0x0CF8
#define ISE_PCI_CONFIG_DATA                    0x0CFC

#define ISE_PCI_MAX_SLOTS                      32
#define ISE_PCI_MAX_FUNC                       8

// PCI vendor_id
#define ISE_PCI_VENDOR_INVALID                 0xFFFF

#define ISE_PCI_VENDOR_CMEDIA                  0x13F6
#define ISE_PCI_DEVICE_CMEDIA_CMI8738_0        0x0011
#define ISE_PCI_DEVICE_CMEDIA_CMI8738          0x0111

#define ISE_PCI_VENDOR_ULI                     0x10B9
#define ISE_PCI_DEVICE_ULI_CMI8738             0x0111

// PCI classes - bits [23:16] of class code
#define ISE_PCI_CLASS_UNKNOWN                  0x00
#define ISE_PCI_CLASS_MASS_STORAGE             0x01
#define ISE_PCI_CLASS_NETWORK                  0x02
#define ISE_PCI_CLASS_DISPLAY                  0x03
#define ISE_PCI_CLASS_MULTIMEDIA               0x04
#define ISE_PCI_CLASS_MEMORY                   0x05
#define ISE_PCI_CLASS_BRIDGE                   0x06
#define ISE_PCI_CLASS_COMMUNICATION            0x07
#define ISE_PCI_CLASS_PERIPHERAL               0x08
#define ISE_PCI_CLASS_INPUT_DEVICE             0x09
#define ISE_PCI_CLASS_DOCKING_STATION          0x0A
#define ISE_PCI_CLASS_PROCESSOR                0x0B
#define ISE_PCI_CLASS_SERIAL_BUS               0x0C
#define ISE_PCI_CLASS_WIRELESS                 0x0D

// PCI subclasses - bits [15:8] of class code
// mass storage
#define ISE_PCI_SUBCLASS_MS_SCSI               0x00
#define ISE_PCI_SUBCLASS_MS_IDE                0x01
#define ISE_PCI_SUBCLASS_MS_FLOPPY             0x02
#define ISE_PCI_SUBCLASS_MS_IPI                0x03
#define ISE_PCI_SUBCLASS_MS_RAID               0x04
#define ISE_PCI_SUBCLASS_MS_ATA                0x05
#define ISE_PCI_SUBCLASS_MS_SATA               0x06
#define ISE_PCI_SUBCLASS_MS_SA_SCSI            0x07
#define ISE_PCI_SUBCLASS_MS_NV_MEMORY          0x08

// network
#define ISE_PCI_SUBCLASS_NET_ETHERNET          0x00
#define ISE_PCI_SUBCLASS_NET_TOKEN_RING        0x01
#define ISE_PCI_SUBCLASS_NET_FDDI              0x02
#define ISE_PCI_SUBCLASS_NET_ATM               0x03
#define ISE_PCI_SUBCLASS_NET_ISDN              0x04
#define ISE_PCI_SUBCLASS_NET_WORLDFIP          0x05
#define ISE_PCI_SUBCLASS_NET_PICMG             0x06
#define ISE_PCI_SUBCLASS_NET_INFINIBAND        0x07
#define ISE_PCI_SUBCLASS_NET_FABRIC            0x08

// display
#define ISE_PCI_SUBCLASS_DISP_VGA              0x00
#define ISE_PCI_SUBCLASS_DISP_XGA              0x01
#define ISE_PCI_SUBCLASS_DISP_3D               0x02

// multimedia
#define ISE_PCI_SUBCLASS_MM_VIDEO              0x00
#define ISE_PCI_SUBCLASS_MM_AUDIO              0x01
#define ISE_PCI_SUBCLASS_MM_TELEPHONY          0x02
#define ISE_PCI_SUBCLASS_MM_AUDIO_DEVICE       0x03

// memory
#define ISE_PCI_SUBCLASS_MEM_RAM               0x00
#define ISE_PCI_SUBCLASS_MEM_FLASH             0x01

// bridge
#define ISE_PCI_SUBCLASS_BR_HOST               0x00
#define ISE_PCI_SUBCLASS_BR_ISA                0x01
#define ISE_PCI_SUBCLASS_BR_EISA               0x02
#define ISE_PCI_SUBCLASS_BR_MCA                0x03
#define ISE_PCI_SUBCLASS_BR_PCI                0x04
#define ISE_PCI_SUBCLASS_BR_PCMCIA             0x05
#define ISE_PCI_SUBCLASS_BR_NUBUS              0x06
#define ISE_PCI_SUBCLASS_BR_CARDBUS            0x07
#define ISE_PCI_SUBCLASS_BR_RACEWAY            0x08
#define ISE_PCI_SUBCLASS_BR_PCI2               0x09
#define ISE_PCI_SUBCLASS_BR_INFINIBAND         0x0A

// communication

// peripheral

// input device

// docking station

// processor

// serial bus
#define ISE_PCI_SUBCLASS_SB_FIREWIRE           0x00
#define ISE_PCI_SUBCLASS_SB_ACCESS             0x01
#define ISE_PCI_SUBCLASS_SB_SSA                0x02
#define ISE_PCI_SUBCLASS_SB_USB                0x03
#define ISE_PCI_SUBCLASS_SB_FIBRE              0x04
#define ISE_PCI_SUBCLASS_SB_SMBUS              0x05
#define ISE_PCI_SUBCLASS_SB_INFINIBAND         0x06
#define ISE_PCI_SUBCLASS_SB_IPMI               0x07
#define ISE_PCI_SUBCLASS_SB_SERCOS             0x08
#define ISE_PCI_SUBCLASS_SB_CANBUS             0x09

// wireless


// PCI interfaces - bits[7:0] of class code
// usb interfaces
#define ISE_PCI_INTF_USB_UHCI                  0x00
#define ISE_PCI_INTF_USB_OHCI                  0x10
#define ISE_PCI_INTF_USB_EHCI                  0x20
#define ISE_PCI_INTF_USB_XHCI                  0x30
#define ISE_PCI_INTF_USB_DEVICE                0xFE

// generate a config address
#define ISE_PCI_CONFIG_GEN_ADDR(b, s, f, o)     ((((uint32_t)(b) & 0xff) << 16) | \
                                                 (((uint32_t)(s) & 0x1f) << 11) | \
                                                 (((uint32_t)(f) & 0x7) << 8) | \
                                                 ((uint32_t)(o) & 0xff) | \
                                                 0x80000000L)

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t revision_id;
    uint32_t class_code;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    void* priv;  // driver specific data struct
} ise_pci_header_t;

typedef struct {
    ise_pci_header_t slot[ISE_PCI_MAX_SLOTS][ISE_PCI_MAX_FUNC];
} ise_pci_t;

extern ise_pci_t ise_pci;

// check for PCI bus - 0 is success, anything else is failure
int ise_pci_detect();
uint32_t ise_pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void ise_pci_write_config16(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t data);
void ise_pci_write_config32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t data);
uint32_t ise_pci_bar_size(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar);

void ise_pci_scan();

// Register or unregister an interrupt handler
bool ise_pci_register_isr(int interrupt_num, void (_interrupt FAR *isr)());
void ise_pci_unregister_isr(int interrupt_num, void (_interrupt FAR *isr)());

#endif  // #ifndef __ISE_PCI_H