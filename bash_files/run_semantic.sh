#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
INPUT=$1

run_phase () {
    OUTPUT=$("$1" "$INPUT" 2>&1)
    echo "$OUTPUT"

    if echo "$OUTPUT" | grep -Ei "Invalid character|syntax error|Status: FAILED" > /dev/null; then
        exit 1
    fi
}

run_phase "$SCRIPT_DIR/../tests/lexer"
run_phase "$SCRIPT_DIR/../tests/parser"
run_phase "$SCRIPT_DIR/../tests/semantic"