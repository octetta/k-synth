#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "skred_ds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WT_SIZE 2048

int main() {
    ma_engine engine;

    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        printf("Failed to init engine.\n");
        return -1;
    }

    /* Generate a simple Sine Wavetable */
    float* p_wavetable = (float*)malloc(WT_SIZE * sizeof(float));
    for (int i = 0; i < WT_SIZE; i++) {
        p_wavetable[i] = (float)sin(((double)i / WT_SIZE) * 2.0 * 3.14159265358979323846);
    }

    /* * If the wavetable represents 1 cycle, its base_hz is Engine_SR / WT_SIZE.
     * e.g., at 48000Hz, a 2048 buffer is naturally ~23.43Hz.
     */
    float natural_hz = (float)engine.sampleRate / (float)WT_SIZE;

    skred_voice_t v0;
    skred_voice_init(engine.sampleRate, p_wavetable, WT_SIZE, engine.sampleRate, natural_hz, &v0);
    
    /* Wrap our custom source in a standard ma_sound so it plays through the engine */
    ma_sound sound;
    ma_sound_init_from_data_source(&engine, &v0, 0, NULL, &sound);
    ma_sound_start(&sound);

    v0.is_playing = 1;
    skred_voice_set_freq(&v0, 440.0f, 0.0f); /* Set starting freq */

    printf("Skred Universal Voice Engine Running.\n");
    printf("Commands:\n");
    printf("  freq <hz> <ease_ms>\n");
    printf("  vol <db> <ease_ms> (0 is max before offset, -96 is silence)\n");
    printf("  pan <val> <ease_ms> (-1.0 to 1.0)\n");
    printf("  dir <val> <ease_ms> (1.0 fwd, -1.0 rev, 0.0 stop)\n");
    printf("  mode <0|1|2> (0:oneshot, 1:forward, 2:pingpong)\n");
    printf("  quit\n\n");

    char cmd[32];
    float val1, val2;
    int mode_val;

    while (1) {
        printf("> ");
        if (scanf("%s", cmd) <= 0) break;
        if (strcmp(cmd, "quit") == 0) break;

        if (strcmp(cmd, "freq") == 0) {
            scanf("%f %f", &val1, &val2);
            skred_voice_set_freq(&v0, val1, val2);
        } else if (strcmp(cmd, "vol") == 0) {
            scanf("%f %f", &val1, &val2);
            skred_voice_set_vol(&v0, val1, val2);
        } else if (strcmp(cmd, "pan") == 0) {
            scanf("%f %f", &val1, &val2);
            skred_voice_set_pan(&v0, val1, val2);
        } else if (strcmp(cmd, "dir") == 0) {
            scanf("%f %f", &val1, &val2);
            skred_voice_set_dir(&v0, val1, val2);
        } else if (strcmp(cmd, "mode") == 0) {
            scanf("%d", &mode_val);
            if (mode_val >= 0 && mode_val <= 2) {
                skred_voice_set_loop(&v0, 0.0, (double)WT_SIZE - 1.0, (skred_loop_mode_t)mode_val);
                if (mode_val == 0) skred_voice_play(&v0); /* retrigger if oneshot */
            }
        }
    }

    ma_sound_uninit(&sound);
    ma_engine_uninit(&engine);
    free(p_wavetable);
    return 0;
}
