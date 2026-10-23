#!/bin/bash
set -eo pipefail

SRC="qst_barrier_2d.c my_mkl_malloc.c"
BIN="qst_barrier_2d"
INPUT_FILE="${1:-input.in}"       # Default input file, or pass via command line

HEIGHT_SEQ="1 2 3 4 5 6 7 8 9 10" 
WIDTH_SEQ="1 2 3 4 5 6 7 8 9 10" 

export MKL_NUM_THREADS=1
export OMP_NUM_THREADS=1
export OPENBLAS_NUM_THREADS=1

RESULTS_DIR="${RESULTS_DIR:-results}"
TEMP_RUN_DIR="${TEMP_RUN_DIR:-temp_run_data}"

TOTAL_CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || nproc 2>/dev/null || echo 20)

echo "=================================================="
echo "Resource Validation Check"
echo "  - RUNS (Concurrent tasks): $RUNS"
echo "  - SPLITS (MPI Ranks/task): $SPLITS"
echo "  - Total Requested Ranks:   $NP"
echo "  - Hardware Cores Detected: $TOTAL_CORES"
echo "=================================================="

if [ "$NP" -gt "$TOTAL_CORES" ]; then
    echo "ERROR: Requested processes ($NP) exceed physical cores ($TOTAL_CORES)." >&2
    exit 1
fi

if [ -f /opt/intel/oneapi/setvars.sh ]; then
    source /opt/intel/oneapi/setvars.sh > /dev/null 2>&1 || true
fi

mkdir -p "$RESULTS_DIR"
mkdir -p "$TEMP_RUN_DIR"

COMPILER=$(command -v mpicc || echo "gcc")
echo "==> Compiling source files using $COMPILER..."

$COMPILER -O3 -march=native -ffast-math $SRC \
  -lmkl_intel_lp64 -lmkl_sequential -lmkl_core \
  -lpthread -lm -ldl \
  -o "$BIN"

BASE_DIR="$PWD"
ABS_BIN="${BASE_DIR}/${BIN}"

ny=$(grep -iE "^[[:space:]]*Ny" "$INPUT_FILE" | grep -oE "[0-9]+" | head -n 1)
if [ -z "$ny" ]; then
    echo "Warning: Ny not found in $INPUT_FILE."
    exit 1
fi

let "ny--"

iran_value=$(grep -iE "^[[:space:]]*iran" "$INPUT_FILE" | grep -oE "[0-9]+" | head -n 1
if [ -z "$iran_value" ]; then
    echo "Warning: iran not found in $INPUT_FILE."
    exit 1
fi

for bh in $HEIGHT_SEQ; do
    for bw in $WIDTH_SEQ; do
        if [ "$bw" -gt "$ny" ]; then
            break
        fi

        echo -e "\n--------------------------------------------------"
        echo "Running simulation: Height = $bh | Width = $bw | Base iran = $iran_value | RUNS = $RUNS | SPLITS = $SPLITS | Total NP = $NP"
        echo "--------------------------------------------------"

        # Launch RUNS concurrent background tasks, each with unique seed offset
        for ((r=0; r<RUNS; r++)); do
            run_iran=$(( iran_value + $RANDOM%10000 ))
            RUN_DIR="${TEMP_RUN_DIR}/run_h${bh}_w${bw}_r${r}"
            mkdir -p "$RUN_DIR"

            RUN_INPUT="${RUN_DIR}/input.in"
            cp "$INPUT_FILE" "$RUN_INPUT"

            sed -i "s/^[[:space:]]*iran.*/iran = $run_iran/" "$RUN_INPUT"
            sed -i "s/^[[:space:]]*barrier_height.*/barrier_height = $bh/" "$RUN_INPUT"
            sed -i "s/^[[:space:]]*barrier_width.*/barrier_width = $bw/" "$RUN_INPUT"

            ABS_RUN_INPUT="${BASE_DIR}/${RUN_INPUT}"
            LOG_FILE="${BASE_DIR}/${RESULTS_DIR}/run_h${bh}_w${bw}_r${r}.log"

            (
                cd "$RUN_DIR"
                mpirun -n "$SPLITS" "$ABS_BIN" "$ABS_RUN_INPUT" > "$LOG_FILE" 2>&1
            ) &
        done

        # Wait for all parallel background runs to complete
        wait

        # Evaluate maximum fidelity across all parallel instances
        MAX_RUN_FIDELITY="0.0"
        BEST_R=-1

        for ((r=0; r<RUNS; r++)); do
            RUN_DIR="${TEMP_RUN_DIR}/run_h${bh}_w${bw}_r${r}"
            FID_FILE="${RUN_DIR}/fidelity_history.txt"

            if [ -f "$FID_FILE" ]; then
                RUN_MAX=$(awk -F',' 'NR>1 {if ($3 > max) max=$3} END {print max}' "$FID_FILE" 2>/dev/null)
                if (( $(echo "${RUN_MAX:-0.0} > $MAX_RUN_FIDELITY" | bc -l 2>/dev/null || echo 0) )); then
                    MAX_RUN_FIDELITY="$RUN_MAX"
                    BEST_R=$r
                fi
                mv "$FID_FILE" "${RESULTS_DIR}/fidelity_h${bh}_w${bw}_r${r}.txt" 2>/dev/null || true
            fi
        done

        echo "--> Maximum fidelity achieved: $MAX_RUN_FIDELITY"

        # Extract data artifacts from the highest-performing task directory
        if [ "$BEST_R" -ge 0 ]; then
            BEST_RUN_DIR="${TEMP_RUN_DIR}/run_h${bh}_w${bw}_r${BEST_R}"
            (
                cd "$BEST_RUN_DIR" 2>/dev/null || exit
                shopt -s nullglob
                mv gmon.out* *.dat *.csv qst_*.out *.out "${BASE_DIR}/${RESULTS_DIR}/" 2>/dev/null || true
                shopt -u nullglob
            )
        fi

        # Cleanup temporary run directories for current iteration
        rm -rf ${TEMP_RUN_DIR}/run_h${bh}_w${bw}_r*
    done
done
