#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ksynth.h"

K vars[26] = {0};

/* --- Lifecycle --- */

K k_new_double_array(int n) {
    if (n < 0) n = 0;
    struct KVal *x = malloc(sizeof(struct KVal));
    if (!x) return NULL;
    x->r = 1;
    x->type = K_TYPE_DOUBLE_ARRAY;
    x->n = n;
    x->data.d = NULL; // Initialize to NULL
    if (n > 0) {
        x->data.d = malloc(sizeof(double) * n);
        if (!x->data.d) {
            free(x);
            return NULL;
        }
    }
    return (K)x;
}

K k_new_k_array(int n) {
    if (n < 0) n = 0;
    struct KVal *x = malloc(sizeof(struct KVal));
    if (!x) return NULL;
    x->r = 1;
    x->type = K_TYPE_K_ARRAY;
    x->n = n;
    x->data.k = NULL; // Initialize to NULL
    if (n > 0) {
        x->data.k = malloc(sizeof(K) * n);
        if (!x->data.k) {
            free(x);
            return NULL;
        }
        for (int i = 0; i < n; ++i) {
            x->data.k[i] = NULL; // Initialize K pointers to NULL
        }
    }
    return (K)x;
}

// Original k_new now creates a K_TYPE_DOUBLE_ARRAY
K k_new(int n) {
    return k_new_double_array(n);
}

void k_free(K x) {
    if (!x || --x->r) return; // Decrement ref count; if not zero, don't free yet

    if (x->type == K_TYPE_DOUBLE_ARRAY) {
        free(x->data.d); // Free the double array
    } else if (x->type == K_TYPE_K_ARRAY) {
        if (x->data.k) {
            for (int i = 0; i < x->n; ++i) {
                k_free(x->data.k[i]); // Recursively free nested K objects
            }
            free(x->data.k); // Free the array of K pointers
        }
    }
    free(x); // Free the KVal struct itself
}

K k_view(int n, double *ptr) {
    K x = k_new_double_array(n); // Use k_new_double_array
    if (x && ptr) memcpy(x->data.d, ptr, n * sizeof(double)); // Access data.d
    return x;
}

void bind_scalar(char name, double val) {
    if (name < 'A' || name > 'Z') return;
    int i = name - 'A';
    K x = k_new_double_array(1); x->data.d[0] = val; // Use k_new_double_array and data.d
    if (vars[i]) k_free(vars[i]);
    vars[i] = x;
}

K k_get(char name) {
    if (name < 'A' || name > 'Z' || !vars[name - 'A']) return NULL;
    K v = vars[name - 'A'];
    v->r++; return v;
}

/* --- Verbs & Operators (Your Working Logic) --- */

K mo(char c, K b) {
    if (!b) return NULL;

    // Handle K_TYPE_K_ARRAY by recursively applying the operation
    if (b->type == K_TYPE_K_ARRAY) {
        K x = k_new_k_array(b->n);
        if (!x) { k_free(b); return NULL; }
        for (int i = 0; i < b->n; ++i) {
            K res = mo(c, b->data.k[i]); // Recursive call
            if (!res) { // Handle error from recursive call
                for (int j = 0; j < i; ++j) k_free(x->data.k[j]); // Free already processed items
                k_free(x);
                k_free(b);
                return NULL;
            }
            x->data.k[i] = res; // Assign result, ref count is handled by recursive mo and its k_free(b)
        }
        k_free(b); // Free the original container
        return x;
    }

    // From here, b must be K_TYPE_DOUBLE_ARRAY
    // Operations like '!', '+', '>' typically operate on numbers, so restrict them to double arrays
    if (b->type != K_TYPE_DOUBLE_ARRAY && (c == '!' || c == '+' || c == '>')) {
        fprintf(stderr, "Error: Monadic operation '%c' expects a double array.\n", c);
        k_free(b);
        return NULL;
    }

    K x;
    if (c == '!') {
        int n = (int)b->data.d[0]; k_free(b);
        x = k_new_double_array(n);
        for(int j=0; j<n; j++) x->data.d[j] = (double)j;
        return x;
    }

    if (c == '+') { // Sum all elements
        double t = 0;
        for (int i = 0; i < b->n; i++) t += b->data.d[i];
        x = k_new_double_array(1); x->data.d[0] = t;
        k_free(b); return x;
    }
    if (c == '>') { // Max absolute value
        double m = 0;
        for (int i = 0; i < b->n; i++) if(fabs(b->data.d[i]) > m) m = fabs(b->data.d[i]);
        x = k_new_double_array(1); x->data.d[0] = m;
        k_free(b); return x;
    }

    // Element-wise operations on double arrays
    x = k_new_double_array(b->n);
    if (!x) { k_free(b); return NULL; }
    for (int i = 0; i < b->n; i++) {
        double v = b->data.d[i];
        switch (c) {
            case 's': x->data.d[i] = sin(v); break;
            case 't': x->data.d[i] = tan(v); break;
            case 'h': x->data.d[i] = tanh(v); break;
            case 'a': x->data.d[i] = fabs(v); break;
            case 'q': x->data.d[i] = sqrt(fabs(v)); break;
            case 'l': x->data.d[i] = log(fabs(v)); break;
            case 'e': x->data.d[i] = exp(v); break;
            case '_': x->data.d[i] = floor(v); break;
            case 'r': x->data.d[i] = ((double)rand() / (double)RAND_MAX) * 2.0 - 1.0; break;
            case 'p': x->data.d[i] = M_PI * v; break; // Use M_PI for PI
            case '~': x->data.d[i] = b->data.d[b->n - 1 - i]; break;
            case 'x': x->data.d[i] = exp(-5.0 * v); break;  // e^(-5t)
            case 'd': x->data.d[i] = tanh(v * 3.0); break;  // soft clipping
            case 'v': x->data.d[i] = floor(v * 4.0) / 4.0 ; break;  // 2-bit quantizer
            case 'm': {
                unsigned int clock = i;
                unsigned int h = (clock * 13) ^ (clock >> 5) ^ (clock * 193);
                x->data.d[i] = (h & 128) ? 0.7 : -0.7; // Hard square-pulse output
                break;
            }
            case 'b': {
                double f_freq[] = {2.43, 3.01, 3.52, 4.11, 5.23, 6.78}; 
                double s_val = 0;
                for (int j = 0; j < 6; j++) {
                    s_val += (sin(i * 0.1 * f_freq[j]) > 0) ? 1.0 : -1.0;
                }
                x->data.d[i] = s_val / 6.0;
                break;
            }
            case 'u': {
                x->data.d[i] = (i < 10) ? (double)i / 10.0 : 1.0;
                break;
            }
            default:  x->data.d[i] = v; break;
        }
    }
    k_free(b); return x;
}

K dy(char c, K a, K b) {
    K x = NULL; // The result K object

    if (!a || !b) {
        if (a) k_free(a);
        if (b) k_free(b);
        return NULL;
    }

    // Handle K_TYPE_K_ARRAY with K_TYPE_K_ARRAY (recursive)
    if (a->type == K_TYPE_K_ARRAY && b->type == K_TYPE_K_ARRAY) {
        // Assume matching sizes for now, or take the minimum
        int min_n = (a->n < b->n) ? a->n : b->n;
        x = k_new_k_array(min_n);
        if (!x) { /* fall through to k_free(a) and k_free(b) */ }
        else {
            for (int i = 0; i < min_n; ++i) {
                x->data.k[i] = dy(c, a->data.k[i], b->data.k[i]); // Recursive call
                if (!x->data.k[i]) {
                    // Error in recursive call, free what's been allocated
                    for (int j = 0; j < i; ++j) k_free(x->data.k[j]);
                    k_free(x); x = NULL; // Set x to NULL to indicate error
                    break;
                }
            }
        }
    }

    // Handle scalar broadcasting (one is K_TYPE_DOUBLE_ARRAY with n=1, other is K_TYPE_K_ARRAY or K_TYPE_DOUBLE_ARRAY)
    else if (a->type == K_TYPE_DOUBLE_ARRAY && a->n == 1 && b->type == K_TYPE_K_ARRAY) {
        x = k_new_k_array(b->n);
        if (!x) { /* fall through */ }
        else {
            for (int i = 0; i < b->n; ++i) {
                x->data.k[i] = dy(c, a, b->data.k[i]); // Apply scalar 'a' to each element of 'b'
                if (!x->data.k[i]) {
                    for (int j = 0; j < i; ++j) k_free(x->data.k[j]);
                    k_free(x); x = NULL;
                    break;
                }
            }
        }
    }
    else if (b->type == K_TYPE_DOUBLE_ARRAY && b->n == 1 && a->type == K_TYPE_K_ARRAY) {
        x = k_new_k_array(a->n);
        if (!x) { /* fall through */ }
        else {
            for (int i = 0; i < a->n; ++i) {
                x->data.k[i] = dy(c, a->data.k[i], b); // Apply scalar 'b' to each element of 'a'
                if (!x->data.k[i]) {
                    for (int j = 0; j < i; ++j) k_free(x->data.k[j]);
                    k_free(x); x = NULL;
                    break;
                }
            }
        }
    }

    // From here, both a and b must be K_TYPE_DOUBLE_ARRAY for standard ops,
    // or one of them is scalar being applied to a K_TYPE_DOUBLE_ARRAY
    // The previous checks ensure that if a K_TYPE_K_ARRAY was involved, it was handled.

    // Specific handling for 'f', 'y', '#' should operate on flat double arrays for now
    else if ((c == 'f' || c == 'y' || c == '#') && (a->type != K_TYPE_DOUBLE_ARRAY || b->type != K_TYPE_DOUBLE_ARRAY)) {
        fprintf(stderr, "Error: Dyadic operation '%c' expects double arrays for filtering, delay or take.\n", c);
        // x remains NULL
    } else if (c == 'f') { // Filter
        x = k_new_double_array(b->n); double b0 = 0, b1 = 0;
        if (!x) { /* fall through */ }
        else {
            for (int i = 0; i < b->n; i++) {
                double ct = (a->n > i) ? a->data.d[i] : a->data.d[0];
                double rs = (a->n >= 2) ? a->data.d[1] : 0.0;
                if (ct > 0.95) ct = 0.95;
                if (rs > 3.98) rs = 3.98;
                double in = b->data.d[i] - (rs * b1);
                b0 += ct * (in - b0); b1 += ct * (b0 - b1); x->data.d[i] = b1;
            }
        }
    } else if (c == 'y') { // Delay/Feedback
        int d = (int)a->data.d[0]; x = k_new_double_array(b->n);
        if (!x) { /* fall through */ }
        else {
            for (int i = 0; i < b->n; i++)
                x->data.d[i] = b->data.d[i] + (0.4 * ((i >= d) ? x->data.d[i-d] : 0));
        }
    } else if (c == '#') { // Take/Reshape
        int n = (int)a->data.d[0]; x = k_new_double_array(n);
        if (!x) { /* fall through */ }
        else {
            if (b->n > 0) for (int i = 0; i < n; i++) x->data.d[i] = b->data.d[i % b->n];
        }
    } else if (c == ',') { // Concatenation
        if (a->type == K_TYPE_DOUBLE_ARRAY && b->type == K_TYPE_DOUBLE_ARRAY) {
            x = k_new_double_array(a->n + b->n);
            if (!x) { /* fall through */ }
            else {
                memcpy(x->data.d, a->data.d, a->n * sizeof(double));
                memcpy(x->data.d + a->n, b->data.d, b->n * sizeof(double));
            }
        } else { // Handle K_TYPE_K_ARRAY or mixed types by converting to K_TYPE_K_ARRAY
            K a_src_container = NULL; // Temporary container for 'a'
            K b_src_container = NULL; // Temporary container for 'b'

            // Prepare a_src_container: ensure it's a K_TYPE_K_ARRAY
            if (a->type == K_TYPE_DOUBLE_ARRAY) {
                a_src_container = k_new_k_array(1);
                if (!a_src_container) { /* error handling will be handled by final frees */ }
                else { a_src_container->data.k[0] = a; } // Transfer ownership of 'a' to a_src_container
            } else {
                a_src_container = a; // Use 'a' directly as the source container
                a_src_container->r++; // Increment ref count because a_src_container takes a reference
            }

            // Prepare b_src_container: ensure it's a K_TYPE_K_ARRAY
            if (b->type == K_TYPE_DOUBLE_ARRAY) {
                b_src_container = k_new_k_array(1);
                if (!b_src_container) { /* error handling will be handled by final frees */ k_free(a_src_container); } // Free a_src_container if it was newly created
                else { b_src_container->data.k[0] = b; } // Transfer ownership of 'b' to b_src_container
            } else {
                b_src_container = b; // Use 'b' directly as the source container
                b_src_container->r++; // Increment ref count because b_src_container takes a reference
            }

            if (!a_src_container || !b_src_container) { /* error already printed */ }
            else {
                x = k_new_k_array(a_src_container->n + b_src_container->n);
                if (!x) { /* fall through */ }
                else {
                    // Transfer elements to result 'x' and increment their ref counts
                    for (int i = 0; i < a_src_container->n; ++i) {
                        x->data.k[i] = a_src_container->data.k[i];
                        x->data.k[i]->r++; // Result 'x' takes a new reference to the element
                    }
                    for (int i = 0; i < b_src_container->n; ++i) {
                        x->data.k[a_src_container->n + i] = b_src_container->data.k[i];
                        x->data.k[a_src_container->n + i]->r++; // Result 'x' takes a new reference to the element
                    }
                }
            }
            // Now k_free the temporary K_TYPE_K_ARRAY containers (a_src_container, b_src_container).
            if (a_src_container) k_free(a_src_container);
            if (b_src_container) k_free(b_src_container);

        } // End of K_TYPE_K_ARRAY or mixed type concat
    } else { // Generic element-wise operations
        // Ensure both are double arrays or one is a scalar
        if (a->type != K_TYPE_DOUBLE_ARRAY || b->type != K_TYPE_DOUBLE_ARRAY) {
            fprintf(stderr, "Error: Dyadic operation '%c' expects double arrays or scalar broadcasting.\n", c);
            // x remains NULL
        } else {
            int mn = a->n > b->n ? a->n : b->n; x = k_new_double_array(mn);
            if (!x) { /* fall through */ }
            else {
                for (int i = 0; i < mn; i++) {
                    double va = a->data.d[i % a->n], vb = b->data.d[i % b->n];
                    switch (c) {
                        case '+': x->data.d[i] = va + vb; break;
                        case '*': x->data.d[i] = va * vb; break;
                        case '-': x->data.d[i] = va - vb; break;
                        case '%': x->data.d[i] = (vb == 0) ? 0 : va / vb; break;
                        case '^': x->data.d[i] = pow(fabs(va), vb); break;
                        case '&': x->data.d[i] = va < vb ? va : vb; break;
                        case '|': x->data.d[i] = va > vb ? va : vb; break;
                        default:  x->data.d[i] = 0; break;
                    }
                }
            }
        }
    }
    k_free(a); // Free a once
    k_free(b); // Free b once
    return x; // Return the result
}