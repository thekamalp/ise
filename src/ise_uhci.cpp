// InfiniScroll Engine
// ise_uhci.h
// Universal Host Controller Interface
//
// Kamal Pillai
// 2/4/2019

#include "ise.h"

// --------------------------------------------------------------------------------------------
// USB driver calls

// allocates size in bytes of 16B entries from memory pool
void* ise_uhci_alloc_mem(ise_usb_hcd_t* _hcd, int size)
{
    int num_entries = (size + 15) / 16; // divide by 16, but round up

    // only support up to 32 entries (512 bytes)
    if(num_entries <= 0 || num_entries > 32) return NULL;

    ise_uhci_t* ise_uhci = (ise_uhci_t*) _hcd;
    //uint32_t size_mask = (num_entries==32) ? ~0L : (1L << num_entries) - 1;
    void* entry = NULL;
    int offset;
    int p;
    //int i, j;
    // Loop through each pool of memory for an empty slot
    for(p=0; p<ISE_UHCI_MAX_MEM_POOL && entry == NULL; p++) {
        // if we need to use this pool, and it hasn't been allocated, do so now
        if(ise_uhci->mem_pool[p] == NULL) {
            // allocate 8KB, 16B aligned
            ise_uhci->mem_pool[p] = (uint32_t*) ise_mem_aligned_malloc(0x2000, 4, ISE_MEM_REGION_DOS);
            // if we couldn't allocate the pool, fail
            if(ise_uhci->mem_pool[p] == NULL) return NULL;
        }
        offset = ise_mem_alloc_entries(num_entries, ise_uhci->mem_alloc[p], 16);
        if(offset >= 0) {
            // scale offset to account for 4 DWORDS per entry
            offset = offset << 2;
            entry = ise_uhci->mem_pool[p] + offset;
        }
    }
    return entry;
}

// frees size in bytes of 16B entries back to memory pool
void ise_uhci_free_mem(ise_usb_hcd_t* _hcd, void* entry, int size)
{
    int num_entries = (size + 15) / 16; // divide by 16, but round up
    // only support up to 32 entries (512 Bytes)
    if(num_entries <= 0 || num_entries > 32) return;

    ise_uhci_t* ise_uhci = (ise_uhci_t*) _hcd;
    //uint32_t size_mask = (num_entries==32) ? ~0L : (1L << num_entries) - 1;
    uint32_t entry_addr = (uint32_t) entry;
    uint32_t mem_pool_addr;
    uint32_t offset;
    bool done = false;
    int p;
    // Loop through each memory to pool to find which pool the address belongs to
    for(p=0; p<ISE_UHCI_MAX_MEM_POOL && !done; p++) {
        mem_pool_addr = (uint32_t) ise_uhci->mem_pool[p];
        // if entry address is within 8KB of the memory pool address,
        // then it belongs to the bool
        // note, offset goes negative, it is treated as a very large positive value
        offset = entry_addr - mem_pool_addr;
        if(offset < 0x2000) {
            // scale offset for 16 Bytes per entry
            offset = offset >> 4;
            done = ise_mem_free_entries(offset, num_entries, ise_uhci->mem_alloc[p], 16);
        }
    }
}

// initialize a queue
void ise_uhci_init_queue(ise_uhci_queue_t* q)
{
    // mark all pointers as invalid
    q->head_link = 0x1;
    q->current_element_link = 0x1;
    q->start_element_link = 0x1;
    q->end_element_link = 0x1;
}

// initialize a transfer descriptor
void ise_uhci_init_td(ise_uhci_td_t* td)
{
    // mark next pointer as invalid
    td->link = 0x1;
    // everything else 0 out
    td->command = 0x0;
    td->device = 0x0;
    td->buffer = 0x0;
}

// sets next pointer of q0 to point to q1
void ise_uhci_link_queues(ise_uhci_queue_t* q0, ise_uhci_queue_t* q1)
{
    // set bit 1 to indicate a queue
    q0->head_link = ((uint32_t) q1) | 0x2;
}

// allocate a queue
ise_usb_queue_t* ise_uhci_alloc_queue(ise_usb_hcd_t* _hcd)
{
    ise_uhci_queue_t* q = (ise_uhci_queue_t*) ise_uhci_alloc_mem(_hcd, 16);
    ise_uhci_init_queue(q);
    return (ise_usb_queue_t*) q;
}

// free a queue
void ise_uhci_free_queue(ise_usb_hcd_t* _hcd, ise_usb_queue_t* _q)
{
    uint32_t* q = (uint32_t*) _q;
    ise_uhci_free_mem(_hcd, q, 16);
}

// allocate a transfer descriptor
ise_usb_td_t* ise_uhci_alloc_td(ise_usb_hcd_t* _hcd)
{
    ise_uhci_td_t* td = (ise_uhci_td_t*) ise_uhci_alloc_mem(_hcd, 16);
    ise_uhci_init_td(td);
    return (ise_usb_td_t*) td;
}

// free a transfer descriptor
void ise_uhci_free_td(ise_usb_hcd_t* _hcd, ise_usb_td_t* _td)
{
    uint32_t* td = (uint32_t*) _td;
    ise_uhci_free_mem(_hcd, td, 16);
}

// get a primary queue
ise_usb_queue_t* ise_uhci_get_queue(ise_usb_hcd_t* _hcd, int q_id)
{
    ise_uhci_t* ise_uhci = (ise_uhci_t*) _hcd;
    ise_usb_queue_t* q = NULL;
    if(q_id < 8) {
        int index = (q_id == 0) ? 0 : (1 << (q_id-1));
        uint32_t q_ptr = ise_uhci->frame_list[index];
        q = (ise_usb_queue_t*) (q_ptr & ~0xFL);
    } else {
        switch(q_id) {
            case ISE_USB_QUEUE_BULK:
                q = (ise_usb_queue_t*) ise_uhci->bulk_queue;
                break;
            case ISE_USB_QUEUE_CONTROL:
                q = (ise_usb_queue_t*) ise_uhci->control_queue;
                break;
        }
    }

    return q;
}

// queues in uhci don't have this info, so nothing to do
void ise_uhci_fill_qh(ise_usb_queue_t* /*_q*/, ise_usb_hc_t* /*ise_usb*/, uint8_t /*device_id*/, uint8_t /*end_pt*/)
{ }

// fills out an already allocated transfer descriptor
void ise_uhci_fill_td(ise_usb_td_t* _td, ise_usb_td_info_t* td_info)
{
    ise_uhci_td_t* td = (ise_uhci_td_t*) _td;

    td->buffer = ise_mem_linear_to_pa((uint32_t) td_info->buffer);             // Buffer pointer

    td->device = 0;
    td->device |= ((td_info->length - 1) & 0x7FF) << 21;                       // Maximum length
    td->device |= (td_info->dt) ? 0x00080000 : 0x0;                            // Data toggle
    td->device |= (td_info->end_pt & 0xF) << 15;                               // End point
    td->device |= (td_info->device & 0xFF) << 8;                               // Device address
    td->device |= (td_info->pid & 0xFF);                                       // Packet ID

    td->command = 0;
    td->command |= (td_info->spd) ? 0x20000000 : 0x0;                          // Short packet detect
    td->command |= 0x18000000;                                                 // Error count
    td->command |= (td_info->speed == ISE_USB_SPEED_LOW) ? 0x04000000 : 0x0;   // Low speed
    td->command |= (td_info->ioc) ? 0x01000000 : 0x0;                          // Interrupt on completion
    td->command |= 0x00800000;                                                 // active bit - should be written last, in case td already in queue
}

// adds q1 to head of the q0
void ise_uhci_push_queue(ise_usb_queue_t* _q0, ise_usb_queue_t* _q1)
{
    ise_uhci_queue_t* q0 = (ise_uhci_queue_t*) _q0;
    ise_uhci_queue_t* q1 = (ise_uhci_queue_t*) _q1;

    // set the next pointer of q1 to the current head of q0
    q1->head_link = q0->start_element_link & ~0xCL;
    // set the driver's private head of q0 to point to q1
    // set bit 1 to indicate a queue
    q0->start_element_link = ((uint32_t) q1) | 0x2;
    // if end is NULL, which means the queue was empty
    // point the end to start
    if(q0->end_element_link & 0x1) q0->end_element_link = q0->start_element_link;
}

// adds q1 to tail of the q0
void ise_uhci_enqueue_queue(ise_usb_queue_t* _q0, ise_usb_queue_t* _q1)
{
    ise_uhci_queue_t* q0 = (ise_uhci_queue_t*) _q0;
    ise_uhci_queue_t* q1 = (ise_uhci_queue_t*) _q1;

    // adding to tail, so set q1's next to NULL
    q1->head_link = 0x1;
    // if queue is currently empty
    if(q0->start_element_link & 0x1) {
        // set start and end to q1
        q0->start_element_link = ((uint32_t) q1) | 0x2;
        q0->end_element_link = q0->start_element_link;
    } else {
        // if not empty, set the current end's link to q1
        // first check if the current end is a queue
        if(q0->end_element_link & 0x2) {
            ise_uhci_queue_t* end_queue = (ise_uhci_queue_t*) (q0->end_element_link & ~0xFL);
            end_queue->head_link = ((uint32_t) q1) | 0x2;
        } else {
            ise_uhci_td_t* end_td = (ise_uhci_td_t*) (q0->end_element_link & ~0xFL);
            end_td->link = ((uint32_t) q1) | 0x2;
        }
        // then set end to q1
        q0->end_element_link = ((uint32_t) q1) | 0x2;
    }
}

// finds and removes q1 from q0
// returns true if removed
bool ise_uhci_dequeue_queue(ise_usb_queue_t* _q0, ise_usb_queue_t* _q1)
{
    ise_uhci_queue_t* q0 = (ise_uhci_queue_t*) _q0;
    ise_uhci_queue_t* q1 = (ise_uhci_queue_t*) _q1;
    uint32_t q1_val = (uint32_t) _q1;

    uint32_t prev_ptr = 0x1;
    uint32_t* next_ptr = &(q0->start_element_link);
    bool removed = false;
    bool done = (*next_ptr & 0x1) ? true : false;
    while(!done) {
        if(*next_ptr & 0x2) {
            // this is a queue
            if((*next_ptr & ~0xFL) == (q1_val & ~0xFL)) {
                // we found the queue
                *next_ptr = q1->head_link;
                // if the current points to the one we're removing, advance it
                if((q0->current_element_link & ~0xFL) == (q1_val & ~0xFL)) {
                    q0->current_element_link = q1->head_link;
                }
                // if the end points to the one we're removing, point it to the previous
                if((q0->end_element_link & ~0xFL) == (q1_val & ~0xFL)) {
                    q0->end_element_link = prev_ptr;
                }
                removed = true;
                done = true;
            } else {
                // not found, so move on
                prev_ptr = *next_ptr;
                next_ptr = (uint32_t*) (*next_ptr & ~0xFL);
            }
        } else {
            // this is a td, move on
            prev_ptr = *next_ptr;
            next_ptr = (uint32_t*) (*next_ptr & ~0xFL);
        }
        done |= (*next_ptr & 0x1) ? true : false;
    }
    // if start is NULL, point end to NULL
    if(q0->start_element_link & 0x1) q0->end_element_link = 0x1;
    return removed;
}

// adds td to head of q0
void ise_uhci_push_td(ise_usb_queue_t* _q0, ise_usb_td_t* _td)
{
    ise_uhci_queue_t* q0 = (ise_uhci_queue_t*) _q0;
    ise_uhci_td_t* td = (ise_uhci_td_t*) _td;

    // set the next pointer of td to the current head of q0
    // maintain bit 2 of td link, which indicates breadth or depth first
    td->link = q0->start_element_link | (td->link & 0x4);
    // set the driver's private head pointer to point to td
    q0->start_element_link = (uint32_t) td;
    // if end is NULL, which means the queue was empty
    // point the end to start
    if(q0->end_element_link & 0x1) q0->end_element_link = q0->start_element_link;
}

// adds td to tail of q0
void ise_uhci_enqueue_td(ise_usb_queue_t* _q0, ise_usb_td_t* _td)
{
    ise_uhci_queue_t* q0 = (ise_uhci_queue_t*) _q0;
    ise_uhci_td_t* td = (ise_uhci_td_t*) _td;

    // adding to tail, so set td's next to NULL
    td->link = 0x1;
    // if queue is currently empty
    if(q0->start_element_link & 0x1) {
        // set start and end to td
        q0->start_element_link = ((uint32_t) td);
        q0->end_element_link = q0->start_element_link;
    } else {
        // if not empty, set the current end's link to td
        // first check if the current end is a queue
        if(q0->end_element_link & 0x2) {
            ise_uhci_queue_t* end_queue = (ise_uhci_queue_t*) (q0->end_element_link & ~0xFL);
            end_queue->head_link = ((uint32_t) td);
        } else {
            ise_uhci_td_t* end_td = (ise_uhci_td_t*) (q0->end_element_link & ~0xFL);
            end_td->link = ((uint32_t) td);
        }
        // then set end to td
        q0->end_element_link = ((uint32_t) td);
    }
}

// check if a TD is completed
bool ise_uhci_is_td_done(ise_usb_td_t* _td)
{
    ise_uhci_td_t* td = (ise_uhci_td_t*) _td;
    
    // bit 23 is the active bit
    // if it's clear, the host controller completed this TD
    return (td->command & 0x800000) ? false : true;
}

// check if a TD is nak'd
/*bool ise_uhci_is_td_nakd(ise_usb_td_t* _td)
{
    ise_uhci_td_t* td = (ise_uhci_td_t*) _td;
    
    // bit 18 is the nakd bit
    // none of the other status (except active) can be set, otherwise, this is a fatal error
    return ((td->command & 0x7E0000) == 0x80000) ? true : false;
}*/

// check if td has fatal error
bool ise_uhci_is_td_fatal(ise_usb_td_t* _td)
{
    ise_uhci_td_t* td = (ise_uhci_td_t*) _td;
    
    // bit 18 is the nakd bit
    // none of the other status (except active) can be set, otherwise, this is a fatal error
    return (td->command & 0x760000) ? true : false;
}

// check if stalled bit is set
bool ise_uhci_is_td_stalled(ise_usb_td_t* _td)
{
    ise_uhci_td_t* td = (ise_uhci_td_t*) _td;
    
    // bit 22 is the stalled bit
    return ((td->command & 0x7E0000) == 0x400000) ? true : false;
}

// check if the TD is short
bool ise_uhci_is_td_short(ise_usb_td_t* _td)
{
    ise_uhci_td_t* td = (ise_uhci_td_t*) _td;
    // is the td inactive, with no bad status
    if((td->command & 0xFE0000) == 0x0) {
        int act_len = (td->command + 1) & 0x7ff;
        int max_len = ((td->device >> 21) + 1) & 0x7ff;
        return (act_len < max_len) ? true : false;
    }
    return false;
}

// activate TD
void ise_uhci_activate_td(ise_usb_td_t* _td)
{
    ise_uhci_td_t* td = (ise_uhci_td_t*) _td;

    // bit 23 is the active bit
    td->command = (td->command & 0xFF000000L) | 0x800000L;
}

// deactivate TD
void ise_uhci_deactivate_td(ise_usb_td_t* _td)
{
    ise_uhci_td_t* td = (ise_uhci_td_t*) _td;

    // bit 23 is the active bit
    td->command &= ~0x800000L;
}

// clear any status bits in TD
void ise_uhci_clear_td_status(ise_usb_td_t* _td)
{
    ise_uhci_td_t* td = (ise_uhci_td_t*) _td;

    // bit 22:17 are status bits
    td->command &= 0xFF81FFFFL;
}

// restart TD, if it completed without errors
// returns positive value if restart, otherwise, an error
// 0 means data received, 1 means a NAK
int ise_uhci_restart_td(ise_usb_td_t* _td, uint32_t addr, int len)
{
    ise_uhci_td_t* td = (ise_uhci_td_t*) _td;

    // first check if all the status bits (except the nak bit) are clear
    bool inactive = (td->command & 0xF60000) == 0x0;
    // or nak is set and all other status bits (except active) are clear
    bool nak = (td->command & 0x7E0000) == 0x080000;
    if(inactive || nak) {
        // toggle the data toggle bit, if not a nack
        if((td->command & 0x80000) == 0x0) td->device ^= 0x80000;
        // set address
        td->buffer = ise_mem_linear_to_pa(addr);
        // clear max packet length, and reload with new length
        td->device &= 0x1FFFFF;
        td->device |= ((len & 0x7FF) << 21);
        // clear the status bits
        td->command &= 0xFF000000;
		// NULL buffer - return error
		if(td->buffer == 0x0) return -1;
        // activate td
        td->command |= 0x800000;
        return (nak) ? 1 : 0;
    } else {
        return -1;
    }
}

// remove completed td's in the queue, and optionally frees them
// does not remove sub-queues, nor the td's in those sub-queues
void ise_uhci_dequeue_completed_tds(ise_usb_hcd_t* _hcd, ise_usb_queue_t* _q0, bool free)
{
    ise_uhci_t* ise_uhci = (ise_uhci_t*) _hcd;
    ise_uhci_queue_t* q0 = (ise_uhci_queue_t*) _q0;
    uint32_t* last_queue_link = &(q0->start_element_link);
    uint32_t next_ptr = q0->start_element_link;
    ise_uhci_queue_t* q1;
    ise_uhci_td_t* td;
    bool done = (next_ptr & 1) ? true : false;
    while(!done) {
        if(next_ptr & 0x2) {
            *last_queue_link = next_ptr;
            // this is a queue
            q1 = (ise_uhci_queue_t*) (next_ptr & ~0xFL);
            // if the current head is valid, then we're done
            if((q1->current_element_link & 0x1) == 0) done = true;
            last_queue_link = &(q1->head_link);
            next_ptr = q1->head_link;
        } else {
            // this is td
            td = (ise_uhci_td_t*) (next_ptr & ~0xFL);
            next_ptr = td->link;
            // if the td is active we're done
            if(td->command & 0x800000) {
                done = true;
            } else {
                // if not active, free it, or re-initialize it
                if(free) ise_uhci_free_td(_hcd, (ise_usb_td_t*) td);
                else ise_uhci_init_td(td);
            }
        }
        done |= (next_ptr & 1) ? true : false;
    }
    // set the driver private head pointer
    //q0->start_element_link = next_ptr;
    *last_queue_link = next_ptr;
    // if start is NULL, point end to NULL
    if(q0->start_element_link & 0x1) q0->end_element_link = 0x1;
    // if the last next ptr is null, point the end to the last queue
    else if(next_ptr & 0x1) q0->end_element_link = ((uint32_t) last_queue_link) | 0x2;
}

// find and remove a particular td from a queue
// returns true if td removed
bool ise_uhci_dequeue_td(ise_usb_queue_t* _q0, ise_usb_td_t* _td)
{
    ise_uhci_queue_t* q0 = (ise_uhci_queue_t*) _q0;
    ise_uhci_td_t* td = (ise_uhci_td_t*) _td;
    uint32_t td_val = (uint32_t) _td;

    uint32_t prev_ptr = 0x1;
    uint32_t* next_ptr = &(q0->start_element_link);
    bool removed = false;
    bool done = (*next_ptr & 0x1) ? true : false;
    while(!done) {
        if(*next_ptr & 0x2) {
            // this is a queue, move on
            prev_ptr = *next_ptr;
            next_ptr = (uint32_t*) (*next_ptr & ~0xFL);
        } else {
            // this is a td
            if((*next_ptr & ~0xFL) == (td_val & ~0xFL)) {
                // we found the td
                *next_ptr = td->link;
                // if the current points to the one we're removing, advance it
                if((q0->current_element_link & ~0xFL) == (td_val & ~0xFL)) {
                    q0->current_element_link = td->link;
                }
                // if the end points to the one we're removing, point it to the previous
                if((q0->end_element_link & ~0xFL) == (td_val & ~0xFL)) {
                    q0->end_element_link = prev_ptr;
                }
                removed = true;
                done = true;
            } else {
                // not found, so move on
                prev_ptr = *next_ptr;
                next_ptr = (uint32_t*) (*next_ptr & ~0xFL);
            }
        }
        done |= (*next_ptr & 0x1) ? true : false;
    }
    // if start is NULL, point end to NULL
    if(q0->start_element_link & 0x1) q0->end_element_link = 0x1;
    return removed;
}


// restart a queue
void ise_uhci_restart_queue(ise_usb_hcd_t* /*_hcd*/, ise_usb_queue_t* _q)
{
    ise_uhci_queue_t* q = (ise_uhci_queue_t*) _q;
    q->current_element_link = q->start_element_link;
}

// start a queue from the current element
void ise_uhci_start_queue(ise_usb_hcd_t* /*_hcd*/, ise_usb_queue_t* _q)
{
    ise_uhci_queue_t* q = (ise_uhci_queue_t*) _q;
    q->current_element_link &= ~1L;
}

// stop a queue
void ise_uhci_stop_queue(ise_usb_queue_t* _q)
{
    ise_uhci_queue_t* q = (ise_uhci_queue_t*) _q;
    q->current_element_link |= 1L;
}

// clear everything out of a queue
void ise_uhci_clear_queue(ise_usb_queue_t* _q)
{
    ise_uhci_queue_t* q = (ise_uhci_queue_t*) _q;
    q->current_element_link |= 1L;
    q->start_element_link |= 1L;
    q->end_element_link |= 1L;
}

// check if a queue is done with all work
bool ise_uhci_is_queue_done(ise_usb_queue_t* _q)
{
    ise_uhci_queue_t* q = (ise_uhci_queue_t*) _q;
    return (q->current_element_link & 0x1) ? true : false;
}

// check if queue has fatal error
bool ise_uhci_is_queue_fatal(ise_usb_queue_t* )
{
    // nothing to check for uhci
    return false;
}

// check for port status change
// returns -1 if port does not change
// otherwise, bit[2:0] is the speed of the device
// and bit [7] indicates a connected if 1, disconnected if 0
// this function should be called periodically
int ise_uhci_port_changed(ise_usb_hc_t* ise_usb, int device_id, int port)
{
    ise_uhci_t* ise_uhci = (ise_uhci_t*) (ise_usb->hcd);
    if(device_id != 0 || port >= ise_uhci->num_ports) return -1;

    int port_addr = ise_uhci->usb_base + ISE_UHCI_REG_PORTSC + 2*port;
    int port_data = inpw(port_addr);
    // if enable or connection change, we found the port
    if(port_data & 0xA) {
        // set the upper bit if port is connected
        int status = ((port_data & 0x1) ? 0x80 : 0x00);
        // set the port speed
        status |= ((port_data & 0x100) ? ISE_USB_SPEED_LOW : ISE_USB_SPEED_FULL) & 0x7;
        // return the port status
        return status;
    }

    return -1;
}

// reset and enable port
// returns port status (same as port_changed function) on success
// returns -1 if reset fails
int ise_uhci_reset_port(ise_usb_hc_t* ise_usb, int device_id, int port)
{
    ise_uhci_t* ise_uhci = (ise_uhci_t*) (ise_usb->hcd);
    if(device_id != 0 || port >= ise_uhci->num_ports) return -1;
    int port_addr = ise_uhci->usb_base + ISE_UHCI_REG_PORTSC + 2*port;

    //printf("Resetting port %d 0x%x\n", port, inpw(port_addr));
    // reset
    int data;
    //ise_time_wait(100); // wait at least 100 ms between port connected and reset
    data = inpw(port_addr) | 0x200;
    outpw(port_addr, data);
    ise_time_wait(50);  // wait at least 50ms
    data = inpw(port_addr);
    //printf("port reset: 0x%x\n", data);
    data &= ~0x20A;
    outpw(port_addr, data);
    ise_time_uwait(250); // wait 250 us

    data = inpw(port_addr);
    if(data & 1) {
        data |= 0x4;
        outpw(port_addr, data);
        //printf("Early port enable: 0x%x\n", data);
    }
    ise_time_wait(10);
    data = inpw(port_addr);
    if(data & 0x8) {
        outpw(port_addr, data);
        //printf("port enable changed: 0x%x", data);
    }

    int status = -1;
    if(!(data & 0x1)) {
        // if nothing attached, no need to enable port
        status = 0;
    } else if(data & 0x4) {
        // if port is enabled, provide port status
        status = ((data & 0x1) ? 0x80 : 0x00);
        // set the port speed
        status |= ((data & 0x100) ? ISE_USB_SPEED_LOW : ISE_USB_SPEED_FULL) & 0x7;
    }

    /*do {
        data = inpw(port_addr);
    } while((data & 0xA) == 0);

    // the port changed
    if(data & 1) {
        // port connected, so enable it, and clear the change bits
        outpw(port_addr, data | 0xE);
        status = 0x80 | (((data & 0x100) ? ISE_USB_SPEED_LOW : ISE_USB_SPEED_FULL) & 0x7);
    } else {
        status = 0;
    }*/

    /*int i;
    for(i=0; i<16; i++) {
        ise_time_wait(10);  // wait at least 10ms
        data = inpw(port_addr);
        printf("Reset loop %d: 0x%x\n", i, data);
    
        // if enable or connection change, clear those bits
        // then try again
        if(data & 0xA) {
            outpw(port_addr, data & 0x124E);
            continue;
        }
        
        // if nothing attached, no need to enable port
        if(!(data & 0x1)) {
            status = 0;
            break;
        }
        
        // if port is enabled, we're done
        if(data & 0x4) {
            status = ((data & 0x1) ? 0x80 : 0x00);
            // set the port speed
            status |= ((data & 0x100) ? ISE_USB_SPEED_LOW : ISE_USB_SPEED_FULL) & 0x7;
            break;
        }
        
        // enable the port
        outpw(port_addr, data | 0x4);
    }*/
    
    //printf("Done Resetting port 0x%x\n", inpw(port_addr));
    return status;
}

void ise_uhci_disable_port(ise_usb_hc_t* ise_usb, int device_id, int port)
{
    ise_uhci_t* ise_uhci = (ise_uhci_t*) (ise_usb->hcd);
    if(device_id != 0 || port >= ise_uhci->num_ports) return;

    int port_addr = ise_uhci->usb_base + ISE_UHCI_REG_PORTSC + 2*port;
    int port_data = inpw(port_addr);

    outpw(port_addr, port_data & ~(0x4));
    //printf("Disabling port 0x%x\n", inpw(port_addr));
}

// --------------------------------------------------------------------------------------------
// Initialization routines
// reset uhci controller; returns true if all goes well
bool ise_uhci_reset(ise_uhci_t* ise_uhci)
{
    // global reset 5 times
    int i;
    for(i=0; i<5; i++) {
        outpw(ise_uhci->usb_base + ISE_UHCI_REG_USBCMD, 0x4);
        ise_time_wait(11);  // at least 10ms
        outpw(ise_uhci->usb_base + ISE_UHCI_REG_USBCMD, 0x0);
    }
    
    // check if command is reset
    if(inpw(ise_uhci->usb_base + ISE_UHCI_REG_USBCMD) != 0x0) return false;
    // check if status bit is default value (halted)
    if(inpw(ise_uhci->usb_base + ISE_UHCI_REG_USBSTS) != 0x20) return false;
    // check if SOF is 0x40
    if(inpw(ise_uhci->usb_base + ISE_UHCI_REG_SOFMOD) != 0x40) return false;
    
    // reset the controller
    outpw(ise_uhci->usb_base + ISE_UHCI_REG_USBCMD, 0x2);
    ise_time_wait(50);  // wait a bit
    // check if command has restored to reset value
    if(inpw(ise_uhci->usb_base + ISE_UHCI_REG_USBCMD) != 0x0) return false;

    // Success!
    return true;
}

// allocate initial resources; returns true if success
bool ise_uhci_init_resources(ise_usb_hc_t* ise_usb)
{
    ise_uhci_t* ise_uhci = (ise_uhci_t*) ise_usb->hcd;

    // clear the memory pools
    int p, i;
    for(p=0; p<ISE_UHCI_MAX_MEM_POOL; p++) {
        for(i=0; i<16; i++) {
            ise_uhci->mem_alloc[p][i].i32 = 0x0;
        }
        ise_uhci->mem_pool[p] = NULL;
    }
    
    // allocate the first 8KB memory pool, 16B aligned
    ise_uhci->mem_pool[0] = (uint32_t*) ise_mem_aligned_malloc(0x2000, 4, ISE_MEM_REGION_DOS);
    if(ise_uhci->mem_pool[0] == NULL) return false;
    
    // Find a 4KB aligned region in the first memory pool
    // this will be used for the framelist, so we need to set the bits
    // corresponding to that region as used.
    uint32_t pool_addr = (uint32_t) ise_uhci->mem_pool[0];
    uint32_t framelist_addr = (pool_addr + 0x1000) & ~0xFFFL;
    // Get the offset, in 16B units
    uint32_t framelist_offset = (framelist_addr - pool_addr) >> 4;
    for(i=framelist_offset; i<framelist_offset+256; i++) {
        ise_uhci->mem_alloc[0][i >> 5].i32 |= 1 << (i & 0x1f);
    }
    ise_uhci->frame_list = (uint32_t*) framelist_addr;

    // the config buffer needs CONFIG_BUFFER size, so divide by 16 to get consecutive locations
    // if the frame list offset is over this, we can just allocate the first of these locations
    // otherwise, the locations after the framelist
    uint32_t num_config_buffer_locations = ISE_USB_CONFIG_BUFFER_SIZE / 16;
    uint32_t config_buffer_offset = (framelist_offset >= num_config_buffer_locations) ? 0 : framelist_offset + 256;
    for(i=config_buffer_offset; i<config_buffer_offset+num_config_buffer_locations; i++) {
        ise_uhci->mem_alloc[0][i >> 5].i32 |= 1 << (i & 0x1f);
    }
    ise_usb->config_buffer = (uint8_t*) (pool_addr + (config_buffer_offset << 4));
    ise_usb->config_req_packet = (ise_usb_device_request_packet_t*) ise_uhci_alloc_mem(ise_usb->hcd, 8);
    
    // allocate, initialize and link the primary queues
    // qN -> q1 -> q_bulk -> q_control, where N != 1
    ise_uhci->control_queue = (ise_uhci_queue_t*) ise_uhci_alloc_queue(ise_usb->hcd);
    ise_uhci->bulk_queue = (ise_uhci_queue_t*) ise_uhci_alloc_queue(ise_usb->hcd);
    //ise_uhci_init_queue(ise_uhci->control_queue);
    //ise_uhci_init_queue(ise_uhci->bulk_queue);
    ise_uhci_link_queues(ise_uhci->bulk_queue, ise_uhci->control_queue);
    ise_uhci_queue_t* periodic_queue[8];
    ise_uhci_queue_t* next_queue;
    for(i=0; i<8; i++) {
        periodic_queue[i] = (ise_uhci_queue_t*) ise_uhci_alloc_queue(ise_usb->hcd);
        //ise_uhci_init_queue(periodic_queue[i]);
        next_queue = (i==0) ? ise_uhci->bulk_queue : periodic_queue[0];
        ise_uhci_link_queues(periodic_queue[i], next_queue);
    }

    // initialize the framelist pointers
    // this pattern goes for 128 entries, and then repeats
    //  fl[0] = q1 -> bulk_q -> control_q
    //  fl[1] = q2 -> q1 -> ...
    //  fl[2] = q4 -> q1 -> ...
    //  fl[3] = q2 -> q1 -> ...
    //  fl[4] = q8 -> q1 -> ...
    //  fl[5] = q2 -> q1 -> ...
    //  fl[6] = q4 -> q1 -> ...
    //  fl[7] = q2 -> q1 -> ...
    //  fl[8] = q16-> q1 -> ...
    //   ...
    for(i=0; i<1024; i++) {
        if(i & 0x01) p = 1;
        else if(i & 0x02) p = 2;
        else if(i & 0x04) p = 3;
        else if(i & 0x08) p = 4;
        else if(i & 0x10) p = 5;
        else if(i & 0x20) p = 6;
        else if(i & 0x40) p = 7;
        else p = 0;
        // set bit 1 to indicate a queue
        ise_uhci->frame_list[i] = ((uint32_t) periodic_queue[p]) | 0x2;
        //ise_uhci->frame_list[i] = ((uint32_t) ise_uhci->bulk_queue) | 0x2;
    }

    return true;
}

// enable the device
void ise_uhci_enable(ise_uhci_t* ise_uhci)
{
    // disable interrupts
    outpw(ise_uhci->usb_base + ISE_UHCI_REG_USBINTR, 0x0);
    
    // set frame nuber to 0
    outpw(ise_uhci->usb_base + ISE_UHCI_REG_FRNUM, 0x0);
    
    // set frame list base address
    outpd(ise_uhci->usb_base + ISE_UHCI_REG_FRBASEADD, (uint32_t) ise_uhci->frame_list);
    
    // start of frame modifier should already be 0x40
    // but let's set it again for funsies
    outpw(ise_uhci->usb_base + ISE_UHCI_REG_SOFMOD, 0x40);
    
    // clear all of the status
    outpw(ise_uhci->usb_base + ISE_UHCI_REG_USBSTS, 0xFFFF);
    
    // write the command
    // bit 6 indicates the driver is loaded and is using the device
    // bit 0 tells the HC to start processing
    outpw(ise_uhci->usb_base + ISE_UHCI_REG_USBCMD, 0x41);
    
}

void ise_uhci_debug(FILE* file, ise_usb_hc_t* ise_usb)
{
    ise_uhci_t* ise_uhci = (ise_uhci_t*) ise_usb->hcd;
    if(file == NULL) file = stdout;
    fprintf(file, "command 0x%x\t", inpw(ise_uhci->usb_base + ISE_UHCI_REG_USBCMD));
    fprintf(file, "status 0x%x\t", inpw(ise_uhci->usb_base + ISE_UHCI_REG_USBSTS));
    fprintf(file, "frbase 0x%x\t", inpd(ise_uhci->usb_base + ISE_UHCI_REG_FRBASEADD));
    fprintf(file, "frnum 0x%x\t", inpw(ise_uhci->usb_base + ISE_UHCI_REG_FRNUM));
    fprintf(file, "framelist\n");
    ise_mem_dump(file, ise_uhci->frame_list, 4);
    ise_mem_dump(file, ise_uhci_get_queue(ise_usb->hcd, ISE_USB_QUEUE_2MS), 4);
    ise_mem_dump(file, ise_uhci_get_queue(ise_usb->hcd, ISE_USB_QUEUE_1MS), 4);
    fprintf(file, "bulk_q\n");
    ise_mem_dump(file, ise_uhci->bulk_queue, 4);
    fprintf(file, "control_q\n");
    ise_mem_dump(file, ise_uhci->control_queue, 4);
}

// count ports
int ise_uhci_count_ports(ise_uhci_t* ise_uhci)
{
    int p = 0;
    bool done = false;
    while(!done) {
        // Bit 7 of each valid port should be 1
        if(inpw(ise_uhci->usb_base + ISE_UHCI_REG_PORTSC + 2*p) & 0x80) {
            p++;
            // if we reach the maximum number of ports, then we're done
            if(p >= ISE_UHCI_RH_MAX_PORTS) done  = true;
        } else {
            done = true;
        }
    }
    return p;
}

// free resources
void ise_uhci_free_resources(ise_uhci_t* ise_uhci)
{
    int p;
    for(p=0; p<ISE_UHCI_MAX_MEM_POOL; p++) {
        if(ise_uhci->mem_pool[p]) {
            ise_mem_aligned_free(ise_uhci->mem_pool[p]);
            ise_uhci->mem_pool[p] = NULL;
        }
    }
}

// basic setup of uhci controller
bool ise_uhci_setup(ise_usb_hc_t* ise_usb)
{
    ise_uhci_t* ise_uhci = (ise_uhci_t*) ise_usb->hcd;

    // Get base address (should be in I/O space)
    ise_uhci->usb_base = ise_pci_read_config(0, ise_usb->pci_slot, ise_usb->pci_func, 0x20);

    // Check if base is in I/O space (lsb is 1)
    if(!(ise_uhci->usb_base & 1)) {
        printf("UHCI device using MMIO - not supported\n");
        return false;
    }

    // clear out the bottom bits (not really part of the address)
    ise_uhci->usb_base &= 0XFFFFFFFC;

    // check if another driver has already been installed (check the configured bit)
    if(inpw(ise_uhci->usb_base + ISE_UHCI_REG_USBCMD) & 0x40) {
        printf("UHCI device on slot/func 0x%x/0x%x - another driver already loaded\n", ise_usb->pci_slot, ise_usb->pci_func);
        return false;
    }
    //printf("Warning: ignoring driver check!!!!!!!!!!!\n");

    // Get the interrupt number
    ise_usb->interrupt_num = ise_pci_read_config(0, ise_usb->pci_slot, ise_usb->pci_func, 0x3C) & 0xFF;
    if(ise_usb->interrupt_num < 3 || ise_usb->interrupt_num > 0xF) {
        // in Dos, should only have interrupts between 0x3 and 0xF...
        printf("UHCI device on slot/func 0x%x/0x%x - bad interrupt number\n", ise_usb->pci_slot, ise_usb->pci_func);
        return false;
    }

    // Reset and see if it succeeds
    if(!ise_uhci_reset(ise_uhci)) {
        printf("UHCI device on slot/func 0x%x/0x%x - reset failed\n", ise_usb->pci_slot, ise_usb->pci_func);
        return false;
    }

    // allocate resources
    if(!ise_uhci_init_resources(ise_usb)) {
        printf("UHCI device on slot/func 0x%x/0x%x - could not allocate memory pool\n", ise_usb->pci_slot, ise_usb->pci_func);
        return false;
    }

    // Register interrupt handler
    /*if(!ise_pci_register_isr(ise_usb->interrupt_num, ise_usb_isr)) {
        printf("UHCI device on slot/func 0x%x/0x%x - could not register ISR\n", ise_usb->pci_slot, ise_usb->pci_func);
        ise_uhci_free_resources(ise_uhci);
        return false;
    }*/

    // count number of ports
    ise_uhci->num_ports = ise_uhci_count_ports(ise_uhci);

    // Success!
    return true;
}

// install the driver for all uhci devices
void ise_uhci_install()
{
    printf("installing uhci...\n");
    int s, f;
    for(s=0; s<ISE_PCI_MAX_SLOTS && ise_usb_num_hosts < ISE_USB_MAX_HC; s++) {
        for(f=0; f<ISE_PCI_MAX_FUNC && ise_usb_num_hosts < ISE_USB_MAX_HC; f++) {

            // First look for a valid device, that has the UHCI class code
            if(ise_pci.slot[s][f].vendor_id != ISE_PCI_VENDOR_INVALID && ise_pci.slot[s][f].class_code == ISE_UHCI_PCI_CLASS) {
                // Start registering the device
                ise_usb[ise_usb_num_hosts].pci_slot = s;
                ise_usb[ise_usb_num_hosts].pci_func = f;
                ise_pci.slot[s][f].priv = (void*) &ise_usb[ise_usb_num_hosts];
                
                // allocate uhci data
                ise_usb[ise_usb_num_hosts].hcd = (ise_usb_hcd_t*) malloc(sizeof(ise_uhci_t));
                if(ise_usb[ise_usb_num_hosts].hcd) {

                    // Setup the device, and if successful, count it
                    ise_uhci_t* ise_uhci = (ise_uhci_t*)ise_usb[ise_usb_num_hosts].hcd;
                    memset(ise_uhci, 0, sizeof(ise_uhci_t));
                    if(ise_uhci_setup(&ise_usb[ise_usb_num_hosts])) {
                        // display info
                        printf("UHCI device on slot/func 0x%x/0x%x\n", s, f);
                        printf("    USB base 0x%x   num ports %d\n", ise_uhci->usb_base, ise_uhci->num_ports);

                        // allocate root hub
                        ise_usb[ise_usb_num_hosts].device[0] = (ise_usb_device_t*) malloc(sizeof(ise_usb_hub_t));
                        memset(ise_usb[ise_usb_num_hosts].device[0], 0, sizeof(ise_usb_hub_t));
                        ise_usb_hub_t* root_hub = (ise_usb_hub_t*) ise_usb[ise_usb_num_hosts].device[0];
                        root_hub->num_ports = ise_uhci->num_ports;
                        
                        root_hub->port_changed = ise_uhci_port_changed;
                        root_hub->reset_port = ise_uhci_reset_port;
                        root_hub->disable_port = ise_uhci_disable_port;
                        ise_usb[ise_usb_num_hosts].alloc_mem = ise_uhci_alloc_mem;
                        ise_usb[ise_usb_num_hosts].free_mem = ise_uhci_free_mem;
                        ise_usb[ise_usb_num_hosts].alloc_queue = ise_uhci_alloc_queue;
                        ise_usb[ise_usb_num_hosts].free_queue = ise_uhci_free_queue;
                        ise_usb[ise_usb_num_hosts].alloc_td = ise_uhci_alloc_td;
                        ise_usb[ise_usb_num_hosts].free_td = ise_uhci_free_td;
                        ise_usb[ise_usb_num_hosts].get_queue = ise_uhci_get_queue;
                        ise_usb[ise_usb_num_hosts].fill_qh = ise_uhci_fill_qh;
                        ise_usb[ise_usb_num_hosts].fill_td = ise_uhci_fill_td;
                        ise_usb[ise_usb_num_hosts].push_queue = ise_uhci_push_queue;
                        ise_usb[ise_usb_num_hosts].enqueue_queue = ise_uhci_enqueue_queue;
                        ise_usb[ise_usb_num_hosts].dequeue_queue = ise_uhci_dequeue_queue;
                        ise_usb[ise_usb_num_hosts].push_td = ise_uhci_push_td;
                        ise_usb[ise_usb_num_hosts].enqueue_td = ise_uhci_enqueue_td;
                        ise_usb[ise_usb_num_hosts].is_td_done = ise_uhci_is_td_done;
                        ise_usb[ise_usb_num_hosts].is_td_fatal = ise_uhci_is_td_fatal;
                        ise_usb[ise_usb_num_hosts].is_td_stalled = ise_uhci_is_td_stalled;
                        ise_usb[ise_usb_num_hosts].is_td_short = ise_uhci_is_td_short;
                        ise_usb[ise_usb_num_hosts].activate_td = ise_uhci_activate_td;
                        ise_usb[ise_usb_num_hosts].deactivate_td = ise_uhci_deactivate_td;
                        ise_usb[ise_usb_num_hosts].clear_td_status = ise_uhci_clear_td_status;
                        ise_usb[ise_usb_num_hosts].restart_td = ise_uhci_restart_td;
                        ise_usb[ise_usb_num_hosts].dequeue_completed_tds = ise_uhci_dequeue_completed_tds;
                        ise_usb[ise_usb_num_hosts].dequeue_td = ise_uhci_dequeue_td;
                        ise_usb[ise_usb_num_hosts].restart_queue = ise_uhci_restart_queue;
                        ise_usb[ise_usb_num_hosts].start_queue = ise_uhci_start_queue;
                        ise_usb[ise_usb_num_hosts].stop_queue = ise_uhci_stop_queue;
                        ise_usb[ise_usb_num_hosts].clear_queue = ise_uhci_clear_queue;
                        ise_usb[ise_usb_num_hosts].is_queue_done = ise_uhci_is_queue_done;
                        ise_usb[ise_usb_num_hosts].is_queue_fatal = ise_uhci_is_queue_fatal;
                        ise_usb_num_hosts++;
                        
                        // enable host controller
                        ise_uhci_enable(ise_uhci);
                    } else {
                        free(ise_usb[ise_usb_num_hosts].hcd);
                        ise_usb[ise_usb_num_hosts].hcd = NULL;
                    }

                }
            }

        }
    }
}

// uninstall the driver
void ise_uhci_uninstall()
{
    int h;
    for(h=0; h<ise_usb_num_hosts; h++) {
        if(ise_pci.slot[ise_usb[h].pci_slot][ise_usb[h].pci_func].class_code == ISE_UHCI_PCI_CLASS) {
            ise_uhci_t* ise_uhci = (ise_uhci_t*) ise_usb[h].hcd;
            if(ise_uhci) {
                ise_uhci_reset(ise_uhci);
                ise_uhci_free_resources(ise_uhci);
                free(ise_uhci);
                ise_usb[h].hcd = NULL;
            }
            ise_pci.slot[ise_usb[h].pci_slot][ise_usb[h].pci_func].priv = NULL;
        }
    }
}
