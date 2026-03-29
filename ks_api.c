#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ksynth.h"

typedef struct ks_api_state {
    ks_ctx *ctx;
    float  *ks_buf;
    int     ks_len;
    double *repl_vals;
    int     repl_n;
    char    repl_str[1024];
    float  *var_buf;
    int     var_len;
    struct ks_api_state *next;
} ks_api_state;

static ks_api_state *g_states = NULL;
static ks_api_state *g_default_state = NULL;

static void ks_api_clear_buffers(ks_api_state *st) {
    if (!st) return;
    free(st->ks_buf);
    st->ks_buf = NULL;
    st->ks_len = 0;
    free(st->repl_vals);
    st->repl_vals = NULL;
    st->repl_n = 0;
    st->repl_str[0] = 0;
    free(st->var_buf);
    st->var_buf = NULL;
    st->var_len = 0;
}

static ks_api_state *ks_api_find(uintptr_t handle) {
    ks_api_state *it = g_states;
    while (it) {
        if ((uintptr_t)it == handle) return it;
        it = it->next;
    }
    return NULL;
}

static ks_api_state *ks_api_create_state(size_t mem_limit, long long gas_limit) {
    ks_api_state *st = (ks_api_state*)calloc(1, sizeof(*st));
    if (!st) return NULL;
    st->ctx = ks_create(mem_limit, gas_limit);
    if (!st->ctx) {
        free(st);
        return NULL;
    }
    st->next = g_states;
    g_states = st;
    return st;
}

static void ks_api_destroy_state(ks_api_state *st) {
    if (!st) return;
    if (g_default_state == st) g_default_state = NULL;
    ks_api_state **link = &g_states;
    while (*link) {
        if (*link == st) {
            *link = st->next;
            break;
        }
        link = &(*link)->next;
    }
    ks_api_clear_buffers(st);
    if (st->ctx) ks_destroy(st->ctx);
    free(st);
}

static ks_api_state *ks_api_ensure_default_legacy(void) {
    if (!g_default_state) {
        g_default_state = ks_api_create_state(1024 * 1024 * 1024, 1000000000LL);
    }
    return g_default_state;
}

/* New context-handle wrapper API */
uintptr_t ks_ctx_create(void) {
    ks_api_state *st = ks_api_create_state(512 * 1024 * 1024, 500000000LL);
    return (uintptr_t)st;
}

void ks_ctx_destroy(uintptr_t handle) {
    ks_api_state *st = ks_api_find(handle);
    if (!st) return;
    ks_api_destroy_state(st);
}

int ks_ctx_run(uintptr_t handle, const char *script) {
    ks_api_state *st = ks_api_find(handle);
    if (!st || !st->ctx) return -1;

    ks_clear_vars(st->ctx);
    ks_api_clear_buffers(st);
    if (!script) return -1;

    const char *p = script;
    while (*p) {
        const char *nl = strchr(p, '\n');
        int len = nl ? (int)(nl - p) : (int)strlen(p);
        if (len > 0 && p[0] != '/') {
            K result = ks_eval(st->ctx, p, (size_t)len);
            if (result) k_free(st->ctx, result);
            if (st->ctx->last_status != KS_OK) return -1;
        }
        p = nl ? nl + 1 : p + len;
    }

    K w = k_get(st->ctx, 'W');
    if (!w || w->n <= 0) {
        if (w) k_free(st->ctx, w);
        return -1;
    }

    st->ks_len = w->n;
    st->ks_buf = (float*)malloc((size_t)st->ks_len * sizeof(float));
    if (!st->ks_buf) {
        k_free(st->ctx, w);
        st->ks_len = 0;
        return -1;
    }

    for (int i = 0; i < st->ks_len; i++) {
        double v = w->f[i];
        if (v > 1.0) v = 1.0;
        if (v < -1.0) v = -1.0;
        st->ks_buf[i] = (float)v;
    }
    k_free(st->ctx, w);
    return st->ks_len;
}

int ks_ctx_repl(uintptr_t handle, const char *expr) {
    ks_api_state *st = ks_api_find(handle);
    if (!st || !st->ctx) return -1;

    st->repl_str[0] = 0;
    free(st->repl_vals);
    st->repl_vals = NULL;
    st->repl_n = 0;

    if (!expr || !*expr) return 0;

    K result = ks_eval(st->ctx, expr, strlen(expr));
    if (st->ctx->last_status != KS_OK) {
        snprintf(st->repl_str, sizeof(st->repl_str), "Error: %s", ks_strerror(st->ctx->last_status));
        return -1;
    }
    if (!result) {
        snprintf(st->repl_str, sizeof(st->repl_str), "nil");
        return 0;
    }

    st->repl_n = result->n;
    st->repl_vals = (double*)malloc((size_t)st->repl_n * sizeof(double));
    if (st->repl_vals) {
        for (int i = 0; i < st->repl_n; i++) st->repl_vals[i] = result->f[i];
    }

    if (result->n == 1) {
        snprintf(st->repl_str, sizeof(st->repl_str), "%g", result->f[0]);
    } else {
        int show = result->n < 6 ? result->n : 6;
        int pos = 0;
        pos += snprintf(st->repl_str + pos, sizeof(st->repl_str) - (size_t)pos, "[");
        for (int i = 0; i < show && pos < (int)sizeof(st->repl_str) - 30; i++) {
            if (i > 0) pos += snprintf(st->repl_str + pos, sizeof(st->repl_str) - (size_t)pos, "  ");
            pos += snprintf(st->repl_str + pos, sizeof(st->repl_str) - (size_t)pos, "%g", result->f[i]);
        }
        if (result->n > 6) {
            snprintf(st->repl_str + pos, sizeof(st->repl_str) - (size_t)pos, " ...]  len=%d", result->n);
        } else {
            snprintf(st->repl_str + pos, sizeof(st->repl_str) - (size_t)pos, "]");
        }
    }

    k_free(st->ctx, result);
    return 0;
}

const char *ks_ctx_repl_str(uintptr_t handle) {
    ks_api_state *st = ks_api_find(handle);
    if (!st) return "";
    return st->repl_str;
}

int ks_ctx_get_var(uintptr_t handle, int letter_upper) {
    ks_api_state *st = ks_api_find(handle);
    if (!st || !st->ctx) return 0;

    free(st->var_buf);
    st->var_buf = NULL;
    st->var_len = 0;
    if (letter_upper < 'A' || letter_upper > 'Z') return 0;

    K v = st->ctx->vars[letter_upper - 'A'];
    if (!v || v->n <= 0) return 0;

    st->var_buf = (float*)malloc((size_t)v->n * sizeof(float));
    if (!st->var_buf) return 0;
    for (int i = 0; i < v->n; i++) st->var_buf[i] = (float)v->f[i];
    st->var_len = v->n;
    return v->n;
}

float *ks_ctx_get_var_buf(uintptr_t handle) {
    ks_api_state *st = ks_api_find(handle);
    if (!st) return NULL;
    return st->var_buf;
}

int ks_ctx_repl_length(uintptr_t handle) {
    ks_api_state *st = ks_api_find(handle);
    if (!st) return 0;
    return st->repl_n;
}

int ks_ctx_repl_get_floats(uintptr_t handle, float *out, int max_n) {
    ks_api_state *st = ks_api_find(handle);
    if (!st || !out || max_n <= 0) return 0;
    int n = st->repl_n < max_n ? st->repl_n : max_n;
    if (st->repl_vals) {
        for (int i = 0; i < n; i++) out[i] = (float)st->repl_vals[i];
    }
    return n;
}

float *ks_ctx_get_buffer(uintptr_t handle) {
    ks_api_state *st = ks_api_find(handle);
    if (!st) return NULL;
    return st->ks_buf;
}

int ks_ctx_get_length(uintptr_t handle) {
    ks_api_state *st = ks_api_find(handle);
    if (!st) return 0;
    return st->ks_len;
}

const char *ks_ctx_get_error(uintptr_t handle) {
    ks_api_state *st = ks_api_find(handle);
    if (!st || !st->ctx) return "invalid context";
    if (st->ctx->last_status != KS_OK) return ks_strerror(st->ctx->last_status);
    return "";
}

/* Backwards-compatible singleton wrappers */
void ks_init(void) {
    if (g_default_state) {
        ks_api_destroy_state(g_default_state);
    }
    g_default_state = ks_api_create_state(512 * 1024 * 1024, 500000000LL);
}

int ks_run(const char *script) {
    ks_api_state *st = ks_api_ensure_default_legacy();
    if (!st) return -1;
    return ks_ctx_run((uintptr_t)st, script);
}

int ks_repl(const char *expr) {
    ks_api_state *st = ks_api_ensure_default_legacy();
    if (!st) return -1;
    return ks_ctx_repl((uintptr_t)st, expr);
}

const char *ks_repl_str(void) {
    ks_api_state *st = ks_api_ensure_default_legacy();
    if (!st) return "";
    return st->repl_str;
}

int ks_get_var(int letter_upper) {
    ks_api_state *st = ks_api_ensure_default_legacy();
    if (!st) return 0;
    return ks_ctx_get_var((uintptr_t)st, letter_upper);
}

float *ks_get_var_buf(void) {
    ks_api_state *st = ks_api_ensure_default_legacy();
    if (!st) return NULL;
    return st->var_buf;
}

int ks_repl_length(void) {
    ks_api_state *st = ks_api_ensure_default_legacy();
    if (!st) return 0;
    return st->repl_n;
}

int ks_repl_get_floats(float *out, int max_n) {
    ks_api_state *st = ks_api_ensure_default_legacy();
    if (!st) return 0;
    return ks_ctx_repl_get_floats((uintptr_t)st, out, max_n);
}

float *ks_get_buffer(void) {
    ks_api_state *st = ks_api_ensure_default_legacy();
    if (!st) return NULL;
    return st->ks_buf;
}

int ks_get_length(void) {
    ks_api_state *st = ks_api_ensure_default_legacy();
    if (!st) return 0;
    return st->ks_len;
}

const char *ks_get_error(void) {
    ks_api_state *st = ks_api_ensure_default_legacy();
    if (!st || !st->ctx) return "";
    if (st->ctx->last_status != KS_OK) return ks_strerror(st->ctx->last_status);
    return "";
}
