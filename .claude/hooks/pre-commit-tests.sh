#!/usr/bin/env bash
# PreToolUse hook (Bash): blocks a `git commit` unless nfs_tests and nfs_client_tests build and
# pass. The logs are left in build/.

set -uo pipefail

COMMAND="$(jq -r '.tool_input.command // ""')"
source "$(dirname "$0")/common.sh"
is_git_commit "$COMMAND" || exit 0

if [ ! -f build/CMakeCache.txt ]; then
    mkdir -p build
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug > build/hook-cmake.log 2>&1 ||
        fail "CMake configuration failed, see build/hook-cmake.log"
fi

cmake --build build --target nfs_tests nfs_client_tests -j"$(nproc)" \
    > build/hook-build.log 2>&1 || fail "The tests do not build, see build/hook-build.log"

# SDL turns SIGTERM into a quit event: a hung test is only stopped by SIGKILL
timeout -s KILL 600 ./build/nfs_tests --gtest_brief=1 > build/hook-tests.log 2>&1 ||
    fail "nfs_tests failed, see build/hook-tests.log"

QT_QPA_PLATFORM=offscreen timeout -s KILL 600 ./build/nfs_client_tests --gtest_brief=1 \
    > build/hook-client-tests.log 2>&1 ||
    fail "nfs_client_tests failed, see build/hook-client-tests.log"

exit 0
