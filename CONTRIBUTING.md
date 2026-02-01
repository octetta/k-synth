# Contributing to k/synth

### The "Simple" Test
Before submitting a Pull Request, ask: 
> "Can this be expressed as a combination of existing monadic or dyadic
verbs?" 

### Engineering Standards
* **Code is a Liability**: Prefer one line of dense, vectorized C over ten lines of iterative loops.
* **Memory**: Adhere strictly to the reference counting (`r`) logic within the C Kernel.
* **Minimalism**: If a feature isn't essential to "Base Camp" synthesis, it belongs in a user script, not the core.
