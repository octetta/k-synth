/ --- k/synth v0.0.5 Test Suite ---

/ 1. Verify basic math and right-associativity
10 - 5 - 2 / should be 7

/ 2. Verify Monadic composition with Scan and Reshape
\ 5 # 1 2 / should be 1 3 4 6 7

/ 3. Generate 16 samples of a 440Hz Sine wave
s (2 * p * 440 % 44100) * ! 16

/ 4. Test Additive synthesis peak
(s (2 * p * 100 % 16) * ! 16) + (s (2 * p * 200 % 16) * ! 16)

/ 5. Test Floor (Bitcrusher) logic
0.5 * _ 2 * s (2 * p * 50 % 16) * ! 16
