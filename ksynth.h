#ifndef KSYNTH_H
#define KSYNTH_H

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

/* Cross-platform Thread Local Storage */
#if defined(_MSC_VER)
  #define KS_TLS __declspec(thread)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  #define KS_TLS _Thread_local
#else
  #define KS_TLS __thread
#endif

typedef enum {
    KS_OK = 0,
    KS_ERR_SYNTAX,       /* Malformed ksynth code */
    KS_ERR_OOM,          /* Memory budget exceeded */
    KS_ERR_GAS,          /* Execution time (gas) limit reached */
    KS_ERR_SIGSEGV,      /* Caught segmentation fault */
    KS_ERR_SIGFPE,       /* Caught floating point exception */
    KS_ERR_SIGILL,       /* Caught illegal instruction */
    KS_ERR_INVALID_ARGS, /* Invalid function arguments */
    KS_ERR_INTERNAL      /* Unexpected internal error */
} ks_status;

typedef struct { int r, n; double f[]; } *K;

typedef struct ks_ctx {
    K vars[26];          /* A-Z user variables */
    K args[2];           /* x, y function arguments */
    
    size_t mem_limit;    /* Max bytes allowed for K objects */
    size_t mem_used;     /* Current bytes used by K objects */
    
    long long gas_limit; /* Max operations allowed for evaluation */
    long long gas_used;  /* Current operations consumed */
    
    jmp_buf recover;     /* For sandboxing escape */
    ks_status last_status;
    char last_err_msg[256];
} ks_ctx;

/* Context Lifecycle */
ks_ctx* ks_create(size_t mem_limit, long long gas_limit);
void ks_destroy(ks_ctx *ctx);
void ks_clear_vars(ks_ctx *ctx);

/* Evaluation API */
K ks_eval(ks_ctx *ctx, const char *code, size_t len);
const char* ks_strerror(ks_status status);

/* Internal-ish K Lifecycle (now requires context for tracking) */
K k_new(ks_ctx *ctx, int n);
void k_free(ks_ctx *ctx, K x);

/* Function support */
K k_func(ks_ctx *ctx, char *body);
K k_call(ks_ctx *ctx, K fn, K *call_args, int nargs);
int k_is_func(K x);
char* k_func_body(K x);

/* C API Integration */
K k_view(ks_ctx *ctx, int n, double *ptr);
void bind_scalar(ks_ctx *ctx, char name, double val);
K k_get(ks_ctx *ctx, char name);

/* Output Helper */
void p(ks_ctx *ctx, K x);

#endif
