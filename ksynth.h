#ifndef KSYNTH_H
#define KSYNTH_H

#include <stdlib.h>
#include <string.h>

typedef struct { int r, n; double f[]; } *K;
extern K vars[26];    // A-Z
extern K args[3];     // x, y, z (function arguments)

K e(char **s);
void p(K x);
void k_free(K x);
K k_new(int n);

// Function support
K k_func(char *body);
K k_call(K fn, K *call_args, int nargs);
int k_is_func(K x);

// C API Integration
K k_view(int n, double *ptr);
void bind_scalar(char name, double val);
K k_get(char name);
char* k_func_body(K x);

#endif
