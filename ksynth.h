#ifndef KSYNTH_H
#define KSYNTH_H

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdint.h>

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

/* Wrapper API (context handle based, suitable for WebAssembly and embedders) */
uintptr_t ks_ctx_create(void);
void ks_ctx_destroy(uintptr_t handle);
int ks_ctx_run(uintptr_t handle, const char *script);
int ks_ctx_repl(uintptr_t handle, const char *expr);
const char *ks_ctx_repl_str(uintptr_t handle);
int ks_ctx_get_var(uintptr_t handle, int letter_upper);
float *ks_ctx_get_var_buf(uintptr_t handle);
int ks_ctx_repl_length(uintptr_t handle);
int ks_ctx_repl_get_floats(uintptr_t handle, float *out, int max_n);
float *ks_ctx_get_buffer(uintptr_t handle);
int ks_ctx_get_length(uintptr_t handle);
const char *ks_ctx_get_error(uintptr_t handle);

/* Legacy singleton wrappers (kept for compatibility) */
void ks_init(void);
int ks_run(const char *script);
int ks_repl(const char *expr);
const char *ks_repl_str(void);
int ks_get_var(int letter_upper);
float *ks_get_var_buf(void);
int ks_repl_length(void);
int ks_repl_get_floats(float *out, int max_n);
float *ks_get_buffer(void);
int ks_get_length(void);
const char *ks_get_error(void);

#endif
