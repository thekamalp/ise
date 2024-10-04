// InfiniScroll Engine
// ise_usb.cpp
// Universal Serial Bus driver
//
// Kamal Pillai
// 2/11/2019

#include "ise.h"

#define ISE_USB_DEBUG_FILE_NAME "dump.txt"
FILE* ise_usb_debug_file = NULL;

int ise_usb_num_hosts = 0;
ise_usb_hc_t ise_usb[ISE_USB_MAX_HC];

uint32_t timer_tick = 0;
uint32_t hid_tick = 0;
uint32_t ise_usb_time_tick = 0;
void _interrupt (FAR *ise_usb_prev_timer_isr)() = NULL;

int ise_usb_num_hid_interfaces = 0;
ise_usb_hid_interface_t* ise_usb_hid_interface[ISE_USB_HID_MAX_INTERFACES] = {NULL};

int ise_usb_num_hubs = 0;
ise_usb_hub_t* ise_usb_hub[ISE_USB_MAX_HUBS] = {NULL};

//int ise_usb_irq_count[16] = {0};
//void _interrupt (FAR *ise_usb_prev_irq_isr[16])();
//uint16_t ise_usb_irq_mask = 0xFFFF;

void ise_usb_uninstall_device(ise_usb_hc_t* ise_usb, uint8_t device_id)
{
    int i, j;
    if(ise_usb->device[device_id]) {
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Freeing device id %d\n", device_id);

        if(ise_usb->device[device_id]->manufacturer) free(ise_usb->device[device_id]->manufacturer);
        if(ise_usb->device[device_id]->product) free(ise_usb->device[device_id]->product);
        for(i=0; i<ise_usb->device[device_id]->num_interfaces; i++) {
            if(ise_usb->device[device_id]->interface[i]) {
                if(ise_usb->device[device_id]->interface[i]->class_code == ISE_USB_CLASS_HID) {
                    ise_usb_hid_interface_t* hid_interface = (ise_usb_hid_interface_t*) (ise_usb->device[device_id]->interface[i]);
                    for(j=0; j<ise_usb_num_hid_interfaces; ) {
                        if(ise_usb_hid_interface[j] == hid_interface) {
                            ise_usb_num_hid_interfaces--;
                            ise_usb_hid_interface[j] = ise_usb_hid_interface[ise_usb_num_hid_interfaces];
                            ise_usb_hid_interface[ise_usb_num_hid_interfaces] = NULL;
                        } else {
                            j++;
                        }
                    }
                    if(hid_interface->data_queue) {
						ise_usb->clear_queue(hid_interface->data_queue);
                        ise_usb->dequeue_queue(hid_interface->interrupt_queue, hid_interface->data_queue);
                        ise_usb->free_queue(ise_usb->hcd, hid_interface->data_queue);
                    }
                    if(hid_interface->in_td) {
                        //ise_usb->dequeue_td(hid_interface->interrupt_queue, hid_interface->in_td);
                        ise_usb->free_td(ise_usb->hcd, hid_interface->in_td);
                    }
                    if(hid_interface->data) {
                        //int ep = hid_interface->in_endpoint;
                        //int max_packet_size = ise_usb->device[device_id]->endpoint[ep].max_packet_size;
                        //ise_usb->free_mem(ise_usb->hcd, hid_interface->data, max_packet_size);
						free(hid_interface->data);
                    }
                }
                free(ise_usb->device[device_id]->interface[i]);
            }
        }
        if(ise_usb->device[device_id]->class_code == ISE_USB_CLASS_HUB && device_id != 0) {
            ise_usb_external_hub_t* hub_device = (ise_usb_external_hub_t*) (ise_usb->device[device_id]);
            for(j=ise_usb_num_hosts; j<ise_usb_num_hubs; ) {
                if(ise_usb_hub[j] == (ise_usb_hub_t*) hub_device) {
                    ise_usb_num_hubs--;
                    ise_usb_hub[j] = ise_usb_hub[ise_usb_num_hubs];
                    ise_usb_hub[ise_usb_num_hubs] = NULL;
                } else {
                    j++;
                }
            }
            if(hub_device->data_queue) {
				ise_usb->clear_queue(hub_device->data_queue);
                ise_usb->dequeue_queue(hub_device->interrupt_queue, hub_device->data_queue);
                ise_usb->free_queue(ise_usb->hcd, hub_device->data_queue);
            }
            if(hub_device->in_td) {
                //ise_usb->dequeue_td(hub_device->interrupt_queue, hub_device->in_td);
                ise_usb->free_td(ise_usb->hcd, hub_device->in_td);
            }
            ise_usb_queue_t* ctrl_q = ise_usb->device[device_id]->device_ctrl_q;
            if(ctrl_q == NULL) ctrl_q = ise_usb->get_queue(ise_usb->hcd, ISE_USB_QUEUE_CONTROL);
            for(j=0; j<2; j++) {
                if(hub_device->ctrl_td[j]) {
                    ise_usb->dequeue_td(ctrl_q, hub_device->ctrl_td[j]);
                    ise_usb->free_td(ise_usb->hcd, hub_device->ctrl_td[j]);
                }
            }
            if(hub_device->hub_data) {
                //int ep = hub_device->in_endpoint;
                //int max_packet_size = ise_usb->device[device_id]->endpoint[ep].max_packet_size;
                //ise_usb->free_mem(ise_usb->hcd, hub_device->hub_data, max_packet_size);
				free(hub_device->hub_data);
            }
        }
        if(ise_usb->device[device_id]->device_ctrl_q) {
            ise_usb_queue_t* host_ctrl_q = ise_usb->get_queue(ise_usb->hcd, ISE_USB_QUEUE_CONTROL);
            ise_usb->dequeue_queue(host_ctrl_q, ise_usb->device[device_id]->device_ctrl_q);
            ise_usb->free_queue(ise_usb->hcd, ise_usb->device[device_id]->device_ctrl_q);
        }
        free(ise_usb->device[device_id]);
        ise_usb->device[device_id] = NULL;
    }
}

void ise_usb_install()
{
    if(ise_usb_num_hosts >= ISE_USB_MAX_HC) return;
    memset(&ise_usb[ise_usb_num_hosts], 0, (ISE_USB_MAX_HC-ise_usb_num_hosts)*sizeof(ise_usb_hc_t));
    int h = ise_usb_num_hosts, i;
    
    //if(ise_usb_debug_file == NULL) {
    //    ise_usb_debug_file = fopen(ISE_USB_DEBUG_FILE_NAME, "wb");
    //}

    ise_uhci_install();
    ise_ehci_install();
    ise_ohci_install();

    // get the current irq mask, if we don't have it already
    //if(ise_usb_irq_mask == 0xFFFF) {
    //    ise_usb_irq_mask = (uint16_t) ((inp(ISE_IRQ_PIC1_DATA) << 8) | inp(ISE_IRQ_PIC0_DATA));
    //}

    // Initialize configuration state,
    // and allocate resources to do configuration
    for(; h<ise_usb_num_hosts; h++) {
        ise_usb[h].host_id = h;
        ise_usb[h].config_state = ISE_USB_CONFIG_STATE_IDLE;
        ise_usb[h].num_config_packets = 0;
        if(ise_usb[h].alloc_td) {
            for(i=0; i<ISE_USB_MAX_CONFIG_TDS; i++) {
                ise_usb[h].config_td[i] = ise_usb[h].alloc_td(ise_usb[h].hcd);
            }
        }
        if(ise_usb_num_hubs < ISE_USB_MAX_HUBS) {
            ise_usb_hub[ise_usb_num_hubs] = (ise_usb_hub_t*) ise_usb[h].device[0];
            if(ise_usb_hub[ise_usb_num_hubs]) {
                ise_usb_hub[ise_usb_num_hubs]->device.interface[0] = (ise_usb_interface_t*) malloc(sizeof(ise_usb_interface_t));
                ise_usb_hub[ise_usb_num_hubs]->device.interface[0]->class_code = ISE_USB_CLASS_HUB;
                ise_usb_hub[ise_usb_num_hubs]->device.interface[0]->subclass_code = 0;
                ise_usb_hub[ise_usb_num_hubs]->device.interface[0]->protocol = 0;
                ise_usb_hub[ise_usb_num_hubs]->device.interface[0]->host_id = ise_usb_num_hubs;
                ise_usb_hub[ise_usb_num_hubs]->device.num_interfaces = 1;
                ise_usb_hub[ise_usb_num_hubs]->device.class_code = ISE_USB_CLASS_HUB;
                ise_usb_hub[ise_usb_num_hubs]->device.device_id = 0;
            }
            ise_usb_num_hubs++;
        }
        
        /*if(ise_usb[h].interrupt_num > 2) {
            if(ise_usb_irq_count[ise_usb[h].interrupt_num] == 0) {
                if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "IRQ 0x%x vector %d\n", ise_usb[h].interrupt_num, ISE_IRQ_INTERRUPT_VECTOR(ise_usb[h].interrupt_num));
                ise_usb_prev_irq_isr[ise_usb[h].interrupt_num] = _dos_getvect(ISE_IRQ_INTERRUPT_VECTOR(ise_usb[h].interrupt_num));
                _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(ise_usb[h].interrupt_num), ise_usb_isr);
                int interrupt_num = ise_usb[h].interrupt_num;
                int new_interrupt_mask, pic_data;
                if(interrupt_num < 8) {
                    pic_data = ISE_IRQ_PIC0_DATA;
                } else {
                    pic_data = ISE_IRQ_PIC1_DATA;
                    interrupt_num -= 8;
                }
                new_interrupt_mask = inp(pic_data) & ~(1 << interrupt_num);
                outp(pic_data, new_interrupt_mask);
            }
            ise_usb_irq_count[ise_usb[h].interrupt_num]++;
        }*/
        
        //ise_pci_register_isr(ise_usb[h].interrupt_num, ise_usb_isr);
    }

    if(ise_usb_prev_timer_isr == NULL) {
        ise_usb_prev_timer_isr = _dos_getvect(ISE_IRQ_INTERRUPT_VECTOR(0));
        _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(0), ise_usb_isr);
        ise_time_set_divider(ISE_USB_TIME_DIVIDER_LOG2, &ise_usb_time_tick);
    }

}

void ise_usb_uninstall()
{
    int h, i;
	//ise_time_wait(1000); // wait for any queued up devices to send data before uninstalling everything

     if(ise_usb_prev_timer_isr) {
        ise_time_set_divider(0, NULL);
        _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(0), ise_usb_prev_timer_isr);
    }

   for(h=0; h<ise_usb_num_hosts; h++) {
        for(i=0; i<ISE_USB_MAX_DEVICES; i++) {
            ise_usb_uninstall_device(&ise_usb[h], i);
        }
        /*if(ise_usb[h].interrupt_num > 2) {
            ise_usb_irq_count[ise_usb[h].interrupt_num]--;
            if(ise_usb_irq_count[ise_usb[h].interrupt_num] == 0) {
                int interrupt_num = ise_usb[h].interrupt_num;
                if(ise_usb_irq_mask & (1 << interrupt_num)) {
                    int new_interrupt_mask, pic_data;
                    if(interrupt_num < 8) {
                        pic_data = ISE_IRQ_PIC0_DATA;
                    } else {
                        pic_data = ISE_IRQ_PIC1_DATA;
                        interrupt_num -= 8;
                    }
                    new_interrupt_mask = inp(pic_data) | (1 << interrupt_num);
                    outp(pic_data, new_interrupt_mask);
                }
                if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "IRQ 0x%x vector %d\n", ise_usb[h].interrupt_num, ISE_IRQ_INTERRUPT_VECTOR(ise_usb[h].interrupt_num));
                _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(ise_usb[h].interrupt_num), ise_usb_prev_irq_isr[ise_usb[h].interrupt_num]);
            } 
        }*/
        
        //ise_pci_unregister_isr(ise_usb[h].interrupt_num, ise_usb_isr);
    }

    ise_ehci_uninstall();
    ise_ohci_uninstall();
    ise_uhci_uninstall();

    ise_usb_num_hosts = 0;
    if(ise_usb_debug_file) {
        fprintf(ise_usb_debug_file, "ticks: %d, hid_ticks: %d, timer_ticks: %d\n", ise_usb_time_tick, hid_tick, timer_tick);
        if(ise_usb_debug_file != stdout) fclose(ise_usb_debug_file);
        ise_usb_debug_file = NULL;
    }
}

// converts status received from port to status used by engine
uint8_t ise_usb_hub_convert_port_status(uint32_t in_status)
{
    uint8_t out_status;
    // bottom 2 bits are port connected (bit 0) and port enabled (bit 1) in both input and output
    out_status = (uint8_t) (in_status & 0x3);
    // port indicator (bit 12) maps to bit 3
    if(in_status & 0x1000) out_status |= 0x4;
    // bit 10:9 in input is the speed, which then maps to bit 6:4
    if(in_status & 0x200) {
        out_status |= ISE_USB_SPEED_LOW << 4;
    } else {
        out_status |= ((in_status & 0x400) ? ISE_USB_SPEED_HIGH : ISE_USB_SPEED_FULL) << 4;
    }
    // bit 16, 17 and 20 are the port connection, enable and reset change signals
    // if any of these change, then report that the prot has been changed
    out_status |= (in_status & 0x130000) ? 0x80 : 0x0;
    return out_status;
}

int ise_usb_send_control(ise_usb_hc_t* ise_usb, int device_id, ise_usb_device_request_packet_t* config_req_packet, void* data_buffer, ise_usb_td_t** ctrl_td, int max_td, bool ioc)
{
    int i;
    ise_usb_td_info_t td_info;
    ise_usb_queue_t* ctrl_q = ise_usb->device[device_id]->device_ctrl_q;
    if(ctrl_q == NULL) ctrl_q = ise_usb->get_queue(ise_usb->hcd, ISE_USB_QUEUE_CONTROL);
    int remaining_length = config_req_packet->length;
    int max_packet_size = ise_usb->device[device_id]->max_packet_size;
    int num_packets = 0;
    uint8_t* buffer = (uint8_t*) data_buffer;

    td_info.spd = false;
    td_info.speed = ise_usb->device[device_id]->speed;
    td_info.ioc = (remaining_length == 0) ? ioc : false;
    td_info.length = sizeof(ise_usb_device_request_packet_t);
    td_info.dt = false;
    td_info.end_pt = 0;
    td_info.device = device_id;
    td_info.pid = ISE_USB_PID_SETUP;
    td_info.buffer = config_req_packet;
    ise_usb->fill_td(ctrl_td[num_packets], &td_info);
    num_packets++;

    td_info.pid = (config_req_packet->request_type & ISE_USB_REQUEST_TYPE_IN) ? ISE_USB_PID_IN : ISE_USB_PID_OUT;
    while(remaining_length > 0 && num_packets < max_td) {
        if(remaining_length < max_packet_size) {
            td_info.ioc = ioc;
            td_info.length = remaining_length;
        } else {
            td_info.ioc = false;
            td_info.length = max_packet_size;
        }
        td_info.buffer = buffer;
        td_info.dt = !td_info.dt;
        ise_usb->fill_td(ctrl_td[num_packets], &td_info);
        num_packets++;
        buffer += td_info.length;
        remaining_length -= td_info.length;
    }
    for(i=num_packets-1; i>=0; i--) {
        ise_usb->push_td(ctrl_q, ctrl_td[i]);
    }
    return num_packets;
}

void ise_usb_ack_control(ise_usb_hc_t* ise_usb, int device_id, int pid, ise_usb_td_t* ctrl_td, bool ioc)
{
    ise_usb_td_info_t td_info;
    ise_usb_queue_t* ctrl_q = ise_usb->device[device_id]->device_ctrl_q;
    if(ctrl_q == NULL) ctrl_q = ise_usb->get_queue(ise_usb->hcd, ISE_USB_QUEUE_CONTROL);

    td_info.spd = false;
    td_info.speed = ise_usb->device[device_id]->speed;
    td_info.ioc = ioc;
    td_info.length = 0;
    td_info.dt = true;
    td_info.end_pt = 0;
    td_info.device = device_id;
    td_info.pid = pid;
    td_info.buffer = NULL;
    ise_usb->fill_td(ctrl_td, &td_info);
    ise_usb->push_td(ctrl_q, ctrl_td);
}

int ise_usb_wait_for_completion(ise_usb_hc_t* ise_usb, int device_id, ise_usb_td_t* ctrl_td)
{
    int i;
    for(i=0; i<10 && !ise_usb->is_td_done(ctrl_td); i++) {
        ise_time_wait(5);
    }
    if(i==10) {
        return -1;
    }
    ise_usb_queue_t* ctrl_q = ise_usb->device[device_id]->device_ctrl_q;
    if(ctrl_q == NULL) ctrl_q = ise_usb->get_queue(ise_usb->hcd, ISE_USB_QUEUE_CONTROL);
    ise_usb->dequeue_completed_tds(ise_usb->hcd, ctrl_q, false);
    return 0;
}

void ise_usb_hub_update_port_status(ise_usb_hc_t* ise_usb, int device_id, int port)
{
    int num_packets;
    ise_usb_queue_t* host_ctrl_q = ise_usb->get_queue(ise_usb->hcd, ISE_USB_QUEUE_CONTROL);
    ise_usb_queue_t* ctrl_q = ise_usb->device[device_id]->device_ctrl_q;
    if(ctrl_q == NULL) ctrl_q = host_ctrl_q;
    ise_usb_external_hub_t* hub_device = (ise_usb_external_hub_t*) ise_usb->device[device_id];
    
    ise_usb_device_request_packet_t* config_req_packet = (ise_usb_device_request_packet_t*) (hub_device->hub_data + 2);

    // Get port status
    config_req_packet->request_type = ISE_USB_REQUEST_TYPE_IN | ISE_USB_REQUEST_TYPE_CLASS | ISE_USB_REQUEST_TYPE_OTHER;
    config_req_packet->request = ISE_USB_REQUEST_GET_STATUS;
    config_req_packet->value = ISE_USB_HUB_REQUEST_PORT_CONNECTION;
    config_req_packet->index = (uint8_t) (port+1);
    config_req_packet->length = 4;
        
    // Send a setup packet
    num_packets = ise_usb_send_control(ise_usb, device_id, config_req_packet, hub_device->hub_data + 2, &(hub_device->ctrl_td[0]), 2, false);
    ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
    ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
    if(ise_usb_wait_for_completion(ise_usb, device_id, hub_device->ctrl_td[num_packets - 1]) != 0) {
        if(ise_usb_debug_file) {
            fprintf(ise_usb_debug_file, "Could not get port status\n");
            ise_mem_dump(ise_usb_debug_file, ctrl_q, 4);
            ise_mem_dump(ise_usb_debug_file, hub_device->ctrl_td[0], 4);
            ise_mem_dump(ise_usb_debug_file, config_req_packet, 2);
        }
        return;
    }

    // Process the data
    hub_device->port_status[port] = ise_usb_hub_convert_port_status(*(hub_device->hub_data + 2));
    if(ise_usb_debug_file) {
        fprintf(ise_usb_debug_file, "Got port %d status: 0x%x\n", port, hub_device->port_status[port]);
        ise_mem_dump(ise_usb_debug_file, hub_device->hub_data + 2, 1);
    }

    // Send an ack
    ise_usb_ack_control(ise_usb, device_id, ISE_USB_PID_OUT, hub_device->ctrl_td[0], false);
    ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
    ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
    num_packets = 1;
    if(ise_usb_wait_for_completion(ise_usb, device_id, hub_device->ctrl_td[num_packets - 1]) != 0) {
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Could not acknowledge port status\n");
        return;
    }

}

int ise_usb_hub_port_changed(ise_usb_hc_t* ise_usb, int device_id, int port)
{
    ise_usb_external_hub_t* hub_device = (ise_usb_external_hub_t*) ise_usb->device[device_id];
    if(hub_device->port_status[port] & 0x80) {
        ise_usb_hub_update_port_status(ise_usb, device_id, port);

        int port_status = hub_device->port_status[port];
        int status = -1;
        // get connection status
        if(port_status & 0x80) {
            status = (port_status & 0x1) ? 0x80 : 0x00;
            // get the port speed
            status |= (port_status >> 4) & 0x7;
        }
        return status;
    } else {
        return -1;
    }
}

int ise_usb_hub_reset_port(ise_usb_hc_t* ise_usb, int device_id, int port)
{
    int num_packets;
    ise_usb_queue_t* host_ctrl_q = ise_usb->get_queue(ise_usb->hcd, ISE_USB_QUEUE_CONTROL);
    ise_usb_queue_t* ctrl_q = ise_usb->device[device_id]->device_ctrl_q;
    if(ctrl_q == NULL) ctrl_q = host_ctrl_q;
    ise_usb_external_hub_t* hub_device = (ise_usb_external_hub_t*) ise_usb->device[device_id];
    ise_usb_device_request_packet_t* config_req_packet = (ise_usb_device_request_packet_t*) (hub_device->hub_data + 2);

    // Reset port
    config_req_packet->request_type = ISE_USB_REQUEST_TYPE_OUT | ISE_USB_REQUEST_TYPE_CLASS | ISE_USB_REQUEST_TYPE_OTHER;
    config_req_packet->request = ISE_USB_REQUEST_SET_FEATURE;
    config_req_packet->value = ISE_USB_HUB_REQUEST_PORT_RESET;
    config_req_packet->index = (uint8_t) (port+1);
    config_req_packet->length = 0;

    // Send a setup packet
    num_packets = ise_usb_send_control(ise_usb, device_id, config_req_packet, hub_device->hub_data + 2, &(hub_device->ctrl_td[0]), 2, false);
    ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
    ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
    //ise_time_wait(10);
    // Wait for completion
    if(ise_usb_wait_for_completion(ise_usb, device_id, hub_device->ctrl_td[num_packets - 1]) != 0) {
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Could not reset port\n");
        return -1;
    }

    // Clear port change fields from 0x10 to 0x14
    int f;
    for(f=0x10; f<=0x14; f++) {
        config_req_packet->request_type = ISE_USB_REQUEST_TYPE_OUT | ISE_USB_REQUEST_TYPE_CLASS | ISE_USB_REQUEST_TYPE_OTHER;
        config_req_packet->request = ISE_USB_REQUEST_CLEAR_FEATURE;
        config_req_packet->value = f;
        config_req_packet->index = (uint8_t) (port+1);
        config_req_packet->length = 0;

        // Send a setup packet
        num_packets = ise_usb_send_control(ise_usb, device_id, config_req_packet, hub_device->hub_data + 2, &(hub_device->ctrl_td[1]), 1, false);

        // Send the ack for the previous setup packet
        ise_usb_ack_control(ise_usb, device_id, ISE_USB_PID_IN, hub_device->ctrl_td[0], false);
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        num_packets++;

        // Wait for completion
        if(ise_usb_wait_for_completion(ise_usb, device_id, hub_device->ctrl_td[num_packets - 1]) != 0) {
            if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Could not clear port change 0x%x\n", f);
            return -1;
        }
    }

    // Send an ack
    ise_usb_ack_control(ise_usb, device_id, ISE_USB_PID_IN, hub_device->ctrl_td[0], false);
    ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
    ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
    num_packets = 1;

    // Wait for completion
    if(ise_usb_wait_for_completion(ise_usb, device_id, hub_device->ctrl_td[num_packets - 1]) != 0) {
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Could not acknowledge port reset clear\n");
        return -1;
    }

    if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Port %d reset\n", port);

    // Re-read the port status
    ise_usb_hub_update_port_status(ise_usb, device_id, port);
    hub_device->port_status[port] &= 0x7f;
    int port_status = hub_device->port_status[port];
    int status;
    // port should also be enabled - if not, it should be treated as disconnected
    status = ((port_status & 0x3) == 0x3) ? 0x80 : 0x00;
    // get the port speed
    status |= (port_status >> 4) & 0x7;

    return status;
}

void ise_usb_hub_disable_port(ise_usb_hc_t* ise_usb, int device_id, int port)
{
    int num_packets;
    ise_usb_queue_t* host_ctrl_q = ise_usb->get_queue(ise_usb->hcd, ISE_USB_QUEUE_CONTROL);
    ise_usb_queue_t* ctrl_q = ise_usb->device[device_id]->device_ctrl_q;
    if(ctrl_q == NULL) ctrl_q = host_ctrl_q;
    ise_usb_external_hub_t* hub_device = (ise_usb_external_hub_t*) ise_usb->device[device_id];
    ise_usb_device_request_packet_t* config_req_packet = (ise_usb_device_request_packet_t*) (hub_device->hub_data + 2);

    // Disable port
    config_req_packet->request_type = ISE_USB_REQUEST_TYPE_OUT | ISE_USB_REQUEST_TYPE_CLASS | ISE_USB_REQUEST_TYPE_OTHER;
    config_req_packet->request = ISE_USB_REQUEST_CLEAR_FEATURE;
    config_req_packet->value = ISE_USB_HUB_REQUEST_PORT_ENABLE;
    config_req_packet->index = (uint8_t) (port+1);
    config_req_packet->length = 0;

    // Send a setup packet
    num_packets = ise_usb_send_control(ise_usb, device_id, config_req_packet, hub_device->hub_data + 2, &(hub_device->ctrl_td[0]), 2, false);
    ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
    ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
    // Wait for completion
    if(ise_usb_wait_for_completion(ise_usb, device_id, hub_device->ctrl_td[num_packets - 1]) != 0) {
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Could not disable port\n");
        return;
    }

    // Send an ack
    ise_usb_ack_control(ise_usb, device_id, ISE_USB_PID_IN, hub_device->ctrl_td[0], false);
    ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
    ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
    num_packets = 1;

    // Wait for completion
    if(ise_usb_wait_for_completion(ise_usb, device_id, hub_device->ctrl_td[num_packets - 1]) != 0) {
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Could not acknowledge port disable\n");
        return;
    }
    if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Port %d disabled\n", port);
}

void ise_usb_ucs2_to_ascii(uint16_t* string16, char* string8, int length)
{
    int i;
    for(i=0; i<length; i++) {
        *string8 = (char) (*string16 & 0xFF);
        string8++;
        string16++;
    }
    *string8 = '\0';
}

// Constructs and enqueue TDs needed for a config request
// config request packet must be filled prior to this call
// this call will not restart the queue
void ise_usb_enqueue_config_request(ise_usb_hc_t* ise_usb)
{
    ise_usb_td_info_t td_info;
    int config_device_id = ise_usb->config_device_id;
    int remaining_length = ise_usb->config_req_packet->length;
    ise_usb_queue_t* ctrl_q = NULL;
    if(config_device_id) {
        // if we have assigned a device id, use its control queue
        ctrl_q = ise_usb->device[config_device_id]->device_ctrl_q;
    }
    if(ctrl_q == NULL) {
        // if not, use the host control queue
        ctrl_q = ise_usb->get_queue(ise_usb->hcd, ISE_USB_QUEUE_CONTROL);
    }
    uint8_t* buffer;

    // Send a setup packet
    td_info.spd = false;
    td_info.speed = ise_usb->device[config_device_id]->speed;
    td_info.ioc = (remaining_length == 0) ? true : false;
    td_info.length = sizeof(ise_usb_device_request_packet_t);
    td_info.dt = false;
    td_info.end_pt = 0;
    td_info.device = config_device_id;
    td_info.pid = ISE_USB_PID_SETUP;
    td_info.buffer = ise_usb->config_req_packet;
    ise_usb->fill_td(ise_usb->config_td[ise_usb->num_config_packets], &td_info);
    ise_usb->enqueue_td(ctrl_q, ise_usb->config_td[ise_usb->num_config_packets]);
    ise_usb->num_config_packets++;

    // Send packets to send or receive the data
    buffer = ise_usb->config_buffer;
    td_info.spd = true;
    td_info.length = ise_usb->device[config_device_id]->max_packet_size;
    if(td_info.length < 8) td_info.length = 8;
    td_info.pid = (ise_usb->config_req_packet->request_type & ISE_USB_REQUEST_TYPE_IN) ? ISE_USB_PID_IN : ISE_USB_PID_OUT;
    for( ; ise_usb->num_config_packets<ISE_USB_MAX_CONFIG_TDS && remaining_length > 0;
         remaining_length -= td_info.length,
         buffer += td_info.length) {

        td_info.dt = !td_info.dt;
        td_info.buffer = buffer;
        if(remaining_length <= td_info.length) {
            // last TD
            td_info.ioc = true;
            td_info.length = remaining_length;
        }
        ise_usb->fill_td(ise_usb->config_td[ise_usb->num_config_packets], &td_info);
        ise_usb->enqueue_td(ctrl_q, ise_usb->config_td[ise_usb->num_config_packets]);
        ise_usb->num_config_packets++;
    }
}

ise_usb_descriptor_t* ise_usb_next_descriptor(ise_usb_descriptor_t* desc)
{
    return (ise_usb_descriptor_t*) (((uint8_t*) desc) + desc->generic.length);
}

int ise_usb_select_queue(int interval)
{
    int queue_select;
    for(queue_select=0; queue_select < 8; queue_select++) {
        if(2 << queue_select > interval) break;
    }
    return queue_select;
}

int ise_usb_parse_configuration_descriptor(ise_usb_hc_t* ise_usb)
{
    size_t alloc_size;
    int i, j, e;
    int config_device_id = ise_usb->config_device_id;
    ise_usb_descriptor_t* desc = (ise_usb_descriptor_t*) ise_usb->config_buffer;
    ise_usb->device[config_device_id]->num_interfaces = desc->configuration.num_interfaces;
    ise_usb_hid_interface_t* hid_interface;
    ise_usb_external_hub_t* hub_device;
    for(i=0; i<ise_usb->device[config_device_id]->num_interfaces; i++) {
        desc = ise_usb_next_descriptor(desc);
        if(desc->generic.desc_type != ISE_USB_DESC_TYPE_INTERFACE) {
            if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Configuration parse error: expecting interface descriptor, actually received 0x%x\n", desc->generic.desc_type);
            return -1;
        }

        // determine how much to allocate for the interface, based on class
        switch(desc->interface.interface_class) {
        case ISE_USB_CLASS_HID: alloc_size = sizeof(ise_usb_hid_interface_t); break;
        default: alloc_size = sizeof(ise_usb_interface_t); break;
        }
        ise_usb->device[config_device_id]->interface[i] = (ise_usb_interface_t*) malloc(alloc_size);
        ise_usb->device[config_device_id]->interface[i]->class_code = desc->interface.interface_class;
        ise_usb->device[config_device_id]->interface[i]->subclass_code = desc->interface.interface_subclass;
        ise_usb->device[config_device_id]->interface[i]->protocol = desc->interface.interface_protocol;
        ise_usb->device[config_device_id]->interface[i]->host_id = ise_usb->host_id;
        int num_endpoints = desc->interface.num_endpoints;

        // Parse any extra descriptors for the given class
        switch(desc->interface.interface_class) {
        case ISE_USB_CLASS_HID:
            // the hid descriptor
            desc = ise_usb_next_descriptor(desc);
            hid_interface = (ise_usb_hid_interface_t*) (ise_usb->device[config_device_id]->interface[i]);
            hid_interface->device = ise_usb->device[config_device_id];
            if(desc->generic.desc_type != ISE_USB_DESC_TYPE_HID) {
                if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Configuration parse error: expecting HID descriptor, actually received 0x%x\n", desc->generic.desc_type);
                return -1;
            }
            // if more than 1 descriptors for this HID, and the second is the hid report, use it
            if(desc->hid.num_descriptors > 1 && desc->hid.class2_desc_type == ISE_USB_DESC_TYPE_HID_REPORT) {
                // use type to store descriptor length
                hid_interface->hid_type = desc->hid.class2_desc_length1 * 256 + desc->hid.class2_desc_length0;
                if(hid_interface->hid_type > ISE_USB_CONFIG_BUFFER_SIZE) {
                    if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Configuration error: HID report greater than config buffer size - %d Bytes\n", hid_interface->hid_type);
                    return -1;
                }
                
            } else if(desc->hid.num_descriptors > 0 && desc->hid.class_desc_type == ISE_USB_DESC_TYPE_HID_REPORT) {
                // use type to store descriptor length
                hid_interface->hid_type = desc->hid.class_desc_length1 * 256 + desc->hid.class_desc_length0;
                if(hid_interface->hid_type > ISE_USB_CONFIG_BUFFER_SIZE) {
                    if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Configuration error: HID report greater than config buffer size - %d Bytes\n", hid_interface->hid_type);
                    return -1;
                }
            } else {
                if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Configuration error: no HID report found\n");
                return -1;
            }
            break;
        }
        
        // Parse endpoint descriptors
        for(e=0; e<num_endpoints; e++) {
            desc = ise_usb_next_descriptor(desc);
            if(desc->generic.desc_type != ISE_USB_DESC_TYPE_ENDPOINT) {
                if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Configuration parse error: expecting endpoint descriptor, actually received 0x%x\n", desc->generic.desc_type);
                return -1;
            }

            int ep = desc->endpoint.endpoint_address & 0xF;
            ise_usb->device[config_device_id]->endpoint[ep].attributes = desc->endpoint.attributes & 0x7f;
            ise_usb->device[config_device_id]->endpoint[ep].attributes |= desc->endpoint.endpoint_address & 0x80;
            ise_usb->device[config_device_id]->endpoint[ep].interval = desc->endpoint.interval;
            ise_usb->device[config_device_id]->endpoint[ep].max_packet_size = desc->endpoint.max_packet_size;
            if(ise_usb->device[config_device_id]->class_code == ISE_USB_CLASS_HUB) {
                hub_device = (ise_usb_external_hub_t*) (ise_usb->device[config_device_id]);
                if(desc->endpoint.endpoint_address & 0x80) {
                    hub_device->in_endpoint = ep;
                    int queue_select = ise_usb_select_queue(ise_usb->device[config_device_id]->endpoint[ep].interval);
                    hub_device->interrupt_queue = ise_usb->get_queue(ise_usb->hcd, queue_select);
                }
                if(hub_device->hub_data == NULL) {
                    int max_packet_size = ise_usb->device[config_device_id]->endpoint[ep].max_packet_size;
                    hub_device->hub_data = (uint32_t*) malloc(max_packet_size);//ise_usb->alloc_mem(ise_usb->hcd, max_packet_size);
                    *(hub_device->hub_data+0) = 0x0;
                    *(hub_device->hub_data+1) = 0x0;
                    *(hub_device->hub_data+2) = 0x0;
                    *(hub_device->hub_data+3) = 0x0;
                }
                if(hub_device->data_queue == NULL) {
                    hub_device->data_queue = ise_usb->alloc_queue(ise_usb->hcd);
                    ise_usb->fill_qh(hub_device->data_queue, ise_usb, config_device_id, ep);
                }
                if(hub_device->in_td == NULL) {
                    hub_device->in_td = ise_usb->alloc_td(ise_usb->hcd);
                }
                for(j=0; j<2; j++) {
                    if(hub_device->ctrl_td[j] == NULL) {
                        hub_device->ctrl_td[j] = ise_usb->alloc_td(ise_usb->hcd);
                    }
                }
                hub_device->hub.port_changed = ise_usb_hub_port_changed;
                hub_device->hub.reset_port = ise_usb_hub_reset_port;
                hub_device->hub.disable_port = ise_usb_hub_disable_port;

            } else if(ise_usb->device[config_device_id]->interface[i]->class_code == ISE_USB_CLASS_HID) {
                hid_interface = (ise_usb_hid_interface_t*) (ise_usb->device[config_device_id]->interface[i]);
                if(desc->endpoint.endpoint_address & 0x80) {
                    hid_interface->in_endpoint = ep;
                } else {
                    hid_interface->out_endpoint = ep;
                }
            }
        }
    }
    return 0;
}

void ise_usb_parse_hid_report(ise_usb_hc_t* ise_usb)
{
    int config_device_id = ise_usb->config_device_id;
    ise_usb_hid_interface_t* hid_interface = (ise_usb_hid_interface_t*) (ise_usb->device[config_device_id]->interface[ise_usb->config_current_interface]);
    int hid_report_length = hid_interface->hid_type;
    int ep = hid_interface->in_endpoint;
    int max_packet_size = ise_usb->device[config_device_id]->endpoint[ep].max_packet_size;
    hid_interface->hid_type = 0x0;
    hid_interface->hid_num_controls = 0;
    hid_interface->data = (uint32_t*) malloc(max_packet_size);//ise_usb->alloc_mem(ise_usb->hcd, max_packet_size);
    hid_interface->data_queue = ise_usb->alloc_queue(ise_usb->hcd);
    ise_usb->fill_qh(hid_interface->data_queue, ise_usb, config_device_id, ep);
    hid_interface->in_td = ise_usb->alloc_td(ise_usb->hcd);

    if(ise_usb_debug_file) {
        fprintf(ise_usb_debug_file, "HID report size: 0x%x\n", hid_report_length);
        ise_mem_dump(ise_usb_debug_file, ise_usb->config_buffer, 32);
    }

    int i, item, s, r, item_size;
    int global_item_stack = 0;
    uint32_t global_items[32][10] = {0};
    uint8_t usage_queue_index = 0;
    uint32_t usage_queue_min[32] = {0};
    uint32_t usage_queue_max[32] = {0};
    uint32_t usage;
    uint32_t control_bit_position = 0;
    uint32_t report_count, report_size;
    for(i=0; i<hid_report_length; i++) {
        item_size = ise_usb->config_buffer[i] & ISE_USB_HID_ITEM_SIZE_MASK;
        item_size = (1 << item_size) >> 1;  // size is 0, 1, 2, or 4
        switch(ise_usb->config_buffer[i] & ISE_USB_HID_ITEM_TYPE_MASK) {
        case ISE_USB_HID_ITEM_MAIN:
            // process the main items
            switch(ise_usb->config_buffer[i] & ISE_USB_HID_ITEM_MASK) {
            case ISE_USB_HID_ITEM_COLLECTION:
                if(item_size > 0) {
                    switch(ise_usb->config_buffer[i+1]) {
                    case ISE_USB_HID_COLLECTION_APPLICATION:
                        if(hid_interface->hid_type == 0) hid_interface->hid_type = usage_queue_min[0];
                        break;
                    }
                }
                break;

            case ISE_USB_HID_ITEM_INPUT:
                s = 0;
                usage = usage_queue_min[0];
                report_count = global_items[global_item_stack][ISE_USB_HID_ITEM_REPORT_COUNT >> 4];
                report_size = global_items[global_item_stack][ISE_USB_HID_ITEM_REPORT_SIZE >> 4];
                for(r=0; r<report_count; r++) {
                    if((usage >> 16) == ISE_USB_HID_USAGE_PAGE_BUTTONS && report_size == 1 && usage > usage_queue_min[s]) {
                        // handle buttons by lumping the vector into a single control
                        hid_interface->hid_control[hid_interface->hid_num_controls-1].length++;;
                    } else {
                        if(((usage >> 16) != ISE_USB_HID_USAGE_PAGE_UNDEFINED) && ((usage & 0xFFFF) != 0x0) &&
                           //((usage >> 16) < 0x100) && ((usage & 0xFFFF) < 0x100) &&
                            (hid_interface->hid_num_controls < ISE_USB_HID_MAX_CONTROLS)) {
                            // if the usage is defined and we have an empty slot, add the control
                            hid_interface->hid_control[hid_interface->hid_num_controls].control_type = usage;
                            hid_interface->hid_control[hid_interface->hid_num_controls].length = (uint8_t) report_size;
                            hid_interface->hid_control[hid_interface->hid_num_controls].flags = 0x0;
                            hid_interface->hid_control[hid_interface->hid_num_controls].shift = (uint16_t) control_bit_position;
                            if(global_items[global_item_stack][ISE_USB_HID_ITEM_LOGICAL_MIN >> 4] & 0x80000000) {
                                // if logical minimum is negative, use signed values
                                hid_interface->hid_control[hid_interface->hid_num_controls].flags |= 0x80;
                            }
							//int32_t default_value = (global_items[global_item_stack][ISE_USB_HID_ITEM_LOGICAL_MIN >> 4] + global_items[global_item_stack][ISE_USB_HID_ITEM_LOGICAL_MAX >> 4]) / 2;
							//default_value &= ~(~0x0 << report_size);
							//int control_dword0 = control_bit_position / 32;
							//int control_dword1 = (control_bit_position + report_size) / 32;
							//int control_shift0 = control_bit_position & 0x1F;
							//int control_shift1 = 32 - control_shift0;
							//uint32_t* data = (uint32_t*) hid_interface->data;
							//if(control_dword0 < max_packet_size/4) {
							//	data[control_dword0] |= default_value << control_shift0;
							//	if(control_dword1 > control_dword0 && control_dword1 < max_packet_size/4) {
							//		data[control_dword1] |= default_value >> control_shift1;
							//	}
							//}
                            hid_interface->hid_num_controls++;
                        }
                    }
                    if(usage == usage_queue_max[s]) {
                        if(s<usage_queue_index-1) {
                            s++;
                            usage = usage_queue_min[s];
                        }
                    } else {
                        usage++;
                    }
                    control_bit_position += report_size;
                }
                break;

            case ISE_USB_HID_ITEM_OUTPUT:
                break;
            }
            // reset usage queue
            usage_queue_min[0] = 0;
            usage_queue_max[0] = 0;
            usage_queue_index = 0;
            break;

        case ISE_USB_HID_ITEM_GLOBAL:
            switch(ise_usb->config_buffer[i] & ISE_USB_HID_ITEM_MASK) {
            case ISE_USB_HID_ITEM_PUSH:
                if(global_item_stack < 32) {
                    global_item_stack++;
                    for(item=0; item<10; item++) {
                        global_items[global_item_stack][item] = global_items[global_item_stack-1][item];
                    }
                }
                break;
                
            case ISE_USB_HID_ITEM_POP:
                if(global_item_stack > 0) {
                    global_item_stack--;
                }
                break;

            case ISE_USB_HID_ITEM_REPORT_ID:
                // this is a global item, but we don't really need to store it
                control_bit_position = 8;
                // Really need to distinguish the various reports that can be generated
                break;
                
            default:
				if(global_item_stack < 32) {
					item = ise_usb->config_buffer[i] >> 4;
					if(item < 10) {
						bool sign = false;
						uint8_t data;
						global_items[global_item_stack][item] = 0;
						for(s=0; s<item_size; s++) {
							data = ise_usb->config_buffer[i+1+s];
							global_items[global_item_stack][item] |= ((uint32_t) data) << 8*s;
							sign = (data & 0x80) ? true : false;
						}
						// sign extend
						if(sign) global_items[global_item_stack][item] |= ((~0L) << (item_size*8));
					}
				}
                break;
            }
            break;

        case ISE_USB_HID_ITEM_LOCAL:
            usage = (item_size <= 2) ? (global_items[global_item_stack][ISE_USB_HID_ITEM_USAGE_PAGE >> 4] << 16) : 0x0;
            for(s=0; s<item_size; s++) {
                usage |= ((uint32_t) ise_usb->config_buffer[i+1+s]) << 8*s;
            }
            switch(ise_usb->config_buffer[i] & ISE_USB_HID_ITEM_MASK) {
            case ISE_USB_HID_ITEM_USAGE:
                if(usage_queue_index < 32) {
                    usage_queue_min[usage_queue_index] = usage;
                    usage_queue_max[usage_queue_index] = usage;
                    usage_queue_index++;
                }
                break;

            case ISE_USB_HID_ITEM_USAGE_MIN:
                if(usage_queue_index < 32) {
                    usage_queue_min[usage_queue_index] = usage;
                }
                break;

            case ISE_USB_HID_ITEM_USAGE_MAX:
                if(usage_queue_index < 32) {
                    usage_queue_max[usage_queue_index] = usage;
                    usage_queue_index++;
                }
                break;

            // Don't care about the other local items

            }
            break;
        }
        i += item_size;
    }
    if(ise_usb_debug_file) {
        fprintf(ise_usb_debug_file, "HID controls, num controls: %d\n", hid_interface->hid_num_controls);
        ise_mem_dump(ise_usb_debug_file, hid_interface->hid_control, hid_interface->hid_num_controls * 2);
    }

    int queue_select = ise_usb_select_queue(ise_usb->device[config_device_id]->endpoint[ep].interval);
    hid_interface->interrupt_queue = ise_usb->get_queue(ise_usb->hcd, queue_select);

    *((uint32_t*) hid_interface->data) = 0x0;
    *(((uint32_t*) hid_interface->data)+1) = 0x0;
    if(ise_usb_debug_file) {
        fprintf(ise_usb_debug_file, "hid type: 0x%x\n", hid_interface->hid_type);
        ise_mem_dump(ise_usb_debug_file, hid_interface->in_td, 4);
        ise_mem_dump(ise_usb_debug_file, hid_interface->data, 2);
    }

}

void ise_usb_set_next_interface_state(ise_usb_hc_t* ise_usb)
{
    int config_device_id = ise_usb->config_device_id;
    ise_usb->config_state = ISE_USB_CONFIG_STATE_COMPLETE;
    while(ise_usb->config_current_interface < ise_usb->device[config_device_id]->num_interfaces) {
        if(ise_usb->device[config_device_id]->interface[ise_usb->config_current_interface]) {
            if(ise_usb->device[config_device_id]->class_code == ISE_USB_CLASS_HUB) {
                ise_usb->config_state = ISE_USB_CONFIG_STATE_HUB_GET_DESCRIPTOR;
                break;
            } else if(ise_usb->device[config_device_id]->interface[ise_usb->config_current_interface]->class_code == ISE_USB_CLASS_HID) {
                ise_usb->config_state = ISE_USB_CONFIG_STATE_HID_SET_IDLE;
                break;
            }
        }
        ise_usb->config_current_interface++;
    }
}

// enumerates device
int ise_usb_enum_device(ise_usb_hc_t* ise_usb)
{
    ise_usb_td_info_t td_info;
    ise_usb_descriptor_t* desc;
    ise_usb_hid_interface_t* hid_interface;
    ise_usb_external_hub_t* hub_device;
    int config_device_id = ise_usb->config_device_id;
    // get parent hub of the device we're configuring
    ise_usb_hub_t* parent_hub = (ise_usb_hub_t*) ise_usb->device[ise_usb->device[0]->parent_device_id];
    ise_usb_queue_t* ctrl_q = NULL;
    ise_usb_queue_t* host_ctrl_q = ise_usb->get_queue(ise_usb->hcd, ISE_USB_QUEUE_CONTROL);
    if(config_device_id) {
        // if we have an assigned device id, use the device's control queue
        ctrl_q = ise_usb->device[config_device_id]->device_ctrl_q;
    }
    if(ctrl_q == NULL) {
        // otherwise, use the host control queue
        ctrl_q = host_ctrl_q;
    }
    int i, length, ep;
    uint16_t* buffer16;

    // Check if previous requests have completed
    if(ise_usb->num_config_packets > 0) {
        ise_usb_queue_t* prev_ctrl_q = (ise_usb->config_state == ISE_USB_CONFIG_STATE_SET_ADDRESS_ACK) ? host_ctrl_q : ctrl_q;
        // if not all TD's are complete
        if(!(ise_usb->is_td_done(ise_usb->config_td[ise_usb->num_config_packets-1])) ||
           ise_usb->is_td_short(ise_usb->config_td[ise_usb->num_config_packets-1])) {
            // Loop through each config packet
            bool short_packet = false;
            ise_usb_td_t* td;
            for(i = 0; i<ise_usb->num_config_packets; i++) {
                td = ise_usb->config_td[i];
                if(ise_usb->is_td_short(td)) {
                    short_packet = true;
                    ise_usb->stop_queue(prev_ctrl_q);
                    //if(ise_usb_debug_file) {
                    //    fprintf(ise_usb_debug_file, "Short packet\n");
                    //    ise_mem_dump(ise_usb_debug_file, prev_ctrl_q, 20);
                    //}
                }
                // if we found a short packet, deactivate everything else
                if(short_packet) {
                    ise_usb->deactivate_td(td);
                }
            }
        }

        bool is_fatal = ise_usb->is_queue_fatal(prev_ctrl_q);
        int fatal_td = 0;
        for(i = 0; i<ise_usb->num_config_packets; i++) {
            if(ise_usb->is_td_fatal(ise_usb->config_td[i])) {
                is_fatal = true;
                fatal_td = i;
            }
        }

        if(!is_fatal) {
            static bool first_incomplete = true;
            if(ise_usb->is_td_done(ise_usb->config_td[ise_usb->num_config_packets-1])) {
                if(ise_usb_debug_file) {
                    fprintf(ise_usb_debug_file, "TDs complete: config state 0x%x\n", ise_usb->config_state);
                    //ise_mem_dump(ise_usb_debug_file, prev_ctrl_q, 8);
                    //ise_uhci_debug(ise_usb_debug_file, ise_usb);
                    //for(i = 0; i<ise_usb->num_config_packets; i++) {
                    //    ise_mem_dump(ise_usb_debug_file, ise_usb->config_td[i], 5);
                    //}
                    //ise_mem_dump(ise_usb_debug_file, ise_usb->config_req_packet, 2);
                    //ise_mem_dump(ise_usb_debug_file, ise_usb->config_buffer, 2);
                }

                // clear out completed tds
                ise_usb->dequeue_completed_tds(ise_usb->hcd, prev_ctrl_q, false);
                //if(ise_usb_debug_file) ise_mem_dump(ise_usb_debug_file, prev_ctrl_q, 20);

                if(ise_usb->config_state == ISE_USB_CONFIG_STATE_SET_ADDRESS ||
                    ise_usb->config_state == ISE_USB_CONFIG_STATE_SET_ADDRESS_ACK) {
                    // On set address or state afterward, ack is
                    // on the host control queue
                    ise_usb->dequeue_completed_tds(ise_usb->hcd, host_ctrl_q, false);
                }

                // if only packet, it was an ack...no need to ack an ack
                if(ise_usb->num_config_packets > 1) {
                    // acknowledge receipt of the previous request
                    td_info.spd = false;
                    td_info.speed = ise_usb->device[config_device_id]->speed;
                    td_info.ioc = false;
                    td_info.length = 0;
                    td_info.dt = true;
                    td_info.end_pt = 0;
                    // if we're setting address, use device id 0
                    td_info.device = (ise_usb->config_state == ISE_USB_CONFIG_STATE_SET_ADDRESS) ? 0 : config_device_id;
                    // direction is opposite of last request
                    td_info.pid = (ise_usb->config_req_packet->request_type & ISE_USB_REQUEST_TYPE_IN) ? ISE_USB_PID_OUT : ISE_USB_PID_IN;
                    td_info.buffer = NULL;
                    ise_usb->fill_td(ise_usb->config_td[0], &td_info);
                    ise_usb->push_td((ise_usb->config_state == ISE_USB_CONFIG_STATE_SET_ADDRESS) ? host_ctrl_q : ctrl_q, ise_usb->config_td[0]);
                    ise_usb->num_config_packets = 1;
                } else {
                    ise_usb->num_config_packets = 0;
                }
                first_incomplete = true;

            } else {
                if(ise_usb_debug_file) {
                    fprintf(ise_usb_debug_file, "TDs incomplete: config state 0x%x\n", ise_usb->config_state);
                    if(first_incomplete) {
                        ise_mem_dump(ise_usb_debug_file, prev_ctrl_q, 8);
                        //ise_uhci_debug(ise_usb_debug_file, ise_usb);
                        for(i = 0; i<ise_usb->num_config_packets; i++) {
                            ise_mem_dump(ise_usb_debug_file, ise_usb->config_td[i], 5);
                        }
                        ise_mem_dump(ise_usb_debug_file, ise_usb->config_req_packet, 2);
                        ise_mem_dump(ise_usb_debug_file, ise_usb->config_buffer, 2);
                    }
                    first_incomplete = false;
                }
                return 0;
            }
        } else {
            // fatal error
            if(ise_usb_debug_file) {
                fprintf(ise_usb_debug_file, "Fatal TD: config state 0x%x\n", ise_usb->config_state);
                ise_mem_dump(ise_usb_debug_file, prev_ctrl_q, 8);
                for(i = 0; i<ise_usb->num_config_packets; i++) {
                    ise_mem_dump(ise_usb_debug_file, ise_usb->config_td[i], 4);
                }
                ise_mem_dump(ise_usb_debug_file, ise_usb->config_req_packet, 2);
            }
            ise_usb->clear_queue(prev_ctrl_q);
            ise_usb->num_config_packets = 0;
            ise_usb->config_state = ISE_USB_CONFIG_STATE_IDLE;
            parent_hub->disable_port(ise_usb, ise_usb->device[0]->parent_device_id, ise_usb->device[0]->port);
            //ise_usb->config_state = ISE_USB_CONFIG_STATE_START;  // really should be done atomically
            //int port_status = parent_hub->reset_port(ise_usb->hcd, ise_usb->device[0]->port);
            //if(port_status < 0) {
            //    if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Could not reset port!\n");
            //}
            //if(parent_hub->port_device_id[ise_usb->device[0]->port]) {
            //    ise_usb_uninstall_device(ise_usb, parent_hub->port_device_id[ise_usb->device[0]->port]);
            //}
            //if(port_status & 0x80) {
            //    ise_usb->config_device_id = 0;
            //} else {
            //    // go back to idle state, if no device
            //    ise_usb->config_state = ISE_USB_CONFIG_STATE_IDLE;
            //}

            // Check for errors in host controller or TD's
            return 0;
        }

    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Once we have the data for the current state, process it, and go to next state
    switch(ise_usb->config_state) {
    case ISE_USB_CONFIG_STATE_START:
        ise_usb->fill_qh(host_ctrl_q, ise_usb, 0, 0);
        ise_usb->config_state = ISE_USB_CONFIG_STATE_GET_DESC_LENGTH;
        break;

    case ISE_USB_CONFIG_STATE_GET_DESC_LENGTH:
        ise_usb->config_state = ISE_USB_CONFIG_STATE_SET_ADDRESS;
        break;
        
    case ISE_USB_CONFIG_STATE_SET_ADDRESS:
        ise_usb->config_state = (config_device_id == 0) ? ISE_USB_CONFIG_STATE_COMPLETE : ISE_USB_CONFIG_STATE_SET_ADDRESS_ACK;
        break;
        
    case ISE_USB_CONFIG_STATE_SET_ADDRESS_ACK:
        ise_usb->config_state = ISE_USB_CONFIG_STATE_GET_DESC;
        break;

    case ISE_USB_CONFIG_STATE_GET_DESC:
        desc = (ise_usb_descriptor_t*) ise_usb->config_buffer;
        ise_usb->config_manufacturer_string_id = desc->device.manufacturer;
        ise_usb->config_product_string_id = desc->device.product;
        ise_usb->config_state = ISE_USB_CONFIG_STATE_GET_LANG;
        break;
        
    case ISE_USB_CONFIG_STATE_GET_LANG:
        length = *(ise_usb->config_buffer);
        // number of entries in array;
        // remove 2 bytes for length and type
        // divide by 2, since each array element is 2 bytes
        length = (length-2) / 2;
        buffer16 = (uint16_t*) (ise_usb->config_buffer + 2);
        ise_usb->config_lang_id = 0; // no language chosen
        for(i=0; i<length; i++, buffer16++) {
            if(*buffer16 == ISE_USB_LANGID_ENGLISH_US)
                ise_usb->config_lang_id = ISE_USB_LANGID_ENGLISH_US;
            else if((*buffer16 & 0xFF == ISE_USB_LANGID_ENGLISH) && (ise_usb->config_lang_id == 0))
                ise_usb->config_lang_id = *buffer16;
        }
        ise_usb->config_state = ISE_USB_CONFIG_STATE_GET_MANUFACTURER;
        break;

    case ISE_USB_CONFIG_STATE_GET_MANUFACTURER:
        if(ise_usb->config_manufacturer_string_id) {
            length = *(ise_usb->config_buffer);
            // take the length of the descriptor divided by 2 for number of characters to allocate
            // this includes the null terminator, since we didn't remove the first 2 bytes
            length = length / 2;
            ise_usb->device[config_device_id]->manufacturer = (char*) malloc(length);
            ise_usb_ucs2_to_ascii((uint16_t*) (ise_usb->config_buffer + 2),
                ise_usb->device[config_device_id]->manufacturer,
                length-1);
            if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Manufacturer: %s\n", ise_usb->device[config_device_id]->manufacturer);
        }
        
        ise_usb->config_state = ISE_USB_CONFIG_STATE_GET_PRODUCT;
        break;

    case ISE_USB_CONFIG_STATE_GET_PRODUCT:
        if(ise_usb->config_product_string_id) {
            length = *(ise_usb->config_buffer);
            // take the length of the descriptor divided by 2 for number of characters to allocate
            // this includes the null terminator, since we didn't remove the first 2 bytes
            length = length / 2;
            ise_usb->device[config_device_id]->product = (char*) malloc(length);
            ise_usb_ucs2_to_ascii((uint16_t*) (ise_usb->config_buffer + 2),
                ise_usb->device[config_device_id]->product,
                length-1);
            if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Product: %s\n", ise_usb->device[config_device_id]->product);
        }
        
        ise_usb->config_state = ISE_USB_CONFIG_STATE_GET_CONFIG_DESC;
        break;

    case ISE_USB_CONFIG_STATE_GET_CONFIG_DESC:
        desc = (ise_usb_descriptor_t*) ise_usb->config_buffer;
        if(desc->configuration.total_length > ISE_USB_CONFIG_BUFFER_SIZE) {
            // Should allocate separate buffer, and re-try; but abort, for now
            if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "config total length > config buffer size - %d Bytes\n", desc->configuration.total_length);
            ise_usb->config_state = ISE_USB_CONFIG_STATE_COMPLETE;
        } else {
            //if(ise_usb_debug_file) ise_mem_dump(ise_usb_debug_file, desc, (desc->configuration.total_length + 3) / 4);
            int err = ise_usb_parse_configuration_descriptor(ise_usb);
            ise_usb->config_state = (err == 0) ? ISE_USB_CONFIG_STATE_SET_CONFIGURATION : ISE_USB_CONFIG_STATE_COMPLETE;
        }
        break;

    case ISE_USB_CONFIG_STATE_SET_CONFIGURATION:
        ise_usb->config_current_interface = 0;
        ise_usb_set_next_interface_state(ise_usb);
        break;

    case ISE_USB_CONFIG_STATE_HID_SET_IDLE:
        ise_usb->config_state = ISE_USB_CONFIG_STATE_HID_GET_REPORT;
        break;

    case ISE_USB_CONFIG_STATE_HID_GET_REPORT:
        ise_usb_parse_hid_report(ise_usb);
        // Boot devices need to set the protocol to use the HID report
        if(ise_usb->device[config_device_id]->interface[ise_usb->config_current_interface]->subclass_code == ISE_USB_HID_SUBCLASS_BOOT) {
            ise_usb->config_state = ISE_USB_CONFIG_STATE_HID_SET_PROTOCOL;
        } else {
            ise_usb->config_state = ISE_USB_CONFIG_STATE_HID_START;
        }
        break;
        
    case ISE_USB_CONFIG_STATE_HID_SET_PROTOCOL:
        ise_usb->config_state = ISE_USB_CONFIG_STATE_HID_START;
        break;
        
    case ISE_USB_CONFIG_STATE_HID_START:
        hid_interface = (ise_usb_hid_interface_t*) (ise_usb->device[config_device_id]->interface[ise_usb->config_current_interface]);
        ep = hid_interface->in_endpoint;
        td_info.spd = false;
        td_info.speed = ise_usb->device[config_device_id]->speed;
        td_info.ioc = true;
        td_info.length = ise_usb->device[config_device_id]->endpoint[ep].max_packet_size;
        td_info.dt = false;
        td_info.end_pt = ep;
        td_info.device = config_device_id;
        td_info.pid = ISE_USB_PID_IN;
        td_info.buffer = hid_interface->data;
        ise_usb->fill_td(hid_interface->in_td, &td_info);
        ise_usb->enqueue_td(hid_interface->data_queue, hid_interface->in_td);
        ise_usb->restart_queue(ise_usb->hcd, hid_interface->data_queue);
        ise_usb->enqueue_queue(hid_interface->interrupt_queue, hid_interface->data_queue);
        ise_usb->restart_queue(ise_usb->hcd, hid_interface->interrupt_queue);
        ise_usb_hid_interface[ise_usb_num_hid_interfaces] = hid_interface;
        ise_usb_num_hid_interfaces++;

        ise_usb->config_current_interface++;
        ise_usb_set_next_interface_state(ise_usb);
        break;

    case ISE_USB_CONFIG_STATE_HUB_GET_DESCRIPTOR:
        hub_device = (ise_usb_external_hub_t*) (ise_usb->device[config_device_id]);
        desc = (ise_usb_descriptor_t*) ise_usb->config_buffer;
        hub_device->hub.num_ports = desc->hub.num_ports;
        hub_device->potpgt = desc->hub.potpgt;
        ise_usb->config_product_string_id = hub_device->hub.num_ports; // used to loop through each port to power them
        ise_usb->config_state = ISE_USB_CONFIG_STATE_HUB_POWER_PORTS;
        break;

    case ISE_USB_CONFIG_STATE_HUB_POWER_PORTS:
        hub_device = (ise_usb_external_hub_t*) (ise_usb->device[config_device_id]);
        ise_usb->config_product_string_id--;
        if(ise_usb->config_product_string_id==0) {
            ise_usb->config_time = ise_time_get_time();
            ise_usb->config_state = ISE_USB_CONFIG_STATE_HUB_WAIT_FOR_POTPGT;
        }
        break;

    case ISE_USB_CONFIG_STATE_HUB_WAIT_FOR_POTPGT:
        hub_device = (ise_usb_external_hub_t*) (ise_usb->device[config_device_id]);
        if(ise_time_get_time() - ise_usb->config_time > 2*hub_device->potpgt) {
            ise_usb->config_product_string_id = hub_device->hub.num_ports; // used to loop through each port
            ise_usb->config_state = ISE_USB_CONFIG_STATE_HUB_CLEAR_C_PORT_CONNECTION;
        }
        break;

    case ISE_USB_CONFIG_STATE_HUB_CLEAR_C_PORT_CONNECTION:
        hub_device = (ise_usb_external_hub_t*) (ise_usb->device[config_device_id]);
        ise_usb->config_product_string_id--;
        if(ise_usb->config_product_string_id==0) {
            ise_usb->config_product_string_id = hub_device->hub.num_ports; // used to loop through each port
            ise_usb->config_state = ISE_USB_CONFIG_STATE_HUB_GET_PORT_STATUS;
        }
        break;
        
    case ISE_USB_CONFIG_STATE_HUB_GET_PORT_STATUS:
        hub_device = (ise_usb_external_hub_t*) (ise_usb->device[config_device_id]);
        hub_device->port_status[ise_usb->config_product_string_id-1] = ise_usb_hub_convert_port_status(*((uint32_t*) (ise_usb->config_buffer)));
        // if port is connected, set the connection status change for that port
        if(hub_device->port_status[ise_usb->config_product_string_id-1] & 0x1) hub_device->port_status[ise_usb->config_product_string_id-1] |= 0x80;
        ise_usb->config_product_string_id--;
        if(ise_usb->config_product_string_id==0) {
            ise_usb->config_state = ISE_USB_CONFIG_STATE_HUB_START;
        }
        break;
        
    case ISE_USB_CONFIG_STATE_HUB_START:
        hub_device = (ise_usb_external_hub_t*) (ise_usb->device[config_device_id]);
        ep = hub_device->in_endpoint;
        td_info.spd = false;
        td_info.speed = ise_usb->device[config_device_id]->speed;
        td_info.ioc = true;
        td_info.length = ise_usb->device[config_device_id]->endpoint[ep].max_packet_size;
        if(td_info.length > 8) td_info.length = 8;
        td_info.dt = false;
        td_info.end_pt = ep;
        td_info.device = config_device_id;
        td_info.pid = ISE_USB_PID_IN;
        td_info.buffer = hub_device->hub_data;
        ise_usb->fill_td(hub_device->in_td, &td_info);
        ise_usb->enqueue_td(hub_device->data_queue, hub_device->in_td);
        ise_usb->restart_queue(ise_usb->hcd, hub_device->data_queue);
        ise_usb->enqueue_queue(hub_device->interrupt_queue, hub_device->data_queue);
        ise_usb->restart_queue(ise_usb->hcd, hub_device->interrupt_queue);
        ise_usb_hub[ise_usb_num_hubs] = (ise_usb_hub_t*) ise_usb->device[config_device_id];
        ise_usb_num_hubs++;

        ise_usb->config_current_interface++;
        ise_usb_set_next_interface_state(ise_usb);
        break;

    case ISE_USB_CONFIG_STATE_COMPLETE:
        ise_usb->config_device_id = 0;
        ise_usb->config_state = ISE_USB_CONFIG_STATE_IDLE;
        break;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    // For each state, make a request for the data
    switch (ise_usb->config_state) {
    case ISE_USB_CONFIG_STATE_GET_DESC_LENGTH:
        if(ise_usb_debug_file) {
            fprintf(ise_usb_debug_file, "Getting desc length...\n");
            //ise_uhci_debug(ise_usb_debug_file, ise_usb);
        }
        // Create request packet, and place to receive descriptor
        ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_IN | ISE_USB_REQUEST_TYPE_STANDARD | ISE_USB_REQUEST_TYPE_DEVICE;
        ise_usb->config_req_packet->request = ISE_USB_REQUEST_GET_DESCRIPTOR;
        ise_usb->config_req_packet->value = ISE_USB_DESC_TYPE_DEVICE << 8;
        ise_usb->config_req_packet->index = 0;
        ise_usb->config_req_packet->length = 8;

        //ise_mem_dump(ise_usb_debug_file, ctrl_q, 4);
        ise_usb_enqueue_config_request(ise_usb);

        //if(ise_usb_debug_file) {
            //ise_ehci_debug(ise_usb_debug_file, ise_usb);
            //ise_mem_dump(ise_usb_debug_file, (uint32_t*) (*((uint32_t*) ctrl_q + 2) & ~0xF), 4);
        //    ise_mem_dump(ise_usb_debug_file, ctrl_q, 8);
        //}

        //if(ise_usb_debug_file) {
        //    ise_uhci_debug(ise_usb);
        //    ise_mem_dump(ise_usb_debug_file, ctrl_q, 8);
        //    ise_mem_dump(ise_usb_debug_file, ise_usb->config_td[0], 5);
        //    ise_mem_dump(ise_usb_debug_file, ise_usb->config_td[1], 5);
        //    ise_mem_dump(ise_usb_debug_file, ise_usb->config_req_packet, 2);
        //    ise_mem_dump(ise_usb_debug_file, ise_usb->config_buffer, 2);
        //}
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        //ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;
        
    case ISE_USB_CONFIG_STATE_SET_ADDRESS:
        // Set the address for this device
        desc = (ise_usb_descriptor_t*) ise_usb->config_buffer;
        if(ise_usb_debug_file) {
            fprintf(ise_usb_debug_file, "Setting device id...\n");
            fprintf(ise_usb_debug_file, "length = %d\n", desc->device.length);
        }

        for(i=1; i<ISE_USB_MAX_DEVICES; i++) {
            if(ise_usb->device[i] == NULL) {
                // make sure it exists; TODO: make sure it's a hub
                if(parent_hub) {
                    int device_struct_size = sizeof(ise_usb_device_t);
                    if(desc->device.device_class == ISE_USB_CLASS_HUB) {
                        device_struct_size = sizeof(ise_usb_external_hub_t);
                    }
                    ise_usb->device[i] = (ise_usb_device_t*) malloc(device_struct_size);
                    memset(ise_usb->device[i], 0, device_struct_size);
                    ise_usb->device[i]->class_code = desc->device.device_class;
                    ise_usb->device[i]->device_id = i;
                    ise_usb->device[i]->parent_device_id = ise_usb->device[0]->parent_device_id;
                    ise_usb->device[i]->speed = ise_usb->device[0]->speed;
                    ise_usb->device[i]->port = ise_usb->device[0]->port;
                    ise_usb->device[i]->max_packet_size = desc->device.max_packet_size;
                    ise_usb->device[i]->endpoint[0].max_packet_size = desc->device.max_packet_size;
                    config_device_id = i;
                    parent_hub->port_device_id[ise_usb->device[i]->port] = config_device_id;
                } else {
                    if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Parent hub not found\n");
                    config_device_id = 0;
                }
                break;
            }
        }
        if(config_device_id) {
            // Create request packet, and place to receive descriptor
            ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_OUT | ISE_USB_REQUEST_TYPE_STANDARD | ISE_USB_REQUEST_TYPE_DEVICE;
            ise_usb->config_req_packet->request = ISE_USB_REQUEST_SET_ADDRESS;
            ise_usb->config_req_packet->value = config_device_id;
            ise_usb->config_req_packet->index = 0;
            ise_usb->config_req_packet->length = 0;

            ise_usb_enqueue_config_request(ise_usb);
            ise_usb->config_device_id = config_device_id;

            ise_usb->device[config_device_id]->device_ctrl_q = ise_usb->alloc_queue(ise_usb->hcd);
            if(ise_usb->device[config_device_id]->device_ctrl_q) {
                ise_usb->fill_qh(ise_usb->device[config_device_id]->device_ctrl_q, ise_usb, config_device_id, 0);
                ise_usb->enqueue_queue(host_ctrl_q, ise_usb->device[config_device_id]->device_ctrl_q);
            }
        } else {
            // no devices left
            if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "No USB device id's left on host pci s/f 0x%x/0x%x\n", ise_usb->pci_slot, ise_usb->pci_func);
        }
        //if(ise_usb_debug_file) {
        //    ise_uhci_debug(ise_usb);
        //    ise_mem_dump(ise_usb_debug_file, ctrl_q, 8);
        //    ise_mem_dump(ise_usb_debug_file, ise_usb->config_td[0], 4);
        //    ise_mem_dump(ise_usb_debug_file, ise_usb->config_td[1], 4);
        //}
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        //ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;

    case ISE_USB_CONFIG_STATE_SET_ADDRESS_ACK:
        if(ise_usb_debug_file) {
            fprintf(ise_usb_debug_file, "Wait for setting device id ack...\n");
            //ise_mem_dump(ise_usb_debug_file, ctrl_q, 8);
            //ise_mem_dump(ise_usb_debug_file, ise_usb->config_td[0], 4);
        }
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;

    case ISE_USB_CONFIG_STATE_GET_DESC:
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Getting desc...\n");

        // Get the descriptor with the real length
        desc = (ise_usb_descriptor_t*) ise_usb->config_buffer;
        ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_IN | ISE_USB_REQUEST_TYPE_STANDARD | ISE_USB_REQUEST_TYPE_DEVICE;
        ise_usb->config_req_packet->request = ISE_USB_REQUEST_GET_DESCRIPTOR;
        ise_usb->config_req_packet->value = ISE_USB_DESC_TYPE_DEVICE << 8;
        ise_usb->config_req_packet->index = 0;
        ise_usb->config_req_packet->length = desc->device.length;

        ise_usb_enqueue_config_request(ise_usb);
        //if(ise_usb_debug_file) {
        //    ise_uhci_debug(ise_usb);
        //    ise_mem_dump(ise_usb_debug_file, ctrl_q, 8);
        //    ise_mem_dump(ise_usb_debug_file, ise_usb->config_td[0], 4);
        //    ise_mem_dump(ise_usb_debug_file, ise_usb->config_td[1], 4);
        //}
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;

    case ISE_USB_CONFIG_STATE_GET_LANG:
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Getting supported languages...\n");

        ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_IN | ISE_USB_REQUEST_TYPE_STANDARD | ISE_USB_REQUEST_TYPE_DEVICE;
        ise_usb->config_req_packet->request = ISE_USB_REQUEST_GET_DESCRIPTOR;
        ise_usb->config_req_packet->value = ISE_USB_DESC_TYPE_STRING << 8;
        ise_usb->config_req_packet->index = 0;
        ise_usb->config_req_packet->length = 256;
        
        ise_usb_enqueue_config_request(ise_usb);
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;

    case ISE_USB_CONFIG_STATE_GET_MANUFACTURER:
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Getting manufacturer...\n");
        
        if(ise_usb->config_manufacturer_string_id) {
            ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_IN | ISE_USB_REQUEST_TYPE_STANDARD | ISE_USB_REQUEST_TYPE_DEVICE;
            ise_usb->config_req_packet->request = ISE_USB_REQUEST_GET_DESCRIPTOR;
            ise_usb->config_req_packet->value = (ISE_USB_DESC_TYPE_STRING << 8) | ise_usb->config_manufacturer_string_id;
            ise_usb->config_req_packet->index = ise_usb->config_lang_id;
            ise_usb->config_req_packet->length = 256;
        
            ise_usb_enqueue_config_request(ise_usb);
            
        } else {
            if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "No manufacturer string found on device\n");            
        }
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;

    case ISE_USB_CONFIG_STATE_GET_PRODUCT:
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Getting product...\n");
        
        if(ise_usb->config_product_string_id) {
            ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_IN | ISE_USB_REQUEST_TYPE_STANDARD | ISE_USB_REQUEST_TYPE_DEVICE;
            ise_usb->config_req_packet->request = ISE_USB_REQUEST_GET_DESCRIPTOR;
            ise_usb->config_req_packet->value = (ISE_USB_DESC_TYPE_STRING << 8) | ise_usb->config_product_string_id;
            ise_usb->config_req_packet->index = ise_usb->config_lang_id;
            ise_usb->config_req_packet->length = 256;
        
            ise_usb_enqueue_config_request(ise_usb);
            
        } else {
            if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "No manufacturer string found on device\n");            
        }
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;

    case ISE_USB_CONFIG_STATE_GET_CONFIG_DESC:
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Getting configuration descriptor...\n");
        
        // we always use the first configuration
        ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_IN | ISE_USB_REQUEST_TYPE_STANDARD | ISE_USB_REQUEST_TYPE_DEVICE;
        ise_usb->config_req_packet->request = ISE_USB_REQUEST_GET_DESCRIPTOR;
        ise_usb->config_req_packet->value = ISE_USB_DESC_TYPE_CONFIGURATION << 8;
        ise_usb->config_req_packet->index = 0;
        ise_usb->config_req_packet->length = 256;
        
        ise_usb_enqueue_config_request(ise_usb);
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;

    case ISE_USB_CONFIG_STATE_SET_CONFIGURATION:
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Setting configuration...\n");
        
        ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_OUT | ISE_USB_REQUEST_TYPE_STANDARD | ISE_USB_REQUEST_TYPE_DEVICE;
        ise_usb->config_req_packet->request = ISE_USB_REQUEST_SET_CONFIGURATION;
        ise_usb->config_req_packet->value = desc->configuration.configuration_value;
        ise_usb->config_req_packet->index = 0;
        ise_usb->config_req_packet->length = 0;
        
        ise_usb_enqueue_config_request(ise_usb);
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);        
        break;

    case ISE_USB_CONFIG_STATE_HID_SET_IDLE:
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Setting Idle...\n");
        
        ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_OUT | ISE_USB_REQUEST_TYPE_CLASS | ISE_USB_REQUEST_TYPE_INTERFACE;
        ise_usb->config_req_packet->request = ISE_USB_REQUEST_HID_SET_IDLE;
        ise_usb->config_req_packet->value = 0;  // indefinite on all reports
        ise_usb->config_req_packet->index = ise_usb->config_current_interface;
        ise_usb->config_req_packet->length = 0;
        
        ise_usb_enqueue_config_request(ise_usb);
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;

    case ISE_USB_CONFIG_STATE_HID_GET_REPORT:
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Getting HID Report...\n");

        hid_interface = (ise_usb_hid_interface_t*) (ise_usb->device[config_device_id]->interface[ise_usb->config_current_interface]);
        ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_IN | ISE_USB_REQUEST_TYPE_STANDARD | ISE_USB_REQUEST_TYPE_INTERFACE;
        ise_usb->config_req_packet->request = ISE_USB_REQUEST_GET_DESCRIPTOR;
        ise_usb->config_req_packet->value = ISE_USB_DESC_TYPE_HID_REPORT << 8;
        ise_usb->config_req_packet->index = ise_usb->config_current_interface;
        ise_usb->config_req_packet->length = (uint16_t) hid_interface->hid_type;
        
        ise_usb_enqueue_config_request(ise_usb);
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;

    case ISE_USB_CONFIG_STATE_HID_SET_PROTOCOL:
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Setting Protocol...\n");
        
        ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_OUT | ISE_USB_REQUEST_TYPE_CLASS | ISE_USB_REQUEST_TYPE_INTERFACE;
        ise_usb->config_req_packet->request = ISE_USB_REQUEST_HID_SET_PROTOCOL;
        ise_usb->config_req_packet->value = ISE_USB_HID_SUBCLASS_BOOT;
        ise_usb->config_req_packet->index = ise_usb->config_current_interface;
        ise_usb->config_req_packet->length = 0;
        
        ise_usb_enqueue_config_request(ise_usb);
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;

    case ISE_USB_CONFIG_STATE_HID_START:
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Starting HID interface...\n");
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);

        break;

    case ISE_USB_CONFIG_STATE_HUB_GET_DESCRIPTOR:
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Getting Hub descriptor...\n");
        
        ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_IN | ISE_USB_REQUEST_TYPE_CLASS | ISE_USB_REQUEST_TYPE_DEVICE;
        ise_usb->config_req_packet->request = ISE_USB_REQUEST_GET_DESCRIPTOR;
        ise_usb->config_req_packet->value = ISE_USB_DESC_TYPE_HUB << 8;
        ise_usb->config_req_packet->index = 0;
        ise_usb->config_req_packet->length = sizeof(ise_usb_hub_descriptor_t);
        
        ise_usb_enqueue_config_request(ise_usb);
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;
        
    case ISE_USB_CONFIG_STATE_HUB_POWER_PORTS:
        hub_device = (ise_usb_external_hub_t*) (ise_usb->device[config_device_id]);
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Powering hub port %d...\n", ise_usb->config_product_string_id);

        ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_OUT | ISE_USB_REQUEST_TYPE_CLASS | ISE_USB_REQUEST_TYPE_OTHER;
        ise_usb->config_req_packet->request = ISE_USB_REQUEST_SET_FEATURE;
        ise_usb->config_req_packet->value = ISE_USB_HUB_REQUEST_PORT_POWER;
        ise_usb->config_req_packet->index = ise_usb->config_product_string_id;
        ise_usb->config_req_packet->length = 0;
        
        ise_usb_enqueue_config_request(ise_usb);
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;

    case ISE_USB_CONFIG_STATE_HUB_WAIT_FOR_POTPGT:
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Waiting for POTPGT...\n");
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;

    case ISE_USB_CONFIG_STATE_HUB_CLEAR_C_PORT_CONNECTION:
        hub_device = (ise_usb_external_hub_t*) (ise_usb->device[config_device_id]);
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Clearing port %d connection change status...\n", ise_usb->config_product_string_id);

        ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_OUT | ISE_USB_REQUEST_TYPE_CLASS | ISE_USB_REQUEST_TYPE_OTHER;
        ise_usb->config_req_packet->request = ISE_USB_REQUEST_CLEAR_FEATURE;
        ise_usb->config_req_packet->value = ISE_USB_HUB_REQUEST_C_PORT_CONNECTION;
        ise_usb->config_req_packet->index = ise_usb->config_product_string_id;
        ise_usb->config_req_packet->length = 0;
        
        ise_usb_enqueue_config_request(ise_usb);
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;
        
    case ISE_USB_CONFIG_STATE_HUB_GET_PORT_STATUS:
        hub_device = (ise_usb_external_hub_t*) (ise_usb->device[config_device_id]);
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Get port %d status...\n", ise_usb->config_product_string_id);

        ise_usb->config_req_packet->request_type = ISE_USB_REQUEST_TYPE_IN | ISE_USB_REQUEST_TYPE_CLASS | ISE_USB_REQUEST_TYPE_OTHER;
        ise_usb->config_req_packet->request = ISE_USB_REQUEST_GET_STATUS;
        ise_usb->config_req_packet->value = ISE_USB_HUB_REQUEST_PORT_CONNECTION;
        ise_usb->config_req_packet->index = ise_usb->config_product_string_id;
        ise_usb->config_req_packet->length = 4;
        
        ise_usb_enqueue_config_request(ise_usb);
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;
        
    case ISE_USB_CONFIG_STATE_HUB_START:
        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Starting Hub device...\n");
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);

        break;

    case ISE_USB_CONFIG_STATE_COMPLETE:
        if(ise_usb_debug_file) {
            fprintf(ise_usb_debug_file, "Config complete\n");
            //ise_mem_dump(ise_usb_debug_file, ise_usb->config_buffer, 8);
        }
        ise_usb->restart_queue(ise_usb->hcd, ctrl_q);
        ise_usb->restart_queue(ise_usb->hcd, host_ctrl_q);
        break;
    }

    return 0;
}

// Get HID input
int32_t ise_usb_hid_parse_data(ise_usb_hid_control_t* control, uint32_t* data)
{
    uint8_t length = control->length;
    if(length == 0) return 0;
    
    int index = (control->shift) >> 5;
    int offset = (control->shift) & 0x1f;
    int32_t data0 = *(data + index) >> offset;
    int32_t data1 = (offset > 0) ? *(data + index + 1) << (32-offset) : 0;

    int32_t parsed_data = ~((~0L) << length);
    parsed_data &= (data0 | data1);

    /*if(control->shift >= 32) {
        parsed_data &= (*(data + 1)) >> (control->shift - 32);
    } else if(control->shift > 0) {
        parsed_data &= ((*data) >> control->shift) | ((*(data + 1)) << (32 - control->shift));
    } else {
        parsed_data &= *data;
    }*/

    // the control is signed, and the top bit is set, sign extend
    if((control->flags & 0x80) && (parsed_data & (1 << (length-1)))) parsed_data |=  (~0L) << length;

    return parsed_data;
}

uint32_t ise_usb_hid_get_buttons(ise_usb_hid_interface_t* hid_interface)
{
    uint32_t buttons = 0x0;
    uint32_t button_num;
    int i;
    for(i=0; i<hid_interface->hid_num_controls; i++) {
        if((hid_interface->hid_control[i].control_type >> 16) == ISE_USB_HID_USAGE_PAGE_BUTTONS) {
            button_num = hid_interface->hid_control[i].control_type & 0xFFFF;
            if(button_num > 0) button_num--;
            buttons |= ise_usb_hid_parse_data(&(hid_interface->hid_control[i]), (uint32_t*) hid_interface->data) << button_num;
        } else if(hid_interface->hid_control[i].control_type == ISE_USB_HID_USAGE_DESKTOP_START) {
            button_num = 31;
            buttons |= ise_usb_hid_parse_data(&(hid_interface->hid_control[i]), (uint32_t*) hid_interface->data) << button_num;
        } else if(hid_interface->hid_control[i].control_type == ISE_USB_HID_USAGE_DESKTOP_SELECT) {
            button_num = 30;
            buttons |= ise_usb_hid_parse_data(&(hid_interface->hid_control[i]), (uint32_t*) hid_interface->data) << button_num;
        }
    }
    return buttons;
}

int32_t ise_usb_hid_get_control(ise_usb_hid_interface_t* hid_interface, uint32_t control)
{
    int32_t value = 0;
    int i;
    for(i=0; i<hid_interface->hid_num_controls; i++) {
        if(hid_interface->hid_control[i].control_type == control) {
            if(hid_interface->hid_type == ISE_USB_HID_USAGE_DESKTOP_MOUSE && hid_interface->nak_count > 3) {
                value = 0;
            } else {
                value = ise_usb_hid_parse_data(&(hid_interface->hid_control[i]), (uint32_t*) hid_interface->data);
            }
        }
    }
    return value;
}

// This function is timing sensitive, and should be called by an interrupt
// ideally this should be done by the host controller interrupt - but this isn't working (crashing in dos32a?)
// so instead, using PIT timer interrupt
void ise_usb_hid_tick()
{
    int i, status;
    ise_usb_hc_t* ise_usb_ptr;
    ise_usb_hid_interface_t* hid_interface;
    ise_usb_td_t* td;
    for(i=0; i<ise_usb_num_hid_interfaces; i++) {
        hid_interface = ise_usb_hid_interface[i];
        ise_usb_ptr = &ise_usb[hid_interface->intf.host_id];
        ise_usb_endpoint_t endpoint = hid_interface->device->endpoint[hid_interface->in_endpoint];
        td = hid_interface->in_td;

        /*if(ise_usb_ptr->is_queue_done(hid_interface->data_queue)) {
            status = ise_usb_ptr->restart_td(td, (uint32_t) hid_interface->data, endpoint.max_packet_size);
            ise_usb_ptr->restart_queue(ise_usb_ptr->hcd, hid_interface->data_queue);
            if(ise_usb_ptr->is_queue_done(hid_interface->interrupt_queue)) ise_usb_ptr->restart_queue(ise_usb_ptr->hcd, hid_interface->interrupt_queue);
            if(status != 0) {
                if(hid_interface->nak_count < 255) hid_interface->nak_count++;
            } else {
                hid_interface->nak_count = 0;
            }
            hid_tick++;
        }*/
        status = ise_usb_ptr->restart_td(td, (uint32_t) hid_interface->data, endpoint.max_packet_size);
        if(status >= 0) {
            ise_usb_ptr->restart_queue(ise_usb_ptr->hcd, hid_interface->data_queue);
            if(ise_usb_ptr->is_queue_done(hid_interface->interrupt_queue)) ise_usb_ptr->restart_queue(ise_usb_ptr->hcd, hid_interface->interrupt_queue);
            if(status == 1) {
                if(hid_interface->nak_count < 255) hid_interface->nak_count++;
            } else {
                hid_interface->nak_count = 0;
            }
            hid_tick++;
        } else if(hid_interface->nak_count < 255) hid_interface->nak_count++;
    }

}

// Should be called by host controller interrupt, but it's not working so using timer interrupt
void ise_usb_hub_tick()
{
    int i, p;
    ise_usb_hc_t* ise_usb_ptr;
    ise_usb_external_hub_t* hub_device;
    ise_usb_td_t* td;
    // the first num_hosts hubs are root hubs, so no need to process them
    for(i=ise_usb_num_hosts; i<ise_usb_num_hubs; i++) {
        hub_device = (ise_usb_external_hub_t*) ise_usb_hub[i];
        if(hub_device) {
            ise_usb_ptr = &ise_usb[hub_device->hub.device.interface[0]->host_id];
            ise_usb_endpoint_t endpoint = hub_device->hub.device.endpoint[hub_device->in_endpoint];
            td = hub_device->in_td;
            //if(ise_usb_debug_file) {
            //    fprintf(ise_usb_debug_file, "Hub tick\n");
            //    ise_mem_dump(ise_usb_debug_file, hub_device->data_queue, 20);
                //ise_mem_dump(ise_usb_debug_file, (uint32_t*) hub_device->data_queue + 3, 4);
            //}
            if(ise_usb_ptr->restart_td(td, (uint32_t) hub_device->hub_data, endpoint.max_packet_size) >= 0) {
                //if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Hub data: 0x%x.\n", hub_device->hub_data[0]);
                for(p=1; p<=hub_device->hub.num_ports; p++) {
                    if(hub_device->hub_data[p/32] & (1 << (p&0x1f))) hub_device->port_status[p-1] |= 0x80;
                }
                // Clear the data after capturing the port changes
                for(p=0;p<=hub_device->hub.num_ports/32; p++) {
                    hub_device->hub_data[p] = 0;
                }
                ise_usb_ptr->restart_queue(ise_usb_ptr->hcd, hub_device->data_queue);
                if(ise_usb_ptr->is_queue_done(hub_device->interrupt_queue)) ise_usb_ptr->restart_queue(ise_usb_ptr->hcd, hub_device->interrupt_queue);
                //if(ise_usb_debug_file) ise_mem_dump(ise_usb_debug_file, hub_device->data_queue, 20);
            }
        }
    }
}

// periodically called to check usb port status
void ise_usb_tick()
{
    int hub_id, host_id;

    int hid_id;
    for(hid_id=0; hid_id < ise_usb_num_hid_interfaces; hid_id++) {
        ise_usb_hid_interface_t* hid_interface = ise_usb_hid_interface[hid_id];
        int x, y, rx, ry;
        uint32_t buttons;
        x = ise_usb_hid_get_control(hid_interface, ISE_USB_HID_USAGE_DESKTOP_X);
        y = ise_usb_hid_get_control(hid_interface, ISE_USB_HID_USAGE_DESKTOP_Y);
        rx = ise_usb_hid_get_control(hid_interface, ISE_USB_HID_USAGE_DESKTOP_Z);
        ry = ise_usb_hid_get_control(hid_interface, ISE_USB_HID_USAGE_DESKTOP_RZ);
        buttons = ise_usb_hid_get_buttons(hid_interface);
        //uint32_t* data = (uint32_t*) hid_interface->data;
        //ise_mem_dump(NULL, data, 2);
        //printf("id: %d x: %d y: %d rx: %d ry: %d buttons: 0x%08x\n", hid_id, x, y, rx, ry, buttons);
    }

    ise_usb_hub_t* hub;
    ise_usb_hc_t* ise_usb_ptr;
    for(hub_id=0; hub_id<ise_usb_num_hubs; hub_id++) {
        /*
        for(d=1; d<ISE_USB_MAX_DEVICES; d++) {
            if(ise_usb[h].device[d] && d != ise_usb[h].config_device_id) {
                for(i=0; i<ise_usb[h].device[d]->num_interfaces; i++) {
                    if(ise_usb[h].device[d]->interface[i]->class_code == ISE_USB_CLASS_HID) {
                        ise_usb_hid_tick(&ise_usb[h], (ise_usb_hid_interface_t*) ise_usb[h].device[d]->interface[i]);
                    }
                }
            }
        }
        */
        hub = ise_usb_hub[hub_id];
        if(hub) {
            host_id = hub->device.interface[0]->host_id;
            ise_usb_ptr = &(ise_usb[host_id]);
            if(ise_usb_ptr->config_state != ISE_USB_CONFIG_STATE_IDLE) {
                // check if port changed, and we're on the hub being configured
                if(hub->device.device_id == ise_usb_ptr->device[0]->parent_device_id &&
                   hub->port_changed(ise_usb_ptr, hub->device.device_id, ise_usb_ptr->device[0]->port) >= 0) {
                    // if so, restart device enumeration
                    ise_usb_ptr->config_state = ISE_USB_CONFIG_STATE_IDLE;
                    ise_usb_queue_t* ctrl_q = ise_usb_ptr->get_queue(ise_usb_ptr->hcd, ISE_USB_QUEUE_CONTROL);
                    ise_usb_ptr->clear_queue(ctrl_q);
                    ise_usb_ptr->num_config_packets = 0;
                    if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Port changed on device being enumerated.\n");
                } else {
                    // otherwise, continue enumeration
                    ise_usb_enum_device(ise_usb_ptr);
                }
            }

            int p, port_status;
            int num_ports = hub->num_ports;
            for(p=0; p<num_ports && ise_usb_ptr->config_state == ISE_USB_CONFIG_STATE_IDLE; p++) {
                port_status = hub->port_changed(ise_usb_ptr, hub->device.device_id, p);
                if(port_status >= 0) {
                    if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "port changed - status: 0x%x\n", port_status);
                    //printf("num hosts %d num hubs %d port changed %d\n", ise_usb_num_hosts, ise_usb_num_hubs, port_status);
                    ise_usb_ptr->config_state = ISE_USB_CONFIG_STATE_START;  // really should be done atomically
                    port_status = hub->reset_port(ise_usb_ptr, hub->device.device_id, p);
                    if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "port reset - status: 0x%x\n", port_status);
                    if(port_status < 0) {
                        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Could not reset port!\n");
                        port_status = 0;
                    }
                    if(hub->port_device_id[p]) {
                        ise_usb_uninstall_device(ise_usb_ptr, hub->port_device_id[p]);
                    }
                    if(ise_usb_debug_file) fprintf(ise_usb_debug_file,
                        "USB device slot/func 0x%x/0x%x device %d port %d - %s\n",
                        ise_usb_ptr->pci_slot, ise_usb_ptr->pci_func, hub->device.device_id, p,
                        ((port_status & 0x80) ? "device connected" : "device disconnected"));
                    if(port_status & 0x80) {
                        ise_usb_ptr->config_device_id = 0;
                        ise_usb_ptr->num_config_packets = 0;
                        ise_usb_ptr->device[0]->parent_device_id = hub->device.device_id;
                        ise_usb_ptr->device[0]->speed = port_status & 0x7;
                        ise_usb_ptr->device[0]->port = p;
                        ise_usb_enum_device(ise_usb_ptr);
                    } else {
                        // go back to idle state, if no device
                        ise_usb_ptr->config_state = ISE_USB_CONFIG_STATE_IDLE;
                    }
                }
            }
        }


        /*if(ise_usb[h].config_state != ISE_USB_CONFIG_STATE_IDLE) {
            // check if port changed
            if(root_hub->port_changed(&(ise_usb[h]), ise_usb[h].device[0]->port) >= 0) {
                // if so, restart device enumeration
                ise_usb[h].config_state = ISE_USB_CONFIG_STATE_IDLE;
                ise_usb_queue_t* ctrl_q = ise_usb[h].get_queue(ise_usb[h].hcd, ISE_USB_QUEUE_CONTROL);
                ise_usb[h].clear_queue(ctrl_q);
                ise_usb[h].num_config_packets = 0;
                if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Port changed on device being enumerated.\n");
            } else {
                // otherwise, continue enumeration
                ise_usb_enum_device(&ise_usb[h]);
            }
        }
        if(root_hub) {  // check that we have root hub
            int p, port_status;
            for(p=0; p<root_hub->num_ports && ise_usb[h].config_state == ISE_USB_CONFIG_STATE_IDLE; p++) {
                port_status = root_hub->port_changed(&(ise_usb[h]), p);
                if(port_status >= 0) {
                    ise_usb[h].config_state = ISE_USB_CONFIG_STATE_START;  // really should be done atomically
                    port_status = root_hub->reset_port(&(ise_usb[h]), p);
                    if(port_status < 0) {
                        if(ise_usb_debug_file) fprintf(ise_usb_debug_file, "Could not reset port!\n");
                    }
                    if(root_hub->port_device_id[p]) {
                        ise_usb_uninstall_device(&ise_usb[h], root_hub->port_device_id[p]);
                    }
                    if(ise_usb_debug_file) fprintf(ise_usb_debug_file,
                        "USB device slot/func 0x%x/0x%x port %d - %s\n",
                        ise_usb[h].pci_slot, ise_usb[h].pci_func, p,
                        ((port_status & 0x80) ? "device connected" : "device disconnected"));
                    if(port_status & 0x80) {
                        ise_usb[h].config_device_id = 0;
                        ise_usb[h].num_config_packets = 0;
                        ise_usb[h].device[0]->parent_device_id = 0;
                        ise_usb[h].device[0]->speed = port_status & 0x7;
                        ise_usb[h].device[0]->port = p;
                        ise_usb_enum_device(&ise_usb[h]);
                    } else {
                        // go back to idle state, if no device
                        ise_usb[h].config_state = ISE_USB_CONFIG_STATE_IDLE;
                    }
                }
            }
        }*/
    }
}

void _interrupt FAR ise_usb_isr()
{
    /*int h;
    for(h=0; h<ise_usb_num_hosts; h++) {
        // Check if the interrupt matches this devices interrupt
        if(ise_usb[h].interrupt_num == interrupt_num) {
            // Check if host controller is in the middle of configuring a device
            if(ise_usb[h].config_state != ISE_USB_CONFIG_STATE_IDLE) {
                // continue enumeration
                ise_usb_enum_device(&ise_usb[h]);
            }
        }
    }*/
    ise_usb_time_tick++;

    // process all hid interfaces
    ise_usb_hid_tick();

    // comparing to 1, so that hub_tick doesn't get called on the same time slice as
    // the default timer routines
    if((ise_usb_time_tick & ISE_USB_HUB_TIME_MASK) == 1) {
        ise_usb_hub_tick();
    }

    if(ise_usb_time_tick & ISE_USB_TIME_MASK) {
        outp(ISE_IRQ_PIC0_COMMAND, 0x20);
    } else {
        timer_tick++;
        _chain_intr(ise_usb_prev_timer_isr);
    }
    //outp(ISE_IRQ_PIC1_COMMAND, 0x20);
    //outp(ISE_IRQ_PIC0_COMMAND, 0x20);
}
