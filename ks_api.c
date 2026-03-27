#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ksynth.h"

/* Internal state managed by API */
static ks_ctx *global_ctx = NULL;
static float  *ks_buf     = NULL;
static int     ks_len     = 0;

/* REPL result buffer — separate from ks_buf */
static double *repl_vals = NULL;
static int     repl_n    = 0;
static char    repl_str[1024] = {0};

/* ── helpers ─────────────────────────────────────────────────────────── */

static void ensure_ctx(void) {
    if (!global_ctx) {
        /* Default limits: 1GB memory, 1B operations */
        global_ctx = ks_create(1024 * 1024 * 1024, 1000000000LL);
    }
}

/* ── public API ──────────────────────────────────────────────────────── */

void ks_init(void) {
    if (global_ctx) ks_destroy(global_ctx);
    /* Set default limits for safety (can be tuned) */
    global_ctx = ks_create(512 * 1024 * 1024, 500000000LL);
    ks_buf = NULL;
    ks_len = 0;
}

int ks_run(const char *script) {
    ensure_ctx();
    ks_clear_vars(global_ctx);

    if (ks_buf) { free(ks_buf); ks_buf = NULL; ks_len = 0; }

    const char *p = script;
    while (*p) {
        const char *nl = strchr(p, '\n');
        int len = nl ? (int)(nl - p) : (int)strlen(p);
        
        if (len > 0 && p[0] != '/') {
            K result = ks_eval(global_ctx, p, len);
            if (result) k_free(global_ctx, result);
            
            if (global_ctx->last_status != KS_OK) {
                return -1;
            }
        }
        p = nl ? nl + 1 : p + len;
    }

    K w = k_get(global_ctx, 'W');
    if (!w || w->n <= 0) {
        if (w) k_free(global_ctx, w);
        return -1;
    }

    ks_len = w->n;
    ks_buf = (float*)malloc(ks_len * sizeof(float));
    if (!ks_buf) {
        k_free(global_ctx, w);
        ks_len = 0;
        return -1;
    }
    for (int i = 0; i < ks_len; i++) {
        double v = w->f[i];
        if (v >  1.0) v =  1.0;
        if (v < -1.0) v = -1.0;
        ks_buf[i] = (float)v;
    }
    k_free(global_ctx, w);
    return ks_len;
}

int ks_repl(const char *expr) {
    ensure_ctx();
    repl_str[0] = 0;
    if (repl_vals) { free(repl_vals); repl_vals = NULL; repl_n = 0; }

    if (!expr || !*expr) return 0;

    K result = ks_eval(global_ctx, expr, strlen(expr));

    if (global_ctx->last_status != KS_OK) {
        snprintf(repl_str, sizeof(repl_str), "Error: %s", ks_strerror(global_ctx->last_status));
        return -1;
    }

    if (!result) {
        snprintf(repl_str, sizeof(repl_str), "nil");
        return 0;
    }

    /* Copy result values */
    repl_n    = result->n;
    repl_vals = (double*)malloc((size_t)repl_n * sizeof(double));
    if (repl_vals)
        for (int i = 0; i < repl_n; i++) repl_vals[i] = result->f[i];

    /* Format result summary */
    if (result->n == 1) {
        snprintf(repl_str, sizeof(repl_str), "%g", result->f[0]);
    } else {
        int show = result->n < 6 ? result->n : 6;
        int pos = 0;
        pos += snprintf(repl_str + pos, sizeof(repl_str) - pos, "[");
        for (int i = 0; i < show && pos < (int)sizeof(repl_str) - 30; i++) {
            if (i > 0) pos += snprintf(repl_str + pos, sizeof(repl_str) - pos, "  ");
            pos += snprintf(repl_str + pos, sizeof(repl_str) - pos, "%g", result->f[i]);
        }
        if (result->n > 6)
            snprintf(repl_str + pos, sizeof(repl_str) - pos, " ...]  len=%d", result->n);
        else
            snprintf(repl_str + pos, sizeof(repl_str) - pos, "]");
    }

    /* Free result (unless it was just assigned to a var - but we k_get it if needed anyway) */
    k_free(global_ctx, result);
    return 0;
}

const char *ks_repl_str(void) { return repl_str; }

static float *get_var_fbuf = NULL;
static int    get_var_flen = 0;

int ks_get_var(int letter_upper) {
    ensure_ctx();
    free(get_var_fbuf); get_var_fbuf = NULL; get_var_flen = 0;
    if (letter_upper < 'A' || letter_upper > 'Z') return 0;
    K v = global_ctx->vars[letter_upper - 'A'];
    if (!v || v->n <= 0) return 0;
    get_var_fbuf = (float*)malloc((size_t)v->n * sizeof(float));
    if (!get_var_fbuf) return 0;
    for (int i = 0; i < v->n; i++) get_var_fbuf[i] = (float)v->f[i];
    get_var_flen = v->n;
    return v->n;
}

float *ks_get_var_buf(void) { return get_var_fbuf; }
int ks_repl_length(void) { return repl_n; }

int ks_repl_get_floats(float *out, int max_n) {
    int n = repl_n < max_n ? repl_n : max_n;
    if (repl_vals) for (int i = 0; i < n; i++) out[i] = (float)repl_vals[i];
    return n;
}

float *ks_get_buffer(void) { return ks_buf; }
int ks_get_length(void) { return ks_len; }
const char *ks_get_error(void) { 
    if (global_ctx && global_ctx->last_status != KS_OK)
        return ks_strerror(global_ctx->last_status);
    return "";
}
