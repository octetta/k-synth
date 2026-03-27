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
        if (type == 0) s = sinf(2.0f * (float)M_PI * t);
        else if (type == 1) s = 2.0f * t - 1.0f;
        else s = (t < 0.5f) ? 1.0f : -1.0f;
        buf[i] = s * 0.25f;
    }
}

int main(void)
{
    SimpleAudioContext ctx;
    SimpleWavetableVoice voices[NUM_VOICES];
    SimpleAudioLFO lfoVibrato, lfoTremolo;

    generate_wavetable(sharedSine,   0);
    generate_wavetable(sharedSaw,    1);
    generate_wavetable(sharedSquare, 2);

    if (simple_audio_init(&ctx, 0.42f) != MA_SUCCESS) {
        printf("Failed to init audio engine\n");
        return -1;
    }

    printf("Creating %d wavetable voices...\n", NUM_VOICES);

    for (int i = 0; i < NUM_VOICES; ++i) {
        const float* startTable = (i % 3 == 0) ? sharedSaw : sharedSine;
        float baseFreq = 110.0f * powf(2.0f, (i % 12) / 12.0f);

        simple_wavetable_create(&ctx, startTable, WAVETABLE_SIZE, 0.78f, baseFreq, &voices[i]);
        simple_wavetable_set_pan(&voices[i], -0.85f + 1.7f * (i / (float)(NUM_VOICES-1)));
        simple_wavetable_play(&voices[i]);
    }

    /* Slow continuous tremolo + clear vibrato */
    simple_audio_lfo_init(&lfoVibrato, sharedSine,   WAVETABLE_SIZE, 6.5f, 0.40f, LFO_TARGET_PITCH);
    simple_audio_lfo_init(&lfoTremolo, sharedSaw,    WAVETABLE_SIZE, 2.5f, 0.80f, LFO_TARGET_VOLUME);  /* slow repeating tremolo */

    printf("\n=== Running ===\n");
    printf("You should hear clear vibrato (pitch wobble) and slow repeating tremolo (volume pulsing up and down regularly).\n");
    printf("Press Enter to exit...\n");

    double t = 0.0;
    float deltaTime = 0.004f;

    while (1) {
        t += deltaTime;

        for (int i = 0; i < NUM_VOICES; ++i) {
            simple_audio_lfo_update(&lfoVibrato, &voices[i], deltaTime);
            simple_audio_lfo_update(&lfoTremolo, &voices[i], deltaTime);
        }

#ifdef _WIN32
        if (_kbhit()) break;
        Sleep(4);
#else
        usleep(4000);
#endif
    }

    simple_audio_lfo_deinit(&lfoVibrato);
    simple_audio_lfo_deinit(&lfoTremolo);

    for (int i = 0; i < NUM_VOICES; ++i) {
        simple_wavetable_uninit(&voices[i]);
    }
    simple_audio_uninit(&ctx);

    printf("Clean shutdown.\n");
    return 0;
}
