#include "skred_ds.h"
#include <math.h>
#include <stdlib.h>

#define SKRED_HEADROOM_DB -12.0
#define SKRED_PI 3.14159265358979323846

static double calculate_alpha(float ease_ms, ma_uint32 sample_rate) {
    if (ease_ms <= 0.0f) return 1.0;
    return 1.0 - exp(-1.0 / ((double)sample_rate * ((double)ease_ms / 1000.0)));
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
        if (!p_voice->is_playing) break;

        p_voice->current_freq += p_voice->alpha_freq * (p_voice->target_freq - p_voice->current_freq);
        p_voice->current_vol  += p_voice->alpha_vol  * (p_voice->target_vol  - p_voice->current_vol);
        p_voice->current_pan  += p_voice->alpha_pan  * (p_voice->target_pan  - p_voice->current_pan);
        p_voice->current_dir  += p_voice->alpha_dir  * (p_voice->target_dir  - p_voice->current_dir);

        double p_norm = (p_voice->current_pan + 1.0) * 0.5;
        if (p_norm < 0.0) p_norm = 0.0;
        if (p_norm > 1.0) p_norm = 1.0;
        double theta = p_norm * (SKRED_PI * 0.5);
        double gain_l = cos(theta);
        double gain_r = sin(theta);

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

        p_out[frames_generated * 2]     = sample * (float)(gain_l * p_voice->current_vol);
        p_out[frames_generated * 2 + 1] = sample * (float)(gain_r * p_voice->current_vol);
        frames_generated++;

        double increment = (p_voice->current_freq / p_voice->base_hz) * sr_ratio;
        p_voice->read_index += increment * p_voice->current_dir;

        if (p_voice->loop_mode == skred_loop_oneshot_t) {
            if (p_voice->read_index >= end || p_voice->read_index <= start) p_voice->is_playing = 0;
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
    skred_voice_read,
    NULL,
    skred_voice_get_data_format,
    NULL, NULL, NULL, 0
};

ma_result skred_voice_init(ma_uint32 engine_sample_rate, float* p_buffer, ma_uint32 buffer_frames, ma_uint32 buffer_sample_rate, float base_hz, skred_voice_t* p_voice) {
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
    p_voice->current_freq = base_hz;
    p_voice->target_freq = base_hz;
    p_voice->alpha_freq = 1.0;
    p_voice->current_vol = db_to_linear(0.0f);
    p_voice->target_vol = p_voice->current_vol;
    p_voice->alpha_vol = 1.0;
    p_voice->current_pan = 0.0;
    p_voice->target_pan = 0.0;
    p_voice->alpha_pan = 1.0;
    p_voice->current_dir = 1.0;
    p_voice->target_dir = 1.0;
    p_voice->alpha_dir = 1.0;
    p_voice->is_playing = 0;
    return MA_SUCCESS;
}

void skred_voice_set_freq(skred_voice_t* p_voice, float hz, float ease_ms) {
    p_voice->target_freq = (double)hz;
    p_voice->alpha_freq = calculate_alpha(ease_ms, p_voice->engine_sample_rate);
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

void skred_voice_play(skred_voice_t* p_voice) {
    p_voice->is_playing = 1;
}

void skred_voice_stop(skred_voice_t* p_voice) {
    p_voice->is_playing = 0;
    p_voice->read_index = 0.0;
}

void skred_voice_set_buffer(skred_voice_t* p_voice, float* p_new_buffer, ma_uint32 new_frames) {
    /* 1. Calculate the new base frequency for this buffer size.
       Formula: Base Hz = Sample Rate / Buffer Length */
    float new_base_hz = (float)p_voice->buffer_sample_rate / (float)new_frames;

    /* 2. Handle bounds safely (Shrink-First approach) */
    double new_end = (double)new_frames - 1.0;
    
    if (new_frames < p_voice->buffer_frames) {
        p_voice->loop_end = new_end;
        if (p_voice->read_index >= new_end) {
            p_voice->read_index = 0.0;
        }
    }

    /* 3. Atomic Pointer Swap */
    p_voice->p_buffer = p_new_buffer;

    /* 4. Update metadata and Expand bounds if necessary */
    p_voice->buffer_frames = new_frames;
    p_voice->loop_end = new_end;
    p_voice->base_hz = new_base_hz;
}
