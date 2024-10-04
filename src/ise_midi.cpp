// InfiniScroll Engine
// ise_midi.cpp
// Midi related functions
//
// Kamal Pillai
// 4/17/2019

#include "ise.h"

// Reads a value of length size in beg-endian format froma  file
void ise_midi_fread_bigend(FILE* file, void* data, uint8_t length)
{
    int i;
    uint8_t* d = (uint8_t*) data;
    for(i=length-1; i >=0 ; i--) {
        fread(d + i, 1, 1, file);
    }
}

// Reads a midi-style variable length value
// returns number of bytes read
int ise_midi_fread_var_bigend(FILE* file, uint32_t* data)
{
    int i;
    bool done = false;
    uint8_t new_data;
    *data = 0;
    for(i=0; i<4 && !done; i++) {
        fread(&new_data, 1, 1, file);
        *data <<= 7;
        *data |= new_data & 0x7f;
        if((new_data & 0x80) == 0) done = true;
    }
    return i;
}

// parses midi track poionted to by file, and stores in midi event array, if non-NULL
// returns the number of events in the current track
// File should be open with the position of the track to be parsed
int ise_midi_parse_track(FILE* file, ise_midi_header_t* header, ise_midi_event_t* track, int* base_opl_channel)
{
    int num_events = 0;
    int cur_file_pos;
    ise_midi_event_t* cur_event = track;
    
    uint32_t chunk_type;
    uint32_t chunk_length;
    uint32_t running_length = 0;
    uint32_t next_delta_time, delta_time = 0;
    int time = 0;
    uint8_t i, c0, c1, event_type, meta_event_type, midi_channel, midi_event, ch;
    uint32_t length;
    int max_opl_channels = 16; // can go up to 18 in non-percussion mode, voice 15 is for percussion
    int num_opl_channel_used = 0;
    uint8_t midi_channel_voice[16] = {0};
    uint8_t midi_channel_volume[16] = {0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
                                       0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F};
    ise_midi_opl_channel_t opl_channel[18];
    memset(opl_channel, -1, 18*sizeof(ise_midi_opl_channel_t));
    opl_channel[15].midi_channel = 9; // percussion
    ise_midi_event_t* op_channel_last_note_off_event[18] = {NULL}; // indicate last event where a note was turned off
    int op_channel_last_note_off_time[18] = {0};  // indicates last time a note was turned off
    int op_channel_reset_threshold_time = 16*header->division;
    
    ise_midi_fread_bigend(file, &chunk_type, sizeof(uint32_t));
    ise_midi_fread_bigend(file, &chunk_length, sizeof(uint32_t));
    if(chunk_type == ISE_MIDI_TRACK_CHUNK_ID) {
        // We have a valid midi track chunk
        while(running_length < chunk_length) {
            running_length += ise_midi_fread_var_bigend(file, &next_delta_time);
            delta_time += next_delta_time;
            if(delta_time >= 0xFF) {
                if(cur_event) {
                    cur_event->data = 0xFF000000 | delta_time;
                    cur_event++;
                }
                num_events++;
                delta_time = 0;
            }
            time += next_delta_time;
            cur_file_pos = ftell(file);
            running_length += fread(&c0, 1, 1, file);
            if(c0 & 0x80) {
                event_type = c0;
                if(event_type != ISE_MIDI_EVENT_TYPE_SYSEX && event_type != ISE_MIDI_EVENT_TYPE_SYSEX2 && event_type != ISE_MIDI_EVENT_TYPE_META) {
                    running_length += fread(&c0, 1, 1, file);
                }
            }
            //printf("fpos 0x%x (d %d, t %d): ", cur_file_pos, next_delta_time, time);
            switch(event_type) {
            case ISE_MIDI_EVENT_TYPE_SYSEX:
            case ISE_MIDI_EVENT_TYPE_SYSEX2:
                // first get length, then skip over sysex events
                running_length += ise_midi_fread_var_bigend(file, &length);
                fseek(file, length, SEEK_CUR);
                running_length += length;
                //printf("sysex event, length = %d\n", length);
                break;
            case ISE_MIDI_EVENT_TYPE_META:
                running_length += fread(&meta_event_type, 1, 1, file);
                running_length += ise_midi_fread_var_bigend(file, &length);
                switch(meta_event_type) {
                case ISE_MIDI_META_EVENT_TYPE_CHAN_PREFIX:
                case ISE_MIDI_META_EVENT_TYPE_END_TRACK:
                case ISE_MIDI_META_EVENT_TYPE_SET_TEMPO:
                    if(cur_event) {
                        cur_event->data = 0;
                        cur_event->f.delta_time = (uint8_t) delta_time;
                        cur_event->f.command = meta_event_type;
                        if(length == 1) {
                            running_length += fread(&cur_event->f.data0, 1, 1, file);
                        }
                        if(meta_event_type == ISE_MIDI_META_EVENT_TYPE_END_TRACK) {
                            for(i=*base_opl_channel; i<max_opl_channels; i++) {
                                if(op_channel_last_note_off_event[i]) {
                                    op_channel_last_note_off_event[i]->f.data1 = 0xFF;
                                }
                            }
                        }
                        if(meta_event_type == ISE_MIDI_META_EVENT_TYPE_SET_TEMPO) {
                            uint32_t usec_per_qtr_note = 0;
                            ise_midi_fread_bigend(file, &usec_per_qtr_note, 3);
                            running_length += 3;
                            uint32_t systime_per_tick = usec_per_qtr_note / (215* header->division);
                            if(systime_per_tick >= 0x10000) {
                                printf("Set tempo overflow!\n");
                            }
                            cur_event->data |= systime_per_tick & 0xFFFF;
                        }
                        cur_event++;
                    } else {
                        fseek(file, length, SEEK_CUR);
                        running_length += length;
                    }
                    num_events++;
                    delta_time = 0;
                    break;
                default:
                    fseek(file, length, SEEK_CUR);
                    running_length += length;
                    break;
                }
                //printf("meta event 0x%x, length = %d\n", meta_event_type, length);
                break;
            default:
                midi_channel = event_type & 0xF;
                midi_event = event_type & 0xF0;
                if(midi_event == ISE_MIDI_MSG_NOTE_OFF ||
                   midi_event == ISE_MIDI_MSG_NOTE_ON ||
                   midi_event == ISE_MIDI_MSG_CONTROL_CHANGE ||
                   midi_event == ISE_MIDI_MSG_PITCH_WHEEL || 
                   midi_event == ISE_MIDI_MSG_KEY_PRESSURE) {
                       running_length += fread(&c1, 1, 1, file);
                       if(midi_event == ISE_MIDI_MSG_NOTE_ON && c1 == 0) {
                           // if note on and velocity is 0, it's really an off message
                           midi_event = ISE_MIDI_MSG_NOTE_OFF;
                       }
                }
                // Midi messages - channel set in lower 4 bits of event_type
                switch(midi_event) {
                case ISE_MIDI_MSG_NOTE_OFF:
                    for(i=*base_opl_channel; i<max_opl_channels; i++) {
                        if(opl_channel[i].midi_channel == midi_channel && (opl_channel[i].note == c0 || i == 15)) {
                            if(cur_event) {
                                op_channel_last_note_off_event[i] = cur_event;
                                cur_event->f.delta_time = (uint8_t) delta_time;
                                cur_event->f.command = midi_event | i;
                                cur_event->f.data0 = c0;
                                cur_event->f.data1 = 0x0;
                                cur_event++;
                            }
                            op_channel_last_note_off_time[i] = time;
                            opl_channel[i].note = -1;
                            num_events++;
                            delta_time = 0;
                        }
                    }
                    break;

                case ISE_MIDI_MSG_NOTE_ON:
                    ch = 0xFF;
                    for(i=*base_opl_channel; i<max_opl_channels; i++) {
                        if(opl_channel[i].midi_channel == midi_channel && (opl_channel[i].note == -1 || i ==15)) {
                            ch = i;
                            break;
                        }
                        if(opl_channel[i].midi_channel == -1 && ch == 0xFF) {
                            ch = i;
                        }
                    }
                    if(ch != 0xFF) {
                        if((opl_channel[ch].midi_channel == -1) || ((time - op_channel_last_note_off_time[ch]) > op_channel_reset_threshold_time)) {
                            if(op_channel_last_note_off_event[ch]) {
                                op_channel_last_note_off_event[ch]->f.data1 = 0xFF;
                            }
                            if(cur_event) {
                                cur_event->f.delta_time = (uint8_t) delta_time;
                                cur_event->f.command = ISE_MIDI_MSG_PROGRAM_CHANGE | ch;
                                cur_event->f.data0 = midi_channel_voice[midi_channel];
                                cur_event++;
                            }
                            opl_channel[ch].midi_channel = midi_channel;
                            num_events++;
                            delta_time = 0;
                        }
                        op_channel_last_note_off_event[ch] = NULL;
                        op_channel_last_note_off_time[ch] = 0x7FFFFFFF;
                        if(cur_event) {
                            cur_event->f.delta_time = (uint8_t) delta_time;
                            cur_event->f.command = midi_event | ch;
                            cur_event->f.data0 = c0;
                            cur_event->f.data1 = (c1 * midi_channel_volume[midi_channel]) >> 7;
                            cur_event++;
                        }
                        opl_channel[ch].note = c0;
                        num_events++;
                        delta_time = 0;
                        if(ch >= num_opl_channel_used) num_opl_channel_used = ch + 1;
                    } else {
                        printf("fpos 0x%x (d %d, t %d): ", cur_file_pos, next_delta_time, time);
                        printf("Midi: unable to find free voice!\n");
                        ise_mem_dump(NULL, opl_channel, 15);
                    }
                    break;
                        
                //case ISE_MIDI_MSG_CONTROL_CHANGE:
                case ISE_MIDI_MSG_PITCH_WHEEL:
                    for(i=*base_opl_channel; i<max_opl_channels; i++) {
                        if(opl_channel[i].midi_channel == midi_channel) {
                            if(cur_event) {
                                cur_event->f.delta_time = (uint8_t) delta_time;
                                cur_event->f.command = midi_event | i;
                                cur_event->f.data0 = c0;
                                cur_event->f.data1 = c1;
                                cur_event++;
                            }
                            //op_channel_last_note_off_event[i] = NULL;
                            //op_channel_last_note_off_time[i] = 0x7FFFFFFF;
                            num_events++;
                            delta_time = 0;
                        }
                    }
                    break;

                case ISE_MIDI_MSG_PROGRAM_CHANGE:
                    if(midi_channel != 9) {
                        midi_channel_voice[midi_channel] = c0;
                        for(i=*base_opl_channel; i<max_opl_channels; i++) {
                            if(opl_channel[i].midi_channel == midi_channel) {
                                opl_channel[i].midi_channel = -1;
                            }
                        }
                    }
                    break;

                case ISE_MIDI_MSG_CONTROL_CHANGE:
                    switch(c0) {
                    case ISE_MIDI_CTRL_MAIN_VOLUME:
                        midi_channel_volume[midi_channel] = c1;
                        break;
                    }
                    break;

                case ISE_MIDI_MSG_KEY_PRESSURE:
                case ISE_MIDI_MSG_CHANNEL_PRESSURE:
                    break;
                         
                default:
                    printf("fpos 0x%x (d %d, t %d): ", cur_file_pos, next_delta_time, time);
                    printf("Unknown midi message 0x%x\n", event_type);
                    //return num_events;
                }
            }
        }
    }
    *base_opl_channel = num_opl_channel_used;
    //printf("chunk length = %d, running length = %d\n", chunk_length, running_length);
    return num_events;
}

uint32_t ise_midi_get_delta_time(ise_midi_event_t** e)
{
    uint32_t next_delta_time = (*e)->f.delta_time;
    uint32_t delta_time = 0;
    while(next_delta_time == 0xFF) {
        delta_time += (*e)->data & 0xFFFFFF;
        (*e)++;
        next_delta_time = (*e)->f.delta_time;
    }
    delta_time += next_delta_time;
    return delta_time;
}

void ise_midi_set_delta_time(ise_midi_event_t** e, uint32_t delta_time)
{
    if(delta_time >= 0xFF) {
        (*e)->data = delta_time | 0xFF000000;
        (*e)++;
        (*e)->f.delta_time = 0x0;
    } else {
        (*e)->f.delta_time = (uint8_t) delta_time;
    }
}

// combines 2 tracks into a single track
void ise_midi_mix_tracks(ise_midi_event_t* dest, ise_midi_event_t* src0, ise_midi_event_t* src1, int* num_channels_used)
{
    bool src0_done = (src0->f.command == ISE_MIDI_META_EVENT_TYPE_END_TRACK);
    bool src1_done = (src1->f.command == ISE_MIDI_META_EVENT_TYPE_END_TRACK);
    uint32_t src0_delta_time = ise_midi_get_delta_time(&src0);
    uint32_t src1_delta_time = ise_midi_get_delta_time(&src1);
    uint8_t channel_map[16];
    memset(channel_map, 0xFF, 16);
    channel_map[15] = 15;  // percussion must use this channel
    *num_channels_used = 0;
    uint32_t channels_used = 0x0;

    //printf("Start mix...\n");
    while(!src0_done || !src1_done) {
        if((src0_delta_time <= src1_delta_time && !src0_done) || src1_done) {
            ise_midi_set_delta_time(&dest, src0_delta_time);
            dest->data &= 0xFF000000;
            dest->data |= src0->data & 0xFFFFFF;
            //printf("src0: 0x%x %d\n", src0->data, src0_done);
            src0++;
            src1_delta_time -= src0_delta_time;
            src0_delta_time = ise_midi_get_delta_time(&src0);
            if(src0->f.command == ISE_MIDI_META_EVENT_TYPE_END_TRACK) src0_done = true;
        } else {
            ise_midi_set_delta_time(&dest, src1_delta_time);
            dest->data &= 0xFF000000;
            dest->data |= src1->data & 0xFFFFFF;
            //printf("src1: 0x%x\n", src1->data);
            src1++;
            src0_delta_time -= src1_delta_time;
            src1_delta_time = ise_midi_get_delta_time(&src1);
            if(src1->f.command == ISE_MIDI_META_EVENT_TYPE_END_TRACK) src1_done = true;
        }
        if(dest->f.command & 0x80) {
            // midi event - need to remap channels
            uint8_t channel = dest->f.command & 0xF;
            // check if channel is unmapped
            if(channel_map[channel] == 0xFF) {
                // if unmapped, allocate the first available channel
                int i;
                // pick the first available channel
                for(i=0; i<16; i++) {
                    if(((channels_used >> i) & 1) == 0) {
                        break;
                    }
                }
                // assign the mapping
                channel_map[channel] = i;
                channels_used |= (1 << i);
                //printf("alloc ch %d to %d  cmd: 0x%x\n", channel, i, dest->f.command & 0xF0);
                // if this is the highest channel we've assigned, set num channels used field
                if(i >= *num_channels_used) *num_channels_used = i+1;
            }
            dest->f.command &= 0xF0;
            dest->f.command |= channel_map[channel] & 0xF;
            // if we have a note off, with the marker indicating we're done with channel,
            // then clear the mapping
            if(((dest->f.command & 0xF0) == ISE_MIDI_MSG_NOTE_OFF) && (dest->f.data1 == 0xFF)) {
                //printf("freeing ch %d from %d\n", channel, channel_map[channel]);
                channels_used &= ~(1 << channel_map[channel]);
                channel_map[channel] = 0xFF;
            }
        }
        dest++;
    }
    // end the track
    dest->data = 0x0;
    dest->f.command = ISE_MIDI_META_EVENT_TYPE_END_TRACK;
}

ise_midi_event_t* ise_midi_load_track(const char* filename)
{
    FILE* file = fopen( filename, "rb" );
    if( file == NULL ) return NULL;

    ise_midi_event_t* final_track = NULL;
    ise_midi_header_t midi_header;
    ise_midi_fread_bigend(file, &midi_header.chunk_type, sizeof(uint32_t));
    ise_midi_fread_bigend(file, &midi_header.length, sizeof(uint32_t));
    if(midi_header.chunk_type == ISE_MIDI_HEADER_CHUNK_ID && midi_header.length == 6) {
        ise_midi_fread_bigend(file, &midi_header.format, sizeof(uint16_t));
        ise_midi_fread_bigend(file, &midi_header.num_tracks, sizeof(uint16_t));
        ise_midi_fread_bigend(file, &midi_header.division, sizeof(uint16_t));
        int i, first_track_pos;
        int max_track_events = 0, total_events = 0;
        int num_channels = 0;
        first_track_pos = ftell(file);
        //midi_header.num_tracks = 5;
        for(i=0; i<midi_header.num_tracks; i++) {
            int num_events = ise_midi_parse_track(file, &midi_header, NULL, &num_channels);
            //printf("number of events on track  %d = %d\n", i, num_events);
            total_events += num_events;
            if(num_events > max_track_events) max_track_events = num_events;
        }
        fseek(file, first_track_pos, SEEK_SET);
        num_channels = 0;
        ise_midi_event_t* new_track = (ise_midi_event_t*) malloc(max_track_events * sizeof(ise_midi_event_t));
        ise_midi_event_t* mix_track0 = (ise_midi_event_t*) malloc(total_events * sizeof(ise_midi_event_t));
        ise_midi_event_t* mix_track1 = (ise_midi_event_t*) malloc(total_events * sizeof(ise_midi_event_t));
        mix_track0->data = 0x0;
        mix_track0->f.command = ISE_MIDI_META_EVENT_TYPE_END_TRACK;
        for(i=0; i<midi_header.num_tracks; i++) {
            int num_events = ise_midi_parse_track(file, &midi_header, new_track, &num_channels);
            //ise_mem_dump(NULL, new_track, num_events);
            ise_midi_mix_tracks(mix_track1, mix_track0, new_track, &num_channels);
            final_track = mix_track1;
            mix_track1 = mix_track0;
            mix_track0 = final_track;
        }
        free(new_track);
        free(mix_track1);
        //ise_mem_dump(NULL, final_track, total_events);
        //printf("num channels = %d\n", num_channels);
    } else {
        printf("Bad header - chunk_type 0x%x length %d\n", midi_header.chunk_type, midi_header.length);
    }
    fclose(file);
    return final_track;
}

void ise_midi_free_track(ise_midi_event_t* track)
{
    free(track);
}
