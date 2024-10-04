// InfiniScroll Engine
// ise_midi.h
// Midi related functions
//
// Kamal Pillai
// 4/17/2019

#ifndef __ISE_MIDI_H
#define __ISE_MIDI_H

#define ISE_MIDI_HEADER_CHUNK_ID    0x4D546864   // "MThd"
#define ISE_MIDI_TRACK_CHUNK_ID     0x4D54726B   // "MTrk"

#define ISE_MIDI_EVENT_TYPE_SYSEX   0xF0
#define ISE_MIDI_EVENT_TYPE_SYSEX2  0xF7
#define ISE_MIDI_EVENT_TYPE_META    0xFF

#define ISE_MIDI_META_EVENT_TYPE_CHAN_PREFIX   0x20
#define ISE_MIDI_META_EVENT_TYPE_END_TRACK     0x2F
#define ISE_MIDI_META_EVENT_TYPE_SET_TEMPO     0x51

#define ISE_MIDI_MSG_NOTE_OFF                  0x80
#define ISE_MIDI_MSG_NOTE_ON                   0x90
#define ISE_MIDI_MSG_KEY_PRESSURE              0xA0
#define ISE_MIDI_MSG_CONTROL_CHANGE            0xB0
#define ISE_MIDI_MSG_PROGRAM_CHANGE            0xC0
#define ISE_MIDI_MSG_CHANNEL_PRESSURE          0xD0
#define ISE_MIDI_MSG_PITCH_WHEEL               0xE0

#define ISE_MIDI_CTRL_MAIN_VOLUME              0x07

#pragma pack(push, 1)

typedef struct {
    uint32_t chunk_type;
    uint32_t length;
    uint16_t format;
    uint16_t num_tracks;
    uint16_t division;
} ise_midi_header_t;

#pragma pack(pop)

typedef struct {
    int8_t note; // -1 if off
    int8_t midi_channel; // -1 if none assigned
    int8_t voice;  // -1 if none assigned
    int8_t reserved;
} ise_midi_opl_channel_t;

typedef struct {
    uint8_t data0;
    uint8_t data1;
    uint8_t command;
    uint8_t delta_time;
} ise_midi_event_field_t;

typedef union {
    ise_midi_event_field_t f;
    uint32_t data;
} ise_midi_event_t;

void ise_midi_fread_bigend(FILE* file, void* data, uint8_t length);
int ise_midi_fread_var_bigend(FILE* file, uint32_t* data);
ise_midi_event_t* ise_midi_load_track(const char* filename);
uint32_t ise_midi_get_delta_time(ise_midi_event_t** e);


#endif  //  #ifndef __ISE_MIDI_H