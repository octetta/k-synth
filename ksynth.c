#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ksynth.h"

K vars[26] = {0};

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

/* --- Scan Adverb --- */

K scan(char op, K b) {
    if (!b || b->n < 1) return b;
    K x = k_new(b->n);
    double acc;
    
    switch(op) {
        case '+':  // running sum (integration)
            acc = 0.0;
            for (int i = 0; i < b->n; i++) {
                acc += b->f[i];
                x->f[i] = acc;
            }
            break;
            
        case '*':  // running product (compound)
            acc = 1.0;
            for (int i = 0; i < b->n; i++) {
                acc *= b->f[i];
                x->f[i] = acc;
            }
            break;
            
        case '-':  // running difference
            acc = 0.0;
            for (int i = 0; i < b->n; i++) {
                acc -= b->f[i];
                x->f[i] = acc;
            }
            break;
            
        case '%':  // running division
            acc = 1.0;
            for (int i = 0; i < b->n; i++) {
                if (b->f[i] != 0) acc /= b->f[i];
                x->f[i] = acc;
            }
            break;
            
        case '&':  // running minimum
            acc = b->n > 0 ? b->f[0] : 0.0;
            x->f[0] = acc;
            for (int i = 1; i < b->n; i++) {
                if (b->f[i] < acc) acc = b->f[i];
                x->f[i] = acc;
            }
            break;
            
        case '|':  // running maximum
            acc = b->n > 0 ? b->f[0] : 0.0;
            x->f[0] = acc;
            for (int i = 1; i < b->n; i++) {
                if (b->f[i] > acc) acc = b->f[i];
                x->f[i] = acc;
            }
            break;
            
        case '^':  // running power
            acc = b->n > 0 ? b->f[0] : 1.0;
            x->f[0] = acc;
            for (int i = 1; i < b->n; i++) {
                acc = pow(acc, b->f[i]);
                x->f[i] = acc;
            }
            break;
            
        default:
            // Scan doesn't make sense for this verb
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
    K x;
    if (c == '!') {
        int n = (int)b->f[0]; k_free(b);
        x = k_new(n);
        for(int j=0; j<n; j++) x->f[j] = (double)j;
        return x;
    }

    if (c == '+') {
        double t = 0;
        for (int i = 0; i < b->n; i++) t += b->f[i];
        x = k_new(1); x->f[0] = t;
        k_free(b); return x;
    }
    if (c == '>') {
        double m = 0;
        for (int i = 0; i < b->n; i++) if(fabs(b->f[i]) > m) m = fabs(b->f[i]);
        x = k_new(1); x->f[0] = m;
        k_free(b); return x;
    }

    // Stereo channel extraction
    if (c == 'j') {  // extract left channel from interleaved stereo
        if (b->n < 2) { k_free(b); return k_new(0); }
        x = k_new(b->n / 2);
        for (int i = 0; i < x->n; i++) x->f[i] = b->f[i*2];
        k_free(b); return x;
    }

    if (c == 'k') {  // extract right channel from interleaved stereo
        if (b->n < 2) { k_free(b); return k_new(0); }
        x = k_new(b->n / 2);
        for (int i = 0; i < x->n; i++) x->f[i] = b->f[i*2+1];
        k_free(b); return x;
    }

    x = k_new(b->n);
    for (int i = 0; i < b->n; i++) {
        double v = b->f[i];
        switch (c) {
            case 's': x->f[i] = sin(v); break;
            case 't': x->f[i] = tan(v); break;
            case 'h': x->f[i] = tanh(v); break;
            case 'a': x->f[i] = fabs(v); break;
            case 'q': x->f[i] = sqrt(fabs(v)); break;
            case 'l': x->f[i] = log(fabs(v)); break;
            case 'e': x->f[i] = exp(v); break;
            case '_': x->f[i] = floor(v); break;
            case 'r': x->f[i] = ((double)rand() / (double)RAND_MAX) * 2.0 - 1.0; break;
            case 'p': x->f[i] = 3.14159265358979323846 * v; break;
            case '~': x->f[i] = b->f[b->n - 1 - i]; break;
            case 'x': x->f[i] = exp(-5.0 * v); break;  // e^(-5t)
            case 'd': x->f[i] = tanh(v * 3.0); break;  // soft clipping
            case 'v': x->f[i] = floor(v * 4.0) / 4.0 ; break;  // 2-bit quantizer
            case 'm': {
                // Deterministic metallic bit-shredding
                unsigned int clock = i;
                unsigned int h = (clock * 13) ^ (clock >> 5) ^ (clock * 193);
                x->f[i] = (h & 128) ? 0.7 : -0.7;
                break;
            }
            case 'b': {
                // 909 metallic frequencies
                double f[] = {2.43, 3.01, 3.52, 4.11, 5.23, 6.78}; 
                double s = 0;
                for (int j = 0; j < 6; j++) {
                    s += (sin(i * 0.1 * f[j]) > 0) ? 1.0 : -1.0;
                }
                x->f[i] = s / 6.0;
                break;
            }
            case 'u': {
                // Linear attack over first 10 samples
                x->f[i] = (i < 10) ? (double)i / 10.0 : 1.0;
                break;
            }
            default:  x->f[i] = v; break;
        }
    }
    k_free(b); return x;
}

K dy(char c, K a, K b) {
    if (!a || !b) { k_free(a); k_free(b); return NULL; }
    K x;
    
    // Stereo interleave
    if (c == 'z') {
        int mn = (a->n < b->n) ? a->n : b->n;
        x = k_new(mn * 2);
        for (int i = 0; i < mn; i++) {
            x->f[i*2]   = a->f[i];   // left
            x->f[i*2+1] = b->f[i];   // right
        }
        k_free(a); k_free(b); return x;
    }
    
    if (c == 'f') {
        x = k_new(b->n); double b0 = 0, b1 = 0;
        for (int i = 0; i < b->n; i++) {
            double ct = (a->n > i) ? a->f[i] : a->f[0];
            double rs = (a->n >= 2) ? a->f[1] : 0.0;
            if (ct > 0.95) ct = 0.95;
            if (rs > 3.98) rs = 3.98;
            double in = b->f[i] - (rs * b1);
            b0 += ct * (in - b0); b1 += ct * (b0 - b1); x->f[i] = b1;
        }
    } else if (c == 'y') {
        int d = (int)a->f[0]; x = k_new(b->n);
        for (int i = 0; i < b->n; i++)
            x->f[i] = b->f[i] + (0.4 * ((i >= d) ? x->f[i-d] : 0));
    } else if (c == '#') {
        int n = (int)a->f[0]; x = k_new(n);
        if (b->n > 0) for (int i = 0; i < n; i++) x->f[i] = b->f[i % b->n];
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
                case '^': x->f[i] = pow(fabs(va), vb); break;
                case '&': x->f[i] = va < vb ? va : vb; break;
                case '|': x->f[i] = va > vb ? va : vb; break;
                default:  x->f[i] = 0; break;
            }
        }
    }
    k_free(a); k_free(b); return x;
}

/* --- Parser with Scan Support --- */

K e(char **s);

K atom(char **s) {
    while (**s == ' ') (*s)++;
    if (!**s || **s == '\n' || **s == ')' || **s == ';') return NULL;

    // Paren handling: (A+B) or explicit (1 2 3)
    if (**s == '(') {
        (*s)++; K x = e(s);
        if (**s == ')') (*s)++;
        return x;
    }

    char c = **s;
    // Number handling: slurp multiple space-separated numbers into one K array
    if ((c >= '0' && c <= '9') || (c == '-' && (*s)[1] >= '0')) {
        double buf[1024]; int n = 0;
        char *ptr = *s;
        while (n < 1024) {
            buf[n++] = strtod(ptr, &ptr);
            char *peek = ptr;
            while (*peek == ' ') peek++;
            // Only continue if the next thing is another number
            if (!((*peek >= '0' && *peek <= '9') || (*peek == '-' && peek[1] >= '0'))) break;
            ptr = peek;
        }
        *s = ptr;
        K x = k_new(n); memcpy(x->f, buf, n * sizeof(double));
        return x;
    }

    (*s)++;
    if (**s == ':') {
        (*s)++; K x = e(s);
        if (c >= 'A' && c <= 'Z') {
            int i = c - 'A';
            if (vars[i]) k_free(vars[i]);
            if (x) { x->r++; vars[i] = x; }
        }
        if (x) x->r++; // REPL safety: return a reference that main.c can free
        return x;
    }
    
    if (c >= 'A' && c <= 'Z') return k_get(c);
    
    // Check for scan adverb
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
    if (!**s || **s == '\n' || **s == ')' || **s == ';') return x;
    char op = *(*s)++;
    return dy(op, x, e(s));
}

void p(K x) {
    if (!x) return;
    if (x->n == 1) printf("%.4f\n", x->f[0]);
    else printf("Array[%d]\n", x->n);
}
