#include "skred_ds.h"
#include <math.h>
#include <stdlib.h>

#define SKRED_HEADROOM_DB -12.0
#define SKRED_PI 3.14159265358979323846

/* 99% target precision one-pole filter coefficient */
static double calculate_alpha(float ease_ms, ma_uint32 sample_rate) {
    if (ease_ms <= 1.0f) return 1.0; 
    double time_in_seconds = (double)ease_ms / 1000.0;
    return 1.0 - exp(-4.605 / ((double)sample_rate * time_in_seconds));
}

static double db_to_linear(float db) {
    if (db <= -96.0f) return 0.0;
    return pow(10.0, (db + SKRED_HEADROOM_DB) / 20.0);
}

static ma_result skred_voice_get_data_format(ma_data_source* p_data_source, ma_format* p_format, ma_uint32* p_channels, ma_uint32* p_sample_rate, ma_channel* p_channel_map, size_t channel_map_cap) {
    skred_voice_t* p_voice = (skred_voice_t*)p_data_source;
    if (p_format)      *p_format      = ma_format_f32;
    if (p_channels)    *p_channels    = 2;
    if (p_sample_rate) *p_sample_rate = p_voice->engine_sample_rate;
    (void)p_channel_map;
    (void)channel_map_cap;
    return MA_SUCCESS;
}

static ma_result skred_voice_read(ma_data_source* p_data_source, void* p_frames_out, ma_uint64 frame_count, ma_uint64* p_frames_read) {
    skred_voice_t* p_voice = (skred_voice_t*)p_data_source;
    float* p_out = (float*)p_frames_out;
    ma_uint64 frames_generated = 0;

    double start = p_voice->loop_start;
    double end = p_voice->loop_end;
    double sr_ratio = (double)p_voice->buffer_sample_rate / (double)p_voice->engine_sample_rate;

    for (ma_uint64 i = 0; i < frame_count; ++i) {
        if (!p_voice->is_playing || p_voice->adsr_state == SKRED_ADSR_IDLE) break;

        /* ADSR Processing */
        switch (p_voice->adsr_state) {
            case SKRED_ADSR_ATTACK:
                p_voice->env_val += p_voice->attack_inc;
                if (p_voice->env_val >= 1.0) {
                    p_voice->env_val = 1.0;
                    p_voice->adsr_state = SKRED_ADSR_DECAY;
                }
                break;
            case SKRED_ADSR_DECAY:
                p_voice->env_val -= p_voice->decay_inc;
                if (p_voice->env_val <= p_voice->sustain_level) {
                    p_voice->env_val = p_voice->sustain_level;
                    p_voice->adsr_state = SKRED_ADSR_SUSTAIN;
                }
                break;
            case SKRED_ADSR_SUSTAIN:
                p_voice->env_val = p_voice->sustain_level;
                break;
            case SKRED_ADSR_RELEASE:
                p_voice->env_val -= p_voice->release_inc;
                if (p_voice->env_val <= 0.0001) {
                    p_voice->env_val = 0.0;
                    p_voice->adsr_state = SKRED_ADSR_IDLE;
                    p_voice->is_playing = 0;
                    continue; 
                }
                break;
            default: break;
        }

        /* Parameter Smoothing: Linear Ramp for Freq */
        if (p_voice->freq_step != 0.0) {
            p_voice->current_freq += p_voice->freq_step;
            if ((p_voice->freq_step > 0.0 && p_voice->current_freq >= p_voice->target_freq) ||
                (p_voice->freq_step < 0.0 && p_voice->current_freq <= p_voice->target_freq)) {
                p_voice->current_freq = p_voice->target_freq;
                p_voice->freq_step = 0.0;
            }
        }

        /* Parameter Smoothing: Exponential for Vol, Pan, Dir */
        p_voice->current_vol  += p_voice->alpha_vol  * (p_voice->target_vol  - p_voice->current_vol);
        p_voice->current_pan  += p_voice->alpha_pan  * (p_voice->target_pan  - p_voice->current_pan);
        p_voice->current_dir  += p_voice->alpha_dir  * (p_voice->target_dir  - p_voice->current_dir);

        /* LFO Processing (Wavetable Interpolation) */
        float lfo_val = 0.0f;
        if (p_voice->p_lfo_buffer && p_voice->lfo_frames > 0 && p_voice->lfo_freq > 0.0) {
            double lfo_inc = (p_voice->lfo_freq * (double)p_voice->lfo_frames) / (double)p_voice->engine_sample_rate;
            p_voice->lfo_read_index += lfo_inc;
            while (p_voice->lfo_read_index >= p_voice->lfo_frames) {
                p_voice->lfo_read_index -= p_voice->lfo_frames;
            }
            
            ma_uint32 l0 = (ma_uint32)p_voice->lfo_read_index;
            ma_uint32 l1 = (l0 + 1 >= p_voice->lfo_frames) ? 0 : l0 + 1;
            float l_frac = (float)(p_voice->lfo_read_index - l0);
            lfo_val = (1.0f - l_frac) * p_voice->p_lfo_buffer[l0] + l_frac * p_voice->p_lfo_buffer[l1];
        }

        /* Apply Modulations */
        double mod_freq = p_voice->current_freq + (lfo_val * p_voice->mod_depth_freq);
        double mod_vol  = p_voice->current_vol  + (lfo_val * p_voice->mod_depth_vol);
        double mod_pan  = p_voice->current_pan  + (lfo_val * p_voice->mod_depth_pan);
        if (mod_vol < 0.0) mod_vol = 0.0;

        /* Panning Math (Equal Power) */
        double p_norm = (mod_pan + 1.0) * 0.5;
        if (p_norm < 0.0) p_norm = 0.0;
        if (p_norm > 1.0) p_norm = 1.0;
        double theta = p_norm * (SKRED_PI * 0.5);
        double gain_l = cos(theta);
        double gain_r = sin(theta);

        /* Main Audio Buffer Interpolation */
        ma_uint32 idx0 = (ma_uint32)p_voice->read_index;
        ma_uint32 idx1 = idx0 + 1;

        if (idx1 >= (ma_uint32)end) {
            if (p_voice->loop_mode == skred_loop_forward_t) idx1 = (ma_uint32)start;
            else idx1 = idx0;
        }
        
        double frac = p_voice->read_index - (double)idx0;
        float s0 = p_voice->p_buffer[idx0];
        float s1 = p_voice->p_buffer[idx1];
        float sample = (float)((1.0 - frac) * s0 + frac * s1);

        /* Output with ADSR Envelope */
        double final_gain = mod_vol * p_voice->env_val;
        p_out[frames_generated * 2]     = sample * (float)(gain_l * final_gain);
        p_out[frames_generated * 2 + 1] = sample * (float)(gain_r * final_gain);
        frames_generated++;

        /* Playhead Advancement */
        double increment = (mod_freq / p_voice->base_hz) * sr_ratio;
        p_voice->read_index += increment * p_voice->current_dir;

        /* Loop / Bounds Logic with OneShot Safety Clamp */
        if (p_voice->loop_mode == skred_loop_oneshot_t) {
            if ((p_voice->current_dir > 0 && p_voice->read_index >= end) || 
                (p_voice->current_dir < 0 && p_voice->read_index <= start)) {
                
                skred_voice_note_off(p_voice); 
                
                /* Clamp index to prevent unsigned integer underflow wrap on next iteration */
                if (p_voice->read_index >= end) p_voice->read_index = end;
                if (p_voice->read_index <= start) p_voice->read_index = start;
            }
        } else if (p_voice->loop_mode == skred_loop_forward_t) {
            while (p_voice->read_index >= end) p_voice->read_index -= (end - start);
            while (p_voice->read_index < start) p_voice->read_index += (end - start);
        } else if (p_voice->loop_mode == skred_loop_pingpong_t) {
            if (p_voice->read_index >= end) {
                p_voice->read_index = end - (p_voice->read_index - end);
                p_voice->current_dir = -p_voice->current_dir;
                p_voice->target_dir = -p_voice->target_dir;
            } else if (p_voice->read_index <= start) {
                p_voice->read_index = start + (start - p_voice->read_index);
                p_voice->current_dir = -p_voice->current_dir;
                p_voice->target_dir = -p_voice->target_dir;
            }
        }
    }

    if (p_frames_read) *p_frames_read = frames_generated;
    return (frames_generated == 0) ? MA_AT_END : MA_SUCCESS;
}

static ma_data_source_vtable g_skred_voice_vtable = {
    skred_voice_read, NULL, skred_voice_get_data_format, NULL, NULL, NULL, 0
};

ma_result skred_voice_init(ma_uint32 engine_sample_rate, float* p_buffer, ma_uint32 buffer_frames, ma_uint32 buffer_sample_rate, double base_hz, skred_voice_t* p_voice) {
    ma_data_source_config base_config = ma_data_source_config_init();
    base_config.vtable = &g_skred_voice_vtable;
    ma_result result = ma_data_source_init(&base_config, &p_voice->base);
    if (result != MA_SUCCESS) return result;

    p_voice->p_buffer = p_buffer;
    p_voice->buffer_frames = buffer_frames;
    p_voice->buffer_sample_rate = buffer_sample_rate;
    p_voice->engine_sample_rate = engine_sample_rate;
    p_voice->base_hz = base_hz;
    
    p_voice->read_index = 0.0;
    p_voice->loop_start = 0.0;
    p_voice->loop_end = (double)buffer_frames - 1.0;
    p_voice->loop_mode = skred_loop_forward_t;
    p_voice->is_playing = 0;

    p_voice->current_freq = base_hz;
    p_voice->target_freq = base_hz;
    p_voice->freq_step = 0.0;
    
    p_voice->current_vol = db_to_linear(0.0f);
    p_voice->target_vol = p_voice->current_vol;
    p_voice->alpha_vol = 1.0;
    
    p_voice->current_pan = 0.0;
    p_voice->target_pan = 0.0;
    p_voice->alpha_pan = 1.0;
    
    p_voice->current_dir = 1.0;
    p_voice->target_dir = 1.0;
    p_voice->alpha_dir = 1.0;

    p_voice->adsr_state = SKRED_ADSR_IDLE;
    p_voice->env_val = 0.0;
    skred_voice_set_adsr(p_voice, 5.0f, 100.0f, 0.8f, 50.0f); 

    p_voice->p_lfo_buffer = NULL;
    p_voice->lfo_frames = 0;
    p_voice->lfo_read_index = 0.0;
    skred_voice_set_lfo(p_voice, 0.0f, 0.0f, 0.0f, 0.0f);

    return MA_SUCCESS;
}

void skred_voice_set_buffer(skred_voice_t* p_voice, float* p_new_buffer, ma_uint32 new_frames) {
    double new_base_hz = (double)p_voice->buffer_sample_rate / (double)new_frames;
    double new_end = (double)new_frames - 1.0;
    
    if (new_frames < p_voice->buffer_frames) {
        p_voice->loop_end = new_end;
        if (p_voice->read_index >= new_end) p_voice->read_index = 0.0;
    }
    p_voice->p_buffer = p_new_buffer;
    p_voice->buffer_frames = new_frames;
    p_voice->loop_end = new_end;
    p_voice->base_hz = new_base_hz;
}

void skred_voice_set_sample(skred_voice_t* p_voice, float* p_new_buffer, ma_uint32 new_frames, int is_oneshot) {
    skred_voice_set_buffer(p_voice, p_new_buffer, new_frames);
    if (is_oneshot) {
        p_voice->loop_mode = skred_loop_oneshot_t;
        p_voice->base_hz = 1.0;     
        p_voice->target_freq = 1.0; 
        p_voice->current_freq = 1.0;
        p_voice->freq_step = 0.0;
    } else {
        p_voice->loop_mode = skred_loop_forward_t;
    }
}

void skred_voice_set_freq(skred_voice_t* p_voice, float hz, float ease_ms) {
    p_voice->target_freq = (double)hz;
    
    if (ease_ms <= 0.0f) {
        p_voice->current_freq = p_voice->target_freq;
        p_voice->freq_step = 0.0;
    } else {
        double frames = ((double)p_voice->engine_sample_rate * (double)ease_ms) / 1000.0;
        p_voice->freq_step = (p_voice->target_freq - p_voice->current_freq) / frames;
    }
}

void skred_voice_set_vol(skred_voice_t* p_voice, float db, float ease_ms) {
    p_voice->target_vol = db_to_linear(db);
    p_voice->alpha_vol = calculate_alpha(ease_ms, p_voice->engine_sample_rate);
}

void skred_voice_set_pan(skred_voice_t* p_voice, float pan, float ease_ms) {
    p_voice->target_pan = (double)pan;
    p_voice->alpha_pan = calculate_alpha(ease_ms, p_voice->engine_sample_rate);
}

void skred_voice_set_dir(skred_voice_t* p_voice, float dir, float ease_ms) {
    p_voice->target_dir = (double)dir;
    p_voice->alpha_dir = calculate_alpha(ease_ms, p_voice->engine_sample_rate);
}

void skred_voice_set_loop(skred_voice_t* p_voice, double start_idx, double end_idx, skred_loop_mode_t mode) {
    p_voice->loop_start = start_idx;
    p_voice->loop_end = end_idx;
    p_voice->loop_mode = mode;
}

void skred_voice_set_adsr(skred_voice_t* p_voice, float a_ms, float d_ms, float s_level, float r_ms) {
    double sr = (double)p_voice->engine_sample_rate;
    p_voice->attack_inc  = (a_ms > 0.0f) ? 1.0 / (sr * (a_ms / 1000.0)) : 1.0;
    p_voice->decay_inc   = (d_ms > 0.0f) ? (1.0 - s_level) / (sr * (d_ms / 1000.0)) : 1.0;
    p_voice->sustain_level = (double)s_level;
    p_voice->release_inc = (r_ms > 0.0f) ? 1.0 / (sr * (r_ms / 1000.0)) : 1.0;
}

void skred_voice_set_lfo_wave(skred_voice_t* p_voice, float* p_buf, ma_uint32 frames) {
    p_voice->p_lfo_buffer = p_buf;
    p_voice->lfo_frames = frames;
    p_voice->lfo_read_index = 0.0; /* Reset phase on wave change */
}

void skred_voice_set_lfo(skred_voice_t* p_voice, float lfo_hz, float depth_freq, float depth_vol, float depth_pan) {
    p_voice->lfo_freq = (double)lfo_hz;
    p_voice->mod_depth_freq = (double)depth_freq;
    p_voice->mod_depth_vol = (double)depth_vol;
    p_voice->mod_depth_pan = (double)depth_pan;
}

void skred_voice_note_on(skred_voice_t* p_voice) {
    p_voice->is_playing = 1;
    p_voice->adsr_state = SKRED_ADSR_ATTACK;
    p_voice->env_val = 0.0; 
    
    /* Snap current volume to target to avoid lag on fast transients */
    p_voice->current_vol = p_voice->target_vol; 

    /* Sync LFO phase on note start */
    p_voice->lfo_read_index = 0.0;

    if (p_voice->loop_mode == skred_loop_oneshot_t && p_voice->target_dir < 0.0) {
        p_voice->read_index = p_voice->loop_end;
    } else {
        p_voice->read_index = p_voice->loop_start;
    }
}

void skred_voice_note_off(skred_voice_t* p_voice) {
    if (p_voice->adsr_state != SKRED_ADSR_IDLE) {
        p_voice->adsr_state = SKRED_ADSR_RELEASE;
    }
}

void skred_voice_stop(skred_voice_t* p_voice) {
    p_voice->is_playing = 0;
    p_voice->adsr_state = SKRED_ADSR_IDLE;
    p_voice->env_val = 0.0;
}

void skred_voice_trig(skred_voice_t* p_voice) {
    /* Hard-slam the ADSR for instantaneous drum triggering */
    p_voice->attack_inc = 1.0; 
    p_voice->decay_inc = 0.0;
    p_voice->sustain_level = 1.0;
    p_voice->release_inc = 1.0;
    skred_voice_note_on(p_voice);
}
