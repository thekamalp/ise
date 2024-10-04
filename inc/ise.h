// InfiniScroll Engine
// ise.h
// Main header file for engine
//
// Kamal Pillai
// 1/13//2019

#ifndef __ISE_H
#define __ISE_H

#include <dos.h>
#include <bios.h>
#include <conio.h>
#include <i86.h>
#include <signal.h>
#include <stdint.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(__386__) && defined(__DOS__)
    #define INT86(a, b, c) int386(a, b, c)
    #define INT86X(a, b, c, d) int386x(a, b, c, d)
    #define FAR
    #define DOS32
#else
    #define INT86(a, b, c) int86(a, b, c)
    #define INT86X(a, b, c, d) int86x(a, b, c, d)
    #define FAR far
    #define DOS16
#endif

#include "ise_math.h"

#ifdef DOS32
    #include "ise_mem.h"
    #include "ise_pci.h"
    #include "ise_usb.h"
#endif

#include "ise_opl.h"
#include "ise_dma.h"
#include "ise_dsp.h"
#include "ise_irq.h"
#include "ise_time.h"
#include "ise_gfx.h"
#include "ise_kbd.h"

#endif // #ifndef __ISE_H
