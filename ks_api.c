#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ksynth.h"

/* Internal state */
static float  *ks_buf    = NULL;
static int     ks_len    = 0;
static char    ks_err[256] = {0};

/* REPL result buffer — separate from ks_buf */
static double *repl_vals = NULL;
static int     repl_n    = 0;
static char    repl_str[1024] = {0};

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
 * Clears all variables before running — each script is self-contained.
 */
int ks_run(const char *script) {
    ks_err[0] = 0;

    if (ks_buf) { free(ks_buf); ks_buf = NULL; ks_len = 0; }

    clear_vars();

    const char *p = script;
    char line[8192];
    while (*p) {
        const char *nl = strchr(p, '\n');
        int len = nl ? (int)(nl - p) : (int)strlen(p);
        if (len >= (int)sizeof(line)) len = (int)sizeof(line) - 1;
        memcpy(line, p, len);
        line[len] = 0;
        p = nl ? nl + 1 : p + len;

        if (line[0] == 0 || line[0] == '/') continue;

        K result = run_line(line);
        if (result) k_free(result);
    }

    K w = k_get('W');
    if (!w || w->n <= 0) {
        snprintf(ks_err, sizeof(ks_err),
                 w ? "W is empty" : "no W variable set");
        if (w) k_free(w);
        return -1;
    }

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

/*
 * Evaluate one expression WITHOUT clearing variables.
 * State (a-z) persists between calls — this is the REPL entry point.
 *
 * Returns 0 on success, -1 on error.
 * After success: ks_repl_str() returns a human-readable result summary.
 * After error:   ks_get_error() returns the error message.
 */
int ks_repl(const char *expr) {
    ks_err[0]   = 0;
    repl_str[0] = 0;
    if (repl_vals) { free(repl_vals); repl_vals = NULL; repl_n = 0; }

    if (!expr || !*expr) {
        repl_str[0] = 0;
        return 0;
    }

    /* Run the expression — do NOT clear vars */
    char buf[8192];
    strncpy(buf, expr, sizeof(buf)-1);
    buf[sizeof(buf)-1] = 0;
    char *s = buf;
    K result = e(&s);

    if (!result) {
        snprintf(repl_str, sizeof(repl_str), "nil");
        return 0;
    }

    /* Check whether result is owned by vars[] — if so don't free it */
    int in_vars = 0;
    for (int i = 0; i < 26; i++)
        if (vars[i] == result) { in_vars = 1; break; }

    /* Format result into repl_str */
    if (result->n == 1) {
        snprintf(repl_str, sizeof(repl_str), "%g", result->f[0]);
    } else {
        int show = result->n < 6 ? result->n : 6;
        int pos = 0;
        pos += snprintf(repl_str + pos, sizeof(repl_str) - pos, "[");
        for (int i = 0; i < show && pos < (int)sizeof(repl_str) - 20; i++) {
            if (i > 0) pos += snprintf(repl_str + pos, sizeof(repl_str) - pos, "  ");
            pos += snprintf(repl_str + pos, sizeof(repl_str) - pos, "%g", result->f[i]);
        }
        if (result->n > 6)
            snprintf(repl_str + pos, sizeof(repl_str) - pos,
                     " ...]  len=%d", result->n);
        else
            snprintf(repl_str + pos, sizeof(repl_str) - pos, "]");
    }

    if (!in_vars) k_free(result);
    return 0;
}

/* Result string from last ks_repl call */
const char *ks_repl_str(void) { return repl_str; }

/* Pointer to the float output buffer (valid until next ks_run call) */
float *ks_get_buffer(void) { return ks_buf; }

/* Number of samples in the buffer */
int ks_get_length(void) { return ks_len; }

/* Last error string (empty string if no error) */
const char *ks_get_error(void) { return ks_err; }
