#!/bin/bash
# Run the provided tests.
#   ./runtests.sh       run all tests
#   ./runtests.sh 1     run only the Stage 1 (linked list) tests
#   ./runtests.sh 2     run only the Stage 2 (basic shell) tests
#   ./runtests.sh 3     run only the Stage 3 (background jobs) tests
#   ./runtests.sh design  run only the program design checks

STAGE=${1:-all}
PASSED=0
FAILED=0

pass() { echo "PASS: $1"; PASSED=$((PASSED + 1)); }
fail() { echo "FAIL: $1"; FAILED=$((FAILED + 1)); }

# check - run a command; PASS if it succeeds, otherwise FAIL and show its output
check() {
    local name=$1
    shift
    local out
    if out=$("$@" 2>&1); then
        pass "$name"
    else
        fail "$name"
        [ -n "$out" ] && echo "$out"
    fi
}

# output_test N - feed cmdN.txt to the shell and compare with cmdN_output.txt
output_test() {
    local out
    out=$(timeout 15 ./shell < cmd$1.txt 2>&1 | diff -w - cmd$1_output.txt)
    if [ -z "$out" ]; then
        pass "cmd$1.txt"
    else
        fail "cmd$1.txt (lines marked < are your output, > are expected)"
        echo "$out"
    fi
}

memcheck() {
    timeout 30 valgrind -q --error-exitcode=1 --leak-check=full "$@" > /dev/null
}

# zombie_test - a finished foreground child must be reaped with waitpid
zombie_test() {
    local fifo out pid zombies
    fifo=$(mktemp -u /tmp/shelltest_XXXXXX)
    out=$(mktemp /tmp/shellout_XXXXXX)
    mkfifo "$fifo"
    ./shell < "$fifo" > "$out" 2>&1 &
    pid=$!
    exec 3>"$fifo"
    echo "sleep 1" >&3
    echo "echo zombiecheck" >&3
    sleep 1.5
    zombies=$(ps --no-headers -o ppid,stat | awk -v p="$pid" '$1 == p && $2 ~ /^Z/' | wc -l)
    echo "exit" >&3
    exec 3>&-
    wait "$pid"
    if ! grep -q zombiecheck "$out"; then
        echo "the shell did not run 'echo zombiecheck'"; rm -f "$fifo" "$out"; return 1
    fi
    rm -f "$fifo" "$out"
    [ "$zombies" -eq 0 ] || { echo "found $zombies zombie child process(es)"; return 1; }
}

# kill_test - start a background sleep, kill it with the kill built-in,
# then check that status shows none and the shell can exit
kill_test() {
    local fifo out pid sleeppid rc=0
    fifo=$(mktemp -u /tmp/shelltest_XXXXXX)
    out=$(mktemp /tmp/shellout_XXXXXX)
    mkfifo "$fifo"
    ./shell < "$fifo" > "$out" 2>&1 &
    pid=$!
    exec 3>"$fifo"
    echo "sleep 30 &" >&3
    echo "status" >&3
    sleep 0.5
    sleeppid=$(grep -E '[0-9]+.*sleep' "$out" | grep -oE '[0-9]+' | head -1)
    if [ -z "$sleeppid" ]; then
        echo "status did not show the background sleep's pid"; rc=1
    else
        echo "kill $sleeppid" >&3
        sleep 0.5
        echo "" >&3     # blank line: one more loop iteration so the shell sweeps
        echo "status" >&3
        echo "exit" >&3
        sleep 0.5
        if ! grep -q "Killed process $sleeppid" "$out"; then
            echo "no 'Killed process $sleeppid' message"; rc=1
        elif ! tail -2 "$out" | grep -q "Processes currently active: none"; then
            echo "status after kill should show none"; rc=1
        elif kill -0 "$pid" 2>/dev/null; then
            echo "shell did not exit after the job was killed"; rc=1
        fi
    fi
    exec 3>&-
    kill -9 "$pid" 2>/dev/null
    pkill -x -P "$pid" sleep 2>/dev/null
    [ $rc -ne 0 ] && { echo "--- shell output:"; cat "$out"; }
    rm -f "$fifo" "$out"
    return $rc
}

# globals_test - no file-scope variables in main.c or list.c (nm lists
# variables as data/bss symbols; static locals appear as name.N and are allowed)
globals_test() {
    nm main.o list.o | awk 'NF == 3 && $2 ~ /^[BbDdCGgSs]$/ && $3 !~ /\./ {
        print "global variable: " $3; found = 1 } END { exit found }'
}

# comments_test - every function except main has a comment right before it,
# and no TODO comments remain
comments_test() {
    awk '
    FNR == 1 { prev = "" }
    /^[A-Za-z_][A-Za-z0-9_ \t*]*\(/ && !/;[ \t]*$/ {
        name = $0; sub(/[ \t]*\(.*/, "", name)
        n = split(name, w, /[ \t*]+/); name = w[n]
        if (name != "main" && prev !~ /\*\/[ \t]*$/ && prev !~ /^[ \t]*\/\//) {
            print FILENAME ":" FNR ": function " name " has no comment before it"
            bad = 1
        }
    }
    /[^ \t]/ { prev = $0 }
    END { exit bad }' main.c list.c || return 1
    if grep -n TODO main.c list.c; then
        echo "remove the TODO comments above (replace them with your own comments)"
        return 1
    fi
}

# build first; any compiler warning counts as a failure
BUILD=$(make 2>&1)
if [ $? -ne 0 ]; then
    echo "$BUILD"
    echo "FAIL: make (fix the compile errors above first)"
    exit 1
elif echo "$BUILD" | grep -q warning; then
    echo "$BUILD"
    fail "make (compiler warnings above)"
else
    pass "make (no warnings)"
fi

if [ "$STAGE" = all ] || [ "$STAGE" = 1 ]; then
    check "./listtest" ./listtest
    check "valgrind ./listtest" memcheck ./listtest
fi
if [ "$STAGE" = all ] || [ "$STAGE" = 2 ]; then
    for N in 0 1 2 3; do output_test $N; done
    check "foreground child is reaped (no zombies)" zombie_test
    check "valgrind ./shell < cmd2.txt" memcheck ./shell < cmd2.txt
fi
if [ "$STAGE" = all ] || [ "$STAGE" = 3 ]; then
    for N in 4 5 6; do output_test $N; done
    check "kill built-in" kill_test
    check "valgrind ./shell < cmd7.txt" memcheck ./shell < cmd7.txt
fi
if [ "$STAGE" = all ] || [ "$STAGE" = design ]; then
    check "no global variables" globals_test
    check "function comments, no TODOs left" comments_test
fi

echo "$PASSED passed, $FAILED failed"
[ $FAILED -eq 0 ]
