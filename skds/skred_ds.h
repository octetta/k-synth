#ifndef SKRED_DS_H
#define SKRED_DS_H

#include "miniaudio.h"

typedef enum {
    skred_loop_oneshot_t = 0,
    skred_loop_forward_t = 1,
    skred_loop_pingpong_t = 2
} skred_loop_mode_t;

typedef enum {
    SKRED_ADSR_IDLE = 0,
    SKRED_ADSR_ATTACK,
    SKRED_ADSR_DECAY,
    SKRED_ADSR_SUSTAIN,
    SKRED_ADSR_RELEASE
} skred_adsr_state_t;

typedef struct {
    ma_data_source_base base;

    /* Buffer Info */
    float* p_buffer;
    ma_uint32 buffer_frames;
    ma_uint32 buffer_sample_rate;
    ma_uint32 engine_sample_rate;

    /* Playback State */
    double read_index;
    double loop_start;
    double loop_end;
    skred_loop_mode_t loop_mode;
    int is_playing;

    /* Base Parameters (Double precision for phase integrity) */
    double base_hz;
    
    /* Frequency uses Linear Ramping to prevent asymptotic stalling */
    double current_freq;
    double target_freq;
    double freq_step;

    /* Vol, Pan, Dir use Exponential Smoothing */
    double current_vol;
    double target_vol;
    double alpha_vol;

    double current_pan;
    double target_pan;
    double alpha_pan;

    double current_dir;
    double target_dir;
    double alpha_dir;

    /* ADSR Envelope */
    skred_adsr_state_t adsr_state;
    double env_val;
    double attack_inc;
    double decay_inc;
    double sustain_level;
    double release_inc;

    /* LFO Modulation (Wavetable based) */
    float* p_lfo_buffer;
    ma_uint32 lfo_frames;
    double lfo_read_index;
    double lfo_freq;
    double mod_depth_freq; 
    double mod_depth_vol;  
    double mod_depth_pan;  

} skred_voice_t;

ma_result skred_voice_init(ma_uint32 engine_sample_rate, float* p_buffer, ma_uint32 buffer_frames, ma_uint32 buffer_sample_rate, double base_hz, skred_voice_t* p_voice);

void skred_voice_set_buffer(skred_voice_t* p_voice, float* p_new_buffer, ma_uint32 new_frames);
void skred_voice_set_sample(skred_voice_t* p_voice, float* p_new_buffer, ma_uint32 new_frames, int is_oneshot);

void skred_voice_set_freq(skred_voice_t* p_voice, float hz, float ease_ms);
void skred_voice_set_vol(skred_voice_t* p_voice, float db, float ease_ms);
void skred_voice_set_pan(skred_voice_t* p_voice, float pan, float ease_ms);
void skred_voice_set_dir(skred_voice_t* p_voice, float dir, float ease_ms);
void skred_voice_set_loop(skred_voice_t* p_voice, double start_idx, double end_idx, skred_loop_mode_t mode);

void skred_voice_set_adsr(skred_voice_t* p_voice, float a_ms, float d_ms, float s_level, float r_ms);
void skred_voice_set_lfo_wave(skred_voice_t* p_voice, float* p_buf, ma_uint32 frames);
void skred_voice_set_lfo(skred_voice_t* p_voice, float lfo_hz, float depth_freq, float depth_vol, float depth_pan);

void skred_voice_note_on(skred_voice_t* p_voice);
void skred_voice_note_off(skred_voice_t* p_voice);
void skred_voice_stop(skred_voice_t* p_voice);
void skred_voice_trig(skred_voice_t* p_voice); /* ADSR Gate bypass */

#endif // SKRED_DS_H
