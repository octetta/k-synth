#ifndef KSYNTH_H
#define KSYNTH_H

#include <stdlib.h>
#include <string.h>

typedef struct { int r, n; double f[]; } *K;
extern K vars[26];

K e(char **s);
void p(K x);
void k_free(K x);
K k_new(int n);

// C API Integration
K k_view(int n, double *ptr);
void bind_scalar(char name, double val);
K k_get(char name);

#endif
