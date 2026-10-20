#!/usr/bin/env bash
# Shared by the hooks of this folder. Sourcing it moves to the repository root.

ROOT="$(git rev-parse --show-toplevel)"
cd "$ROOT" || exit 1

# Code given by the course, not linted
EXCLUDED='^common/(queue|liberror|resolver|resolvererror|socket|thread)\.(cpp|h)$'

# Blocks the tool call (exit 2): the message is shown to Claude
fail() {
    echo "[hook] $1" >&2
    exit 2
}

# True when the Bash command runs `git commit`, also inside compound commands
is_git_commit() {
    echo "$1" | grep -qE '(^|[;&|(]\s*|\s)git\s+commit(\s|$)'
}

# Linters pinned to the versions of .pre-commit-config.yaml, installed on first use in .tools/
TOOLS="$ROOT/.tools"
export PYTHONPATH="$TOOLS${PYTHONPATH:+:$PYTHONPATH}"
CLANG_FORMAT="$TOOLS/bin/clang-format"
CLANG_FORMAT_STYLE="file:$ROOT/.clang-format"

require_linters() {
    if [ ! -x "$CLANG_FORMAT" ] || [ ! -f "$TOOLS/bin/cpplint" ]; then
        pip3 install --quiet --target "$TOOLS" clang-format==14.0.6 cpplint==1.6.0 >&2 ||
            fail "Could not install clang-format 14 and cpplint 1.6.0 with pip3"
    fi
}

require_cppcheck() {
    command -v cppcheck > /dev/null ||
        fail "cppcheck is not installed. Ask the user to run: sudo apt install cppcheck"
}

# cppcheck with the arguments of .pre-commit-config.yaml. Also ignores the two information
# messages newer cppcheck versions report as errors and the course headers it analyzes
# through the includes.
run_cppcheck() {
    cppcheck --quiet --inline-suppr --enable=all --library=qt \
        --suppress=missingIncludeSystem --suppress=missingInclude --suppress=unusedFunction \
        --suppress=unmatchedSuppression --suppress=unusedStructMember \
        --suppress=normalCheckLevelMaxBranches --suppress=checkersReport \
        --suppress='*:common/queue.h' --suppress='*:common/thread.h' \
        --suppress='*:common/socket.h' --suppress='*:common/liberror.h' \
        --suppress='*:common/resolver.h' --suppress='*:common/resolvererror.h' \
        --language=c++ --std=c++20 --error-exitcode=1 "$@" >&2
}
