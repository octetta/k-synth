/ --- 1. The Notes (Re-using our successful blocks) ---
L:!5512
R:3.8
E:1 - (L/5512)

/ Low
M:((L!400)*0.005)-1
A:( (0.05+0.8*E*E),R ) f M

/ Mid
M:((L!300)*0.006)-1
B:( (0.05+0.9*E*E),R ) f M

/ High
M:((L!200)*0.01)-1
D:( (0.1+0.8*E*E),R ) f M

/ --- 2. Assemble the Pattern ---
X:A,B,D,B

/ --- 3. The Delay (y) ---
/ 4134 samples is roughly a 'dotted 16th' at 44.1k
/ This makes the echoes fall between the notes
X:4134 y X

/ --- 4. Final Saturation ---
/ We saturate AFTER the delay so the echoes also growl
X:h 6 * X
\p X
