#include <stdio.h>
#include <string.h>
#include "ksynth.h"

/*
Build:
  gcc -O2 -I. examples/simple_api.c ksynth.c -lm -o simple_api

Run:
  ./simple_api
*/

static void print_f64_array(const char *label, const double *values, int n) {
    int i;
    printf("%s (n=%d): [", label, n);
    for (i = 0; i < n; i++) {
        if (i > 0) printf(", ");
        printf("%g", values[i]);
    }
    printf("]\n");
}

static void print_i32_array(const char *label, const int *values, int n) {
    int i;
    printf("%s (n=%d): [", label, n);
    for (i = 0; i < n; i++) {
        if (i > 0) printf(", ");
        printf("%d", values[i]);
    }
    printf("]\n");
}

int main(void) {
    ks_ctx *ctx = ks_create(16 * 1024 * 1024, 1000000);
    if (!ctx) {
        fprintf(stderr, "failed to create ksynth context\n");
        return 1;
    }

    {
        K result = ks_eval(ctx, "2+3", strlen("2+3"));
        double out[1];
        int n;

        if (!result || ctx->last_status != KS_OK) {
            fprintf(stderr, "eval failed: %s\n", ks_strerror(ctx->last_status));
            ks_destroy(ctx);
            return 1;
        }

        n = k_copy_to_f64(result, out, 1);
        print_f64_array("2+3", out, n);
        k_free(ctx, result);
    }

    {
        float a[] = {1.0f, 2.0f, 3.0f, 4.0f};
        int b[] = {10, 20, 30, 40};
        K result;
        K avec;
        K bvec;
        float a_out[4];
        int b_out[4];
        double result_out[4];
        int n_a;
        int n_b;
        int n_result;

        bind_array_f32(ctx, 'A', 4, a);
        bind_array_i32(ctx, 'B', 4, b);
        bind_scalar(ctx, 'N', 0.5);

        result = ks_eval(ctx, "A + B*N", strlen("A + B*N"));
        if (!result || ctx->last_status != KS_OK) {
            fprintf(stderr, "eval failed: %s\n", ks_strerror(ctx->last_status));
            ks_destroy(ctx);
            return 1;
        }

        avec = k_get(ctx, 'A');
        bvec = k_get(ctx, 'B');

        n_a = k_copy_to_f32(avec, a_out, 4);
        n_b = k_copy_to_i32(bvec, b_out, 4);
        n_result = k_copy_to_f64(result, result_out, 4);

        printf("host arrays provided from C:\n");
        print_f64_array("A as copied back to host", (double[]){a_out[0], a_out[1], a_out[2], a_out[3]}, n_a);
        print_i32_array("B as copied back to host", b_out, n_b);
        print_f64_array("A + B*N", result_out, n_result);

        if (n_result > 2) {
            printf("third element = %g\n", result_out[2]);
        }

        k_free(ctx, avec);
        k_free(ctx, bvec);
        k_free(ctx, result);
    }

    {
        double d[] = {0.25, 0.5, 0.75};
        K result = ks_eval(ctx, "C: A,B", strlen("C: A,B"));
        K c;
        K dvec;
        double c_out[8];
        double d_out[3];
        int n_c;
        int n_d;

        if (!result || ctx->last_status != KS_OK) {
            fprintf(stderr, "eval failed: %s\n", ks_strerror(ctx->last_status));
            ks_destroy(ctx);
            return 1;
        }

        k_free(ctx, result);

        c = k_get(ctx, 'C');
        dvec = k_from_f64(ctx, 3, d);

        n_c = k_copy_to_f64(c, c_out, 8);
        n_d = k_copy_to_f64(dvec, d_out, 3);

        print_f64_array("C", c_out, n_c);
        print_f64_array("D from C doubles", d_out, n_d);

        k_free(ctx, c);
        k_free(ctx, dvec);
    }

    ks_destroy(ctx);
    return 0;
}
