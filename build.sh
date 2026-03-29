#!/usr/bin/env bash
# build.sh — compile ksynth to WebAssembly via Emscripten, then build docs HTML
#
# Prerequisites:
#   source /path/to/emsdk/emsdk_env.sh
#
# To rebuild docs only (no Emscripten needed):
#   python3 docs-build.py
#
# Output:
#   ksynth.js, ksynth.wasm — WebAssembly engine
#   guide.html, readme.html — documentation
#
# Serve with: python3 -m http.server 8080

set -euo pipefail

if ! command -v emcc >/dev/null 2>&1; then
  echo "error: emcc not found in PATH."
  echo "hint: activate Emscripten first (emsdk_env.sh or mise shell)."
  echo "example (mise): eval \"\$(mise activate bash)\""
  exit 1
fi

EMCC_VERSION="$(emcc -dumpversion 2>/dev/null || true)"
if [ -z "$EMCC_VERSION" ]; then
  EMCC_VERSION="$(emcc -v 2>&1 | sed -n '1s/.*) //p')"
fi
echo "Using emcc: ${EMCC_VERSION:-unknown}"

EXPORTED_FUNCTIONS='[
  "_ks_ctx_create",
  "_ks_ctx_destroy",
  "_ks_ctx_run",
  "_ks_ctx_repl",
  "_ks_ctx_repl_str",
  "_ks_ctx_get_var",
  "_ks_ctx_get_var_buf",
  "_ks_ctx_get_buffer",
  "_ks_ctx_get_length",
  "_ks_ctx_get_error",
  "_ks_init",
  "_ks_run",
  "_ks_repl",
  "_ks_repl_str",
  "_ks_get_var",
  "_ks_get_var_buf",
  "_ks_get_buffer",
  "_ks_get_length",
  "_ks_get_error",
  "_malloc",
  "_free"
]'

EXPORTED_RUNTIME='[
  "ccall",
  "cwrap",
  "UTF8ToString",
  "stringToUTF8",
  "lengthBytesUTF8",
  "HEAPF32"
]'

emcc \
  ksynth.c \
  ks_api.c \
  -O2 \
  -lm \
  -s WASM=1 \
  -s MODULARIZE=1 \
  -s EXPORT_NAME=KSynth \
  -s EXPORTED_FUNCTIONS="$(echo $EXPORTED_FUNCTIONS | tr -d ' \n')" \
  -s EXPORTED_RUNTIME_METHODS="$(echo $EXPORTED_RUNTIME | tr -d ' \n')" \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s TOTAL_STACK=1mb \
  -s ENVIRONMENT=web \
  -o ksynth.js

echo "Build complete: ksynth.js + ksynth.wasm"
echo "Serve with: python3 -m http.server 8080"

# Build documentation HTML from markdown sources
echo "Building docs..."
python3 docs-build.py
