# Technical Specification: k/synth

### 1. Architectural Constraints
* **Variable Limit**: Variables must be single characters (A-Z).
* **Evaluation**: Strict right-associativity ($A + B \times C$ calculates $B \times C$ first).
* **Playback**: `\p` strictly accepts one variable; no inline expressions.

### 2. The Language (Verbs & Operators)
| Type | Symbol | Function |
| :--- | :--- | :--- |
| Monadic | `s, t, h, a, q, l, e, _, ~, !, r, p` | Sine, Tan, Tanh, Abs, Sqrt, Log, Exp, Floor, Rev, Iota, Rand, Pi |
| Dyadic | `+ - * % ^ & | ~ . ! # f y` | Math, Power, Clip, Lag, Dot, Mod, Reshape, Filter, Delay |

### 3. Structural Logic
The engine utilizes a custom `K` struct:
* `r`: Reference count
* `n`: Array length
* `f`: Double-precision data pointer
