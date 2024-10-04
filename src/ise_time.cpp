// InfiniScroll Engine
// ise_time.cpp
// Programmable interval timer routines
//
// Kamal Pillai
// 2/5/2019

#include "ise.h"

/*
uint32_t ise_time_tick = 0;
void _interrupt (FAR *prev_timer_isr)() = NULL;

void _interrupt FAR ise_timer_isr()
{
    ise_time_tick++;
    if(prev_timer_isr) prev_timer_isr();
}
*/

int ise_time_divider_log2 = 0;
volatile uint32_t FAR* ise_time_tick = ISE_TIME_BIOS_TICK;

void ise_time_set_count()
{
    int count = 0x100 >> ise_time_divider_log2;
    outp(ISE_TIME_PORT_CH0, count & 0xFF);
}

void ise_time_install()
{
    outp(ISE_TIME_PORT_MODE, 0x26);  //  read high byte on channel 0
    ise_time_set_count();
    //prev_timer_isr = _dos_getvect(ISE_TIME_IRQ);
    //_dos_setvect(ISE_TIME_IRQ, ise_timer_isr);
}

void ise_time_uninstall()
{
    //if(prev_timer_isr) _dos_setvect(ISE_TIME_IRQ, prev_timer_isr);
    //prev_timer_isr = NULL;
}

void ise_time_set_divider(int divider_log2, volatile uint32_t FAR* tick)
{
    ise_time_divider_log2 = divider_log2;
    ise_time_tick = (divider_log2 == 0 || tick == NULL) ? ISE_TIME_BIOS_TICK : tick;
    ise_time_set_count();
}


uint32_t ise_time_get_time()
{
    uint32_t timer;
    uint32_t ch0_time_hi;//, ch0_time_lo;
    do {
        timer = *ise_time_tick;
        ch0_time_hi = inp(ISE_TIME_PORT_CH0);
        //ch0_time_lo = inp(ISE_TIME_PORT_CH0);
    } while(timer != *ise_time_tick);  // if timer incremented while reading 8254

    if(ise_time_divider_log2 >= 8) {
        timer = timer >> (ise_time_divider_log2 - 8);
    } else {
        // this is a count down time, so negate, and mask out bottom 8 bits
        ch0_time_hi = (-ch0_time_hi) & (0xFF >> ise_time_divider_log2);

        timer = (timer << (8-ise_time_divider_log2)) | ch0_time_hi;
    }

    return timer;
}

uint32_t ise_time_get_delta(uint32_t* last_time_ptr)
{
    uint32_t last_time = *last_time_ptr;
    uint32_t this_time = ise_time_get_time();
    uint32_t delta_time;
    if(this_time < last_time) {
        //if((this_time & 0xFFFFFF00) == (last_time & 0xFFFFFF00)) {
        //    // this happens if the tick count didn't get updated when we read the 8254
        //    delta_time = (0x100 | this_time & 0xff) - (last_time & 0xff);
        //} else {
        //    // otherwise, we hit the wrap-around case
        //    delta_time = ISE_TIME_TICKS_PER_DAY - last_time + this_time;
        //}
		delta_time = 0;
    } else {
        delta_time = this_time - last_time;
		*last_time_ptr = this_time;
    }
    return (3*delta_time)/14;
}

void ise_time_uwait(uint32_t wait_time)
{
    wait_time = (wait_time + 214) / 215;
    //wait_time += wait_time / 5;  // convert us to ticks by multiplying by 1.2
    uint32_t start_time = ise_time_get_time();
    uint32_t this_time = start_time;
    while((this_time < start_time) || (this_time - start_time < wait_time)) {
        this_time = ise_time_get_time();
    }
}

void ise_time_wait(uint32_t wait_time)
{
    //wait_time *= 1193;  // convert ms to ticks
    wait_time *= 14;
    wait_time /= 3;
    uint32_t start_time = ise_time_get_time();
    uint32_t this_time = start_time;
    while((this_time < start_time) || (this_time - start_time < wait_time)) {
        ise_dsp_tick();
        ise_opl_tick();
        this_time = ise_time_get_time();
    }
}
