// InfiniScroll Engine
// ise_dsp.h
// Digital Sound Processor related header
//
// Kamal Pillai
// 5/15/2019

#ifndef __ISE_DSP_H
#define __ISE_DSP_H

#define ISE_DSP_MEM_BUFFER_SIZE 0x2000  // 8KB

#define ISE_DSP_NUM_VOICES 20  // 15 to 19 are for percussive voices
#define ISE_DSP_NUM_MELODIC_VOICES 15
#define ISE_DSP_NUM_PERCUSSIVE_VOICES (ISE_DSP_NUM_VOICES - ISE_DSP_NUM_MELODIC_VOICES)

// cmi8738 registers
#define ISE_DSP_CMI8738_FUNC_CTRL_0            0x00
#define ISE_DSP_CMI8738_FUNC_CTRL_1            0x04
#define ISE_DSP_CMI8738_CH_FORMAT              0x08
#define ISE_DSP_CMI8738_INT_HOLD_CLR           0x0C
#define ISE_DSP_CMI8738_INT_STATUS             0x10
#define ISE_DSP_CMI8738_LEG_CTRL_STAT          0x14
#define ISE_DSP_CMI8738_MISC_CTRL              0x18
#define ISE_DSP_CMI8738_TDMA_POS               0x1C
#define ISE_DSP_CMI8738_SBVERSION              0x20
#define ISE_DSP_CMI8738_DEV_CFG                0x21
#define ISE_DSP_CMI8738_MIXER_ADDR             0x22
#define ISE_DSP_CMI8738_MIXER_DATA             0x23
#define ISE_DSP_CMI8738_MIXER_CFG0             0x24
#define ISE_DSP_CMI8738_MIXER_CFG1             0x25
#define ISE_DSP_CMI8738_MIXER_CFG2             0x26
#define ISE_DSP_CMI8738_MIXER_CFG3             0x27
#define ISE_DSP_CMI8738_CH0_FRAME_REG0         0x80
#define ISE_DSP_CMI8738_CH0_FRAME_REG1         0x84
#define ISE_DSP_CMI8738_CH1_FRAME_REG0         0x88
#define ISE_DSP_CMI8738_CH1_FRAME_REG1         0x8C

// Mixer registers
#define ISE_DSP_MIXER_INTERRUPT_SETUP          0x80
#define ISE_DSP_MIXER_DMA_SETUP                0x81
#define ISE_DSP_MIXER_INTERRUPT_STATUS         0x82

// DSP commands
#define ISE_DSP_CMD_GET_VERSION                0xE1

#define ISE_DSP_CMD_TIME_CONSTANT              0x40
#define ISE_DSP_CMD_OUT_SAMPLE_RATE            0x41
#define ISE_DSP_CMD_IN_SAMPLE_RATE             0x42
#define ISE_DSP_CMD_BLOCK_TRANSFER_SIZE        0x48
#define ISE_DSP_CMD_DMA_8B_OUT_AI_LEGACY       0x1C
#define ISE_DSP_CMD_SPEAKER_ENABLE             0xD1
#define ISE_DSP_CMD_SPEAKER_DISABLE            0xD3

#define ISE_DSP_CMD_DMA_8B_OUT_SC              0xC0
#define ISE_DSP_CMD_DMA_8B_OUT_AI              0xC6
#define ISE_DSP_CMD_DMA_8B_IN_SC               0xC8
#define ISE_DSP_CMD_DMA_8B_IN_AI               0xCE

#define ISE_DSP_CMD_DMA_16B_OUT_SC             0xB0
#define ISE_DSP_CMD_DMA_16B_OUT_AI             0xB6
#define ISE_DSP_CMD_DMA_16B_IN_SC              0xB8
#define ISE_DSP_CMD_DMA_16B_IN_AI              0xBE

#define ISE_DSP_CMD_MODE_MONO_UNSIGNED         0x00
#define ISE_DSP_CMD_MODE_STEREO_UNSIGNED       0x20
#define ISE_DSP_CMD_MODE_MONO_SIGNED           0x10
#define ISE_DSP_CMD_MODE_STEREO_SIGNED         0x30

#define ISE_DSP_CMD_MODE_STEREO_FLAG           0x20
#define ISE_DSP_CMD_MODE_SIGNED_FLAG           0x10

#define ISE_DSP_CMD_PAUSE_8B_DMA               0xD0
#define ISE_DSP_CMD_CONTINUE_8B_DMA            0xD4
#define ISE_DSP_CMD_PAUSE_16B_DMA              0xD5
#define ISE_DSP_CMD_CONTINUE_16B_DMA           0xD6
#define ISE_DSP_CMD_EXIT_8B_AI_DMA             0xDA
#define ISE_DSP_CMD_EXIT_16B_AI_DMA            0xD9

#pragma pack(push, 1)

#define ISE_DSP_RIFF_CHUNK_ID                  0x46464952  // "RIFF" in little endian

// Wave Riff enums
#define ISE_DSP_WAV_FORMAT_ID                  0x45564157  // "WAVE" in little endian
#define ISE_DSP_WAV_SUBCHUNK_FMT_ID            0x20746D66  // "fmt " in little endian
#define ISE_DSP_WAV_SUBCHUNK_DATA_ID           0x61746164  // "data" in little endian

// SoundFont2 Riff enums
#define ISE_DSP_SF2_FORMAT_ID                  0x6B626673  // "sfbk" in little endian
#define ISE_DSP_SF2_SUBCHUNK_LIST_ID           0x5453494C  // "LIST" in little endian
#define ISE_DSP_SF2_INFO_ID                    0x4F464E49  // "INFO" in little endian

#define ISE_DSP_SF2_SAMPLE_DATA_ID             0x61746473  // "sdta" in little endian
#define ISE_DSP_SF2_SAMPLE_ID                  0x6C706D73  // "smpl" in little endian
#define ISE_DSP_SF2_SAMPLE24_ID                0x34326D73  // "sm24" in little endian

#define ISE_DSP_SF2_PRESET_DATA_ID             0x61746470  // "pdta" in little endian
#define ISE_DSP_SF2_PRESET_HEADER_ID           0x72646870  // "phdr" in little endian
#define ISE_DSP_SF2_PRESET_BAG_ID              0x67616270  // "pbag" in little endian
#define ISE_DSP_SF2_PRESET_MOD_ID              0x646F6D70  // "pmod" in little endian
#define ISE_DSP_SF2_PRESET_GEN_ID              0x6E656770  // "pgen" in little endian
#define ISE_DSP_SF2_INST_ID                    0x74736E69  // "inst" in little endian
#define ISE_DSP_SF2_INST_BAG_ID                0x67616269  // "ibag" in little endian
#define ISE_DSP_SF2_INST_MOD_ID                0x646F6D69  // "imod" in little endian
#define ISE_DSP_SF2_INST_GEN_ID                0x6E656769  // "igen" in little endian
#define ISE_DSP_SF2_SAMPLE_HEADER_ID           0x72646873  // "shdr" in little endian

// sound font generators
#define ISE_DSP_SF2_GEN_START_ADDR_OFFSET      00
#define ISE_DSP_SF2_GEN_END_ADDR_OFFSET        01
#define ISE_DSP_SF2_GEN_START_LOOP_OFFSET      02
#define ISE_DSP_SF2_GEN_END_LOOP_OFFSET        03
#define ISE_DSP_SF2_GEN_START_ADDR_COARSE      04
#define ISE_DSP_SF2_GEN_END_ADDR_COARSE        12
#define ISE_DSP_SF2_GEN_DELAY_VOL_ENV          33
#define ISE_DSP_SF2_GEN_ATTACK_VOL_ENV         34
#define ISE_DSP_SF2_GEN_HOLD_VOL_ENV           35
#define ISE_DSP_SF2_GEN_DECAY_VOL_ENV          36
#define ISE_DSP_SF2_GEN_SUSTAIN_VOL_ENV        37
#define ISE_DSP_SF2_GEN_RELEASE_VOL_ENV        38
#define ISE_DSP_SF2_GEN_INSTRUMENT             41
#define ISE_DSP_SF2_GEN_KEY_RANGE              43
#define ISE_DSP_SF2_GEN_VEL_RANGE              44
#define ISE_DSP_SF2_GEN_START_LOOP_COARSE      45
#define ISE_DSP_SF2_GEN_KEY_NUM                46
#define ISE_DSP_SF2_GEN_END_LOOP_COARSE        50
#define ISE_DSP_SF2_GEN_SAMPLE_ID              53
#define ISE_DSP_SF2_GEN_SAMPLE_MODE            54
#define ISE_DSP_SF2_GEN_OVERRIDE_PITCH         58

typedef struct {
    uint32_t chunk_id;
    uint32_t chunk_size;
    uint32_t format_id;
} ise_dsp_riff_header_t;

typedef struct {
    uint32_t id;
    uint32_t size;
} ise_dsp_riff_subchunk_header_t;

typedef struct {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
} ise_dsp_wav_format_t;

typedef struct {
    char preset_name[20];
    uint16_t preset;
    uint16_t bank;
    uint16_t preset_bag_ndx;
    uint32_t library;
    uint32_t genre;
    uint32_t morphology;
} ise_dsp_sf2_phdr_t;

typedef struct {
    uint16_t gen_ndx;
    uint16_t mod_ndx;
} ise_dsp_sf2_bag_t;

typedef struct {
    uint16_t mod_src_op;
    uint16_t mod_dst_op;
    uint16_t mod_amount;
    uint16_t mod_amt_src_op;
    uint16_t mod_trans_op;
} ise_dsp_sf2_mod_list_t;

typedef struct {
    uint8_t lo;
    uint8_t hi;
} ise_dsp_sf2_range_t;

typedef union {
    ise_dsp_sf2_range_t range;
    int16_t s16;
    uint16_t u16;
} ise_dsp_sf2_gen_amount_t;

typedef struct {
    uint16_t gen_op;
    ise_dsp_sf2_gen_amount_t gen_amount;
} ise_dsp_sf2_gen_list_t;

typedef struct {
    char inst_name[20];
    uint16_t inst_bag_ndx;
} ise_dsp_sf2_inst_t;

typedef struct {
    char sample_name[20];
    uint32_t start;
    uint32_t end;
    uint32_t start_loop;
    uint32_t end_loop;
    uint32_t sample_rate;
    uint8_t orig_pitch;
    int8_t pitch_correction;
    uint16_t sample_link;
    uint16_t sample_type;
} ise_dsp_sf2_sample_t;

#pragma pack(pop)

typedef struct {
    uint8_t bytes_per_sample;
    uint8_t num_channels;
    uint16_t reserved;
    uint32_t samples_per_second;
    uint32_t size;
    uint8_t* mem;
} ise_dsp_pcm_t;

// forward declaration
struct ise_dsp_sf2_preset_s;
typedef struct ise_dsp_sf2_preset_s ise_dsp_sf2_preset_t;

struct ise_dsp_sf2_preset_s {
    uint8_t key_lo;
    uint8_t key_hi;
    uint8_t vel_lo;
    uint8_t vel_hi;
    uint8_t key_pitch;
    int8_t pitch_correction;
    uint8_t sample_mode;
    uint32_t start_sample;
    uint32_t end_sample;
    uint32_t start_loop_sample;
    uint32_t end_loop_sample;
    uint32_t sample_rate;
    uint32_t delay_samples;
    uint32_t attack_samples;
    uint32_t hold_samples;
    uint32_t decay_samples;
    uint32_t sustain_level;
    uint32_t release_samples;
    ise_dsp_sf2_preset_t* next;
};

typedef struct {
    int16_t* sample_data;
    ise_dsp_sf2_preset_t* presets[129];  // index 128 for percussion
} ise_dsp_sf2_t;

#define ISE_DSP_VOL_ENV_OFF                    0
#define ISE_DSP_VOL_ENV_DELAY                  1
#define ISE_DSP_VOL_ENV_ATTACK                 2
#define ISE_DSP_VOL_ENV_HOLD                   3
#define ISE_DSP_VOL_ENV_DECAY                  4
#define ISE_DSP_VOL_ENV_SUSTAIN                5
#define ISE_DSP_VOL_ENV_RELEASE                6

typedef struct {
    uint32_t cur_vol_env;
    uint32_t cur_sample_vol_env;
    uint32_t cur_index;
    uint32_t cur_fract;  // 16 lsb for fraction of index
    uint32_t sample_inc;  // fixed point, 16.16 increment
    uint32_t volume;  // full volume at 0x10000, set to 0 to turn off voice
    uint32_t max_volume;            // fixed point, 16.16
    uint32_t sustain_volume;        // fixed point, 16.16
    uint32_t attack_volume_inc;     // fixed point, 16.16
    uint32_t decay_volume_dec;      // fixed point, 16.16
    uint32_t release_volume_dec;    // fixed point, 16.16
    ise_dsp_sf2_preset_t* base_preset;
    ise_dsp_sf2_preset_t* preset;
} ise_dsp_vox_t;

void ise_dsp_load_wav_file(const char* filename, ise_dsp_pcm_t* pcm);
void ise_dsp_free_pcm(ise_dsp_pcm_t* pcm);
void ise_dsp_play_pcm(ise_dsp_pcm_t* pcm);
void ise_dsp_stop_pcm();

void ise_dsp_load_sf2_file(const char* filename, ise_dsp_sf2_t* sf2);
void ise_dsp_free_sf2(ise_dsp_sf2_t* sf2);
void ise_dsp_play_track(ise_midi_event_t* track);

void ise_dsp_init_cmi8738(int slot, int function);
void ise_dsp_init_sb();

void ise_dsp_tick();

void ise_dsp_install();
void ise_dsp_uninstall();

#endif  //  #ifndef __ISE_DSP_H
