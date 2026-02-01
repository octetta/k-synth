#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ksynth.h"

K vars[26] = {0};

K k_new(int n) {
    K x = malloc(sizeof(*x) + sizeof(double) * (n < 0 ? 0 : n));
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
    if (name < 'A' || name > 'Z') return NULL;
    K v = vars[name - 'A'];
    if (v) v->r++;
    return v;
}

K mo(char c, K b) {
    if (!b) return NULL;
    if (c == '!') {
        int n = (int)b->f[0]; k_free(b);
        K x = k_new(n);
        for(int j=0; j<n; j++) x->f[j] = (double)j;
        return x;
    }
    if (c == 'r') {
        int n = (int)b->f[0]; k_free(b);
        K x = k_new(n);
        for(int j=0; j<n; j++) 
            x->f[j] = (((double)rand() / (double)RAND_MAX) * 2.0) - 1.0;
        return x;
    }
    if (c == 'p') {
        K x = k_new(b->n);
        for(int i=0; i<b->n; i++) x->f[i] = 3.141592653589793;
        k_free(b); return x;
    }
    K x = k_new(b->n);
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
            case '~': x->f[i] = b->f[b->n - 1 - i]; break;
            default:  x->f[i] = v; break;
        }
    }
    k_free(b); return x;
}

K dy(char c, K a, K b) {
    if (!a || !b) { k_free(a); k_free(b); return NULL; }
    K x;
    if (c == 'f') { // Resonant Filter
        x = k_new(b->n);
        double b0 = 0, b1 = 0;
        for (int i = 0; i < b->n; i++) {
            double cutoff = (a->n > i) ? a->f[i] : a->f[0];
            double res = 0.0;
            
            // Automation check: if a is at least twice as long as signal b
            if (a->n >= 2 * b->n) {
                cutoff = a->f[i];
                res = a->f[i + b->n];
            } 
            // Pair check: if a is just [cutoff, res]
            else if (a->n == 2) {
                res = a->f[1];
            }

            if (cutoff > 0.95) cutoff = 0.95;
            if (res > 3.98) res = 3.98;

            double in = b->f[i] - (res * b1);
            b0 += cutoff * (in - b0);
            b1 += cutoff * (b0 - b1);
            x->f[i] = b1;
        }
        k_free(a); k_free(b); return x;
    }
    if (c == 'y') { // Feedback Delay
        int offset = (int)a->f[0];
        x = k_new(b->n);
        for (int i = 0; i < b->n; i++) {
            double delayed = (i >= offset) ? x->f[i - offset] : 0;
            x->f[i] = b->f[i] + (0.4 * delayed);
        }
        k_free(a); k_free(b); return x;
    }
    if (c == '#') {
        int n = (int)a->f[0]; x = k_new(n);
        if (b->n > 0) for (int i = 0; i < n; i++) x->f[i] = b->f[i % b->n];
        k_free(a); k_free(b); return x;
    }
    if (c == ',') {
        x = k_new(a->n + b->n);
        memcpy(x->f, a->f, a->n * sizeof(double));
        memcpy(x->f + a->n, b->f, b->n * sizeof(double));
        k_free(a); k_free(b); return x;
    }
    int mn = a->n > b->n ? a->n : b->n;
    x = k_new(mn);
    for (int i = 0; i < mn; i++) {
        double va = a->f[i % a->n], vb = b->f[i % b->n];
        switch (c) {
            case '+': x->f[i] = va + vb; break;
            case '*': x->f[i] = va * vb; break;
            case '-': x->f[i] = va - vb; break;
            case '%': x->f[i] = (vb == 0) ? 0 : va / vb; break;
            case '^': x->f[i] = pow(fabs(va), vb); break;
            case '&': x->f[i] = fmin(va, vb); break;
            case '|': x->f[i] = fmax(va, vb); break;
            case '=': x->f[i] = (va == vb); break;
            case '!': x->f[i] = fmod(va, vb); break;
            default:  x->f[i] = 0; break;
        }
    }
    k_free(a); k_free(b); return x;
}

K e(char **s);

K atom(char **s) {
    while (**s == ' ') (*s)++;
    if (!**s || **s == '\n' || **s == ')' || **s == ';') return NULL;
    if (**s == '(') { (*s)++; K x = e(s); if (**s == ')') (*s)++; return x; }
    char c = **s;
    if ((c >= '0' && c <= '9') || (c == '-' && (*s)[1] >= '0')) {
        char *ptr; double v = strtod(*s, &ptr); *s = ptr;
        K x = k_new(1); x->f[0] = v; return x;
    }
    (*s)++;
    if (**s == ':') {
        (*s)++; K x = e(s);
        if (c >= 'A' && c <= 'Z') {
            int i = c - 'A';
            if (vars[i]) k_free(vars[i]);
            if (x) { x->r++; vars[i] = x; }
        }
        return x;
    }
    if (c >= 'A' && c <= 'Z') {
        K v = vars[c - 'A'];
        if (!v) return NULL;
        K x = k_new(v->n); memcpy(x->f, v->f, v->n * sizeof(double));
        return x;
    }
    return mo(c, e(s));
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
