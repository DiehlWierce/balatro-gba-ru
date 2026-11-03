#!/bin/bash

set -eu

run_test() {
    name="$1"

    echo "==============================================================================="
    echo "Running test for: $1"
    echo "==============================================================================="

    cd "$name" 2>&1 > /dev/null
    make clean > /dev/null
    make > /dev/null
    ./build/"$name"_test
    cd - 2>&1 > /dev/null
}

run_test bitset
run_test list
run_test pool
run_test util
run_test hand_levels
run_test card_rules
run_test tarot_rules
run_test joker_economy
run_test joker_cards
run_test joker_batch4
run_test joker_batch5
run_test joker_batch6
run_test card_modifiers
run_test voucher
run_test ante_table
run_test edition_rules
run_test spectral_rules
run_test deck_rules
