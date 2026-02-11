#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "ksynth.h"
#include "miniaudio.h"
#include "bestline.h"

#define MAX_VOICES 8  // Maximum simultaneous playback voices

typedef struct {
  K buffer;           // Audio buffer
  int idx;            // Current playback position
  int stereo;         // 0 = mono, 1 = stereo
  int active;         // 1 = playing, 0 = empty slot
} Voice;

volatile Voice voices[MAX_VOICES] = {0};

void cb(ma_device* d, void* o, const void* i, ma_uint32 n) {
  float* out = (float*)o;
  
  // Clear output buffer
  for (ma_uint32 j = 0; j < n * 2; j++) {
    out[j] = 0.0f;
  }
  
  // Mix all active voices
  for (int v = 0; v < MAX_VOICES; v++) {
    if (!voices[v].active) continue;
    
    K buf = voices[v].buffer;
    int idx = voices[v].idx;
    int stereo = voices[v].stereo;
    
    if (!buf) {
      voices[v].active = 0;
      continue;
    }
    
    for (ma_uint32 j = 0; j < n; j++) {
      if (idx >= buf->n) {
        // Voice finished
        voices[v].active = 0;
        break;
      }
      
      if (stereo && idx + 1 < buf->n) {
        // Stereo: read two samples
        out[j*2]   += (float)buf->f[idx++];  // L
        out[j*2+1] += (float)buf->f[idx++];  // R
      } else {
        // Mono: duplicate to both channels
        float v = (float)buf->f[idx++];
        out[j*2]   += v;  // L
        out[j*2+1] += v;  // R
      }
    }
    
    voices[v].idx = idx;
  }
}

int write_wav_from_k(char* name, double* ptr, ma_uint64 frames, ma_uint32 chans, ma_uint32 sample_rate);
void p_view(K x, int opts);

void handle_play(char *ptr) {
}

char get_var(char *ptr) {
  while (*ptr == ' ') ptr++;
  char v_name = *ptr;
  if (v_name >= 'A' && v_name <= 'Z') return v_name;
  return '\0';
}

int show = 0;
int opts = 0;

void handle_line(char* line) {
  while (*line == ' ') line++;

  if (line[0] == '\0' || line[0] == '/') return;

  // Strip inline comments (everything after first '/')
  // But be careful: '/' inside strings or functions should be preserved
  // Simple approach: find first '/' not inside {}
  char *comment_start = NULL;
  int brace_depth = 0;
  for (char *p = line; *p; p++) {
    if (*p == '{') brace_depth++;
    else if (*p == '}') brace_depth--;
    else if (*p == '/' && brace_depth == 0) {
      comment_start = p;
      break;
    }
  }
  if (comment_start) {
    *comment_start = '\0';  // Truncate at comment
  }

  if (line[0] == '\\') {
    
    if (line[1] == 't') { 
      show = (show == 0) ? 1 : 0;
      
    } else if (line[1] == 'p') {
      // \p X    - play mono (duplicate to both channels)
      // \ps X   - play stereo (interleaved L/R)
      char *arg = line + 2;
      while (*arg == ' ') arg++;
      
      int is_stereo = 0;
      if (*arg == 's') {
        is_stereo = 1;
        arg++;
      }
      
      char v_name = get_var(arg);
      if (v_name) {
        K v = vars[v_name - 'A'];
        if (v) {
          // Find empty voice slot
          int slot = -1;
          for (int i = 0; i < MAX_VOICES; i++) {
            if (!voices[i].active) {
              slot = i;
              break;
            }
          }
          
          if (slot == -1) {
            printf("No free voice slots (max %d)\n", MAX_VOICES);
            return;
          }
          
          // Free old buffer if any
          if (voices[slot].buffer) {
            k_free((K)voices[slot].buffer);
          }
          
          // Start new voice
          v->r++;
          voices[slot].buffer = v;
          voices[slot].idx = 0;
          voices[slot].stereo = is_stereo;
          voices[slot].active = 1;
          
          printf("playing %c in slot %d (%s)\n", v_name, slot, is_stereo ? "stereo" : "mono");
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

    } else if (line[1] == 's') {
      // \s X    - save mono
      // \ss X   - save stereo (interleaved L/R)
      char *arg = line + 2;
      while (*arg == ' ') arg++;
      
      int is_stereo = 0;
      if (*arg == 's') {
        is_stereo = 1;
        arg++;
      }
      
      char v_name = get_var(arg);
      if (v_name) {
        K v = vars[v_name - 'A'];
        if (v) {
          char name[1024];
          struct timeval tv;
          gettimeofday(&tv, NULL);
          double ts = (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
          
          ma_uint32 channels = is_stereo ? 2 : 1;
          ma_uint64 frames = is_stereo ? (v->n / 2) : v->n;
          
          snprintf(name, sizeof(name), "%c-%f.wav", v_name, ts);
          printf("write %c to %s (%s, %lld frames)\n", 
                 v_name, name, is_stereo ? "stereo" : "mono", frames);
          write_wav_from_k(name, v->f, frames, channels, 44100);
        } else {
          printf("nothing in %c\n", v_name);
        }
      }

    } else if (line[1] == 'v') {
      if (line[2] == '\0') {
        for (int v_name='A'; v_name<='Z'; v_name++) {
          K v = vars[v_name - 'A'];
          if (v) {
            printf("%c ", v_name);
            p_view(v, opts);
          }
        }
      } else {
        char v_name = get_var(line + 2);
        if (v_name) {
          K v = vars[v_name - 'A'];
          if (v) {
            printf("%c ", v_name);
            p_view(v, opts);
          }
        }
      }
      
    } else if (line[1] == 'x') {
      // \x - show playing voices
      printf("Active voices:\n");
      for (int i = 0; i < MAX_VOICES; i++) {
        if (voices[i].active && voices[i].buffer) {
          K buf = voices[i].buffer;
          int pct = (voices[i].idx * 100) / buf->n;
          printf("  [%d] %s %d/%d (%d%%)\n", 
                 i, 
                 voices[i].stereo ? "stereo" : "mono",
                 voices[i].idx,
                 buf->n,
                 pct);
        }
      }
      
    } else if (line[1] == 'q') {
      // \q - stop all playback
      for (int i = 0; i < MAX_VOICES; i++) {
        if (voices[i].buffer) {
          k_free((K)voices[i].buffer);
          voices[i].buffer = NULL;
        }
        voices[i].active = 0;
      }
      printf("Stopped all voices\n");
    }
  } else {
    char *ptr = line;
    K r = e(&ptr);
    if (show && r) {
      p_view(r, 1);
    }
  }
}

void p_view(K x, int opts) {
  if (!x || x->n <= 0) {
    printf("[0]\n");
    return;
  }

  printf("[%d] ", x->n);
  
  // Element Preview (First 10 or fewer)
  printf("(");
  int limit = (x->n < 10) ? x->n : 10;
  for (int i = 0; i < limit; i++) {
    printf("%.4f%s", x->f[i], (i == limit - 1) ? "" : " ");
  }
  if (x->n > 10) printf(" ...");
  printf(")\n");

  if (opts) return;

  // Sparkline (Braille)
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
  printf("\n");
}

#define CHUNK_FRAME_COUNT 4096

int write_wav_from_k(char* filename, double* ptr, ma_uint64 frames, ma_uint32 chans, ma_uint32 sample_rate) {
  ma_result result;
  ma_encoder encoder;
  ma_encoder_config config;

  config = ma_encoder_config_init(ma_encoding_format_wav, ma_format_f32, chans, sample_rate);

  result = ma_encoder_init_file(filename, &config, &encoder);
  if (result != MA_SUCCESS) {
    fprintf(stderr, "ma_encoder_init_file failed with error: %d\n", result);
    return -1;
  }

  // Fixed-size stack buffer
  float chunk_buffer[CHUNK_FRAME_COUNT * 2]; // Supporting up to stereo
  
  ma_uint64 frames_processed = 0;

  while (frames_processed < frames) {
    ma_uint64 frames_to_process = frames - frames_processed;
    if (frames_to_process > CHUNK_FRAME_COUNT) {
      frames_to_process = CHUNK_FRAME_COUNT;
    }

    // Convert double -> float into our stack-allocated chunk
    for (ma_uint64 i = 0; i < frames_to_process * chans; ++i) {
      chunk_buffer[i] = (float)ptr[frames_processed * chans + i];
    }

    // Write the chunk
    ma_uint64 frames_written;
    result = ma_encoder_write_pcm_frames(&encoder, chunk_buffer, frames_to_process, &frames_written);
    
    if (result != MA_SUCCESS) {
      printf("FAIL\n");
      break;
    }

    frames_processed += frames_written;
  }

  ma_encoder_uninit(&encoder);
  printf("frames_processed %lld\n", frames_processed);
  return (result == MA_SUCCESS) ? 0 : -1;
}

int main() {
  // Stereo output
  ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
  cfg.playback.format = ma_format_f32;
  cfg.playback.channels = 2;
  cfg.sampleRate = 44100;
  cfg.dataCallback = cb;
  ma_device dev;
  if (ma_device_init(NULL, &cfg, &dev) != MA_SUCCESS) return 1;
  ma_device_start(&dev);

  bestlineHistoryLoad("history.txt");
  printf("ksynth v2.1.0 (with functions, stereo, and multi-voice playback)\n");
  printf("exit \\l load | \\p[s] play | \\w wait | \\s[s] save | \\v view | \\t toggle\n");
  printf("new: \\x status | \\q stop all | up to %d simultaneous voices\n", MAX_VOICES);

  char* line;
  while ((line = bestline("> ")) != NULL) {
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
  
  // Cleanup voices
  for (int i = 0; i < MAX_VOICES; i++) {
    if (voices[i].buffer) {
      k_free((K)voices[i].buffer);
    }
  }
  
  ma_device_uninit(&dev);
  return 0;
}
