#!/usr/bin/env bash
# PreToolUse hook (Bash): blocks a `git commit` unless clang-format, cpplint and cppcheck pass
# on the files being committed (same checks and exclusions as .pre-commit-config.yaml).

set -uo pipefail

COMMAND="$(jq -r '.tool_input.command // ""')"
source "$(dirname "$0")/common.sh"
is_git_commit "$COMMAND" || exit 0

# Files in the commit: the staged ones, plus the modified tracked ones with `git commit -a`
CHANGED="$(git diff --cached --name-only --diff-filter=ACMR)"
if echo "$COMMAND" | grep -qE 'git commit.*(\s-[a-zA-Z]*a|--all)'; then
    CHANGED="$CHANGED"$'\n'"$(git diff --name-only --diff-filter=ACMR)"
fi
mapfile -t FILES < <(echo "$CHANGED" | grep -E '\.(cpp|h)$' | grep -vE "$EXCLUDED" | sort -u)

[ ${#FILES[@]} -eq 0 ] && exit 0

require_linters
require_cppcheck

"$CLANG_FORMAT" --style="$CLANG_FORMAT_STYLE" --dry-run --Werror "${FILES[@]}" >&2 ||
    fail "Wrong format. Fix it with: .tools/bin/clang-format --style=file -i <files>"

python3 "$TOOLS/bin/cpplint" --quiet "${FILES[@]}" >&2 || fail "cpplint found problems"

run_cppcheck "${FILES[@]}" || fail "cppcheck found problems"

exit 0
