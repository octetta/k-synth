# ksynth web — user guide

## overview

ksynth web is a browser-based live-coding environment for the ksynth synthesis language. It compiles ksynth scripts in WebAssembly and plays audio directly through the Web Audio API.

The interface has three functional areas:

- **slot strip** — two rows of 8 across the top, slots `0`–`F`. Holds audio buffers ready to trigger.
- **notebook** — the left panel (or upper area on mobile). An append-only log of every script you have run, with waveform previews.
- **editor** — the right panel (or lower area on mobile). Where you write and run ksynth code.

A **pad panel** overlay turns the 16 slots into a playable instrument. A **patches** button fetches `.ks` files directly from the `octetta/k-synth` GitHub repo.

The toolbar has **save** and **load** buttons for persisting sessions as `.json` files, a **theme** toggle (retro / dark / light) remembered across reloads, and **guide** / **readme** buttons that open documentation in a new tab.

---

## setup

After building with `build.sh`, serve the directory with any static file server:

```sh
python3 -m http.server 8080
```

Open `http://localhost:8080`. The status indicator in the bottom-right of the editor area reads `wasm ready` in green when the engine is available. Audio initialises on the first user gesture.

---

## tutorial

This section walks from zero to a playable melodic instrument. No prior ksynth knowledge required.

### step 1 — your first sound

Type the following into the editor and press `Ctrl+Enter`:

```
N: 4410
T: !N
W: w s +\(N#(440*(6.28318%44100)))
```

You should hear a 100ms sine wave at 440 Hz (concert A). A green cell appears in the notebook showing the waveform. What each line does:

- `N: 4410` — 4410 samples = 100ms at 44100 Hz
- `T: !N` — time index: a ramp from 0 to N−1
- `W: ...` — the output buffer. `w s` writes samples. `+\(N#...)` is a phase accumulator (running sum over N samples). `440*(6.28318%44100)` is the per-sample phase increment for 440 Hz

`W` is always the output. Every script must set it.

### step 2 — a bell sound

A real bell sound comes from FM synthesis, not additive partials. The classic two-operator FM bell works like this: a carrier oscillator is phase-modulated by a modulator oscillator. The modulation index (how much the modulator deflects the carrier's phase) starts high — creating a bright, complex spread of sidebands at the attack — then decays fast, leaving a cleaner tone underneath. The amplitude decays more slowly. That combination of fast index decay plus slow amplitude decay is what makes it sound like a struck bell.

ksynth's right-associative parse makes FM natural. `s P + Q` parses as `s(P + s(Q))` — carrier phase plus modulator sine — which is exactly the FM formula.

```
N: 88200
T: !N
A: e(T*(0-3%N))
I: 3.5*e(T*(0-40%N))
C: 440*(6.28318%44100)
M: 440*(6.28318%44100)
P: +\(N#C)
Q: +\(N#M)
W: w A*(s P+(I*s Q))
```

- `N: 88200` — 2 seconds at 44100 Hz
- `A: e(T*(0-3%N))` — amplitude envelope: decays over ~2 seconds
- `I: 3.5*e(T*(0-40%N))` — modulation index: starts at 3.5, decays to near zero in about 50ms. This fast transient is the clang of the strike. Index 3.5 means a peak frequency deviation of ±1540 Hz — enough for a bright attack without turning into noise
- `C` — carrier phase increment: 440 Hz
- `M` — modulator at the same frequency as the carrier (ratio 1:1). This places sidebands at 0 Hz, 880 Hz, 1320 Hz, etc. With a decaying index the partials shift in amplitude over time, which is what gives the bell its characteristic evolving tone
- `P`, `Q` — carrier and modulator phase accumulators
- `W: w A*(s P+(I*s Q))` — right-assoc parses as `A × sin(P + I×sin(Q))`, the FM formula

**Experiment:**

Change `40` in the index decay to `80` for a sharper, drier attack, or `20` for a longer metallic clang. Change `3.5` (peak index) to `2` for a softer bell or `5` for something harsher. Change the amplitude decay constant `3` to `6` for a shorter ring or `1.5` for a long lingering tone. Change the modulator ratio from `1.0` to `1.4` for a more inharmonic, tubular bell quality.

### step 3 — bank it to a slot

After running the bell script, find its cell in the notebook. Click `→0` to bank the buffer to slot 0. The slot card at the top updates to show the waveform and a label taken from the script.

Click the slot card to play it back. Right-click it to rename it `bell` or adjust its base pitch.

### step 4 — play it melodically

This is where it gets interesting. The buffer you synthesised is a fixed recording of one pitch. The pad panel can play it back at different rates to hit different pitches — no re-synthesis needed.

1. With the bell in slot 0, click the `pads` button.
2. Click the **melodic** preset at the top of the panel.
3. All 16 pads now play slot 0 at different semitone offsets:

```
row 0 (top)    +9   +10   +11   +12  st
row 1          +5    +6    +7    +8  st
row 2          +1    +2    +3    +4  st
row 3 (btm)    0    -2    -4    -7  st
```

The playback rate for each pad is `2^(semitones/12)`. At 0 semitones, the bell plays at its original pitch. At +12 it plays an octave up (2× speed). At −7 it plays a perfect fifth below (≈0.707× speed).

The bottom row gives you root, a whole tone down, a minor third down, and a perfect fifth down — common harmonic intervals for bass movement. The upper rows ascend chromatically. Click the pads. You are playing the bell at different pitches.

**Tip:** right-click the slot card and use **set base rate** to shift all pads together by a fixed number of semitones if you want to transpose the whole instrument.

### step 5 — a second voice

Run a second patch — a softer marimba-like FM tone — and bank it to slot 1:

```
N: 132300
T: !N
A: e(T*(0-1.5%N))
I: 2*e(T*(0-40%N))
C: 220*(6.28318%44100)
M: 220*(6.28318%44100)
P: +\(N#C)
Q: +\(N#M)
W: w A*(s P+(I*s Q))
```

220 Hz (an octave below the bell), 3-second decay, index 2 for a gentler attack. Rounder and warmer than the bell. Bank it to slot 1.

Now right-click some pads in the panel. Set them to slot 1 with semitone offsets. You now have two voices: some pads play the bell, others play the soft tone, each at its own pitch.

**Drum preset vs melodic preset:** the **drum** preset assigns pad N to slot N at 0 semitones — one pad per voice, good for kits. **Melodic** points all pads at slot 0 for pitched play. These are just starting points; right-click any pad to configure it independently.

---

## editor

Write ksynth code one assignment per line. Comments use `/`.

```
/ 440 Hz sine with amplitude envelope
N: 44100
T: !N
F: 440*(6.28318%44100)
P: +\(N#F)
E: e(T*(0-6%N))
W: w (s P)*E
```

Note: `w s (E*P)` would be wrong — it scales the phase ramp, sweeping pitch to zero. `(s P)*E` correctly scales the sine output by the envelope.

`Ctrl+Enter` or `Cmd+Enter` to run. All variables clear before each run — each script is a complete standalone program.

`Ctrl+L` or `clear` removes all cells from the notebook. Does not affect slots.

---

## history navigation

- `↑` at the **first line** of the editor — loads the previous run
- `↓` at the **last line** — moves forward through history; at the most recent entry, clears the editor

History is in-memory and resets on page reload.

---

## notebook

Each run appends a cell. Successful cells show status, source, waveform, and bank buttons (`→0`–`→F`). Failed cells show the error in red.

Click the **source code** in any cell to copy it back into the editor. Click the **waveform** to audition it without banking. Hover a cell and click **✕** to remove it.

---

## slots

16 slots in a 2×8 grid at the top, indexed `0`–`F`. Each holds a buffer, a label, and a base pitch offset.

**Banking** — click `→N` in a notebook cell.

**Playing** — click any filled slot card.

**Context menu** (right-click):

- `▶ play` — play the slot
- `rename` — set a custom label
- `set base rate` — base pitch offset in semitones (±24). Affects all pads on this slot.
- `clear slot` — empty the slot

---

## pad panel

Click `pads` to open. `Escape` or click outside to close.

4×4 grid of 16 trigger pads. Each pad has a slot and a semitone offset. Final playback rate:

```
rate = 2^(slot.baseSemitones/12) × 2^(pad.semitones/12)
```

Right-click any pad to configure it.

**Keyboard triggers** (panel must be open, editor must not have focus):

| Keys | Pads |
|------|------|
| `1`–`9` | pads 0–8 |
| `0` | pad 9 |
| `a`–`f` | pads 10–15 |

Each pad button shows its key shortcut in the top-right corner. The button flashes on trigger whether triggered by key or click.

---

## save / load

Click `save` to download the current session as a `.json` file named `ksynth-YYYY-MM-DD-HH-MM-SS.json`. The file contains:

- all 16 slot buffers (audio encoded as base64), labels, and base rates
- all 16 pad assignments and semitone offsets
- the full script history with waveform data

Click `load` to restore a saved session. The notebook is rebuilt from the history, slots are repopulated, and the pad grid is restored. No scripts are re-run — audio is decoded directly from the file.

---

## patches browser

Click `patches` to open. On first open it fetches the file tree from `octetta/k-synth` on GitHub (requires internet; unauthenticated GitHub API, 60 req/hr limit).

Files are listed grouped by directory. Type to filter. Click any file to load its content into the editor. The script is not run automatically — review and press `Ctrl+Enter` when ready.

---

## notes and limitations

**Variable isolation** — variables `A`–`Z` clear before each run. No state carries between cells.

**Sample rate** — fixed at 44100 Hz.

**Audio headroom** — a master gain stage (0.25×) followed by a dynamics limiter sits between all voices and the output. This prevents clipping when multiple pads fire simultaneously at the cost of an overall level reduction of about 12 dB. If you need more volume, use an external mixer or turn up your system volume.

**Audio context** — Web Audio requires a user gesture before audio plays. Click anywhere to resume if audio stops.

**Persistence** — nothing survives a page reload. Keep scripts as `.ks` files on disk or in the repo and reload via the patches browser.
