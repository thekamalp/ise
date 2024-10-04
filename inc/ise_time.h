// InfiniScroll Engine
// ise_time.h
// Programmable interval timer routines
//
// Kamal Pillai
// 2/5/2019

#ifndef __ISE_TIME_H
#define __ISE_TIME_H

#define ISE_TIME_PORT_CH0                      0x40
#define ISE_TIME_PORT_CH1                      0x41
#define ISE_TIME_PORT_CH2                      0x42
#define ISE_TIME_PORT_MODE                     0x43

#define ISE_TIME_IRQ                           0x1C
#define ISE_TIME_BIOS_TICK                     ((volatile uint32_t FAR *) 0x46C)

#define ISE_TIME_TICKS_PER_DAY                 1573040L


//void _interrupt FAR ise_timer_isr();

void ise_time_install();
void ise_time_uninstall();

void ise_time_set_divider(int divider_log2, volatile uint32_t FAR* tick);

uint32_t ise_time_get_time();
uint32_t ise_time_get_delta(uint32_t* last_time_ptr);
void ise_time_uwait(uint32_t wait_time);
void ise_time_wait(uint32_t wait_time);

#endif  //  #ifndef __ISE_TIME_H

