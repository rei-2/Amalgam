#!/usr/bin/bash

# Needs to be started in tmux.

script_dir() {
    dirname "$(readlink -f "$0")"
}

fuzzer_dir() {
    printf '%s/fuzzers\n' "$(script_dir)"
}

fuzzer_list() {
    find "$(fuzzer_dir)" -maxdepth 1 -type f \( -name '*.cpp' -or -name '*.c' \) -printf '%P\n' \
      | while read -r fuzzer; do
        fuzzer="${fuzzer#fuzz_}"
        printf '%s\n' "${fuzzer%.c*}"
    done
}

usage() {
    printf '%s: HARNESS FUZZER\n\n' "$(basename "$0")"
    printf '    HARNESS ∈ {\n'
    # We want word-splitting here so that each fuzzer ends up as a separate
    # argument.
    # shellcheck disable=SC2046
    printf '%30s,\n' $(fuzzer_list | tr '\n' ' ')
    printf '    }\n'
    printf '    FUZZER ∈ {afl, afl++}\n'
}

if [[ $# -ne 2 ]]; then
    usage
    exit 1
fi

case "$2" in
    afl++)
        export AFL_PATH=/home/dkasak/code/projects/afl/afl++
        export AFL_AUTORESUME=1
        AFL_ARGS_FUZZER0="-D"
        AFL_ARGS_FUZZER1="-L 0"
        AFL_ARGS_FUZZER2="-p rare"
        AFL_ARGS_FUZZER3="-p fast"
        AFL_ARGS_FUZZER4="-p exploit"
        AFL_ARGS_FUZZER5="-p explore"
        ;;
    afl)
        export AFL_PATH=/usr/bin
        ;;
    *)
        printf 'Unknown fuzzer: %s\n' "$2"
        exit 1
        ;;
esac

export AFL=$AFL_PATH/afl-fuzz
export AFL_TMPDIR=/tmp

case "$1" in
    group_decrypt)
        FUZZER_ARG1="fuzzing/$1/pickled-inbound-group-session.txt"
        ;;
    decrypt)
        FUZZER_ARG1="fuzzing/$1/pickled-session.txt"
        FUZZER_ARG2="1"
        ;;
    decode_message)
        ;;
    unpickle_session)
        ;;
    unpickle_account)
        ;;
    unpickle_account_test)
        ;;
    unpickle_megolm_outbound)
        ;;
    *)
        printf 'Unknown harness: %s\n' "$1"
        exit 1
        ;;
esac

cd "$(script_dir)" || exit 1

# Fuzzer args are deliberately not quoted below so that word-splitting happens.
# This is used so that they expand into nothing in cases where they are missing
# or to expand into multiple arguments from a string definition.

# shellcheck disable=SC2086
tmux new-window -d -n "M" -- \
    "$AFL" -i "corpora/$1/in" -o "corpora/$1/out" -M i0 "$AFL_ARGS_FUZZER0" \
            -- "../build/fuzzers/fuzz_$1" $FUZZER_ARG1 $FUZZER_ARG2

# shellcheck disable=SC2086
tmux new-window -d -n "S1" -- \
    "$AFL" -i "corpora/$1/in" -o "corpora/$1/out" -S i1 "$AFL_ARGS_FUZZER1" \
            -- "../build/fuzzers/fuzz_$1" $FUZZER_ARG1 $FUZZER_ARG2

# shellcheck disable=SC2086
tmux new-window -d -n "S2" -- \
    "$AFL" -i "corpora/$1/in" -o "corpora/$1/out" -S i2 $AFL_ARGS_FUZZER2 \
            -- "../build/fuzzers/fuzz_$1" $FUZZER_ARG1 $FUZZER_ARG2

# shellcheck disable=SC2086
tmux new-window -d -n "S3" -- \
    "$AFL" -i "corpora/$1/in" -o "corpora/$1/out" -S i3 $AFL_ARGS_FUZZER3 \
            -- "../build/fuzzers/fuzz_$1" $FUZZER_ARG1 $FUZZER_ARG2

# shellcheck disable=SC2086
tmux new-window -d -n "S4" -- \
    "$AFL" -i "corpora/$1/in" -o "corpora/$1/out" -S i4 $AFL_ARGS_FUZZER4 \
            -- "../build/fuzzers/fuzz_$1_asan" $FUZZER_ARG1 $FUZZER_ARG2

# shellcheck disable=SC2086
tmux new-window -d -n "S5" -- \
    "$AFL" -i "corpora/$1/in" -o "corpora/$1/out" -S i5 $AFL_ARGS_FUZZER5 \
            -- "../build/fuzzers/fuzz_$1" $FUZZER_ARG1 $FUZZER_ARG2
