#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>
#include "ksynth.h"

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


static void bind_alias(ks_ctx *ctx, const char *name, const char *code) {
    char *dup = strdup(code);
    K f = k_func(ctx, dup);
    free(dup);
    if (!f) return;
    int len = strlen((char*)f->f) + 1;
    int ndoubles = (len + sizeof(double) - 1) / sizeof(double);
    K perm = k_new_perm(ctx, ndoubles);
    if (perm) {
        perm->n = -1;
        memcpy(perm->f, f->f, len);
        k_set_var_str(ctx, name, perm);
    }
}

static void ks_init_aliases(ks_ctx *ctx) {
    bind_alias(ctx, "sin", "s x");
    bind_alias(ctx, "cos", "c x");
    bind_alias(ctx, "tan", "t x");
    bind_alias(ctx, "tanh", "h x");
    bind_alias(ctx, "abs", "a x");
    bind_alias(ctx, "sqrt", "q x");
    bind_alias(ctx, "log", "l x");
    bind_alias(ctx, "exp", "e x");
    bind_alias(ctx, "floor", "_ x");
    bind_alias(ctx, "rand", "r x");
    bind_alias(ctx, "pi", "p x");
    bind_alias(ctx, "rev", "i x");
    bind_alias(ctx, "idx", "! x");
    bind_alias(ctx, "phase", "~ x");
    bind_alias(ctx, "sum", "+ x");
    bind_alias(ctx, "peak", "> x");
    bind_alias(ctx, "norm", "w x");
    bind_alias(ctx, "left", "j x");
    bind_alias(ctx, "right", "k x");
    bind_alias(ctx, "quantize", "v x");
    bind_alias(ctx, "saw", "o x");
    bind_alias(ctx, "slice", "x S y");
    bind_alias(ctx, "speed", "x Z y");
    bind_alias(ctx, "delay", "x D y");
}

ks_ctx* ks_create(size_t mem_limit, long long gas_limit, double sample_rate) {
    ks_ctx *ctx = calloc(1, sizeof(ks_ctx));
    if (!ctx) return NULL;
    
    ctx->arena_base = malloc(mem_limit);
    if (!ctx->arena_base) {
        free(ctx);
        return NULL;
    }
    
    ctx->arena_ptr = ctx->arena_base;
    ctx->arena_end = ctx->arena_base + mem_limit;
    ctx->mem_limit = mem_limit;
    ctx->gas_limit = gas_limit;
    ctx->sample_rate = sample_rate;
    ctx->dict = NULL;
    ks_init_aliases(ctx);
    
    return ctx;
}

K k_get_var_str(ks_ctx *ctx, const char *name) {
    ks_dict_entry *curr = ctx->dict;
    while (curr) {
        if (strcmp(curr->name, name) == 0) return curr->val;
        curr = curr->next;
    }
    return NULL;
}

void k_set_var_str(ks_ctx *ctx, const char *name, K x) {
    ks_dict_entry *curr = ctx->dict;
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            if (curr->val) k_free(ctx, curr->val);
            curr->val = x;
            return;
        }
        curr = curr->next;
    }
    ks_dict_entry *entry = malloc(sizeof(ks_dict_entry));
    entry->name = strdup(name);
    entry->val = x;
    entry->next = ctx->dict;
    ctx->dict = entry;
}

void ks_clear_vars(ks_ctx *ctx) {
    if (!ctx) return;
    ks_dict_entry *curr = ctx->dict;
    while (curr) {
        ks_dict_entry *next = curr->next;
        if (curr->val) k_free(ctx, curr->val);
        free(curr->name);
        free(curr);
        curr = next;
    }
    ctx->dict = NULL;
    ks_init_aliases(ctx);
    ctx->args[0] = NULL;
    ctx->args[1] = NULL;
}

void ks_destroy(ks_ctx *ctx) {
    if (!ctx) return;
    ks_clear_vars(ctx);
    free(ctx->arena_base);
    free(ctx);
}

/* --- K Lifecycle --- */

/* Alignment for the bump allocator — double is 8 bytes, that's our ceiling. */
#define KS_ALIGN 8
#define KS_ALIGN_UP(n) (((n) + (KS_ALIGN-1)) & ~(size_t)(KS_ALIGN-1))

/* Arena-allocated K: lives only for the duration of the current ks_eval call.
   k_free is a no-op; the arena is reset as a whole in ks_eval. */
K k_new(ks_ctx *ctx, int n) {
    if (n < 0) n = 0;
    size_t sz = KS_ALIGN_UP(sizeof(struct { int r, n; double f[]; }) + sizeof(double) * n);

    if (ctx->arena_ptr + sz > ctx->arena_end) {
        ctx->last_status = KS_ERR_OOM;
        longjmp(ctx->recover, 1);
    }

    K x = (K)ctx->arena_ptr;
    ctx->arena_ptr += sz;
    x->r = 1; x->n = n;
    return x;
}

/* Persistent K: malloc'd, survives across ks_eval calls.
   Used for vars[] (A-Z) only. Freed explicitly by ks_clear_vars/ks_destroy. */
K k_new_perm(ks_ctx *ctx, int n) {
    if (n < 0) n = 0;
    size_t sz = sizeof(struct { int r, n; double f[]; }) + sizeof(double) * n;
    K x = malloc(sz);
    if (!x) {
        /* k_new_perm is called both inside ks_eval (from the assignment
           operator) and outside it (from bind_scalar). longjmping to
           ctx->recover when called outside eval is UB — recover hasn't
           been initialised by setjmp yet. Return NULL instead and let
           the call site handle it. */
        ctx->last_status = KS_ERR_OOM;
        return NULL;
    }
    x->r = 1; x->n = n;
    return x;
}

static int k_is_arena_owned(ks_ctx *ctx, K x) {
    return ctx && x &&
           (char *)x >= ctx->arena_base &&
           (char *)x <  ctx->arena_end;
}

/* k_free: arena objects are owned by the arena, so this is a no-op for them.
   Persistent objects still use refcounts so external users (audio voices,
   embedders) can hold a variable buffer after its var slot is overwritten. */
void k_free(ks_ctx *ctx, K x) {
    if (!x) return;
    if (k_is_arena_owned(ctx, x)) return;
    if (--x->r <= 0) free(x);
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
    (void)ks_bind_vector(ctx, name, &val, 1);
}

ks_status ks_bind_vector(ks_ctx *ctx, char name, const double *values,
                         size_t length) {
    if (!ctx || name < 'A' || name > 'Z' ||
        (length > 0 && !values) || length > 1000000) {
        if (ctx) ctx->last_status = KS_ERR_INVALID_ARGS;
        return KS_ERR_INVALID_ARGS;
    }

    K x = k_new_perm(ctx, (int)length);
    if (!x) return KS_ERR_OOM;
    if (length) memcpy(x->f, values, length * sizeof(double));

    int i = name - 'A';
    char vn[2] = {(char)(i+'A'), 0};
    K old = k_get_var_str(ctx, vn);
    k_set_var_str(ctx, vn, x);
    k_free(ctx, old);
    ctx->last_status = KS_OK;
    return KS_OK;
}

/* k_get returns an arena-allocated copy of the var's value.
   The perm object in vars[] is left untouched; the copy lives for
   the duration of the current eval. */
K k_get(ks_ctx *ctx, char name) {
    char vn[2] = {name, 0};
    K v = k_get_var_str(ctx, vn);
    if (!v) return NULL;
    /* already got v */
    if (k_is_func(v)) {
        /* Functions: arena-copy the func object so the body pointer
           still points into the perm allocation's flex array. */
        int len = strlen((char*)v->f) + 1;
        int ndoubles = (len + sizeof(double) - 1) / sizeof(double);
        K x = k_new(ctx, ndoubles);
        x->n = -1;
        memcpy(x->f, v->f, len);
        return x;
    }
    K x = k_new(ctx, v->n);
    memcpy(x->f, v->f, v->n * sizeof(double));
    return x;
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

typedef enum { TOK_EOF, TOK_SYM, TOK_ID, TOK_NUM, TOK_FUNC } TokenType;
typedef struct {
    TokenType type;
    char c_val;
    char str_val[32];
    K k_val;
} Token;

K e_tok(ks_ctx *ctx, Token **t);
K expr_tok(ks_ctx *ctx, Token **t);
K atom_tok(ks_ctx *ctx, Token **t);
int ks_lex(ks_ctx *ctx, const char *code, Token *tokens, int max_tokens);

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

    if (nargs > 0 && call_args[0]) ctx->args[0] = call_args[0];
    if (nargs > 1 && call_args[1]) ctx->args[1] = call_args[1];

    Token tokens[4096];
    ks_lex(ctx, body, tokens, 4096);
    Token *t = tokens;
    K result = e_tok(ctx, &t);

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
                acc = safe_val(pow(acc, b->f[i]));
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

/* --- Verbs & Operators ---
 *
 * mo(char c, K b)       : Monadic verbs (single argument, e.g., 's' sine).
 * dy(char c, K a, K b)  : Dyadic verbs (two arguments, e.g., 't' wavetable).
 * scan(char op, K b)    : Adverb operations (e.g., '+\' running sum).
 *
 * Execution scales to the max length of the input vectors. If vectors
 * are unequal, the shorter vector cycles. All loops are bounds-checked
 * by the GAS_CHECK macro to prevent audio thread lockups.
 */

K mo(ks_ctx *ctx, char c, K b) {
    if (!b) return NULL;

    if (c >= 'A' && c <= 'Z') {
        char vn[2] = {c, 0};
        K var = k_get_var_str(ctx, vn);
        if (k_is_func(var)) {
            K call_args[1] = {b};
            return k_call(ctx, var, call_args, 1);
        }
    }

    K x;

    if (c == '!') {
        int n = (int)b->f[0]; k_free(ctx, b);
        if (n < 0 || n > 1000000) { ctx->last_status = KS_ERR_INVALID_ARGS; longjmp(ctx->recover, 1); }
        GAS_CHECK(ctx, n);
        x = k_new(ctx, n);
        for (int j = 0; j < n; j++) x->f[j] = (double)j;
        return x;
    }

    if (c == '~') {
        int n = (int)b->f[0]; k_free(ctx, b);
        if (n < 1 || n > 1000000) return k_new(ctx, 0);
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
            case 'p': x->f[i] = (v == 0) ? ctx->sample_rate : M_PI * v; break;
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
                /* Monadic b: fixed-pitch buzz at 110 Hz (default organ bass).
                   For pitched use, prefer dyadic form: freq b V */
                double ff[] = {2.43, 3.01, 3.52, 4.11, 5.23, 6.78};
                double phase_inc = 110.0 * (2.0 * M_PI / ctx->sample_rate);
                double ss = 0;
                for (int j = 0; j < 6; j++)
                    ss += (sin(i * phase_inc * ff[j]) > 0) ? 1.0 : -1.0;
                x->f[i] = ss / 6.0;
                break;
            }
            case 'u': {
                /* Monadic u: fixed 10-sample anti-click ramp.
                   For a longer ramp, use dyadic form: N u V */
                x->f[i] = (i < 10) ? (double)i / 10.0 : 1.0;
                break;
            }
            case 'n': x->f[i] = 440.0 * pow(2.0, (v - 69.0) / 12.0); break;
            default: x->f[i] = v; break;
        }
    }
    k_free(ctx, b); return x;
}

K dy(ks_ctx *ctx, char c, K a, K b) {
    if (!a || !b) { k_free(ctx, a); k_free(ctx, b); return NULL; }

    if (c >= 'A' && c <= 'Z') {
        char vn[2] = {c, 0};
        K var = k_get_var_str(ctx, vn);
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
            K nv = k_get_var_str(ctx, "N");
            n_out = (nv && nv->n > 0) ? (int)nv->f[0] : 0;
        }
        int tbl_len = a->n;
        if (n_out < 1 || tbl_len < 1) { k_free(ctx, a); k_free(ctx, b); return k_new(ctx, 0); }

        GAS_CHECK(ctx, n_out);
        double phase_inc = freq_hz * (double)tbl_len / ctx->sample_rate;
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

    if (c == 'b') {
        /* Dyadic b: freq b signal — pitched band-limited buzz at freq Hz.
           Same 6-oscillator metallic cluster as monadic b, tuned to freq.
           Output length = b->n (the signal vector). */
        double freq = (a->n > 0) ? a->f[0] : 110.0;
        if (freq < 1.0) freq = 1.0;
        double phase_inc = freq * (2.0 * M_PI / ctx->sample_rate);
        double ff[] = {2.43, 3.01, 3.52, 4.11, 5.23, 6.78};
        GAS_CHECK(ctx, b->n);
        x = k_new(ctx, b->n);
        for (int i = 0; i < b->n; i++) {
            double ss = 0;
            for (int j = 0; j < 6; j++)
                ss += (sin(i * phase_inc * ff[j]) > 0) ? 1.0 : -1.0;
            x->f[i] = ss / 6.0;
        }
        k_free(ctx, a); k_free(ctx, b); return x;
    }

    if (c == 'u') {
        /* Dyadic u: N u signal — anti-click ramp over first N samples.
           Ramps from 0 to 1 over N samples then holds at 1.0.
           N < 1 is treated as 1; use N=0 to effectively bypass. */
        int ramp = (a->n > 0 && a->f[0] >= 1.0) ? (int)a->f[0] : 1;
        GAS_CHECK(ctx, b->n);
        x = k_new(ctx, b->n);
        for (int i = 0; i < b->n; i++)
            x->f[i] = (i < ramp) ? (double)i / (double)ramp : 1.0;
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
    }

    if (c == 'g') {
        GAS_CHECK(ctx, b->n);
        x = k_new(ctx, b->n);
        double s0 = 0.0, s1 = 0.0;
        double static_f = a->f[0];
        double q_val    = (a->n >= 2) ? a->f[1] : 0.5;
        double damp     = 1.0 / (q_val < 0.01 ? 0.01 : q_val);
        for (int i = 0; i < b->n; i++) {
            double f_hz    = (a->n == b->n) ? a->f[i] : static_f;
            double f_coeff = 2.0 * sin(M_PI * f_hz / ctx->sample_rate);
            if (f_coeff > 1.99) f_coeff = 1.99;
            double hp = b->f[i] - s0 - damp * s1;
            s1 += f_coeff * hp; s0 += f_coeff * s1;
            s0 = safe_val(s0); s1 = safe_val(s1);
            x->f[i] = s1;
        }
        k_free(ctx, a); k_free(ctx, b); return x;
    }

    if (c == 'y') {
        int dd   = (int)a->f[0];
        double g = (a->n > 1) ? a->f[1] : 0.4;
        GAS_CHECK(ctx, b->n);
        x = k_new(ctx, b->n);
        for (int i = 0; i < b->n; i++) {
            double delayed = (i >= dd) ? x->f[i-dd] : 0;
            x->f[i] = safe_val(b->f[i] + (g * delayed));
        }
        k_free(ctx, a); k_free(ctx, b); return x;
    }

    if (c == '#') {
        int n = (int)a->f[0];
        if (n < 0 || n > 1000000) { ctx->last_status = KS_ERR_INVALID_ARGS; k_free(ctx, a); k_free(ctx, b); longjmp(ctx->recover, 1); }
        GAS_CHECK(ctx, n);
        x = k_new(ctx, n);
        if (b->n > 0) for (int i = 0; i < n; i++) x->f[i] = b->f[i % b->n];
        k_free(ctx, a); k_free(ctx, b); return x;
    }

    if (c == 'D') {
        int dd   = (int)a->f[0];
        double g = (a->n > 1) ? a->f[1] : 0.4;
        GAS_CHECK(ctx, b->n);
        x = k_new(ctx, b->n);
        for (int i = 0; i < b->n; i++) {
            double delayed = (i >= dd) ? x->f[i-dd] : 0;
            x->f[i] = safe_val(b->f[i] + (g * delayed));
        }
        k_free(ctx, a); k_free(ctx, b); return x;
    }

    if (c == 'S') {
        int start = 0;
        int len = 0;
        if (b->n >= 1) start = (int)b->f[0];
        if (b->n >= 2) len = (int)b->f[1];
        if (len <= 0) { k_free(ctx, a); k_free(ctx, b); return k_new(ctx, 0); }
        GAS_CHECK(ctx, len);
        x = k_new(ctx, len);
        for (int i = 0; i < len; i++) {
            int idx = start + i;
            x->f[i] = (idx >= 0 && idx < a->n) ? a->f[idx] : 0.0;
        }
        k_free(ctx, a); k_free(ctx, b); return x;
    }

    if (c == 'Z') {
        double speed = (b->n > 0) ? b->f[0] : 1.0;
        if (speed <= 0.0) speed = 1.0;
        int len = (int)((double)a->n / speed);
        GAS_CHECK(ctx, len);
        x = k_new(ctx, len);
        for(int i=0; i<len; i++) {
            double pos = i * speed;
            int idx = (int)pos;
            double frac = pos - idx;
            double v1 = (idx >= 0 && idx < a->n) ? a->f[idx] : 0.0;
            double v2 = (idx+1 >= 0 && idx+1 < a->n) ? a->f[idx+1] : 0.0;
            x->f[i] = v1 * (1.0 - frac) + v2 * frac;
        }
        k_free(ctx, a); k_free(ctx, b); return x;
    }

    if (c == ',') {
        int n = a->n + b->n;
        GAS_CHECK(ctx, n);
        x = k_new(ctx, n);
        memcpy(x->f, a->f, a->n * sizeof(double));
        memcpy(x->f + a->n, b->f, b->n * sizeof(double));
        k_free(ctx, a); k_free(ctx, b); return x;
    }

    /* arithmetic: element-wise, length = max of inputs, shorter side cycles */
    {
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

/* --- Parser & Evaluator ---
 * The parser uses tokens.
 */

K e_tok(ks_ctx *ctx, Token **t);
K expr_tok(ks_ctx *ctx, Token **t);

int ks_lex(ks_ctx *ctx, const char *code, Token *tokens, int max_tokens) {
    int count = 0;
    const char *p = code;
    while (*p && count < max_tokens - 1) {
        while (*p == ' ') p++;
        if (*p == '/') {
            while (*p && *p != '\n') p++;
            if (*p == '\n') p++;
            continue;
        }
        if (!*p) break;

        if (*p == '(' || *p == ')' || *p == ';' || *p == ':' || *p == '{' || *p == '}' || *p == '\\') {
            tokens[count].type = TOK_SYM;
            tokens[count].c_val = *p;
            count++;
            if (*p == '{') {
                p++;
                const char *start = p;
                int depth = 1;
                while (*p && depth > 0) {
                    if (*p == '{') depth++;
                    else if (*p == '}') depth--;
                    p++;
                }
                if (depth == 0) {
                    int len = (p - 1) - start;
                    char *body = malloc(len + 1);
                    memcpy(body, start, len);
                    body[len] = '\0';
                    tokens[count-1].type = TOK_FUNC;
                    tokens[count-1].k_val = k_func(ctx, body);
                    free(body);
                    continue; // Skip the standard p++ below
                }
            } else {
                p++;
                continue;
            }
        }

        if ((*p >= '0' && *p <= '9') || (*p == '.' && p[1] >= '0' && p[1] <= '9')) {
            double buf[1024]; int n = 0;
            char *ptr = (char*)p;
            while (n < 1024) {
                buf[n++] = strtod(ptr, &ptr);
                char *after = ptr;
                char *peek = ptr;
                while (*peek == ' ') peek++;
                int had_space = (peek != after);
                if (*peek >= '0' && *peek <= '9') { ptr = peek; continue; }
                if (*peek == '-' && (peek[1] >= '0' && peek[1] <= '9') && had_space) { ptr = peek; continue; }
                if (had_space && ((*peek >= 'A' && *peek <= 'Z') || (*peek >= 'a' && *peek <= 'z'))) {
                    // Peek ahead to see if it's a valid registered variable
                    // Wait! The legacy parser specifically checks if it's a single-letter variable not followed by ':'
                    // Here, we can just use the standard lexing loop to handle array construction in atom().
                    // It's cleaner to let `atom()` build the array by consuming TOK_NUM and TOK_ID tokens.
                    // But legacy ksynth builds the array natively inside the number parser!
                    // Let's preserve the exact legacy array logic here for numbers:
                    if (peek[1] != ':') {
                        // In old parser, it only looks at 'A'-'Z' here. Let's do the same for legacy compatibility.
                        if (*peek >= 'A' && *peek <= 'Z') {
                            char vn[2] = {*peek, 0};
                            K v = k_get_var_str(ctx, vn);
                            if (v && v->n == 1) { buf[n++] = v->f[0]; ptr = peek + 1; continue; }
                        }
                    }
                }
                break;
            }
            p = ptr;
            K x = k_new(ctx, n); memcpy(x->f, buf, n * sizeof(double));
            tokens[count].type = TOK_NUM;
            tokens[count].k_val = x;
            count++;
            continue;
        }

        // Greedy Fallback Lexing for Identifiers/Verbs
        if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || *p == '_' || strchr("+-*%^&|<>=,#", *p)) {
            char buf[32]; int wl = 0;
            const char *ptr = p;
            
            // If it's a math operator, it's just 1 char.
            if (strchr("+-*%^&|<>=,#", *ptr)) {
                buf[wl++] = *ptr++;
            } else {
                while ((*ptr >= 'A' && *ptr <= 'Z') || (*ptr >= 'a' && *ptr <= 'z') || (*ptr >= '0' && *ptr <= '9') || *ptr == '_') {
                    if (wl < 31) buf[wl++] = *ptr;
                    ptr++;
                }
            }
            buf[wl] = 0;

            if (wl > 1) {
                // Check if the whole word is a registered variable
                if (k_get_var_str(ctx, buf)) {
                    tokens[count].type = TOK_ID;
                    strcpy(tokens[count].str_val, buf);
                    count++;
                    p = ptr;
                    continue;
                }
                // (Future: check multi-letter verbs here. For now, fall through).
            }

            // Fallback: emit only the first character!
            tokens[count].type = TOK_ID;
            tokens[count].str_val[0] = *p;
            tokens[count].str_val[1] = '\0';
            count++;
            p++;
            continue;
        }

        // Catch-all
        tokens[count].type = TOK_ID;
        tokens[count].str_val[0] = *p;
        tokens[count].str_val[1] = '\0';
        count++;
        p++;
    }
    tokens[count].type = TOK_EOF;
    return count;
}

K atom_tok(ks_ctx *ctx, Token **t) {
    Token *tk = *t;
    if (tk->type == TOK_EOF || (tk->type == TOK_SYM && (tk->c_val == ')' || tk->c_val == ';'))) return NULL;

    if (tk->type == TOK_SYM && tk->c_val == '(') {
        (*t)++; K x = e_tok(ctx, t);
        if ((*t)->type == TOK_SYM && (*t)->c_val == ')') (*t)++;
        return x;
    }

    if (tk->type == TOK_FUNC || tk->type == TOK_NUM) {
        K x = tk->k_val;
        (*t)++;
        return x; // (Arena lifetime, so it's safe to return)
    }

    // It's a TOK_ID
    char *word = tk->str_val;
    (*t)++;
    
    // Is it an assignment? A:2
    if ((*t)->type == TOK_SYM && (*t)->c_val == ':') {
        (*t)++; K x = expr_tok(ctx, t);
        if (x) {
            K perm;
            if (k_is_func(x)) {
                int len = strlen((char*)x->f) + 1;
                int ndoubles = (len + sizeof(double) - 1) / sizeof(double);
                perm = k_new_perm(ctx, ndoubles);
                if (!perm) longjmp(ctx->recover, 1);
                perm->n = -1;
                memcpy(perm->f, x->f, len);
            } else {
                perm = k_new_perm(ctx, x->n);
                if (!perm) longjmp(ctx->recover, 1);
                memcpy(perm->f, x->f, x->n * sizeof(double));
            }
            k_set_var_str(ctx, word, perm);
        }
        return x;
    }
    
    // Is it a variable lookup?
    K first = k_get_var_str(ctx, word);
    if (first) {
        // Build array if followed by other single-value variables
        if (first->n != 1) {
            if (k_is_func(first)) {
                int len = strlen((char*)first->f) + 1;
                int ndoubles = (len + sizeof(double) - 1) / sizeof(double);
                K clone = k_new(ctx, ndoubles);
                clone->n = -1;
                memcpy(clone->f, first->f, len);
                return clone;
            } else {
                K clone = k_new(ctx, first->n);
                memcpy(clone->f, first->f, first->n * sizeof(double));
                return clone;
            }
        }
        double buf[1024]; int n = 0;
        buf[n++] = first->f[0];
        
        // Peek ahead for A B C style array construction
        while ((*t)->type == TOK_ID) {
            if (((*t) + 1)->type == TOK_SYM && ((*t) + 1)->c_val == ':') break;
            K v = k_get_var_str(ctx, (*t)->str_val);
            if (!v || v->n != 1) break;
            buf[n++] = v->f[0];
            (*t)++;
        }
        
        K x = k_new(ctx, n);
        memcpy(x->f, buf, n * sizeof(double));
        return x;
    }
    
    // Implicit args
    if (strcmp(word, "x") == 0) return ctx->args[0] ? ctx->args[0] : k_new(ctx, 0);
    if (strcmp(word, "y") == 0) return ctx->args[1] ? ctx->args[1] : k_new(ctx, 0);
    
    // Monadic/Adverb evaluation
    // Unary minus
    if (strcmp(word, "-") == 0) {
        K arg = expr_tok(ctx, t);
        if (!arg) return NULL;
        K x = k_new(ctx, arg->n);
        for(int i=0; i<arg->n; i++) x->f[i] = -arg->f[i];
        k_free(ctx, arg);
        return x;
    }

    int is_scan = 0;
    if ((*t)->type == TOK_SYM && (*t)->c_val == '\\') {
        is_scan = 1;
        (*t)++;
    }
    K arg = expr_tok(ctx, t);
    if (is_scan) return scan(ctx, word[0], arg); // Assuming length 1 for legacy
    else return mo(ctx, word[0], arg);
}

K expr_tok(ks_ctx *ctx, Token **t) {
    K x = atom_tok(ctx, t);
    
    // Function calls
    if (k_is_func(x) && (*t)->type != TOK_EOF && !((*t)->type == TOK_SYM && ((*t)->c_val == ')' || (*t)->c_val == ';' || (*t)->c_val == '}'))) {
        // Is the next token an operator?
        int is_operator = 0;
        if ((*t)->type == TOK_ID && strlen((*t)->str_val) == 1) {
            if (strchr("+-*%^&|<>=,#osfzt haqle rpciw dvmbu jkn gSZD", (*t)->str_val[0])) {
                is_operator = 1;
            }
        }
        if (!is_operator) {
            K arg = expr_tok(ctx, t);
            K call_args[1] = {arg};
            K result = k_call(ctx, x, call_args, 1);
            // k_free(ctx, x); // Wait, x is arena allocated or from dictionary. We don't free dictionary variables.
            // In legacy, x from atom() was arena allocated (e.g. from {}). But what if it's from dictionary?
            // Actually legacy ksynth didn't free dictionary functions here. Wait, k_call doesn't care.
            return result;
        }
    }
    
    if ((*t)->type == TOK_EOF || ((*t)->type == TOK_SYM && ((*t)->c_val == ')' || (*t)->c_val == ';' || (*t)->c_val == '}'))) return x;
    
    // Dyadic operator
    if ((*t)->type == TOK_ID) {
        char op = (*t)->str_val[0];
        (*t)++;
        return dy(ctx, op, x, expr_tok(ctx, t));
    }
    
    return x;
}

K e_tok(ks_ctx *ctx, Token **t) {
    K x = expr_tok(ctx, t);
    while ((*t)->type == TOK_SYM && (*t)->c_val == ';') {
        (*t)++;
        if (x && !k_is_func(x)) { } // no-op for arena
        if ((*t)->type == TOK_EOF || ((*t)->type == TOK_SYM && ((*t)->c_val == ')' || (*t)->c_val == '}'))) return k_new(ctx, 0);
        x = expr_tok(ctx, t);
    }
    return x;
}



/* --- Missing Helpers --- */


K k_from_f64(ks_ctx *ctx, int n, const double *ptr) {
    K x = k_new(ctx, n);
    if (x && ptr) {
        GAS_CHECK(ctx, n);
        memcpy(x->f, ptr, n * sizeof(double));
    }
    return x;
}

K k_from_f32(ks_ctx *ctx, int n, const float *ptr) {
    K x = k_new(ctx, n);
    if (x && ptr) {
        GAS_CHECK(ctx, n);
        for (int i = 0; i < n; i++) x->f[i] = (double)ptr[i];
    }
    return x;
}

K k_from_i32(ks_ctx *ctx, int n, const int *ptr) {
    K x = k_new(ctx, n);
    if (x && ptr) {
        GAS_CHECK(ctx, n);
        for (int i = 0; i < n; i++) x->f[i] = (double)ptr[i];
    }
    return x;
}

int k_copy_to_f64(K x, double *out, int max_n) {
    if (!x || !out || max_n <= 0 || x->n <= 0) return 0;
    int n = x->n < max_n ? x->n : max_n;
    memcpy(out, x->f, (size_t)n * sizeof(double));
    return n;
}

int k_copy_to_f32(K x, float *out, int max_n) {
    if (!x || !out || max_n <= 0 || x->n <= 0) return 0;
    int n = x->n < max_n ? x->n : max_n;
    for (int i = 0; i < n; i++) out[i] = (float)x->f[i];
    return n;
}

int k_copy_to_i32(K x, int *out, int max_n) {
    if (!x || !out || max_n <= 0 || x->n <= 0) return 0;
    int n = x->n < max_n ? x->n : max_n;
    for (int i = 0; i < n; i++) out[i] = (int)x->f[i];
    return n;
}

void bind_array_f64(ks_ctx *ctx, char name, int n, const double *ptr) {
    ks_bind_vector(ctx, name, ptr, n);
}

void bind_array_f32(ks_ctx *ctx, char name, int n, const float *ptr) {
    K x = k_from_f32(ctx, n, ptr);
    char vn[2] = {name, 0};
    if (x) {
        K perm = k_new_perm(ctx, x->n);
        if (perm) {
            memcpy(perm->f, x->f, x->n * sizeof(double));
            k_set_var_str(ctx, vn, perm);
        }
    }
}

void bind_array_i32(ks_ctx *ctx, char name, int n, const int *ptr) {
    K x = k_from_i32(ctx, n, ptr);
    char vn[2] = {name, 0};
    if (x) {
        K perm = k_new_perm(ctx, x->n);
        if (perm) {
            memcpy(perm->f, x->f, x->n * sizeof(double));
            k_set_var_str(ctx, vn, perm);
        }
    }
}

/* --- Public API --- */

static K k_clone_owned(ks_ctx *ctx, K source) {
    if (!source) return NULL;
    if (k_is_func(source)) {
        int len = (int)strlen((char *)source->f) + 1;
        int ndoubles = (len + (int)sizeof(double) - 1) / (int)sizeof(double);
        K copy = k_new_perm(ctx, ndoubles);
        if (!copy) return NULL;
        copy->n = -1;
        memcpy(copy->f, source->f, (size_t)len);
        return copy;
    }

    K copy = k_new_perm(ctx, source->n);
    if (!copy) return NULL;
    if (source->n > 0) {
        memcpy(copy->f, source->f, (size_t)source->n * sizeof(double));
    }
    return copy;
}

K ks_eval(ks_ctx *ctx, const char *code, size_t len) {
    if (!ctx || !code) return NULL;

    ctx->last_status = KS_OK;
    ctx->gas_used = 0;

    /* Save arena position — on longjmp or normal return we reset to here,
       reclaiming all temporaries allocated during this eval in one shot. */
    char *arena_checkpoint = ctx->arena_ptr;

    char *buf = malloc(len + 1);
    if (!buf) {
        ctx->last_status = KS_ERR_OOM;
    } else {
        memcpy(buf, code, len);
        buf[len] = '\0';
    }

    K result = NULL;
    if (buf && setjmp(ctx->recover) == 0) {
        Token tokens[4096];
        ks_lex(ctx, buf, tokens, 4096);
        Token *t = tokens;
        result = e_tok(ctx, &t);
        if (result) result = k_clone_owned(ctx, result);
    }
    free(buf);
    /* Both the success and longjmp paths fall through here.
       Reset the arena — all temporaries are gone. */
    ctx->arena_ptr  = arena_checkpoint;
    ctx->args[0]    = ctx->args[1] = NULL; /* were arena ptrs, now dangling */

    /* The returned object is an owned copy. The caller releases it with
       k_free(); all evaluator intermediates were reclaimed above. */
    return result;
}

void p(ks_ctx *ctx, K x) {
    (void)ctx;
    if (!x) { printf("(null)\n"); return; }
    if (k_is_func(x)) { printf("{%s}\n", k_func_body(x)); return; }
    if (x->n == 0) { printf("()\n"); return; }
    if (x->n == 1) { printf("%.6g\n", x->f[0]); return; }
    /* Print up to 8 elements; indicate truncation if longer */
    int show = x->n < 8 ? x->n : 8;
    printf("[");
    for (int i = 0; i < show; i++) printf("%s%.6g", i ? " " : "", x->f[i]);
    if (x->n > show) printf(" ... (%d total)", x->n);
    printf("]\n");
}

/*
Copyright (c) 2026 octetta / Joseph Stewart
MIT license at https://github.com/octetta/k-synth
*/

void print_lex(ks_ctx *ctx, const char *c) {
    Token tokens[100];
    int n = ks_lex(ctx, c, tokens, 100);
    for(int i=0; i<n; i++) {
        if(tokens[i].type == TOK_ID) printf("ID: %s\n", tokens[i].str_val);
        else if(tokens[i].type == TOK_SYM) printf("SYM: %c\n", tokens[i].c_val);
        else printf("TYPE: %d\n", tokens[i].type);
    }
}
