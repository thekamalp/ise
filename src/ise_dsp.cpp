// InfiniScroll Engine
// ise_dsp.h
// Digital Sound Processor related header
//
// Kamal Pillai
// 5/15/2019

#include "ise.h"
extern uint32_t ise_opl_base;
uint32_t ise_dsp_base;

uint16_t ise_dsp_version = 0;
uint8_t ise_dsp_irq = 7;
uint8_t ise_dsp_dma8_channel = 1;
uint8_t ise_dsp_dma16_channel = 5;

uint8_t* ise_dsp_mem_buffer = NULL;

ise_dsp_pcm_t* ise_dsp_pcm = NULL;
uint8_t* ise_dsp_pcm_data = NULL;
uint32_t ise_dsp_pcm_length = 0;
uint8_t ise_dsp_pcm_subsample = 0;

ise_midi_event_t* ise_dsp_midi_track = NULL;
ise_midi_event_t* ise_dsp_midi_event = NULL;
uint32_t ise_dsp_midi_delta_tick = 0;
uint32_t ise_dsp_cur_sample_in_midi_tick = 0;
uint32_t ise_dsp_samples_per_midi_tick = 240;
uint8_t ise_dsp_percussive_note_mapping[ISE_DSP_NUM_PERCUSSIVE_VOICES] = {0};

/*uint32_t ise_dsp_freq_table[12] = {0x01000000, 0xF1A1BF, 0xE411F0, 0xD744FD,
                                     0xCB2FF5, 0XBFC887, 0xB504F3, 0xAADC08,
                                     0xA14518, 0x9837F0, 0x8FACD6, 0x879C7C};
*/
/*uint32_t ise_dsp_freq_table[240] = {0x1000000, 0xFF42FF, 0xFE868A, 0xFDCAA0, 0xFD0F41, 0xFC546C, 0xFB9A21, 0xFAE060, 0xFA2727, 0xF96E78, 0xF8B651, 0xF7FEB2, 0xF7479A, 0xF69109, 0xF5DB00, 0xF5257D, 0xF4707F, 0xF3BC08, 0xF30815, 0xF254A8,
    0xF1A1BF, 0xF0EF5A, 0xF03D78, 0xEF8C1B, 0xEEDB40, 0xEE2AE7, 0xED7B11, 0xECCBBC, 0xEC1CEA, 0xEB6E98, 0xEAC0C6, 0xEA1376, 0xE966A5, 0xE8BA53, 0xE80E81, 0xE7632E, 0xE6B859, 0xE60E03, 0xE5642A, 0xE4BACE,
    0xE411F0, 0xE3698E, 0xE2C1A8, 0xE21A3F, 0xE17351, 0xE0CCDE, 0xE026E7, 0xDF8169, 0xDEDC66, 0xDE37DD, 0xDD93CD, 0xDCF037, 0xDC4D19, 0xDBAA73, 0xDB0846, 0xDA6690, 0xD9C552, 0xD9248B, 0xD8843B, 0xD7E460,
    0xD744FC, 0xD6A60E, 0xD60795, 0xD56991, 0xD4CC01, 0xD42EE6, 0xD3923F, 0xD2F60C, 0xD25A4B, 0xD1BEFE, 0xD12424, 0xD089BC, 0xCFEFC5, 0xCF5641, 0xCEBD2E, 0xCE248C, 0xCD8C5A, 0xCCF499, 0xCC5D48, 0xCBC667,
    0xCB2FF5, 0xCA99F2, 0xCA045E, 0xC96F38, 0xC8DA80, 0xC84637, 0xC7B25A, 0xC71EEB, 0xC68BE9, 0xC5F953, 0xC5672A, 0xC4D56C, 0xC4441A, 0xC3B333, 0xC322B7, 0xC292A6, 0xC20300, 0xC173C3, 0xC0E4F0, 0xC05687,
    0xBFC886, 0xBF3AEF, 0xBEADC0, 0xBE20F9, 0xBD949A, 0xBD08A3, 0xBC7D13, 0xBBF1EB, 0xBB6728, 0xBADCCD, 0xBA52D7, 0xB9C948, 0xB9401E, 0xB8B759, 0xB82EF9, 0xB7A6FE, 0xB71F67, 0xB69835, 0xB61166, 0xB58AFB,
    0xB504F3, 0xB47F4E, 0xB3FA0B, 0xB3752B, 0xB2F0AD, 0xB26C91, 0xB1E8D7, 0xB1657D, 0xB0E285, 0xB05FED, 0xAFDDB6, 0xAF5BDF, 0xAEDA68, 0xAE5950, 0xADD898, 0xAD583E, 0xACD844, 0xAC58A8, 0xABD96A, 0xAB5A8A,
    0xAADC08, 0xAA5DE3, 0xA9E01B, 0xA962B0, 0xA8E5A2, 0xA868F0, 0xA7EC9A, 0xA770A0, 0xA6F502, 0xA679BE, 0xA5FED6, 0xA58449, 0xA50A16, 0xA4903D, 0xA416BE, 0xA39D99, 0xA324CD, 0xA2AC5A, 0xA23441, 0xA1BC80,
    0xA14517, 0xA0CE07, 0xA0574E, 0x9FE0EE, 0x9F6AE4, 0x9EF532, 0x9E7FD6, 0x9E0AD2, 0x9D9623, 0x9D21CB, 0x9CADC9, 0x9C3A1C, 0x9BC6C5, 0x9B53C3, 0x9AE116, 0x9A6EBD, 0x99FCB9, 0x998B09, 0x9919AD, 0x98A8A5,
    0x9837F0, 0x97C78E, 0x97577F, 0x96E7C4, 0x96785A, 0x960943, 0x959A7E, 0x952C0A, 0x94BDE8, 0x945018, 0x93E298, 0x93756A, 0x93088C, 0x929BFE, 0x922FC1, 0x91C3D3, 0x915835, 0x90ECE7, 0x9081E7, 0x901737,
    0x8FACD6, 0x8F42C3, 0x8ED8FE, 0x8E6F88, 0x8E065F, 0x8D9D84, 0x8D34F6, 0x8CCCB6, 0x8C64C2, 0x8BFD1C, 0x8B95C1, 0x8B2EB4, 0x8AC7F2, 0x8A617C, 0x89FB51, 0x899573, 0x892FDF, 0x88CA96, 0x886598, 0x8800E5,
    0x879C7C, 0x87385D, 0x86D488, 0x8670FD, 0x860DBB, 0x85AAC3, 0x854814, 0x84E5AD, 0x84838F, 0x8421BA, 0x83C02C, 0x835EE7, 0x82FDEA, 0x829D34, 0x823CC6, 0x81DC9F, 0x817CBE, 0x811D25, 0x80BDD2, 0x805EC6};
*/
uint32_t ise_dsp_freq_table[300] = {
   0x1000000, 0xFF68C1, 0xFED1DB, 0xFE3B4F, 0xFDA51B, 0xFD0F41, 0xFC79BF, 0xFBE495, 0xFB4FC3, 0xFABB49, 0xFA2727, 0xF9935D, 0xF8FFEA, 0xF86CCE, 0xF7DA08, 0xF7479A, 0xF6B582, 0xF623C0, 0xF59255, 0xF5013F, 0xF4707F, 0xF3E015, 0xF35000, 0xF2C040, 0xF230D5,
    0xF1A1BF, 0xF112FD, 0xF08490, 0xEFF676, 0xEF68B1, 0xEEDB40, 0xEE4E22, 0xEDC157, 0xED34E0, 0xECA8BB, 0xEC1CEA, 0xEB916A, 0xEB063E, 0xEA7B63, 0xE9F0DB, 0xE966A5, 0xE8DCC0, 0xE8532C, 0xE7C9EA, 0xE740F9, 0xE6B859, 0xE6300A, 0xE5A80B, 0xE5205C, 0xE498FE,
    0xE411F0, 0xE38B31, 0xE304C2, 0xE27EA3, 0xE1F8D2, 0xE17351, 0xE0EE1F, 0xE0693B, 0xDFE4A6, 0xDF605F, 0xDEDC66, 0xDE58BC, 0xDDD55F, 0xDD524F, 0xDCCF8D, 0xDC4D19, 0xDBCAF1, 0xDB4917, 0xDAC789, 0xDA4647, 0xD9C552, 0xD944A9, 0xD8C44D, 0xD8443B, 0xD7C476,
    0xD744FC, 0xD6C5CE, 0xD646EA, 0xD5C852, 0xD54A04, 0xD4CC01, 0xD44E49, 0xD3D0DA, 0xD353B6, 0xD2D6DC, 0xD25A4B, 0xD1DE04, 0xD16207, 0xD0E653, 0xD06AE8, 0xCFEFC5, 0xCF74EC, 0xCEFA5B, 0xCE8013, 0xCE0612, 0xCD8C5A, 0xCD12EA, 0xCC99C1, 0xCC20E0, 0xCBA847,
    0xCB2FF5, 0xCAB7E9, 0xCA4025, 0xC9C8A8, 0xC95171, 0xC8DA80, 0xC863D6, 0xC7ED72, 0xC77754, 0xC7017C, 0xC68BE9, 0xC6169C, 0xC5A194, 0xC52CD1, 0xC4B853, 0xC4441A, 0xC3D026, 0xC35C76, 0xC2E90A, 0xC275E3, 0xC20300, 0xC19060, 0xC11E05, 0xC0ABEC, 0xC03A18,
    0xBFC886, 0xBF5738, 0xBEE62C, 0xBE7564, 0xBE04DE, 0xBD949A, 0xBD2499, 0xBCB4DA, 0xBC455D, 0xBBD622, 0xBB6728, 0xBAF871, 0xBA89FA, 0xBA1BC5, 0xB9ADD1, 0xB9401E, 0xB8D2AC, 0xB8657A, 0xB7F889, 0xB78BD8, 0xB71F67, 0xB6B337, 0xB64746, 0xB5DB96, 0xB57024,
    0xB504F3, 0xB49A00, 0xB42F4D, 0xB3C4D9, 0xB35AA4, 0xB2F0AD, 0xB286F5, 0xB21D7C, 0xB1B441, 0xB14B44, 0xB0E285, 0xB07A04, 0xB011C1, 0xAFA9BB, 0xAF41F3, 0xAEDA68, 0xAE731A, 0xAE0C09, 0xADA535, 0xAD3E9E, 0xACD844, 0xAC7226, 0xAC0C44, 0xABA69F, 0xAB4135,
    0xAADC08, 0xAA7716, 0xAA1260, 0xA9ADE5, 0xA949A6, 0xA8E5A2, 0xA881D9, 0xA81E4B, 0xA7BAF8, 0xA757E0, 0xA6F502, 0xA6925E, 0xA62FF5, 0xA5CDC6, 0xA56BD1, 0xA50A16, 0xA4A894, 0xA4474C, 0xA3E63E, 0xA38569, 0xA324CD, 0xA2C46A, 0xA26440, 0xA2044F, 0xA1A497,
    0xA14517, 0xA0E5D0, 0xA086C1, 0xA027EA, 0x9FC94B, 0x9F6AE4, 0x9F0CB5, 0x9EAEBD, 0x9E50FD, 0x9DF375, 0x9D9623, 0x9D3909, 0x9CDC26, 0x9C7F7A, 0x9C2304, 0x9BC6C5, 0x9B6ABC, 0x9B0EEA, 0x9AB34E, 0x9A57E9, 0x99FCB9, 0x99A1BF, 0x9946FB, 0x98EC6C, 0x989213,
    0x9837F0, 0x97DE01, 0x978448, 0x972AC4, 0x96D175, 0x96785A, 0x961F74, 0x95C6C3, 0x956E46, 0x9515FD, 0x94BDE8, 0x946608, 0x940E5B, 0x93B6E2, 0x935F9D, 0x93088C, 0x92B1AE, 0x925B03, 0x92048B, 0x91AE47, 0x915835, 0x910256, 0x90ACAA, 0x905731, 0x9001EA,
    0x8FACD6, 0x8F57F3, 0x8F0343, 0x8EAEC5, 0x8E5A79, 0x8E065F, 0x8DB276, 0x8D5EBF, 0x8D0B3A, 0x8CB7E5, 0x8C64C2, 0x8C11D0, 0x8BBF0F, 0x8B6C7F, 0x8B1A20, 0x8AC7F2, 0x8A75F4, 0x8A2426, 0x89D289, 0x89811C, 0x892FDF, 0x88DED2, 0x888DF5, 0x883D48, 0x87ECCA,
    0x879C7C, 0x874C5D, 0x86FC6E, 0x86ACAE, 0x865D1D, 0x860DBB, 0x85BE88, 0x856F84, 0x8520AF, 0x84D208, 0x84838F, 0x843545, 0x83E729, 0x83993B, 0x834B7C, 0x82FDEA, 0x82B086, 0x826350, 0x821647, 0x81C96C, 0x817CBE, 0x81303E, 0x80E3EB, 0x8097C5, 0x804BCC
};

uint32_t ise_dsp_pcm_play_tick = 0;
uint32_t ise_dsp_pcm_load_tick = 0;

bool ise_dsp_irq_orig_mask = false;
void _interrupt (FAR *prev_dsp_isr)() = NULL;

bool (*ise_dsp_reset)() = NULL;
void (*ise_dsp_play)(ise_dsp_pcm_t* pcm) = NULL;
void (*ise_dsp_pause)() = NULL;
void (*ise_dsp_stop_loop)() = NULL;

ise_dsp_sf2_t ise_dsp_sf2;
ise_dsp_vox_t ise_dsp_vox[ISE_DSP_NUM_VOICES] = {0};

// --------------------------------------------------------------------
// Convert one PCM format to another
void ise_dsp_convert_pcm(ise_dsp_pcm_t* src, ise_dsp_pcm_t* dst)
{
    //printf("src ch %d by %d sr %d\n", src->num_channels, src->bytes_per_sample, src->samples_per_second);
    int src_sample_size = src->bytes_per_sample * src->num_channels;
    int src_samples = src->size / src_sample_size;
    uint32_t dst_to_src8 = (dst->samples_per_second << 8) / src->samples_per_second;
    uint32_t src_to_dst8 = (src->samples_per_second << 8) / dst->samples_per_second;
    int dst_samples = (src_samples * dst_to_src8) >> 8;
    int dst_sample_size = dst->bytes_per_sample * dst->num_channels;
    dst->size = dst_sample_size * dst_samples;
    dst->mem = (uint8_t*) malloc(dst->size);
    //memcpy(dst->mem, src->mem, dst->size);
    //memset(dst->mem, 0, dst->size);
    int max_channels = (src->num_channels > dst->num_channels) ? src->num_channels : dst->num_channels;
    uint32_t src_channel_mask = src->num_channels - 1;
    uint32_t dst_channel_mask = dst->num_channels - 1;
    int d, s, c;
    uint8_t* src_mem8 = src->mem;
    uint8_t* dst_mem8 = dst->mem;
    int16_t* src_mem16 = (int16_t*) src_mem8;
    int16_t* dst_mem16 = (int16_t*) dst_mem8;
    //printf("ssamples %d dsamples %d\n", src_samples, dst_samples);
    int convert_mode = ((dst->bytes_per_sample - 1) << 1) | (src->bytes_per_sample - 1);
    bool stero_to_mono = (dst->num_channels == 2) && (src->num_channels == 1);
    for(d=0, s = 0; d<dst_samples; d++, s += src_to_dst8) {
        for(c=0; c<max_channels; c++) {
            int dc = d*dst->num_channels + (c & dst_channel_mask);
            int sc = s*src->num_channels + (c & src_channel_mask);
            int sci = sc >> 8;
            int scf = sc & 0xFF;
            int sample = 0;
            switch(convert_mode) {
            case 0:  // uint8 -> uint8
                sample = ((src_mem8[sci] * (0x100 - scf)) + (src_mem8[sci+1] * scf)) >> 8;
                if(stero_to_mono && c==1) {
                    *(dst_mem8 + dc) >>= 1;
                    *(dst_mem8 + dc) += (uint8_t) (sample >> 1);
                } else {
                    *(dst_mem8 + dc) = (uint8_t) sample;
                }
                break;
            case 1:  // int16 -> uint8
                sample = ((src_mem16[sci] * (0x100 - scf)) + (src_mem16[sci+1] * scf)) >> 8;
                sample += 0x8000;
                sample >>= 8;
                if(stero_to_mono && c==1) {
                    *(dst_mem8 + dc) >>= 1;
                    *(dst_mem8 + dc) += (uint8_t) (sample >> 1);
                } else {
                    *(dst_mem8 + dc) = (uint8_t) sample;
                }
                break;
            case 2:  // uint8 -> int16
                sample = ((src_mem8[sci] * (0x100 - scf)) + (src_mem8[sci+1] * scf)) >> 8;
                sample |= (sample << 8);
                sample -= 0x8000;
                if(stero_to_mono && c==1) {
                    *(dst_mem16 + dc) >>= 1;
                    *(dst_mem16 + dc) += (int16_t) (sample >> 1);
                } else {
                    *(dst_mem16 + dc) = (int16_t) sample;
                }
                break;
            case 3:  // int16 -> int16
                sample = ((src_mem16[sci] * (0x100 - scf)) + (src_mem16[sci+1] * scf)) >> 8;
                if(stero_to_mono && c==1) {
                    *(dst_mem16 + dc) >>= 1;
                    *(dst_mem16 + dc) += (int16_t) (sample >> 1);
                } else {
                    *(dst_mem16 + dc) = (int16_t) sample;
                }
                break;
            }
        }
    }
}

// Loads a wav file, allocate memory, return attributes
void ise_dsp_load_wav_file(const char* filename, ise_dsp_pcm_t* pcm)
{
    memset(pcm, 0, sizeof(ise_dsp_pcm_t));

    FILE* file = fopen( filename, "rb" );
    if( file == NULL ) return;

    ise_dsp_riff_header_t header;
    ise_dsp_riff_subchunk_header_t subchunk;
    ise_dsp_wav_format_t format;
    uint32_t sub_chunk1_id;
    
    // read the header, and next subchunk
    fread(&header, sizeof(ise_dsp_riff_header_t), 1, file);
    fread(&subchunk, sizeof(ise_dsp_riff_subchunk_header_t), 1, file);
    fread(&format, sizeof(ise_dsp_wav_format_t), 1, file);
    // if there's extra space after first subchunk, go past it
    if(subchunk.size > sizeof(ise_dsp_wav_format_t)) fseek(file, subchunk.size - sizeof(ise_dsp_wav_format_t), SEEK_CUR);
    sub_chunk1_id = subchunk.id;
    // read the data subchunk
    fread(&subchunk, sizeof(ise_dsp_riff_subchunk_header_t), 1, file);

    // Make sure all id's match, and we have linear pcm data
    if(header.chunk_id == ISE_DSP_RIFF_CHUNK_ID && header.format_id == ISE_DSP_WAV_FORMAT_ID &&
       sub_chunk1_id == ISE_DSP_WAV_SUBCHUNK_FMT_ID && subchunk.id == ISE_DSP_WAV_SUBCHUNK_DATA_ID &&
       format.audio_format == 1) {
        // fill pcm info
        ise_dsp_pcm_t src_pcm;
        src_pcm.bytes_per_sample = format.bits_per_sample / 8;
        src_pcm.num_channels = (uint8_t) format.num_channels;
        src_pcm.samples_per_second = format.sample_rate;
        src_pcm.size = subchunk.size;
        src_pcm.mem = (uint8_t*) malloc(src_pcm.size);
        if(src_pcm.mem) {
            fread(src_pcm.mem, src_pcm.size, 1, file);
        }
        // convert pcm to natively played format
        pcm->bytes_per_sample = 2;
        pcm->num_channels = 1;
        pcm->samples_per_second = 44100;
        //pcm->size = src_pcm.size;
        //pcm->mem = src_pcm.mem;
        ise_dsp_convert_pcm(&src_pcm, pcm);
        // free source memory
        free(src_pcm.mem);

        /*pcm->bytes_per_sample = format.bits_per_sample / 8;
        pcm->num_channels = (uint8_t) format.num_channels;
        pcm->samples_per_second = format.sample_rate;
        pcm->size = subchunk.size;
        pcm->mem = (uint8_t*) malloc(pcm->size);
        if(pcm->mem) {
            fread(pcm->mem, pcm->size, 1, file);
        }*/
    }
    fclose(file);
}

void ise_dsp_free_pcm(ise_dsp_pcm_t* pcm)
{
    if(pcm->mem) free(pcm->mem);
    pcm->mem = NULL;
}

// --------------------------------------------------------------------
// convert timecents to number of samples, assuming 44100 samples/second
uint32_t ise_dsp_sf2_timecents_to_samples(int16_t tc)
{
    const uint32_t TC_44100 = 18514;
    float norm_tc = (TC_44100 + tc) / 1200.0f;
    uint32_t samples = (norm_tc < 0.0f) ? 0 : pow(2, norm_tc);
    return samples;
}

// Process generator
void ise_dsp_process_sf2_gen(ise_dsp_sf2_preset_t* preset, ise_dsp_sf2_gen_list_t* gen)
{
    switch(gen->gen_op) {
    case ISE_DSP_SF2_GEN_START_ADDR_OFFSET:    preset->start_sample += gen->gen_amount.u16; break;
    case ISE_DSP_SF2_GEN_END_ADDR_OFFSET:      preset->end_sample += gen->gen_amount.u16; break;
    case ISE_DSP_SF2_GEN_START_LOOP_OFFSET:    preset->start_loop_sample += gen->gen_amount.u16; break;
    case ISE_DSP_SF2_GEN_END_LOOP_OFFSET:      preset->end_loop_sample += gen->gen_amount.u16; break;
    case ISE_DSP_SF2_GEN_START_ADDR_COARSE:    preset->start_sample += gen->gen_amount.u16 << 16; break;
    case ISE_DSP_SF2_GEN_END_ADDR_COARSE:      preset->end_sample += gen->gen_amount.u16 << 16; break;
    case ISE_DSP_SF2_GEN_KEY_RANGE:            preset->key_hi += gen->gen_amount.range.hi; preset->key_lo += gen->gen_amount.range.lo; break;
    case ISE_DSP_SF2_GEN_VEL_RANGE:            preset->vel_hi += gen->gen_amount.range.hi; preset->vel_lo += gen->gen_amount.range.lo; break;
    case ISE_DSP_SF2_GEN_START_LOOP_COARSE:    preset->start_loop_sample += gen->gen_amount.u16 << 16; break;
    case ISE_DSP_SF2_GEN_KEY_NUM:              preset->key_pitch += (uint8_t) gen->gen_amount.u16; break;
    case ISE_DSP_SF2_GEN_END_LOOP_COARSE:      preset->end_loop_sample += gen->gen_amount.u16 << 16; break;
    case ISE_DSP_SF2_GEN_SAMPLE_MODE:          preset->sample_mode = (uint8_t) gen->gen_amount.u16; break;
    case ISE_DSP_SF2_GEN_OVERRIDE_PITCH:       preset->key_pitch = (uint8_t) gen->gen_amount.u16; break;
    case ISE_DSP_SF2_GEN_DELAY_VOL_ENV:        preset->delay_samples = ise_dsp_sf2_timecents_to_samples(gen->gen_amount.s16); break;
    case ISE_DSP_SF2_GEN_ATTACK_VOL_ENV:       preset->attack_samples = ise_dsp_sf2_timecents_to_samples(gen->gen_amount.s16); break;
    case ISE_DSP_SF2_GEN_HOLD_VOL_ENV:         preset->hold_samples = ise_dsp_sf2_timecents_to_samples(gen->gen_amount.s16); break;
    case ISE_DSP_SF2_GEN_DECAY_VOL_ENV:        preset->decay_samples = ise_dsp_sf2_timecents_to_samples(gen->gen_amount.s16); break;
    case ISE_DSP_SF2_GEN_SUSTAIN_VOL_ENV:      preset->sustain_level = (0x10000 * gen->gen_amount.u16) / 1000; break;
    case ISE_DSP_SF2_GEN_RELEASE_VOL_ENV:      preset->release_samples = ise_dsp_sf2_timecents_to_samples(gen->gen_amount.s16); break;
    }
}

void ise_dsp_process_sf2_shdr(ise_dsp_sf2_preset_t* preset, ise_dsp_sf2_sample_t* shdr)
{
    preset->start_sample += shdr->start;
    preset->end_sample += shdr->end;
    preset->start_loop_sample += shdr->start_loop;
    preset->end_loop_sample += shdr->end_loop;
    preset->sample_rate = shdr->sample_rate;
    if(preset->key_pitch == 0) preset->key_pitch = shdr->orig_pitch;
    preset->pitch_correction += shdr->pitch_correction;
}

bool ise_dsp_sf2_presets_match(ise_dsp_sf2_preset_t* p0, ise_dsp_sf2_preset_t* p1)
{
    //printf("Comparing 0x%x to 0x%x ", p0, p1);
    bool match = p0->key_lo == p1->key_hi+1;
    //if(!match) {printf("ky %x %x\n", p1->key_lo, p1->key_hi); return match;}
    if(p0->vel_lo != p1->vel_lo) match = false;
    //if(!match) {printf("vl\n"); return match;}
    if(p0->vel_hi != p1->vel_hi) match = false;
    //if(!match) {printf("vh\n"); return match;}
    if(p0->key_pitch != p1->key_pitch) match = false;
    //if(!match) {printf("kp\n"); return match;}
    if(p0->pitch_correction != p1->pitch_correction) match = false;
    //if(!match) {printf("pc\n"); return match;}
    if(p0->sample_mode != p1->sample_mode) match = false;
    //if(!match) {printf("sm\n"); return match;}
    if(p0->start_sample != p1->start_sample) match = false;
    //if(!match) {printf("ss\n"); return match;}
    if(p0->end_sample != p1->end_sample) match = false;
    //if(!match) {printf("es\n"); return match;}
    if(p0->start_loop_sample != p1->start_loop_sample) match = false;
    //if(!match) {printf("sl\n"); return match;}
    if(p0->end_loop_sample != p1->end_loop_sample) match = false;
    //if(!match) {printf("el\n"); return match;}
    if(p0->sample_rate != p1->sample_rate) match = false;
    //if(!match) {printf("sr\n"); return match;}
    //printf(" match\n");
    if(p0->delay_samples != p1->delay_samples) match = false;
    if(p0->attack_samples != p1->attack_samples) match = false;
    if(p0->hold_samples != p1->hold_samples) match = false;
    if(p0->decay_samples != p1->decay_samples) match = false;
    if(p0->sustain_level != p1->sustain_level) match = false;
    if(p0->release_samples != p1->release_samples) match = false;
    return match;
}

// Load sound font 2 file
void ise_dsp_load_sf2_file(const char* filename, ise_dsp_sf2_t* sf2)
{
    memset(sf2, 0, sizeof(ise_dsp_sf2_t));

    FILE* file = fopen( filename, "rb" );
    if( file == NULL ) return;

    int read_size;
    ise_dsp_riff_header_t header;
    ise_dsp_riff_subchunk_header_t subchunk;

    // read RIFF header
    fread(&header, sizeof(ise_dsp_riff_header_t), 1, file);
    if(header.chunk_id != ISE_DSP_RIFF_CHUNK_ID || header.format_id != ISE_DSP_SF2_FORMAT_ID) {
        printf("Bad RIFF header: 0x%x\n", ftell(file));
        fclose(file);
        return;
    }
    
    // Read Info subchunk
    fread(&header, sizeof(ise_dsp_riff_header_t), 1, file);
    if(header.chunk_id != ISE_DSP_SF2_SUBCHUNK_LIST_ID || header.format_id != ISE_DSP_SF2_INFO_ID) {
        printf("Bad INFO subchunk: 0x%x\n", ftell(file));
        fclose(file);
        return;
    }
    // skip over info subchunk
    // need to subtract 4, since we already read the "INFO" id
    fseek(file, header.chunk_size-4, SEEK_CUR);
    
    // Read sample data subchunk
    fread(&header, sizeof(ise_dsp_riff_header_t), 1, file);
    if(header.chunk_id != ISE_DSP_SF2_SUBCHUNK_LIST_ID || header.format_id != ISE_DSP_SF2_SAMPLE_DATA_ID) {
        printf("Bad sample data: 0x%x\n", ftell(file));
        fclose(file);
        return;
    }

    // Read sample subchunk
    fread(&subchunk, sizeof(ise_dsp_riff_subchunk_header_t), 1, file);
    if(subchunk.id != ISE_DSP_SF2_SAMPLE_ID) {
        printf("Bad sample: 0x%x\n", ftell(file));
        fclose(file);
        return;
    }

    // allocate and read the sound sample data
    sf2->sample_data = (int16_t*) malloc(subchunk.size);
    read_size = fread(sf2->sample_data, 1, subchunk.size, file);
    if(read_size != subchunk.size) {
        printf("Error reading sample data; sample size=%d, read size=%d\n", subchunk.size, read_size);
        fclose(file);
        return;
    }

    // read the next chunk
    fread(&header, sizeof(ise_dsp_riff_header_t), 1, file);
    if(header.chunk_id == ISE_DSP_SF2_SAMPLE24_ID) {
        // it may be sm24, and if so, skip it
        // need to subtract 4, since we read the format field
        fseek(file, header.chunk_size-4, SEEK_CUR);
        // read the next chunk
        fread(&header, sizeof(ise_dsp_riff_header_t), 1, file);
    }

    // Check for preset data
    if(header.chunk_id != ISE_DSP_SF2_SUBCHUNK_LIST_ID || header.format_id != ISE_DSP_SF2_PRESET_DATA_ID) {
        printf("Bad preset header: 0x%x\n", ftell(file));
        fclose(file);
        return;
    }
    
    // Read preset header subchunk
    fread(&subchunk, sizeof(ise_dsp_riff_subchunk_header_t), 1, file);
    if(subchunk.id != ISE_DSP_SF2_PRESET_HEADER_ID) {
        printf("Bad preset header: 0x%x\n", ftell(file));
        fclose(file);
        return;
    }
    uint32_t num_preset_headers = subchunk.size / sizeof(ise_dsp_sf2_phdr_t);
    ise_dsp_sf2_phdr_t* phdr = (ise_dsp_sf2_phdr_t*) malloc(subchunk.size);
    if(phdr == NULL) {
        printf("Could not allocate phdr\n");
        fclose(file);
        return;
    }
    read_size = fread(phdr, sizeof(ise_dsp_sf2_phdr_t), num_preset_headers, file);
    if(read_size != num_preset_headers) {
        printf("Incorrect preset headers read: %d expected: %d\n", read_size, num_preset_headers);
        free(phdr);
        fclose(file);
        return;
    }
    
    // Read pbag
    fread(&subchunk, sizeof(ise_dsp_riff_subchunk_header_t), 1, file);
    if(subchunk.id != ISE_DSP_SF2_PRESET_BAG_ID) {
        printf("Bad preset bag: 0x%x\n", ftell(file));
        free(phdr);
        fclose(file);
        return;
    }
    uint32_t num_preset_bags = subchunk.size / sizeof(ise_dsp_sf2_bag_t);
    ise_dsp_sf2_bag_t* pbag = (ise_dsp_sf2_bag_t*) malloc(subchunk.size);
    if(pbag == NULL) {
        printf("Could not allocate pbag\n");
        free(phdr);
        fclose(file);
        return;
    }
    read_size = fread(pbag, sizeof(ise_dsp_sf2_bag_t), num_preset_bags, file);
    if(read_size != num_preset_bags) {
        printf("Incorrect preset bags read: %d expected: %d\n", read_size, num_preset_bags);
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    
    // Skip over pmod
    fread(&subchunk, sizeof(ise_dsp_riff_subchunk_header_t), 1, file);
    if(subchunk.id != ISE_DSP_SF2_PRESET_MOD_ID) {
        printf("Bad preset mod: 0x%x\n", ftell(file));
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    fseek(file, subchunk.size, SEEK_CUR);
    
    // Read pgen
    fread(&subchunk, sizeof(ise_dsp_riff_subchunk_header_t), 1, file);
    if(subchunk.id != ISE_DSP_SF2_PRESET_GEN_ID) {
        printf("Bad preset gen: 0x%x\n", ftell(file));
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    uint32_t num_preset_gens = subchunk.size / sizeof(ise_dsp_sf2_gen_list_t);
    ise_dsp_sf2_gen_list_t* pgen = (ise_dsp_sf2_gen_list_t*) malloc(subchunk.size);
    if(pgen == NULL) {
        printf("Could not allocate pgen\n");
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    read_size = fread(pgen, sizeof(ise_dsp_sf2_gen_list_t), num_preset_gens, file);
    if(read_size != num_preset_gens) {
        printf("Incorrect preset gens read: %d expected: %d\n", read_size, num_preset_gens);
        free(pgen);
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    
    // Read inst
    fread(&subchunk, sizeof(ise_dsp_riff_subchunk_header_t), 1, file);
    if(subchunk.id != ISE_DSP_SF2_INST_ID) {
        printf("Bad inst: 0x%x\n", ftell(file));
        free(pgen);
        free(pbag);
        free(phdr);
        fclose(file);
    }
    uint32_t num_insts = subchunk.size / sizeof(ise_dsp_sf2_inst_t);
    ise_dsp_sf2_inst_t* inst = (ise_dsp_sf2_inst_t*) malloc(subchunk.size);
    if(inst == NULL) {
        printf("Could not allocate inst\n");
        free(pgen);
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    read_size = fread(inst, sizeof(ise_dsp_sf2_inst_t), num_insts, file);
    if(read_size != num_insts) {
        printf("Incorrect inst read: %d expected: %d\n", read_size, num_insts);
        free(inst);
        free(pgen);
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    
    // read ibag
    fread(&subchunk, sizeof(ise_dsp_riff_subchunk_header_t), 1, file);
    if(subchunk.id != ISE_DSP_SF2_INST_BAG_ID) {
        printf("Bad inst bag: 0x%x\n", ftell(file));
        free(inst);
        free(pgen);
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    uint32_t num_inst_bags = subchunk.size / sizeof(ise_dsp_sf2_bag_t);
    ise_dsp_sf2_bag_t* ibag = (ise_dsp_sf2_bag_t*) malloc(subchunk.size);
    if(ibag == NULL) {
        printf("Could not allocate ibag\n");
        free(inst);
        free(pgen);
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    read_size = fread(ibag, sizeof(ise_dsp_sf2_bag_t), num_inst_bags, file);
    if(read_size != num_inst_bags) {
        printf("Incorrect inst bags read: %d expected: %d\n", read_size, num_inst_bags);
        free(ibag);
        free(inst);
        free(pgen);
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }

    // skip over imod
    fread(&subchunk, sizeof(ise_dsp_riff_subchunk_header_t), 1, file);
    if(subchunk.id != ISE_DSP_SF2_INST_MOD_ID) {
        printf("Bad inst mod: 0x%x\n", ftell(file));
        free(ibag);
        free(inst);
        free(pgen);
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    fseek(file, subchunk.size, SEEK_CUR);

    // read igen
    fread(&subchunk, sizeof(ise_dsp_riff_subchunk_header_t), 1, file);
    if(subchunk.id != ISE_DSP_SF2_INST_GEN_ID) {
        printf("Bad inst gen: 0x%x\n", ftell(file));
        free(ibag);
        free(inst);
        free(pgen);
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    uint32_t num_inst_gens = subchunk.size / sizeof(ise_dsp_sf2_gen_list_t);
    ise_dsp_sf2_gen_list_t* igen = (ise_dsp_sf2_gen_list_t*) malloc(subchunk.size);
    if(igen == NULL) {
        printf("Could not allocate inst gen\n");
        free(ibag);
        free(inst);
        free(pgen);
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    read_size = fread(igen, sizeof(ise_dsp_sf2_gen_list_t), num_inst_gens, file);
    if(read_size != num_inst_gens) {
        printf("Incorrect inst gens read: %d expected: %d\n", read_size, num_inst_gens);
        free(igen);
        free(ibag);
        free(inst);
        free(pgen);
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    
    // Read shdr
    fread(&subchunk, sizeof(ise_dsp_riff_subchunk_header_t), 1, file);
    if(subchunk.id != ISE_DSP_SF2_SAMPLE_HEADER_ID) {
        printf("Bad sample header: 0x%x\n", ftell(file));
        free(igen);
        free(ibag);
        free(inst);
        free(pgen);
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    uint32_t num_sample_headers = subchunk.size / sizeof(ise_dsp_sf2_sample_t);
    ise_dsp_sf2_sample_t* shdr = (ise_dsp_sf2_sample_t*) malloc(subchunk.size);
    if(shdr == NULL) {
        printf("Could not allocate shdr\n");
        free(igen);
        free(ibag);
        free(inst);
        free(pgen);
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    read_size = fread(shdr, sizeof(ise_dsp_sf2_sample_t), num_sample_headers, file);
    if(read_size != num_sample_headers) {
        printf("Incorrect sample headers read: %d expected: %d\n", read_size, num_sample_headers);
        free(shdr);
        free(igen);
        free(ibag);
        free(inst);
        free(pgen);
        free(pbag);
        free(phdr);
        fclose(file);
        return;
    }
    printf("SF2 file succesfully parsed!\n");
    
    //printf("Num_presets=%d\n", num_preset_headers);
    int pi, zi, num_zones, gi, num_gens;
    int pii, zii, num_izones, gii, num_igens;  // for instruments
    int si;
    ise_dsp_sf2_preset_t* preset_node;
    ise_dsp_sf2_preset_t* next_preset = (ise_dsp_sf2_preset_t*) malloc(sizeof(ise_dsp_sf2_preset_t));
    memset(next_preset, 0, sizeof(ise_dsp_sf2_preset_t));
    for(pi=0; pi<num_preset_headers; pi++) {
        num_zones = phdr[pi+1].preset_bag_ndx - phdr[pi].preset_bag_ndx;
        //printf("Index %d, preset name %s preset id %d preset zone %d num zones %d\n", pi,
        //    phdr[pi].preset_name, phdr[pi].preset, phdr[pi].preset_bag_ndx, num_zones);
        for(zi=phdr[pi].preset_bag_ndx; zi<phdr[pi].preset_bag_ndx+num_zones; zi++) {
            num_gens = pbag[zi+1].gen_ndx - pbag[zi].gen_ndx;
            //printf("  Zindex %d first gen %d num gens %d\n", zi, pbag[zi].gen_ndx, num_gens);
            for(gi=pbag[zi].gen_ndx; gi<pbag[zi].gen_ndx+num_gens; gi++) {
                //printf("    Gindex %d op %d amount %d(0x%x)\n", gi, pgen[gi].gen_op, pgen[gi].gen_amount.u16, pgen[gi].gen_amount.u16);
                ise_dsp_process_sf2_gen(next_preset, &pgen[gi]);
                if(pgen[gi].gen_op == ISE_DSP_SF2_GEN_KEY_RANGE) {
                    if(pgen[gi].gen_amount.range.lo != 0x0 || pgen[gi].gen_amount.range.hi != 0x7f) {
                        printf("Unsupported pgen range 0x%x 0x%x\n", pgen[gi].gen_amount.range.lo, pgen[gi].gen_amount.range.hi);
                    }
                    next_preset->key_lo = 0;
                    next_preset->key_hi = 0;
                }
                if(pgen[gi].gen_op == ISE_DSP_SF2_GEN_INSTRUMENT) {
                    // instrument - should be last
                    pii = pgen[gi].gen_amount.u16;
                    num_izones = inst[pii+1].inst_bag_ndx - inst[pii].inst_bag_ndx;
                    //printf("  Iindex %d inst name %s inst zone %d num zones %d\n",
                    //    pii, inst[pii].inst_name, inst[pii].inst_bag_ndx, num_izones);
                    for(zii = inst[pii].inst_bag_ndx; zii <inst[pii].inst_bag_ndx+num_izones; zii++) {
                        num_igens = ibag[zii+1].gen_ndx - ibag[zii].gen_ndx;
                        //if(phdr[pi].bank == 128) printf("    ZIindex %d igen %d num igens %d\n", zii, ibag[zii].gen_ndx, num_igens);
                        for(gii=ibag[zii].gen_ndx; gii<ibag[zii].gen_ndx+num_igens; gii++) {
                            //if(phdr[pi].bank == 128) printf("      GIindex %d op %d amount %d(0x%x)\n", gii, igen[gii].gen_op, igen[gii].gen_amount.u16, igen[gii].gen_amount.u16);
                            ise_dsp_process_sf2_gen(next_preset, &igen[gii]);
                            if(igen[gii].gen_op == ISE_DSP_SF2_GEN_SAMPLE_ID) {
                                si = igen[gii].gen_amount.u16;
                                //if(phdr[pi].bank == 128) printf("      Sindex %d name %s start %d end %d stloop %d endloop %d rate %d pitch %d corr %d type %d link %d\n",
                                //    si, shdr[si].sample_name, shdr[si].start, shdr[si].end, shdr[si].start_loop, shdr[si].end_loop,
                                //    shdr[si].sample_rate, shdr[si].orig_pitch, shdr[si].pitch_correction, shdr[si].sample_type, shdr[si].sample_link);
                                ise_dsp_process_sf2_shdr(next_preset, &shdr[si]);
                                // loop through presets, and look for a match
                                bool match_found = false;
                                bool already_exists = false;
                                int preset = (phdr[pi].bank == 128) ? 128 : phdr[pi].preset;
                                for(preset_node = sf2->presets[preset]; preset_node != NULL && !match_found; preset_node = preset_node->next) {
                                    if(ise_dsp_sf2_presets_match(next_preset, preset_node)) {
                                        preset_node->key_hi = next_preset->key_hi;
                                        match_found = true;
                                    }
                                    if(next_preset->key_lo == preset_node->key_lo && next_preset->key_hi == preset_node->key_hi) {
                                        already_exists = true;
                                    }
                                }
                                if(!already_exists) {
                                    if(!match_found) {
                                        next_preset->next = sf2->presets[preset];
                                        sf2->presets[preset] = next_preset;
                                        next_preset = (ise_dsp_sf2_preset_t*) malloc(sizeof(ise_dsp_sf2_preset_t));
                                    //} else {
                                    //    printf("Match found\n");
                                    }
                                //} else {
                                //    printf("Already exists\n");
                                }
                                memset(next_preset, 0, sizeof(ise_dsp_sf2_preset_t));
                            }
                        }
                    }
                }
            }
        }
    }
    free(next_preset);

    free(shdr);
    free(igen);
    free(ibag);
    free(inst);
    free(pgen);
    free(pbag);
    free(phdr);
    fclose(file);
}

// --------------------------------------------------------------------
// Sound blaster specific functions
void ise_dsp_sb_write(uint8_t data)
{
    int i;
    for(i=0; i<256; i++) {
        if((inp(ise_dsp_base+0xC) & 0x80) == 0) break;
    }
    outp(ise_dsp_base +0xC, data);
}

int ise_dsp_sb_read()
{
    int i;
    for(i=0; i<256; i++) {
        if(inp(ise_dsp_base+0xE) & 0x80) break;
    }
    return inp(ise_dsp_base+0xA);
}

void _interrupt FAR ise_dsp_sb_isr()
{
    outp(ise_dsp_base+0x4, ISE_DSP_MIXER_INTERRUPT_STATUS);
    int status = inp(ise_dsp_base+0x5);
    if(status & 0x3) {
        if(ise_dsp_pcm_load_tick == 0) {
            // if 0, indicates to stop next interrupt
            ise_dsp_pcm_play_tick = 0;
            ise_dsp_sb_write(ISE_DSP_CMD_PAUSE_8B_DMA);
            if((ise_dsp_version & 0xFF00) >= 0x0400) ise_dsp_sb_write(ISE_DSP_CMD_PAUSE_16B_DMA);
        } else {
            ise_dsp_pcm_play_tick++;
        }
        if(status & 0x1) inp(ise_dsp_base+0xE);
        if(status & 0x2) inp(ise_dsp_base+0xF);
    }

    if(ise_dsp_irq >= 8) outp(ISE_IRQ_PIC1_COMMAND, 0x20);
    outp(ISE_IRQ_PIC0_COMMAND, 0x20);
    //_chain_intr(prev_dsp_isr);
}

// reset sound card; returns true if found
bool ise_dsp_sb_reset()
{
    int i;
    outp(ise_dsp_base+0x6, 0x1);
    ise_time_uwait(3);
    outp(ise_dsp_base+0x6, 0x0);
    for(i=0; i<256; i++) {
        if(inp(ise_dsp_base+0xE) & 0x80) break;
    }
    if(i == 256) return false; // DSP not found
    for(i=0; i<256; i++) {
        if(inp(ise_dsp_base+0xA) == 0xAA) break;
    }
    if(i == 256) return false; // DSP not found

    // get version
    ise_dsp_sb_write(ISE_DSP_CMD_GET_VERSION);
    ise_dsp_version = (uint16_t) ise_dsp_sb_read() << 8;
    ise_dsp_version |= (uint16_t) ise_dsp_sb_read();

    // for version 4.xx, get the irq/dma
    if((ise_dsp_version & 0xFF00) >= 0x0400) {
        int status;
        outp(ise_dsp_base+0x4, ISE_DSP_MIXER_INTERRUPT_SETUP);
        status = inp(ise_dsp_base+0x5);
        if(status & 0x1) ise_dsp_irq = 2;
        else if(status & 0x2) ise_dsp_irq = 5;
        else if(status & 0x4) ise_dsp_irq = 7;
        else if(status & 0x8) ise_dsp_irq = 10;
        outp(ise_dsp_base+0x4, ISE_DSP_MIXER_DMA_SETUP);
        status = inp(ise_dsp_base+0x5);
        if(status & 0x1) ise_dsp_dma8_channel = 0;
        else if(status & 0x2) ise_dsp_dma8_channel = 1;
        else if(status & 0x8) ise_dsp_dma8_channel = 3;
        if(status & 0x20) ise_dsp_dma16_channel = 5;
        else if(status & 0x40) ise_dsp_dma16_channel = 6;
        else if(status & 0x80) ise_dsp_dma16_channel = 7;
        else ise_dsp_dma16_channel = ise_dsp_dma8_channel;
    }
    printf("DSP version 0x%x irq 0x%x dma %d/%d\n", ise_dsp_version, ise_dsp_irq, ise_dsp_dma8_channel, ise_dsp_dma16_channel);
    return true;
}

void ise_dsp_sb_play(ise_dsp_pcm_t* pcm)
{
    // stop sound
    ise_dsp_sb_write(ISE_DSP_CMD_PAUSE_8B_DMA);
    if((ise_dsp_version & 0xFF00) >= 0x0400) ise_dsp_sb_write(ISE_DSP_CMD_PAUSE_16B_DMA);

    // setup dma
    int dma_channel = (pcm->bytes_per_sample > 1) ? ise_dsp_dma16_channel : ise_dsp_dma8_channel;
    int length = ISE_DSP_MEM_BUFFER_SIZE;
    ise_dma_setup(ISE_DMA_MODE_SINGLE | ISE_DMA_MODE_ADDR_INC | ISE_DMA_MODE_AUTO_INIT | ISE_DMA_MODE_XFER_READ, dma_channel, ise_dsp_mem_buffer, (uint16_t) length);

    int cmd = 0;
    int mode = 0;
    length = ISE_DSP_MEM_BUFFER_SIZE / 2;
    if(pcm->bytes_per_sample > 1) {
        cmd = ISE_DSP_CMD_DMA_16B_OUT_AI;
        mode |= ISE_DSP_CMD_MODE_SIGNED_FLAG;
        length = length / 2;
    } else {
        cmd = ISE_DSP_CMD_DMA_8B_OUT_AI;
    }
    if(pcm->num_channels > 1) {
        mode |= ISE_DSP_CMD_MODE_STEREO_FLAG;
    }
    length--;

    // setup DSP
    // enable speaker
    ise_dsp_sb_write(ISE_DSP_CMD_SPEAKER_ENABLE);
    // sample rate
    if((ise_dsp_version & 0xFF00) >= 0x0400) {
        ise_dsp_sb_write(ISE_DSP_CMD_OUT_SAMPLE_RATE);
        ise_dsp_sb_write((uint8_t) (pcm->samples_per_second >> 8) & 0xFF);
        ise_dsp_sb_write((uint8_t) pcm->samples_per_second & 0xFF);
        // setup DMA
        ise_dsp_sb_write(cmd);
        ise_dsp_sb_write(mode);
        ise_dsp_sb_write(length & 0xFF);
        ise_dsp_sb_write((length >> 8) & 0xFF);
    } else {
        int time_const = (65536 - 256000000/(pcm->num_channels*pcm->samples_per_second));
        ise_dsp_sb_write(ISE_DSP_CMD_TIME_CONSTANT);
        ise_dsp_sb_write((uint8_t) ((time_const >> 8) & 0xFF));
        // setup DMA
        ise_dsp_sb_write(ISE_DSP_CMD_BLOCK_TRANSFER_SIZE);
        ise_dsp_sb_write(length & 0xFF);
        ise_dsp_sb_write((length >> 8) & 0xFF);
        ise_dsp_sb_write(ISE_DSP_CMD_DMA_8B_OUT_AI_LEGACY);
    }
}

void ise_dsp_sb_pause()
{
    // stop sound
    ise_dsp_sb_write(ISE_DSP_CMD_PAUSE_8B_DMA);
    if((ise_dsp_version & 0xFF00) >= 0x0400) ise_dsp_sb_write(ISE_DSP_CMD_PAUSE_16B_DMA);
}

void ise_dsp_sb_stop_loop()
{
    // prevent further loading of data
    ise_dsp_pcm_load_tick = 0;
    //ise_dsp_sb_write(ISE_DSP_CMD_PAUSE_8B_DMA);
    //if((ise_dsp_version & 0xFF00) >= 0x0400) ise_dsp_sb_write(ISE_DSP_CMD_PAUSE_16B_DMA);
    ise_dsp_sb_write(ISE_DSP_CMD_EXIT_8B_AI_DMA);
    if((ise_dsp_version & 0xFF00) >= 0x0400) ise_dsp_sb_write(ISE_DSP_CMD_EXIT_16B_AI_DMA);
}

void ise_dsp_init_sb()
{
    ise_dsp_reset = ise_dsp_sb_reset;
    ise_dsp_play = ise_dsp_sb_play;
    ise_dsp_pause = ise_dsp_sb_pause;
    ise_dsp_stop_loop = ise_dsp_sb_stop_loop;

    // install ISR
    ise_dsp_sb_reset();
    prev_dsp_isr = _dos_getvect(ISE_IRQ_INTERRUPT_VECTOR(ise_dsp_irq));
    _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(ise_dsp_irq), ise_dsp_sb_isr);
    
}

// --------------------------------------------------------------------
// C-media 8738 functions
void _interrupt FAR ise_dsp_cmi8738_isr()
{
    uint32_t int_mask, data;
    // check if either channel is causing an interrupt
    int_mask = inpd(ise_dsp_base+ISE_DSP_CMI8738_INT_STATUS);
    if(int_mask & 0x3) {
        if(ise_dsp_pcm_load_tick == 0) {
            // if 0, indicates to stop next interrupt
            ise_dsp_pcm_play_tick = 0;
            data = inpd(ise_dsp_base+ISE_DSP_CMI8738_FUNC_CTRL_0);
            outpd(ise_dsp_base+ISE_DSP_CMI8738_FUNC_CTRL_0, data | 0xC);
        } else {
            ise_dsp_pcm_play_tick++;
        }
    }
    // clear interrupts
    data = inpd(ise_dsp_base+ISE_DSP_CMI8738_INT_HOLD_CLR);
    outpd(ise_dsp_base+ISE_DSP_CMI8738_INT_HOLD_CLR, data & ~(int_mask << 16));
    outpd(ise_dsp_base+ISE_DSP_CMI8738_INT_HOLD_CLR, data);

    if(ise_dsp_irq >= 8) outp(ISE_IRQ_PIC1_COMMAND, 0x20);
    outp(ISE_IRQ_PIC0_COMMAND, 0x20);
    //_chain_intr(prev_dsp_isr);
}

bool ise_dsp_cmi8738_reset()
{
    // disable interrupts
    outpd(ise_dsp_base+ISE_DSP_CMI8738_INT_HOLD_CLR, 0x0);
    
    // reset both channels, and set to output
    outpd(ise_dsp_base+ISE_DSP_CMI8738_FUNC_CTRL_0, 0xC0000);
    ise_time_wait(100);  // wait 100 ms
    outpd(ise_dsp_base+ISE_DSP_CMI8738_FUNC_CTRL_0, 0x0);
    // enable bus mastering, and legacy FM
    outpd(ise_dsp_base+ISE_DSP_CMI8738_FUNC_CTRL_1, 0x10);
    outpd(ise_dsp_base+ISE_DSP_CMI8738_CH_FORMAT, 0x0);
    // enable FM synth
    uint32_t data = inpd(ise_dsp_base+ISE_DSP_CMI8738_MISC_CTRL) | 0x80000;
    outpd(ise_dsp_base+ISE_DSP_CMI8738_MISC_CTRL, data);
    // Make sure FM synth and wave stream are unmuted
    data = inp(ise_dsp_base+ISE_DSP_CMI8738_MIXER_CFG0) & ~0xC0;
    outp(ise_dsp_base+ISE_DSP_CMI8738_MIXER_CFG0, data);
    
    // enable interrupts on channel 0
    outpd(ise_dsp_base+ISE_DSP_CMI8738_INT_HOLD_CLR, 0x10000);

    return true;
}

void ise_dsp_cmi8738_play(ise_dsp_pcm_t* pcm)
{
    uint32_t data;

    // disable all channels
    outpd(ise_dsp_base+ISE_DSP_CMI8738_FUNC_CTRL_0, 0x0);
    
    // setup channel 0 and count registers
    int length = ISE_DSP_MEM_BUFFER_SIZE;
    if(pcm->bytes_per_sample > 1) {
        data = inpd(ise_dsp_base+ISE_DSP_CMI8738_CH_FORMAT);
        // set ch0 to 16b mono (for now)
        outpd(ise_dsp_base+ISE_DSP_CMI8738_CH_FORMAT, (data & ~0x3) | 0x2);
        length = length / 2;
    } else {
        // set ch0 to 8b mono
        data = inpd(ise_dsp_base+ISE_DSP_CMI8738_CH_FORMAT);
        outpd(ise_dsp_base+ISE_DSP_CMI8738_CH_FORMAT, (data & ~0x3));
    }
    if(pcm->num_channels > 1) {
        // set ch0 to stereo
        data = inpd(ise_dsp_base+ISE_DSP_CMI8738_CH_FORMAT);
        outpd(ise_dsp_base+ISE_DSP_CMI8738_CH_FORMAT, data | 0x1);
        length = length / 2;
    }
    length--;    

    int period_length = length / 2;

    outpd(ise_dsp_base+ISE_DSP_CMI8738_CH0_FRAME_REG0, (uint32_t) ise_dsp_mem_buffer);
    data = (period_length << 16) | (length & 0xFFFF);
    outpd(ise_dsp_base+ISE_DSP_CMI8738_CH0_FRAME_REG1, data);

    // sample rate
    data = inpd(ise_dsp_base+ISE_DSP_CMI8738_FUNC_CTRL_1) & ~0x1C00;
    if(pcm->samples_per_second <= 5512) data |= 0x0 << 10;
    else if(pcm->samples_per_second <= 8000) data |= 0x4 << 10;
    else if(pcm->samples_per_second <= 11025) data |= 0x1 << 10;
    else if(pcm->samples_per_second <= 16000) data |= 0x5 << 10;
    else if(pcm->samples_per_second <= 22050) data |= 0x2 << 10;
    else if(pcm->samples_per_second <= 32000) data |= 0x6 << 10;
    else if(pcm->samples_per_second <= 44100) data |= 0x3 << 10;
    else data |= 0x7 << 10;
    outpd(ise_dsp_base+ISE_DSP_CMI8738_FUNC_CTRL_1, data);

    // enable channel 0
    outpd(ise_dsp_base+ISE_DSP_CMI8738_FUNC_CTRL_0,0x10000);
}

void ise_dsp_cmi8738_pause()
{
    // stop sound
    uint32_t data = inpd(ise_dsp_base+ISE_DSP_CMI8738_FUNC_CTRL_0);
    outpd(ise_dsp_base+ISE_DSP_CMI8738_FUNC_CTRL_0, data | 0xC);
}

void ise_dsp_cmi8738_stop_loop()
{
    ise_dsp_pcm_load_tick = 0;
}

void ise_dsp_init_cmi8738(int slot, int function)
{
    uint32_t data;
    
    printf("Found Cmedia 8738 chipset\n");
    ise_dsp_base = ise_pci_read_config(0, slot, function, 0x10) & ~0x1L;
    ise_dsp_irq = (uint8_t) ise_pci_read_config(0, slot, function, 0x3C) & 0xF;
    // Get sound blaster base address
    //data = inpd(ise_dsp_base+0x14);
    //ise_opl_base = 0x220 + 0x20 * ((data >> 26) & 0x3);
    ise_opl_base = ise_dsp_base + 0x50;
    // enable legacy devices (SB16 & FM)
    //data = inpd(ise_dsp_base+0x04) | 0x8;
    //outpd(ise_dsp_base+0x04, data);
    // enable FM synth
    data = inpd(ise_dsp_base+0x18) | 0x80000;
    outpd(ise_dsp_base+0x18, data);
    // Make sure FM synth and wave stream are unmuted
    data = inp(ise_dsp_base+0x24) & ~0xC0;
    outp(ise_dsp_base+0x24, data);

    ise_dsp_reset = ise_dsp_cmi8738_reset;
    ise_dsp_play = ise_dsp_cmi8738_play;
    ise_dsp_pause = ise_dsp_cmi8738_pause;
    ise_dsp_stop_loop = ise_dsp_cmi8738_stop_loop;

    // install ISR
    prev_dsp_isr = _dos_getvect(ISE_IRQ_INTERRUPT_VECTOR(ise_dsp_irq));
    _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(ise_dsp_irq), ise_dsp_cmi8738_isr);
}

// --------------------------------------------------------------------
// Generic control functions
void ise_dsp_install()
{
    // Check base address - need to install opl first
    if(ise_dsp_base == 0) {
        // Opl base cannot be 0
        return;
    }
    
    // Reset DSP and check for sound card
    if(!ise_dsp_reset()) return;
    printf("DSP detected\n");
    
    int i;
    uint32_t dma_mem[3];
    // Loop to allocate a memory buffer that does not straddle a 64KB page
    for(i=0; i<3; i++) {
        dma_mem[i] = (uint32_t) ise_mem_aligned_malloc(ISE_DSP_MEM_BUFFER_SIZE, 4, ISE_MEM_REGION_DOS);
        // check if buffer straddles 64KB page
        if((dma_mem[i] & ~0xFFFF) == ((dma_mem[i] + ISE_DSP_MEM_BUFFER_SIZE) & ~0xFFFF)) {
            // if not, we're done
            break;
        }
    }
    if(i<3) {
        printf("DSP memory buffer allocated (iteration %d): 0x%x\n", i, dma_mem[i]);
        ise_dsp_mem_buffer = (uint8_t*) dma_mem[i];
    }
    // Free up anything that's unused
    for(i=i-1; i>=0; i--) {
        ise_mem_aligned_free((void*) dma_mem[i]);
    }
    if(ise_dsp_mem_buffer == NULL) return;
    
    int pic = (ise_dsp_irq >= 8) ? ISE_IRQ_PIC1_DATA : ISE_IRQ_PIC0_DATA;
    int int_mask = inp(pic);
    // check if irq is masked
    if(int_mask & (1 << (ise_dsp_irq & 0x7))) {
        ise_dsp_irq_orig_mask = true;
        int_mask &= ~(1 << (ise_dsp_irq & 0x7));
        outp(pic, int_mask);
    }

    ise_dsp_load_sf2_file("gm.sf2", &ise_dsp_sf2);
    
}

void ise_dsp_play_track(ise_midi_event_t* track)
{
    ise_dsp_pause();

    ise_dsp_midi_track = track;
    ise_dsp_midi_event = track;
    ise_dsp_midi_delta_tick = 0;
    ise_dsp_cur_sample_in_midi_tick = 0;
    int v;
    for(v=0; v<ISE_DSP_NUM_VOICES; v++) {
        ise_dsp_vox[v].cur_vol_env = ISE_DSP_VOL_ENV_OFF;
        ise_dsp_vox[v].base_preset = NULL;
        ise_dsp_vox[v].preset = NULL;
        ise_dsp_vox[v].max_volume = 0;
    }

    ise_dsp_pcm_t pcm;
    pcm.bytes_per_sample = 2;
    pcm.num_channels = 1;
    pcm.samples_per_second = 44100;
    pcm.size = 0;
    pcm.mem = NULL;

    ise_dsp_tick();

    ise_dsp_play(&pcm);
}

int32_t ise_dsp_sample_pcm()
{
    if(ise_dsp_pcm_length == 0) return 0;
    int32_t sample = *((int16_t*) ise_dsp_pcm_data);
    ise_dsp_pcm_data += 2;
    ise_dsp_pcm_length -= 2;
    return sample;

    /*uint8_t subsample_inc = ( (ise_dsp_pcm->samples_per_second < 7500) ? 1 :
                            ( (ise_dsp_pcm->samples_per_second < 15000) ? 2 :
                            ( (ise_dsp_pcm->samples_per_second < 30000) ? 4 : 0)));
    int32_t sample;
    int length;
    if(ise_dsp_pcm->bytes_per_sample == 2) {
        int16_t* mem16 = (int16_t*) ise_dsp_pcm_data;
        sample = *mem16;
        if(ise_dsp_pcm->num_channels == 2) {
            mem16++;
            sample += *mem16;
            sample >>= 1;
            length = 4;
        } else {
            length = 2;
        }
        if(ise_dsp_pcm_subsample) {
            mem16++;
            int32_t s1 = (*mem16);
            if(ise_dsp_pcm->num_channels == 2) {
                mem16++;
                s1 += (*mem16);
                s1 >>= 1;
            }
            sample *= (8-ise_dsp_pcm_subsample);
            sample += ise_dsp_pcm_subsample * s1;
            sample >>= 3;
        }
    } else {
        uint8_t* mem8 = ise_dsp_pcm_data;
        int d8 = *mem8;
        sample = ((d8 << 8) | d8) - 0x8000;
        if(ise_dsp_pcm->num_channels == 2) {
            mem8++;
            d8 = *(mem8);
            sample += ((d8 << 8) | d8) - 0x8000;
            sample >>= 1;
            length = 2;
        } else {
            length = 1;
        }
        if(ise_dsp_pcm_subsample) {
            mem8++;
            d8 = (*mem8);
            int32_t s1 = ((d8 << 8) | d8) - 0x8000;
            if(ise_dsp_pcm->num_channels == 2) {
                mem8++;
                d8 = (*mem8);
                s1 += ((d8 << 8) | d8) - 0x8000;;
                s1 >>= 1;
            }
            sample *= (8-ise_dsp_pcm_subsample);
            sample += ise_dsp_pcm_subsample * s1;
            sample >>= 3;
        }
    }
    ise_dsp_pcm_subsample += subsample_inc;
    ise_dsp_pcm_subsample &= 0x7;
    if(ise_dsp_pcm_subsample == 0) {
        ise_dsp_pcm_data += length;
        ise_dsp_pcm_length -= length;
    }

    return sample;
    */
}

void ise_dsp_play_pcm(ise_dsp_pcm_t* pcm)
{
    if(ise_dsp_pcm == pcm && (ise_dsp_pcm_length != 0) && (pcm->size - ise_dsp_pcm_length) < ISE_DSP_MEM_BUFFER_SIZE) return;
    ise_dsp_pcm = pcm;
    ise_dsp_pcm_data = pcm->mem;
    ise_dsp_pcm_length = pcm->size;
    ise_dsp_pcm_subsample = 0;
    int length = pcm->size;
    if(length > ISE_DSP_MEM_BUFFER_SIZE/2) length = ISE_DSP_MEM_BUFFER_SIZE/2;

    int16_t* mem16;
    int s;

    if(ise_dsp_midi_event == NULL) {
        ise_dsp_pause();
        ise_dsp_pcm_load_tick = (length > ISE_DSP_MEM_BUFFER_SIZE/2) ? 2 : 1;
        ise_dsp_pcm_play_tick = 0;
        mem16 = (int16_t*) ise_dsp_mem_buffer;

        for(s=0; s<length/2; s++) {
            *mem16 = ise_dsp_sample_pcm();
            mem16++;
        }
        ise_dsp_pcm_t pcm;
        pcm.bytes_per_sample = 2;
        pcm.num_channels = 1;
        pcm.samples_per_second = 44100;
        pcm.size = 0;
        pcm.mem = NULL;

        ise_dsp_play(&pcm);
        if(ise_dsp_pcm_length==0) ise_dsp_stop_loop();
    } else {
        mem16 = (int16_t*) ((ise_dsp_pcm_play_tick & 1) ? ise_dsp_mem_buffer : ise_dsp_mem_buffer + ISE_DSP_MEM_BUFFER_SIZE/2);
        for(s=0; s<length/2; s++) {
            *mem16 += ise_dsp_sample_pcm();
            mem16++;
        }
    }
}

void ise_dsp_stop_pcm()
{
	ise_dsp_pcm_length = 0;
}

int32_t ise_dsp_sample_vox(int v)
{
    // determine which part of volume envelpoe we are in
    if(ise_dsp_vox[v].cur_vol_env == ISE_DSP_VOL_ENV_DELAY && ise_dsp_vox[v].cur_sample_vol_env >= ise_dsp_vox[v].preset->delay_samples) {
        ise_dsp_vox[v].cur_vol_env = ISE_DSP_VOL_ENV_ATTACK;
        ise_dsp_vox[v].cur_sample_vol_env = 0;
        ise_dsp_vox[v].volume = ise_dsp_vox[v].attack_volume_inc;
    }
    if(ise_dsp_vox[v].cur_vol_env == ISE_DSP_VOL_ENV_ATTACK && ise_dsp_vox[v].cur_sample_vol_env >= ise_dsp_vox[v].preset->attack_samples) {
        ise_dsp_vox[v].cur_vol_env = ISE_DSP_VOL_ENV_HOLD;
        ise_dsp_vox[v].cur_sample_vol_env = 0;
        ise_dsp_vox[v].volume = ise_dsp_vox[v].max_volume;
    }
    if(ise_dsp_vox[v].cur_vol_env == ISE_DSP_VOL_ENV_HOLD && ise_dsp_vox[v].cur_sample_vol_env >= ise_dsp_vox[v].preset->hold_samples) {
        ise_dsp_vox[v].cur_vol_env = ISE_DSP_VOL_ENV_DECAY;
        ise_dsp_vox[v].cur_sample_vol_env = 0;
        ise_dsp_vox[v].volume = ise_dsp_vox[v].max_volume - ise_dsp_vox[v].decay_volume_dec;
    }
    if(ise_dsp_vox[v].cur_vol_env == ISE_DSP_VOL_ENV_DECAY && ise_dsp_vox[v].cur_sample_vol_env >= ise_dsp_vox[v].preset->decay_samples) {
        ise_dsp_vox[v].cur_vol_env = ISE_DSP_VOL_ENV_SUSTAIN;
        ise_dsp_vox[v].cur_sample_vol_env = 0;
        ise_dsp_vox[v].volume = ise_dsp_vox[v].sustain_volume;
    }
    if(ise_dsp_vox[v].cur_vol_env == ISE_DSP_VOL_ENV_RELEASE) {
        if(ise_dsp_vox[v].cur_sample_vol_env == 0) ise_dsp_vox[v].volume = ise_dsp_vox[v].sustain_volume - ise_dsp_vox[v].release_volume_dec;
        if(ise_dsp_vox[v].cur_sample_vol_env >= ise_dsp_vox[v].preset->release_samples) {
            ise_dsp_vox[v].cur_vol_env = ISE_DSP_VOL_ENV_OFF;
            ise_dsp_vox[v].volume = 0;
        }
    }

    int32_t sample;
    sample = ise_dsp_sf2.sample_data[ise_dsp_vox[v].cur_index] * (0x10000 - ise_dsp_vox[v].cur_fract);
    sample += ise_dsp_sf2.sample_data[ise_dsp_vox[v].cur_index+1] * ise_dsp_vox[v].cur_fract;
    sample >>= 16;
    sample *= (ise_dsp_vox[v].volume >> 16);
    ise_dsp_vox[v].cur_fract += ise_dsp_vox[v].sample_inc;
    ise_dsp_vox[v].cur_index += ise_dsp_vox[v].cur_fract >> 16;
    ise_dsp_vox[v].cur_fract &= 0xFFFF;
    if(ise_dsp_vox[v].cur_index >= ise_dsp_vox[v].preset->end_sample) {
        ise_dsp_vox[v].cur_index = ise_dsp_vox[v].preset->start_sample;
        ise_dsp_vox[v].cur_vol_env = ISE_DSP_VOL_ENV_OFF;
    }
    if(ise_dsp_vox[v].cur_index >= ise_dsp_vox[v].preset->end_loop_sample) {
        ise_dsp_vox[v].cur_index = ise_dsp_vox[v].preset->start_loop_sample + (ise_dsp_vox[v].cur_index - ise_dsp_vox[v].preset->end_loop_sample);
    }
    
    ise_dsp_vox[v].cur_sample_vol_env++;
    switch(ise_dsp_vox[v].cur_vol_env) {
    case ISE_DSP_VOL_ENV_ATTACK:  ise_dsp_vox[v].volume += ise_dsp_vox[v].attack_volume_inc; break;
    case ISE_DSP_VOL_ENV_DECAY:   ise_dsp_vox[v].volume -= ise_dsp_vox[v].decay_volume_dec; break;
    case ISE_DSP_VOL_ENV_RELEASE: ise_dsp_vox[v].volume -= ise_dsp_vox[v].release_volume_dec; break;
    }
    return sample >> 16;

}

void ise_dsp_tick()
{
    //if(ise_dsp_midi_event == NULL && ise_dsp_pcm_length == 0) return;

    // make sure the loaded data is 2 ahead of what's being played, to fill upo the buffer
    while((ise_dsp_midi_event != NULL || ise_dsp_pcm_length != 0) && ((int32_t) (ise_dsp_pcm_load_tick - ise_dsp_pcm_play_tick) < 2)) {
        //printf("l: %d p: %d\n", ise_dsp_pcm_load_tick, ise_dsp_pcm_play_tick);
        uint8_t* mem = (ise_dsp_pcm_load_tick & 1) ? ise_dsp_mem_buffer + (ISE_DSP_MEM_BUFFER_SIZE/2) : ise_dsp_mem_buffer;
        uint16_t* mem16 = (uint16_t*) mem;
        int length = ISE_DSP_MEM_BUFFER_SIZE/4;  // in samples
        int s, v;
        ise_midi_event_t* cur_event;
        uint32_t event_delta_tick;
        bool midi_done;
        // loop through all samples that we need load
        for(s=0; s<length; s++) {
            // First process any midi event
            midi_done = (ise_dsp_midi_event) ? false : true;
            while(!midi_done) {
                cur_event = ise_dsp_midi_event;
                event_delta_tick = ise_midi_get_delta_time(&cur_event);
                if(ise_dsp_midi_delta_tick >= event_delta_tick) {
                    ise_dsp_midi_delta_tick -= event_delta_tick;
                    ise_dsp_midi_event = cur_event;
                    // Global events
                    switch(cur_event->f.command) {
                    case ISE_MIDI_META_EVENT_TYPE_END_TRACK:
                        ise_dsp_midi_event = NULL;//ise_opl_track;
                        break;
                    case ISE_MIDI_META_EVENT_TYPE_SET_TEMPO:
                        ise_dsp_samples_per_midi_tick = ((cur_event->data & 0xFFFF) * 38755) >> 12;
                        break;
                    }
                    uint8_t channel = cur_event->f.command & 0xF;
                    uint8_t note = cur_event->f.data0;
                    uint8_t velocity = cur_event->f.data1+1;
                    uint8_t octave;
                    uint16_t delta_key;
                    // Channel events
                    switch(cur_event->f.command & 0xF0) {
                    case ISE_MIDI_MSG_NOTE_OFF:
                        if(channel != 15) {
                            // non-percussive
                            if(ise_dsp_vox[channel].cur_vol_env != ISE_DSP_VOL_ENV_OFF) {
                                ise_dsp_vox[channel].cur_vol_env = ISE_DSP_VOL_ENV_RELEASE;
                                ise_dsp_vox[channel].cur_sample_vol_env = 0;
                            }
                        } else {
                            bool found = false;
                            for(channel=0; !found && channel<ISE_DSP_NUM_PERCUSSIVE_VOICES; channel++) {
                                if(ise_dsp_percussive_note_mapping[channel] == note) {
                                    found = true;
                                    break;
                                }
                            }
                            if(found) {
                                ise_dsp_percussive_note_mapping[channel] = 0;
                                channel += ISE_DSP_NUM_MELODIC_VOICES;
                                //printf("note off: %d ch %d\n", note, channel);
                                if(ise_dsp_vox[channel].cur_vol_env != ISE_DSP_VOL_ENV_OFF) {
                                    ise_dsp_vox[channel].cur_vol_env = ISE_DSP_VOL_ENV_RELEASE;
                                    ise_dsp_vox[channel].cur_sample_vol_env = 0;
                                }
                            }
                        }
                        break;
                    case ISE_MIDI_MSG_NOTE_ON:
                        if(channel != 15) {
                            // non-percussive
                            // find a preset for the note being played
                            for(ise_dsp_vox[channel].preset = ise_dsp_vox[channel].base_preset; ise_dsp_vox[channel].preset != NULL; ise_dsp_vox[channel].preset = ise_dsp_vox[channel].preset->next) {
                                if(note >= ise_dsp_vox[channel].preset->key_lo && note <= ise_dsp_vox[channel].preset->key_hi) break;
                            }
                            // make sure a preset is assigned to the voice
                            if(ise_dsp_vox[channel].preset) {
                                int pitch_correction = ise_dsp_vox[channel].preset->pitch_correction;
                                delta_key = 8*12;  // base frequencies are pre-multiplied by 8 octaves
                                delta_key -= (note - ise_dsp_vox[channel].preset->key_pitch);
                                if(pitch_correction > 0) {
                                    delta_key--;
                                    pitch_correction = 100 - pitch_correction;
                                } else {
                                    pitch_correction = -pitch_correction;
                                }
                                pitch_correction /= 4;
                                octave = delta_key / 12;
                                delta_key = delta_key % 12;
                                delta_key *= 25;
                                delta_key += pitch_correction;
                                
                                if(ise_dsp_vox[channel].preset->sample_rate < 30000) octave++;  // should be 22050, or less
                                if(ise_dsp_vox[channel].preset->sample_rate < 15000) octave++;  // should be 11025, or less
                                if(ise_dsp_vox[channel].preset->sample_rate < 7500) octave++;   // should be 5512
                                ise_dsp_vox[channel].cur_vol_env = ISE_DSP_VOL_ENV_DELAY;
                                ise_dsp_vox[channel].cur_sample_vol_env = 0;
                                ise_dsp_vox[channel].cur_index = ise_dsp_vox[channel].preset->start_sample;
                                ise_dsp_vox[channel].cur_fract = 0;
                                ise_dsp_vox[channel].sample_inc = ise_dsp_freq_table[delta_key] >> octave;
                                ise_dsp_vox[channel].volume = 0;
                                ise_dsp_vox[channel].max_volume = (velocity << 23) | (velocity << 16) | (velocity << 9) | (velocity << 2);
                                ise_dsp_vox[channel].sustain_volume = (ise_dsp_vox[channel].max_volume >> 16) * ise_dsp_vox[channel].preset->sustain_level;
                                ise_dsp_vox[channel].sustain_volume = (ise_dsp_vox[channel].sustain_volume < ise_dsp_vox[channel].max_volume) ? ise_dsp_vox[channel].max_volume - ise_dsp_vox[channel].sustain_volume : 0;
                                ise_dsp_vox[channel].attack_volume_inc = ise_dsp_vox[channel].max_volume / (ise_dsp_vox[channel].preset->attack_samples + 1);
                                ise_dsp_vox[channel].decay_volume_dec = (ise_dsp_vox[channel].max_volume - ise_dsp_vox[channel].sustain_volume) / (ise_dsp_vox[channel].preset->decay_samples + 1);
                                ise_dsp_vox[channel].release_volume_dec = ise_dsp_vox[channel].sustain_volume / (ise_dsp_vox[channel].preset->release_samples + 1);
                            }
                        } else {
                            bool found = false;
                            for(channel=0; !found && channel < ISE_DSP_NUM_PERCUSSIVE_VOICES; channel++) {
                                if(ise_dsp_vox[ISE_DSP_NUM_MELODIC_VOICES+channel].cur_vol_env == ISE_DSP_VOL_ENV_OFF) {
                                    found = true;
                                    break;
                                }
                            }
                            if(!found) {
                                int best_channel;
                                uint32_t best_vol_env = 0;
                                for(channel=0; channel < ISE_DSP_NUM_PERCUSSIVE_VOICES; channel++) {
                                    if(ise_dsp_vox[ISE_DSP_NUM_MELODIC_VOICES+channel].cur_vol_env >= best_vol_env) {
                                        best_vol_env = ise_dsp_vox[ISE_DSP_NUM_MELODIC_VOICES+channel].cur_vol_env;
                                        best_channel = channel;
                                    }
                                }
                                channel = best_channel;
                            }
                            ise_dsp_percussive_note_mapping[channel] = note;
                            channel += ISE_DSP_NUM_MELODIC_VOICES;
                            octave = 0;
                            // find a preset for the note being played
                            for(ise_dsp_vox[channel].preset = ise_dsp_sf2.presets[128]; ise_dsp_vox[channel].preset != NULL; ise_dsp_vox[channel].preset = ise_dsp_vox[channel].preset->next) {
                                if(note >= ise_dsp_vox[channel].preset->key_lo && note <= ise_dsp_vox[channel].preset->key_hi) break;
                            }
                            // make sure a preset is assigned to the voice
                            if(ise_dsp_vox[channel].preset) {
                                if(ise_dsp_vox[channel].preset->sample_rate < 30000) octave++;  // should be 22050, or less
                                if(ise_dsp_vox[channel].preset->sample_rate < 15000) octave++;  // should be 11025, or less
                                if(ise_dsp_vox[channel].preset->sample_rate < 7500) octave++;   // should be 5512
                                ise_dsp_vox[channel].cur_vol_env = ISE_DSP_VOL_ENV_DELAY;
                                ise_dsp_vox[channel].cur_sample_vol_env = 0;
                                ise_dsp_vox[channel].cur_index = ise_dsp_vox[channel].preset->start_sample;
                                ise_dsp_vox[channel].cur_fract = 0;
                                ise_dsp_vox[channel].sample_inc = 0x10000 >> octave;
                                ise_dsp_vox[channel].volume = 0;
                                ise_dsp_vox[channel].max_volume = (velocity << 23) | (velocity << 16) | (velocity << 9) | (velocity << 2);
                                ise_dsp_vox[channel].sustain_volume = (ise_dsp_vox[channel].max_volume >> 16) * ise_dsp_vox[channel].preset->sustain_level;
                                ise_dsp_vox[channel].sustain_volume = (ise_dsp_vox[channel].sustain_volume < ise_dsp_vox[channel].max_volume) ? ise_dsp_vox[channel].max_volume - ise_dsp_vox[channel].sustain_volume : 0;
                                ise_dsp_vox[channel].attack_volume_inc = ise_dsp_vox[channel].max_volume / (ise_dsp_vox[channel].preset->attack_samples + 1);
                                ise_dsp_vox[channel].decay_volume_dec = (ise_dsp_vox[channel].max_volume - ise_dsp_vox[channel].sustain_volume) / (ise_dsp_vox[channel].preset->decay_samples + 1);
                                ise_dsp_vox[channel].release_volume_dec = ise_dsp_vox[channel].sustain_volume / (ise_dsp_vox[channel].preset->release_samples + 1);
                                //printf("note on: %d ch %d lo %d hi %d st %d end %d\n", note, channel, ise_dsp_vox[channel].preset->key_lo, ise_dsp_vox[channel].preset->key_hi, ise_dsp_vox[channel].preset->start_sample, ise_dsp_vox[channel].preset->end_sample);
                            }
                        }
                        break;
                    case ISE_MIDI_MSG_PROGRAM_CHANGE:
                        if(channel != 15) {
                            // non-precussive
                            ise_dsp_vox[channel].base_preset = ise_dsp_sf2.presets[note];
                        }
                        break;
                    }
                    if(cur_event->f.command != ISE_MIDI_META_EVENT_TYPE_END_TRACK) {
                        ise_dsp_midi_event++;
                    }
                } else {
                    midi_done = true;
                }
            }
            
            // Load in all samples
            int32_t sample = 0;
            for(v=0; v<ISE_DSP_NUM_VOICES; v++) {
                if(ise_dsp_vox[v].cur_vol_env != ISE_DSP_VOL_ENV_OFF) {
                    sample += ise_dsp_sample_vox(v);
                }
            }
            if(ise_dsp_pcm_length) {
                sample += ise_dsp_sample_pcm();
            }
            if(sample > 0x7FFF) sample = 0x7FFF;
            if(sample < -0x8000) sample = -0x8000;
            *mem16 = sample;
            mem16++;
            
            // increment the midi tick, if needed
            ise_dsp_cur_sample_in_midi_tick++;
            if(ise_dsp_cur_sample_in_midi_tick >= ise_dsp_samples_per_midi_tick) {
                ise_dsp_cur_sample_in_midi_tick -= ise_dsp_samples_per_midi_tick;
                ise_dsp_midi_delta_tick++;
            }
        }
        if(ise_dsp_midi_event == NULL && ise_dsp_pcm_length == 0) ise_dsp_stop_loop();
        ise_dsp_pcm_load_tick++;
    }
}

void ise_dsp_uninstall()
{
    if(ise_dsp_pause) ise_dsp_pause();
    if(ise_dsp_reset) ise_dsp_reset();
    if(ise_dsp_irq_orig_mask) {
        int pic = (ise_dsp_irq >= 8) ? ISE_IRQ_PIC1_DATA : ISE_IRQ_PIC0_DATA;
        int int_mask = inp(pic) | (1 << (ise_dsp_irq & 0x7));
        outp(pic, int_mask);
    }
    if(prev_dsp_isr) {
        _dos_setvect(ISE_IRQ_INTERRUPT_VECTOR(ise_dsp_irq), prev_dsp_isr);
        prev_dsp_isr = NULL;
    }
    if(ise_dsp_mem_buffer) {
        ise_mem_aligned_free(ise_dsp_mem_buffer);
        ise_dsp_mem_buffer = NULL;
    }
}