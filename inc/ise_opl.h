// InfiniScroll Engine
// ise_opl.h
// Yamaha OPL2/3 FM synthesizer
//
// Kamal Pillai
// 4/15/2019

#ifndef __ISE_OPL_H
#define __ISE_OPL_H

#include "ise_midi.h"

#define ISE_OPL_PCI_CLASS  ((ISE_PCI_CLASS_MULTIMEDIA << 16) | (ISE_PCI_SUBCLASS_MM_AUDIO << 8))

#define ISE_OPL_DEVICE_NOT_FOUND               0x0
#define ISE_OPL_DEVICE_OPL1                    0x1  // not supported
#define ISE_OPL_DEVICE_OPL2                    0x2
#define ISE_OPL_DEVICE_OPL3                    0x3

#define ISE_OPL_REG_TEST                       0x01
#define ISE_OPL_REG_TIMER1                     0x02
#define ISE_OPL_REG_TIMER2                     0x03
#define ISE_OPL_REG_TIMER_CONTROL              0x04   // on first set of regs
#define ISE_OPL_REG_FOUR_OP_ENABLE             0x104  // on second set of regs (opl3)
#define ISE_OPL_REG_OPL3_MODE_ENABLE           0x105  // on second set og regs (opl3)
#define ISE_OPL_REG_NOTE_SEL                   0x08
#define ISE_OPL_REG_FM_MULT                    0x20
#define ISE_OPL_REG_KEY_SCALE                  0x40
#define ISE_OPL_REG_ATTACK_DECAY               0x60
#define ISE_OPL_REG_SUSTAIN_RELEASE            0x80
#define ISE_OPL_REG_FREQ                       0xA0
#define ISE_OPL_REG_OCTAVE                     0xB0
#define ISE_OPL_REG_RHYTHM                     0xBD
#define ISE_OPL_REG_FEEDBACK                   0xC0
#define ISE_OPL_REG_WAVE_SELECT                0xE0

#define ISE_OPL_FNUM_C                         0x159
#define ISE_OPL_FNUM_C_SHARP                   0x16D
#define ISE_OPL_FNUM_D                         0x183
#define ISE_OPL_FNUM_D_SHARP                   0x19A
#define ISE_OPL_FNUM_E                         0x1B2
#define ISE_OPL_FNUM_F                         0x1CC
#define ISE_OPL_FNUM_F_SHARP                   0x1E8
#define ISE_OPL_FNUM_G                         0x205
#define ISE_OPL_FNUM_G_SHARP                   0x224
#define ISE_OPL_FNUM_A                         0x244
#define ISE_OPL_FNUM_A_SHARP                   0x267
#define ISE_OPL_FNUM_B                         0x28B

#define ISE_ADLIB_BANK_FILE_SIG0               0x44410001  // {1, 0, 'A', 'D'}
#define ISE_ADLIB_BANK_FILE_SIG1               0x2D42494C  // {'L', 'I', 'B', '-'}

#pragma pack(push, 1)

// Adlib .bnk file format structures
typedef struct {
    uint32_t sig0;  // includes major/minor version in first 2 bytes
    uint32_t sig1;
    uint16_t num_used;
    uint16_t num_instruments;
    uint32_t offset_name;
    uint32_t offset_data;
    uint32_t pad0;
    uint32_t pad1;
} ise_adlib_bank_file_header_t;

typedef struct {
    uint16_t index;
    uint8_t flags;
    char name[9];
} ise_adlib_bank_file_name_t;

typedef struct {
    uint8_t ksl;
    uint8_t multiple;
    uint8_t feedback;
    uint8_t attack;
    uint8_t sustain;
    uint8_t eg;
    uint8_t decay;
    uint8_t release_rate;
    uint8_t total_level;
    uint8_t am;
    uint8_t vib;
    uint8_t ksr;
    uint8_t con;
} ise_adlib_bank_file_opl_regs_t;

typedef struct {
    uint8_t percussive;
    uint8_t voice_num;
    ise_adlib_bank_file_opl_regs_t opl_modulator;
    ise_adlib_bank_file_opl_regs_t opl_carrier;
    uint8_t mod_wave_sel;
    uint8_t car_wave_sel;
} ise_adlib_bank_file_instrument_t;

// Junglevision op3 file format
// First 32 Bytes must match the signature
#define ISE_OP3_FILE_SIGNATURE "Junglevision Patch File\x1A\0\0\0\0\0\0\0\0"

typedef struct {
    uint32_t sig[8];           // 32 Byte signature
    uint16_t num_melodic;      // number of melodic instruments
    uint16_t num_percussive;   // number of percussive instruments
    uint16_t start_melodic;    // starting offset of melodic instruments
    uint16_t start_percussive; // starting offset of percussive instruments
} ise_op3_file_header_t;

typedef struct {
    uint8_t fm_mult;
    uint8_t key_scale;
    uint8_t attack_decay;
    uint8_t sustain_release;
    uint8_t wave_select;
} ise_opl_operator_regs_t;

typedef struct {
    uint8_t flags;
    uint8_t perc_note_num;
    ise_opl_operator_regs_t op1;
    uint8_t connection12;  // connection between op1 and op2
    ise_opl_operator_regs_t op2;
    ise_opl_operator_regs_t op3;
    uint8_t connection34;
    ise_opl_operator_regs_t op4;
} ise_op3_instrument_t;

// Op2 bank format
// First 8 bytes must match the signature
#define ISE_OP2_FILE_SIGNATURE                 "#OPL_II#"
#define ISE_OP2_FILE_NUM_INSTRUMENTS           175

typedef struct {
    uint8_t fm_mult;
    uint8_t attack_decay;
    uint8_t sustain_release;
    uint8_t wave_select;
    uint8_t key_scale;
    uint8_t output_level;
} ise_op2_operator_regs_t;

typedef struct {
    uint16_t flags;
    uint8_t detune;
    uint8_t note;
    ise_op2_operator_regs_t op1;
    uint8_t connection12;
    ise_op2_operator_regs_t op2;
    uint8_t reserved0;
    int16_t note_offset0;
    ise_op2_operator_regs_t op3;
    uint8_t connection34;
    ise_op2_operator_regs_t op4;
    uint8_t reserved1;
    int16_t note_offset1;
} ise_op2_instrument_t;
#pragma pack(pop)

typedef struct {
    ise_op3_instrument_t opl3;
    int16_t note_offset[2];
    uint8_t detune;
} ise_opl_instrument_t;

typedef struct {
    char name[9];
    uint8_t midi_low;
    uint8_t midi_high;
} ise_adlib_instrument_name_mapping_t;

/*typedef struct {
    ise_opl_operator_regs_t mod_regs;
    ise_opl_operator_regs_t car_regs;
    uint8_t feedback;
} ise_opl_instrument_t;
*/

int ise_opl_install();
void ise_opl_uninstall();

void ise_opl_play_track(ise_midi_event_t* track);
void ise_opl_tick();

#endif  //  #ifndef __ISE_OPL_H