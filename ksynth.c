#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <setjmp.h>
#include "ksynth.h"

/* --- Thread-Local Tracking for Signal Handling --- */

static KS_TLS ks_ctx *current_ks_ctx = NULL;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* --- Error Strings --- */

const char* ks_strerror(ks_status status) {
    switch (status) {
        case KS_OK:           return "ok";
        case KS_ERR_SYNTAX:    return "syntax error";
        case KS_ERR_OOM:       return "out of memory";
        case KS_ERR_GAS:       return "gas limit exceeded";
        case KS_ERR_SIGSEGV:   return "segmentation fault";
        case KS_ERR_SIGFPE:    return "floating point exception";
        case KS_ERR_SIGILL:    return "illegal instruction";
        case KS_ERR_INVALID_ARGS: return "invalid arguments";
        case KS_ERR_INTERNAL:  return "internal error";
        default:               return "unknown error";
    }
}

/* --- Signal Handling --- */

static void ks_handle_signal(int sig) {
    if (!current_ks_ctx) return;
    
    switch (sig) {
        case SIGSEGV: current_ks_ctx->last_status = KS_ERR_SIGSEGV; break;
        case SIGFPE:  current_ks_ctx->last_status = KS_ERR_SIGFPE; break;
        case SIGILL:  current_ks_ctx->last_status = KS_ERR_SIGILL; break;
        default:      current_ks_ctx->last_status = KS_ERR_INTERNAL; break;
    }
    longjmp(current_ks_ctx->recover, 1);
}

/* --- Gas Helper --- */

#define GAS_CHECK(ctx, n) do { \
    (ctx)->gas_used += (n); \
    if ((ctx)->gas_limit > 0 && (ctx)->gas_used > (ctx)->gas_limit) { \
        (ctx)->last_status = KS_ERR_GAS; \
        longjmp((ctx)->recover, 1); \
    } \
} while(0)

/* --- Safe Value Helper --- */

static inline double safe_val(double v) {
    if (isnan(v) || isinf(v)) return 0.0;
    if (v > 1e6) return 1e6;
    if (v < -1e6) return -1e6;
    return v;
}

/* --- Context Lifecycle --- */

ks_ctx* ks_create(size_t mem_limit, long long gas_limit) {
    ks_ctx *ctx = calloc(1, sizeof(ks_ctx));
    if (!ctx) return NULL;
    ctx->mem_limit = mem_limit;
    ctx->gas_limit = gas_limit;
    return ctx;
}

void ks_clear_vars(ks_ctx *ctx) {
    if (!ctx) return;
    for (int i = 0; i < 26; i++) {
        if (ctx->vars[i]) {
            k_free(ctx, ctx->vars[i]);
            ctx->vars[i] = NULL;
        }
    }
    for (int i = 0; i < 2; i++) {
        if (ctx->args[i]) {
            k_free(ctx, ctx->args[i]);
            ctx->args[i] = NULL;
        }
    }
}

void ks_destroy(ks_ctx *ctx) {
    if (!ctx) return;
    ks_clear_vars(ctx);
    free(ctx);
}

/* --- K Lifecycle --- */

K k_new(ks_ctx *ctx, int n) {
    if (n < 0) n = 0;
    size_t sz = sizeof(struct { int r, n; double f[]; }) + sizeof(double) * n;
    
    if (ctx->mem_limit > 0 && ctx->mem_used + sz > ctx->mem_limit) {
        ctx->last_status = KS_ERR_OOM;
        longjmp(ctx->recover, 1);
    }
    
    K x = malloc(sz);
    if (!x) {
        ctx->last_status = KS_ERR_OOM;
        longjmp(ctx->recover, 1);
    }
    
    ctx->mem_used += sz;
    x->r = 1; x->n = n; return x;
}

void k_free(ks_ctx *ctx, K x) {
    if (!x) return;
    if (!--x->r) {
        size_t sz = sizeof(struct { int r, n; double f[]; }) + sizeof(double) * (x->n < 0 ? 0 : x->n);
        /* If it's a function (n=-1), we still need to calculate the actual allocated size.
           The function logic uses n=-1 but allocates based on length. 
           We'll improve this tracking. */
        if (x->n == -1) {
            // Re-calculate size for functions. In k_func we use ndoubles.
            // Let's make it consistent.
            char *body = (char*)x->f;
            int len = strlen(body) + 1;
            int ndoubles = (len + sizeof(double) - 1) / sizeof(double);
            sz = sizeof(struct { int r, n; double f[]; }) + sizeof(double) * ndoubles;
        }
        
        if (ctx->mem_used >= sz) ctx->mem_used -= sz;
        else ctx->mem_used = 0;
        
        free(x);
    }
}

K k_view(ks_ctx *ctx, int n, double *ptr) {
    K x = k_new(ctx, n);
    if (x && ptr) {
        GAS_CHECK(ctx, n);
        memcpy(x->f, ptr, n * sizeof(double));
    }
    return x;
}

void bind_scalar(ks_ctx *ctx, char name, double val) {
    if (name < 'A' || name > 'Z') return;
    int i = name - 'A';
    K x = k_new(ctx, 1); x->f[0] = val;
    if (ctx->vars[i]) k_free(ctx, ctx->vars[i]);
    ctx->vars[i] = x;
}

K k_get(ks_ctx *ctx, char name) {
    if (name < 'A' || name > 'Z' || !ctx->vars[name - 'A']) return NULL;
    K v = ctx->vars[name - 'A'];
    v->r++; return v;
}

/* --- Function Support --- */

K k_func(ks_ctx *ctx, char *body) {
    int len = strlen(body) + 1;
    int ndoubles = (len + sizeof(double) - 1) / sizeof(double);
    K x = k_new(ctx, ndoubles);
    x->n = -1;
    memcpy(x->f, body, len);
    return x;
}

int k_is_func(K x) {
    return x && x->n == -1;
}

char* k_func_body(K x) {
    return k_is_func(x) ? (char*)x->f : NULL;
}

/* Forward declarations for evaluation */
K e(ks_ctx *ctx, char **s);

K k_call(ks_ctx *ctx, K fn, K *call_args, int nargs) {
    if (!k_is_func(fn)) return NULL;

    char *body = k_func_body(fn);
    if (!body) return NULL;

    GAS_CHECK(ctx, 10); /* Function call overhead */

    int uses_x = (strchr(body, 'x') != NULL);
    int uses_y = (strchr(body, 'y') != NULL);

    int required = 0;
    if (uses_y) required = 2;
    else if (uses_x) required = 1;

    if (nargs < required) {
        return k_new(ctx, 0);
    }

    K old_x = ctx->args[0];
    K old_y = ctx->args[1];

    ctx->args[0] = k_new(ctx, 0);
    ctx->args[1] = k_new(ctx, 0);

    if (nargs > 0 && call_args[0]) {
        k_free(ctx, ctx->args[0]);
        call_args[0]->r++;
        ctx->args[0] = call_args[0];
    }
    if (nargs > 1 && call_args[1]) {
        k_free(ctx, ctx->args[1]);
        call_args[1]->r++;
        ctx->args[1] = call_args[1];
    }

    char *s = body;
    K result = e(ctx, &s);

    if (ctx->args[0]) k_free(ctx, ctx->args[0]);
    if (ctx->args[1]) k_free(ctx, ctx->args[1]);

    ctx->args[0] = old_x;
    ctx->args[1] = old_y;

    return result;
}

/* --- Scan Adverb --- */

K scan(ks_ctx *ctx, char op, K b) {
    if (!b || b->n < 1) return b;
    K x = k_new(ctx, b->n);
    double acc;

    GAS_CHECK(ctx, b->n);

    switch(op) {
        case '+':
            acc = 0.0;
            for (int i = 0; i < b->n; i++) { acc += b->f[i]; x->f[i] = acc; }
            break;
        case '*':
            acc = 1.0;
            for (int i = 0; i < b->n; i++) { acc *= b->f[i]; x->f[i] = acc; }
            break;
        case '-':
            acc = 0.0;
            for (int i = 0; i < b->n; i++) { acc -= b->f[i]; x->f[i] = acc; }
            break;
        case '%':
            acc = 1.0;
            for (int i = 0; i < b->n; i++) {
                if (b->f[i] != 0) acc /= b->f[i];
                x->f[i] = acc;
            }
            break;
        case '&':
            acc = b->f[0];
            x->f[0] = acc;
            for (int i = 1; i < b->n; i++) {
                if (b->f[i] < acc) acc = b->f[i];
                x->f[i] = acc;
            }
            break;
        case '|':
            acc = b->f[0];
            x->f[0] = acc;
            for (int i = 1; i < b->n; i++) {
                if (b->f[i] > acc) acc = b->f[i];
                x->f[i] = acc;
            }
            break;
        case '^':
            acc = b->f[0];
            x->f[0] = acc;
            for (int i = 1; i < b->n; i++) {
                acc = pow(acc, b->f[i]);
                x->f[i] = acc;
            }
            break;
        default:
            memcpy(x->f, b->f, b->n * sizeof(double));
            break;
    }

    k_free(ctx, b);
    return x;
}

/* --- Verbs & Operators --- */

K mo(ks_ctx *ctx, char c, K b) {
    if (!b) return NULL;

    if (c >= 'A' && c <= 'Z') {
        K var = ctx->vars[c - 'A'];
        if (k_is_func(var)) {
            K call_args[1] = {b};
            return k_call(ctx, var, call_args, 1);
        }
    }

    K x;

    if (c == '!') {
        int n = (int)b->f[0]; k_free(ctx, b);
        GAS_CHECK(ctx, n);
        x = k_new(ctx, n);
        for (int j = 0; j < n; j++) x->f[j] = (double)j;
        return x;
    }

    if (c == '~') {
        int n = (int)b->f[0]; k_free(ctx, b);
        if (n < 1) return k_new(ctx, 0);
        GAS_CHECK(ctx, n);
        x = k_new(ctx, n);
        double twopi = 2.0 * M_PI;
        for (int j = 0; j < n; j++) x->f[j] = twopi * (double)j / (double)n;
        return x;
    }

    if (c == '+') {
        double t = 0;
        GAS_CHECK(ctx, b->n);
        for (int i = 0; i < b->n; i++) t += b->f[i];
        x = k_new(ctx, 1); x->f[0] = t;
        k_free(ctx, b); return x;
    }

    if (c == '>') {
        double m = 0;
        GAS_CHECK(ctx, b->n);
        for (int i = 0; i < b->n; i++) if (fabs(b->f[i]) > m) m = fabs(b->f[i]);
        x = k_new(ctx, 1); x->f[0] = m;
        k_free(ctx, b); return x;
    }

    if (c == 'w') {
        double pk = 0.0;
        GAS_CHECK(ctx, b->n);
        for (int i = 0; i < b->n; i++) if (fabs(b->f[i]) > pk) pk = fabs(b->f[i]);
        x = k_new(ctx, b->n);
        double scale = (pk > 1e-10) ? 1.0 / pk : 0.0;
        for (int i = 0; i < b->n; i++) x->f[i] = b->f[i] * scale;
        k_free(ctx, b); return x;
    }

    if (c == 'j') {
        if (b->n < 2) { k_free(ctx, b); return k_new(ctx, 0); }
        int n = b->n / 2;
        GAS_CHECK(ctx, n);
        x = k_new(ctx, n);
        for (int i = 0; i < x->n; i++) x->f[i] = b->f[i*2];
        k_free(ctx, b); return x;
    }

    if (c == 'k') {
        if (b->n < 2) { k_free(ctx, b); return k_new(ctx, 0); }
        int n = b->n / 2;
        GAS_CHECK(ctx, n);
        x = k_new(ctx, n);
        for (int i = 0; i < x->n; i++) x->f[i] = b->f[i*2+1];
        k_free(ctx, b); return x;
    }

    if (c == 'v') {
        GAS_CHECK(ctx, b->n);
        x = k_new(ctx, b->n);
        for (int i = 0; i < b->n; i++)
            x->f[i] = floor(b->f[i] * 4.0) / 4.0;
        k_free(ctx, b); return x;
    }

    GAS_CHECK(ctx, b->n);
    x = k_new(ctx, b->n);
    for (int i = 0; i < b->n; i++) {
        double v = b->f[i];
        switch (c) {
            case 's': x->f[i] = sin(v); break;
            case 'c': x->f[i] = cos(v); break;
            case 't': x->f[i] = tan(v); break;
            case 'h': x->f[i] = tanh(v); break;
            case 'a': x->f[i] = fabs(v); break;
            case 'q': x->f[i] = sqrt(fabs(v)); break;
            case 'l': x->f[i] = log(fabs(v) + 1e-10); break;
            case 'e': {
                double cl = (v > 100) ? 100 : ((v < -100) ? -100 : v);
                x->f[i] = exp(cl);
                break;
            }
            case '_': x->f[i] = floor(v); break;
            case 'r': x->f[i] = ((double)rand() / (double)RAND_MAX) * 2.0 - 1.0; break;
            case 'p': x->f[i] = (v == 0) ? 44100 : M_PI * v; break;
            case 'i': x->f[i] = b->f[b->n - 1 - i]; break;
            case 'x': x->f[i] = exp(-5.0 * v); break;
            case 'd': x->f[i] = tanh(v * 3.0); break;
            case 'm': {
                unsigned int clock = i;
                unsigned int hh = (clock * 13) ^ (clock >> 5) ^ (clock * 193);
                x->f[i] = (hh & 128) ? 0.7 : -0.7;
                break;
            }
            case 'b': {
                double ff[] = {2.43, 3.01, 3.52, 4.11, 5.23, 6.78};
                double ss = 0;
                for (int j = 0; j < 6; j++)
                    ss += (sin(i * 0.1 * ff[j]) > 0) ? 1.0 : -1.0;
                x->f[i] = ss / 6.0;
                break;
            }
            case 'u': x->f[i] = (i < 10) ? (double)i / 10.0 : 1.0; break;
            case 'n': x->f[i] = 440.0 * pow(2.0, (v - 69.0) / 12.0); break;
            default: x->f[i] = v; break;
        }
    }
    k_free(ctx, b); return x;
}

K dy(ks_ctx *ctx, char c, K a, K b) {
    if (!a || !b) { k_free(ctx, a); k_free(ctx, b); return NULL; }

    if (c >= 'A' && c <= 'Z') {
        K var = ctx->vars[c - 'A'];
        if (k_is_func(var)) {
            K call_args[2] = {a, b};
            return k_call(ctx, var, call_args, 2);
        }
    }

    K x;

    if (c == 'z') {
        int mn = (a->n < b->n) ? a->n : b->n;
        GAS_CHECK(ctx, mn * 2);
        x = k_new(ctx, mn * 2);
        for (int i = 0; i < mn; i++) {
            x->f[i*2]   = a->f[i];
            x->f[i*2+1] = b->f[i];
        }
        k_free(ctx, a); k_free(ctx, b); return x;
    }

    if (c == 'o') {
        GAS_CHECK(ctx, (long long)a->n * b->n);
        x = k_new(ctx, a->n);
        for (int i = 0; i < a->n; i++) {
            double acc = 0.0;
            for (int j = 0; j < b->n; j++)
                acc += sin(a->f[i] * b->f[j]);
            x->f[i] = acc;
        }
        k_free(ctx, a); k_free(ctx, b); return x;
    }

    if (c == '$') {
        GAS_CHECK(ctx, (long long)a->n * b->n);
        x = k_new(ctx, a->n);
        for (int i = 0; i < a->n; i++) {
            double acc = 0.0;
            for (int j = 0; j < b->n; j++)
                acc += b->f[j] * sin(a->f[i] * (double)(j + 1));
            x->f[i] = acc;
        }
        k_free(ctx, a); k_free(ctx, b); return x;
    }

    if (c == 't') {
        if (a->n < 1 || b->n < 1) { k_free(ctx, a); k_free(ctx, b); return k_new(ctx, 0); }
        double freq_hz = b->f[0];
        int    n_out;
        if (b->n >= 2) {
            n_out = (int)b->f[1];
        } else {
            K nv = ctx->vars['N' - 'A'];
            n_out = (nv && nv->n > 0) ? (int)nv->f[0] : 0;
        }
        int tbl_len = a->n;
        if (n_out < 1 || tbl_len < 1) { k_free(ctx, a); k_free(ctx, b); return k_new(ctx, 0); }

        GAS_CHECK(ctx, n_out);
        double phase_inc = freq_hz * (double)tbl_len / 44100.0;
        double phase     = 0.0;
        x = k_new(ctx, n_out);

        for (int i = 0; i < n_out; i++) {
            while (phase >= tbl_len) phase -= tbl_len;
            while (phase <  0.0)    phase += tbl_len;
            int    idx  = (int)phase;
            double frac = phase - idx;
            int    idx2 = (idx + 1) % tbl_len;
            x->f[i] = a->f[idx] * (1.0 - frac) + a->f[idx2] * frac;
            phase += phase_inc;
        }
        k_free(ctx, a); k_free(ctx, b); return x;
    }

    if (c == 'v') {
        double levels = (a->n > 0 && a->f[0] > 0) ? a->f[0] : 4.0;
        GAS_CHECK(ctx, b->n);
        x = k_new(ctx, b->n);
        for (int i = 0; i < b->n; i++)
            x->f[i] = floor(b->f[i] * levels) / levels;
        k_free(ctx, a); k_free(ctx, b); return x;
    }

    if (c == 'f') {
        GAS_CHECK(ctx, b->n);
        x = k_new(ctx, b->n); double b0 = 0, b1 = 0;
        for (int i = 0; i < b->n; i++) {
            double ct = (a->n > i) ? a->f[i] : a->f[0];
            double rs = (a->n >= 2) ? a->f[1] : 0.0;
            if (ct > 0.95) ct = 0.95;
            if (rs > 3.98) rs = 3.98;
            double in = b->f[i] - (rs * b1);
            b0 += ct * (in - b0); b1 += ct * (b0 - b1);
            b0 = safe_val(b0); b1 = safe_val(b1);
            x->f[i] = b1;
        }
        k_free(ctx, a); k_free(ctx, b); return x;
    } else if (c == 'g') {
        GAS_CHECK(ctx, b->n);
        x = k_new(ctx, b->n);
        double s0 = 0.0, s1 = 0.0;
        double static_f = a->f[0];
        double q_val    = (a->n >= 2) ? a->f[1] : 0.5;
        double damp     = 1.0 / (q_val < 0.01 ? 0.01 : q_val);
        for (int i = 0; i < b->n; i++) {
            double f_hz    = (a->n == b->n) ? a->f[i] : static_f;
            double f_coeff = 2.0 * sin(M_PI * f_hz / 44100.0);
            if (f_coeff > 1.99) f_coeff = 1.99;
            double hp = b->f[i] - s0 - damp * s1;
            s1 += f_coeff * hp; s0 += f_coeff * s1;
            s0 = safe_val(s0); s1 = safe_val(s1);
            x->f[i] = s1;
        }
        k_free(ctx, a); k_free(ctx, b); return x;
    } else if (c == 'y') {
        int dd   = (int)a->f[0];
        double g = (a->n > 1) ? a->f[1] : 0.4;
        GAS_CHECK(ctx, b->n);
        x = k_new(ctx, b->n);
        for (int i = 0; i < b->n; i++) {
            double delayed = (i >= dd) ? x->f[i-dd] : 0;
            x->f[i] = safe_val(b->f[i] + (g * delayed));
        }
        k_free(ctx, a); k_free(ctx, b); return x;
    } else if (c == '#') {
        int n = (int)a->f[0];
        GAS_CHECK(ctx, n);
        x = k_new(ctx, n);
        if (b->n > 0) for (int i = 0; i < n; i++) x->f[i] = b->f[i % b->n];
        k_free(ctx, a); k_free(ctx, b); return x;
    } else if (c == ',') {
        int n = a->n + b->n;
        GAS_CHECK(ctx, n);
        x = k_new(ctx, n);
        memcpy(x->f, a->f, a->n * sizeof(double));
        memcpy(x->f + a->n, b->f, b->n * sizeof(double));
        k_free(ctx, a); k_free(ctx, b); return x;
    } else {
        int mn = a->n > b->n ? a->n : b->n;
        GAS_CHECK(ctx, mn);
        x = k_new(ctx, mn);
        for (int i = 0; i < mn; i++) {
            double va = a->f[i % a->n], vb = b->f[i % b->n];
            switch (c) {
                case '+': x->f[i] = va + vb; break;
                case '*': x->f[i] = va * vb; break;
                case '-': x->f[i] = va - vb; break;
                case '%': x->f[i] = (vb == 0) ? 0 : va / vb; break;
                case '^': x->f[i] = safe_val(pow(fabs(va), vb)); break;
                case '&': x->f[i] = va < vb ? va : vb; break;
                case '|': x->f[i] = va > vb ? va : vb; break;
                case '<': x->f[i] = va < vb ? 1.0 : 0.0; break;
                case '>': x->f[i] = va > vb ? 1.0 : 0.0; break;
                case '=': x->f[i] = va == vb ? 1.0 : 0.0; break;
                default:  x->f[i] = 0; break;
            }
        }
        k_free(ctx, a); k_free(ctx, b); return x;
    }
}

/* --- Parser --- */

K atom(ks_ctx *ctx, char **s);

K expr(ks_ctx *ctx, char **s) {
    K x = atom(ctx, s);
    while (**s == ' ') (*s)++;

    if (k_is_func(x) && **s && **s != '\n' && **s != ')' && **s != ';' && **s != '}' && **s != '/') {
        char peek = **s;
        int is_operator = strchr("+-*%^&|<>=,#osfzt haqle rpciw dvmbu jkn g", peek) != NULL;
        if (!is_operator) {
            K arg = expr(ctx, s);
            K call_args[1] = {arg};
            K result = k_call(ctx, x, call_args, 1);
            k_free(ctx, x);
            return result;
        }
    }

    if (!**s || **s == '\n' || **s == ')' || **s == ';' || **s == '}' || **s == '/') return x;
    char op = *(*s)++;
    return dy(ctx, op, x, expr(ctx, s));
}

K atom(ks_ctx *ctx, char **s) {
    while (**s == ' ') (*s)++;
    if (**s == '/') {
        while (**s && **s != '\n') (*s)++;
        if (**s == '\n') (*s)++;
        return atom(ctx, s);
    }
    if (!**s || **s == '\n' || **s == ')' || **s == ';') return NULL;

    if (**s == '(') {
        (*s)++; K x = e(ctx, s);
        if (**s == ')') (*s)++;
        return x;
    }

    if (**s == '{') {
        (*s)++;
        char *start = *s;
        int depth = 1;
        while (**s && depth > 0) {
            if (**s == '{') depth++;
            else if (**s == '}') depth--;
            (*s)++;
        }
        if (depth == 0) {
            int len = (*s - 1) - start;
            char *body = malloc(len + 1);
            memcpy(body, start, len);
            body[len] = '\0';
            K func = k_func(ctx, body);
            free(body);
            return func;
        }
        return NULL;
    }

    char c = **s;

    if ((c >= '0' && c <= '9') || (c == '.' && (*s)[1] >= '0') ||
        (c == '-' && ((*s)[1] >= '0' || (*s)[1] == '.'))) {
        double buf[1024]; int n = 0;
        char *ptr = *s;
        while (n < 1024) {
            buf[n++] = strtod(ptr, &ptr);
            char *after = ptr;
            char *peek  = ptr;
            while (*peek == ' ') peek++;
            int had_space = (peek != after);
            if (*peek >= '0' && *peek <= '9') { ptr = peek; continue; }
            if (*peek == '-' && peek[1] >= '0' && had_space) { ptr = peek; continue; }
            if (had_space && *peek >= 'A' && *peek <= 'Z' && peek[1] != ':') {
                K v = ctx->vars[*peek - 'A'];
                if (v && v->n == 1) { buf[n++] = v->f[0]; ptr = peek + 1; continue; }
            }
            break;
        }
        *s = ptr;
        K x = k_new(ctx, n); memcpy(x->f, buf, n * sizeof(double));
        return x;
    }

    (*s)++;

    if (**s == ':') {
        (*s)++; K x = expr(ctx, s);
        if (c >= 'A' && c <= 'Z') {
            int i = c - 'A';
            if (ctx->vars[i]) k_free(ctx, ctx->vars[i]);
            if (x) { x->r++; ctx->vars[i] = x; }
        }
        if (x) x->r++;
        return x;
    }

    if (c >= 'A' && c <= 'Z') {
        K first = k_get(ctx, c);
        if (!first || first->n != 1) return first;
        double buf[1024]; int n = 0;
        buf[n++] = first->f[0];
        k_free(ctx, first);
        char *ptr = *s;
        while (n < 1024) {
            char *peek = ptr;
            while (*peek == ' ') peek++;
            if (peek == ptr) break;
            if (*peek < 'A' || *peek > 'Z') break;
            if (peek[1] == ':') break;
            K v = ctx->vars[*peek - 'A'];
            if (!v || v->n != 1) break;
            buf[n++] = v->f[0];
            ptr = peek + 1;
        }
        *s = ptr;
        K x = k_new(ctx, n);
        memcpy(x->f, buf, n * sizeof(double));
        return x;
    }

    if (c == 'x') return ctx->args[0] ? (ctx->args[0]->r++, ctx->args[0]) : k_new(ctx, 0);
    if (c == 'y') return ctx->args[1] ? (ctx->args[1]->r++, ctx->args[1]) : k_new(ctx, 0);

    int is_scan = 0;
    while (**s == ' ') (*s)++;
    if (**s == '\\') { is_scan = 1; (*s)++; }
    K arg = e(ctx, s);
    if (is_scan) return scan(ctx, c, arg);
    else return mo(ctx, c, arg);
}

K e(ks_ctx *ctx, char **s) {
    K x = atom(ctx, s);
    while (**s == ' ') (*s)++;
    while (**s == ';') {
        (*s)++;
        if (x) k_free(ctx, x);
        while (**s == ' ') (*s)++;
        if (!**s || **s == '\n' || **s == ')' || **s == '}') return k_new(ctx, 0);
        x = atom(ctx, s);
        while (**s == ' ') (*s)++;
    }
    if (k_is_func(x) && **s && **s != '\n' && **s != ')' && **s != ';' && **s != '/') {
        char peek = **s;
        int is_operator = strchr("+-*%^&|<>=,#osfzt haqle rpciw dvmbu jkn g", peek) != NULL;
        if (!is_operator) {
            K arg = e(ctx, s);
            K call_args[1] = {arg};
            K result = k_call(ctx, x, call_args, 1);
            k_free(ctx, x);
            return result;
        }
    }
    if (!**s || **s == '\n' || **s == ')' || **s == ';' || **s == '/') return x;
    char op = *(*s)++;
    return dy(ctx, op, x, e(ctx, s));
}

/* --- Public API --- */

K ks_eval(ks_ctx *ctx, const char *code, size_t len) {
    if (!ctx || !code) return NULL;
    
    /* Setup sandbox environment */
    current_ks_ctx = ctx;
    ctx->last_status = KS_OK;
    
    /* Setup signals */
    void (*old_segv)(int) = signal(SIGSEGV, ks_handle_signal);
    void (*old_fpe)(int)  = signal(SIGFPE,  ks_handle_signal);
    void (*old_ill)(int)  = signal(SIGILL,  ks_handle_signal);
    
    K result = NULL;
    if (setjmp(ctx->recover) == 0) {
        /* Evaluation */
        char *buf = malloc(len + 1);
        if (!buf) {
            ctx->last_status = KS_ERR_OOM;
        } else {
            memcpy(buf, code, len);
            buf[len] = '\0';
            char *p_code = buf;
            result = e(ctx, &p_code);
            free(buf);
        }
    } else {
        /* Recovered from longjmp */
        result = NULL;
    }
    
    /* Cleanup signals */
    signal(SIGSEGV, old_segv);
    signal(SIGFPE,  old_fpe);
    signal(SIGILL,  old_ill);
    current_ks_ctx = NULL;
    
    return result;
}

void p(ks_ctx *ctx, K x) {
    if (!x) return;
    if (k_is_func(x)) {
        printf("{%s}\n", k_func_body(x));
        return;
    }
    if (x->n == 1) printf("%.4f\n", x->f[0]);
    else printf("Array[%d]\n", x->n);
}
