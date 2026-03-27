#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    ma_result result;
    ma_engine engine;
    ma_waveform waveform;
    ma_waveform_config waveform_config;
    ma_sound sound;

    /* 1. Initialize the high-level engine */
    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine.\n");
        return -1;
    }

    /* 2. Configure and initialize the waveform data source */
    /* Using native f32, 1 channel, and inheriting the engine's sample rate */
    waveform_config = ma_waveform_config_init(
        ma_format_f32, 
        1, 
        engine.sampleRate, 
        ma_waveform_type_sine, 
        1.0,   /* Amplitude (kept at 1.0; we will use ma_sound_set_volume instead) */
        440.0  /* Initial frequency */
    );
    
    result = ma_waveform_init(&waveform_config, &waveform);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize waveform.\n");
        ma_engine_uninit(&engine);
        return -1;
    }

    /* 3. Wrap the waveform in an ma_sound object */
    /* This attaches the waveform to the engine's node graph, giving us panning/volume control */
    result = ma_sound_init_from_data_source(
        &engine, 
        (ma_data_source*)&waveform, 
        0, /* Flags: e.g., MA_SOUND_FLAG_NO_SPATIALIZATION could be used here to optimize */
        NULL, 
        &sound
    );
    if (result != MA_SUCCESS) {
        printf("Failed to initialize sound.\n");
        ma_engine_uninit(&engine);
        return -1;
    }

    printf("Miniaudio High-Level Waveform Generator (v0.11.25)\n");
    printf("--------------------------------------------------\n");
    printf("Commands:\n");
    printf("  play\n");
    printf("  stop\n");
    printf("  type <sine|square|saw>\n");
    printf("  freq <hz>         (e.g., freq 880)\n");
    printf("  vol <0.0 to 1.0>  (e.g., vol 0.5)\n");
    printf("  pan <-1.0 to 1.0> (e.g., pan -1.0 for left)\n");
    printf("  quit\n\n");

    /* 4. Interactive command loop */
    char input_buffer[128];
    while (1) {
        printf("> ");
        if (!fgets(input_buffer, sizeof(input_buffer), stdin)) {
            break;
        }

        /* Strip trailing newline */
        input_buffer[strcspn(input_buffer, "\r\n")] = 0;

        if (strncmp(input_buffer, "quit", 4) == 0 || strncmp(input_buffer, "exit", 4) == 0) {
            break;
        } else if (strncmp(input_buffer, "play", 4) == 0) {
            ma_sound_start(&sound);
            printf("Waveform started.\n");
        } else if (strncmp(input_buffer, "stop", 4) == 0) {
            ma_sound_stop(&sound);
            printf("Waveform stopped.\n");
        } else if (strncmp(input_buffer, "type ", 5) == 0) {
            char* type_str = input_buffer + 5;
            if (strcmp(type_str, "sine") == 0) {
                ma_waveform_set_type(&waveform, ma_waveform_type_sine);
                printf("Type set to Sine\n");
            } else if (strcmp(type_str, "square") == 0) {
                ma_waveform_set_type(&waveform, ma_waveform_type_square);
                printf("Type set to Square\n");
            } else if (strcmp(type_str, "saw") == 0) {
                ma_waveform_set_type(&waveform, ma_waveform_type_sawtooth);
                printf("Type set to Sawtooth\n");
            } else {
                printf("Unknown type. Use: sine, square, saw\n");
            }
        } else if (strncmp(input_buffer, "freq ", 5) == 0) {
            double freq = atof(input_buffer + 5);
            /* Changing frequency on the source avoids resampling artifacts */
            ma_waveform_set_frequency(&waveform, freq);
            printf("Frequency set to %.1f Hz\n", freq);
        } else if (strncmp(input_buffer, "vol ", 4) == 0) {
            float vol = (float)atof(input_buffer + 4);
            ma_sound_set_volume(&sound, vol);
            printf("Volume set to %.2f\n", vol);
        } else if (strncmp(input_buffer, "pan ", 4) == 0) {
            float pan = (float)atof(input_buffer + 4);
            ma_sound_set_pan(&sound, pan);
            printf("Panning set to %.2f\n", pan);
        } else if (strlen(input_buffer) > 0) {
            printf("Commands: play, stop, type <sine|square|saw>, freq <hz>, vol <0-1>, pan <-1 to 1>, quit\n");
        }
    }

    /* 5. Cleanup */
    ma_sound_uninit(&sound);
    ma_engine_uninit(&engine);

    return 0;
}
