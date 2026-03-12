/ Test Floating Point Exception Protection

/ Test 1: Division by zero (already protected)
A: 5 % 0
\v A
/ Expected: 0

/ Test 2: Log of zero (protected with 1e-10)
B: l 0
\v B
/ Expected: large negative number, not -inf

/ Test 3: Huge exponential (clamped to 100)
C: e 1000
\v C
/ Expected: exp(100), not inf

/ Test 4: Negative under exp
D: e -1000
\v D
/ Expected: exp(-100), not 0

/ Test 5: sqrt of negative (already protected with fabs)
E: q -25
\v E
/ Expected: 5

/ Test 6: Filter with extreme parameters
F: 1 10 f (r !100)
\v F
/ Expected: stable output, not NaN

/ Test 7: Delay feedback
G: 10 y (r !100 * 10)
\v G
/ Expected: stable output, not explosion

/ Test 8: Huge power
H: 2 ^ 1000
\v H
/ Expected: 0 (protected by safe_val)

/ Test 9: NaN propagation check
I: (0 % 0) + 5
\v I
/ Expected: 5 (0 + 5, not NaN)
