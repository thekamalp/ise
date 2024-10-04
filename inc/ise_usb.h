// InfiniScroll Engine
// ise_usb.h
// Universal Serial Bus driver
//
// Kamal Pillai
// 2/11/2019

#ifndef __ISE_USB_H
#define __ISE_USB_H

// USB transfer speeds
#define ISE_USB_SPEED_LOW                      0
#define ISE_USB_SPEED_FULL                     1
#define ISE_USB_SPEED_HIGH                     2   // USB 2.0 speeds

// USB Packet IDs
#define ISE_USB_PID_SETUP                      0x2D
#define ISE_USB_PID_IN                         0x69
#define ISE_USB_PID_OUT                        0xE1

// These are pointers to HCD driver specific structures
// Using these instead of void* gives a little more type checking
// and better readibility
typedef struct {} ise_usb_hcd_t;      // Host controller`
typedef struct {} ise_usb_queue_t;    // Host controller queue
typedef struct {} ise_usb_td_t;       // Transfer descriptor

// The transfer descriptor information
// This is passed to the hcd to format the td into
// one readable by the HW
typedef struct {
    bool spd;                  // Short packet detect
    int speed;                 // USB transfer speed
    bool ioc;                  // Interrupt on completion
    int length;                // Bytes to transfer
    bool dt;                   // Data toggle
    int end_pt;                // End point
    int device;                // Device address
    int pid;                   // Packet ID
    void* buffer;              // Pointer to buffer to be read/written
} ise_usb_td_info_t;

// structures and defines for device enumeration/configuration

#define ISE_USB_REQUEST_TYPE_OUT               0x00
#define ISE_USB_REQUEST_TYPE_IN                0x80
#define ISE_USB_REQUEST_TYPE_STANDARD          0x00
#define ISE_USB_REQUEST_TYPE_CLASS             0x20
#define ISE_USB_REQUEST_TYPE_VENDOR            0x40
#define ISE_USB_REQUEST_TYPE_DEVICE            0x00
#define ISE_USB_REQUEST_TYPE_INTERFACE         0x01
#define ISE_USB_REQUEST_TYPE_ENDPOINT          0x02
#define ISE_USB_REQUEST_TYPE_OTHER             0x03

#define ISE_USB_REQUEST_GET_STATUS             0x00
#define ISE_USB_REQUEST_CLEAR_FEATURE          0x01
#define ISE_USB_REQUEST_SET_FEATURE            0x03
#define ISE_USB_REQUEST_SET_ADDRESS            0x05
#define ISE_USB_REQUEST_GET_DESCRIPTOR         0x06
#define ISE_USB_REQUEST_SET_DESCRIPTOR         0x07
#define ISE_USB_REQUEST_GET_CONFIGURATION      0x08
#define ISE_USB_REQUEST_SET_CONFIGURATION      0x09
#define ISE_USB_REQUEST_GET_INTERFACE          0x0A
#define ISE_USB_REQUEST_SET_INTERFACE          0x0B
#define ISE_USB_REQUEST_SYNCH_FRAME            0x0C

#define ISE_USB_DESC_TYPE_DEVICE               0x01
#define ISE_USB_DESC_TYPE_CONFIGURATION        0x02
#define ISE_USB_DESC_TYPE_STRING               0x03
#define ISE_USB_DESC_TYPE_INTERFACE            0x04
#define ISE_USB_DESC_TYPE_ENDPOINT             0x05
#define ISE_USB_DESC_TYPE_DEVICE_QUALIFIER     0x06
#define ISE_USB_DESC_TYPE_OTHER_SPEED_CONFIG   0x07
#define ISE_USB_DESC_TYPE_INTERFACE_POWER      0x08

#define ISE_USB_CLASS_UNKNOWN                  0x00   // for devices, use interface class code
#define ISE_USB_CLASS_AUDIO                    0x01
#define ISE_USB_CLASS_COMMUNICATIONS           0x02
#define ISE_USB_CLASS_HID                      0x03
#define ISE_USB_CLASS_PHYSICAL                 0x05
#define ISE_USB_CLASS_IMAGE                    0x06
#define ISE_USB_CLASS_PRINTER                  0x07
#define ISE_USB_CLASS_MASS_STORAGE             0x08
#define ISE_USB_CLASS_HUB                      0x09
#define ISE_USB_CLASS_CDC_DATA                 0x0A
#define ISE_USB_CLASS_SMART_CARD               0x0B
#define ISE_USB_CLASS_CONTENT_SECURITY         0x0D
#define ISE_USB_CLASS_VIDEO                    0x0E
#define ISE_USB_CLASS_PERSONAL_HEALTH          0x0F
#define ISE_USB_CLASS_AUDIO_VIDEO              0x10
#define ISE_USB_CLASS_BILLBOARD                0x11
#define ISE_USB_CLASS_TYPE_C_BRIDGE            0x12
#define ISE_USB_CLASS_DIAGNOSTIC               0xDC
#define ISE_USB_CLASS_WIRELESS_CONTROLLER      0xE0

// HID specifics
#define ISE_USB_REQUEST_HID_GET_REPORT         0x01
#define ISE_USB_REQUEST_HID_GET_IDLE           0x02
#define ISE_USB_REQUEST_HID_GET_PROTOCOL       0x03
#define ISE_USB_REQUEST_HID_SET_REPORT         0x09
#define ISE_USB_REQUEST_HID_SET_IDLE           0x0A
#define ISE_USB_REQUEST_HID_SET_PROTOCOL       0x0B

#define ISE_USB_DESC_TYPE_HID                  0x21
#define ISE_USB_DESC_TYPE_HID_REPORT           0x22
#define ISE_USB_DESC_TYPE_HID_PHYSICAL_DESC    0x23

#define ISE_USB_HID_SUBCLASS_NONE              0x00
#define ISE_USB_HID_SUBCLASS_BOOT              0x01

// HID items - bottom 2 bits are masked off, since these are length field
#define ISE_USB_HID_ITEM_MASK                  0xFC
#define ISE_USB_HID_ITEM_TYPE_MASK             0x0C
#define ISE_USB_HID_ITEM_SIZE_MASK             0x03

// Hub
#define ISE_USB_DESC_TYPE_HUB                  0x29

#define ISE_USB_HUB_REQUEST_PORT_CONNECTION    0x00
#define ISE_USB_HUB_REQUEST_PORT_ENABLE        0x01
#define ISE_USB_HUB_REQUEST_PORT_SUSPEND       0x02
#define ISE_USB_HUB_REQUEST_PORT_OVERCURRENT   0x03
#define ISE_USB_HUB_REQUEST_PORT_RESET         0x04
#define ISE_USB_HUB_REQUEST_PORT_POWER         0x08
#define ISE_USB_HUB_REQUEST_PORT_LOW_SPEED     0x09
#define ISE_USB_HUB_REQUEST_C_PORT_CONNECTION  0x10
#define ISE_USB_HUB_REQUEST_C_PORT_ENABLE      0x11
#define ISE_USB_HUB_REQUEST_C_PORT_SUSPEND     0x12
#define ISE_USB_HUB_REQUEST_C_PORT_OVERCURRENT 0x13
#define ISE_USB_HUB_REQUEST_C_PORT_RESET       0x14
#define ISE_USB_HUB_REQUEST_PORT_TEST          0x15
#define ISE_USB_HUB_REQUEST_PORT_INDICATOR     0x16

// main items
#define ISE_USB_HID_ITEM_MAIN                  0x00
#define ISE_USB_HID_ITEM_INPUT                 0x80
#define ISE_USB_HID_ITEM_OUTPUT                0x90
#define ISE_USB_HID_ITEM_FEATURE               0xB0
#define ISE_USB_HID_ITEM_COLLECTION            0xA0
#define ISE_USB_HID_ITEM_END_COLLECTION        0xC0

// global items
#define ISE_USB_HID_ITEM_GLOBAL                0x04
#define ISE_USB_HID_ITEM_USAGE_PAGE            0x04
#define ISE_USB_HID_ITEM_LOGICAL_MIN           0x14
#define ISE_USB_HID_ITEM_LOGICAL_MAX           0x24
#define ISE_USB_HID_ITEM_PHYSICAL_MIN          0x34
#define ISE_USB_HID_ITEM_PHYSICAL_MAX          0x44
#define ISE_USB_HID_ITEM_UNIT_EXPONENT         0x54
#define ISE_USB_HID_ITEM_UNIT                  0x64
#define ISE_USB_HID_ITEM_REPORT_SIZE           0x74
#define ISE_USB_HID_ITEM_REPORT_ID             0x84
#define ISE_USB_HID_ITEM_REPORT_COUNT          0x94
#define ISE_USB_HID_ITEM_PUSH                  0xA4
#define ISE_USB_HID_ITEM_POP                   0xB4

// local items
#define ISE_USB_HID_ITEM_LOCAL                 0x08
#define ISE_USB_HID_ITEM_USAGE                 0x08
#define ISE_USB_HID_ITEM_USAGE_MIN             0x18
#define ISE_USB_HID_ITEM_USAGE_MAX             0x28
#define ISE_USB_HID_ITEM_DESIGNATOR_INDEX      0x38
#define ISE_USB_HID_ITEM_DESIGNATOR_MIN        0x48
#define ISE_USB_HID_ITEM_DESIGNATOR_MAX        0x58
#define ISE_USB_HID_ITEM_STRING_INDEX          0x78
#define ISE_USB_HID_ITEM_STRING_MIN            0x88
#define ISE_USB_HID_ITEM_STRING_MAX            0x98
#define ISE_USB_HID_ITEM_DELIMITER             0xA8

// HID collection type
#define ISE_USB_HID_COLLECTION_PHYSICAL        0x00
#define ISE_USB_HID_COLLECTION_APPLICATION     0x01
#define ISE_USB_HID_COLLECTION_LOGICAL         0x02
#define ISE_USB_HID_COLLECTION_REPORT          0x03
#define ISE_USB_HID_COLLECTION_NAMED_ARRAY     0x04
#define ISE_USB_HID_COLLECTION_USAGE_SWITCH    0x05
#define ISE_USB_HID_COLLECTION_USAGE_MODIFIER  0x06

// HID usages
// shorten the 32b usage to 16
//#define ISE_USB_HID_SHORT_USAGE(u)             ((uint16_t) ((((u) & 0xFF0000) >> 8) | ((u) & 0xFF)))

#define ISE_USB_HID_USAGE_PAGE_UNDEFINED                   0x00
#define ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP             0x01
#define ISE_USB_HID_USAGE_PAGE_SIMULATION                  0x02
#define ISE_USB_HID_USAGE_PAGE_VR                          0x03
#define ISE_USB_HID_USAGE_PAGE_SPORT                       0x04
#define ISE_USB_HID_USAGE_PAGE_GAME                        0x05
#define ISE_USB_HID_USAGE_PAGE_GENERIC_DEVICE              0x06
#define ISE_USB_HID_USAGE_PAGE_KEYBOARD                    0x07
#define ISE_USB_HID_USAGE_PAGE_LEDS                        0x08
#define ISE_USB_HID_USAGE_PAGE_BUTTONS                     0x09
#define ISE_USB_HID_USAGE_PAGE_ORDINAL                     0x0A
#define ISE_USB_HID_USAGE_PAGE_TELEPHONY                   0x0B
#define ISE_USB_HID_USAGE_PAGE_CONSUMER                    0x0C
#define ISE_USB_HID_USAGE_PAGE_DIGITIZER                   0x0D
#define ISE_USB_HID_USAGE_PAGE_PID                         0x0F
#define ISE_USB_HID_USAGE_PAGE_UNICODE                     0x10
#define ISE_USB_HID_USAGE_PAGE_CAMERA_CONTROL              0x90

// Generic desktop usage
#define ISE_USB_HID_USAGE_DESKTOP_UNDEFINED                (0x00 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_POINTER                  (0x01 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_MOUSE                    (0x02 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_JOYSTICK                 (0x04 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_GAMEPAD                  (0x05 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_KEYBOARD                 (0x06 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_KEYPAD                   (0x07 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_MULTI_AXIS               (0x08 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_TABLET_PC                (0x09 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_X                        (0x30 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_Y                        (0x31 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_Z                        (0x32 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_RX                       (0x33 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_RY                       (0x34 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_RZ                       (0x35 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_SLIDER                   (0x36 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_DIAL                     (0x37 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_WHEEL                    (0x38 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_HATSWITCH                (0x39 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_COUNTED_BUFFER           (0x3A | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_BYTE_COUNT               (0x3B | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_MOTION_WAKEUP            (0x3C | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_START                    (0x3D | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_SELECT                   (0x3E | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_VX                       (0x40 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_VY                       (0x41 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_VZ                       (0x42 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_VBRX                     (0x43 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_VBRY                     (0x44 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_VBRZ                     (0x45 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_VNO                      (0x46 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_DPAD_UP                  (0x90 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_DPAD_DOWN                (0x91 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_DPAD_RIGHT               (0x92 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))
#define ISE_USB_HID_USAGE_DESKTOP_DPAD_LEFT                (0x93 | (ISE_USB_HID_USAGE_PAGE_GENERIC_DESKTOP << 16))

// LED usgae
#define ISE_USB_HID_USAGE_LEDS_UNDEFINED                   (0x00 | (ISE_USB_HID_USAGE_PAGE_LEDS << 16))
#define ISE_USB_HID_USAGE_LEDS_NUM_LOCK                    (0x01 | (ISE_USB_HID_USAGE_PAGE_LEDS << 16))
#define ISE_USB_HID_USAGE_LEDS_CAPS_LOCK                   (0x02 | (ISE_USB_HID_USAGE_PAGE_LEDS << 16))
#define ISE_USB_HID_USAGE_LEDS_SCROLL_LOCK                 (0x03 | (ISE_USB_HID_USAGE_PAGE_LEDS << 16))
#define ISE_USB_HID_USAGE_LEDS_COMPOSE                     (0x04 | (ISE_USB_HID_USAGE_PAGE_LEDS << 16))
#define ISE_USB_HID_USAGE_LEDS_KANA                        (0x05 | (ISE_USB_HID_USAGE_PAGE_LEDS << 16))
#define ISE_USB_HID_USAGE_LEDS_POWER                       (0x06 | (ISE_USB_HID_USAGE_PAGE_LEDS << 16))
#define ISE_USB_HID_USAGE_LEDS_SHIFT                       (0x07 | (ISE_USB_HID_USAGE_PAGE_LEDS << 16))
#define ISE_USB_HID_USAGE_LEDS_DO_NOT_DISTURB              (0x08 | (ISE_USB_HID_USAGE_PAGE_LEDS << 16))
#define ISE_USB_HID_USAGE_LEDS_MUTE                        (0x09 | (ISE_USB_HID_USAGE_PAGE_LEDS << 16))

// Sony DualShock4 vendor defined usage page
#define ISE_USB_HID_USAGE_PAGE_SONY_DS4                    0xFF80

// converts a desktop usage to Sony DS4 usage
#define ISE_USB_HID_USAGE_SONY_DS4(u)                      (((u) & 0xFFFF) | (ISE_USB_HID_USAGE_PAGE_SONY_DS4 << 16))

#pragma pack(push, 1)
typedef struct {
    uint8_t length;
    uint8_t desc_type;
} ise_usb_generic_descriptor_t;

typedef struct {
    uint8_t length;
    uint8_t desc_type;
    uint16_t release_num;
    uint8_t device_class;
    uint8_t device_subclass;
    uint8_t protocol;
    uint8_t max_packet_size;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t device_rel;
    uint8_t manufacturer;
    uint8_t product;
    uint8_t serial_num;
    uint8_t configurations;
} ise_usb_device_descriptor_t;

typedef struct {
    uint8_t length;
    uint8_t desc_type;
    uint16_t total_length;
    uint8_t num_interfaces;
    uint8_t configuration_value;
    uint8_t configuration_string_id;
    uint8_t attributes;
    uint8_t max_power;
} ise_usb_configuration_descriptor_t;

typedef struct {
    uint8_t length;
    uint8_t desc_type;
    uint8_t interface_num;
    uint8_t alt_setting;
    uint8_t num_endpoints;
    uint8_t interface_class;
    uint8_t interface_subclass;
    uint8_t interface_protocol;
    uint8_t interface_string_id;
} ise_usb_interface_descriptor_t;

typedef struct {
    uint8_t length;
    uint8_t desc_type;
    uint8_t endpoint_address;
    uint8_t attributes;
    uint16_t max_packet_size;
    uint8_t interval;
} ise_usb_endpoint_descriptor_t;

typedef struct {
    uint8_t length;
    uint8_t desc_type;
    uint16_t hid_revision;
    uint8_t country_code;
    uint8_t num_descriptors;
    uint8_t class_desc_type;
    uint8_t class_desc_length0;
    uint8_t class_desc_length1;
    uint8_t class2_desc_type;
    uint8_t class2_desc_length0;
    uint8_t class2_desc_length1;
} ise_usb_hid_descriptor_t;

typedef struct {
    uint8_t length;
    uint8_t desc_type;
    uint8_t num_ports;
    uint8_t hub_chars;
    uint8_t reserved;
    uint8_t potpgt;
    uint8_t max_hub_current;
    // 1 bit per port + bit 0 (reserved) for removable
    // then repeated for power control mask
    uint8_t port_chars[32];
} ise_usb_hub_descriptor_t;

typedef union {
    ise_usb_generic_descriptor_t generic;
    ise_usb_device_descriptor_t device;
    ise_usb_configuration_descriptor_t configuration;
    ise_usb_interface_descriptor_t interface;
    ise_usb_endpoint_descriptor_t endpoint;
    ise_usb_hid_descriptor_t hid;
    ise_usb_hub_descriptor_t hub;
} ise_usb_descriptor_t;

typedef struct {
    uint8_t request_type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} ise_usb_device_request_packet_t;
#pragma pack(pop)

// These are pre-allocated queues
// All other work should be placed on these queues
// The first 8 queues are for preiodic work for the speicified interval
#define ISE_USB_QUEUE_1MS                      0
#define ISE_USB_QUEUE_2MS                      1
#define ISE_USB_QUEUE_4MS                      2
#define ISE_USB_QUEUE_8MS                      3
#define ISE_USB_QUEUE_16MS                     4
#define ISE_USB_QUEUE_32MS                     5
#define ISE_USB_QUEUE_64MS                     6
#define ISE_USB_QUEUE_128MS                    7
// Bulk transfer queue
#define ISE_USB_QUEUE_BULK                     14
// control transfers
#define ISE_USB_QUEUE_CONTROL                  15

// Configuration state machine
#define ISE_USB_CONFIG_STATE_IDLE                          0x0000
#define ISE_USB_CONFIG_STATE_START                         0x0001
#define ISE_USB_CONFIG_STATE_GET_DESC_LENGTH               0x0002
#define ISE_USB_CONFIG_STATE_SET_ADDRESS                   0x0003
#define ISE_USB_CONFIG_STATE_SET_ADDRESS_ACK               0x0004
#define ISE_USB_CONFIG_STATE_GET_DESC                      0x0005
#define ISE_USB_CONFIG_STATE_GET_LANG                      0x0006
#define ISE_USB_CONFIG_STATE_GET_MANUFACTURER              0x0007
#define ISE_USB_CONFIG_STATE_GET_PRODUCT                   0x0008
#define ISE_USB_CONFIG_STATE_GET_CONFIG_DESC               0x0009
#define ISE_USB_CONFIG_STATE_SET_CONFIGURATION             0x000A
// HID states
#define ISE_USB_CONFIG_STATE_HID_SET_IDLE                  0x0100
#define ISE_USB_CONFIG_STATE_HID_GET_REPORT                0x0101
#define ISE_USB_CONFIG_STATE_HID_SET_PROTOCOL              0x0102
#define ISE_USB_CONFIG_STATE_HID_START                     0x0103
// Hub states
#define ISE_USB_CONFIG_STATE_HUB_GET_DESCRIPTOR            0x0200
#define ISE_USB_CONFIG_STATE_HUB_POWER_PORTS               0x0201
#define ISE_USB_CONFIG_STATE_HUB_WAIT_FOR_POTPGT           0x0202
#define ISE_USB_CONFIG_STATE_HUB_CLEAR_C_PORT_CONNECTION   0x0203
#define ISE_USB_CONFIG_STATE_HUB_GET_PORT_STATUS           0x0204
#define ISE_USB_CONFIG_STATE_HUB_START                     0x0205
// End states
#define ISE_USB_CONFIG_STATE_COMPLETE                      0xFFFF

// USB languages ids
#define ISE_USB_LANGID_ENGLISH                 0x09   // bottom byte
#define ISE_USB_LANGID_ENGLISH_US              0x0409

// Maximum number of host controller in the system
#define ISE_USB_MAX_HC                         4

// Size of configuration buffer in bytes
#define ISE_USB_CONFIG_BUFFER_SIZE             512

// maximum number of setup TDs
#define ISE_USB_MAX_CONFIG_TDS                 ((ISE_USB_CONFIG_BUFFER_SIZE / 8) + 4)

// maximum devices per host controller (including the setup device 0)
#define ISE_USB_MAX_DEVICES                    128

// maximum interfaces per device - imposed by implementation, not USB!
#define ISE_USB_MAX_INTERFACES                 4

// maximum endpointes per interface
#define ISE_USB_MAX_ENDPOINTS                  16

// number of HID controls, and what each mean
#define ISE_USB_HID_MAX_CONTROLS               16

// number of HID interfaces in system
#define ISE_USB_HID_MAX_INTERFACES             16

// max ports per hub
#define ISE_USB_HUB_MAX_PORTS                  16

// max hubs in a system - must be greater than or equal to ISE_USB_MAX_HC
#define ISE_USB_MAX_HUBS                       16

typedef struct {
    // bottom 2 bits are type (0=control, 1=iso, 2=bulk, 3=interrupt)
    // top bit is direction (0=out, 1=in)
    uint8_t attributes;
    uint8_t interval;
    uint16_t max_packet_size;
} ise_usb_endpoint_t;

typedef struct {
    uint8_t class_code;
    uint8_t subclass_code;
    uint8_t protocol;
    uint8_t host_id;
} ise_usb_interface_t;

typedef struct {
    uint32_t control_type;
    uint16_t shift;
    uint8_t  length;
    uint8_t  flags;   // bit 7 for sign, others reserved
} ise_usb_hid_control_t;


typedef struct {
    uint8_t class_code;
    uint8_t device_id;
    uint8_t parent_device_id; // for root hub, the parent of device being configured
    uint8_t port;             // port device attached number; for root hub, port of device being configured
    uint8_t configurations;
    uint8_t max_packet_size;
    uint8_t speed;            // for root hub, speed of device being configured
    uint8_t num_interfaces;
    ise_usb_queue_t* device_ctrl_q;
    char* manufacturer;
    char* product;
    ise_usb_interface_t* interface[ISE_USB_MAX_INTERFACES];
    ise_usb_endpoint_t endpoint[ISE_USB_MAX_ENDPOINTS];
} ise_usb_device_t;

// forward declaration
struct ise_usb_hc_s;
typedef struct ise_usb_hc_s ise_usb_hc_t;

struct ise_usb_hc_s {
    // PCI location
    int pci_slot;
    int pci_func;

    int host_id;

    // interrupt number
    uint32_t interrupt_num;
    
    // device structures
    ise_usb_device_t* device[ISE_USB_MAX_DEVICES];

    // structures to setup/configure device
    int config_state;   // indicates what step inconfiguration we're in
    int config_device_id;    // device id being configured
    int num_config_packets;  // number of config TDs sent for prior config state
    uint16_t config_lang_id;
    uint8_t config_manufacturer_string_id;
    uint8_t config_product_string_id;
    uint8_t config_current_interface;
    uint32_t config_time;
    ise_usb_device_request_packet_t* config_req_packet;
    uint8_t* config_buffer;
    ise_usb_td_t* config_td[ISE_USB_MAX_CONFIG_TDS];

    // host controller specific data
    ise_usb_hcd_t* hcd;

    // function pointers
    void* (*alloc_mem)(ise_usb_hcd_t* _hcd, int size);
    void (*free_mem)(ise_usb_hcd_t* _hcd, void* entry, int size);
    ise_usb_queue_t* (*alloc_queue)(ise_usb_hcd_t* _hcd);
    void (*free_queue)(ise_usb_hcd_t* _hcd, ise_usb_queue_t* _q);
    ise_usb_td_t* (*alloc_td)(ise_usb_hcd_t* _hcd);
    void (*free_td)(ise_usb_hcd_t* _hcd, ise_usb_td_t* _td);
    ise_usb_queue_t* (*get_queue)(ise_usb_hcd_t* _hcd, int q_id);
    void (*fill_qh)(ise_usb_queue_t* _q, ise_usb_hc_t* ise_usb, uint8_t device_id, uint8_t end_pt);
    void (*fill_td)(ise_usb_td_t* _td, ise_usb_td_info_t* td_info);
    void (*push_queue)(ise_usb_queue_t* _q0, ise_usb_queue_t* _q1);
    void (*enqueue_queue)(ise_usb_queue_t* _q0, ise_usb_queue_t* _q1);
    bool (*dequeue_queue)(ise_usb_queue_t* _q0, ise_usb_queue_t* _q1);
    void (*push_td)(ise_usb_queue_t* _q0, ise_usb_td_t* _td);
    void (*enqueue_td)(ise_usb_queue_t* _q0, ise_usb_td_t* _td);
    bool (*is_td_done)(ise_usb_td_t* _td);
    bool (*is_td_fatal)(ise_usb_td_t* _td);
    bool (*is_td_stalled)(ise_usb_td_t* _td);
    bool (*is_td_short)(ise_usb_td_t* _td);
    void (*activate_td)(ise_usb_td_t* _td);
    void (*deactivate_td)(ise_usb_td_t* _td);
    void (*clear_td_status)(ise_usb_td_t* _td);
    int  (*restart_td)(ise_usb_td_t* _td, uint32_t addr, int len);
    void (*dequeue_completed_tds)(ise_usb_hcd_t* _hcd, ise_usb_queue_t* _q0, bool free);
    bool (*dequeue_td)(ise_usb_queue_t* _q0, ise_usb_td_t* _td);
    void (*restart_queue)(ise_usb_hcd_t* _hcd, ise_usb_queue_t* _q);
    void (*start_queue)(ise_usb_hcd_t* _hcd, ise_usb_queue_t* _q);
    void (*stop_queue)(ise_usb_queue_t* _q);
    void (*clear_queue)(ise_usb_queue_t* _q);
    bool (*is_queue_done)(ise_usb_queue_t* _q);
    bool (*is_queue_fatal)(ise_usb_queue_t* _q);

};

typedef struct {
    ise_usb_interface_t intf;
    ise_usb_queue_t* interrupt_queue;
    ise_usb_queue_t* data_queue;
    void* data;
    ise_usb_td_t* in_td;
    ise_usb_device_t* device;
    uint32_t hid_type;
    uint8_t hid_num_controls;
    uint8_t in_endpoint;
    uint8_t out_endpoint;
    uint8_t nak_count;
    ise_usb_hid_control_t hid_control[ISE_USB_HID_MAX_CONTROLS];
} ise_usb_hid_interface_t;

typedef struct {
    ise_usb_device_t device;
    uint8_t num_ports;
    uint8_t port_device_id[ISE_USB_HUB_MAX_PORTS];
    int  (*port_changed)(ise_usb_hc_t* ise_usb, int device_id, int port);
    int  (*reset_port)  (ise_usb_hc_t* ise_usb, int device_id, int port);
    void (*disable_port)(ise_usb_hc_t* ise_usb, int device_id, int port);
} ise_usb_hub_t;

typedef struct {
    ise_usb_hub_t hub;
    uint8_t in_endpoint;
    uint8_t potpgt;
    uint8_t port_status[ISE_USB_HUB_MAX_PORTS];
    ise_usb_queue_t* interrupt_queue;
    ise_usb_queue_t* data_queue;
    ise_usb_td_t* in_td;
    ise_usb_td_t* ctrl_td[2];
    uint32_t* hub_data;
} ise_usb_external_hub_t;


extern int ise_usb_num_hosts;
extern ise_usb_hc_t ise_usb[ISE_USB_MAX_HC];

extern int ise_usb_num_hid_interfaces;
extern ise_usb_hid_interface_t* ise_usb_hid_interface[ISE_USB_HID_MAX_INTERFACES];

uint32_t ise_usb_hid_get_buttons(ise_usb_hid_interface_t* hid_interface);
int32_t ise_usb_hid_get_control(ise_usb_hid_interface_t* hid_interface, uint32_t control);
uint64_t ise_usb_hid_get_keys(ise_usb_hid_interface_t* hid_interface);

#define ISE_USB_TIME_DIVIDER_LOG2              3
#define ISE_USB_TIME_DIVIDER                   (1 << ISE_USB_TIME_DIVIDER_LOG2)
#define ISE_USB_TIME_MASK                      (ISE_USB_TIME_DIVIDER - 1)

// Hub processing needs to be called at least evey 255 msb
// We can do around every 220 ms, by calling it 4 times less often than the standard timer interrupt
#define ISE_USB_HUB_TIME_DIVIDER_LOG2          (ISE_USB_TIME_DIVIDER_LOG2+4)
#define ISE_USB_HUB_TIME_DIVIDER               (1 << ISE_USB_HUB_TIME_DIVIDER_LOG2)
#define ISE_USB_HUB_TIME_MASK                  (ISE_USB_HUB_TIME_DIVIDER - 1)

void _interrupt FAR ise_usb_isr();

// detects and install any usb host controller devices
// pci bus must be pre-scanned
void ise_usb_install();
void ise_usb_uninstall();

// periodically called to check usb port status
void ise_usb_tick();

#include "ise_uhci.h"
#include "ise_ohci.h"
#include "ise_ehci.h"

#endif  //  #ifndef __ISE_USB_H
