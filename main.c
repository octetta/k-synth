#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ksynth.h"
#include "miniaudio.h" // No IMPLEMENTATION define here
#include "bestline.h"

volatile K play_ptr = NULL;
volatile int play_idx = 0;

void cb(ma_device* d, void* o, const void* i, ma_uint32 n) {
    float* out = (float*)o;
    K cur = (K)play_ptr;
    int idx = play_idx;
    for (ma_uint32 j = 0; j < n; j++) {
        out[j] = (cur && idx < cur->n) ? (float)cur->f[idx++] : 0.0f;
    }
    play_idx = idx;
}

void p_view(K x);

void handle_line(char* line) {
    while (*line == ' ') line++;
    if (line[0] == '\0' || line[0] == '/') return;

    if (line[0] == '\\') {
        if (line[1] == 'p') { 
            char *ptr = line + 2; while (*ptr == ' ') ptr++;
            char v_name = *ptr;
            if (v_name >= 'A' && v_name <= 'Z') {
                K v = vars[v_name - 'A'];
                if (v) {
                    K old = (K)play_ptr;
                    v->r++; play_idx = 0; play_ptr = v;
                    if (old) k_free(old);
                }
            }
        } else if (line[1] == 'l') { 
            char *fn = line + 2; while (*fn == ' ') fn++;
            FILE *f = fopen(fn, "r");
            if (!f) { printf("Error: %s\n", fn); return; }
            char buf[1024];
            while (fgets(buf, sizeof(buf), f)) {
                buf[strcspn(buf, "\n")] = 0;
                handle_line(buf);
            }
            fclose(f);
        } else if (line[1] == 'w') { 
            int ms = atoi(line + 2);
            if (ms > 0) usleep(ms * 1000);
        }
    } else {
        char *ptr = line;
        K r = e(&ptr);
        if (r) {
          p_view(r);
          p(r);
          k_free(r);
        }
    }
}

// Add this above handle_line in main.c
void p_view(K x) {
    if (!x || x->n <= 0) return;

    // 1. Sparkline (Braille)
    int width = 64; 
    printf("   ");
    for (int i = 0; i < width; i++) {
        int idx = (i * x->n) / width;
        double v = x->f[idx];
        // Map -1.0..1.0 to 0..7 for Braille dots
        int level = (int)((v + 1.0) * 3.5);
        if (level < 0) level = 0;
        if (level > 7) level = 7;
        static const char* dots[] = {" ","⠂","⠒","⠖","⠶","⠷","⠿","⣿"};
        printf("%s", dots[level]);
    }
    
    // 2. Element Preview (First 10 or fewer)
    printf("\n   (");
    int limit = (x->n < 10) ? x->n : 10;
    for (int i = 0; i < limit; i++) {
        printf("%.4f%s", x->f[i], (i == limit - 1) ? "" : " ");
    }
    if (x->n > 10) printf(" ...");
    printf(")\n");
}

int main() {
    ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
    cfg.playback.format = ma_format_f32;
    cfg.playback.channels = 1;
    cfg.sampleRate = 44100;
    cfg.dataCallback = cb;
    ma_device dev;
    if (ma_device_init(NULL, &cfg, &dev) != MA_SUCCESS) return 1;
    ma_device_start(&dev);

    bestlineHistoryLoad("history.txt");
    printf("ksynth v1.8.0 | \\l load | \\p play | \\w wait\n");

    char* line;
    while ((line = bestline("ksynth> ")) != NULL) {
        char *saveptr;
        char *token = strtok_r(line, "\n", &saveptr);
        while (token) {
          if (!strcmp(token, "exit")) { free(line); goto done; }
          if (token[0] != '\0') {
              bestlineHistoryAdd(token);
              handle_line(token);
              token = strtok_r(NULL, "\n", &saveptr);
          }
        }
        free(line);
    }
    done:
    bestlineHistorySave("history.txt");
    ma_device_uninit(&dev);
    return 0;
}
