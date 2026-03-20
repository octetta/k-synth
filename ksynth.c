#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ksynth.h"

K vars[26] = {0};   // A-Z user variables
K args[2] = {0};    // x, y function arguments

/* --- Safe Value Helper --- */

static inline double safe_val(double v) {
    if (isnan(v) || isinf(v)) return 0.0;
    if (v > 1e6) return 1e6;
    if (v < -1e6) return -1e6;
    return v;
}

/* --- Lifecycle --- */

K k_new(int n) {
    if (n < 0) n = 0;
    K x = malloc(sizeof(*x) + sizeof(double) * n);
    if (!x) return NULL;
    x->r = 1; x->n = n; return x;
}

void k_free(K x) { if (x && !--x->r) free(x); }

K k_view(int n, double *ptr) {
    K x = k_new(n);
    if (x && ptr) memcpy(x->f, ptr, n * sizeof(double));
    return x;
}

void bind_scalar(char name, double val) {
    if (name < 'A' || name > 'Z') return;
    int i = name - 'A';
    K x = k_new(1); x->f[0] = val;
    if (vars[i]) k_free(vars[i]);
    vars[i] = x;
}

K k_get(char name) {
    if (name < 'A' || name > 'Z' || !vars[name - 'A']) return NULL;
    K v = vars[name - 'A'];
    v->r++; return v;
}

/* --- Function Support --- */

K k_func(char *body) {
    int len = strlen(body) + 1;
    int ndoubles = (len + sizeof(double) - 1) / sizeof(double);
    K x = k_new(ndoubles);
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

K k_call(K fn, K *call_args, int nargs) {
    if (!k_is_func(fn)) return NULL;

    char *body = k_func_body(fn);
    if (!body) return NULL;

    int uses_x = (strchr(body, 'x') != NULL);
    int uses_y = (strchr(body, 'y') != NULL);

    int required = 0;
    if (uses_y) required = 2;
    else if (uses_x) required = 1;

    if (nargs < required) {
        fprintf(stderr, "Error: function requires %d argument%s, got %d\n",
                required, required == 1 ? "" : "s", nargs);
        return k_new(0);
    }

    K old_x = args[0];
    K old_y = args[1];

    args[0] = k_new(0);
    args[1] = k_new(0);

    if (nargs > 0 && call_args[0]) {
        k_free(args[0]);
        call_args[0]->r++;
        args[0] = call_args[0];
    }
    if (nargs > 1 && call_args[1]) {
        k_free(args[1]);
        call_args[1]->r++;
        args[1] = call_args[1];
    }

    char *s = body;
    K result = e(&s);

    if (args[0]) k_free(args[0]);
    if (args[1]) k_free(args[1]);

    args[0] = old_x;
    args[1] = old_y;

    return result;
}

/* --- Scan Adverb --- */

K scan(char op, K b) {
    if (!b || b->n < 1) return b;
    K x = k_new(b->n);
    double acc;

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
            fprintf(stderr, "Warning: scan not supported for '%c'\n", op);
            memcpy(x->f, b->f, b->n * sizeof(double));
            break;
    }

    k_free(b);
    return x;
}

/* --- Verbs & Operators --- */

K mo(char c, K b) {
    if (!b) return NULL;

    if (c >= 'A' && c <= 'Z') {
        K var = vars[c - 'A'];
        if (k_is_func(var)) {
            K call_args[1] = {b};
            return k_call(var, call_args, 1);
        }
    }

    K x;

    /* --- Scalar-consuming verbs (take N, return N-element vector) --- */

    if (c == '!') {
        int n = (int)b->f[0]; k_free(b);
        x = k_new(n);
        for (int j = 0; j < n; j++) x->f[j] = (double)j;
        return x;
    }

    /* ~ : phase ramp 0 .. 2π*(N-1)/N */
    if (c == '~') {
        int n = (int)b->f[0]; k_free(b);
        if (n < 1) return k_new(0);
        x = k_new(n);
        double twopi = 6.28318530717958647692;
        for (int j = 0; j < n; j++) x->f[j] = twopi * (double)j / (double)n;
        return x;
    }

    /* --- Reduction verbs (return scalar) --- */

    if (c == '+') {
        double t = 0;
        for (int i = 0; i < b->n; i++) t += b->f[i];
        x = k_new(1); x->f[0] = t;
        k_free(b); return x;
    }

    if (c == '>') {
        double m = 0;
        for (int i = 0; i < b->n; i++) if (fabs(b->f[i]) > m) m = fabs(b->f[i]);
        x = k_new(1); x->f[0] = m;
        k_free(b); return x;
    }

    /* w : peak normalize */
    if (c == 'w') {
        double pk = 0.0;
        for (int i = 0; i < b->n; i++) if (fabs(b->f[i]) > pk) pk = fabs(b->f[i]);
        x = k_new(b->n);
        double scale = (pk > 1e-10) ? 1.0 / pk : 0.0;
        for (int i = 0; i < b->n; i++) x->f[i] = b->f[i] * scale;
        k_free(b); return x;
    }

    /* --- Stereo channel extraction --- */

    if (c == 'j') {
        if (b->n < 2) { k_free(b); return k_new(0); }
        x = k_new(b->n / 2);
        for (int i = 0; i < x->n; i++) x->f[i] = b->f[i*2];
        k_free(b); return x;
    }

    if (c == 'k') {
        if (b->n < 2) { k_free(b); return k_new(0); }
        x = k_new(b->n / 2);
        for (int i = 0; i < x->n; i++) x->f[i] = b->f[i*2+1];
        k_free(b); return x;
    }

    /* v : quantize to 4 levels (monadic default) */
    if (c == 'v') {
        x = k_new(b->n);
        for (int i = 0; i < b->n; i++)
            x->f[i] = floor(b->f[i] * 4.0) / 4.0;
        k_free(b); return x;
    }

    /* --- Element-wise verbs --- */

    x = k_new(b->n);
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
            case 'p': {
                if (v == 0) x->f[i] = 44100;
                else x->f[i] = 3.14159265358979323846 * v;
                break;
            }
            /* i : reverse — formerly ~ */
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
            case 'u': {
                x->f[i] = (i < 10) ? (double)i / 10.0 : 1.0;
                break;
            }
            case 'n': {
                x->f[i] = 440.0 * pow(2.0, (v - 69.0) / 12.0);
                break;
            }
            default: x->f[i] = v; break;
        }
    }
    k_free(b); return x;
}

K dy(char c, K a, K b) {
    if (!a || !b) { k_free(a); k_free(b); return NULL; }

    if (c >= 'A' && c <= 'Z') {
        K var = vars[c - 'A'];
        if (k_is_func(var)) {
            K call_args[2] = {a, b};
            return k_call(var, call_args, 2);
        }
    }

    K x;

    /* z : stereo interleave */
    if (c == 'z') {
        int mn = (a->n < b->n) ? a->n : b->n;
        x = k_new(mn * 2);
        for (int i = 0; i < mn; i++) {
            x->f[i*2]   = a->f[i];
            x->f[i*2+1] = b->f[i];
        }
        k_free(a); k_free(b); return x;
    }

    /* o : additive outer-product-sum, equal amplitudes
     * a = phase vector [N], b = harmonic numbers [M]
     * result[i] = Σ_j sin(a[i] * b[j])
     * produces a wavetable of length N (one value per phase point)
     * all harmonics contribute equally — use $ for amplitude weighting */
    if (c == 'o') {
        x = k_new(a->n);
        for (int i = 0; i < a->n; i++) {
            double acc = 0.0;
            for (int j = 0; j < b->n; j++)
                acc += sin(a->f[i] * b->f[j]);
            x->f[i] = acc;
        }
        k_free(a); k_free(b); return x;
    }

    /* $ : weighted additive synthesis from amplitude spectrum
     * a = phase vector [N], b = amplitude array [M]
     * b[j] is the amplitude of harmonic (j+1)
     * result[i] = Σ_j b[j] * sin(a[i] * (j+1))
     * produces a wavetable of length N
     * example: P $ (1 .5 .333 .25) gives 4-harmonic sawtooth */
    if (c == '$') {
        x = k_new(a->n);
        for (int i = 0; i < a->n; i++) {
            double acc = 0.0;
            for (int j = 0; j < b->n; j++)
                acc += b->f[j] * sin(a->f[i] * (double)(j + 1));
            x->f[i] = acc;
        }
        k_free(a); k_free(b); return x;
    }

    /* t : wavetable DDS oscillator
     * a = wavetable vector (any length, one cycle)
     * b = freq_hz           — use vars[N] for sample count
     *   or [freq_hz  n]     — explicit sample count via comma: T t 330,D
     *
     * Example:
     *   N: 44100
     *   P: +\(1024#(6.28318%1024))
     *   T: s P
     *   W: w T t 440       / uses N for duration
     *   W: w T t 440,88200 / explicit duration
     */
    if (c == 't') {
        if (a->n < 1 || b->n < 1) { k_free(a); k_free(b); return k_new(0); }
        double freq_hz = b->f[0];
        int    n_out;
        if (b->n >= 2) {
            n_out = (int)b->f[1];
        } else {
            /* fall back to vars[N] */
            K nv = vars['N' - 'A'];
            n_out = (nv && nv->n > 0) ? (int)nv->f[0] : 0;
        }
        int tbl_len = a->n;
        if (n_out < 1 || tbl_len < 1) { k_free(a); k_free(b); return k_new(0); }

        double phase_inc = freq_hz * (double)tbl_len / 44100.0;
        double phase     = 0.0;
        x = k_new(n_out);

        for (int i = 0; i < n_out; i++) {
            while (phase >= tbl_len) phase -= tbl_len;
            while (phase <  0.0)    phase += tbl_len;
            int    idx  = (int)phase;
            double frac = phase - idx;
            int    idx2 = (idx + 1) % tbl_len;
            x->f[i] = a->f[idx] * (1.0 - frac) + a->f[idx2] * frac;
            phase += phase_inc;
        }
        k_free(a); k_free(b); return x;
    }

    /* v : quantize b to N levels, a = number of levels */
    if (c == 'v') {
        double levels = (a->n > 0 && a->f[0] > 0) ? a->f[0] : 4.0;
        x = k_new(b->n);
        for (int i = 0; i < b->n; i++)
            x->f[i] = floor(b->f[i] * levels) / levels;
        k_free(a); k_free(b); return x;
    }

    /* f : two-pole lowpass filter (Chamberlin simplified / cascaded one-pole)
     * a = [cutoff]  or  [cutoff resonance]
     *   cutoff    — normalised coefficient 0.0–0.95 (not Hz)
     *               approx: ct ≈ 2*sin(π*freq/SR) for freq << SR/2
     *               e.g. 0.1 ≈ 700 Hz, 0.3 ≈ 2.2 kHz, 0.7 ≈ 6.5 kHz
     *   resonance — 0.0 (none) to ~3.9 (near instability), default 0
     *               feedback is from the LP tap (b1), not the BP tap (b0)
     *               this gives a gentle shelf boost near cutoff rather than
     *               a sharp resonant peak — stable and musical, not Moog-style
     * b = signal vector
     *
     * Derived outputs in ksynth script:
     *   highpass (rs=0 only): HP: signal - (ct f signal)  — clean at zero resonance
     *                         with resonance: signal-LP has shelf artefacts, use with care
     *   bandpass: (hi f signal) - (lo f signal)  — works correctly at any resonance
     * usage: 0.2 f signal   or   0.2 1.5 f signal
     */
    if (c == 'f') {
        x = k_new(b->n); double b0 = 0, b1 = 0;
        for (int i = 0; i < b->n; i++) {
            double ct = (a->n > i) ? a->f[i] : a->f[0];
            double rs = (a->n >= 2) ? a->f[1] : 0.0;
            if (ct > 0.95) ct = 0.95;
            if (rs > 3.98) rs = 3.98;
            double in = b->f[i] - (rs * b1);
            b0 += ct * (in - b0); b1 += ct * (b0 - b1);
            b0 = safe_val(b0);
            b1 = safe_val(b1);
            x->f[i] = b1;
        }

    /* g : Chamberlin state-variable lowpass filter, Hz input
     * Same topology as f but cutoff is specified in Hz directly.
     * a = [freq_hz]  or  [freq_hz  q]
     *   freq_hz — cutoff frequency in Hz (0–20000)
     *   q       — resonance 0.01–~3.9, default 0.5 (Q=2, gentle peak)
     * b = signal vector
     * usage: 800 g signal   or   800 2.0 g signal
     * For a modulated cutoff, pass a vector of the same length as b:
     *   F: 200+600*(s P)    / LFO sweeping 200-800 Hz
     *   W: w F g signal
     */
    } else if (c == 'g') {
        x = k_new(b->n);
        double s0 = 0.0, s1 = 0.0;
        double static_f = a->f[0];
        double q_val    = (a->n >= 2) ? a->f[1] : 0.5;
        double damp     = 1.0 / (q_val < 0.01 ? 0.01 : q_val);

        for (int i = 0; i < b->n; i++) {
            double f_hz    = (a->n == b->n) ? a->f[i] : static_f;
            double f_coeff = 2.0 * sin(M_PI * f_hz / 44100.0);
            if (f_coeff > 1.99) f_coeff = 1.99;
            double hp = b->f[i] - s0 - damp * s1;
            s1 += f_coeff * hp;
            s0 += f_coeff * s1;
            s0 = safe_val(s0);
            s1 = safe_val(s1);
            x->f[i] = s1;
        }
    /* y : feedback delay, a = [samples] or [samples gain] */
    } else if (c == 'y') {
        int dd   = (int)a->f[0];
        double g = (a->n > 1) ? a->f[1] : 0.4;
        x = k_new(b->n);
        for (int i = 0; i < b->n; i++) {
            double delayed = (i >= dd) ? x->f[i-dd] : 0;
            x->f[i] = safe_val(b->f[i] + (g * delayed));
        }
    /* # : repeat/tile b to length a */
    } else if (c == '#') {
        int n = (int)a->f[0]; x = k_new(n);
        if (b->n > 0) for (int i = 0; i < n; i++) x->f[i] = b->f[i % b->n];
    /* , : concatenate */
    } else if (c == ',') {
        x = k_new(a->n + b->n);
        memcpy(x->f, a->f, a->n * sizeof(double));
        memcpy(x->f + a->n, b->f, b->n * sizeof(double));
    } else {
        int mn = a->n > b->n ? a->n : b->n; x = k_new(mn);
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
    }
    k_free(a); k_free(b); return x;
}

/* --- Parser --- */

K e(char **s);
K atom(char **s);

K expr(char **s) {
    K x = atom(s);
    while (**s == ' ') (*s)++;

    if (k_is_func(x) && **s && **s != '\n' && **s != ')' && **s != ';' && **s != '}' && **s != '/') {
        char peek = **s;
        int is_operator = (peek == '+' || peek == '-' || peek == '*' ||
                          peek == '%' || peek == '^' || peek == '&' ||
                          peek == '|' || peek == '<' || peek == '>' ||
                          peek == '=' || peek == ',' || peek == '#' ||
                          peek == 'o' || peek == '$' ||
                          peek == 'f' || peek == 'z' ||
                          peek == 's' || peek == 't' || peek == 'h' ||
                          peek == 'a' || peek == 'q' || peek == 'l' ||
                          peek == 'e' || peek == 'r' || peek == 'p' ||
                          peek == 'c' || peek == 'i' || peek == 'w' ||
                          peek == 'd' || peek == 'v' || peek == 'm' ||
                          peek == 'b' || peek == 'u' || peek == 'j' ||
                          peek == 'k' || peek == 'n' || peek == 'g');
        if (!is_operator) {
            K arg = expr(s);
            K call_args[1] = {arg};
            K result = k_call(x, call_args, 1);
            k_free(x);
            return result;
        }
    }

    if (!**s || **s == '\n' || **s == ')' || **s == ';' || **s == '}' || **s == '/') return x;
    char op = *(*s)++;
    return dy(op, x, expr(s));
}

K atom(char **s) {
    while (**s == ' ') (*s)++;
    /* / begins a line comment — skip to end of line, then continue */
    if (**s == '/') {
        while (**s && **s != '\n') (*s)++;
        if (**s == '\n') (*s)++;
        /* tail-call: try again on the next line */
        return atom(s);
    }
    if (!**s || **s == '\n' || **s == ')' || **s == ';') return NULL;

    if (**s == '(') {
        (*s)++; K x = e(s);
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
            K func = k_func(body);
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
            char *after = ptr;               /* position right after parsed number */
            char *peek  = ptr;
            while (*peek == ' ') peek++;
            /* K rule: '-' with whitespace before it = negation (continue vector)
             *         '-' flush against previous token = subtraction (stop)    */
            int had_space = (peek != after);
            if (*peek >= '0' && *peek <= '9') {
                ptr = peek; continue;        /* next positive literal */
            }
            if (*peek == '-' && peek[1] >= '0' && had_space) {
                ptr = peek; continue;        /* spaced minus = negation, vector continues */
            }
            /* K convention: a scalar variable following a number (with space)
             * continues the vector — e.g. "330 D" where D=88200 gives [330,88200] */
            if (had_space && *peek >= 'A' && *peek <= 'Z' && peek[1] != ':') {
                K v = vars[*peek - 'A'];
                if (v && v->n == 1) {
                    buf[n++] = v->f[0];
                    ptr = peek + 1;
                    continue;
                }
            }
            break;                           /* flush minus = subtraction, or end of input */
        }
        *s = ptr;
        K x = k_new(n); memcpy(x->f, buf, n * sizeof(double));
        return x;
    }

    (*s)++;

    if (**s == ':') {
        (*s)++; K x = expr(s);
        if (c >= 'A' && c <= 'Z') {
            int i = c - 'A';
            if (vars[i]) k_free(vars[i]);
            if (x) { x->r++; vars[i] = x; }
        }
        if (x) x->r++;
        return x;
    }

    if (c >= 'A' && c <= 'Z') {
        K first = k_get(c);
        if (!first || first->n != 1) return first;
        /* K convention: scalar variables separated by spaces form a vector.
         * "M D" where M=440 D=88200 gives [440, 88200]. Same rule as numerics. */
        double buf[1024];
        int n = 0;
        buf[n++] = first->f[0];
        k_free(first);
        char *ptr = *s;
        while (n < 1024) {
            char *peek = ptr;
            while (*peek == ' ') peek++;
            if (peek == ptr) break;                    /* no space — stop */
            if (*peek < 'A' || *peek > 'Z') break;    /* not a variable — stop */
            if (peek[1] == ':') break;                 /* assignment target — stop */
            K v = vars[*peek - 'A'];
            if (!v || v->n != 1) break;               /* not a scalar — stop */
            buf[n++] = v->f[0];
            ptr = peek + 1;
        }
        *s = ptr;
        if (n == 1) {
            K x = k_new(1); x->f[0] = buf[0]; return x;
        }
        K x = k_new(n);
        memcpy(x->f, buf, n * sizeof(double));
        return x;
    }

    if (c == 'x') return args[0] ? (args[0]->r++, args[0]) : k_new(0);
    if (c == 'y') return args[1] ? (args[1]->r++, args[1]) : k_new(0);

    int is_scan = 0;
    while (**s == ' ') (*s)++;
    if (**s == '\\') {
        is_scan = 1;
        (*s)++;
    }

    K arg = e(s);

    if (is_scan) {
        return scan(c, arg);
    } else {
        return mo(c, arg);
    }
}

K e(char **s) {
    K x = atom(s);
    while (**s == ' ') (*s)++;

    while (**s == ';') {
        (*s)++;
        if (x) k_free(x);
        while (**s == ' ') (*s)++;
        if (!**s || **s == '\n' || **s == ')' || **s == '}') return k_new(0);
        x = atom(s);
        while (**s == ' ') (*s)++;
    }

    if (k_is_func(x) && **s && **s != '\n' && **s != ')' && **s != ';' && **s != '/') {
        char peek = **s;
        int is_operator = (peek == '+' || peek == '-' || peek == '*' ||
                          peek == '%' || peek == '^' || peek == '&' ||
                          peek == '|' || peek == '<' || peek == '>' ||
                          peek == '=' || peek == ',' || peek == '#' ||
                          peek == 'o' || peek == '$' ||
                          peek == 'f' || peek == 'y' || peek == 'z' ||
                          peek == 's' || peek == 't' || peek == 'h' ||
                          peek == 'a' || peek == 'q' || peek == 'l' ||
                          peek == 'e' || peek == 'r' || peek == 'p' ||
                          peek == 'c' || peek == 'i' || peek == 'w' ||
                          peek == 'd' || peek == 'v' || peek == 'm' ||
                          peek == 'b' || peek == 'u' || peek == 'j' ||
                          peek == 'k' || peek == 'n' || peek == 'g');
        if (!is_operator) {
            K arg = e(s);
            K call_args[1] = {arg};
            K result = k_call(x, call_args, 1);
            k_free(x);
            return result;
        }
    }

    if (!**s || **s == '\n' || **s == ')' || **s == ';' || **s == '/') return x;
    char op = *(*s)++;
    return dy(op, x, e(s));
}

void p(K x) {
    if (!x) return;
    if (k_is_func(x)) {
        printf("{%s}\n", k_func_body(x));
        return;
    }
    if (x->n == 1) printf("%.4f\n", x->f[0]);
    else printf("Array[%d]\n", x->n);
}
