/*
 * real_time_wavetable_example.c
 * 
 * Shows 4 one-shot voices + 12 wavetable (wrapping) voices
 * with live frequency, volume, and pan control.
 */

#define MINIAUDIO_IMPLEMENTATION
#define SIMPLE_AUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "simple_audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE  48000
#define NUM_VOICES   16

typedef struct {
    SimpleAudioContext ctx;
    SimpleAudioVoice   voices[NUM_VOICES];
    float*             buffers[NUM_VOICES];
} AudioDemo;

ma_result generate_buffer(float frequency, ma_uint64* pFrames, float** ppBuffer, ma_bool32 isWavetable)
{
    ma_uint64 frames = isWavetable ? 2048 : (ma_uint64)(SAMPLE_RATE * 4.0); // short loop for wavetable, longer for one-shot
    float* buf = (float*)malloc(frames * 2 * sizeof(float));
    if (!buf) return MA_OUT_OF_MEMORY;

    for (ma_uint64 i = 0; i < frames; ++i) {
        float t = (float)i / SAMPLE_RATE;
        float sample = (float)sin(2.0 * M_PI * frequency * t) * 0.25f;

        buf[i*2]   = sample;
        buf[i*2+1] = sample;
    }

    *pFrames = frames;
    *ppBuffer = buf;
    return MA_SUCCESS;
}

int main(void)
{
    AudioDemo demo;
    memset(&demo, 0, sizeof(demo));

    simple_audio_init(&demo.ctx);

    printf("Creating 4 one-shot + 12 wavetable voices...\n");

    for (int i = 0; i < NUM_VOICES; ++i) {
        ma_bool32 isWavetable = (i >= 4);                 // first 4 = one-shot, rest = looping wavetable
        float baseFreq = 220.0f * powf(2.0f, (i % 12) / 12.0f);

        ma_uint64 frames;
        float* buf = NULL;
        generate_buffer(baseFreq, &frames, &buf, isWavetable);

        demo.buffers[i] = buf;

        simple_audio_create_voice(&demo.ctx, buf, frames, 2,
                                  0.7f, isWavetable, &demo.voices[i]);

        simple_audio_voice_set_pan(&demo.voices[i], -0.9f + 1.8f * (i / (float)(NUM_VOICES-1)));

        if (isWavetable) {
            simple_audio_voice_play(&demo.voices[i]);   // start looping immediately
        } else {
            simple_audio_voice_play(&demo.voices[i]);   // one-shot will play once
        }

        printf("Voice %2d: %s, base ≈ %.1f Hz\n", i, isWavetable ? "wavetable (looping)" : "one-shot", baseFreq);
    }

    printf("\n=== Real-time control demo running ===\n");
    printf("All voices are playing. Watch the console for live changes.\n");
    printf("Press Enter to stop...\n");

    /* Real-time control loop (60 Hz updates) */
    double t = 0.0;
    while (1) {
        t += 0.016;   // ~60 updates per second

        /* Example real-time modulation on the 12 wavetable voices */
        for (int i = 4; i < NUM_VOICES; ++i) {
            /* Sweep frequency around the base (like an LFO on pitch) */
            float freqMod = 220.0f * (1.0f + 0.8f * sinf((float)t * 3.0f + i));
            simple_audio_voice_set_frequency(&demo.voices[i], freqMod);

            /* Gentle volume breathing */
            float vol = 0.6f + 0.3f * sinf((float)t * 1.5f + i * 0.7f);
            simple_audio_voice_set_volume(&demo.voices[i], vol);

            /* Slow panning */
            float pan = sinf((float)t * 0.8f + i);
            simple_audio_voice_set_pan(&demo.voices[i], pan);
        }

#ifdef _WIN32
        Sleep(16);
#else
        usleep(16000);
#endif

        //if (kbhit()) break;   // press any key to exit (add #include <conio.h> on Windows)
        fgetc(stdin);
    }

    /* Cleanup */
    for (int i = 0; i < NUM_VOICES; ++i) {
        simple_audio_voice_stop(&demo.voices[i]);
        simple_audio_voice_uninit(&demo.voices[i]);
        free(demo.buffers[i]);
    }
    simple_audio_uninit(&demo.ctx);

    return 0;
}
