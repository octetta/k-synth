/ analyze.k - Signal Quality Check
/ 1. Create a test vector of 1000 steps
V: ! 1000
/ 2. Generate a sine wave
S: s V % 10
/ 3. Check stats
M: m S
V: v S
/ Results should appear in REPL
M
V
