#!/usr/bin/env bash
#
# Binary seam of the experiment: runs the three sync modes many times each,
# aggregates the result lines and emits an asymmetric verdict.
#
# Responsibilities:
# - Run the binary RUNS_PER_MODE times per sync mode.
# - Count the divergent runs and aggregate the elapsed times of each mode.
# - Fail when FULL ever diverges, or when NONE / NO_MUTEX never diverge.
#
# Usage: tests/battery.sh [binary]   (default: bin/prodcons)

set -u

# Runs per sync mode. Raise it if a race turns out to be rare on the platform,
# lower it if the battery gets slow.
RUNS_PER_MODE=20

BINARY="${1:-bin/prodcons}"

failed=0

# Reads "name=value" out of one result line of the binary.
field() {
    local line="$1" name="$2" token
    for token in $line; do
        case "$token" in
            "$name"=*)
                printf '%s' "${token#*=}"
                return 0
                ;;
        esac
    done
    printf 'battery: no field %s in result line: %s\n' "$name" "$line" >&2
    exit 1
}

# Runs one sync mode RUNS_PER_MODE times and prints its verdict line.
#
# mode:     argv value passed to the binary (none, no-mutex, full).
# polarity: "invariant" (any divergence fails) or "race" (no divergence fails).
run_mode() {
    local mode="$1" polarity="$2"
    local divergent=0 times="" line difference lost_items verdict note run

    for ((run = 1; run <= RUNS_PER_MODE; run++)); do
        if ! line="$("$BINARY" "$mode")"; then
            printf 'battery: %s %s failed to run\n' "$BINARY" "$mode" >&2
            exit 1
        fi
        difference="$(field "$line" difference)"
        lost_items="$(field "$line" lost_items)"
        if [ "$difference" != "0" ] || [ "$lost_items" != "0" ]; then
            divergent=$((divergent + 1))
        fi
        times+="$(field "$line" time_ms)"$'\n'
    done

    note=""
    if [ "$polarity" = "invariant" ]; then
        if [ "$divergent" -eq 0 ]; then
            verdict="PASS"
        else
            verdict="FAIL"
            note="  <- exclusao mutua deveria tornar a divergencia impossivel"
            failed=1
        fi
    else
        if [ "$divergent" -gt 0 ]; then
            verdict="PASS"
        else
            verdict="FAIL"
            note="  <- janela de race estreita demais nesta plataforma (achado do relatorio)"
            failed=1
        fi
    fi

    printf 'mode=%-9s runs=%-3d divergent=%-3d %s  %s%s\n' \
        "$mode" "$RUNS_PER_MODE" "$divergent" "$verdict" \
        "$(printf '%s' "$times" | awk 'NF {
                 n++; sum += $1
                 if (min == "" || $1 < min) min = $1
                 if ($1 > max) max = $1
             }
             END { printf "time_ms min=%.3f mean=%.3f max=%.3f", min, sum / n, max }')" \
        "$note"
}

if [ ! -x "$BINARY" ]; then
    printf 'battery: binary not found or not executable: %s\n' "$BINARY" >&2
    exit 1
fi

printf 'bateria sobre %s — %d execucoes por modo\n\n' "$BINARY" "$RUNS_PER_MODE"

run_mode full "invariant"
run_mode no-mutex "race"
run_mode none "race"

printf '\n'
if [ "$failed" -eq 0 ]; then
    printf 'bateria: PASS\n'
else
    printf 'bateria: FAIL\n'
fi
exit "$failed"
