# 🎹 Analog Synth Drum Machine Sounds

Classic analog drum synthesis techniques for creating warm, punchy, vintage drum sounds.

---

## 🎛️ Setup First

```k
/ Initialize
A: !8000          / Longer samples for decay
E: 1 - 0.00008 * A / Slow linear envelope
P: E ^ 6           / Very fast (transients)
M: E ^ 3           / Medium (body)
Q: E ^ 2           / Gentle (tails)
```

---

## 🥁 KICK DRUMS

### Classic 808-Style Kick (Pitch Sweep + Sine)

```k
/ Pitch envelope: starts high, drops fast
T: 1 - 0.0008 * A
T: T ^ 3           / Exponential drop

/ Sine wave that follows pitch envelope  
/ Starts ~120 Hz, drops to ~40 Hz
B: s p (0.00272 + (0.00454 * T)) * A

/ Apply amplitude envelope
K: 0.8 & -0.8 | 5 * B * M * M

\p K
\s K
```

### Deep Sub Kick (Pure Low End)

```k
/ Constant 50 Hz tone
B: s p 0.00227 * A

/ Very steep envelope for punch
K: 0.9 & -0.9 | 6 * B * E ^ 4

\p K
```

### 909-Style Kick (Chirp + Click)

```k
/ Body: pitch sweep using chirp
B: s 0.006 * A

/ Click: brief high frequency
C: s 0.08 * A

/ Layer with different envelopes
K: 0.7 & -0.7 | (4 * B * E * E) + (0.4 * C * P * P)

\p K
```

### Punchy Kick (Attack + Sustain)

```k
/ Low fundamental ~55 Hz
B: s p 0.0025 * A

/ Harmonic overtone for attack
H: s p 0.005 * A

/ Mix: sharp attack, sustained body
K: 0.75 & -0.75 | (3.5 * B * M) + (1.5 * H * P)

\p K
```

### Boomy 808 (Long Decay)

```k
/ Ultra-low 35 Hz
B: s p 0.00159 * A

/ Slow envelope with gentle curve
S: 1 - 0.00005 * A

/ Soft saturation
K: h 4 * B * S * S

\p K
```

---

## 🥁 SNARE DRUMS

### 808 Snare (Dual Oscillator + Noise)

```k
/ Two detuned oscillators for "buzz"
/ ~160 Hz
T: s p 0.0036 * A
/ ~170 Hz (slightly sharp)
U: s p 0.00385 * A

/ Noise component (heavy filtering)
N: 0.65 f r A

/ Mix: tonal body + noise snap
S: 0.8 & -0.8 | (2 * (T + U) * M) + (1.2 * N * M)

\p S
```

### 909 Snare (Resonant Filter)

```k
/ Tonal part with slight pitch drop
T: s p 0.004 * A * (1 - 0.0001 * A)

/ Resonant filtered noise for "snap"
N: (0.4;2.8) f r A

/ Combine
S: 0.85 & -0.85 | (1.8 * T * M) + (1.5 * N * P)

\p S
```

### Clap (Noise Burst)

```k
/ Multiple noise bursts for "flam"
N: 0.55 f r A

/ Modulation for texture
L: s 45 * A

/ Create burst pattern
C: N * (P + (0.25 * P * L) + (0.15 * P * P))

\p C
```

### Rim Shot (Metallic)

```k
/ Two metallic frequencies (inharmonic)
/ ~300 Hz
S:44100
X:300%F
Y:440%F
T: s p X * A
/ ~440 Hz
U: s p Y * A

/ Very sharp envelope
R: 0.7 & -0.7 | 3 * (T + U * 0.6) * P * P

\p R
```

---

## 🎩 HI-HATS

### Closed 808 Hi-Hat (Metallic)

```k
/ Six square waves (inharmonic ratios)

/ 4:1 ratio
F: (s 0.04 * A > 0) - 0.5

/ Slightly off
G: (s 0.043 * A > 0) - 0.5

/ More off
H: (s 0.051 * A > 0) - 0.5

I: (s 0.057 * A > 0) - 0.5
J: (s 0.063 * A > 0) - 0.5  
L: (s 0.071 * A > 0) - 0.5

/ Sum and filter
M: F + G + H + I + J + L
N: 0.92 f M

/ Very short envelope
H: N * P * P * 0.5

\p H
```

### Closed 909 Hi-Hat (Filtered Noise)

```k
/ Very bright filtered noise
H: (0.98 f r A) * P * 0.6

\p H
```

### Open Hi-Hat (Long Decay)

```k
/ Same as closed but longer envelope
N: 0.94 f r A
O: N * E * 0.7

\p O
```

### Pedal Hi-Hat (Foot Chick)

```k
/ Low thump
T: s p 0.009 * A

/ Short noise burst  
N: 0.96 f r A

/ Combine
H: 0.6 & -0.6 | (1.5 * T * P * P) + (0.4 * N * P)

\p H
```

---

## 🪘 TOMS (Pitched Drums)

### Low Tom (~80 Hz)

```k
/ Fundamental with harmonic
B: s p 0.00363 * A
H: s p 0.00726 * A * 0.3

/ Add subtle noise for attack
N: 0.75 f r A * P

/ Combine
T: 0.75 & -0.75 | (3 * (B + H) * M) + (0.5 * N)

\p T
```

### Mid Tom (~110 Hz)

```k
B: s p 0.00499 * A
H: s p 0.00998 * A * 0.3
N: 0.75 f r A * P
T: 0.75 & -0.75 | (3 * (B + H) * M) + (0.5 * N)

\p T
```

### High Tom (~150 Hz)

```k
B: s p 0.0068 * A
H: s p 0.0136 * A * 0.3
N: 0.75 f r A * P
T: 0.75 & -0.75 | (3 * (B + H) * M) + (0.5 * N)

\p T
```

### Tuned Tom (With Pitch Bend)

```k
/ Pitch envelope (drops 20%)
D: 1 - 0.0002 * A

/ Fundamental at ~100 Hz with bend
B: s p (0.00454 * D) * A

/ Tom sound
T: 0.75 & -0.75 | 3.5 * B * M

\p T
```

---

## 🔔 PERCUSSION

### Cowbell (Two Tones)

```k
/ Two metallic frequencies (inharmonic)
R: 44100

/ ~900 Hz
X: 900 % R
T: s p X * A

/ ~1350 Hz
Y: 1350 % R
U: s p Y * A

/ Sharp attack, medium decay
C: 0.7 & -0.7 | (T + U * 0.8) * M

\p C
```

### Clave (Short Wood Block)

```k
/ Very high pitch ~2.5 kHz
T: s p 0.1134 * A

/ Extremely short
C: T * P * P * P * 0.6

\p C
```

### Conga (Low)

```k
/ ~100 Hz with pitch drop
D: 1 - 0.001 * A
B: s p (0.00454 * D) * A

/ Plus noise for "skin" texture
N: 0.5 f r A

/ Sharp but not as tight as kick
C: 0.7 & -0.7 | (2.5 * B * P) + (0.3 * N * P)

\p C
```

### Conga (High)

```k
/ ~200 Hz with pitch drop
D: 1 - 0.001 * A
B: s p (0.00907 * D) * A
N: 0.5 f r A
C: 0.7 & -0.7 | (2.5 * B * P) + (0.3 * N * P)

\p C
```

### Tambourine

```k
/ Multiple jingly frequencies
R: 44100

/ ~600 Hz
F: s p (600 % R) * A

/ ~900 Hz
G: s p (900 % R) * A

/ ~1200 Hz
H: s p (1200 % R) * A

/ Bright noise
N: 0.6 f r A

/ Combine
T: (F + G + H + N) * M * 0.6

\p T
```

### Shaker

```k
/ Very bright noise
N: 0.45 f r A

/ Very short bursts
S: N * P * P * 0.5

\p S
```

---

## 🎸 SYNTH EFFECTS

### Laser Zap

```k
/ Extreme pitch sweep down
D: (1 - 0.0005 * A) ^ 2
L: s p (0.05 * D) * A
Z: L * M * 2

\p Z
```

### Explosion

```k
/ Low rumble with noise burst
R: 44100
/ ~50 Hz
B: s p (50 % R) * A
N: 0.4 f r A

/ Pitch drops, noise fades
D: 1 - 0.0002 * A
X: (2 * B * (D ^ 2)) + (N * P)

\p X
```

### Blip

```k
/ Short high tone
/ ~1000 Hz
R: 44100
B: s p (2000%R) * A
L: B * P * P * P * 0.7

\p L
```

---

## 🎚️ Advanced Techniques

### FM Kick (Frequency Modulation)

```k
/ Modulator (fast pitch sweep)
M: s p 0.0136 * A * (1 - 0.0008 * A)

/ Carrier modulated by modulator
/ Creates complex harmonic content
C: s p (0.00227 + (0.005 * M)) * A

/ Apply envelope
K: 0.8 & -0.8 | 4 * C * E * E

\p K
```

### Resonant Kick (Self-Oscillating Filter)

```k
/ Low-level noise to trigger filter
N: r A * 0.05

/ Very high resonance on low cutoff
/ Filter rings at ~60 Hz
K: (0.06;3.95) f N

/ Shape with envelope
K: 0.85 & -0.85 | K * M * 4

\p K
```

### Detuned Snare (Analog Drift)

```k
/ Three slightly detuned oscillators
T: s p 0.004 * A
/ +1.25% sharp
U: s p 0.00405 * A
/ -1.25% flat
V: s p 0.00395 * A

/ Filtered noise
N: 0.7 f r A

/ Mix all
S: 0.8 & -0.8 | (1.5 * (T + U + V) * M) + (N * M)

\p S
```

### Gated Reverb Snare

```k
/ Basic snare
T: s p 0.004 * A
N: 0.7 f r A
B: (1.5 * T * M) + (N * M)

/ Reverb tail (multiple delays)
D: B 1000 y B
D: D 1500 y D
D: D 2200 y D

/ Gate the reverb (hard cut)
G: 1 - 0.0003 * A
G: G > 0.3

/ Combine
S: 0.8 & -0.8 | B + (D * G * 0.4)

\p S
```

### Stereo Hi-Hat (Width)

```k
/ Left and right slightly different
L: (0.98 f r A) * P * 0.6
R: (0.975 f r A) * P * 0.6

/ Interleave
H: L z R

\ps H
\ss H
```

---

## 🎼 Complete Analog Kit

```k
/ Setup
A: !12000
E: 1 - 0.00008 * A
P: E ^ 6
M: E ^ 3

/ === KICK ===
/ Pitch sweep method
T: 1 - 0.0008 * A
T: T ^ 3
B: s p (0.00272 + (0.00454 * T)) * A
K: 0.8 & -0.8 | 5 * B * M * M

/ === SNARE ===
T: s p 0.0036 * A
U: s p 0.00385 * A
N: 0.65 f r A
S: 0.8 & -0.8 | (2 * (T + U) * M) + (1.2 * N * M)

/ === CLOSED HAT ===
H: (0.98 f r A) * P * 0.6

/ === OPEN HAT ===
O: (0.94 f r A) * E * 0.7

/ === LOW TOM ===
B: s p 0.00363 * A
C: s p 0.00726 * A * 0.3
N: 0.75 f r A * P
D: 0.75 & -0.75 | (3 * (B + C) * M) + (0.5 * N)

/ === HIGH TOM ===
B: s p 0.0068 * A
C: s p 0.0136 * A * 0.3
N: 0.75 f r A * P
F: 0.75 & -0.75 | (3 * (B + C) * M) + (0.5 * N)

/ === CLAP ===
N: 0.55 f r A
L: s 45 * A
C: N * (P + (0.25 * P * L) + (0.15 * P * P))

/ === RIM ===
T: s p 0.0068 * A
U: s p 0.01 * A
R: 0.7 & -0.7 | 3 * (T + U * 0.6) * P * P

/ === COWBELL ===
T: s p 0.0204 * A
U: s p 0.0306 * A
B: 0.7 & -0.7 | (T + U * 0.8) * M

/ Play them
\p K
\p S
\p H
\p O
\p D
\p F
\p C
\p R
\p B

/ Save them
\s K
\s S
\s H
```

---

## 💡 Analog Character Tips

### 1. Pitch Instability (Analog Drift)

```k
/ Add very slow LFO to pitch
L: s 0.00005 * A * 0.02
B: s p (0.00454 + L) * A * E
```

### 2. Saturation (Warmth)

```k
/ Tanh for analog-style soft clipping
T: s p 0.005 * A
W: h 3 * T * E      / Warm saturation
```

### 3. Filter Resonance (Character)

```k
/ Moderate resonance adds punch
N: (0.5;1.8) f r A
```

### 4. Detuning (Thickness)

```k
/ Multiple oscillators slightly off
T: s p 0.005 * A
U: s p 0.00505 * A
K: (T + U) * E * 0.5
```

### 5. Noise Texture

```k
/ Add subtle noise to everything
T: s p 0.005 * A
N: r A * 0.05
K: (T + N) * E
```

---

## 🎯 Frequency Reference (Analog Standards)

```k
/ KICKS
/ 808: 35-65 Hz
T: s p 0.00159 * A  / 35 Hz
T: s p 0.00295 * A  / 65 Hz

/ SNARES
/ 808/909: 150-220 Hz
T: s p 0.0068 * A   / 150 Hz
T: s p 0.00998 * A  / 220 Hz

/ TOMS
/ Low: 65-85 Hz
/ Mid: 85-110 Hz  
/ High: 110-150 Hz

/ COWBELL: 800-1000 Hz
/ CLAVE: 2-3 kHz
/ HI-HATS: Noise (filtered 8-12 kHz)
```

---

## 🔬 Sound Design Workflow

1. **Start with the right frequency**
   - Use the frequency reference
   - `\p` to test

2. **Shape the envelope**
   - Kicks: E^3 to E^4
   - Snares: E^2 to E^3
   - Hi-hats: E^5 to E^6

3. **Add harmonics or noise**
   - Harmonics for tone
   - Noise for texture

4. **Add character**
   - Slight detuning
   - Saturation
   - Filtering

5. **Clip appropriately**
   - `0.8 & -0.8 |` for punch
   - Not too hot (distortion)

---

**Trust the analog formulas. These are time-tested techniques.** 🎛️
