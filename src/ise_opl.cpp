// InfiniScroll Engine
// ise_opl.cpp
// Yamaha OPL2/3 FM synthesizer
//
// Kamal Pillai
// 4/15/2019

#include "ise.h"

extern uint32_t ise_dsp_base;
uint32_t ise_opl_base = 0;
ise_midi_event_t* ise_opl_track = NULL;
ise_midi_event_t* ise_opl_cur_event = NULL;
uint32_t ise_opl_last_time = 0;
uint32_t ise_opl_systime_per_tick = 24;
uint8_t ise_opl_rhythm_reg = 0x20;
bool ise_opl_4op = false;
uint16_t ise_opl_4op_channel = 0;
// for each channel, indicates:
// 2op mode: 0 - FM; 1 - AM
// 4op mode: 0 - FM/FM; 1 - AM/FM; 2 - FM/AM; 3 - AM/AM
// bit 7 indicates pseudo-4op mode
uint8_t ise_opl_channel_mode[16] = {0};

extern uint8_t ise_dsp_irq;
extern uint8_t ise_dsp_dma8_channel;
extern uint8_t ise_dsp_dma16_channel;

// store key scale field for each op
uint8_t ise_opl_key_scale[36] = {0};

const ise_adlib_instrument_name_mapping_t ise_opl_ins_mapping[] = {
    {"ELPIANO", 3, 3},    // Electric Piano
    {"ELPIANO", 5, 6},    // Electric Piano
    {"EPIANO", 3, 3},    // Electric Piano
    {"EPIANO", 5, 6},    // Electric Piano
    {"HNKPIANO", 4, 4},   // Honky-tonk Piano
    {"PIANO", 1, 6},      // Piano
    {"HARPSCRD", 7, 7},   // Harpsichord
    {"HARPSI", 7, 7},     // Harpsichord
    {"CLAV", 8, 8},       // Clavinet
    {"CELESTA", 9, 9},    // Celesta
    {"GLOCK", 10, 10},      // Glockenspiel
    {"MUSICBOX", 11, 11}, // Music box
    {"VIB", 12, 12},      // Vibraphone
    {"MARIMBA", 13, 13},  // Marimba
    {"XYLO", 14, 14},     // Xylophone
    {"BELL", 15, 15},     // Tubular Bells
    {"DULC", 16, 16},     // Dulcimer
    {"ORGNPERC", 18, 18}, // Percussive organ
    {"ROCKORGN", 19, 19}, // Rock organ
    {"RCKORGAN", 19, 19}, // Rock Organ
    {"CHURORGN", 20, 20}, // Church organ
    {"REEDORGN", 21, 21}, // Reed organ
    {"ORGAN", 17, 21},    // Organ
    {"ACCORD", 22, 22},   // Accordian
    {"HARMON", 23, 23},   // Harmonica
    {"TANGOACC", 24, 24}, // Tango accordian
    {"JAZZGUIT", 27, 27}, // Jazz electric guitar
    {"JAZZGTR", 27, 27},  // Jazz electric guitar
    {"STEELGT", 26, 26},  // Steel acoustic guitar
    {"ELGUIT", 28, 28},   // Clean electric guitar
    {"EGUIT", 28, 28},    // Clean electric guitar
    {"CLNGUIT", 28, 28},  // Clean electric guitar
    {"CLNGTR", 28, 28},   // Clean electric guitar
    {"MUTEGUIT", 29, 29}, // Muted electric guitar
    {"MUTEGTR", 29, 29},  // Muted electric guitar
    {"FUZGUIT", 30, 30},  // Overdriven guitar
    {"OVERGUIT", 30, 30}, // Overdriven guitar
    {"OVERGTR", 30, 30},  // Overdriven guitar
    {"DISTGUIT", 31, 31}, // Distortion guitar
    {"DISTGTR", 31, 31},  // Distortion guitar
    {"GUIT", 25, 32},     // Guitar
    {"ELBASS", 34, 35},   // Electric bass
    {"FLESBASS", 36, 36}, // Fretless bass
    {"SLAPBASS", 37, 38}, // Slap bass
    {"SLPBASS", 37, 38},  // Slap bass
    {"SYNBASS", 39, 40},  // Synth bass
    {"BASSOON", 71, 71},  // Bassoon
    {"BASS", 33, 40},     // Bass
    {"VIOLIN", 41, 41},   // Violin
    {"VIOLA", 42, 42},    // Viola
    {"CELLO", 43, 43},    // Cello
    {"CBASS", 44, 44},    // Contrabass
    {"TREMSTRN", 45, 45}, // Tremolo strings
    {"PIZZSTRN", 46, 46}, // Pizzicato strings
    {"HARP", 47, 47},     // Harp
    {"TIMPANI", 48, 48},  // Timpani
    {"SYNSTRN", 51, 52},  // Synth strings
    {"STRING", 49, 52},   // Strings
    {"AAH", 53, 53},      // Voice Aahs
    {"OOH", 54, 54},      // Voice Oohs
    {"CHOIR", 53, 55},    // Choir
    {"VOICE", 53, 55},    // Voice
    {"ORCHHIT", 56, 56},  // Orchestra hit
    {"TRUMP", 57, 57},    // Trumpet
    {"TROMB", 58, 58},    // Trombone
    {"TUBA", 59, 59},     // Tuba
    {"MUTETRMP", 60, 60}, // Muted trumpet
    {"FRHORN", 61, 61},   // French horn
    {"SYNBRASS", 63, 64}, // Synth brass
    {"SYNBRS", 63, 64},   // Synth brass
    {"BRASS", 62, 64},    // Brass
    {"SOPSAX", 65, 65},   // Soprano sax
    {"ALTOSAX", 66, 66},  // Alto sax
    {"TENSAX", 67, 67},   // Tenor sax
    {"BARISAX", 68, 68},  // Baritone sax
    {"SAX", 65, 68},      // Sax
    {"OBOE", 69, 69},     // Oboe
    {"ENGHORN", 70, 70},  // English horn
    {"CLAR", 72, 72},     // Clarinet
    {"PICC", 73, 73},     // Piccolo
    {"FLUTE", 74, 74},    // Flute
    {"RECORDER", 75, 75}, // Recorder
    {"PANFL", 76, 76},    // Pan flute
    {"BOTTLE", 77, 77},   // Blown bottle
    {"SHAK", 78, 78},     // Shakuhachi
    {"WHISTLE", 79, 79},  // Whistle
    {"OCAR", 80, 80},     // Ocarina
    {"SITAR", 105, 105},  // Sitar
    {"BANJO", 106, 106},  // Banjo
    {"SHAMI", 107, 107},  // Shamisen
    {"KOTO", 108, 108},   // Koto
    {"BAGPIPE", 110, 110},// Bag pipe
    {"FIDDLE", 111, 111}, // Fiddle
    {"SHANAI", 112, 112}, // Shanai
    {"TNKBELL", 113, 113},// Tinkle bell
    {"AGOGO", 114, 114},  // Agogo
    {"STLDRUM", 115, 115},// Steel drums
    {"WOODBLK", 116, 116},// Wood block
    {"TAIKO", 117, 117},  // Taiko drum
    {"MELTOM", 118, 118}, // Melodic Tom
    {"SYNDRUM", 119, 119},// Synth drum
    {"", 127, 127}        // End
};
     
// FM synthesized instrument registers for General Midi instruments
ise_opl_instrument_t ise_opl_instrument[128];
// current instrument for the given channel
ise_opl_instrument_t* ise_opl_current_instrument[16] = {NULL};

const uint16_t ise_opl_freq_table[12] = {
    ISE_OPL_FNUM_C, ISE_OPL_FNUM_C_SHARP,
    ISE_OPL_FNUM_D, ISE_OPL_FNUM_D_SHARP,
    ISE_OPL_FNUM_E,
    ISE_OPL_FNUM_F, ISE_OPL_FNUM_F_SHARP,
    ISE_OPL_FNUM_G, ISE_OPL_FNUM_G_SHARP,
    ISE_OPL_FNUM_A, ISE_OPL_FNUM_A_SHARP,
    ISE_OPL_FNUM_B };

void (*ise_opl_write_reg)(int addr, int data, int channel, int op) = NULL;

void ise_opl_load_default_instrument()
{
    int i;
    ise_opl_4op = false;
    for(i=0; i<128; i++) {
        ise_opl_instrument[i].opl3.flags = 0;
        ise_opl_instrument[i].opl3.connection12 = 0x36;             // enable both speakers, some feedback
        ise_opl_instrument[i].opl3.op1.fm_mult = 0x01;              // Set multiple to 1
        ise_opl_instrument[i].opl3.op1.key_scale = 0x4F;            // Set level to 40 dB
        ise_opl_instrument[i].opl3.op1.attack_decay = 0xF1;         // Quick attack, long delay
        ise_opl_instrument[i].opl3.op1.sustain_release = 0x53;      // medium sustain and release
        ise_opl_instrument[i].opl3.op1.wave_select = 0x0;           // sine wave
        ise_opl_instrument[i].opl3.op2.fm_mult = 0x01;              // Set multiple to 1
        ise_opl_instrument[i].opl3.op2.key_scale = 0x10;            // Set level to 47 dB
        ise_opl_instrument[i].opl3.op2.attack_decay = 0xD2;         // Quick attack, long delay
        ise_opl_instrument[i].opl3.op2.sustain_release = 0x74;      // medium sustain and release
        ise_opl_instrument[i].opl3.op2.wave_select = 0x0;           // sine wave
        ise_opl_instrument[i].note_offset[0] = 0;                   // no offset
        ise_opl_instrument[i].note_offset[1] = 0;                   // no offset
        ise_opl_instrument[i].detune = 0x80;                        // no detuning
    }
}

void ise_op3_load_file(const char* filename)
{
    FILE* file = fopen( filename, "rb" );
    if( file == NULL ) return;

    // Enable wave form select
    ise_opl_write_reg(ISE_OPL_REG_TEST, 0x20, 0, 0);

    // Set all instruments to the default instrument type
    ise_opl_load_default_instrument();

    int i;
    const uint32_t* op3_signature = (const uint32_t*) ISE_OP3_FILE_SIGNATURE;
    ise_op3_file_header_t header;
    fread(&header, sizeof(ise_op3_file_header_t), 1, file);
    if(header.sig[0] == *(op3_signature+0) && header.sig[1] == *(op3_signature+1) && 
       header.sig[2] == *(op3_signature+2) && header.sig[3] == *(op3_signature+3) && 
       header.sig[4] == *(op3_signature+4) && header.sig[5] == *(op3_signature+5) && 
       header.sig[6] == *(op3_signature+6) && header.sig[7] == *(op3_signature+7) ) {
        for(i=header.start_melodic; i<header.num_melodic; i++) {
            fread(&(ise_opl_instrument[i].opl3), sizeof(ise_op3_instrument_t), 1, file);
            ise_opl_instrument[i].note_offset[0] = 0;                   // no offset
            ise_opl_instrument[i].note_offset[1] = 0;                   // no offset
            ise_opl_instrument[i].detune = 0x80;                        // no detuning
            
            //ise_opl_instrument[i].opl3.connection12 &= 0x01;
            ise_opl_instrument[i].opl3.connection12 |= 0x30;
            //ise_opl_instrument[i].opl3.connection34 &= 0x01;
            ise_opl_instrument[i].opl3.connection34 |= 0x30;
            if(ise_opl_instrument[i].opl3.flags & 0x81) {
                ise_opl_4op = true;
            }
        }
    }
    fclose(file);
}

void ise_op2_load_file(const char* filename)
{
    FILE* file = fopen( filename, "rb" );
    if( file == NULL ) return;

    // Enable wave form select
    ise_opl_write_reg(ISE_OPL_REG_TEST, 0x20, 0, 0);

    // Set all instruments to the default instrument type
    ise_opl_load_default_instrument();

    int i;
    const uint32_t* op2_signature = (const uint32_t*) ISE_OP2_FILE_SIGNATURE;
    uint32_t file_signature[2];
    fread(file_signature, 8, 1, file);
    if(file_signature[0] == op2_signature[0] && file_signature[1] == op2_signature[1]) {
        ise_op2_instrument_t ins;
        for(i=0; i<128; i++) {
            fread(&ins, sizeof(ise_op2_instrument_t), 1, file);
            ise_opl_instrument[i].opl3.flags = (ins.flags & 0x4) << 5;
            ise_opl_instrument[i].detune = ins.detune;
            ise_opl_instrument[i].note_offset[0] = ins.note_offset0;
            ise_opl_instrument[i].note_offset[1] = ins.note_offset1;
            ise_opl_instrument[i].opl3.perc_note_num = ins.note;
            ise_opl_instrument[i].opl3.connection12 = ins.connection12;
            ise_opl_instrument[i].opl3.connection34 = ins.connection34;
            ise_opl_instrument[i].opl3.op1.fm_mult = ins.op1.fm_mult;
            ise_opl_instrument[i].opl3.op1.key_scale = ins.op1.key_scale;
            ise_opl_instrument[i].opl3.op1.key_scale|= ins.op1.output_level;
            ise_opl_instrument[i].opl3.op1.attack_decay = ins.op1.attack_decay;
            ise_opl_instrument[i].opl3.op1.sustain_release = ins.op1.sustain_release;
            ise_opl_instrument[i].opl3.op1.wave_select = ins.op1.wave_select;
            ise_opl_instrument[i].opl3.op2.fm_mult = ins.op2.fm_mult;
            ise_opl_instrument[i].opl3.op2.key_scale = ins.op2.key_scale;
            ise_opl_instrument[i].opl3.op2.key_scale|= ins.op2.output_level;
            ise_opl_instrument[i].opl3.op2.attack_decay = ins.op2.attack_decay;
            ise_opl_instrument[i].opl3.op2.sustain_release = ins.op2.sustain_release;
            ise_opl_instrument[i].opl3.op2.wave_select = ins.op2.wave_select;
            ise_opl_instrument[i].opl3.op3.fm_mult = ins.op3.fm_mult;
            ise_opl_instrument[i].opl3.op3.key_scale = ins.op3.key_scale;
            ise_opl_instrument[i].opl3.op3.key_scale|= ins.op3.output_level;
            ise_opl_instrument[i].opl3.op3.attack_decay = ins.op3.attack_decay;
            ise_opl_instrument[i].opl3.op3.sustain_release = ins.op3.sustain_release;
            ise_opl_instrument[i].opl3.op3.wave_select = ins.op3.wave_select;
            ise_opl_instrument[i].opl3.op4.fm_mult = ins.op4.fm_mult;
            ise_opl_instrument[i].opl3.op4.key_scale = ins.op4.key_scale;
            ise_opl_instrument[i].opl3.op4.key_scale|= ins.op4.output_level;
            ise_opl_instrument[i].opl3.op4.attack_decay = ins.op4.attack_decay;
            ise_opl_instrument[i].opl3.op4.sustain_release = ins.op4.sustain_release;
            ise_opl_instrument[i].opl3.op4.wave_select = ins.op4.wave_select;

            //ise_opl_instrument[i].opl3.connection12 &= 0x01;
            ise_opl_instrument[i].opl3.connection12 |= 0x30;
            //ise_opl_instrument[i].opl3.connection34 &= 0x01;
            ise_opl_instrument[i].opl3.connection34 |= 0x30;
            if(ise_opl_instrument[i].opl3.flags & 0x81) {
                ise_opl_4op = true;
            }
        }
    }

    fclose(file);
}

void ise_opl_load_bank_file(const char* filename)
{
    FILE* file = fopen( filename, "rb" );
    if( file == NULL ) return;

    // Enable wave form select
    ise_opl_write_reg(ISE_OPL_REG_TEST, 0x20, 0, 0);

    // Set all instruments to the default instrument type
    ise_opl_load_default_instrument();

    ise_adlib_bank_file_header_t header;
    ise_adlib_bank_file_name_t instrument_name;
    ise_adlib_bank_file_instrument_t instrument_regs;
    // mapping of instruments in file to general midi instruments
    uint8_t instrument_mapping[256];
    bool ins_used[128] = {false};

    int i, j;
    for(i=0; i<256; i++) {
        instrument_mapping[i] = 128; // default mapping
    }

    fread(&header, sizeof(ise_adlib_bank_file_header_t), 1, file);
    // Check header signatures
    if(header.sig0 == ISE_ADLIB_BANK_FILE_SIG0 && header.sig1 == ISE_ADLIB_BANK_FILE_SIG1) {

        // We should already be at the name offset, but in case not, go there now
        fseek(file, header.offset_name, SEEK_SET);

        // cap number of instruments to 256
        if(header.num_instruments > 256) header.num_instruments = 256;
        
        // Read each instrument name from the file
        for(i=0; i<header.num_instruments; i++) {
            fread(&instrument_name, sizeof(ise_adlib_bank_file_name_t), 1, file);
            // Check if record is in use
            if(instrument_name.flags) {
                const ise_adlib_instrument_name_mapping_t* ins_map = ise_opl_ins_mapping;
                // Loop all the instruments that we recognize
                while(ins_map->name[0]) { // while name is not an empty string
                    // find the instrument name
                    if(strstr(instrument_name.name, ins_map->name)) {
                        // Loop through all instruments that we can map to
                        for(j=ins_map->midi_low-1; j<=ins_map->midi_high-1; j++) {
                            // Check if instrument is not used
                            if(!ins_used[j]) {
                                // if not, use it, and map it
                                ins_used[j] = true;
                                instrument_mapping[i] = j;
                                //printf("Mapping %s->%s (%d)\n", instrument_name.name, ins_map->name, j);
                                break;
                            }
                        }
                        if(instrument_mapping[i] == 128) {
                            //printf("%s no space\n", instrument_name.name);
                            break;
                        }
                    }
                    ins_map++;
                    if(instrument_mapping[i] != 128) {
                        break;
                    }
                }
                //if(instrument_mapping[i] == 128) {
                //    printf("%s unmapped\n", instrument_name.name);
                //}
            }
        }
        
        // Move to the beginning of the instrument register section
        fseek(file, header.offset_data, SEEK_SET);
        
        // Read each instrument setting
        for(i=0; i<header.num_instruments; i++) {
            fread(&instrument_regs, sizeof(ise_adlib_bank_file_instrument_t), 1, file);
            // Check if the instrument is mapped, and it's non-percussive
            uint8_t ins_map_index = instrument_mapping[i];
            if(instrument_regs.percussive == 0 && ins_map_index < 128) {
                ise_opl_instrument[i].opl3.flags = 0;
                ise_opl_instrument[i].opl3.connection12 = 0x36;

                ise_opl_instrument[ins_map_index].opl3.op1.fm_mult = instrument_regs.opl_modulator.multiple & 0xF;
                ise_opl_instrument[ins_map_index].opl3.op1.fm_mult |= (instrument_regs.opl_modulator.ksr & 0x1) << 4;
                ise_opl_instrument[ins_map_index].opl3.op1.fm_mult |= (instrument_regs.opl_modulator.eg & 0x1) << 5;
                ise_opl_instrument[ins_map_index].opl3.op1.fm_mult |= (instrument_regs.opl_modulator.vib & 0x1) << 6;
                ise_opl_instrument[ins_map_index].opl3.op1.fm_mult |= (instrument_regs.opl_modulator.am & 0x1) << 7;
                ise_opl_instrument[ins_map_index].opl3.op1.key_scale = (instrument_regs.opl_modulator.ksl & 0x3) << 6;
                ise_opl_instrument[ins_map_index].opl3.op1.key_scale |= (instrument_regs.opl_modulator.total_level & 0x3F);
                ise_opl_instrument[ins_map_index].opl3.op1.attack_decay = (instrument_regs.opl_modulator.attack & 0xF) << 4;
                ise_opl_instrument[ins_map_index].opl3.op1.attack_decay |= (instrument_regs.opl_modulator.decay & 0xF);
                ise_opl_instrument[ins_map_index].opl3.op1.sustain_release = (instrument_regs.opl_modulator.sustain & 0xF) << 4;
                ise_opl_instrument[ins_map_index].opl3.op1.sustain_release |= (instrument_regs.opl_modulator.release_rate & 0xF);
                ise_opl_instrument[ins_map_index].opl3.op1.wave_select = instrument_regs.mod_wave_sel;

                ise_opl_instrument[ins_map_index].opl3.op2.fm_mult = instrument_regs.opl_carrier.multiple & 0xF;
                ise_opl_instrument[ins_map_index].opl3.op2.fm_mult |= (instrument_regs.opl_carrier.ksr & 0x1) << 4;
                ise_opl_instrument[ins_map_index].opl3.op2.fm_mult |= (instrument_regs.opl_carrier.eg & 0x1) << 5;
                ise_opl_instrument[ins_map_index].opl3.op2.fm_mult |= (instrument_regs.opl_carrier.vib & 0x1) << 6;
                ise_opl_instrument[ins_map_index].opl3.op2.fm_mult |= (instrument_regs.opl_carrier.am & 0x1) << 7;
                ise_opl_instrument[ins_map_index].opl3.op2.key_scale = (instrument_regs.opl_carrier.ksl & 0x3) << 6;
                ise_opl_instrument[ins_map_index].opl3.op2.key_scale |= (instrument_regs.opl_carrier.total_level & 0x3F);
                ise_opl_instrument[ins_map_index].opl3.op2.attack_decay = (instrument_regs.opl_carrier.attack & 0xF) << 4;
                ise_opl_instrument[ins_map_index].opl3.op2.attack_decay |= (instrument_regs.opl_carrier.decay & 0xF);
                ise_opl_instrument[ins_map_index].opl3.op2.sustain_release = (instrument_regs.opl_carrier.sustain & 0xF) << 4;
                ise_opl_instrument[ins_map_index].opl3.op2.sustain_release |= (instrument_regs.opl_carrier.release_rate & 0xF);
                ise_opl_instrument[ins_map_index].opl3.op2.wave_select = instrument_regs.car_wave_sel;

                ise_opl_instrument[ins_map_index].note_offset[0] = 0;                   // no offset
                ise_opl_instrument[ins_map_index].note_offset[1] = 0;                   // no offset
                ise_opl_instrument[ins_map_index].detune = 0x80;                        // no detuning
                //ise_opl_instrument[ins_map_index].opl3.feedback = 0x30 |(instrument_regs.opl_modulator.feedback & 0x7) << 1;
            }
        }
    }
    fclose(file);
}

int ise_opl_get_reg_addr(int addr, int channel, int op)
{
    bool probe_addr = addr == ISE_OPL_REG_FEEDBACK || addr == ISE_OPL_REG_FM_MULT || addr == ISE_OPL_REG_ATTACK_DECAY || addr == ISE_OPL_REG_SUSTAIN_RELEASE || addr == ISE_OPL_REG_WAVE_SELECT;
    //if(probe_addr) {
    //    printf("ch: %d op: %d -> ", channel, op);
    //}
    int offset = 0;
    if(channel != 15) {
        // non-percussive; percussive generates op directly
        if(channel >= 6) channel += 3;
        if(ise_opl_4op && channel >= 3) channel += 6;
        op = op*3 + (channel % 3) + 6*(channel / 3);
    }
    if(op >= 18) {
        offset += 0x100;
        op -= 18;
    }
    bool channel_addr = (addr == ISE_OPL_REG_FREQ || addr == ISE_OPL_REG_OCTAVE || addr == ISE_OPL_REG_FEEDBACK);
    if(channel_addr) {
        offset += 3*(op/6) + (op % 3);
    } else {
        offset += 8*(op/6) + (op % 6);
    }
    //if(probe_addr) {
    //    printf("offset 0x%x -- data = 0x%x\n", offset, data);
    //}
    return addr + offset;

    /*int offset = 0;
    bool channel_addr = (addr == ISE_OPL_REG_FREQ || addr == ISE_OPL_REG_OCTAVE || addr == ISE_OPL_REG_FEEDBACK);
    if(ise_opl_4op && channel < 6) {
        if(channel >= 3) channel += 6;
        if(op & 2) channel += 3;
        op &= 1;
    }
    if(channel > 8) {
        // for channels 9 and above, use second port
        // this is denoted by adding an offset 0x100
        offset += 0x100;
        channel -= 9;
    }
    if(channel_addr) {
        offset += channel;
    } else {
        offset += 8 * (channel / 3) + 3 * op + (channel % 3);
    }
    return addr + offset;
    */
}

void ise_opl2_write_reg(int addr, int data, int channel, int op)
{
    addr = ise_opl_get_reg_addr(addr, channel, op);
    if(addr < 0x100) {
        outp(ise_opl_base, addr);
        ise_time_uwait(3);
        outp(ise_opl_base+1, data);
        ise_time_uwait(23);
    }
}

void ise_opl3_write_reg(int addr, int data, int channel, int op)
{
    addr = ise_opl_get_reg_addr(addr, channel, op);
    int base = ise_opl_base + ((addr >= 0x100) ? 2 : 0);
    addr &= 0xFF;
    outp(base, addr);
    outp(base+1, data);
}

int ise_opl_install()
{
#ifdef DOS32
    int s, f;
    for(s=0; s<ISE_PCI_MAX_SLOTS; s++) {
        for(f=0; f<ISE_PCI_MAX_FUNC; f++) {

            // Look for vendor/device id
            switch(ise_pci.slot[s][f].vendor_id) {
            case ISE_PCI_VENDOR_CMEDIA:
                switch(ise_pci.slot[s][f].device_id) {
                case ISE_PCI_DEVICE_CMEDIA_CMI8738_0:
                case ISE_PCI_DEVICE_CMEDIA_CMI8738:
                    ise_dsp_init_cmi8738(s, f);
                    break;
                }
                break;
            case ISE_PCI_VENDOR_ULI:
                switch(ise_pci.slot[s][f].device_id) {
                case ISE_PCI_DEVICE_ULI_CMI8738:
                    ise_dsp_init_cmi8738(s, f);
                    break;
                }
                break;
            }
        }
    }
#endif 
    if(ise_opl_base == 0) {
        ise_dsp_init_sb();
        // if we don't have a base address, try parsing the blaster env variable
        const char* blaster = getenv("BLASTER");
        if(blaster) {
            const char* c = blaster;
            while(*c != '\0') {
                if(*c == 'A') {
                    c++;
                    while(*c == ' ') c++;
                    while((*c >= '0' && *c <= '9') || (*c >= 'A' && *c <= 'F') || (*c >= 'a' && *c <= 'f')) {
                        if(*c >= '0' && *c <= '9') {
                            ise_opl_base = (ise_opl_base << 4) + (*c - '0');
                        } else if(*c >= 'A' && *c <= 'F') {
                            ise_opl_base = (ise_opl_base << 4) + (*c - 'A' + 10);
                        } else if(*c >= 'a' && *c <= 'f') {
                            ise_opl_base = (ise_opl_base << 4) + (*c - 'a' + 10);
                        }
                        c++;
                    }
                } else if(*c == 'I') {
                    ise_dsp_irq = 0;
                    c++;
                    while(*c == ' ') c++;
                    while((*c >= '0' && *c <= '9') || (*c >= 'A' && *c <= 'F') || (*c >= 'a' && *c <= 'f')) {
                        if(*c >= '0' && *c <= '9') {
                            ise_dsp_irq = (ise_dsp_irq << 4) + (*c - '0');
                        } else if(*c >= 'A' && *c <= 'F') {
                            ise_dsp_irq = (ise_dsp_irq << 4) + (*c - 'A' + 10);
                        } else if(*c >= 'a' && *c <= 'f') {
                            ise_dsp_irq = (ise_dsp_irq << 4) + (*c - 'a' + 10);
                        }
                        c++;
                    }
                } else if(*c == 'D') {
                    ise_dsp_dma8_channel = 0;
                    c++;
                    while(*c == ' ') c++;
                    while((*c >= '0' && *c <= '9') || (*c >= 'A' && *c <= 'F') || (*c >= 'a' && *c <= 'f')) {
                        if(*c >= '0' && *c <= '9') {
                            ise_dsp_dma8_channel = (ise_dsp_dma8_channel << 4) + (*c - '0');
                        } else if(*c >= 'A' && *c <= 'F') {
                            ise_dsp_dma8_channel = (ise_dsp_dma8_channel << 4) + (*c - 'A' + 10);
                        } else if(*c >= 'a' && *c <= 'f') {
                            ise_dsp_dma8_channel = (ise_dsp_dma8_channel << 4) + (*c - 'a' + 10);
                        }
                        c++;
                    }
                } else if(*c == 'H') {
                    ise_dsp_dma16_channel = 0;
                    c++;
                    while(*c == ' ') c++;
                    while((*c >= '0' && *c <= '9') || (*c >= 'A' && *c <= 'F') || (*c >= 'a' && *c <= 'f')) {
                        if(*c >= '0' && *c <= '9') {
                            ise_dsp_dma16_channel = (ise_dsp_dma16_channel << 4) + (*c - '0');
                        } else if(*c >= 'A' && *c <= 'F') {
                            ise_dsp_dma16_channel = (ise_dsp_dma16_channel << 4) + (*c - 'A' + 10);
                        } else if(*c >= 'a' && *c <= 'f') {
                            ise_dsp_dma16_channel = (ise_dsp_dma16_channel << 4) + (*c - 'a' + 10);
                        }
                        c++;
                    }
                } else {
                    c++;
                }
            }
            ise_dsp_base = ise_opl_base;
        }
    }
    // if we still haven't found a base address, try 0x388
    if(ise_opl_base == 0) {
        ise_opl_base = 0x388;

        // Check for OPL2
        // reset the two timers
        ise_opl2_write_reg(ISE_OPL_REG_TIMER_CONTROL, 0x60, 0, 0);
        // reset IRQ
        ise_opl2_write_reg(ISE_OPL_REG_TIMER_CONTROL, 0x80, 0, 0);
        int status1 = inp(ise_opl_base);
        ise_opl2_write_reg(ISE_OPL_REG_TIMER1, 0xFF, 0, 0);
        // start timer 1
        ise_opl2_write_reg(ISE_OPL_REG_TIMER_CONTROL, 0x21, 0, 0);
        ise_time_uwait(80);
        int status2 = inp(ise_opl_base);
        if(status1 != 0x0 || status2 != 0xC0) {
            ise_opl_base = 0x0;
            return ISE_OPL_DEVICE_NOT_FOUND;
        }
        // Check for OPL3
        status1 = inp(ise_opl_base);
        if((status1 & 0x6) != 0x0) {
            ise_opl_write_reg = ise_opl2_write_reg;
            printf("opl2 base 0x%x\n", ise_opl_base);
            return ISE_OPL_DEVICE_OPL2;
        }
    }

    // reset the two timers
    ise_opl2_write_reg(ISE_OPL_REG_TIMER_CONTROL, 0x60, 0, 0);
    // reset IRQ
    ise_opl2_write_reg(ISE_OPL_REG_TIMER_CONTROL, 0x80, 0, 0);

    //ise_opl2_write_reg(ISE_OPL_REG_TIMER1, 0xC0, 0, 0);
    // start timer 1
    //ise_opl2_write_reg(ISE_OPL_REG_TIMER_CONTROL, 0x1, 0, 0);

    ise_opl_write_reg = ise_opl3_write_reg;
    

    ise_opl_write_reg(ISE_OPL_REG_OPL3_MODE_ENABLE, 1, 0, 0);
    ise_opl_write_reg(ISE_OPL_REG_FOUR_OP_ENABLE, 0, 0, 0);

    // clear all registers
    /*ise_opl_write_reg(ISE_OPL_REG_TEST, 0x0, 0, 0);
    ise_opl_write_reg(ISE_OPL_REG_TIMER1, 0x0, 0, 0);
    ise_opl_write_reg(ISE_OPL_REG_TIMER2, 0x0, 0, 0);
    ise_opl_write_reg(ISE_OPL_REG_TIMER_CONTROL, 0x0, 0, 0);
    ise_opl_write_reg(ISE_OPL_REG_FOUR_OP_ENABLE, 0x0, 0, 0);
    ise_opl_write_reg(ISE_OPL_REG_OPL3_MODE_ENABLE, 0x0, 0, 0);
    ise_opl_write_reg(ISE_OPL_REG_NOTE_SEL, 0x0, 0, 0);
    int i, j;
    for(i=0; i<18; i++) {
        for(j=0; j<2; j++) {
            ise_opl_write_reg(ISE_OPL_REG_FM_MULT, 0x0, i, j);
            ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, 0x0, i, j);
            ise_opl_write_reg(ISE_OPL_REG_ATTACK_DECAY, 0x0, i, j);
            ise_opl_write_reg(ISE_OPL_REG_SUSTAIN_RELEASE, 0x0, i, j);
            ise_opl_write_reg(ISE_OPL_REG_WAVE_SELECT, 0x0, i, j);
        }
        ise_opl_write_reg(ISE_OPL_REG_FREQ, 0x0, i, 0);
        ise_opl_write_reg(ISE_OPL_REG_OCTAVE, 0x0, i, 0);
        ise_opl_write_reg(ISE_OPL_REG_FEEDBACK, 0x0, i, 0);
    }
    ise_opl_write_reg(ISE_OPL_REG_RHYTHM, 0x0, 0, 0);
    */

    // Test sound
    //ise_opl_write_reg(ISE_OPL_REG_FM_MULT, 0x1, 0, 0);          // Set multiple to 1
    //ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, 0x10, 0, 0);       // Set level to 40 dB
    //ise_opl_write_reg(ISE_OPL_REG_ATTACK_DECAY, 0xF0, 0, 0);    // Quick attack, long delay
    //ise_opl_write_reg(ISE_OPL_REG_SUSTAIN_RELEASE, 0x77, 0, 0); // medium sustain and release
    //ise_opl_write_reg(ISE_OPL_REG_FM_MULT, 0x1, 0, 1);          // Set multiple to 1
    //ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, 0x00, 0, 1);       // Set level to 47 dB
    //ise_opl_write_reg(ISE_OPL_REG_ATTACK_DECAY, 0xF0, 0, 1);    // Quick attack, long delay
    //ise_opl_write_reg(ISE_OPL_REG_SUSTAIN_RELEASE, 0x77, 0, 1); // medium sustain and release

    // Set default voices, and turn them off
    int i;
    for(i=0; i<6; i++) {
        ise_opl_write_reg(ISE_OPL_REG_FM_MULT, 0x01, i, 0);         // Set multiple to 1
        ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, 0x4F, i, 0);       // Set level to 40 dB
        ise_opl_write_reg(ISE_OPL_REG_ATTACK_DECAY, 0xF1, i, 0);    // Quick attack, long delay
        ise_opl_write_reg(ISE_OPL_REG_SUSTAIN_RELEASE, 0x53, i, 0); // medium sustain and release
        ise_opl_write_reg(ISE_OPL_REG_FM_MULT, 0x01, i, 1);         // Set multiple to 1
        ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, 0x10, i, 1);       // Set level to 47 dB
        ise_opl_write_reg(ISE_OPL_REG_ATTACK_DECAY, 0xD2, i, 1);    // Quick attack, long delay
        ise_opl_write_reg(ISE_OPL_REG_SUSTAIN_RELEASE, 0x74, i, 1); // medium sustain and release
        ise_opl_write_reg(ISE_OPL_REG_FEEDBACK, 0x06, i, 0);
        ise_opl_write_reg(ISE_OPL_REG_OCTAVE, 0x0, i, 0);           // turn off voice
        ise_opl_write_reg(ISE_OPL_REG_FEEDBACK, 0x30, i, 0);        // enable both speakers
    }
    
    // Set default voices, and turn them off
    for(i=6; i<15; i++) {
        ise_opl_write_reg(ISE_OPL_REG_FM_MULT, 0x01, i, 0);         // Set multiple to 1
        ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, 0x4F, i, 0);       // Set level to 40 dB
        ise_opl_write_reg(ISE_OPL_REG_ATTACK_DECAY, 0xF1, i, 0);    // Quick attack, long delay
        ise_opl_write_reg(ISE_OPL_REG_SUSTAIN_RELEASE, 0x53, i, 0); // medium sustain and release
        ise_opl_write_reg(ISE_OPL_REG_FM_MULT, 0x01, i, 1);         // Set multiple to 1
        ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, 0x10, i, 1);       // Set level to 47 dB
        ise_opl_write_reg(ISE_OPL_REG_ATTACK_DECAY, 0xD2, i, 1);    // Quick attack, long delay
        ise_opl_write_reg(ISE_OPL_REG_SUSTAIN_RELEASE, 0x74, i, 1); // medium sustain and release
        ise_opl_write_reg(ISE_OPL_REG_FEEDBACK, 0x06, i, 0);
        ise_opl_write_reg(ISE_OPL_REG_OCTAVE, 0x0, i, 0);           // turn off voice
        ise_opl_write_reg(ISE_OPL_REG_FEEDBACK, 0x30, i, 0);        // enable both speakers
    }

    // enable percussion mode, and turn off percussion instruments
    ise_opl_write_reg(ISE_OPL_REG_RHYTHM, ise_opl_rhythm_reg, 0, 0);

    // set percussion voices
    for(i=12; i<16; i++) {
        ise_opl_write_reg(ISE_OPL_REG_FM_MULT, 0x01, 15, i);         // Set multiple to 1
        ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, 0x10, 15, i);       // Set level to 47 dB
        ise_opl_write_reg(ISE_OPL_REG_ATTACK_DECAY, 0x88, 15, i);    // Quick attack, quick delay
        ise_opl_write_reg(ISE_OPL_REG_SUSTAIN_RELEASE, 0x88, 15, i); // quick sustain and release
        ise_opl_write_reg(ISE_OPL_REG_OCTAVE, 0x8, 15, i);           // turn off voice
        ise_opl_write_reg(ISE_OPL_REG_FEEDBACK, 0x30, 15, i);        // enable both speakers
    }
    
    //ise_opl_load_bank_file("standard.bnk");
    ise_op3_load_file("fat2.op3");
    //ise_op2_load_file("genmidi.op2");

    printf("opl3 base 0x%x\n", ise_opl_base);
    return ISE_OPL_DEVICE_OPL3;
}

void ise_opl_uninstall()
{
    if(ise_opl_write_reg) {
        int i;
        for(i=0; i<18; i++) {
            ise_opl_write_reg(ISE_OPL_REG_OCTAVE, 0, i, 0);
        }
        ise_opl_write_reg(ISE_OPL_REG_RHYTHM, 0, 0, 0);
    }
}

void ise_opl_play_track(ise_midi_event_t* track)
{
    ise_opl_track = track;
    ise_opl_cur_event = track;    
    ise_opl_last_time = ise_time_get_time() & ~0x3FL;
    ise_opl_systime_per_tick = 24;
    int i;
    for(i=0; i<16; i++) {
        ise_opl_current_instrument[i] = NULL;
    }
}

uint8_t ise_opl_get_percussion_from_note(uint8_t note, uint8_t* op)
{
    uint8_t percussion = 0x0;
    switch(note) {
    // percussion channel
    case 35:
    case 36:
        // Bass drum
        percussion = 0x10;
        //*channel = 6;
        *op = 15;
        break;
    case 38:
    case 40:
        // snare
        percussion = 0x08;
        //*channel = 7;
        *op = 16;
        break;
    case 41:
    case 43:
    case 45:
    case 47:
    case 48:
    case 50:
        // tom-tom
        percussion = 0x04;
        //*channel = 8;
        *op = 14;
        break;
    case 49:
    case 51:
    case 52:
    case 55:
    case 57:
    case 59:
        // cymbal
        percussion = 0x02;
        //*channel = 8;
        *op = 17;
        break;
    case 42:
    case 44:
    case 46:
        // hi hat
        percussion = 0x1;
        //*channel = 7;
        *op = 13;
        break;
    }

    return percussion;
}

void ise_opl_tick()
{
    if(ise_opl_base && ise_opl_cur_event) {
        bool done = false;
        uint32_t cur_time = ise_time_get_time() & ~0x3FL;
        uint32_t delta_time, delta_tick, event_delta_tick;
        uint8_t channel, op;
        ise_midi_event_t* cur_event = NULL;
        while(!done) {
            delta_time = (cur_time > ise_opl_last_time) ? cur_time - ise_opl_last_time : 0;
            delta_tick = delta_time / ise_opl_systime_per_tick;
            //if(cur_event == NULL) printf("0x%x 0x%x\n", delta_time, delta_tick);
            cur_event = ise_opl_cur_event;
            event_delta_tick = ise_midi_get_delta_time(&cur_event);
            if(delta_tick >= event_delta_tick) {
                ise_opl_cur_event = cur_event;
                // Process event
                switch(cur_event->f.command) {
                case ISE_MIDI_META_EVENT_TYPE_END_TRACK:
                    ise_opl_cur_event = NULL;//ise_opl_track;
                    break;
                case ISE_MIDI_META_EVENT_TYPE_SET_TEMPO:
                    ise_opl_systime_per_tick = cur_event->data & 0xFFFF;
                    break;
                }
                ise_opl_last_time += event_delta_tick * ise_opl_systime_per_tick;
                channel = cur_event->f.command & 0xF;
                // remap channels for percussive mode
                /*if(channel == 15) {
                    // opl percussives are channels 6,7, and 8
                    channel = 6;
                } else if(channel >= 6) {
                    // for non-percussive channels, skip over 6, 7, and 8
                    channel += 3;
                }*/
                uint8_t note = cur_event->f.data0;
                uint8_t velocity = cur_event->f.data1;
                uint8_t output_level = (velocity > 0x3F) ? 0x3F : velocity;// >> 1;
                uint16_t freq, octave;
                switch(cur_event->f.command & 0xF0) {
                case ISE_MIDI_MSG_NOTE_OFF:
                    //ise_opl_write_reg(ISE_OPL_REG_OCTAVE, 0x0, channel, 0);
                    if(channel != 15) {
                        //non percussion channel
                        ise_opl_write_reg(ISE_OPL_REG_OCTAVE, 0, channel, 0);
                        //if(cur_event->f.data1 == 0xFF) {
                        //    printf("ch %d done\n", channel);
                        //}
                    } else {
                        uint8_t percussion = ise_opl_get_percussion_from_note(cur_event->f.data0, &op);
                        if(percussion) {
                            ise_opl_rhythm_reg &= ~percussion;
                            ise_opl_write_reg(ISE_OPL_REG_RHYTHM, ise_opl_rhythm_reg, 0, 0);
                        }
                    }
                    break;
                case ISE_MIDI_MSG_NOTE_ON:
                    if(ise_opl_current_instrument[channel]) {
                        note += (int8_t) ise_opl_current_instrument[channel]->note_offset[0];
                    }
                    freq = ise_opl_freq_table[note % 12];
                    octave = note / 12;
                    if(octave > 0) octave--;
                    else freq >>= 1;   // on the lowest octave, halve the frequency
                    if(octave > 7) octave = 7;
                    if(channel != 15) {
                        uint8_t ksl;
                        // non percussion channel
                        output_level = (~output_level) & 0x3F;  // invert for non-percussive channels
                        if((ise_opl_4op_channel >> channel) & 1) {
                            // 4op mode - op3 must change volume
                            ksl = ise_opl_key_scale[2*channel+19] & 0xC0;
                            ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, output_level | ksl, channel, 3);
                            // op0 changes volume if first is AM mode
                            if(ise_opl_channel_mode[channel] & 0x1) {
                                ksl = ise_opl_key_scale[2*channel] & 0xC0;
                                ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, output_level | ksl, channel, 0);
                            }
                            // if second is AM mode, then either change op1 (if first is FM) or op2 (if AM)
                            if(ise_opl_channel_mode[channel] & 0x2) {
                                int op = (ise_opl_channel_mode[channel] & 0x1) ? 2 : 1;
                                ksl = ise_opl_key_scale[2*channel + ((ise_opl_channel_mode[channel] & 0x1) ? 18 : 1)] & 0xC0;
                                ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, output_level, channel | ksl, op);
                            }
                        } else {
                            // 2op mode - op1 must change volume; op0 does too, if in AM mode
                            ksl = ise_opl_key_scale[2*channel+1] & 0xC0;
                            ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, output_level | ksl, channel, 1);
                            if(ise_opl_channel_mode[channel] & 0x1) {
                                ksl = ise_opl_key_scale[2*channel] & 0xC0;
                                ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, output_level | ksl, channel, 0);
                            }
                            if(ise_opl_channel_mode[channel] & 0x80) {
                                // pseudo-4op mode
                                ksl = ise_opl_key_scale[2*channel+19] & 0xC0;
                                ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, output_level | ksl, channel, 3);
                                if(ise_opl_channel_mode[channel] & 0x2) {
                                    ksl = ise_opl_key_scale[2*channel+18] & 0xC0;
                                    ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, output_level | ksl, channel, 2);
                                }
                            }
                        }
                        ise_opl_write_reg(ISE_OPL_REG_FREQ, freq & 0xFF, channel, 0);
                        ise_opl_write_reg(ISE_OPL_REG_OCTAVE, 0x20 | (octave << 2) | (freq >> 8), channel, 0);
                        if(ise_opl_channel_mode[channel] & 0x80) {
                            // pseudo-4op mode
                            note = cur_event->f.data0;
                            if(ise_opl_current_instrument[channel]) {
                                note += (int8_t) ise_opl_current_instrument[channel]->note_offset[1];
                                freq = ise_opl_freq_table[note % 12];
                                if(ise_opl_current_instrument[channel]->detune != 0x80) {
                                    // detune frequency
                                    int detune = ise_opl_current_instrument[channel]->detune - 0x80;
                                    freq += detune / 2;
                                }
                            } else {
                                freq = ise_opl_freq_table[note % 12];
                            }
                            octave = note / 12;
                            if(octave > 0) octave--;
                            else freq >>= 1;   // on the lowest octave, halve the frequency
                            if(octave > 7) octave = 7;
                            ise_opl_write_reg(ISE_OPL_REG_FREQ, freq & 0xFF, channel, 2);
                            ise_opl_write_reg(ISE_OPL_REG_OCTAVE, 0x20 | (octave << 2) | (freq >> 8), channel, 2);
                        }
                    } else {
                        uint8_t percussion = ise_opl_get_percussion_from_note(cur_event->f.data0, &op);
                        if(percussion) {
                            ise_opl_rhythm_reg |= percussion;
                            ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, output_level, channel, op);
                            ise_opl_write_reg(ISE_OPL_REG_RHYTHM, ise_opl_rhythm_reg, 0, 0);
                        }
                    }
                    break;
                case ISE_MIDI_MSG_PROGRAM_CHANGE:
                    if(channel != 15) {
                        //printf("ch %d: ins 0x%x\n", channel, note);
                        ise_opl_current_instrument[channel] = &(ise_opl_instrument[note]);
                        ise_opl_write_reg(ISE_OPL_REG_FM_MULT, ise_opl_instrument[note].opl3.op1.fm_mult, channel, 0);
                        ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, ise_opl_instrument[note].opl3.op1.key_scale, channel, 0);
                        ise_opl_write_reg(ISE_OPL_REG_ATTACK_DECAY, ise_opl_instrument[note].opl3.op1.attack_decay, channel, 0);
                        ise_opl_write_reg(ISE_OPL_REG_SUSTAIN_RELEASE, ise_opl_instrument[note].opl3.op1.sustain_release, channel, 0);
                        ise_opl_write_reg(ISE_OPL_REG_WAVE_SELECT, ise_opl_instrument[note].opl3.op1.wave_select, channel, 0);
                        ise_opl_key_scale[2*channel] = ise_opl_instrument[note].opl3.op1.key_scale;

                        ise_opl_write_reg(ISE_OPL_REG_FM_MULT, ise_opl_instrument[note].opl3.op2.fm_mult, channel, 1);
                        ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, ise_opl_instrument[note].opl3.op2.key_scale, channel, 1);
                        ise_opl_write_reg(ISE_OPL_REG_ATTACK_DECAY, ise_opl_instrument[note].opl3.op2.attack_decay, channel, 1);
                        ise_opl_write_reg(ISE_OPL_REG_SUSTAIN_RELEASE, ise_opl_instrument[note].opl3.op2.sustain_release, channel, 1);
                        ise_opl_write_reg(ISE_OPL_REG_WAVE_SELECT, ise_opl_instrument[note].opl3.op2.wave_select, channel, 1);
                        ise_opl_key_scale[2*channel+1] = ise_opl_instrument[note].opl3.op2.key_scale;
                        
                        ise_opl_write_reg(ISE_OPL_REG_FEEDBACK, ise_opl_instrument[note].opl3.connection12, channel, 0);
                        
                        if(ise_opl_instrument[note].opl3.flags & 0x81) {
                            ise_opl_write_reg(ISE_OPL_REG_FM_MULT, ise_opl_instrument[note].opl3.op3.fm_mult, channel, 2);
                            ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, ise_opl_instrument[note].opl3.op3.key_scale, channel, 2);
                            ise_opl_write_reg(ISE_OPL_REG_ATTACK_DECAY, ise_opl_instrument[note].opl3.op3.attack_decay, channel, 2);
                            ise_opl_write_reg(ISE_OPL_REG_SUSTAIN_RELEASE, ise_opl_instrument[note].opl3.op3.sustain_release, channel, 2);
                            ise_opl_write_reg(ISE_OPL_REG_WAVE_SELECT, ise_opl_instrument[note].opl3.op3.wave_select, channel, 2);
                            ise_opl_key_scale[2*channel+18] = ise_opl_instrument[note].opl3.op3.key_scale;

                            ise_opl_write_reg(ISE_OPL_REG_FM_MULT, ise_opl_instrument[note].opl3.op4.fm_mult, channel, 3);
                            ise_opl_write_reg(ISE_OPL_REG_KEY_SCALE, ise_opl_instrument[note].opl3.op4.key_scale, channel, 3);
                            ise_opl_write_reg(ISE_OPL_REG_ATTACK_DECAY, ise_opl_instrument[note].opl3.op4.attack_decay, channel, 3);
                            ise_opl_write_reg(ISE_OPL_REG_SUSTAIN_RELEASE, ise_opl_instrument[note].opl3.op4.sustain_release, channel, 3);
                            ise_opl_write_reg(ISE_OPL_REG_WAVE_SELECT, ise_opl_instrument[note].opl3.op4.wave_select, channel, 3);
                            ise_opl_write_reg(ISE_OPL_REG_FEEDBACK, ise_opl_instrument[note].opl3.connection34, channel, 2);
                            ise_opl_key_scale[2*channel+19] = ise_opl_instrument[note].opl3.op4.key_scale;
                            
                            ise_opl_channel_mode[channel] = (ise_opl_instrument[note].opl3.connection12 & 1) | ((ise_opl_instrument[note].opl3.connection34 & 1) << 1);
                            ise_opl_channel_mode[channel] |= (ise_opl_instrument[note].opl3.flags & 0x80);

                        } else {
                            ise_opl_channel_mode[channel] = (ise_opl_instrument[note].opl3.connection12 & 1);
                        }
                        if(ise_opl_instrument[note].opl3.flags & 0x01) {
                            ise_opl_4op_channel |= 1 << channel;
                        } else {
                            ise_opl_4op_channel &= ~(1 << channel);
                        }
                        ise_opl_write_reg(ISE_OPL_REG_FOUR_OP_ENABLE, ise_opl_4op_channel, 0, 0);
                        //printf("ch %d using instr %d\n", channel, note);
                    }
                    break;
                }
                if(cur_event->f.command != ISE_MIDI_META_EVENT_TYPE_END_TRACK) {
                    ise_opl_cur_event++;
                //} else {
                //    printf("OPL done\n");
                }
            } else {
                done = true;
            }
        }
    }
}
