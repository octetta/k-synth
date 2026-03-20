#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <wchar.h>
#include "ksynth.h"
#include "miniaudio.h"
#ifdef _WIN32
#else
#include "bestline.h"
#endif

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

void k_gnuplot(K x, const char *name, const char *path);

void usage(int f);

void handle_line(char* line) {
  while (*line == ' ') line++;

  if (line[0] == '\0' || line[0] == '/') return;

  int len = strlen(line);
  for (int i=0; i<len; i++) {
    if (line[i] == '/') {
      line[i] = '\0';
      break;
    }
  }

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
      
    } else if (line[1] == '?') {
      usage(1);
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
    } else if (line[1] == 'g') {

        char v_name = get_var(line + 2);
        if (v_name) {
          K v = vars[v_name - 'A'];
          if (v) {
            char s[2];
            sprintf(s, "%c", v_name);
            char n[16];
            sprintf(n, "%c.gnuplot", v_name);
            k_gnuplot(v, s, n);
            printf("/ %c.gnuplot\n", v_name);
          }
        }
    }
  } else {
    char *ptr = line;
    K r = e(&ptr);
    if (show && r) {
      p_view(r, 1);
    }
  }
}

void print_scope(double *data, int len, int width, int height);
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
  print_scope(x->f, x->n, 128, 64);
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

void usage(int f) {
  printf("ksynth v2.1.0 (with functions, stereo, and multi-voice playback)\n");
  if (f) {
    printf("exit \\l load | \\p[s] play | \\w wait | \\s[s] save | \\v view | \\t toggle\n");
    printf("\\g[s] gnuplot | \\i[s] s.i16 | \\f[s] s.f32\n");
    printf("\\x status | \\q stop all | up to %d simultaneous voices\n", MAX_VOICES);
  }
}

void doit(char *name) {
  FILE *fp;
  char line[1024];
  fp = fopen(name, "r");
  if (fp == NULL) return;
  while (fgets(line, sizeof(line), fp)) {
    char *token = line;
    handle_line(token);
  }
  fclose(fp);
}

void repl(int f) {
  bestlineHistoryLoad("history.txt");
  char* line;
  while ((line = bestline("> ")) != NULL) {
    char *saveptr;
    char *token = strtok_r(line, "\n", &saveptr);
    while (token) {
      if (!strcmp(token, "\\e")) { free(line); goto done; }
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
}

ma_device_config cfg;
ma_device dev;
int audio_start(void) {
  // Stereo output
  //ma_device_config cfg = ma_device_config_init(ma_device_type_playback);
  cfg = ma_device_config_init(ma_device_type_playback);
  cfg.playback.format = ma_format_f32;
  cfg.playback.channels = 2;
  cfg.sampleRate = 44100;
  cfg.dataCallback = cb;
  //ma_device dev;
  if (ma_device_init(NULL, &cfg, &dev) != MA_SUCCESS) return 1;
  ma_device_start(&dev);
  return 0;
}

int audio_end(void) {
  // Cleanup voices
  for (int i = 0; i < MAX_VOICES; i++) {
    if (voices[i].buffer) {
      k_free((K)voices[i].buffer);
    }
  }
  ma_device_uninit(&dev);
  return 0;
}

int main(int argc, char *argv[]) {
  usage(0);
  int graph = 0;
  int i16 = 0;
  int f32 = 0;
  audio_start();
  if (argc > 1) {
    char gs[] = "W.gnuplot";
    char is[] = "W.i16";
    char fs[] = "W.f32";
    for (int i=1; i<argc; i++) {
      if (argv[i][0] == '-') {
        char c = argv[i][1];
        switch (c) {
          case 'g': graph = (graph == 0) ? 1 : 0; break;
          case 'i': i16 = (i16 == 0) ? 1 : 0; break;
          case 'f': f32 = (f32 == 0) ? 1 : 0; break;
          case 't': show = (show == 0) ? 1 : 0; break;
        }
      } else {
        doit(argv[i]);
        K v = vars['W' - 'A'];
        if (v) {
          if (graph) k_gnuplot(v, "W", gs);
          if (i16) {
            FILE *f = fopen(is, "w");
            fprintf(f, "%d", (int16_t)(v->f[0] * 32767));
            for (int i=1; i<v->n; i++) {
              fprintf(f, ",%d", (int16_t)(v->f[i] * 32767));
            }
            fclose(f);
          }
          if (f32) {
            FILE *f = fopen(fs, "w");
            fprintf(f, "%g", v->f[0]);
            for (int i=1; i<v->n; i++) {
              fprintf(f, ",%g", v->f[i]);
            }
            fclose(f);
          }
        }
      }
    }
  } else {
    repl(0);
  }
  audio_end();
  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Universal "Safe" Colors
#define CLR_WAVE  "\x1b[38;5;34m"  
#define CLR_GRID  "\x1b[38;5;244m" 
#define CLR_CLIP  "\x1b[1;31m"     
#define CLR_TEXT  "\x1b[38;5;240m" 
#define CLR_RESET "\x1b[0m"

void print_scope_graph(double *data, int len, int width, int height) {
    const double min_y = -1.0;
    const double max_y = 1.0;
    int clipped = 0;

    int canvas_w = (width / 2) * 2;
    int canvas_h = (height / 4) * 4;
    
    unsigned char *grid = calloc(canvas_w * canvas_h, sizeof(unsigned char));
    if (!grid) return;

    // We iterate over every pixel column (x) to ensure no gaps
    for (int x = 0; x < canvas_w; x++) {
        // Calculate the position in the data array (floating point)
        double data_pos = (double)x / (canvas_w - 1) * (len - 1);
        int idx_low = (int)floor(data_pos);
        int idx_high = (int)ceil(data_pos);
        double fraction = data_pos - idx_low;

        // Linear interpolation between the two nearest data points
        double val = data[idx_low] * (1.0 - fraction) + data[idx_high] * fraction;

        if (fabs(val) >= 1.0) clipped = 1;

        // Clamp for drawing
        if (val < min_y) val = min_y;
        if (val > max_y) val = max_y;

        int y = (int)((max_y - val) / (max_y - min_y) * (canvas_h - 1));
        if (y >= 0 && y < canvas_h) {
            grid[y * canvas_w + x] = 1;
        }
    }

    // --- Header ---
    printf("\n  " CLR_TEXT "IDX: [0..%d]" CLR_RESET, len - 1);
    if (clipped) printf("%*s" CLR_CLIP "[ CLIP ]" CLR_RESET, (width/2) - 15, "");
    
    printf("\n  ┌" CLR_GRID);
    for(int i=0; i<width/2; i++) printf("─");
    printf(CLR_RESET "┐ " CLR_TEXT "+1.0" CLR_RESET "\n");

    // --- Render ---
    for (int y = 0; y < canvas_h; y += 4) {
        printf("  " CLR_GRID "│" CLR_RESET);
        for (int x = 0; x < canvas_w; x += 2) {
            unsigned int byte_offset = 0;
            int bit_map[8] = {1, 2, 4, 8, 16, 32, 64, 128};
            int dots[8][2] = {{0,0}, {1,0}, {2,0}, {0,1}, {1,1}, {2,1}, {3,0}, {3,1}};

            for (int i = 0; i < 8; i++) {
                int dy = y + dots[i][0];
                int dx = x + dots[i][1];
                if (dy < canvas_h && dx < canvas_w && grid[dy * canvas_w + dx]) {
                    byte_offset |= bit_map[i];
                }
            }

            if (byte_offset == 0) {
                int center_y = canvas_h / 2;
                if (y <= center_y && y + 4 > center_y) printf(CLR_GRID "‥" CLR_RESET);
                else printf(" ");
            } else {
                unsigned int code = 0x2800 + byte_offset;
                printf(CLR_WAVE "%c%c%c" CLR_RESET, (0xE0|(code>>12)), (0x80|((code>>6)&0x3F)), (0x80|(code&0x3F)));
            }
        }
        if (y == canvas_h / 2 - (canvas_h / 2 % 4)) printf(CLR_GRID "┤ " CLR_TEXT " 0.0" CLR_RESET "\n");
        else printf(CLR_GRID "│" CLR_RESET "\n");
    }

    printf("  └" CLR_GRID);
    for(int i=0; i<width/2; i++) printf("─");
    printf(CLR_RESET "┘ " CLR_TEXT "-1.0" CLR_RESET "\n\n");

    free(grid);
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>

/* Universal "Safe" Colors (8-bit ANSI)
   These are chosen to be visible on both black and white backgrounds. */
#define CLR_WAVE  "\x1b[38;5;34m"  /* Medium Green */
#define CLR_GRID  "\x1b[38;5;244m" /* Neutral Gray */
#define CLR_TEXT  "\x1b[38;5;240m" /* Darker Gray */
#define CLR_RESET "\x1b[0m"

/* Helper: Standard Bresenham's line algorithm to fill bits in the grid */
void draw_line(unsigned char *grid, int x1, int y1, int x2, int y2, int w, int h) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        if (x1 >= 0 && x1 < w && y1 >= 0 && y1 < h) {
            grid[y1 * w + x1] = 1;
        }
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

/**
 * print_scope
 * data:   Pointer to double array
 * len:    Number of elements in array
 * width:  Desired width in pixels (Braille chars use 2px width each)
 * height: Desired height in pixels (Braille chars use 4px height each)
 */
void print_scope(double *data, int len, int width, int height) {
    if (len < 2) return;

    // 1. Auto-Scale: Find the actual range of the data
    double min_y = DBL_MAX;
    double max_y = -DBL_MAX;
    for (int i = 0; i < len; i++) {
        if (data[i] < min_y) min_y = data[i];
        if (data[i] > max_y) max_y = data[i];
    }

    // Prevent collapse if data is flat
    if (max_y == min_y) { max_y += 0.1; min_y -= 0.1; }

    // Braille cells are 2x4. Ensure canvas dimensions are multiples.
    int canvas_w = (width / 2) * 2;
    int canvas_h = (height / 4) * 4;
    unsigned char *grid = calloc(canvas_w * canvas_h, sizeof(unsigned char));
    if (!grid) return;

    // 2. Plotting with Line Connection
    int prev_y = -1;
    for (int x = 0; x < canvas_w; x++) {
        // Map pixel x to data index with linear interpolation
        double data_pos = (double)x / (canvas_w - 1) * (len - 1);
        int idx_low = (int)floor(data_pos);
        int idx_high = (int)ceil(data_pos);
        double fraction = data_pos - idx_low;
        
        double val = data[idx_low] * (1.0 - fraction) + 
                     data[idx_high >= len ? len-1 : idx_high] * fraction;

        // Map value to pixel y (inverted for terminal: max_y at top)
        int y = (int)((max_y - val) / (max_y - min_y) * (canvas_h - 1));

        if (prev_y != -1) {
            draw_line(grid, x - 1, prev_y, x, y, canvas_w, canvas_h);
        } else {
            if (y >= 0 && y < canvas_h) grid[y * canvas_w + x] = 1;
        }
        prev_y = y;
    }

    // 3. Header: Show Max Value and Frame
    printf("\n  " CLR_TEXT "MAX: %-10.4f" CLR_RESET, max_y);
    printf("\n  ┌" CLR_GRID);
    for(int i = 0; i < canvas_w / 2; i++) printf("─");
    printf(CLR_RESET "┐\n");

    // 4. Braille Render Loop (Rows of 4px)
    for (int y = 0; y < canvas_h; y += 4) {
        // Calculate value range for this specific row to check for Zero-Crossing
        double row_val_top = max_y - ((double)y / (canvas_h - 1)) * (max_y - min_y);
        double row_val_bot = max_y - ((double)(y + 4) / (canvas_h - 1)) * (max_y - min_y);
        int has_zero = (row_val_top >= 0 && row_val_bot <= 0);

        // Left Margin (0 label)
        if (has_zero) printf(CLR_TEXT "0 " CLR_RESET);
        else printf("  ");

        printf(CLR_GRID "│" CLR_RESET);

        for (int x = 0; x < canvas_w; x += 2) {
            unsigned int byte_offset = 0;
            /* Braille bit-to-dot mapping:
               1  8
               2 16
               4 32
              64 128 (bottom row) */
            int bit_map[8] = {1, 2, 4, 8, 16, 32, 64, 128};
            int dots[8][2] = {{0,0}, {1,0}, {2,0}, {0,1}, {1,1}, {2,1}, {3,0}, {3,1}};

            for (int i = 0; i < 8; i++) {
                int dy = y + dots[i][0];
                int dx = x + dots[i][1];
                if (dy < canvas_h && dx < canvas_w && grid[dy * canvas_w + dx]) {
                    byte_offset |= bit_map[i];
                }
            }

            if (byte_offset == 0) {
                // Background graticule for zero line
                if (has_zero) printf(CLR_GRID "‥" CLR_RESET);
                else printf(" ");
            } else {
                // Encode to UTF-8 Braille block (U+2800 + offset)
                unsigned int code = 0x2800 + byte_offset;
                printf(CLR_WAVE "%c%c%c" CLR_RESET, 
                       (unsigned char)(0xE0 | (code >> 12)),
                       (unsigned char)(0x80 | ((code >> 6) & 0x3F)),
                       (unsigned char)(0x80 | (code & 0x3F)));
            }
        }

        // Right Margin (0 label)
        if (has_zero) printf(CLR_GRID "│" CLR_TEXT " 0" CLR_RESET "\n");
        else printf(CLR_GRID "│" CLR_RESET "\n");
    }

    // 5. Footer: Frame and Min Value
    printf("  └" CLR_GRID);
    for(int i = 0; i < canvas_w / 2; i++) printf("─");
    printf(CLR_RESET "┘\n  " CLR_TEXT "MIN: %-10.4f" CLR_RESET "\n\n", min_y);

    free(grid);
}
