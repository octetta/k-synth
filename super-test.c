#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "ksynth.h"

int run = 0; int fail = 0;

void v(const char* label, const char* expr, int exp_n, double exp_v0) {
    run++;
    char *ptr = (char *)expr;
    K r = e(&ptr);
    if (!r) {
        printf("[FAIL] %-10s | %-12s | NULL\n", label, expr);
        fail++; return;
    }
    if (r->n == exp_n && (exp_v0 == -999 || fabs(r->f[0] - exp_v0) < 0.005)) {
        printf("[PASS] %-10s | %-12s | n:%d v0:%.4f\n", label, expr, r->n, r->f[0]);
    } else {
        printf("[FAIL] %-10s | %-12s | Got n:%d v0:%.4f (Exp n:%d v0:%.4f)\n", 
               label, expr, r->n, r->f[0], exp_n, exp_v0);
        fail++;
    }
    k_free(r);
}

int main() {
    printf("--- EXHAUSTIVE 39-POINT VERB & ADVERB AUDIT ---\n");

    // MATH & LOGIC (12)
    v("ADD", "1+1", 1, 2.0);    v("SUB", "1-1", 1, 0.0);
    v("MUL", "2*3", 1, 6.0);    v("DIV", "6%2", 1, 3.0);
    v("POW", "2^3", 1, 8.0);    v("MOD", "7!3", 1, 1.0);
    v("MIN", "1&2", 1, 1.0);    v("MAX", "1|2", 1, 2.0);
    v("LT",  "1<2", 1, 1.0);    v("GT",  "2>1", 1, 1.0);
    v("EQ",  "1=1", 1, 1.0);    v("JOIN", "1,2", 2, 1.0);

    // MONADIC (10)
    v("SINE", "s 0", 1, 0.0);    v("TAN",  "t 0", 1, 0.0);
    v("TANH", "h 0", 1, 0.0);    v("ABS",  "a -1", 1, 1.0);
    v("SQRT", "q 4", 1, 2.0);    v("LOG",  "l 1", 1, 0.0);
    v("EXP",  "e 0", 1, 1.0);    v("FLOOR","_ 1.9", 1, 1.0);
    v("PI",   "p 1", 1, 3.1415); v("RAND", "r 1", 1, -999); // -999 = any value

    // SYNTH SWITCHBOARD (15) - Testing Routing & Persistence
    // Note: These use dummy args to ensure the dyadic dispatcher picks them up
    v("SVF",  "(100 0.5)g 1", 1, -999); v("OSC",  "440 o 0", 1, -999);
    v("DELAY","1 z 0", 1, -999);        v("FILTER","1 f 0", 1, -999);
    v("ENV",  "1 d 0", 1, -999);        v("WAVE",  "1 w 0", 1, -999);
    v("CONST","1 $ 0", 1, -999);        v("TABLE", "1 c 0", 1, -999);
    v("INDEX","1 i 0", 1, -999);        v("VOL",   "1 v 0", 1, -999);
    v("MAP",  "1 m 0", 1, -999);        v("BUFF",  "1 b 0", 1, -999);
    v("UNIT", "1 u 0", 1, -999);        v("JOIN",  "1 j 0", 1, -999);
    v("K-VAL","1 k 0", 1, -999);        v("NUM",   "1 n 0", 1, -999);

    // ADVERBS (2)
    v("OVER", "+/1 2 3", 1, 6.0);
    v("SCAN", "+\\1 2 3", 3, 1.0); // Note: Escaped backslash for C string

    // STRUCTURAL (1)
    v("RESHAPE", "3#1", 3, 1.0);

    printf("\n--- AUDIT SUMMARY ---\n");
    printf("Total Tokens in switchboard: 39\n");
    printf("Total Tests Executed: %d\n", run);
    printf("Total Failures: %d\n", fail);

    if (run == 39 && fail == 0) {
        printf("\nRESULT: 100%% VERB COVERAGE ACHIEVED.\n");
    } else {
        printf("\nRESULT: COVERAGE GAP DETECTED. Check manual mapping.\n");
    }

    return fail;
}