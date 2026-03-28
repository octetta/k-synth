//#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "skred_ds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "uedit.h"

#define WT_SIZE 4096
#define VMAX 4
#define WMAX 4
#define WZED WMAX

#define PI (3.14159265358979323846)

int main() {
    ma_engine engine;
#if 1
    ma_engine_config engineConfig = ma_engine_config_init();
    engine.sampleRate = 44100;
    if (ma_engine_init(&engineConfig, &engine) != MA_SUCCESS) {
        printf("# Failed to init engine.\n");
        return -1;
    }
    ma_uint32 actual_sr = 44100;
#else
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        printf("# Failed to init engine.\n");
        return -1;
    }
    ma_uint32 actual_sr = ma_engine_get_sample_rate(&engine);
#endif

    printf("# hardware sample rate %u Hz\n", actual_sr);

    float *wd[WMAX+1];
    ma_uint32 ws[WMAX+1];
    //char wm[WMAX+1];

    /* Generate Simple Wavetables (0: Sine, 1: Square, 2: Saw) */
    for (int n = 0; n < 3; n++) {
        float* p_wavetable = (float*)malloc(WT_SIZE * sizeof(float));
        for (int i = 0; i < WT_SIZE; i++) {
            float y = 0.0;
            float t = (float)i / WT_SIZE;
            switch (n) {
                case 0: y = (float)sin(t * 2.0 * PI); break;
                case 1: y = (t < 0.5) ? 1.0 : -1.0; break;
                case 2: y = 2.0 * t - 1.0; break;
            }
            p_wavetable[i] = y;
        }
        wd[n] = p_wavetable;
        ws[n] = WT_SIZE;
        //wm[n] = skred_loop_forward_t;
    }

    // make hidden empty wavetable
    {
        float* p_wavetable = (float*)malloc(WT_SIZE * sizeof(float));
        for (int i = 0; i < WT_SIZE; i++) p_wavetable[i] = 0.0;
        wd[WMAX] = p_wavetable;
        ws[WMAX] = WT_SIZE;
        //wm[WMAX] = skred_loop_forward_t;
    }

    /* Generate Drum Sample for Slot 3 */
    #define DRUM 3
    ma_uint64 drum_frames = engine.sampleRate * 2;
    float* p_drum = (float*)calloc(drum_frames, sizeof(float));
    double p = 0.0;
    for (ma_uint64 i = 0; i < drum_frames; i++) {
        double t = (double)i / engine.sampleRate;
        p_drum[i] = (float)(sin(p * 2.0 * PI) * exp(-t * 5.0));
        p += (60.0 + (100.0 * exp(-t * 10.0))) / engine.sampleRate;
    }
    wd[DRUM] = p_drum;
    ws[DRUM] = drum_frames;
    //wm[DRUM] = skred_loop_oneshot_t;


#if 1
    float natural_hz = (float)((double)actual_sr / (double)WT_SIZE);
#else
    float natural_hz = (float)engine.sampleRate / (float)WT_SIZE;
#endif

    skred_voice_t v[VMAX];
    ma_sound sound[VMAX];
    int wave[VMAX] = {0};

    for (int i = 0; i < VMAX; i++) {
#if 1
        skred_voice_init(engine.sampleRate, wd[WZED], ws[WZED], actual_sr, natural_hz, &v[i]);
#else
        skred_voice_init(engine.sampleRate, wd[WZED], ws[WZED], engine.sampleRate, natural_hz, &v[i]);
#endif
        ma_sound_init_from_data_source(&engine, &v[i], 0, NULL, &sound[i]);
        
        /* Apply reasonable ADSR so notes don't click instantly */
        skred_voice_set_adsr(&v[i], 10.0f, 100.0f, 0.7f, 300.0f);
        wave[i] = WZED;
    }

    printf("Skred Universal Voice Engine Running.\n");
    printf("Commands:\n");
    printf("  v <id>              (Select active voice 0-%d)\n", VMAX-1);
    printf("  on / off            (Trigger ADSR Note On/Off)\n");
    printf("  wave <0-3>          (0:Sine, 1:Square, 2:Saw, 3:Drum)\n");
    printf("  freq <hz> <ms>      (Base frequency)\n");
    printf("  vol <db> <ms>       (Base volume)\n");
    printf("  pan <val> <ms>      (-1.0 left to 1.0 right)\n");
    printf("  dir <val>           (1.0 fwd, -1.0 rev)\n");
    printf("  adsr <a> <d> <s> <r>\n");
    printf("  lfo <hz> <f_dpth> <v_dpth> <p_dpth>\n");
    printf("  quit\n\n");

    char line[256];
    char cmd[32];
    float f1, f2, f3, f4;
    int tmp;
    int voice = 0;

    while (1) {
#if 1
        char ps[256];
        sprintf(ps, "v%d (State: %d) > ", voice, v[voice].adsr_state);
        int r = uedit(ps, line, sizeof(line));
        if (r < 0) break;
        if (r == 0) continue;
#else
        printf("v%d (State: %d) > ", voice, v[voice].adsr_state);
        if (!fgets(line, sizeof(line), stdin)) break;
#endif
        
        if (sscanf(line, "%31s", cmd) != 1) continue;
        if (strcmp(cmd, "quit") == 0) break;

        if (strcmp(cmd, "v") == 0 && sscanf(line, "%*s %d", &tmp) == 1) {
            if (tmp >= 0 && tmp < VMAX) voice = tmp;
        } 
        else if (strcmp(cmd, "on") == 0) {
            skred_voice_note_on(&v[voice]);
            ma_sound_start(&sound[voice]);
        } 
        else if (strcmp(cmd, "off") == 0) {
            skred_voice_note_off(&v[voice]);
        }
        else if (strcmp(cmd, "stop") == 0) {
            skred_voice_stop(&v[voice]);
        }
        else if (strcmp(cmd, "wave") == 0 && sscanf(line, "%*s %d", &tmp) == 1) {
            if (tmp >= 0 && tmp < WMAX) {
                if (tmp == wave[voice]) {
                  printf("# same wave, don't do anything\n");
                } else {
                  wave[voice] = tmp;
                  if (tmp == DRUM) skred_voice_set_sample(&v[voice], wd[tmp], ws[tmp], 1);
                  else skred_voice_set_sample(&v[voice], wd[tmp], ws[tmp], 0);
                }
            }
        } 
        else if (strcmp(cmd, "freq") == 0 && sscanf(line, "%*s %f %f", &f1, &f2) == 2) {
            skred_voice_set_freq(&v[voice], f1, f2);
        } 
        else if (strcmp(cmd, "vol") == 0 && sscanf(line, "%*s %f %f", &f1, &f2) == 2) {
            skred_voice_set_vol(&v[voice], f1, f2);
        } 
        else if (strcmp(cmd, "pan") == 0 && sscanf(line, "%*s %f %f", &f1, &f2) == 2) {
            skred_voice_set_pan(&v[voice], f1, f2);
        } 
        else if (strcmp(cmd, "dir") == 0 && sscanf(line, "%*s %f %f", &f1, &f2) == 2) {
            skred_voice_set_dir(&v[voice], f1, f2);
        } 
        else if (strcmp(cmd, "adsr") == 0 && sscanf(line, "%*s %f %f %f %f", &f1, &f2, &f3, &f4) == 4) {
            skred_voice_set_adsr(&v[voice], f1, f2, f3, f4);
        }
        else if (strcmp(cmd, "lfo") == 0 && sscanf(line, "%*s %f %f %f %f", &f1, &f2, &f3, &f4) == 4) {
            skred_voice_set_lfo(&v[voice], f1, f2, f3, f4);
        }
    }

    for (int i=0; i<VMAX; i++) ma_sound_uninit(&sound[i]);
    ma_engine_uninit(&engine);
    for (int i=0; i<WMAX; i++) free(wd[i]);
    
    return 0;
}
