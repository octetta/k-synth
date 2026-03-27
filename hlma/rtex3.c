/*
 * main.c - Clean version without runtime wavetable switching glitches
 */

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "simple_audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE      48000
#define NUM_VOICES       8
#define WAVETABLE_SIZE   2048

static float sharedSine[WAVETABLE_SIZE];
static float sharedSaw[WAVETABLE_SIZE];
static float sharedSquare[WAVETABLE_SIZE];

static void generate_wavetable(float* buf, int type)
{
    for (int i = 0; i < WAVETABLE_SIZE; ++i) {
        float t = (float)i / WAVETABLE_SIZE;
        float s = 0.0f;
        if (type == 0)      s = sinf(2.0f * (float)M_PI * t);
        else if (type == 1) s = 2.0f * t - 1.0f;
        else                s = (t < 0.5f) ? 1.0f : -1.0f;

        buf[i] = s * 0.25f;
    }
}

static void make_stereo(const float* mono, float* stereo)
{
    for (int i = 0; i < WAVETABLE_SIZE; ++i) {
        stereo[i*2] = stereo[i*2+1] = mono[i];
    }
}

int main(void)
{
    SimpleAudioContext ctx;
    SimpleAudioVoice   voices[NUM_VOICES];
    float*             voiceBuffers[NUM_VOICES];

    SimpleAudioLFO     lfoVibrato;
    SimpleAudioLFO     lfoTremolo;

    generate_wavetable(sharedSine,   0);
    generate_wavetable(sharedSaw,    1);
    generate_wavetable(sharedSquare, 2);

    if (simple_audio_init(&ctx, 0.45f) != MA_SUCCESS) {
        printf("Failed to init audio\n");
        return -1;
    }

    printf("Creating %d looping voices...\n", NUM_VOICES);

    for (int i = 0; i < NUM_VOICES; ++i) {
        voiceBuffers[i] = (float*)malloc(WAVETABLE_SIZE * 2 * sizeof(float));
        const float* src = (i % 3 == 0) ? sharedSaw : sharedSine;
        make_stereo(src, voiceBuffers[i]);

        float baseFreq = 110.0f * powf(2.0f, (i % 12) / 12.0f);

        simple_audio_create_voice(&ctx, voiceBuffers[i], WAVETABLE_SIZE, 2,
                                  0.78f, MA_TRUE, baseFreq, &voices[i]);

        simple_audio_voice_set_pan(&voices[i], -0.85f + 1.7f * (i / (float)(NUM_VOICES-1)));
        simple_audio_voice_play(&voices[i]);
    }

    /* Stronger but still musical LFOs */
    simple_audio_lfo_init(&lfoVibrato, sharedSine, WAVETABLE_SIZE, 6.2f, 0.22f, LFO_TARGET_PITCH);   /* vibrato */
    simple_audio_lfo_init(&lfoTremolo, sharedSaw,  WAVETABLE_SIZE, 4.0f, 0.45f, LFO_TARGET_VOLUME);  /* tremolo */

    printf("\nDemo running - you should hear clear vibrato and tremolo.\n");
    printf("No more wavetable switching to avoid buzz.\n");
    printf("Press Enter to exit...\n");

    double t = 0.0;
    float deltaTime = 0.008f;

    while (1) {
        t += deltaTime;

        for (int i = 0; i < NUM_VOICES; ++i) {
            simple_audio_lfo_update(&lfoVibrato, &voices[i], deltaTime);
            simple_audio_lfo_update(&lfoTremolo, &voices[i], deltaTime);
        }

#ifdef _WIN32
        if (_kbhit()) break;
        Sleep(8);
#else
        usleep(8000);
#endif
    }

    simple_audio_lfo_deinit(&lfoVibrato);
    simple_audio_lfo_deinit(&lfoTremolo);

    for (int i = 0; i < NUM_VOICES; ++i) {
        simple_audio_voice_stop(&voices[i]);
        simple_audio_voice_uninit(&voices[i]);
        free(voiceBuffers[i]);
    }

    simple_audio_uninit(&ctx);
    printf("Clean shutdown.\n");
    return 0;
}
