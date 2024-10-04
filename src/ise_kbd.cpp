// InfiniScroll Engine
// ise_kbd.cpp
// Keyboard handler routines
//
// Kamal Pillai
// 1/24/2019

#include "ise.h"

static ise_keybd_t* keybd = NULL;

void _interrupt FAR ise_keybd_isr()
{
    static uint8_t extended_key = 0x0;

    if(keybd) {
        uint8_t scancode = (uint8_t) inp(ISE_KEYBD_DATA_PORT);
        if(scancode == ISE_KEYBD_EXTENDED_KEY_SCANCODE) {
            extended_key = 0x80;
        } else {
            uint8_t key_index = (scancode & 0x7f) | extended_key;
            keybd->key[key_index] = (scancode & 0x80) ? 0 : 1;
			extended_key = 0x0;
        }
        if(keybd->enable_default_isr) {
            _chain_intr(keybd->prev_keybd_isr);
        } else {
            uint8_t status = (uint8_t) inp(ISE_KEYBD_STATUS_PORT);
            outp(ISE_KEYBD_STATUS_PORT, status | 0x80);
            outp(ISE_KEYBD_STATUS_PORT, status);
            outp(ISE_IRQ_PIC0_COMMAND, 0x20);
        }
    }
}

void ise_keybd_install(ise_keybd_t *kb)
{
    if(keybd) return; // already installed!!!

    keybd = kb;
    int i;
    for(i=0; i<256; i++) {
        kb->key[i] = 0;
    }
    kb->prev_keybd_isr = _dos_getvect(ISE_KEYBD_INTERRUPT);
    _dos_setvect(ISE_KEYBD_INTERRUPT, ise_keybd_isr);
}

void ise_keybd_uninstall()
{
    if(keybd) {
        _dos_setvect(ISE_KEYBD_INTERRUPT, keybd->prev_keybd_isr);
        keybd = NULL;
    }
}
