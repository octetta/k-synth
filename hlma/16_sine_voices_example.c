/*
 * 16_sine_voices_example.c
 * 
 * Demonstrates using the simple_audio wrapper to play 16 sine waves
 * simultaneously, each with independent frequency, volume, and pan.
 * 
 * Compile with:
 *   #define MINIAUDIO_IMPLEMENTATION
 *   #define SIMPLE_AUDIO_IMPLEMENTATION
 *   #include "miniaudio.h"
 *   #include "simple_audio.h"
 */
    #define MINIAUDIO_IMPLEMENTATION
    #define SIMPLE_AUDIO_IMPLEMENTATION
    #include "miniaudio.h"
    #include "simple_audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>   // for memset

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NUM_VOICES      16
#define SAMPLE_RATE     48000
#define DURATION_SEC    10.0f          // length of each sine buffer (can be short or long)
#define BASE_FREQ       220.0f         // starting frequency for voice 0

typedef struct {
    SimpleAudioContext ctx;
    SimpleAudioVoice   voices[NUM_VOICES];
    float*             buffers[NUM_VOICES];   // we own these float buffers
} AudioDemo;

ma_result generate_sine_buffer(float frequency, ma_uint64* pFrameCount, float** ppBuffer)
{
    ma_uint64 frames = (ma_uint64)(SAMPLE_RATE * DURATION_SEC);
    size_t size = frames * 2 * sizeof(float);   // stereo

    float* buffer = (float*)malloc(size);
    if (!buffer) return MA_OUT_OF_MEMORY;

    for (ma_uint64 i = 0; i < frames; ++i) {
        float t = (float)i / SAMPLE_RATE;
        float sample = (float)sin(2.0 * M_PI * frequency * t) * 0.3f;  // keep amplitude safe

        buffer[i*2 + 0] = sample;  // left
        buffer[i*2 + 1] = sample;  // right
    }

    *pFrameCount = frames;
    *ppBuffer = buffer;
    return MA_SUCCESS;
}

/* Helper to change frequency in real-time via pitch scaling */
void simple_audio_voice_set_frequency(SimpleAudioVoice* voice, float newFrequency)
{
    if (!voice || !voice->is_initialized) return;

    /* Get the original data format of the sound */
    ma_format format;
    ma_uint32 channels;
    ma_uint32 sampleRate;
    ma_sound_get_data_format(&voice->sound, &format, &channels, &sampleRate, NULL, 0);

    /* Pitch scale = desired_freq / original_freq */
    float pitch = newFrequency / BASE_FREQ;   // assuming all buffers were generated at BASE_FREQ
    ma_sound_set_pitch(&voice->sound, pitch);
}

int main(void)
{
    AudioDemo demo;
    memset(&demo, 0, sizeof(demo));

    /* 1. Initialize the audio engine */
    ma_result result = simple_audio_init(&demo.ctx);
    if (result != MA_SUCCESS) {
        printf("Failed to init audio engine: %d\n", result);
        return -1;
    }

    printf("Audio engine initialized. Creating %d sine voices...\n", NUM_VOICES);

    /* 2. Create 16 sine voices with different frequencies */
    for (int i = 0; i < NUM_VOICES; ++i) {
        float freq = BASE_FREQ * powf(2.0f, i / 12.0f);   // chromatic steps (roughly one octave total)

        ma_uint64 frames;
        float* buffer = NULL;

        result = generate_sine_buffer(freq, &frames, &buffer);
        if (result != MA_SUCCESS) {
            printf("Failed to generate buffer for voice %d\n", i);
            goto cleanup;
        }

        demo.buffers[i] = buffer;

        result = simple_audio_create_voice(&demo.ctx, buffer, frames, 2, 0.0f, &demo.voices[i]);
        if (result != MA_SUCCESS) {
            printf("Failed to create voice %d: %d\n", i, result);
            goto cleanup;
        }

        /* Initial settings */
        float volume = 0.6f / (float)(i + 1);          // quieter for higher voices
        float pan    = -0.8f + (1.6f * i / (NUM_VOICES-1));  // spread from left to right

        simple_audio_voice_set_volume(&demo.voices[i], volume);
        simple_audio_voice_set_pan(&demo.voices[i], pan);

        /* Start playing immediately (they loop by default with ma_audio_buffer) */
        simple_audio_voice_play(&demo.voices[i]);

        printf("Voice %2d: freq ≈ %.1f Hz, vol=%.2f, pan=%.2f\n", i, freq, volume, pan);
    }

    printf("\nAll 16 sine waves playing!\n");
    printf("You can now add real-time control in a loop, for example:\n");
    printf(" - Change volume/pan with simple_audio_voice_set_volume/set_pan\n");
    printf(" - Change frequency with simple_audio_voice_set_frequency\n");
    printf("Press Enter to stop and exit...\n");
    getchar();

cleanup:
    /* 3. Clean up */
    for (int i = 0; i < NUM_VOICES; ++i) {
        simple_audio_voice_stop(&demo.voices[i]);
        simple_audio_voice_uninit(&demo.voices[i]);
        free(demo.buffers[i]);          // we allocated these
    }

    simple_audio_uninit(&demo.ctx);
    printf("Cleanup complete.\n");
    return 0;
}
