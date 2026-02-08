#ifndef KSYNTH_H
#define KSYNTH_H

#include <stdlib.h>
#include <string.h>

// Type tags for K objects
typedef enum {
    K_TYPE_DOUBLE_ARRAY, // Represents a flat array of doubles
    K_TYPE_K_ARRAY       // Represents an array of K pointers (nested array)
} KType;

typedef struct KVal {
    int r;      // Reference count
    KType type; // Type tag
    int n;      // Number of elements (doubles or K pointers)
    union {
        double *d;       // Pointer to array of doubles if type is K_TYPE_DOUBLE_ARRAY
        struct KVal **k; // Pointer to array of K pointers if type is K_TYPE_K_ARRAY
    } data;
} *K;
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
