#!/usr/bin/env bash

# bundle.sh - Package source code for LLM context
# Usage: ./bundle.sh [target1] [target2] ... > context.txt

TARGETS=("$@")
if [ ${#TARGETS[@]} -eq 0 ]; then TARGETS=("."); fi

# Define search patterns (One per line for readability)
SEARCH_PARAMS=(
    -type f
    \(
        -name "*.c"
        -o -name "*.h"
        -o -name "*.s"
        -o -name "*.asm"
        -o -name "*.k"
        -o -name "Makefile"
        -o -name "*.sh"
    \)
)

# Define exclusions (One per line)
EXCLUDE_PARAMS=(
    -not -path '*/.*'
    -not -path '*/build*'
    -not -path '*/bin*'
    -not -path '*/obj*'
    -not -path '*/node_modules*'
)

# Internal function to format and output a single file
process_file() {
    local file="$1"
    local rel_path="${file#./}"
    local ext="${file##*.}"
    local lang="text"

    # Map extensions to Markdown language identifiers
    case "$ext" in
        c|h)     lang="c" ;;
        s|asm)   lang="asm" ;;
        k)       lang="k" ;;
        sh)      lang="bash" ;;
        Makefile) lang="makefile" ;;
    esac

    echo "--- FILE: $rel_path ---"
    echo "\`\`\`$lang"
    cat "$file"
    echo -e "\`\`\`\n"
}

# 1. Output the Project Topology
echo "--- REPOSITORY STRUCTURE ---"
for target in "${TARGETS[@]}"; do
    if [ -d "$target" ]; then
        if command -v tree >/dev/null 2>&1; then
            tree -I 'build|bin|obj|.git|node_modules' "$target"
        else
            find "$target" -maxdepth 2 "${EXCLUDE_PARAMS[@]}"
        fi
    else
        echo "[Explicit File]: $target"
    fi
done

echo -e "\n--- SOURCE FILES ---\n"

# 2. Process Files (Using an associative array to prevent duplicates)
declare -A PROCESSED

for target in "${TARGETS[@]}"; do
    if [ -f "$target" ]; then
        # Handle explicit file arguments (bypasses extension filters)
        abs_path=$(realpath "$target")
        if [[ -z "${PROCESSED[$abs_path]}" ]]; then
            process_file "$target"
            PROCESSED[$abs_path]=1
        fi
    elif [ -d "$target" ]; then
        # Handle directory scanning with the defined filters
        while read -r file; do
            abs_path=$(realpath "$file")
            if [[ -z "${PROCESSED[$abs_path]}" ]]; then
                process_file "$file"
                PROCESSED[$abs_path]=1
            fi
        done < <(find "$target" "${SEARCH_PARAMS[@]}" "${EXCLUDE_PARAMS[@]}")
    fi
done
