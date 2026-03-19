#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ksynth.h"

int total = 0, passed = 0;

void test(const char* label, const char* code, int exp_n, double exp_val) {
    total++;
    char* s = strdup(code);
    char* p = s;
    K res = e(&p);
    
    int ok = 0;
    if (res) {
        // Check length and the first element (for scalar/vector representative checks)
        if (res->n == exp_n && (exp_n == 0 || fabs(res->f[0] - exp_val) < 0.001)) {
            ok = 1;
        }
    }

    printf("[%s] %-15s -> ", ok ? "PASS" : "FAIL", code);
    if (!res) {
        printf("NULL\n");
    } else {
        printf("n:%d v[0]:%.4f\n", res->n, res->n > 0 ? res->f[0] : 0.0);
        k_free(res);
    }
    
    if (ok) passed++;
    free(s);
}

int main() {
    printf("=== K-SYNTH COMPREHENSIVE VERB TEST ===\n\n");

    // --- MONADIC VERBS ---
    test("IOTA", "!3", 3, 0.0);          // 0 1 2
    test("PHASE", "~4", 4, 0.0);         // 0 to 2pi ramp
    test("SUM", "+ 1 2 3", 1, 6.0);      // Reduction
    test("MAXABS", "> -10 5", 1, 10.0);  // Peak detection
    test("NORM", "w 0.5 0.5", 2, 1.0);   // Peak normalize
    test("SINE", "s 0", 1, 0.0);         // sin(0)
    test("COS", "c 0", 1, 1.0);          // cos(0)
    test("TANH", "h 100", 1, 1.0);       // Soft clip
    test("ABS", "a -5", 1, 5.0);         // Absolute value
    test("SQRT", "q 16", 1, 4.0);        // Square root
    test("EXP", "e 0", 1, 1.0);          // e^0
    test("FLOOR", "_ 5.7", 1, 5.0);      // Floor
    test("PI", "p 1", 1, 3.1416);        // pi scaling
    test("REV", "i 1 2 3", 3, 3.0);      // Reverse
    test("NOTE", "n 69", 1, 440.0);      // MIDI to Hz
    test("RAND", "r 0", 1, 0.0);         // Random [-1, 1]

    // --- DYADIC OPERATORS ---
    test("ADD", "10+20", 1, 30.0);       // Math
    test("SUB", "50-10", 1, 40.0);
    test("MUL", "6*7", 1, 42.0);
    test("DIV", "100%4", 1, 25.0);
    test("POW", "2^3", 1, 8.0);          // Power
    test("MIN", "10&5", 1, 5.0);         // Minimum
    test("MAX", "10|5", 1, 10.0);        // Maximum
    test("EQ", "5=5", 1, 1.0);           // Comparison
    test("JOIN", "1,2,3", 3, 1.0);       // Concatenate
    test("RESHAPE", "3#9", 3, 9.0);      // Repeat

    // --- DSP SPECIALS ---
    test("OUTER", "0 o 1 2", 1, 0.0);    // Outer-product sum
    test("WEIGHT", "0 $ 0.5", 1, 0.0);   // Weighted sum
    test("CRUSH", "2 v 0.8", 1, 0.5);    // floor(0.8*2)/2
    test("DELAY", "(1 0.5) y 1 0 0", 3, 1.0); // Feedback
    test("FILTER", "(1000 0.5 2) g 5#1", 5, 1.0); // SVF

    // --- ADVERBS ---
    test("SCAN_ADD", "+\\ 1 1 1", 3, 1.0); // 1 2 3

    printf("\n=== FINAL SCORE: %d/%d PASSED ===\n", passed, total);
    return (passed == total) ? 0 : 1;
}