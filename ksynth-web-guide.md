# ksynth web — user guide

## overview

ksynth web is a browser-based live-coding environment for the ksynth synthesis language. It compiles ksynth scripts in WebAssembly and plays audio directly through the Web Audio API. The interface has three functional areas stacked vertically: the slot strip at the top, the notebook in the middle, and the editor at the bottom. A pad panel is available as an overlay.

---

## setup

After building with `build.sh`, serve the directory with any static file server:

```sh
python3 -m http.server 8080
```

Then open `http://localhost:8080` in a browser. Wasm loads asynchronously — the status indicator in the bottom-right corner of the editor area will read `wasm ready` in green when the engine is available. Audio initialises on the first user gesture (click or keypress).

---

## editor

The editor is the `<textarea>` at the bottom of the screen. Write ksynth code here exactly as you would write a `.ks` file — one assignment per line, comments with `/`.

```
N: 4410
T: !N
F: 440*(6.28318%44100)
P: +\(N#F)
W: w s P
```

The `W` variable is the output. Any script that sets `W` will produce audio. A script that does not set `W` will produce an error.

**Running a script** — `Ctrl+Enter` (or `Cmd+Enter` on macOS), or click the `run ⌃↵` button. The script is evaluated as a complete program: all variables are cleared before each run, so each evaluation is independent.

**Clearing the notebook** — `Ctrl+L` or the `clear` button. This removes all cells from the notebook display but does not affect slots or audio buffers.

---

## history navigation

The editor maintains a history of every script you have run. Navigate it with the arrow keys:

- `↑` at the **first line** of the editor — loads the previous entry into the editor
- `↓` at the **last line** of the editor — moves forward through history; at the most recent entry, clears the editor

Navigating history copies the code into the editor for modification. Previous entries in the notebook remain read-only. The history is in-memory and does not persist across page reloads.

---

## notebook

Each time you run a script, a cell is appended to the notebook. Cells are read-only.

A successful cell shows:

- `✓` in green, followed by the sample count and duration in milliseconds
- The source code in muted text
- A waveform display — a single-channel oscilloscope view of the `W` buffer
- A row of bank buttons: `→0` through `→F`

A failed cell shows:

- `✗` in red, followed by the error message (typically `no W variable set` or a parse error)
- The source code

The notebook scrolls independently of the editor. Older cells scroll off the top.

---

## slots

The slot strip runs across the top of the screen. There are 16 slots, indexed `0` through `F` in hexadecimal.

Each slot holds an audio buffer, a label, and a base playback rate. An empty slot shows a dash. A filled slot shows its label and a miniature waveform.

**Banking a result to a slot** — after a successful script evaluation, click one of the `→N` bank buttons in the notebook cell. This copies the audio buffer into slot N and sets the slot label to the first meaningful line of the script.

**Playing a slot** — click any filled slot card in the strip. It plays once at the slot's base rate.

**Slot context menu** — right-click a slot card to access:

- `▶ play` — play the slot
- `rename` — set a custom label (prompt dialog)
- `set base rate` — set the slot's base pitch offset in semitones (prompt dialog, ±24 range). This affects all pads assigned to this slot.
- `clear slot` — remove the audio buffer from the slot

Slot base rate is stored as a semitone offset and displayed in the slot card (e.g. `+0st`, `-3st`). It is factored into playback together with any per-pad pitch offset.

---

## pad panel

Click the `pads` button (top-right of the editor header) to open the pad overlay. Click it again, press `Escape`, or click outside the panel to close it.

The pad panel contains a 4×4 grid of 16 trigger pads. Each pad has two properties:

- **slot** — which of the 16 sample slots it draws from
- **pitch** — a semitone offset applied at trigger time, on top of the slot's base rate

The final playback rate for any pad trigger is:

```
rate = slot.baseRate × 2^(pad.semitones / 12)
```

where `slot.baseRate` is itself `2^(slot.baseSemitones / 12)`.

**Triggering a pad** — click or tap any filled pad. It flashes green briefly and plays the audio once.

**Configuring a pad** — right-click any pad to open its config popup. Set the slot index (dropdown showing all 16 slots with labels) and the semitone offset (numeric input, ±24). Click `ok` to apply.

### presets

Two preset layouts are available via the buttons at the top of the pad panel:

**drum** (default) — each pad N triggers slot N at 0 semitones. Pad 0 plays slot 0, pad 1 plays slot 1, and so on. Intended for a kit where each slot holds a different voice.

**melodic** — all 16 pads trigger slot 0 at different pitches. The layout is:

```
row 0 (top)    +9   +10   +11   +12  st
row 1          +5    +6    +7    +8  st
row 2          +1    +2    +3    +4  st
row 3 (btm)    0    -2    -4    -7  st
```

The bottom row is root, down a tone, down a minor third, down a perfect fifth — common harmonic intervals. The upper rows ascend chromatically. Load one bass or melodic patch into slot 0 and the pads become a playable instrument.

Presets are starting points. Any pad can be reconfigured independently after applying a preset, so hybrid layouts (e.g. drums on pads 0–7, melodic on pads 8–15 from a different slot) work without restriction.

### keyboard shortcuts in the pad panel

When the pad panel is open and the editor does not have focus, number keys `1`–`9` and `0` trigger pads 0–9 respectively.

---

## typical workflows

### building a drum kit

1. Write a kick script in the editor, run it, bank the result to slot 0 (`→0`).
2. Write a snare script, run it, bank to slot 1 (`→1`).
3. Continue for hat, clap, toms, etc.
4. Open the pad panel. The **drum** preset is active by default — pad 0 plays slot 0, pad 1 plays slot 1, and so on.
5. Trigger pads to audition the kit.

### melodic / pitched instrument

1. Write a synth patch script, run it, bank the result to slot 0.
2. Open the pad panel, click **melodic**.
3. All 16 pads now play slot 0 at the pitches shown in the layout above.
4. Right-click any pad to override its semitone offset if you want a different scale.

### hybrid kit

1. Bank drums into slots 0–3.
2. Bank a bass patch into slot 4.
3. Open the pad panel.
4. Apply **drum** preset, then right-click pads 4–7 and set each to slot 4 with semitone offsets (e.g. 0, +2, +5, +7 for a pentatonic cluster).
5. Pads 0–3 trigger drums; pads 4–7 trigger the bass at different pitches.

### iterating on a sound

1. Run a script. If you want to refine it, press `↑` in the editor — the previous script loads.
2. Edit and re-run. A new cell appears in the notebook; the old one remains.
3. When satisfied, bank the new result to a slot. This overwrites any previous content in that slot.

---

## notes and limitations

**Variable isolation** — each script run clears all ksynth variables (`A`–`Z`) before evaluation. There is no state carried between notebook cells. If you need a long script, write it all in the editor at once; the editor supports multi-line input freely.

**Buffer length** — there is no enforced maximum, but very long buffers (several seconds) will take longer to evaluate and consume more memory. The Wasm heap grows as needed up to browser limits.

**Sample rate** — fixed at 44100 Hz. The `W` buffer is always interpreted at this rate.

**Audio context** — the Web Audio API requires a user gesture before audio can play. The context is created on the first click or keypress. If the page has been idle and audio stops working, clicking anywhere will resume it.

**Persistence** — nothing persists across page reloads. History, slot contents, and pad configuration are all in-memory. To save a patch, copy the script text out of a notebook cell (select the code text and copy manually) or keep your `.ks` files on disk and paste them into the editor as needed.
