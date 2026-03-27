//#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846

/* ==========================================================================
 * Custom Data Source: Triangle Oscillator
 * ========================================================================== */

typedef struct {
    ma_data_source_base base;
    ma_format format;
    ma_uint32 channels;
    ma_uint32 sample_rate;
    float frequency;
    double phase;
} triangle_osc_t;

static ma_result triangle_read_cb(ma_data_source* p_data_source, void* p_frames_out, ma_uint64 frame_count, ma_uint64* p_frames_read) {
    triangle_osc_t* p_osc = (triangle_osc_t*)p_data_source;
    float* p_out = (float*)p_frames_out;
    ma_uint64 i;
    double phase_inc = p_osc->frequency / p_osc->sample_rate;

    for (i = 0; i < frame_count; i++) {
        /* Triangle wave math: 4 * abs(phase - floor(phase + 0.5)) - 1.0 */
        double t = p_osc->phase;
        double val = 4.0 * fabs(t - floor(t + 0.5)) - 1.0;
        p_out[i] = (float)val;

        p_osc->phase += phase_inc;
        if (p_osc->phase >= 1.0) {
            p_osc->phase -= 1.0;
        }
    }

    if (p_frames_read != NULL) {
        *p_frames_read = frame_count;
    }
    return MA_SUCCESS;
}

static ma_result triangle_seek_cb(ma_data_source* p_data_source, ma_uint64 frame_index) {
    triangle_osc_t* p_osc = (triangle_osc_t*)p_data_source;
    double phase = (double)frame_index * p_osc->frequency / p_osc->sample_rate;
    p_osc->phase = phase - floor(phase);
    return MA_SUCCESS;
}

static ma_result triangle_get_format_cb(ma_data_source* p_data_source, ma_format* p_format, ma_uint32* p_channels, ma_uint32* p_sample_rate, ma_channel* p_channel_map, size_t channel_map_cap) {
    triangle_osc_t* p_osc = (triangle_osc_t*)p_data_source;
    (void)p_channel_map;
    (void)channel_map_cap;
    if (p_format != NULL) *p_format = p_osc->format;
    if (p_channels != NULL) *p_channels = p_osc->channels;
    if (p_sample_rate != NULL) *p_sample_rate = p_osc->sample_rate;
    return MA_SUCCESS;
}

static ma_result triangle_get_cursor_cb(ma_data_source* p_data_source, ma_uint64* p_cursor) {
    (void)p_data_source;
    if (p_cursor != NULL) *p_cursor = 0; /* Infinite source */
    return MA_SUCCESS;
}

static ma_result triangle_get_length_cb(ma_data_source* p_data_source, ma_uint64* p_length) {
    (void)p_data_source;
    if (p_length != NULL) *p_length = 0; /* 0 indicates infinite length to the engine */
    return MA_SUCCESS;
}

static ma_data_source_vtable g_triangle_vtable = {
    triangle_read_cb,
    triangle_seek_cb,
    triangle_get_format_cb,
    triangle_get_cursor_cb,
    triangle_get_length_cb,
    NULL, /* set_looping */
    0     /* flags */
};

ma_result triangle_osc_init(float sample_rate, float frequency, triangle_osc_t* p_osc) {
    ma_data_source_config base_config = ma_data_source_config_init();
    base_config.vtable = &g_triangle_vtable;

    p_osc->format = ma_format_f32;
    p_osc->channels = 1;
    p_osc->sample_rate = (ma_uint32)sample_rate;
    p_osc->frequency = frequency;
    p_osc->phase = 0.0;

    return ma_data_source_init(&base_config, &p_osc->base);
}

#include "uedit.h"

/* ==========================================================================
 * Main Application
 * ========================================================================== */

int main(int argc, char** argv) {
    ma_result result;
    ma_engine engine;
    ma_waveform waveform;
    ma_sound osc_sound;
    triangle_osc_t tri_osc;
    ma_sound tri_sound;
    ma_audio_buffer drum_buffer;
    ma_sound drum_sound;
    float* p_drum_data = NULL;

    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) return -1;

    /* Init Osc */
    ma_waveform_config wave_config = ma_waveform_config_init(ma_format_f32, 1, engine.sampleRate, ma_waveform_type_sine, 1.0, 440.0);
    ma_waveform_init(&wave_config, &waveform);
    ma_sound_init_from_data_source(&engine, &waveform, 0, NULL, &osc_sound);

    /* Init Tri */
    triangle_osc_init((float)engine.sampleRate, 440.0f, &tri_osc);
    ma_sound_init_from_data_source(&engine, &tri_osc, 0, NULL, &tri_sound);

    /* Init Drum (2 second buffer) */
    ma_uint64 drum_frames = engine.sampleRate * 2;
    p_drum_data = (float*)calloc(drum_frames, sizeof(float));
    double p = 0.0;
    for (ma_uint64 i = 0; i < drum_frames; i++) {
        double t = (double)i / engine.sampleRate;
        p_drum_data[i] = (float)(sin(p * 2.0 * PI) * exp(-t * 5.0));
        p += (60.0 + (100.0 * exp(-t * 10.0))) / engine.sampleRate;
    }
    ma_audio_buffer_config db_cfg = ma_audio_buffer_config_init(ma_format_f32, 1, drum_frames, p_drum_data, NULL);
    ma_audio_buffer_init(&db_cfg, &drum_buffer);
    ma_sound_init_from_data_source(&engine, &drum_buffer, 0, NULL, &drum_sound);

    printf("Dynamic High-Level Control: Osc, Tri, and Drum\n");
    printf("Commands: play/stop <target>, vol <target> <val>, pan <target> <val>, pitch <target> <val>\n");
    printf("Targets: osc, tri, drum\n\n");

    char cmd[32], target[32];
    float val;
    while (1) {
        printf("top\n");
#if 1
        char line[80];
        int r = uedit("> ", line, sizeof(line));
        if (r < 0) break;
        if (r == 0) continue;
        for (int i=0; i<r; i++) {
            if (cmd[i] == '\r' || cmd[i] == '\n') {
                cmd[i] = '\0';
                break;
            }
        }
        char *ptr = line;
        char *out;
#endif
        ptr = line;
        out = strtok(ptr, " ");
        if (out == NULL) break;
        if (strcmp(out, "quit") == 0) break;
        strcpy(cmd, out);
        printf("{%s}\n", cmd);
        ptr += (strlen(out)+1);

        /* Pointer to the sound we want to manipulate */
        ma_sound* selected_sound = NULL;
        out = strtok(ptr, " ");
        if (out == NULL) continue;
        strcpy(target, out);
        printf("{%s}\n", target);
        ptr += (strlen(out)+1);

        if (strcmp(target, "osc") == 0) selected_sound = &osc_sound;
        else if (strcmp(target, "tri") == 0) selected_sound = &tri_sound;
        else if (strcmp(target, "drum") == 0) selected_sound = &drum_sound;
        else continue;

        if (selected_sound) {
            if (strcmp(cmd, "play") == 0) {
                if (selected_sound == &drum_sound) ma_sound_seek_to_pcm_frame(selected_sound, 0);
                ma_sound_start(selected_sound);
                continue;
            } 
            else if (strcmp(cmd, "stop") == 0) ma_sound_stop(selected_sound);
            else {
                out = strtok(ptr, " ");
                if (out == NULL) break;
                val = strtod(out, &ptr);
                if (ptr == NULL) break;
                printf("{%f}\n", val);
                if (strcmp(cmd, "vol") == 0) ma_sound_set_volume(selected_sound, val);
                else if (strcmp(cmd, "pan") == 0) ma_sound_set_pan(selected_sound, val);
                else if (strcmp(cmd, "pitch") == 0) ma_sound_set_pitch(selected_sound, val);
            }
        }
    }

    /* Cleanup */
    ma_sound_uninit(&drum_sound);
    ma_audio_buffer_uninit(&drum_buffer);
    free(p_drum_data);
    ma_sound_uninit(&tri_sound);
    ma_sound_uninit(&osc_sound);
    ma_engine_uninit(&engine);
    return 0;
}
