// InfiniScroll Engine
// ise_kbd.h
// Keyboard handler routines
//
// Kamal Pillai
// 1/24/2019

#ifndef __ISE_KBD_H
#define __ISE_KBD_H

#define ISE_KEYBD_INTERRUPT 0x9
#define ISE_KEYBD_DATA_PORT 0x60
#define ISE_KEYBD_STATUS_PORT 0x61
#define ISE_KEYBD_EXTENDED_KEY_SCANCODE 0xe0

void _interrupt FAR ise_keybd_isr();

typedef struct {
    void _interrupt (FAR *prev_keybd_isr)();
    volatile uint8_t key[256];
    bool enable_default_isr;
} ise_keybd_t;

void ise_keybd_install(ise_keybd_t *kb);
void ise_keybd_uninstall();

#endif  // #ifndef __ISE_KBD_H


