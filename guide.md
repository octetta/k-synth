# ksynth web — user guide

## overview

ksynth web is a browser-based live-coding environment for the ksynth synthesis language. It compiles ksynth scripts in WebAssembly and plays audio directly through the Web Audio API.

The interface has three functional areas:

- **slot strip** — two rows of 8 across the top, slots `0`–`F`. Holds audio buffers ready to trigger.
- **notebook** — the left panel. An append-only log of every script you have run, with waveform previews.
- **editor** — the right panel. Where you write and run ksynth code.

A **pad panel** overlay turns the 16 slots into a playable instrument. A **patches** button fetches `.ks` files directly from the `octetta/k-synth` GitHub repo.

The toolbar has **save** and **load** buttons for persisting sessions as `.json` files, a **theme** toggle (retro / dark / light), and **guide** / **readme** buttons.

---

## setup

After building with `build.sh`, serve the directory with any static file server:

```sh
python3 -m http.server 8080
```

Open `http://localhost:8080`. The status indicator reads `wasm ready` in green when the engine is available. Audio initialises on the first user gesture.

---

## tutorial

### step 1 — your first sound

Type the following into the editor and press `Ctrl+Enter`:

```
N: 4410
T: !N
W: w s +\(N#(440*(6.28318%44100)))
```

You should hear a 100ms sine wave at 440 Hz. A cell appears in the notebook showing the waveform. What each line does:

- `N: 4410` — 4410 samples = 100ms at 44100 Hz
- `T: !N` — time index: a ramp from 0 to N−1
- `W: ...` — the output buffer. `w s` writes samples. `+\(N#...)` is a phase accumulator. `440*(6.28318%44100)` is the per-sample phase increment for 440 Hz

`W` is always the output. Every script must set it.

### step 2 — a bell sound

FM synthesis produces bell tones naturally. A carrier oscillator is phase-modulated by a modulator: the modulation index starts high — bright and complex at the attack — then decays fast. ksynth's right-associative parse makes FM natural: `s P + Q` parses as `s(P + s(Q))`.

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

- `A` — amplitude envelope, decays over ~2 seconds
- `I` — modulation index: starts at 3.5, decays to near zero in ~50ms
- `P`, `Q` — carrier and modulator phase accumulators
- `W` — `A × sin(P + I×sin(Q))`, the FM formula

**Experiment:** change `40` to `80` for a drier attack, `3.5` to `5` for something harsher, or the modulator to `616` Hz (ratio 1.4:1) for a tubular bell quality.

### step 3 — bank it to a slot

After running the bell script, find its cell in the notebook. Click `→0` to bank the buffer to slot 0. Click the slot card to play it. Right-click for the context menu.

### step 4 — play it melodically

1. With the bell in slot 0, click **pads**.
2. Click the **melodic** preset.
3. All 16 pads play slot 0 at different semitone offsets — click them to play the bell at different pitches.

The playback rate for each pad is `2^(semitones/12)`. Right-click the slot card and use **set base rate** to transpose the whole instrument.

### step 5 — a second voice

Bank a second sound to slot 1, then right-click pads to assign them to slot 1 with their own offsets. You now have two independent pitched voices across the grid.

---

## editor

Write ksynth code one assignment per line. Comments use `/`.

`Ctrl+Enter` or `Cmd+Enter` to run. Variables `A`–`Z` reset before each run — each script is self-contained.

`↑` at the first line loads the previous run. `↓` at the last line moves forward through history.

`Ctrl+L` clears the editor.

### REPL strip

The single-line input at the bottom of the editor is a persistent calculator. Type any ksynth expression and press **Enter**:

```
2+2           =>  4
440*6.28318   =>  2763.8
N             =>  44100    (if N was set in the last run)
!8            =>  [0  1  2  3  4  5 ...]  len=8
```

Variables persist between REPL entries and are shared with the main editor — run a script then inspect intermediate values here.

### inspect commands

Type these directly in the editor or REPL and press Enter (or they fire as you type the third character in the editor):

| Command | Action |
|---------|--------|
| `\pV` | Play variable `V` scaled to audio levels |
| `\vV` | Open a graph window showing variable `V` |

The graph window shows the waveform, min/max values, zero line, sample count and duration. `V` is any single letter `a`–`z`.

---

## notebook

Each run appends a cell. Click the **header** to toggle between collapsed (first line only) and expanded (full code). Click **`→ edit`** in the header to copy the code back to the editor. Click the **waveform** to audition without banking.

When a patch is loaded from the browser, the filename appears in the cell header.

---

## slots

16 slots in a 2×8 grid, indexed `0`–`F`. Each holds a buffer, a label, and a base pitch offset.

**Right-click** any slot for the context menu:

| Item | Action |
|------|--------|
| Play | Trigger the slot |
| Set base rate | Pitch offset in semitones (±24). Affects all pads on this slot. |
| Download WAV | Export as 16-bit mono WAV |
| Rename | Set a custom label |
| Clear slot | Empty the slot |

---

## pad panel

Click **`pads`** to open. `Escape` to close.

4×4 grid of 16 trigger pads. Each pad shows its slot, key shortcut, label, and semitone offset.

**Keyboard triggers** (editor must not have focus):

| Keys | Pads |
|------|------|
| `1`–`9` | pads 0–8 |
| `0` | pad 9 |
| `a`–`f` | pads 10–15 |

Right-click any pad to configure its slot and semitone offset. Scroll wheel adjusts semitone offset.

**Presets:** **drum** assigns pad N → slot N at 0 semitones. **melodic** assigns all pads to slot 0 at offsets −7 through +8.

---

## save / load

**save** downloads the session as a `.json` file containing all slot buffers, pad assignments, notebook history, and editor text. **load** restores it completely — no scripts are re-run.

Session files are compatible with ksynth-desktop.

---

## patches browser

**patches** fetches the file tree from `octetta/k-synth` on GitHub. Type to filter. Click any file to load it into the editor. Press `Ctrl+Enter` to run when ready.

---

## notes

**Variable isolation** — `A`–`Z` reset before each main editor run. The REPL preserves state between entries.

**`W` is the output** — every script must set `W`. No `W` → error cell.

**Wavetable oscillator** — `T t [freq n_samples]` plays vector `T` as a DDS oscillator. See readme for examples.

**Literal arrays** — every number after the first must start with a digit `0`–`9`. Use `0.5` not `.5`.

**Sample rate** — fixed at 44100 Hz.
