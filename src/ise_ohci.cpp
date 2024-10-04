// InfiniScroll Engine
// ise_ohci.h
// Open Host Controller Interface
//
// Kamal Pillai
// 2/1//2019

#include "ise.h"

// --------------------------------------------------------------------------------------------
// USB driver calls

// allocates size in bytes of 16B entries from memory pool
void* ise_ohci_alloc_mem(ise_usb_hcd_t* _hcd, int size)
{
    int num_entries = (size + 15) / 16; // divide by 16, but round up

    // only support up to 32 entries (512 bytes)
    if(num_entries <= 0 || num_entries > 32) return NULL;

    ise_ohci_t* ise_ohci = (ise_ohci_t*) _hcd;
    //uint32_t size_mask = (num_entries==32) ? ~0L : (1L << num_entries) - 1;
    void* entry = NULL;
    int offset;
    int p;
    //int i, j;
    // Loop through each pool of memory for an empty slot
    for(p=0; p<ISE_OHCI_MAX_MEM_POOL && entry == NULL; p++) {
        // if we need to use this pool, and it hasn't been allocated, do so now
        if(ise_ohci->mem_pool[p] == NULL) {
            // allocate 8KB, 16B aligned
            ise_ohci->mem_pool[p] = (uint32_t*) ise_mem_aligned_malloc(0x2000, 4, ISE_MEM_REGION_DOS);
            // if we couldn't allocate the pool, fail
            if(ise_ohci->mem_pool[p] == NULL) return NULL;
        }
        offset = ise_mem_alloc_entries(num_entries, ise_ohci->mem_alloc[p], 16);
        if(offset >= 0) {
            // scale offset to account for 4 DWORDS per entry
            offset = offset << 2;
            entry = ise_ohci->mem_pool[p] + offset;
        }
    }
    return entry;
}

// frees size in bytes of 16B entries back to memory pool
void ise_ohci_free_mem(ise_usb_hcd_t* _hcd, void* entry, int size)
{
    int num_entries = (size + 15) / 16; // divide by 16, but round up
    // only support up to 32 entries (512 Bytes)
    if(num_entries <= 0 || num_entries > 32) return;

    ise_ohci_t* ise_ohci = (ise_ohci_t*) _hcd;
    //uint32_t size_mask = (num_entries==32) ? ~0L : (1L << num_entries) - 1;
    uint32_t entry_addr = (uint32_t) entry;
    uint32_t mem_pool_addr;
    uint32_t offset;
    bool done = false;
    int p;
    // Loop through each memory to pool to find which pool the address belongs to
    for(p=0; p<ISE_OHCI_MAX_MEM_POOL && !done; p++) {
        mem_pool_addr = (uint32_t) ise_ohci->mem_pool[p];
        // if entry address is within 8KB of the memory pool address,
        // then it belongs to the bool
        // note, offset goes negative, it is treated as a very large positive value
        offset = entry_addr - mem_pool_addr;
        if(offset < 0x2000) {
            // scale offset for 16 Bytes per entry
            offset = offset >> 4;
            done = ise_mem_free_entries(offset, num_entries, ise_ohci->mem_alloc[p], 16);
        }
    }
}

// initialize a queue
void ise_ohci_init_queue(ise_ohci_queue_t* q, ise_ohci_t* ise_ohci)
{
    // mark all pointers as invalid
    q->command = 0x0;
    q->null_pointer = ise_ohci->dummy_tail_td;
    q->current_pointer = ise_ohci->dummy_tail_td;
    q->next_queue = 0x0;
    q->head_pointer = 0x0;
    q->tail_pointer = 0x0;
    q->end_queue = 0x0;
    q->qtype = 0x0;
}

// initialize a transfer descriptor
void ise_ohci_init_td(ise_ohci_td_t* td, ise_ohci_t* ise_ohci)
{
    // clear to 0
    td->command = 0x0;
    td->buffer = 0x0;
    td->next = ise_ohci->dummy_tail_td;
    td->buffer_end = 0x0;
    td->sw_next = ise_ohci->dummy_tail_td;
}

// sets next pointer of q0 to point to q1
void ise_ohci_link_queues(ise_ohci_queue_t* q0, ise_ohci_queue_t* q1)
{
    q0->next_queue = ((uint32_t) q1);
}

// allocate a queue
ise_usb_queue_t* ise_ohci_alloc_queue(ise_usb_hcd_t* _hcd)
{
    ise_ohci_queue_t* q = (ise_ohci_queue_t*) ise_ohci_alloc_mem(_hcd, sizeof(ise_ohci_queue_t));
    ise_ohci_init_queue(q, (ise_ohci_t*) _hcd);
    return (ise_usb_queue_t*) q;
}

// free a queue
void ise_ohci_free_queue(ise_usb_hcd_t* _hcd, ise_usb_queue_t* _q)
{
    uint32_t* q = (uint32_t*) _q;
    ise_ohci_free_mem(_hcd, q, sizeof(ise_ohci_queue_t));
}

// allocate a transfer descriptor
ise_usb_td_t* ise_ohci_alloc_td(ise_usb_hcd_t* _hcd)
{
    ise_ohci_td_t* td = (ise_ohci_td_t*) ise_ohci_alloc_mem(_hcd, sizeof(ise_ohci_td_t));
    ise_ohci_init_td(td, (ise_ohci_t*) _hcd);
    return (ise_usb_td_t*) td;
}

// free a transfer descriptor
void ise_ohci_free_td(ise_usb_hcd_t* _hcd, ise_usb_td_t* _td)
{
    uint32_t* td = (uint32_t*) _td;
    ise_ohci_free_mem(_hcd, td, sizeof(ise_ohci_td_t));
}

// get a primary queue
ise_usb_queue_t* ise_ohci_get_queue(ise_usb_hcd_t* _hcd, int q_id)
{
    ise_ohci_t* ise_ohci = (ise_ohci_t*) _hcd;
    ise_usb_queue_t* q = NULL;
    // cap to 32ms
    if(q_id > 5 && q_id < ISE_USB_QUEUE_BULK) q_id = 5;

    if(q_id < 8) {
        int index = (q_id == 0) ? 0 : (1 << (q_id-1));
        uint32_t q_ptr = ise_ohci->hcca[index];
        q = (ise_usb_queue_t*) (q_ptr & ~0xFL);
    } else {
        switch(q_id) {
            case ISE_USB_QUEUE_BULK:
                q = (ise_usb_queue_t*) ise_ohci->bulk_queue;
                break;
            case ISE_USB_QUEUE_CONTROL:
                q = (ise_usb_queue_t*) ise_ohci->control_queue;
                break;
        }
    }

    return q;
}

// fill endpoint descriptor
void ise_ohci_fill_qh(ise_usb_queue_t* _q, ise_usb_hc_t* ise_usb, uint8_t device_id, uint8_t end_pt)
{
    ise_ohci_queue_t* q = (ise_ohci_queue_t*) _q;
    ise_ohci_t* ise_ohci = (ise_ohci_t*) ise_usb->hcd;;
    ise_usb_device_t* device = ise_usb->device[device_id];
    if(device == NULL) return;
    uint8_t end_pt_type = device->endpoint[end_pt].attributes & 0x3;
    int max_packet = device->endpoint[end_pt].max_packet_size & 0x7FF;
    if(max_packet < 8) max_packet = 8; // Should be at least 8
    
    q->command = device_id & 0x7F;
    q->command |= (end_pt & 0xF) << 7;
    q->command |= (device->speed == ISE_USB_SPEED_LOW) ? 0x2000 : 0x0;
    q->command |= (end_pt_type == 0x1) ? 0x8000 : 0x0;  // iso needs format bit set
    q->command |= (max_packet & 0x7FF) << 16;
    q->qtype = end_pt_type & 0x3;
}

// fills out an already allocated transfer descriptor
void ise_ohci_fill_td(ise_usb_td_t* _td, ise_usb_td_info_t* td_info)
{
    ise_ohci_td_t* td = (ise_ohci_td_t*) _td;

    td->buffer = ise_mem_linear_to_pa((uint32_t) td_info->buffer);
    td->buffer_end = ise_mem_linear_to_pa(((uint32_t) td_info->buffer) + td_info->length - 1);
    //td->command = 0x40000;
    td->command = (td_info->pid == ISE_USB_PID_SETUP) ? 0x00000 :  // encoded pid
        ((td_info->pid == ISE_USB_PID_IN) ? 0x140000 :
        ((td_info->pid == ISE_USB_PID_OUT) ? 0x80000 : 0x180000));
    td->command |= (td_info->ioc) ? 0x0 : 0xE00000;
    td->command |= (td_info->dt) ? 0x3000000 : 0x2000000;
    td->command |= 0xE0000000;  // set completion code to not accessed
}

// adds q1 to head of the q0
void ise_ohci_push_queue(ise_usb_queue_t* _q0, ise_usb_queue_t* _q1)
{
    ise_ohci_queue_t* q0 = (ise_ohci_queue_t*) _q0;
    ise_ohci_queue_t* q1 = (ise_ohci_queue_t*) _q1;
    
    // set the next pointer of q1 to the current head of q0
    q1->next_queue = q0->next_queue;
    // q0 now points to q1
    q0->next_queue = (uint32_t) q1;
    // if end is NULL, which means the queue was empty
    // point the end to start
    if(q0->end_queue == 0x0) q0->end_queue = q0->next_queue;
}

// adds q1 to tail of q0
void ise_ohci_enqueue_queue(ise_usb_queue_t* _q0, ise_usb_queue_t* _q1)
{
    ise_ohci_queue_t* q0 = (ise_ohci_queue_t*) _q0;
    ise_ohci_queue_t* q1 = (ise_ohci_queue_t*) _q1;

    // if end is null, then this queue has nothing in it
    // treat q0 as the tail
    ise_ohci_queue_t* q_tail = (q0->end_queue == 0x0) ? q0 : (ise_ohci_queue_t*) (q0->end_queue);
        
    q1->next_queue = q_tail->next_queue;
    q_tail->next_queue = (uint32_t) q1;
    q0->end_queue = q_tail->next_queue;
}

// finds and removes q1 from q0
// returns true if removed
bool ise_ohci_dequeue_queue(ise_usb_queue_t* _q0, ise_usb_queue_t* _q1)
{
    ise_ohci_queue_t* q0 = (ise_ohci_queue_t*) _q0;
    ise_ohci_queue_t* q1 = (ise_ohci_queue_t*) _q1;
    uint32_t q1_val = (uint32_t) _q1;

    uint32_t* next_ptr = &(q0->next_queue);
    bool removed = false;
    bool done = (*next_ptr == 0x0) ? true : false;
    while(!done) {
        // if we are pointing to the end, then this is the last item to look at
        done = (*next_ptr == q0->end_queue) ? true : false;
        if((*next_ptr) == (q1_val)) {
            // we found the queue
            *next_ptr = q1->next_queue;
            //if end points to the one we're removing, make it point to one prior
            if((q0->end_queue) == (q1_val)) {
                // if prior is the head, then queue is empty
                q0->end_queue = (next_ptr == &(q0->next_queue)) ? 0x0 : ((uint32_t) next_ptr);
            }
            removed = true;
            done = true;
        } else {
            // not found, so move on
            next_ptr = (uint32_t*) (*next_ptr);
        }
        done |= (*next_ptr == 0x0) ? true : false;
    }
    return removed;
}

// adds td to head of q0
void ise_ohci_push_td(ise_usb_queue_t* _q0, ise_usb_td_t* _td)
{
    ise_ohci_queue_t* q0 = (ise_ohci_queue_t*) _q0;
    ise_ohci_td_t* td = (ise_ohci_td_t*) _td;
    
    // set the next pointer of td to the head of queue
    td->next = (q0->head_pointer == 0x0) ? q0->null_pointer : q0->head_pointer;
    td->sw_next = td->next;
    // queue's head now points to the newly added td
    q0->head_pointer = (uint32_t) td;
    // if tail is NULL, then queue is empty, so point it to the new td
    if(q0->tail_pointer == 0x0) q0->tail_pointer = q0->head_pointer;
}

// adds td to tail of q0
void ise_ohci_enqueue_td(ise_usb_queue_t* _q0, ise_usb_td_t* _td)
{
    ise_ohci_queue_t* q0 = (ise_ohci_queue_t*) _q0;
    ise_ohci_td_t* td = (ise_ohci_td_t*) _td;
    
    // adding to tail, so set td's next to NULL
    td->next = q0->null_pointer;
    td->sw_next = q0->null_pointer;
    // if queue is currently empty
    if(q0->head_pointer == 0x0) {
        q0->head_pointer = ((uint32_t) td);
        q0->tail_pointer = q0->head_pointer;
    } else {
        // if not empty, set the end's next to point to the new td
        ise_ohci_td_t* end_td = (ise_ohci_td_t*) (q0->tail_pointer);
        end_td->next = ((uint32_t) td);
        end_td->sw_next = ((uint32_t) td);
        q0->tail_pointer = end_td->next;
    }
}

// check if a TD is completed
bool ise_ohci_is_td_done(ise_usb_td_t* _td)
{
    ise_ohci_td_t* td = (ise_ohci_td_t*) _td;
    
    // check completion code is anything other than not accessed
    return ((td->command & 0xE0000000) == 0xE0000000) ? false : true;
}

// check if td has fatal error
bool ise_ohci_is_td_fatal(ise_usb_td_t* _td)
{
    ise_ohci_td_t* td = (ise_ohci_td_t*) _td;

    // Any code other than not accessed or No error is fatal
    uint32_t completion_code = (td->command >> 28);
    return ((completion_code == 0x0) || ((completion_code & 0xE) == 0xE)) ? false : true;
}

// check if stalled
bool ise_ohci_is_td_stalled(ise_usb_td_t* _td)
{
    ise_ohci_td_t* td = (ise_ohci_td_t*) _td;
    
    uint32_t completion_code = (td->command >> 28);
    return (completion_code == ISE_OHCI_CC_STALL) ? true : false;
}

bool ise_ohci_is_td_short(ise_usb_td_t* _td)
{
    ise_ohci_td_t* td = (ise_ohci_td_t*) _td;

    uint32_t completion_code = (td->command >> 28);
    return ((completion_code == ISE_OHCI_CC_DATA_UNDERRUN) ||
            ((completion_code == ISE_OHCI_CC_NO_ERROR) && (td->buffer != 0x0))) ? true : false;
}

// activate TD
void ise_ohci_activate_td(ise_usb_td_t* _td)
{
    ise_ohci_td_t* td = (ise_ohci_td_t*) _td;
    
    // set completion code to no access
    td->command |= 0xE0000000;
}    

// deactiveate TD
void ise_ohci_deactivate_td(ise_usb_td_t* _td)
{
    ise_ohci_td_t* td = (ise_ohci_td_t*) _td;

    // clear completion code so it's set to no error
    td->command &= ~0xF0000000;    
}

// clear any status btis in TD - sets back to not accessed
void ise_ohci_clear_td_status(ise_usb_td_t* _td)
{
    ise_ohci_td_t* td = (ise_ohci_td_t*) _td;
    
    td->command |= 0xE0000000;
}

// restart TD if completed without errors
// returns posive value if restart, otherwise an error
int ise_ohci_restart_td(ise_usb_td_t* _td, uint32_t addr, int len)
{
    ise_ohci_td_t* td = (ise_ohci_td_t*) _td;
    
    // first check that completion code indicates no error
    bool inactive = (td->command & 0xF0000000) == 0x00000000;
    if(inactive) {//} || is_short) {
        bool nak = (td->buffer == ise_mem_linear_to_pa(addr)) ? true : false;
        // set the start buffer
        td->buffer = ise_mem_linear_to_pa(addr);
        // set end buffer
        td->buffer_end = ise_mem_linear_to_pa(addr + len - 1);
		// NULL buffer - return error
		if(td->buffer == 0x0) return -1;
        // activate td
        td->command |= 0xE0000000;
        return (nak) ? 1 : 0;
    } else {
        return -1;
    }
}

// remove completed td's in the queue, and optionally frees them
void ise_ohci_dequeue_completed_tds(ise_usb_hcd_t* _hcd, ise_usb_queue_t* _q0, bool free)
{
    ise_ohci_t* ise_ohci = (ise_ohci_t*) _hcd;
    ise_ohci_queue_t* q0 = (ise_ohci_queue_t*) _q0;
    
    uint32_t next_ptr = q0->head_pointer;
    ise_ohci_td_t* td;
    bool done = (next_ptr == 0x0) ? true : false;
    while(!done) {
        td = (ise_ohci_td_t*) (next_ptr);
        next_ptr = td->sw_next;
        // if the td is acive, we're done
        if((td->command & 0xE0000000) == 0xE0000000) {
            done = true;
        } else {
            // if not active, free it, or re-initialize it
            if(free) ise_ohci_free_td(_hcd, (ise_usb_td_t*) td);
            else ise_ohci_init_td(td, (ise_ohci_t*) _hcd);
        }
        done |= (next_ptr == 0x0 || next_ptr == q0->null_pointer) ? true : false;
    }
    // set the head ptr
    q0->head_pointer = (next_ptr == q0->null_pointer) ? 0x0 : next_ptr;
    // if head is NULL, point tail to NULL
    if(q0->head_pointer == 0x0) q0->tail_pointer = 0x0;
}

// find and remove a particular td from a queue
// returns true if td removed
bool ise_ohci_dequeue_td(ise_usb_queue_t* _q0, ise_usb_td_t* _td)
{
    ise_ohci_queue_t* q0 = (ise_ohci_queue_t*) _q0;
    ise_ohci_td_t* td = (ise_ohci_td_t*) _td;
    uint32_t td_val = (uint32_t) _td;

    uint32_t* next_ptr = &(q0->head_pointer);
    bool removed = false;
    bool done = (*next_ptr == 0x0) ? true : false;
    while(!done) {
        // check if this is the td we are looking for
        if((*next_ptr) == (td_val)) {
            // td found
            *next_ptr = td->sw_next;
            // if the current points to the one we're removing, advance it
            if((q0->current_pointer) == (td_val)) {
                q0->current_pointer = td->sw_next;
            }
            removed = true;
            done = true;
        } else {
            // not found, so move on
            ise_ohci_td_t* next_td = (ise_ohci_td_t*) next_ptr;
            next_ptr = (uint32_t*) &(next_td->sw_next);
        }
        done |= (*next_ptr == 0x0 || *next_ptr == q0->null_pointer) ? true : false;
    }
    // if head is NULL, point tail is NULL
    if(q0->head_pointer == 0x0) q0->tail_pointer = 0x0;
    return removed;
}

// restart a queue
void ise_ohci_restart_queue(ise_usb_hcd_t* _hcd, ise_usb_queue_t* _q)
{
    ise_ohci_t* ise_ohci = (ise_ohci_t*) _hcd;
    ise_ohci_queue_t* q = (ise_ohci_queue_t*) _q;
    // When restarting, regenerate the next pointers
    ise_ohci_td_t* td = (ise_ohci_td_t*) (q->head_pointer);
    while((uint32_t) td != q->null_pointer) {
        td->next = td->sw_next;
        td = (ise_ohci_td_t*) (td->next);
    }
    q->command &= ~0x4000L;
    q->current_pointer = (q->head_pointer == 0x0) ? q->null_pointer : q->head_pointer;
    switch(q->qtype & 0x3) {
    case 0x0:  // Control - set control list filled bit
        ise_ohci->reg[ISE_OHCI_HC_COMMAND_STATUS] |= 0x2;
        break;
    case 0x2:  // Bulk - set bulk list filled bit
        ise_ohci->reg[ISE_OHCI_HC_COMMAND_STATUS] |= 0x4;
        break;
    // Iso (0x1) and Interrupt (0x3) nned not set any bits
    }
}

// start a q queue from where it was stopped
void ise_ohci_start_queue(ise_usb_hcd_t* _hcd, ise_usb_queue_t* _q)
{
    ise_ohci_t* ise_ohci = (ise_ohci_t*) _hcd;
    ise_ohci_queue_t* q = (ise_ohci_queue_t*) _q;

    q->command &= ~0x4000L;
    switch(q->qtype & 0x3) {
    case 0x0:  // Control - set control list filled bit
        ise_ohci->reg[ISE_OHCI_HC_COMMAND_STATUS] |= 0x2;
        break;
    case 0x2:  // Bulk - set bulk list filled bit
        ise_ohci->reg[ISE_OHCI_HC_COMMAND_STATUS] |= 0x4;
        break;
    // Iso (0x1) and Interrupt (0x3) nned not set any bits
    }
}

// stop a q queue
void ise_ohci_stop_queue(ise_usb_queue_t* _q)
{
    ise_ohci_queue_t* q = (ise_ohci_queue_t*) _q;
    q->command |= 0x4000L;
}

// clear everything out of a queue
void ise_ohci_clear_queue(ise_usb_queue_t* _q)
{
    ise_ohci_queue_t* q = (ise_ohci_queue_t*) _q;
    q->current_pointer = 0x0;
    q->head_pointer = 0x0;
    q->tail_pointer = 0x0;
}

// check if a queue is done with all work
bool ise_ohci_is_queue_done(ise_usb_queue_t* _q)
{
    ise_ohci_queue_t* q = (ise_ohci_queue_t*) _q;
    // check if the next td is null
    bool done = (q->current_pointer == 0x0) ? true : false;
    if(q->current_pointer == (q->tail_pointer)) {
        ise_ohci_td_t* td = (ise_ohci_td_t*) (q->current_pointer);
        int completion_code = (td->command >> 28);
        if((completion_code & 0xE) != 0xE) done = true;
    }
    return done;
}

// check if queue has fatal error
bool ise_ohci_is_queue_fatal(ise_usb_queue_t* _q)
{
    ise_ohci_queue_t* q = (ise_ohci_queue_t*) _q;
    // we are fatal if the halted bit is set
    bool fatal = (q->current_pointer & 0x1) ? true : false;
    return fatal;
}

// check for port status change
// returns -1 if port does not change
// otherwise, bit[2:0] is the speed of the device
// bit [7] indicates a connected if 1, disconnected if 0
// this function should be called perioidically
int ise_ohci_port_changed(ise_usb_hc_t* ise_usb, int device_id, int port)
{
    ise_ohci_t* ise_ohci = (ise_ohci_t*) (ise_usb->hcd);
    if(device_id != 0 || port >= ise_ohci->num_ports) return -1;
    
    int port_index = ISE_OHCI_HC_RH_PORT_STATUS + port;
    uint32_t port_data = ise_ohci->reg[port_index];
    // check if port connection or port enabled changed
    if(port_data & 0x30000) {
        printf("ohci port %d changed: 0x%x\n", port, port_data);
        // clear port status change bits
        ise_ohci->reg[port_index] = 0x1F0000;
        // check if port is connected, and whether it is low speed
        return ((port_data & 0x1) ? 0x80 : 0x0) | ((port_data & 0x200) ? ISE_USB_SPEED_LOW : ISE_USB_SPEED_FULL);
    }
    // no change
    return -1;
}

// reset and enable port
// returns port status (same as port_changed function) on  success
// returns -1 if reset fails
int ise_ohci_reset_port(ise_usb_hc_t* ise_usb, int device_id, int port)
{
    ise_ohci_t* ise_ohci = (ise_ohci_t*) (ise_usb->hcd);
    if(device_id != 0 || port >= ise_ohci->num_ports) return -1;

    int port_index = ISE_OHCI_HC_RH_PORT_STATUS + port;
    uint32_t port_data = ise_ohci->reg[port_index];
    // check if something is connected before resetting
    if(port_data & 0x1) {
        // reset port
        ise_ohci->reg[port_index] |= 0x10;
        int i;
        for(i=0; i<10; i++) {
            ise_time_wait(50);
            port_data = ise_ohci->reg[port_index];
            // if connect status changed, and no device connected, return 0
            if((port_data & 0x110001) == 0x010000) {
                // Clear change status
                ise_ohci->reg[port_index] = 0x1F0000;
                return 0;
            }
            // if port connected and enabled, return connection, and port speed
            if((port_data & 0x100003) == 0x100003) {
                // Clear change status
                printf("ohci reset port %d: 0x%x\n", port, port_data);
                ise_ohci->reg[port_index] = 0x1F0000;
                return 0x80 | ((port_data & 0x200) ? ISE_USB_SPEED_LOW : ISE_USB_SPEED_FULL);
            }
        }
        // reset failed
        return -1;
    }

    // return that nothing is connected
    return 0;
}

void ise_ohci_disable_port(ise_usb_hc_t* ise_usb, int device_id, int port)
{
    ise_ohci_t* ise_ohci = (ise_ohci_t*) (ise_usb->hcd);
    if(device_id != 0 || port >= ise_ohci->num_ports) return;
    
    int port_index = ISE_OHCI_HC_RH_PORT_STATUS + port;
    ise_ohci->reg[port_index] |= 0x1L;
}

// --------------------------------------------------------------------------------------------
// Initialization routines
// reset ohci controller; returns true if all goes well
bool ise_ohci_reset(ise_ohci_t* ise_ohci)
{
    // Save FM interval before resetting the controller
    uint32_t fm_interval = ise_ohci->reg[ISE_OHCI_HC_FM_INTERVAL];
    
    // perform reset
    ise_ohci->reg[ISE_OHCI_HC_COMMAND_STATUS] |= 0x1;
    ise_time_uwait(30);
    // make sure we're out of reset
    if(ise_ohci->reg[ISE_OHCI_HC_COMMAND_STATUS] & 0x1 == 0x1) {
        printf("OHCI host controller not out of reset\n");
        return false;
    }
    // check two more registers for reset values
    if((ise_ohci->reg[ISE_OHCI_HC_CONTROL] & 0xC0) != 0xC0) {
        printf("OHCI host did not correctly reset registers");
        return false;
    }
    if((ise_ohci->reg[ISE_OHCI_HC_FM_INTERVAL] & 0x3FFF) != 0x2EDF) {
        printf("OCHI host did not correctly reset registers");
        return false;
    }
    
    // restore fm interval
    ise_ohci->reg[ISE_OHCI_HC_FM_INTERVAL] = fm_interval;
    return true;
}

// allocate initial resources; returns true if success
bool ise_ohci_init_resources(ise_usb_hc_t* ise_usb)
{
    ise_ohci_t* ise_ohci = (ise_ohci_t*) ise_usb->hcd;
    
    // get alignment requirements by filling 1's to HCCA registers,
    // and reading back where's 0's are
    ise_ohci->reg[ISE_OHCI_HC_HCCA] = 0xFFFFFFFF;
    uint32_t hcca = ise_ohci->reg[ISE_OHCI_HC_HCCA];
    int i;
    // alignment must be at least 256B
    // we don't handle alignments greater than 4 KB
    for(i=8; i<13; i++) {
        if(hcca & (1 << i)) break;
    }
    if(i > 12) {
        printf("OHCI controller has HCCA alignment greater than 4KB - not supported\n");
        return false;
    }
    
    // clear the memory pools
    int p;
    for(p=0; p<ISE_OHCI_MAX_MEM_POOL; p++) {
        for(i=0; i<16; i++) {
            ise_ohci->mem_alloc[p][i].i32 = 0x0;
        }
        ise_ohci->mem_pool[p] = NULL;
    }

    // allocate the first 8KB memory pool, 16B aligned
    ise_ohci->mem_pool[0] = (uint32_t*) ise_mem_aligned_malloc(0x2000, 4, ISE_MEM_REGION_DOS);
    if(ise_ohci->mem_pool[0] == NULL) return false;

    // Find a 4KB aligned region in the first memory pool
    // this will be used for the hcca, so we need to set the bits
    // corresponding to that region as used.
    uint32_t pool_addr = (uint32_t) ise_ohci->mem_pool[0];
    uint32_t hcca_addr = (pool_addr + 0x1000) & ~0xFFFL;
    // Get the offset, in 16B units
    uint32_t hcca_offset = (hcca_addr - pool_addr) >> 4;
    for(i=hcca_offset; i<hcca_offset+16; i++) {
        ise_ohci->mem_alloc[0][i >> 5].i32 |= 1 << (i & 0x1f);
    }
    ise_ohci->hcca = (uint32_t*) hcca_addr;

    // the config buffer needs CONFIG_BUFFER size, so divide by 16 to get consecutive locations
    // if the hcca offset is over this, we can just allocate the first of these locations
    // otherwise, the locations after the hcca
    uint32_t num_config_buffer_locations = ISE_USB_CONFIG_BUFFER_SIZE / 16;
    uint32_t config_buffer_offset = (hcca_offset >= num_config_buffer_locations) ? 0 : hcca_offset + 16;
    for(i=config_buffer_offset; i<config_buffer_offset+num_config_buffer_locations; i++) {
        ise_ohci->mem_alloc[0][i >> 5].i32 |= 1 << (i & 0x1f);
    }
    ise_usb->config_buffer = (uint8_t*) (pool_addr + (config_buffer_offset << 4));
    ise_usb->config_req_packet = (ise_usb_device_request_packet_t*) ise_ohci_alloc_mem(ise_usb->hcd, 8);

    // allocate a dummy tail
    ise_ohci->dummy_tail_td = 0x0;//(uint32_t) ise_ohci_alloc_mem(ise_usb->hcd, 16);

    // allocate, initialize and link the primary queues
    // qN -> q1 -> q_bulk -> q_control, where N != 1
    ise_ohci->control_queue = (ise_ohci_queue_t*) ise_ohci_alloc_queue(ise_usb->hcd);
    ise_ohci->bulk_queue = (ise_ohci_queue_t*) ise_ohci_alloc_queue(ise_usb->hcd);
    //ise_ohci_link_queues(ise_ohci->bulk_queue, ise_ohci->control_queue);
    ise_ohci_queue_t* periodic_queue[6];
    ise_ohci_queue_t* next_queue;
    for(i=0; i<6; i++) {
        periodic_queue[i] = (ise_ohci_queue_t*) ise_ohci_alloc_queue(ise_usb->hcd);
        //ise_ohci_init_queue(periodic_queue[i]);
        next_queue = (i==0) ? NULL : periodic_queue[0];
        ise_ohci_link_queues(periodic_queue[i], next_queue);
    }

    // initialize the framelist pointers
    // this pattern goes for 32 entries
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
    for(i=0; i<32; i++) {
        if(i & 0x01) p = 1;
        else if(i & 0x02) p = 2;
        else if(i & 0x04) p = 3;
        else if(i & 0x08) p = 4;
        else if(i & 0x10) p = 5;
        else p = 0;
        ise_ohci->hcca[i] = ((uint32_t) periodic_queue[p]);
    }

    return true;
}

// enable the device
void ise_ohci_enable(ise_ohci_t* ise_ohci)
{
    // disable interrupts with master interrupt disable register
    ise_ohci->reg[ISE_OHCI_HC_INTERRUPT_DISABLE] = 0x80000000;
    
    // set Fm interval
    // frame interval default set to 0x2EDF
    // full speed max packet is 0x2778
    // set bit 31 to tell controller the value is modified
    ise_ohci->reg[ISE_OHCI_HC_FM_INTERVAL] = 0xA7782EDF;

    // set periodic start
    // give priority to interrupt list after 1200 frames
    ise_ohci->reg[ISE_OHCI_HC_PERIODIC_START] = 0x2A2F;

    // no power switching - ports are always powered
    ise_ohci->reg[ISE_OHCI_HC_RH_DESCRIPTOR_A] |= 0x200;

    // set HCCA base address
    ise_ohci->reg[ISE_OHCI_HC_HCCA] = (uint32_t) (ise_ohci->hcca);
    
    // set control and bulk queue heads
    ise_ohci->reg[ISE_OHCI_HC_CONTROL_HEAD_ED] = (uint32_t) (ise_ohci->control_queue);
    ise_ohci->reg[ISE_OHCI_HC_BULK_HEAD_ED] = (uint32_t) (ise_ohci->bulk_queue);

    // clear start of frame
    ise_ohci->reg[ISE_OHCI_HC_INTERRUPT_STATUS] = 0x4;

    // run the schedule
    // bit 5:2 are the bulk, control, iso, and interrupt enable bits
    // bit 7:6 is operational state (2 meaning operational)
    ise_ohci->reg[ISE_OHCI_HC_CONTROL] = 0xBC;

}

// free resources
void ise_ohci_free_resources(ise_ohci_t* ise_ohci)
{
    int p;
    for(p=0; p<ISE_OHCI_MAX_MEM_POOL; p++) {
        if(ise_ohci->mem_pool[p]) {
            ise_mem_aligned_free(ise_ohci->mem_pool[p]);
            ise_ohci->mem_pool[p] = NULL;
        }
    }
}

// basic setup of ohci controller
bool ise_ohci_setup(ise_usb_hc_t* ise_usb)
{
    ise_ohci_t* ise_ohci = (ise_ohci_t*) ise_usb->hcd;

    // Get base address (should be in MMIO space)
    void* paddr = (void*) ise_pci_read_config(0, ise_usb->pci_slot, ise_usb->pci_func, 0x10);
    if(((uint32_t) paddr) & 1) {
        printf("OHCI device using IO space - not supported\n");
        return false;
    }
    ise_ohci->reg = (uint32_t*) ise_mem_map(paddr, 0x1000);

    // Get the interrupt number
    ise_usb->interrupt_num = ise_pci_read_config(0, ise_usb->pci_slot, ise_usb->pci_func, 0x3C) & 0xFF;
    if(ise_usb->interrupt_num < 3 || ise_usb->interrupt_num > 0xF) {
        // in Dos, should only have interrupts between 0x3 and 0xF...
        printf("OHCI device on slot/func 0x%x/0x%x - bad interrupt number\n", ise_usb->pci_slot, ise_usb->pci_func);
        return false;
    }

    // Check revision
    uint32_t revision = ise_ohci->reg[ISE_OHCI_HC_REVISION];
    if(revision & 0xFF != 0x10) {
        // Bad revision
        printf("OHCI revision incompatible: revision 0x%x\n", revision & 0xFF);
        return false;
    }

    // Reset and see if it succeeds
    if(!ise_ohci_reset(ise_ohci)) {
        printf("OHCI device on slot/func 0x%x/0x%x - reset failed\n", ise_usb->pci_slot, ise_usb->pci_func);
        return false;
    }

    // allocate resources
    if(!ise_ohci_init_resources(ise_usb)) {
        printf("OHCI device on slot/func 0x%x/0x%x - could not allocate memory pool\n", ise_usb->pci_slot, ise_usb->pci_func);
        return false;
    }

    // If interrupts are used, should be installed here
    
    // get number of ports - can be no more than 15
    ise_ohci->num_ports = ise_ohci->reg[ISE_OHCI_HC_RH_DESCRIPTOR_A] & 0xF;

    // reset downstream ports
    ise_ohci->reg[ISE_OHCI_HC_CONTROL] = 0x00;
    ise_time_wait(50);
    ise_ohci->reg[ISE_OHCI_HC_CONTROL] = 0xC0;
    
    return true;
}

void ise_ohci_install()
{
    printf("installing ohci...\n");
    int s, f;
    for(s=0; s<ISE_PCI_MAX_SLOTS && ise_usb_num_hosts < ISE_USB_MAX_HC; s++) {
        for(f=0; f<ISE_PCI_MAX_FUNC && ise_usb_num_hosts < ISE_USB_MAX_HC; f++) {
            if(ise_pci.slot[s][f].vendor_id != ISE_PCI_VENDOR_INVALID && ise_pci.slot[s][f].class_code == ISE_OHCI_PCI_CLASS) {
                ise_usb[ise_usb_num_hosts].pci_slot = s;
                ise_usb[ise_usb_num_hosts].pci_func = f;
                ise_pci.slot[s][f].priv = (void*) &ise_usb[ise_usb_num_hosts];
                
                // allocate ohci data
                ise_usb[ise_usb_num_hosts].hcd = (ise_usb_hcd_t*) malloc(sizeof(ise_ohci_t));
                if(ise_usb[ise_usb_num_hosts].hcd) {
                    ise_ohci_t* ise_ohci = (ise_ohci_t*) ise_usb[ise_usb_num_hosts].hcd;
                    memset(ise_ohci, 0, sizeof(ise_ohci_t));
                    if(ise_ohci_setup(&ise_usb[ise_usb_num_hosts])) {
                        // display info
                        printf("OHCI device on slot/func 0x%x/0x%x\n", s, f);
                        printf("    USB base 0x%x   num ports %d\n", ise_ohci->reg, ise_ohci->num_ports);

                        // allocate root hub
                        ise_usb[ise_usb_num_hosts].device[0] = (ise_usb_device_t*) malloc(sizeof(ise_usb_hub_t));
                        memset(ise_usb[ise_usb_num_hosts].device[0], 0, sizeof(ise_usb_hub_t));
                        ise_usb_hub_t* root_hub = (ise_usb_hub_t*) ise_usb[ise_usb_num_hosts].device[0];
                        root_hub->num_ports = ise_ohci->num_ports;
                        
                        root_hub->port_changed = ise_ohci_port_changed;
                        root_hub->reset_port = ise_ohci_reset_port;
                        root_hub->disable_port = ise_ohci_disable_port;
                        ise_usb[ise_usb_num_hosts].alloc_mem = ise_ohci_alloc_mem;
                        ise_usb[ise_usb_num_hosts].free_mem = ise_ohci_free_mem;
                        ise_usb[ise_usb_num_hosts].alloc_queue = ise_ohci_alloc_queue;
                        ise_usb[ise_usb_num_hosts].free_queue = ise_ohci_free_queue;
                        ise_usb[ise_usb_num_hosts].alloc_td = ise_ohci_alloc_td;
                        ise_usb[ise_usb_num_hosts].free_td = ise_ohci_free_td;
                        ise_usb[ise_usb_num_hosts].get_queue = ise_ohci_get_queue;
                        ise_usb[ise_usb_num_hosts].fill_qh = ise_ohci_fill_qh;
                        ise_usb[ise_usb_num_hosts].fill_td = ise_ohci_fill_td;
                        ise_usb[ise_usb_num_hosts].push_queue = ise_ohci_push_queue;
                        ise_usb[ise_usb_num_hosts].enqueue_queue = ise_ohci_enqueue_queue;
                        ise_usb[ise_usb_num_hosts].dequeue_queue = ise_ohci_dequeue_queue;
                        ise_usb[ise_usb_num_hosts].push_td = ise_ohci_push_td;
                        ise_usb[ise_usb_num_hosts].enqueue_td = ise_ohci_enqueue_td;
                        ise_usb[ise_usb_num_hosts].is_td_done = ise_ohci_is_td_done;
                        ise_usb[ise_usb_num_hosts].is_td_fatal = ise_ohci_is_td_fatal;
                        ise_usb[ise_usb_num_hosts].is_td_stalled = ise_ohci_is_td_stalled;
                        ise_usb[ise_usb_num_hosts].is_td_short = ise_ohci_is_td_short;
                        ise_usb[ise_usb_num_hosts].activate_td = ise_ohci_activate_td;
                        ise_usb[ise_usb_num_hosts].deactivate_td = ise_ohci_deactivate_td;
                        ise_usb[ise_usb_num_hosts].clear_td_status = ise_ohci_clear_td_status;
                        ise_usb[ise_usb_num_hosts].restart_td = ise_ohci_restart_td;
                        ise_usb[ise_usb_num_hosts].dequeue_completed_tds = ise_ohci_dequeue_completed_tds;
                        ise_usb[ise_usb_num_hosts].dequeue_td = ise_ohci_dequeue_td;
                        ise_usb[ise_usb_num_hosts].restart_queue = ise_ohci_restart_queue;
                        ise_usb[ise_usb_num_hosts].start_queue = ise_ohci_start_queue;
                        ise_usb[ise_usb_num_hosts].stop_queue = ise_ohci_stop_queue;
                        ise_usb[ise_usb_num_hosts].clear_queue = ise_ohci_clear_queue;
                        ise_usb[ise_usb_num_hosts].is_queue_done = ise_ohci_is_queue_done;
                        ise_usb[ise_usb_num_hosts].is_queue_fatal = ise_ohci_is_queue_fatal;
                        ise_usb_num_hosts++;
                        
                        // enable host controller
                        ise_ohci_enable(ise_ohci);
                    } else {
                        free(ise_usb[ise_usb_num_hosts].hcd);
                        ise_usb[ise_usb_num_hosts].hcd = NULL;
                    }

                    /*ise_ohci->reg = (uint32_t*) ise_pci_read_config(0, s, f, 0x10);

                    // display revision
                    printf("reg base addr 0x%x\n", (uint32_t) ise_ohci->reg);
                    uint32_t revision = ise_ohci->reg[ISE_OHCI_HC_REVISION];
                    printf("ohci device on slot/func 0x%x/0x%x...revision 0x%x\n", s, f, revision);
                    */

                }

            }
        }
    }
/*
    if(ise_ohci_num_devices) {
        // if we found any ohci devices...
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

void ise_ohci_uninstall()
{
    int h;
    for(h=0; h<ise_usb_num_hosts; h++) {
        if(ise_pci.slot[ise_usb[h].pci_slot][ise_usb[h].pci_func].class_code == ISE_OHCI_PCI_CLASS) {
            ise_ohci_t* ise_ohci = (ise_ohci_t*) ise_usb[h].hcd;
            if(ise_ohci) {
                ise_ohci_reset(ise_ohci);
                ise_ohci_free_resources(ise_ohci);
                ise_mem_unmap( (void*) ise_ohci->reg );
                free(ise_ohci);
                ise_usb[h].hcd = NULL;
            }
            ise_pci.slot[ise_usb[h].pci_slot][ise_usb[h].pci_func].priv = NULL;
        }
    }
}
