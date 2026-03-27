//#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "skred_ds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WT_SIZE 4096

#define VMAX 4
#define WMAX 4

int main() {
    ma_engine engine;

    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        printf("Failed to init engine.\n");
        return -1;
    }

    float *wd[WMAX];
    ma_uint32 ws[WMAX];

    /* Generate a simple Sine Wavetable */
#define TAU (2.0 * 3.14159265358979323846)
    float *p_wavetable;
    for (int n = 0; n < WMAX; n++) {
      p_wavetable = (float*)malloc(WT_SIZE * sizeof(float));
      for (int i = 0; i < WT_SIZE; i++) {
          float y = 0.0;
          float t = (float)i / WT_SIZE;
          switch (n) {
            default:
            case 0: // sine
              y = (float)sin(((double)i / WT_SIZE) * 2.0 * 3.14159265358979323846);
              //y = (float)sin(((double)i / WT_SIZE) * TAU);
              break;
            case 1: // square
              y = (t < 0.5) ? 1.0 : -1.0;
              break;
            case 2: // saw
              y = 2.0 * t - 1.0;
              break;
          }
          p_wavetable[i] = y;
      }
      wd[n] = p_wavetable;
      ws[n] = WT_SIZE;
    }

    /* * If the wavetable represents 1 cycle, its base_hz is Engine_SR / WT_SIZE.
     * e.g., at 48000Hz, a 2048 buffer is naturally ~23.43Hz.
     */
    float natural_hz = (float)engine.sampleRate / (float)WT_SIZE;

    skred_voice_t v[VMAX];
    ma_sound sound[VMAX];
    for (int i=0; i<VMAX; i++) {
      skred_voice_init(engine.sampleRate, p_wavetable, WT_SIZE, engine.sampleRate, natural_hz, &v[i]);
      /* Wrap our custom source in a standard ma_sound so it plays through the engine */
      ma_sound_init_from_data_source(&engine, &v[i], 0, NULL, &sound[i]);

      v[i].is_playing = 1;
      skred_voice_set_freq(&v[i], 440.0f, 0.0f); /* Set starting freq */
      //ma_sound_start(&sound[i]);
    }

    printf("Skred Universal Voice Engine Running.\n");
    printf("Commands:\n");
    printf("  v <voice>\n");
    printf("  play\n");
    printf("  stop\n");
    printf("  wave <wave index>\n");
    printf("  freq <hz> <ease_ms>\n");
    printf("  vol <db> <ease_ms> (0 is max before offset, -96 is silence)\n");
    printf("  pan <val> <ease_ms> (-1.0 to 1.0)\n");
    printf("  dir <val> <ease_ms> (1.0 fwd, -1.0 rev, 0.0 stop)\n");
    printf("  mode <0|1|2> (0:oneshot, 1:forward, 2:pingpong)\n");
    printf("  quit\n\n");

    char cmd[32];
    float val1, val2;
    int mode_val;
    int voice = 0;
    int tmp;

    while (1) {
        printf("v%d/%d> ", voice, v[0].is_playing);
        if (scanf("%s", cmd) <= 0) break;
        if (strcmp(cmd, "quit") == 0) break;

        if (strcmp(cmd, "v") == 0) {
            scanf("%d", &tmp);
            if (tmp >= 0 && tmp < VMAX) voice = tmp;
        } else if (strcmp(cmd, "wave") == 0) {
            scanf("%d", &tmp);
            if (tmp >= 0 && tmp < WMAX) {
              skred_voice_set_buffer(&v[voice], wd[tmp], ws[tmp]);
            }
        } else if (strcmp(cmd, "stop") == 0) {
            skred_voice_stop(&v[voice]);
        } else if (strcmp(cmd, "play") == 0) {
            skred_voice_play(&v[voice]);
            ma_sound_start(&sound[voice]);
        } else if (strcmp(cmd, "freq") == 0) {
            scanf("%f %f", &val1, &val2);
            skred_voice_set_freq(&v[voice], val1, val2);
        } else if (strcmp(cmd, "vol") == 0) {
            scanf("%f %f", &val1, &val2);
            skred_voice_set_vol(&v[voice], val1, val2);
        } else if (strcmp(cmd, "pan") == 0) {
            scanf("%f %f", &val1, &val2);
            skred_voice_set_pan(&v[voice], val1, val2);
        } else if (strcmp(cmd, "dir") == 0) {
            scanf("%f %f", &val1, &val2);
            skred_voice_set_dir(&v[voice], val1, val2);
        } else if (strcmp(cmd, "mode") == 0) {
            scanf("%d", &mode_val);
            if (mode_val >= 0 && mode_val <= 2) {
                skred_voice_set_loop(&v[voice], 0.0, (double)WT_SIZE - 1.0, (skred_loop_mode_t)mode_val);
                if (mode_val == 0) skred_voice_play(&v[voice]); /* retrigger if oneshot */
            }
        }
    }

    for (int i=0; i<VMAX; i++) {
      ma_sound_uninit(&sound[i]);
    }
    ma_engine_uninit(&engine);
    for (int i=0; i<VMAX; i++) {
      free(wd[i]);
    }
    return 0;
}
