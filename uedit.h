#ifndef UEDIT_H
#define UEDIT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <errno.h>
    #include <sys/ioctl.h>
    #include <sys/select.h>
    #include <termios.h>
    #include <unistd.h>
#endif

#define UEDIT_MAX_LINE 1024
#define CTRL_KEY(k) ((k) & 0x1f)
#define UEDIT_EVENT (-2)
#define UEDIT_KEY_LEFT (-10)
#define UEDIT_KEY_RIGHT (-11)
#define UEDIT_KEY_UP (-12)
#define UEDIT_KEY_DOWN (-13)
#if defined(__GNUC__) || defined(__clang__)
#define UEDIT_UNUSED __attribute__((unused))
#else
#define UEDIT_UNUSED
#endif

#ifdef _WIN32
static int uedit_getch(void) {
    int ch = _getch();
    if (ch == 0 || ch == 224) {
        ch = _getch();
        switch (ch) {
            case 75: return UEDIT_KEY_LEFT;
            case 77: return UEDIT_KEY_RIGHT;
            case 72: return UEDIT_KEY_UP;
            case 80: return UEDIT_KEY_DOWN;
            default: return EOF;
        }
    }
    return ch;
}
#else
static int uedit_getch(void) {
    unsigned char ch;
    ssize_t n = read(STDIN_FILENO, &ch, 1);
    return n == 1 ? (int)ch : EOF;
}
#endif
#define GETCH() uedit_getch()

typedef void (*uedit_event_cb)(void *user);

/* History State */
static char **uedit_hist = NULL;
static int uedit_h_max = 100;
static int uedit_h_cnt = 0;
static int uedit_h_head = 0;
static char uedit_h_tmp[UEDIT_MAX_LINE] = {0}; /* Saves current line during browsing */

#ifndef _WIN32
static struct termios uedit_orig_termios;
static int uedit_raw_enabled = 0;
static int uedit_atexit_registered = 0;
static void uedit_disable_raw_mode(void) {
    if (!uedit_raw_enabled) return;
    tcsetattr(STDIN_FILENO, TCSANOW, &uedit_orig_termios);
    uedit_raw_enabled = 0;
}
static int uedit_enable_raw_mode(void) {
    if (!isatty(STDIN_FILENO)) return -1;
    if (tcgetattr(STDIN_FILENO, &uedit_orig_termios) != 0) return -1;
    if (!uedit_atexit_registered) {
        atexit(uedit_disable_raw_mode);
        uedit_atexit_registered = 1;
    }
    struct termios raw = uedit_orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) return -1;
    uedit_raw_enabled = 1;
    return 0;
}
#else
static DWORD uedit_orig_in_mode, uedit_orig_out_mode;
static int uedit_raw_enabled = 0;
static int uedit_atexit_registered = 0;
static void uedit_disable_raw_mode(void) {
    if (!uedit_raw_enabled) return;
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),  uedit_orig_in_mode);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), uedit_orig_out_mode);
    uedit_raw_enabled = 0;
}
static int uedit_enable_raw_mode(void) {
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE), hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE || hOut == INVALID_HANDLE_VALUE) return -1;
    if (!GetConsoleMode(hIn, &uedit_orig_in_mode) ||
        !GetConsoleMode(hOut, &uedit_orig_out_mode)) return -1;
    if (!uedit_atexit_registered) {
        atexit(uedit_disable_raw_mode);
        uedit_atexit_registered = 1;
    }
    SetConsoleMode(hIn, uedit_orig_in_mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));
    SetConsoleMode(hOut, uedit_orig_out_mode | 0x0004);
    uedit_raw_enabled = 1;
    return 0;
}
#endif

/* Sets history size. If called after init, it clears existing history. */
static void uedit_config_history(int n) {
    if (uedit_hist) {
        for (int i = 0; i < uedit_h_max; i++) free(uedit_hist[i]);
        free(uedit_hist);
    }
    uedit_h_max = n;
    uedit_hist = (char **)calloc(uedit_h_max, sizeof(char *));
    uedit_h_cnt = uedit_h_head = 0;
}

static void uedit_add_history(const char *line) {
    if (!uedit_hist) uedit_config_history(uedit_h_max);
    if (!line || !line[0]) return;
    /* Avoid duplicates at the end of history */
    int last = (uedit_h_head + uedit_h_max - 1) % uedit_h_max;
    if (uedit_h_cnt > 0 && strcmp(uedit_hist[last], line) == 0) return;

    if (!uedit_hist[uedit_h_head]) uedit_hist[uedit_h_head] = malloc(UEDIT_MAX_LINE);
    if (!uedit_hist[uedit_h_head]) return;
    strncpy(uedit_hist[uedit_h_head], line, UEDIT_MAX_LINE - 1);
    uedit_hist[uedit_h_head][UEDIT_MAX_LINE - 1] = '\0';
    uedit_h_head = (uedit_h_head + 1) % uedit_h_max;
    if (uedit_h_cnt < uedit_h_max) uedit_h_cnt++;
}

static void uedit_cursor_visible(int visible) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleCursorInfo(hOut, &info)) {
        info.bVisible = visible ? TRUE : FALSE;
        SetConsoleCursorInfo(hOut, &info);
        return;
    }
#endif
    printf("%s", visible ? "\033[?25h" : "\033[?25l");
}

static int uedit_term_width(void) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        return csbi.dwSize.X;
    return 80;
#else
    struct winsize ws;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
        return ws.ws_col;
    return 80;
#endif
}

/* Renders a width-limited, horizontally-scrolled window of buf centered
 * around the cursor, so lines wider than the terminal never wrap onto
 * multiple rows (which this single-row refresh logic can't track). */
static void uedit_refresh_line(const char *prompt, const char *buf, int cur) {
    int width = uedit_term_width();
    int plen = (int)strlen(prompt);
    int avail = width - plen - 1; /* leave 1 col margin to avoid edge wrap */
    if (avail < 1) avail = 1;

    int len = (int)strlen(buf);
    int start = 0;
    if (cur > avail) start = cur - avail;
    if (start > len) start = len;
    int show = len - start;
    if (show > avail) show = avail;

    uedit_cursor_visible(0);
    printf("\r\033[K%s%.*s\r", prompt, show, buf + start);
    int col = plen + (cur - start);
    if (col > 0) printf("\033[%dC", col);
    uedit_cursor_visible(1);
    fflush(stdout);
}

static int uedit_getch_event(int event_fd, void *event_handle) {
#ifdef _WIN32
    if (event_handle) {
        HANDLE handles[2];
        handles[0] = GetStdHandle(STD_INPUT_HANDLE);
        handles[1] = (HANDLE)event_handle;
        DWORD result = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
        if (result == WAIT_OBJECT_0 + 1) return UEDIT_EVENT;
        if (result != WAIT_OBJECT_0) return EOF;
    }
    return GETCH();
#else
    (void)event_handle;
    if (event_fd < 0) return GETCH();
    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(event_fd, &readfds);
        int maxfd = event_fd > STDIN_FILENO ? event_fd : STDIN_FILENO;
        int ready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (ready < 0) {
            if (errno == EINTR) continue;
            return EOF;
        }
        if (FD_ISSET(event_fd, &readfds)) return UEDIT_EVENT;
        if (FD_ISSET(STDIN_FILENO, &readfds)) return GETCH();
    }
#endif
}

static int uedit_with_event(const char *prompt, char *buf, int max_line,
        int event_fd, void *event_handle, uedit_event_cb on_event,
        void *event_user) {
    int len = 0, cur = 0, c, h_idx = -1;
    if (max_line <= 0) return -1;
    memset(buf, 0, max_line);
    printf("%s", prompt); fflush(stdout);
    if (uedit_enable_raw_mode() != 0) {
        if (!fgets(buf, max_line, stdin)) return -1;
        len = (int)strlen(buf);
        if (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
            buf[--len] = '\0';
        uedit_add_history(buf);
        return len;
    }

    while (1) {
        c = uedit_getch_event(event_fd, event_handle);
        if (c == UEDIT_EVENT) {
            if (on_event) on_event(event_user);
            uedit_refresh_line(prompt, buf, cur);
            continue;
        }
        if (c == EOF) { len = -1; break; }
        if      (c == CTRL_KEY('a')) cur = 0;
        else if (c == CTRL_KEY('e')) cur = len;
        else if (c == CTRL_KEY('u')) { buf[0] = '\0'; len = cur = 0; }
        else if (c == CTRL_KEY('k')) { buf[cur] = '\0'; len = cur; }
        else if (c == CTRL_KEY('l')) { printf("\033[H\033[2J"); }
        else if (c == '\n' || c == '\r') {
            buf[len] = '\0'; putchar('\n');
            uedit_add_history(buf);
            break;
        } else if (c == 127 || c == 8) {
            if (cur > 0) {
                memmove(&buf[cur - 1], &buf[cur], len - cur);
                len--; cur--; buf[len] = '\0';
            }
        } else if (c == UEDIT_KEY_LEFT) {
            if (cur > 0) cur--;
        } else if (c == UEDIT_KEY_RIGHT) {
            if (cur < len) cur++;
        } else if (c == UEDIT_KEY_UP || c == UEDIT_KEY_DOWN) {
            if (h_idx == -1) strncpy(uedit_h_tmp, buf, UEDIT_MAX_LINE - 1);
            if (c == UEDIT_KEY_UP) { if (h_idx < uedit_h_cnt - 1) h_idx++; }
            else { if (h_idx > -1) h_idx--; }

            if (h_idx == -1) strncpy(buf, uedit_h_tmp, max_line - 1);
            else {
                int pos = (uedit_h_head - 1 - h_idx + uedit_h_max) % uedit_h_max;
                strncpy(buf, uedit_hist[pos], max_line - 1);
            }
            buf[max_line - 1] = '\0';
            len = (int)strlen(buf); cur = len;
        } else if (c == 27) {
            c = GETCH();
            if (c == '[') {
                c = GETCH();
                if (c == 'D' && cur > 0) cur--;
                else if (c == 'C' && cur < len) cur++;
                else if (c == 'A' || c == 'B') { /* Up / Down */
                    if (h_idx == -1) strncpy(uedit_h_tmp, buf, UEDIT_MAX_LINE - 1);
                    if (c == 'A') { if (h_idx < uedit_h_cnt - 1) h_idx++; }
                    else { if (h_idx > -1) h_idx--; }

                    if (h_idx == -1) strncpy(buf, uedit_h_tmp, max_line - 1);
                    else {
                        int pos = (uedit_h_head - 1 - h_idx + uedit_h_max) % uedit_h_max;
                        strncpy(buf, uedit_hist[pos], max_line - 1);
                    }
                    buf[max_line - 1] = '\0';
                    len = (int)strlen(buf); cur = len;
                }
            }
        } else if (c == CTRL_KEY('d')) {
            if (len > 0 && cur < len) {
                memmove(&buf[cur], &buf[cur + 1], len - cur - 1);
                len--; buf[len] = '\0';
            } else if (len == 0) { len = -1; break; }
        } else if (c >= 32 && c <= 126 && len < max_line - 1) {
            memmove(&buf[cur + 1], &buf[cur], len - cur);
            buf[cur] = (char)c; len++; cur++;
        }
        uedit_refresh_line(prompt, buf, cur);
    }
    uedit_disable_raw_mode();
    return len;
}

static UEDIT_UNUSED int uedit(const char *prompt, char *buf, int max_line) {
    return uedit_with_event(prompt, buf, max_line, -1, NULL, NULL, NULL);
}

#endif
