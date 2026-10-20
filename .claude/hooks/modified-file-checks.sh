#!/usr/bin/env bash
# PostToolUse hook (Write|Edit) for the C++ file just modified:
#   1. formats it with clang-format and the .clang-format of the repository root
#   2. runs cppcheck on it; problems are reported to Claude (exit 2) to fix them
# Both run in this order in one script so cppcheck never reads a file being rewritten.

set -uo pipefail

FILE="$(jq -r '.tool_response.filePath // .tool_input.file_path // ""')"
source "$(dirname "$0")/common.sh"

[[ "$FILE" =~ \.(cpp|h)$ ]] || exit 0
[ -f "$FILE" ] || exit 0

RELATIVE="$(realpath --relative-to="$ROOT" "$FILE")"
[[ "$RELATIVE" == ../* ]] && exit 0  # outside the repository
echo "$RELATIVE" | grep -qE "$EXCLUDED" && exit 0

require_linters
"$CLANG_FORMAT" --style="$CLANG_FORMAT_STYLE" -i "$RELATIVE" ||
    fail "clang-format could not format $RELATIVE"

require_cppcheck
run_cppcheck "$RELATIVE" || fail "cppcheck found problems in $RELATIVE"

exit 0
