#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ksynth.h"

/* Internal state */
static float  *ks_buf    = NULL;
static int     ks_len    = 0;
static char    ks_err[256] = {0};

/* ── helpers ─────────────────────────────────────────────────────────── */

static void clear_vars(void) {
    for (int i = 0; i < 26; i++) {
        if (vars[i]) { k_free(vars[i]); vars[i] = NULL; }
    }
    for (int i = 0; i < 2; i++) {
        if (args[i]) { k_free(args[i]); args[i] = NULL; }
    }
}

static K run_line(const char *line) {
    char buf[8192];
    strncpy(buf, line, sizeof(buf)-1);
    buf[sizeof(buf)-1] = 0;
    char *s = buf;
    return e(&s);
}

/* ── public API ──────────────────────────────────────────────────────── */

/* Call once at startup */
void ks_init(void) {
    clear_vars();
    ks_buf = NULL;
    ks_len = 0;
    ks_err[0] = 0;
}

/*
 * Run a complete ksynth script (newline-separated lines).
 * Returns number of samples in W on success, -1 on error.
 * Call ks_get_buffer() / ks_get_length() after a successful run.
 */
int ks_run(const char *script) {
    ks_err[0] = 0;

    /* Free previous output buffer */
    if (ks_buf) { free(ks_buf); ks_buf = NULL; ks_len = 0; }

    /* Clear all variables for a fresh evaluation */
    clear_vars();

    /* Process line by line — exactly as ksrun.c does */
    const char *p = script;
    char line[8192];
    while (*p) {
        /* extract one line */
        const char *nl = strchr(p, '\n');
        int len = nl ? (int)(nl - p) : (int)strlen(p);
        if (len >= (int)sizeof(line)) len = (int)sizeof(line) - 1;
        memcpy(line, p, len);
        line[len] = 0;
        p = nl ? nl + 1 : p + len;

        /* skip blank lines and comments */
        if (line[0] == 0 || line[0] == '/') continue;

        K result = run_line(line);
        if (result) k_free(result);
    }

    /* Extract W */
    K w = k_get('W');
    if (!w || w->n <= 0) {
        snprintf(ks_err, sizeof(ks_err),
                 w ? "W is empty" : "no W variable set");
        if (w) k_free(w);
        return -1;
    }

    /* Copy doubles → floats into our buffer */
    ks_len = w->n;
    ks_buf = (float*)malloc(ks_len * sizeof(float));
    if (!ks_buf) {
        snprintf(ks_err, sizeof(ks_err), "out of memory");
        k_free(w);
        ks_len = 0;
        return -1;
    }
    for (int i = 0; i < ks_len; i++) {
        double v = w->f[i];
        if (v >  1.0) v =  1.0;
        if (v < -1.0) v = -1.0;
        ks_buf[i] = (float)v;
    }
    k_free(w);
    return ks_len;
}

/* Pointer to the float output buffer (valid until next ks_run call) */
float *ks_get_buffer(void) { return ks_buf; }

/* Number of samples in the buffer */
int ks_get_length(void) { return ks_len; }

/* Last error string (empty string if no error) */
const char *ks_get_error(void) { return ks_err; }
