#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
INPUT=$1

"$SCRIPT_DIR/../tests/lexer" "$INPUT"