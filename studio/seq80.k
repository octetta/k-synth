/ --- Setup ---
N:44096
L:!N
/ E: Kick/Gate envelope (1.0 down to 0.0)
E:1.0 - e -10 * L % 11024

/ --- B: The Bassline ---
/ Sawtooth filtered with liquid cutoff
W:((L ! 400) * 0.005) - 0.25
C:0.15 + 0.1 * s L % 10000
B:(C,3.5) f W

/ --- S: The Sidechained Result ---
/ This ducks the volume of the bass at every kick interval
S:E * B

/ --- Play the single variable ---
\p S
