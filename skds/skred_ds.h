#ifndef SKRED_DS_H
#define SKRED_DS_H

#include "miniaudio.h"

typedef enum {
    skred_loop_oneshot_t,
    skred_loop_forward_t,
    skred_loop_pingpong_t
} skred_loop_mode_t;

typedef struct {
    ma_data_source_base base;
    
    /* Buffer context */
    float* p_buffer;
    ma_uint32 buffer_frames;
    ma_uint32 buffer_sample_rate;
    ma_uint32 engine_sample_rate;
    float base_hz;
    
    /* Phase & Looping */
    double read_index;
    double loop_start;
    double loop_end;
    skred_loop_mode_t loop_mode;
    
    /* Frequency (Hz) */
    double current_freq;
    double target_freq;
    double alpha_freq;
    
    /* Volume (Linear Gain derived from dB) */
    double current_vol;
    double target_vol;
    double alpha_vol;
    
    /* Panning (-1.0 to 1.0) */
    double current_pan;
    double target_pan;
    double alpha_pan;
    
    /* Playback Direction (1.0 = forward, -1.0 = reverse) */
    double current_dir;
    double target_dir;
    double alpha_dir;
    
    int is_playing;

} skred_voice_t;

/* Initialization */
ma_result skred_voice_init(ma_uint32 engine_sample_rate, float* p_buffer, ma_uint32 buffer_frames, ma_uint32 buffer_sample_rate, float base_hz, skred_voice_t* p_voice);

/* Real-time Control Setters */
void skred_voice_set_freq(skred_voice_t* p_voice, float hz, float ease_ms);
void skred_voice_set_vol(skred_voice_t* p_voice, float db, float ease_ms);
void skred_voice_set_pan(skred_voice_t* p_voice, float pan, float ease_ms);
void skred_voice_set_dir(skred_voice_t* p_voice, float dir, float ease_ms);
void skred_voice_set_loop(skred_voice_t* p_voice, double start_idx, double end_idx, skred_loop_mode_t mode);
void skred_voice_play(skred_voice_t* p_voice);

#endif /* SKRED_DS_H */
