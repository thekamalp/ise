// InfiniScroll Engine
// ise_ehci.h
// Enhanced Host Controller Interface
//
// Kamal Pillai
// 2/2/2019

#include "ise.h"

// --------------------------------------------------------------------------------------------
// USB driver calls

// allocates size in bytes of 32B entries from memory pool
void* ise_ehci_alloc_mem(ise_usb_hcd_t* _hcd, int size)
{
    int num_entries = (size + 31) / 32; // divide by 32, but round up

    // only support up to 32 entries (1024 bytes)
    if(num_entries <= 0 || num_entries > 32) {
        printf("Alloc size too big/small\n");
        return NULL;
    }

    ise_ehci_t* ise_ehci = (ise_ehci_t*) _hcd;
    void* entry = NULL;
    int offset;
    int p;

    // Loop through each pool of memory for an empty slot
    for(p=0; p<ISE_EHCI_MAX_MEM_POOL && entry == NULL; p++) {
        // if we need to use this pool, and it hasn't been allocated, do so now
        if(ise_ehci->mem_pool[p] == NULL) {
            // allocate 8KB, 32B aligned
            ise_ehci->mem_pool[p] = (uint32_t*) ise_mem_aligned_malloc(0x2000, 5, ISE_MEM_REGION_DOS);
            // if we couldn't allocate the pool, fail
            if(ise_ehci->mem_pool[p] == NULL) {
                printf("Could not allocate DOS mem\n");
                return NULL;
            }
        }
        offset = ise_mem_alloc_entries(num_entries, ise_ehci->mem_alloc[p], 8);
        if(offset >= 0) {
            // scale offset to account for 8 DWORDS per entry
            offset = offset << 3;
            entry = ise_ehci->mem_pool[p] + offset;
        }
    }

    if(entry == NULL) {
        printf("Could not allocate entries\n");
    }
    return entry;
}

// frees size in bytes of 32B entries back to memory pool
void ise_ehci_free_mem(ise_usb_hcd_t* _hcd, void* entry, int size)
{
    int num_entries = (size + 31) / 32; // divide by 32, but round up
    // only support up to 32 entries (1024 Bytes)
    if(num_entries <= 0 || num_entries > 32) return;

    ise_ehci_t* ise_ehci = (ise_ehci_t*) _hcd;
    uint32_t entry_addr = (uint32_t) entry;
    uint32_t mem_pool_addr;
    uint32_t offset;
    bool done = false;
    int p;
    // Loop through each memory to pool to find which pool the address belongs to
    for(p=0; p<ISE_EHCI_MAX_MEM_POOL && !done; p++) {
        mem_pool_addr = (uint32_t) ise_ehci->mem_pool[p];
        // if entry address is within 8KB of the memory pool address,
        // then it belongs to the bool
        // note, offset goes negative, it is treated as a very large positive value
        offset = entry_addr - mem_pool_addr;
        if(offset < 0x2000) {
            // scale offset for 32 Bytes per entry
            offset = offset >> 5;
            done = ise_mem_free_entries(offset, num_entries, ise_ehci->mem_alloc[p], 8);
        }
    }
}

// initialize a transfer descriptor
void ise_ehci_init_qtd(ise_ehci_qtd_t* qtd)
{
    int i;
    qtd->next_qtd = 0x1;
    //qtd->alt_next_qtd = 0x1;
    qtd->command = 0x0;
    for(i=0; i<5; i++) {
        qtd->buffer[i] = 0x0;
    }
}

// initialize a 64b transfer descriptor
void ise_ehci_init_qtd64(ise_ehci_qtd64_t* qtd64)
{
    int i;
    ise_ehci_init_qtd(&(qtd64->qtd32));
    for(i=0; i<5; i++) {
        qtd64->high_buffer[i] = 0x0;
    }
}

// initialize a queue
void ise_ehci_init_queue(ise_ehci_queue_t* q)
{
    q->head_link = 0x1;
    q->device = 0x0;
    q->mask = 0x0;
    q->current_link = 0x0;
    q->tail_link = 0x1;
    q->start_link = 0x1;
    q->end_link = 0x1;
    ise_ehci_init_qtd64(&(q->qtd));
}

// set next pointer of q0 to point to q1
void ise_ehci_link_queues(ise_ehci_queue_t* q0, ise_ehci_queue_t* q1)
{
    // set bit 1 to indicate a queue
    q0->head_link = ((uint32_t) q1) | 0x2;
}

// allocate a queue
ise_usb_queue_t* ise_ehci_alloc_queue(ise_usb_hcd_t* _hcd)
{
    ise_ehci_queue_t* q = (ise_ehci_queue_t*) ise_ehci_alloc_mem(_hcd, sizeof(ise_ehci_queue_t));
    ise_ehci_init_queue(q);
    return (ise_usb_queue_t*) q;
}

// free a queue
void ise_ehci_free_queue(ise_usb_hcd_t* _hcd, ise_usb_queue_t* _q)
{
    uint32_t* q = (uint32_t*) _q;
    ise_ehci_free_mem(_hcd, q, sizeof(ise_ehci_queue_t));
}

// allocate a transfer descriptor
ise_usb_td_t* ise_ehci_alloc_td(ise_usb_hcd_t* _hcd)
{
    ise_ehci_t* ise_ehci = (ise_ehci_t*) _hcd;
    // bit 0 of flags indicates 64b host controller
    int size = (ise_ehci->flags & 0x1) ? sizeof(ise_ehci_qtd64_t) : sizeof(ise_ehci_qtd_t);
    ise_ehci_qtd_t* qtd = (ise_ehci_qtd_t*) ise_ehci_alloc_mem(_hcd, size);
    if(ise_ehci->flags & 0x1) ise_ehci_init_qtd64((ise_ehci_qtd64_t*) qtd);
    else ise_ehci_init_qtd(qtd);
    qtd->alt_next_qtd = (uint32_t) ise_ehci->dummy_td;
    return (ise_usb_td_t*) qtd;
}

// free a transfer descriptor
void ise_ehci_free_td(ise_usb_hcd_t* _hcd, ise_usb_td_t* _td)
{
    ise_ehci_t* ise_ehci = (ise_ehci_t*) _hcd;
    uint32_t* td = (uint32_t*) _td;
    // bit 0 of flags indicates 64b host controller
    int size = (ise_ehci->flags & 0x1) ? sizeof(ise_ehci_qtd64_t) : sizeof(ise_ehci_qtd_t);
    ise_ehci_free_mem(_hcd, td, size);
}


// get a primary queue
ise_usb_queue_t* ise_ehci_get_queue(ise_usb_hcd_t* _hcd, int q_id)
{
    ise_ehci_t* ise_ehci = (ise_ehci_t*) _hcd;
    ise_usb_queue_t* q = NULL;
    if(q_id < 8) {
        int index = (q_id == 0) ? 0 : (1 << (q_id-1));
        uint32_t q_ptr = ise_ehci->periodic_list[index];
        q = (ise_usb_queue_t*) (q_ptr & ~0xFL);
    } else {
        switch(q_id) {
            case ISE_USB_QUEUE_BULK:
                q = (ise_usb_queue_t*) ise_ehci->bulk_queue;
                break;
            case ISE_USB_QUEUE_CONTROL:
                q = (ise_usb_queue_t*) ise_ehci->control_queue;
                break;
        }
    }

    return q;
}

// finds the high speed hub's device and port
void ise_ehci_get_high_speed_parent(ise_usb_hc_t* ise_usb, uint8_t device_id, uint8_t* parent_device_id, uint8_t* parent_port)
{
    if(ise_usb->device[device_id] == NULL) {
        // if device is 0, this doesn't matter, so just return 0's
        *parent_device_id = 0;
        *parent_port = 0;
        return;
    }
    
    uint8_t cur_parent = ise_usb->device[device_id]->parent_device_id;
    if(ise_usb->device[cur_parent] && ise_usb->device[cur_parent]->speed == ISE_USB_SPEED_HIGH) {
        // if the parent is high speed, return it
        *parent_device_id = cur_parent;
        *parent_port = ise_usb->device[device_id]->port + 1;
    } else {
        // otherwise, find the parent's parent
        ise_ehci_get_high_speed_parent(ise_usb, cur_parent, parent_device_id, parent_port);
    }
}

// fills out an already allocated queue head
void ise_ehci_fill_qh(ise_usb_queue_t* _q, ise_usb_hc_t* ise_usb, uint8_t device_id, uint8_t end_pt)
{
    ise_ehci_queue_t* q = (ise_ehci_queue_t*) _q;
    ise_ehci_t* ise_ehci = (ise_ehci_t*) ise_usb->hcd;
    q->device = 0;
    q->mask = 0;
    
    ise_usb_device_t* device = ise_usb->device[device_id];
    if(device == NULL) return;

    uint8_t end_pt_type = device->endpoint[end_pt].attributes & 0x3;
    // control flag
    if(device->speed != ISE_USB_SPEED_HIGH && (end_pt_type == 0x0)) {
        // for non-high speed, control end points, set the control flag (bit 27)
        q->device |= 0x8000000;
    }
    // set max packet length
    int max_packet = device->endpoint[end_pt].max_packet_size & 0x7FF;
    if(max_packet < 8) max_packet = 8; // Should be at least 8
    q->device |= max_packet << 16;
    // set data toggle control to come from qtd (bit 14)
    q->device |= 0x4000;
    // enum for speed (bits 13:12)- not the same encoding in SW
    q->device |= (device->speed == ISE_USB_SPEED_LOW) ? 0x1000 :
        ((device->speed == ISE_USB_SPEED_FULL) ? 0x0000 : 
        ((device->speed == ISE_USB_SPEED_HIGH) ? 0x2000 : 0x3000) );
    // end point
    q->device |= ((uint32_t) end_pt & 0xF) << 8;
    // device
    q->device |= device_id & 0x7F;

    // get the high speed parent for this device
    uint8_t parent, port;
    ise_ehci_get_high_speed_parent(ise_usb, device_id, &parent, &port);

    // set multiplier to 1 (bit 31:30)
    q->mask |= 0x40000000;
    // set the high speed parent (bits 22:16) / port (bits 29:23)
    q->mask |= ((uint32_t) port & 0x7f) << 23;
    q->mask |= ((uint32_t) parent & 0x7f) << 16;
    if(device->speed != ISE_USB_SPEED_HIGH && ((end_pt_type == 0x1) || (end_pt_type == 0x3))) {
        // for non-high speed device, that has a periodic (iso or interrupt) end point, set the c-mask
        // for now, just make it 2 to 4 away from s-mask
        q->mask |= 1 << (8 + ((ise_ehci->s_mask_offset + 2) & 0x7));
        q->mask |= 1 << (8 + ((ise_ehci->s_mask_offset + 3) & 0x7));
        q->mask |= 1 << (8 + ((ise_ehci->s_mask_offset + 4) & 0x7));
    }
    if(end_pt_type == 0x3) {
        // for interrupt end points, program s-mask
        // Don't update the s-mask offset  - otherwise have to handle frame span
        q->mask |= 1 << (ise_ehci->s_mask_offset & 0x7);
        //ise_ehci->s_mask_offset = (ise_ehci->s_mask_offset + 1) & 0x7;
    }
}

// fills out an already allocated transfer descriptor
void ise_ehci_fill_td(ise_usb_td_t* _td, ise_usb_td_info_t* td_info)
{
    ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) _td;

    int i;
    // set the start buffer
    td->buffer[0] = ise_mem_linear_to_pa((uint32_t) td_info->buffer);
    for(i=1; i<5; i++) {
        // each additional buffer is an aligned 4KB away
        td->buffer[i] = ise_mem_linear_to_pa((((uint32_t) td_info->buffer) & ~0xFFFL) + (i << 12));
    }
    td->command = 0;
    td->command |= (td_info->dt) ? 0x80000000 : 0x0;              // set data toggle
    td->command |= (td_info->length & 0x7FFF) << 16;              // length of transfer
    td->command |= (td_info->ioc) ? 0x8000 : 0x0;                 // interrupt on complete
    td->command |= 0xC00;                                         // error count
    td->command |= (td_info->pid == ISE_USB_PID_SETUP) ? 0x200 :  // encoded pid
        ((td_info->pid == ISE_USB_PID_IN) ? 0x100 :
        ((td_info->pid == ISE_USB_PID_OUT) ? 0x000 : 0x300));
    td->command |= 0x80;
}
    

// adds q1 to head of q0
void ise_ehci_push_queue(ise_usb_queue_t* _q0, ise_usb_queue_t* _q1)
{
    ise_ehci_queue_t* q0 = (ise_ehci_queue_t*) _q0;
    ise_ehci_queue_t* q1 = (ise_ehci_queue_t*) _q1;
    
    // set head link of q1 to q0
    q1->head_link = q0->head_link;
    // q0 head link should now point to q1
    q0->head_link = ((uint32_t) q1) | 0x2;
    // if tail link is null, point it to head
    if(q0->tail_link & 0x1) q0->tail_link = q0->head_link;
}

// adds q1 to tail of q0
void ise_ehci_enqueue_queue(ise_usb_queue_t* _q0, ise_usb_queue_t* _q1)
{
    ise_ehci_queue_t* q0 = (ise_ehci_queue_t*) _q0;
    ise_ehci_queue_t* q1 = (ise_ehci_queue_t*) _q1;

    // if tail is null, then this queue has nothing in it
    // treat q0 as the tail
    ise_ehci_queue_t* q_tail = (q0->tail_link & 0x1) ? q0 : (ise_ehci_queue_t*) (q0->tail_link & ~0xFL);
        
    q1->head_link = q_tail->head_link;
    q_tail->head_link = ((uint32_t) q1) | 0x2;
    q0->tail_link = q_tail->head_link;
}

// finds and removes q1 from q0
// returns true if removed
bool ise_ehci_dequeue_queue(ise_usb_queue_t* _q0, ise_usb_queue_t* _q1)
{
    ise_ehci_queue_t* q0 = (ise_ehci_queue_t*) _q0;
    ise_ehci_queue_t* q1 = (ise_ehci_queue_t*) _q1;
    uint32_t q1_val = (uint32_t) _q1;

    //printf("dequeue 0x%x from 0x%x\n", q1, q0);
    uint32_t* next_ptr = &(q0->head_link);
    bool removed = false;
    bool done = (*next_ptr & 0x1) ? true : false;
    while(!done) {
        //printf("next_ptr: 0x%x->0x%x\n", next_ptr, *next_ptr);
        // if we are pointing to tail, then this is the last item to look at
        done = (*next_ptr == q0->tail_link) ? true : false;
        if((*next_ptr & 0x6) == 0x2) {
            // this is a queue
            if((*next_ptr & ~0xFL) == (q1_val & ~0xFL)) {
                // we found the queue
                *next_ptr = q1->head_link;
                //if tail points to the one we're removing, make it point to one prior
                if((q0->tail_link & ~0xFL) == (q1_val & ~0xFL)) {
                    // if prior is the head, then queue is empty
                    q0->tail_link = (next_ptr == &(q0->head_link)) ? 0x1 : ((uint32_t) next_ptr) | 0x2;
                }
                removed = true;
                done = true;
            } else {
                // not found, so move on
                next_ptr = (uint32_t*) (*next_ptr & ~0xFL);
            }
        } else {
            // this is not a queue, so move on
            next_ptr = (uint32_t*) (*next_ptr & ~0xFL);
        }
        done |= (*next_ptr & 0x1) ? true : false;
    }
    return removed;
}

// adds td to head of q0
void ise_ehci_push_td(ise_usb_queue_t* _q0, ise_usb_td_t* _td)
{
    ise_ehci_queue_t* q0 = (ise_ehci_queue_t*) _q0;
    ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) _td;
    
    // set the next pointer of td to the start of queue
    td->next_qtd = q0->start_link;
    //td->alt_next_qtd = q0->start_link;
    // queue's start now points to the newly added td
    q0->start_link = (uint32_t) td;
    // if end is NULL, then queue is empty, so point it to the new td
    if(q0->end_link & 0x1) q0->end_link = q0->start_link;
}

// adds td to tail of q0
void ise_ehci_enqueue_td(ise_usb_queue_t* _q0, ise_usb_td_t* _td)
{
    ise_ehci_queue_t* q0 = (ise_ehci_queue_t*) _q0;
    ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) _td;
    
    // adding to tail, so set td's next to NULL
    td->next_qtd = 0x1;
    //td->alt_next_qtd = 0x1;
    // if queue is currently empty
    if(q0->start_link & 0x1) {
        q0->start_link = ((uint32_t) td);
        q0->end_link = q0->start_link;
    } else {
        // if not empty, set the end's next to point to the new td
        ise_ehci_qtd_t* end_td = (ise_ehci_qtd_t*) (q0->end_link & ~0xFL);
        end_td->next_qtd = ((uint32_t) td);
        //end_td->alt_next_qtd = end_td->next_qtd;
        q0->end_link = end_td->next_qtd;
    }
}

// check if a TD is completed
bool ise_ehci_is_td_done(ise_usb_td_t* _td)
{
    ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) _td;
    
    // bit 7 is the active bit
    return (td->command & 0x80) ? false : true;
}

// check if nakd...really if it's a short packet
/*bool ise_ehci_is_td_nakd(ise_usb_td_t* _td)
{
    ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) _td;

    // check if total transfer is non-zero, and active bit is 0
    return ((((td->command >> 16) & 0x7FFF) != 0x0) && ((td->command & 0x80) == 0x0)) ? true : false;
}*/

// check if td has fatal error
bool ise_ehci_is_td_fatal(ise_usb_td_t* _td)
{
    ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) _td;

    // bit 0 is the ping state bit, and bit 7 is the active bit
    // bit 1 is split state
    // ignore those.  Anything else is a fatal error
    return (td->command & 0x7C) ? true : false;
}

// check if stalled bti is set
bool ise_ehci_is_td_stalled(ise_usb_td_t* _td)
{
    ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) _td;
    
    // bit 6 is the halted (or stalled bit in UHCI terms)
    return ((td->command & 0x7E) == 0x40) ? true : false;
}

bool ise_ehci_is_td_short(ise_usb_td_t* _td)
{
    ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) _td;

    // check if total transfer is non-zero, and active bit is 0
    return ((((td->command >> 16) & 0x7FFF) != 0x0) && ((td->command & 0x80) == 0x0)) ? true : false;
}

// activate TD
void ise_ehci_activate_td(ise_usb_td_t* _td)
{
    ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) _td;
    
    // bit 7 is active bit, and clear all other status bits
    td->command = (td->command & 0xFFFFFF00) | 0x80;
}

// deactiveate TD
void ise_ehci_deactivate_td(ise_usb_td_t* _td)
{
    ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) _td;
    
    // bit 7 is the active bit
    td->command &= ~0x80L;
}

// clear any status btis in TD
void ise_ehci_clear_td_status(ise_usb_td_t* _td)
{
    ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) _td;
    
    // bits 6:1 are status btis to clear
    td->command &= ~0x7E;
}

// restart TD if completed without errors
// returns posive value if restart, otherwise an error
int ise_ehci_restart_td(ise_usb_td_t* _td, uint32_t addr, int len)
{
    ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) _td;
    
    // first check that all status btis (except ping) are clear
    bool inactive = (td->command & 0xFE) == 0x0;
    if(inactive) {//} || is_short) {
        // set the start buffer
        td->buffer[0] = ise_mem_linear_to_pa(addr);
        int i;
        for(i=1; i<5; i++) {
            // each additional buffer is an aligned 4KB away
            td->buffer[i] = ise_mem_linear_to_pa((addr & ~0xFFFL) + (i << 12));
        }
        // clear out total bytes, and reload new len
        td->command &= 0x8000FFFF;
        td->command |= ((len & 0x7FFF) << 16);
		// NULL buffer - return error
		if(td->buffer == 0x0) return -1;
        // activate td
        td->command |= 0x80;
        return 0;
    } else {
        return -1;
    }
}

// remove completed td's in the queue, and optionally frees them
void ise_ehci_dequeue_completed_tds(ise_usb_hcd_t* _hcd, ise_usb_queue_t* _q0, bool free)
{
    ise_ehci_t* ise_ehci = (ise_ehci_t*) _hcd;
    ise_ehci_queue_t* q0 = (ise_ehci_queue_t*) _q0;
    
    uint32_t next_ptr = q0->start_link;
    ise_ehci_qtd_t* td;
    bool done = (next_ptr & 0x1) ? true : false;
    while(!done) {
        td = (ise_ehci_qtd_t*) (next_ptr & ~0xFL);
        next_ptr = td->next_qtd;
        // if the td is acive, we're done
        if(td->command & 0x80) {
            done = true;
        } else {
            // if not active, free it, or re-initialize it
            if(free) ise_ehci_free_td(_hcd, (ise_usb_td_t*) td);
            else ise_ehci_init_qtd(td);
        }
        done |= (next_ptr & 0x1) ? true : false;
    }
    // set the start ptr
    q0->start_link = next_ptr;
    // if start is NULL, point end to NULL
    if(q0->start_link & 0x1) q0->end_link = 0x1;
}

// find and remove a particular td from a queue
// returns true if td removed
bool ise_ehci_dequeue_td(ise_usb_queue_t* _q0, ise_usb_td_t* _td)
{
    ise_ehci_queue_t* q0 = (ise_ehci_queue_t*) _q0;
    ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) _td;
    uint32_t td_val = (uint32_t) _td;

    uint32_t* next_ptr = &(q0->start_link);
    bool removed = false;
    bool done = (*next_ptr & 0x1) ? true : false;
    while(!done) {
        // check if this is the td we are looking for
        if((*next_ptr & ~0xFL) == (td_val & ~0xFL)) {
            // td found
            *next_ptr = td->next_qtd;
            // if the current points to the one we're removing, advance it
            if((q0->qtd.qtd32.next_qtd & ~0xFL) == (td_val & ~0xFL)) {
                q0->qtd.qtd32.next_qtd = td->next_qtd;
            }
            // check the alt_next_qtd
            //if((q0->qtd.qtd32.alt_next_qtd & ~0xFL) == (td_val & ~0xFL)) {
            //    q0->qtd.qtd32.alt_next_qtd = td->next_qtd;
            //}
            removed = true;
            done = true;
        } else {
            // not found, so move on
            next_ptr = (uint32_t*) (*next_ptr & 0xFL);
        }
        done |= (*next_ptr & 0x1) ? true : false;
    }
    // if start is NULL, point end is NULL
    if(q0->start_link & 0x1) q0->end_link = 0x1;
    return removed;
}

// restart a queue
void ise_ehci_restart_queue(ise_usb_hcd_t* /*_hcd*/, ise_usb_queue_t* _q)
{
    ise_ehci_queue_t* q = (ise_ehci_queue_t*) _q;
    q->qtd.qtd32.next_qtd = q->start_link;
}

// start a q queue from where it was stopped
void ise_ehci_start_queue(ise_usb_hcd_t* /*_hcd*/, ise_usb_queue_t* _q)
{
    ise_ehci_queue_t* q = (ise_ehci_queue_t*) _q;
    //ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) (q->current_link);
    //td->next_qtd &= ~0x1L;
    //if((td->alt_next_qtd & ~0xFL) != 0x0) {
    //    td->alt_next_qtd &= ~0xFL;
    //}
    q->qtd.qtd32.next_qtd &= ~0x1L;
    //if((q->qtd.qtd32.alt_next_qtd & ~0xFL) != 0x0) {
    //    q->qtd.qtd32.alt_next_qtd &= ~0xFL;
    //}
}

// stop a q queue
void ise_ehci_stop_queue(ise_usb_queue_t* _q)
{
    ise_ehci_queue_t* q = (ise_ehci_queue_t*) _q;
    //ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) (q->current_link);
    //td->command = 0x0;
    //td->next_qtd |= 0x1L;
    //td->alt_next_qtd |= 0x1L;
    q->qtd.qtd32.command = 0x0;
    q->qtd.qtd32.next_qtd |= 0x1L;
    //q->qtd.qtd32.alt_next_qtd |= 0x1L;
    //ise_mem_dump(stdout, q, 8);
    //ise_mem_dump(stdout, &(q->qtd.qtd32.next_qtd), 1);
}

// clear everything out of a queue
void ise_ehci_clear_queue(ise_usb_queue_t* _q)
{
    ise_ehci_queue_t* q = (ise_ehci_queue_t*) _q;
    //ise_ehci_qtd_t* td = (ise_ehci_qtd_t*) (q->current_link);
    //td->next_qtd |= 0x1L;
    q->qtd.qtd32.next_qtd |= 0x1L;
    q->start_link |= 0x1;
    q->end_link |= 0x1;
}

// check if a queue is done with all work
bool ise_ehci_is_queue_done(ise_usb_queue_t* _q)
{
    ise_ehci_queue_t* q = (ise_ehci_queue_t*) _q;
    // check if the next td is null
    bool done = (q->qtd.qtd32.next_qtd & 0x1) ? true : false;
    if((q->current_link == (q->end_link & ~0xFL)) && ((q->qtd.qtd32.command & 0x80) == 0x0)) {
        done = true;
    }
    // if done, and there is a tail link, then check that the tail is also done
    /*if(done && ((q->tail_link & 0x1) == 0)) {
        ise_ehci_queue_t* q_tail = (ise_ehci_queue_t*) (q->tail_link & ~0xFL);
        done = (q_tail->qtd.qtd32.next_qtd & 0x1) ? true : false;
    }*/
    return done;
}

// check if queue has fatal error
bool ise_ehci_is_queue_fatal(ise_usb_queue_t* _q)
{
    ise_ehci_queue_t* q = (ise_ehci_queue_t*) _q;
    ise_ehci_qtd_t* td = &(q->qtd.qtd32);

    // bit 0 is the ping state bit, and bit 7 is the active bit
    // bit 1 is split state
    // ignore those.  Anything else is a fatal error
    return (td->command & 0x7C) ? true : false;
}

// check for port status change
// returns -1 if port does not change
// otherwise, bit[2:0] is the speed of the device
// bit [7] indicates a connected if 1, disconnected if 0
// this function should be called perioidically
int ise_ehci_port_changed(ise_usb_hc_t* ise_usb, int device_id, int port)
{
    ise_ehci_t* ise_ehci = (ise_ehci_t*) (ise_usb->hcd);
    if(device_id != 0 || port >= ise_ehci->num_ports) return -1;
    
    int port_index = ISE_EHCI_OP_PORTSC + port;
    uint32_t port_data = ise_ehci->op[port_index];
    // check if port connection or port enabled changed
    if(port_data & 0xA) {
        printf("ehci port %d changed: 0x%x\n", port, port_data);
        // check if line status indicates low speed
        if((port_data & 0xC00) == 0x400) {
            printf("ehci port %d ownership transferred to companion controller\n", port);
            // transfer ownership to companion controller
            ise_ehci->op[port_index] |= 0x2000;
            // wait a bit before clearing connection change
            ise_time_wait(10);
            ise_ehci->op[port_index] |= 0xA;
            // return that we had no port change
            return -1;
        }
        // otherwise, we may have a full or high speed device
        // we won't know until we do a reset, so let's assume it's
        // high speed for now, and let the reset occur
        // set bit 7 if device connected
        return ((port_data & 0x1) ? 0x80 : 0x0) | ISE_USB_SPEED_HIGH;
    }
    // no change
    return -1;
}

// reset and enable port
// returns port status (same as port_changed function) on  success
// returns -1 if reset fails
int ise_ehci_reset_port(ise_usb_hc_t* ise_usb, int device_id, int port)
{
    ise_ehci_t* ise_ehci = (ise_ehci_t*) (ise_usb->hcd);
    if(device_id != 0 || port >= ise_ehci->num_ports) return -1;

    int port_index = ISE_EHCI_OP_PORTSC + port;

    // set reset bit
    printf("ehci port reset %d\n", port);
    ise_ehci->op[port_index] |= 0x100;
    // wait for reset to be set
    int i = 0;
    while((i<10) && !(ise_ehci->op[port_index] & 0x100)) {
        ise_time_wait(1);
        i++;
    }
    if(i==10) {
        return -1;
    }
    
    // wait 50 ms
    ise_time_wait(50);
    
    // clear reset
    ise_ehci->op[port_index] &= ~0x100;
    // wait for reset to clear
    i = 0;
    while((i<10) && (ise_ehci->op[port_index] & 0x100)) {
        ise_time_wait(1);
        i++;
    }
    
    // wait 10 ms recovery
    ise_time_wait(10);
    
    // clear port changed state
    ise_ehci->op[port_index] |= 0xA;
    
    // check if port is enabled - if so, we have a high speed device
    // attached, and send the return code accordingly
    if(ise_ehci->op[port_index] & 0x4) {
        return ((ise_ehci->op[port_index] & 0x1) ? 0x80 : 0x0) | ISE_USB_SPEED_HIGH;
    } else {
        // otherwise this is a low speed device - change port owner
        // and return that nothing is connected
        ise_ehci->op[port_index] |= 0x2000;
        return 0x0;
    }
}

void ise_ehci_disable_port(ise_usb_hc_t* ise_usb, int device_id, int port)
{
    ise_ehci_t* ise_ehci = (ise_ehci_t*) (ise_usb->hcd);
    if(device_id != 0 || port >= ise_ehci->num_ports) return;
    
    int port_index = ISE_EHCI_OP_PORTSC + port;
    ise_ehci->op[port_index] &= ~0x2L;
}

// --------------------------------------------------------------------------------------------
// Initialization routines
// reset ehci controller; returns true if all goes well
bool ise_ehci_reset(ise_ehci_t* ise_ehci)
{
    // Make sure the HC is halted by clearing the run/stop bit
    ise_ehci->op[ISE_EHCI_OP_USBCMD] &= ~0x1L;
    
    // Reset HC
    ise_ehci->op[ISE_EHCI_OP_USBCMD] |= 0x2;
    // Wait for HC to come out of reset, or until we reach a timeout (20ms)
    int timeout = 20;
    while((ise_ehci->op[ISE_EHCI_OP_USBCMD] & 0x2) && (timeout > 0)) {
        ise_time_wait(1);
        timeout--;
    }
    if(timeout <= 0) {
        printf("EHCI controller did not come out of reset!\n");
        return false;
    }
    
    // Check if HC supports Async schedule park, and see if
    // the command register is reset appropriately
    if(ise_ehci->cap[ISE_EHCI_CAP_HCCPARAMS] & 0x4) {
        if(ise_ehci->op[ISE_EHCI_OP_USBCMD] != 0x00080B00) {
            printf("EHCI command register not reset correctly for Async Park mode 0x%x\n", ise_ehci->op[ISE_EHCI_OP_USBCMD]);
            return false;
        }
    } else {
        if(ise_ehci->op[ISE_EHCI_OP_USBCMD] != 0x00080000) {
            printf("EHCI command register not reset correctly for non-Async Park mode 0x%x\n", ise_ehci->op[ISE_EHCI_OP_USBCMD]);
            return false;
        }
    }
    // Check reste values of other operational registers
    if(ise_ehci->op[ISE_EHCI_OP_USBSTS] != 0x1000) {
        printf("EHCI status register not reset correctly: 0x%x\n", ise_ehci->op[ISE_EHCI_OP_USBSTS]);
        return false;
    }
    if(ise_ehci->op[ISE_EHCI_OP_USBINTR] != 0x0) {
        printf("EHCI interrupt register not reset correctly: 0x%x\n", ise_ehci->op[ISE_EHCI_OP_USBINTR]);
        return false;
    }
    if(ise_ehci->op[ISE_EHCI_OP_FRINDEX] != 0x0) {
        printf("EHCI Frame index register not reset correctly: 0x%x\n", ise_ehci->op[ISE_EHCI_OP_FRINDEX]);
        return false;
    }
    if(ise_ehci->op[ISE_EHCI_OP_CTRLDSSEGMENT] != 0x0) {
        printf("EHCI Control DS Segment register not reset correctly: 0x%x\n", ise_ehci->op[ISE_EHCI_OP_CTRLDSSEGMENT]);
        return false;
    }
    if(ise_ehci->op[ISE_EHCI_OP_CONFIGFLAG] != 0x0) {
        printf("EHCI Config flag register not reset correctly: 0x%x\n", ise_ehci->op[ISE_EHCI_OP_CONFIGFLAG]);
        return false;
    }

    return true;
}

// allocate initial resources; returns true if success
bool ise_ehci_init_resources(ise_usb_hc_t* ise_usb)
{
    ise_ehci_t* ise_ehci = (ise_ehci_t*) ise_usb->hcd;
    
    // clear the memory pools
    int p, i;
    for(p=0; p<ISE_EHCI_MAX_MEM_POOL; p++) {
        for(i=0; i<8; i++) {
            ise_ehci->mem_alloc[p][i].i32 = 0x0;
        }
        ise_ehci->mem_pool[p] = NULL;
    }
    
    // allocate the first 8KB memory pool, 32B aligned
    ise_ehci->mem_pool[0] = (uint32_t*) ise_mem_aligned_malloc(0x2000, 5, ISE_MEM_REGION_DOS);
    if(ise_ehci->mem_pool[0] == NULL) return false;
    
    // Find a 4KB aligned region in the first memory pool
    // this will be used for the periodic list, so we need to set the bits
    // corresponding to that region as used.
    uint32_t pool_addr = (uint32_t) ise_ehci->mem_pool[0];
    uint32_t periodic_list_addr = (pool_addr + 0x1000) & ~0xFFFL;
    // Get the offset, in 32B units
    uint32_t periodic_list_offset = (periodic_list_addr - pool_addr) >> 5;
    for(i=periodic_list_offset; i<periodic_list_offset+128; i++) {
        ise_ehci->mem_alloc[0][i >> 5].i32 |= 1 << (i & 0x1f);
    }
    ise_ehci->periodic_list = (uint32_t*) periodic_list_addr;

    // the config buffer needs CONFIG_BUFFER size, so divide by 32 to get consecutive locations
    // if the periodic list offset is over this, we can just allocate the first of these locations
    // otherwise, the locations after the framelist
    uint32_t num_config_buffer_locations = ISE_USB_CONFIG_BUFFER_SIZE / 32;
    uint32_t config_buffer_offset = (periodic_list_offset >= num_config_buffer_locations) ? 0 : periodic_list_offset + 128;
    for(i=config_buffer_offset; i<config_buffer_offset+num_config_buffer_locations; i++) {
        ise_ehci->mem_alloc[0][i >> 5].i32 |= 1 << (i & 0x1f);
    }
    ise_usb->config_buffer = (uint8_t*) (pool_addr + (config_buffer_offset << 5));
    ise_usb->config_req_packet = (ise_usb_device_request_packet_t*) ise_ehci_alloc_mem(ise_usb->hcd, 8);

    // allocate, initialize and link the primary queues
    // qN -> q1, where N != 1
    ise_ehci->control_queue = (ise_ehci_queue_t*) ise_ehci_alloc_queue(ise_usb->hcd);
    ise_ehci->bulk_queue = (ise_ehci_queue_t*) ise_ehci_alloc_queue(ise_usb->hcd);
    ise_ehci_link_queues(ise_ehci->bulk_queue, ise_ehci->control_queue);
    printf("bq: 0x%x cq: 0x%x\n", ise_ehci->bulk_queue, ise_ehci->control_queue);
    // link control queue back to bulk queue
    ise_ehci_link_queues(ise_ehci->control_queue, ise_ehci->bulk_queue);
    // mark bulk queue as head of reclamation list
    ise_ehci->bulk_queue->device |= 0x8000;
    ise_ehci_queue_t* periodic_queue[8];
    for(i=0; i<8; i++) {
        periodic_queue[i] = (ise_ehci_queue_t*) ise_ehci_alloc_queue(ise_usb->hcd);
        if(i != 0) {
            ise_ehci_link_queues(periodic_queue[i], periodic_queue[0]);
        }
    }

    // allocate dummy td
    ise_ehci->dummy_td = NULL;  // Need the dummy's alt_next to point to NULL
    ise_ehci->dummy_td = (ise_ehci_qtd64_t*) ise_ehci_alloc_td(ise_usb->hcd);

    // initialize the framelist pointers
    // this pattern goes for 128 entries, and then repeats
    //  fl[0] = q1
    //  fl[1] = q2 -> q1
    //  fl[2] = q4 -> q1
    //  fl[3] = q2 -> q1
    //  fl[4] = q8 -> q1
    //  fl[5] = q2 -> q1
    //  fl[6] = q4 -> q1
    //  fl[7] = q2 -> q1
    //  fl[8] = q16-> q1
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
        ise_ehci->periodic_list[i] = ((uint32_t) periodic_queue[p]) | 0x2;
    }

    return true;
}

void ise_ehci_debug(FILE* file, ise_usb_hc_t* ise_usb)
{
    ise_ehci_t* ise_ehci = (ise_ehci_t*) ise_usb->hcd;
    if(file == NULL) file = stdout;
    fprintf(file, "command 0x%x ", ise_ehci->op[ISE_EHCI_OP_USBCMD]);
    fprintf(file, "status 0x%x ", ise_ehci->op[ISE_EHCI_OP_USBSTS]);
    fprintf(file, "pbase 0x%x ", ise_ehci->op[ISE_EHCI_OP_PERIODICLISTBASE]);
    fprintf(file, "abase 0x%x ", ise_ehci->op[ISE_EHCI_OP_ASYNCLISTADDR]);
    fprintf(file, "fr 0x%x\n", ise_ehci->op[ISE_EHCI_OP_FRINDEX]);
    ise_mem_dump(file, ise_ehci->periodic_list, 4);
    ise_mem_dump(file, ise_ehci->bulk_queue, 20);
    ise_mem_dump(file, ise_ehci->control_queue, 20);
    //ise_ehci_queue_t* next_queue = (ise_ehci_queue_t*) (ise_ehci->control_queue->head_link & ~0xFL);
    //ise_mem_dump(file, next_queue, 20);
}

// enable the device
void ise_ehci_enable(ise_ehci_t* ise_ehci)
{
    // Disable interrupts
    ise_ehci->op[ISE_EHCI_OP_USBINTR] = 0x0;
    
    // Reset frame number
    ise_ehci->op[ISE_EHCI_OP_FRINDEX] = 0x0;
    
    // Set the high dword of 64b address
    ise_ehci->op[ISE_EHCI_OP_CTRLDSSEGMENT] = 0x0;
    
    // Set the periodic and async list base
    ise_ehci->op[ISE_EHCI_OP_PERIODICLISTBASE] = (uint32_t) ise_ehci->periodic_list;
    ise_ehci->op[ISE_EHCI_OP_ASYNCLISTADDR] = (uint32_t) ise_ehci->bulk_queue;
    
    // Clear status bits
    ise_ehci->op[ISE_EHCI_OP_USBSTS] = 0x3F;
    
    uint32_t hcsparams = ise_ehci->cap[ISE_EHCI_CAP_HCSPARAMS];
    if(hcsparams & 0x10) {
        // need to explicitly power the ports
        int p;
        for(p=0; p<ise_ehci->num_ports; p++) {
            int port_index = ISE_EHCI_OP_PORTSC + p;
            ise_ehci->op[port_index] |= 0x1000;
        }
    }

    // Set interrupts to fire no more than 8 uframes (1ms)
    // Enable periodic and async lists
    // Set to indicate 1024 frames in periodic list
    // set the run bit
    ise_ehci->op[ISE_EHCI_OP_USBCMD] = 0x80031;
    
    // Set configure flag, so controller takes ownership of all ports
    ise_ehci->op[ISE_EHCI_OP_CONFIGFLAG] = 0x1;
}

// free resources
void ise_ehci_free_resources(ise_ehci_t* ise_ehci)
{
    int p;
    for(p=0; p<ISE_EHCI_MAX_MEM_POOL; p++) {
        if(ise_ehci->mem_pool[p]) {
            ise_mem_aligned_free(ise_ehci->mem_pool[p]);
            ise_ehci->mem_pool[p] = NULL;
        }
    }
}

// basic setup of ehci controller
bool ise_ehci_setup(ise_usb_hc_t* ise_usb)
{
    ise_ehci_t* ise_ehci = (ise_ehci_t*) ise_usb->hcd;

    // Get base address (should be in MMIO space)
    void* paddr = (void*) ise_pci_read_config(0, ise_usb->pci_slot, ise_usb->pci_func, 0x10);
    if(((uint32_t) paddr) & 1) {
        printf("EHCI device using IO space - not supported\n");
        return false;
    }
    ise_ehci->cap = (uint32_t*) ise_mem_map(paddr, 0x1000);

    // Get operational registers
    uint32_t version = ise_ehci->cap[ISE_EHCI_CAP_HCIVERSION];
    ise_ehci->op = ise_ehci->cap + ((version & 0xFF) >> 2);
    ise_ehci->s_mask_offset = 0;

    // check if another driver has already been installed
    //if(ise_ehci->op[ISE_EHCI_OP_CONFIGFLAG] & 0x1) {
    //    printf("EHCI device on slot/func 0x%x/0x%x - another driver already loaded\n", ise_usb->pci_slot, ise_usb->pci_func);
    //    return false;
    //}
    printf("Warning: ignoring driver check!!!!!!!!!!!\n");

    // Get the interrupt number
    ise_usb->interrupt_num = ise_pci_read_config(0, ise_usb->pci_slot, ise_usb->pci_func, 0x3C) & 0xFF;
    if(ise_usb->interrupt_num < 3 || ise_usb->interrupt_num > 0xF) {
        // in Dos, should only have interrupts between 0x3 and 0xF...
        printf("EHCI device on slot/func 0x%x/0x%x - bad interrupt number: 0x%x\n", ise_usb->pci_slot, ise_usb->pci_func, ise_usb->interrupt_num);
        return false;
    }

    // Reset and see if it succeeds
    if(!ise_ehci_reset(ise_ehci)) {
        printf("EHCI device on slot/func 0x%x/0x%x - reset failed\n", ise_usb->pci_slot, ise_usb->pci_func);
        return false;
    }

    // allocate resources
    if(!ise_ehci_init_resources(ise_usb)) {
        printf("EHCI device on slot/func 0x%x/0x%x - could not allocate memory pool\n", ise_usb->pci_slot, ise_usb->pci_func);
        return false;
    }
    
    // Determine number of ports
    uint32_t hcsparams = ise_ehci->cap[ISE_EHCI_CAP_HCSPARAMS];
    ise_ehci->num_ports = (uint8_t) hcsparams & 0xF;

    // store flags
    uint32_t hccparams = ise_ehci->cap[ISE_EHCI_CAP_HCCPARAMS];
    ise_ehci->flags = (uint8_t) (hccparams & 0xFF);
    
    return true;
}

void ise_ehci_install()
{
    printf("installing ehci...\n");
    int s, f;
    for(s=0; s<ISE_PCI_MAX_SLOTS && ise_usb_num_hosts < ISE_USB_MAX_HC; s++) {
        for(f=0; f<ISE_PCI_MAX_FUNC && ise_usb_num_hosts < ISE_USB_MAX_HC; f++) {
            if(ise_pci.slot[s][f].vendor_id != ISE_PCI_VENDOR_INVALID && ise_pci.slot[s][f].class_code == ISE_EHCI_PCI_CLASS) {
                ise_usb[ise_usb_num_hosts].pci_slot = s;
                ise_usb[ise_usb_num_hosts].pci_func = f;
                ise_pci.slot[s][f].priv = (void*) &ise_usb[ise_usb_num_hosts];

                // allocate ehci data
                ise_usb[ise_usb_num_hosts].hcd = (ise_usb_hcd_t*) malloc(sizeof(ise_ehci_t));
                if(ise_usb[ise_usb_num_hosts].hcd) {
                    
                    // Setup device, and if succesful, count it
                    ise_ehci_t* ise_ehci = (ise_ehci_t*) ise_usb[ise_usb_num_hosts].hcd;
                    memset(ise_ehci, 0, sizeof(ise_ehci_t));
                    if(ise_ehci_setup(&ise_usb[ise_usb_num_hosts])) {

                        // display version
                        printf("cap base addr 0x%x\n", (uint32_t) ise_ehci->cap);
                        uint32_t version = ise_ehci->cap[ISE_EHCI_CAP_HCIVERSION];
                        printf("op  base addr 0x%x\n", (uint32_t) ise_ehci->op);
                        printf("ehci device on slot/func 0x%x/0x%x...version 0x%x num_ports 0x%x\n", s, f, version, ise_ehci->num_ports);

                        // allocate root hub
                        ise_usb[ise_usb_num_hosts].device[0] = (ise_usb_device_t*) malloc(sizeof(ise_usb_hub_t));
                        memset(ise_usb[ise_usb_num_hosts].device[0], 0, sizeof(ise_usb_hub_t));
                        ise_usb_hub_t* root_hub = (ise_usb_hub_t*) ise_usb[ise_usb_num_hosts].device[0];
                        root_hub->num_ports = ise_ehci->num_ports;
                        
                        root_hub->port_changed = ise_ehci_port_changed;
                        root_hub->reset_port = ise_ehci_reset_port;
                        root_hub->disable_port = ise_ehci_disable_port;
                        ise_usb[ise_usb_num_hosts].alloc_mem = ise_ehci_alloc_mem;
                        ise_usb[ise_usb_num_hosts].free_mem = ise_ehci_free_mem;
                        ise_usb[ise_usb_num_hosts].alloc_queue = ise_ehci_alloc_queue;
                        ise_usb[ise_usb_num_hosts].free_queue = ise_ehci_free_queue;
                        ise_usb[ise_usb_num_hosts].alloc_td = ise_ehci_alloc_td;
                        ise_usb[ise_usb_num_hosts].free_td = ise_ehci_free_td;
                        ise_usb[ise_usb_num_hosts].get_queue = ise_ehci_get_queue;
                        ise_usb[ise_usb_num_hosts].fill_qh = ise_ehci_fill_qh;
                        ise_usb[ise_usb_num_hosts].fill_td = ise_ehci_fill_td;
                        ise_usb[ise_usb_num_hosts].push_queue = ise_ehci_push_queue;
                        ise_usb[ise_usb_num_hosts].enqueue_queue = ise_ehci_enqueue_queue;
                        ise_usb[ise_usb_num_hosts].dequeue_queue = ise_ehci_dequeue_queue;
                        ise_usb[ise_usb_num_hosts].push_td = ise_ehci_push_td;
                        ise_usb[ise_usb_num_hosts].enqueue_td = ise_ehci_enqueue_td;
                        ise_usb[ise_usb_num_hosts].is_td_done = ise_ehci_is_td_done;
                        ise_usb[ise_usb_num_hosts].is_td_fatal = ise_ehci_is_td_fatal;
                        ise_usb[ise_usb_num_hosts].is_td_stalled = ise_ehci_is_td_stalled;
                        ise_usb[ise_usb_num_hosts].is_td_short = ise_ehci_is_td_short;
                        ise_usb[ise_usb_num_hosts].activate_td = ise_ehci_activate_td;
                        ise_usb[ise_usb_num_hosts].deactivate_td = ise_ehci_deactivate_td;
                        ise_usb[ise_usb_num_hosts].clear_td_status = ise_ehci_clear_td_status;
                        ise_usb[ise_usb_num_hosts].restart_td = ise_ehci_restart_td;
                        ise_usb[ise_usb_num_hosts].dequeue_completed_tds = ise_ehci_dequeue_completed_tds;
                        ise_usb[ise_usb_num_hosts].dequeue_td = ise_ehci_dequeue_td;
                        ise_usb[ise_usb_num_hosts].restart_queue = ise_ehci_restart_queue;
                        ise_usb[ise_usb_num_hosts].start_queue = ise_ehci_start_queue;
                        ise_usb[ise_usb_num_hosts].stop_queue = ise_ehci_stop_queue;
                        ise_usb[ise_usb_num_hosts].clear_queue = ise_ehci_clear_queue;
                        ise_usb[ise_usb_num_hosts].is_queue_done = ise_ehci_is_queue_done;
                        ise_usb[ise_usb_num_hosts].is_queue_fatal = ise_ehci_is_queue_fatal;

                        //ise_usb[ise_usb_num_hosts].device[0] = NULL;
                        ////ise_usb[ise_usb_num_hosts].port_changed = NULL;
                        ////ise_usb[ise_usb_num_hosts].reset_port = NULL;
                        //ise_usb[ise_usb_num_hosts].alloc_queue = NULL;
                        //ise_usb[ise_usb_num_hosts].free_queue = NULL;
                        //ise_usb[ise_usb_num_hosts].alloc_td = NULL;
                        //ise_usb[ise_usb_num_hosts].free_td = NULL;
                        //ise_usb[ise_usb_num_hosts].get_queue = NULL;
                        //ise_usb[ise_usb_num_hosts].fill_td = NULL;
                        //ise_usb[ise_usb_num_hosts].push_queue = NULL;
                        //ise_usb[ise_usb_num_hosts].enqueue_queue = NULL;
                        //ise_usb[ise_usb_num_hosts].dequeue_queue = NULL;
                        //ise_usb[ise_usb_num_hosts].push_td = NULL;
                        //ise_usb[ise_usb_num_hosts].enqueue_td = NULL;
                        //ise_usb[ise_usb_num_hosts].is_td_done = NULL;
                        //ise_usb[ise_usb_num_hosts].dequeue_completed_tds = NULL;
                        //ise_usb[ise_usb_num_hosts].restart_queue = NULL;
                        
                        ise_usb_num_hosts++;
                        ise_ehci_enable(ise_ehci);
                    } else {
                        free(ise_usb[ise_usb_num_hosts].hcd);
                        ise_usb[ise_usb_num_hosts].hcd = NULL;
                    }
                }
            }
        }
    }
/*
    if(ise_ehci_num_devices) {
        // if we found any ehci devices...
        // allocate operational registers (4KB) and hcca space (256B) for each device
        uint32_t *ptr = (uint32_t*) ise_mem_aligned_malloc((0x1000 + ISE_OHCI_HCCA_REGS * sizeof(uint32_t)) * ise_ohci_num_devices, 12);
        int h;
        for(h=0; h<ise_ohci_num_devices; h++) {
            ise_ohci[h].reg = ptr + (h*0x1000 >> 2);
            ise_ohci[h].reg[ISE_OHCI_HC_HCCA] = ((uint32_t) ptr) + ise_ohci_num_devices*0x1000 + h*ISE_OHCI_HCCA_REGS;
            ise_pci_write_config32(0, ise_ohci[h].pci_slot, ise_ohci[h].pci_func, 0x10, (uint32_t) ise_ohci[h].reg);

            uint32_t ctrl_status = ise_pci_read_config(0, ise_ohci[h].pci_slot, ise_ohci[h].pci_func, 0x4);
            // enable bus master and memory access to pci device
            //ise_pci_write_config16(0, ise_ohci[h].pci_slot, ise_ohci[h].pci_func, 0x04, 0x6);

            uint32_t revision = ise_ohci[h].reg[ISE_OHCI_HC_REVISION];
            printf("ohci device %d...ctrl/status 0x%x bar 0x%x revision 0x%x\n", h, ctrl_status, (uint32_t) ise_ohci[h].reg, revision);
        }
    }
*/
}

void ise_ehci_uninstall()
{
    int h;
    for(h=0; h<ise_usb_num_hosts; h++) {
        if(ise_pci.slot[ise_usb[h].pci_slot][ise_usb[h].pci_func].class_code == ISE_EHCI_PCI_CLASS) {
            ise_ehci_t* ise_ehci = (ise_ehci_t*) ise_usb[h].hcd;
            if(ise_ehci) {
                ise_ehci_reset(ise_ehci);
                ise_ehci_free_resources(ise_ehci);
                ise_mem_unmap( (void*) ise_ehci->cap );
                free(ise_ehci);
                ise_usb[h].hcd = NULL;
            }
            ise_pci.slot[ise_usb[h].pci_slot][ise_usb[h].pci_func].priv = NULL;
        }
    }
}
