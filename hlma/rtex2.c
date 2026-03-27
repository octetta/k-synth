/*
 * main.c - Clean wavetable synth example with shared tables + audible LFOs
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

/* Shared wavetables (mono, values roughly -1..1) */
static float sharedSine[WAVETABLE_SIZE];
static float sharedSaw[WAVETABLE_SIZE];
static float sharedSquare[WAVETABLE_SIZE];

/* Fill a mono wavetable */
static void generate_wavetable(float* buf, int type)
{
    for (int i = 0; i < WAVETABLE_SIZE; ++i) {
        float t = (float)i / WAVETABLE_SIZE;
        float s = 0.0f;
        if (type == 0) s = sinf(2.0f * (float)M_PI * t);           /* sine */
        else if (type == 1) s = 2.0f * t - 1.0f;                   /* saw */
        else s = (t < 0.5f) ? 1.0f : -1.0f;                        /* square */

        buf[i] = s * 0.25f;   /* safe amplitude */
    }
}

/* Convert mono table to interleaved stereo buffer for voice */
static void make_stereo_buffer(const float* mono, float* stereo)
{
    for (int i = 0; i < WAVETABLE_SIZE; ++i) {
        stereo[i*2 + 0] = mono[i];
        stereo[i*2 + 1] = mono[i];
    }
}

int main(void)
{
    SimpleAudioContext ctx;
    SimpleAudioVoice   voices[NUM_VOICES];
    float*             voiceBuffers[NUM_VOICES];   /* each voice still needs its own stereo buffer */

    SimpleAudioLFO     lfoVibrato;
    SimpleAudioLFO     lfoTremolo;

    /* Generate shared mono wavetables once */
    generate_wavetable(sharedSine,   0);
    generate_wavetable(sharedSaw,    1);
    generate_wavetable(sharedSquare, 2);

    /* Init engine with safe master gain */
    if (simple_audio_init(&ctx, 0.45f) != MA_SUCCESS) {
        printf("Failed to init audio engine\n");
        return -1;
    }

    printf("Creating %d looping voices using shared wavetables...\n", NUM_VOICES);

    /* Create voices */
    for (int i = 0; i < NUM_VOICES; ++i) {
        voiceBuffers[i] = (float*)malloc(WAVETABLE_SIZE * 2 * sizeof(float));

        /* Start with sine for all, then vary a bit */
        const float* sourceTable = (i % 3 == 0) ? sharedSaw : sharedSine;
        make_stereo_buffer(sourceTable, voiceBuffers[i]);

        float baseFreq = 110.0f * powf(2.0f, (i % 12) / 12.0f);

        simple_audio_create_voice(&ctx, voiceBuffers[i], WAVETABLE_SIZE, 2,
                                  0.78f, MA_TRUE, baseFreq, &voices[i]);

        simple_audio_voice_set_pan(&voices[i], -0.85f + 1.7f * (i / (float)(NUM_VOICES-1)));
        simple_audio_voice_play(&voices[i]);

        printf("Voice %d: base %.1f Hz\n", i, baseFreq);
    }

    /* LFOs - now with more audible depths */
    simple_audio_lfo_init(&lfoVibrato, sharedSine, WAVETABLE_SIZE, 6.2f, 0.18f, LFO_TARGET_PITCH);   /* vibrato */
    simple_audio_lfo_init(&lfoTremolo, sharedSaw,  WAVETABLE_SIZE, 3.8f, 0.40f, LFO_TARGET_VOLUME);  /* tremolo */

    printf("\n=== Demo running ===\n");
    printf("You should hear clear vibrato (pitch wobble) and tremolo (volume pulsing).\n");
    printf("Every ~5 seconds some voices switch wavetables.\n");
    printf("Press Enter to exit...\n");

    double t = 0.0;
    float deltaTime = 0.008f;   /* ~125 Hz update - smooth LFOs */

    while (1) {
        t += deltaTime;

        /* Update LFOs on all voices */
        for (int i = 0; i < NUM_VOICES; ++i) {
            simple_audio_lfo_update(&lfoVibrato, &voices[i], deltaTime);
            simple_audio_lfo_update(&lfoTremolo, &voices[i], deltaTime);
        }

        /* Occasional wavetable switch to demonstrate reuse + runtime change */
        if ((int)t % 5 == 0 && (int)(t * 10) % 10 == 0) {
            for (int i = 0; i < 3; ++i) {
                int idx = ((int)t + i) % NUM_VOICES;
                const float* newTable = (idx % 3 == 0) ? sharedSquare : sharedSaw;
                float* newStereo = voiceBuffers[idx];   /* reuse the same allocated stereo buffer */
                make_stereo_buffer(newTable, newStereo);
                simple_audio_voice_switch_wavetable(&voices[idx], newStereo, WAVETABLE_SIZE, voices[idx].base_frequency);
            }
        }

#ifdef _WIN32
        if (_kbhit()) break;
        Sleep(8);
#else
        usleep(8000);
#endif
    }

    /* Cleanup */
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
