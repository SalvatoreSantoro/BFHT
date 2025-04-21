#!/bin/bash

# Default number of iterations
N=1

# Parse arguments
while getopts "n:" opt; do
  case $opt in
    n) N=$OPTARG ;;
    *) echo "Usage: $0 [-n repetitions]"; exit 1 ;;
  esac
done

# Define possible values
HT_SIZE_TYPES=("PRIMES" "TWO_POWERS")
HT_INITIAL_SIZES=(256 512 1024)
HT_SIZE_MAX_GROWINGS=(20)
PROBING_METHODS=("LINEAR" "QUADRATIC")
ALPHAS=(125 375 500 625 750 875)

# Loop through all combinations
for HT_SIZE_TYPE in "${HT_SIZE_TYPES[@]}"; do
  for HT_INITIAL_SIZE in "${HT_INITIAL_SIZES[@]}"; do
    for HT_SIZE_MAX_GROWING in "${HT_SIZE_MAX_GROWINGS[@]}"; do
      for PROBING in "${PROBING_METHODS[@]}"; do
        for ALPHA in "${ALPHAS[@]}"; do
          echo ">>> Config: HT_SIZE_TYPE=$HT_SIZE_TYPE, HT_INITIAL_SIZE=$HT_INITIAL_SIZE, HT_SIZE_MAX_GROWINGS=$HT_SIZE_MAX_GROWING, PROBING=$PROBING, ALPHA=$ALPHA"
          for ((i = 1; i <= N; i++)); do
            echo "  Run $i/$N"
            make clean > /dev/null 2>&1
            make profiling ALPHA=$ALPHA PROBING=$PROBING HT_INITIAL_SIZE=$HT_INITIAL_SIZE HT_SIZE_TYPE=$HT_SIZE_TYPE HT_SIZE_MAX_GROWINGS=$HT_SIZE_MAX_GROWING > /dev/null 2>&1
            if [[ $? -ne 0 ]]; then
              echo "[ERROR] Make failed on iteration $i"
            fi
          done
        done
      done
    done
  done
done
