/ --- 1. Timing & Length ---
N:44100
L:!N

/ --- 2. Create the Melody (S) ---
/ Three notes: 400 (Low), 300 (Mid), 600 (High)
Q:400,300,600
/ Reshape Q to be the length of N (44100)
/ This stretches the three notes across the whole second
S:N # Q

/ --- 3. The Pitch-Shifting Oscillator ---
/ Instead of a fixed number, we mod L by our melody array S
M:L ! S
M:(M * 0.005) - 1

/ --- 4. The Per-Note Filter Envelope ---
/ To make each note 'pop', the filter should reset every 1/3 second
/ N%3 is 14700
B:L % 14700
E:1 - (B % 14700)
C:0.02 + (0.4 * (E * E))
R:3.8

/ --- 5. The Mix ---
X:(C,R) f M
X:h 3 * X
\p X
