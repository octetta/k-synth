#define MINIAUDIO_IMPLEMENTATION
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

/* ==========================================================================
 * Main Application
 * ========================================================================== */

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    ma_result result;
    ma_engine engine;
    
    /* Objects for standard waveforms */
    ma_waveform waveform;
    ma_sound osc_sound;

    /* Objects for custom triangle data source */
    triangle_osc_t tri_osc;
    ma_sound tri_sound;

    /* Objects for in-memory drum sample placeholder */
    ma_audio_buffer drum_buffer;
    ma_sound drum_sound;
    float* p_drum_data = NULL;

    /* 1. Initialize Engine */
    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine.\n");
        return -1;
    }

    /* 2. Initialize Standard Miniaudio Waveform */
    ma_waveform_config wave_config = ma_waveform_config_init(
        ma_format_f32, 1, engine.sampleRate, ma_waveform_type_sine, 1.0, 440.0
    );
    ma_waveform_init(&wave_config, &waveform);
    ma_sound_init_from_data_source(&engine, &waveform, 0, NULL, &osc_sound);

    /* 3. Initialize Custom Triangle Data Source */
    triangle_osc_init((float)engine.sampleRate, 440.0f, &tri_osc);
    ma_sound_init_from_data_source(&engine, &tri_osc, 0, NULL, &tri_sound);

    /* 4. Generate Long In-Memory Buffer (Drum Sample Placeholder) */
    /* Allocating 2 seconds of audio data */
    ma_uint64 drum_frames = engine.sampleRate * 2;
    p_drum_data = (float*)calloc(drum_frames, sizeof(float));
    
    /* Synthesize a basic electronic drum hit (pitch decaying sine wave) */
    double p = 0.0;
    for (ma_uint64 i = 0; i < drum_frames; i++) {
        double t_sec = (double)i / engine.sampleRate;
        double env = exp(-t_sec * 8.0);              /* Amplitude decay */
        double freq = 50.0 + (150.0 * exp(-t_sec * 15.0)); /* Pitch drop */
        p_drum_data[i] = (float)(sin(p * 2.0 * PI) * env);
        p += freq / engine.sampleRate;
    }

    /* Wrap the raw float array in an ma_audio_buffer */
    ma_audio_buffer_config drum_buf_config = ma_audio_buffer_config_init(
        ma_format_f32, 1, drum_frames, p_drum_data, NULL
    );
    ma_audio_buffer_init(&drum_buf_config, &drum_buffer);
    
    /* * We don't want the drum sample to loop by default or stream endlessly. 
     * We just initialize it. MA_SOUND_FLAG_NO_SPATIALIZATION saves CPU.
     */
    ma_sound_init_from_data_source(&engine, &drum_buffer, MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, &drum_sound);

    printf("Miniaudio Extended Dynamic Generator\n");
    printf("------------------------------------\n");
    printf("Commands:\n");
    printf("  play <osc|tri|drum>\n");
    printf("  stop <osc|tri>\n");
    printf("  type <sine|square|saw> (applies to osc)\n");
    printf("  freq <hz>              (applies to osc & tri)\n");
    printf("  quit\n\n");

    /* 5. Command Loop */
    char input_buffer[128];
    while (1) {
        printf("> ");
        if (!fgets(input_buffer, sizeof(input_buffer), stdin)) {
            break;
        }

        input_buffer[strcspn(input_buffer, "\r\n")] = 0;

        if (strncmp(input_buffer, "quit", 4) == 0 || strncmp(input_buffer, "exit", 4) == 0) {
            break;
        } else if (strncmp(input_buffer, "play osc", 8) == 0) {
            ma_sound_start(&osc_sound);
        } else if (strncmp(input_buffer, "stop osc", 8) == 0) {
            ma_sound_stop(&osc_sound);
        } else if (strncmp(input_buffer, "play tri", 8) == 0) {
            ma_sound_start(&tri_sound);
        } else if (strncmp(input_buffer, "stop tri", 8) == 0) {
            ma_sound_stop(&tri_sound);
        } else if (strncmp(input_buffer, "play drum", 9) == 0) {
            /* For a one-shot sample, rewind to frame 0 before playing */
            ma_sound_seek_to_pcm_frame(&drum_sound, 0);
            ma_sound_start(&drum_sound);
            printf("Triggered drum buffer.\n");
        } else if (strncmp(input_buffer, "type ", 5) == 0) {
            char* type_str = input_buffer + 5;
            if (strcmp(type_str, "sine") == 0) {
                ma_waveform_set_type(&waveform, ma_waveform_type_sine);
            } else if (strcmp(type_str, "square") == 0) {
                ma_waveform_set_type(&waveform, ma_waveform_type_square);
            } else if (strcmp(type_str, "saw") == 0) {
                ma_waveform_set_type(&waveform, ma_waveform_type_sawtooth);
            } else {
                printf("Unknown type.\n");
            }
        } else if (strncmp(input_buffer, "freq ", 5) == 0) {
            double freq = atof(input_buffer + 5);
            /* Update standard waveform */
            ma_waveform_set_frequency(&waveform, freq);
            /* Update custom triangle */
            tri_osc.frequency = (float)freq;
            printf("Frequency set to %.1f Hz\n", freq);
        }
    }

    /* 6. Cleanup */
    ma_sound_uninit(&drum_sound);
    ma_audio_buffer_uninit(&drum_buffer);
    free(p_drum_data);

    ma_sound_uninit(&tri_sound);
    ma_sound_uninit(&osc_sound);
    ma_engine_uninit(&engine);

    return 0;
}
