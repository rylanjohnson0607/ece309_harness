#!/usr/bin/env bash
set -euo pipefail

PROGRAM="./harness"

printf '%s\n' "[1/3] Building with strict warnings..."
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror harness.c -o harness

printf '%s\n' "[2/3] Checking core loop, greeting, tool execution, and shutdown..."
output=$(printf 'hello\ncalc 2 + 3 * 4\nexit\n' | "$PROGRAM")
grep -q 'Hello! This is the mock model greeting.' <<< "$output"
grep -q 'Tool result: 14' <<< "$output"
grep -q 'Goodbye.' <<< "$output"

printf '%s\n' "[3/3] Checking five-turn history management..."
history_output=$(printf 'one\ntwo\nthree\nfour\nfive\nsix\nhistory\nexit\n' | "$PROGRAM")
grep -q 'History: 5/5 turns stored' <<< "$history_output"
grep -q 'user="two"' <<< "$history_output"
! grep -q 'user="one"' <<< "$history_output"
grep -q 'user="six"' <<< "$history_output"

printf '%s\n' "All tests passed."
