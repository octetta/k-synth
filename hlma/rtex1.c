/*
 * main.c - Complete working example using simple_audio.h
 * 
 * Features demonstrated:
 *   - 8 looping wavetable voices (sine/saw/square)
 *   - Independent real-time control: volume, pan, frequency
 *   - Phase reset on some voices
 *   - Two LFOs using arbitrary wavetables (one for vibrato, one for tremolo)
 *   - Conservative master gain to avoid clipping/distortion
 * 
 * Compile command (one-liner):
 *   gcc main.c simple_audio.c -o synth_demo -lm -ldl -lpthread   (Linux/macOS)
 *   cl main.c simple_audio.c miniaudio.c /Fe:synth_demo.exe      (Windows, with miniaudio.c in same folder)
 */

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "simple_audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE      48000
#define NUM_VOICES       8
#define WAVETABLE_SIZE   2048   /* power of 2 is nice but not required */

static void generate_wavetable(float* buf, int type)  /* mono, values ~ -1..1 */
{
        printf("%d : ");
        switch(type) {
          case 0: puts("sin"); break;
          case 1: puts("saw"); break;
          case 2: puts("sqr"); break;
          default: puts("???"); break;
        }

    for (int i = 0; i < WAVETABLE_SIZE; ++i) {
        float t = (float)i / WAVETABLE_SIZE;
        float s = 0.0f;

        if (type == 0) {                    /* sine */
            s = sinf(2.0f * (float)M_PI * t);
        } else if (type == 1) {             /* saw */
            s = 2.0f * t - 1.0f;
        } else {                            /* square */
            s = (t < 0.5f) ? 1.0f : -1.0f;
        }

        buf[i] = s * 0.22f;   /* safe amplitude per voice */
    }
}

int main(void)
{
    SimpleAudioContext ctx;
    SimpleAudioVoice   voices[NUM_VOICES];
    float*             voiceBuffers[NUM_VOICES];

    SimpleAudioLFO     lfoVibrato;
    SimpleAudioLFO     lfoTremolo;
    float*             lfoSine  = (float*)malloc(WAVETABLE_SIZE * sizeof(float));
    float*             lfoSaw   = (float*)malloc(WAVETABLE_SIZE * sizeof(float));

    /* Generate LFO wavetables */
    generate_wavetable(lfoSine, 0);   /* sine for smooth vibrato */
    generate_wavetable(lfoSaw,  1);   /* saw for more aggressive tremolo */

    /* Initialize engine with conservative master gain to prevent clipping */
    ma_result result = simple_audio_init(&ctx, 0.48f);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine\n");
        return -1;
    }

    printf("Creating %d looping wavetable voices...\n", NUM_VOICES);

    /* Create voices */
    for (int i = 0; i < NUM_VOICES; ++i) {
        voiceBuffers[i] = (float*)malloc(WAVETABLE_SIZE * 2 * sizeof(float));

        /* Generate stereo wavetable for the voice (duplicate mono to both channels) */
        float monoTable[WAVETABLE_SIZE];
        generate_wavetable(monoTable, i % 3);   /* cycle sine / saw / square */

        for (int s = 0; s < WAVETABLE_SIZE; ++s) {
            voiceBuffers[i][s*2 + 0] = monoTable[s];
            voiceBuffers[i][s*2 + 1] = monoTable[s];
        }

        float baseFreq = 110.0f * powf(2.0f, (i % 12) / 12.0f);

        result = simple_audio_create_voice(&ctx,
                                           voiceBuffers[i],
                                           WAVETABLE_SIZE,
                                           2,
                                           0.75f,
                                           MA_TRUE,          /* looping = true (wavetable mode) */
                                           baseFreq,
                                           &voices[i]);

        if (result != MA_SUCCESS) {
            printf("Failed to create voice %d\n", i);
        }

        simple_audio_voice_set_pan(&voices[i], -0.8f + 1.6f * (i / (float)(NUM_VOICES - 1)));
        simple_audio_voice_play(&voices[i]);

        printf("Voice %d: base freq %.1f Hz, type %d\n", i, baseFreq, i % 3);
    }

    /* Initialize LFOs */
    simple_audio_lfo_init(&lfoVibrato, lfoSine, WAVETABLE_SIZE, 5.5f, 0.08f, LFO_TARGET_PITCH);   /* gentle vibrato */
    simple_audio_lfo_init(&lfoTremolo, lfoSaw,  WAVETABLE_SIZE, 4.2f, 0.25f, LFO_TARGET_VOLUME);  /* light tremolo */

    printf("\n=== Synth demo running ===\n");
    printf("8 looping wavetable voices + LFO modulation (vibrato + tremolo)\n");
    printf("Master gain is set low to reduce distortion.\n");
    printf("Press Enter to stop and exit...\n");

    double t = 0.0;
    float deltaTime = 0.008f;   /* ~125 Hz update rate - smooth enough for LFOs */

    while (1) {
        t += deltaTime;

        /* Update LFOs on every voice */
        for (int i = 0; i < NUM_VOICES; ++i) {
            simple_audio_lfo_update(&lfoVibrato, &voices[i], deltaTime);
            simple_audio_lfo_update(&lfoTremolo, &voices[i], deltaTime);

            /* Optional extra slow pan movement on top of LFO */
            float extraPan = 0.2f * sinf((float)t * 0.6f + i);
            simple_audio_voice_set_pan(&voices[i], voices[i].base_pan + extraPan);
        }

        /* Occasional phase reset on a few voices (creates rhythmic feel) */
        if ((int)(t * 2) % 12 == 0) {
            for (int i = 0; i < 3; ++i) {
                int idx = (int)t % NUM_VOICES;
                simple_audio_voice_reset_phase(&voices[idx]);
            }
        }

        /* Simple non-blocking input check */
#ifdef _WIN32
        if (_kbhit()) break;
        Sleep(8);
#else
        usleep(8000);
        /* On Linux/macOS you can add a non-blocking key check if desired */
        if (getchar() != EOF) break;   /* fallback - will block until key */
#endif
    }

    /* Cleanup */
    simple_audio_lfo_deinit(&lfoVibrato);
    simple_audio_lfo_deinit(&lfoTremolo);
    free(lfoSine);
    free(lfoSaw);

    for (int i = 0; i < NUM_VOICES; ++i) {
        simple_audio_voice_stop(&voices[i]);
        simple_audio_voice_uninit(&voices[i]);
        free(voiceBuffers[i]);
    }

    simple_audio_uninit(&ctx);

    printf("Demo finished. Clean shutdown.\n");
    return 0;
}
