#!/bin/bash
set -e

# ==========================================
# 1. Configuration & Input Defaults
# ==========================================
SRC="qst_barrier_2d_copy.c my_mkl_malloc.c"
BIN="qst_barrier_2d_copy"
INPUT_FILE="${1:-input.in}"       # Default input file, or pass via command line: ./run.sh my_input.in

NP=${NP:-1}                       # Number of MPI Ranks (Default: 1)
HEIGHT_SEQ="4"                    # E.g., $(seq 1 10) or "4"
WIDTH_SEQ="2"                     # E.g., $(seq 1 2 4) or "2"

RESULTS_DIR="results"
TEMP_RUN_DIR="temp_run_data"

BEST_FIDELITY_OVERALL=0.0

# ==========================================
# 2. Environment Setup
# ==========================================
if [ -f /opt/intel/oneapi/setvars.sh ]; then
    source /opt/intel/oneapi/setvars.sh > /dev/null 2>&1 || true
fi

export MKL_NUM_THREADS=1
export OMP_NUM_THREADS=1
export MKL_DYNAMIC=FALSE

mkdir -p "$RESULTS_DIR"

# ==========================================
# 3. Compilation Step
# ==========================================
COMPILER=$(command -v mpicc || echo "gcc")
echo "==> Compiling source files using $COMPILER..."

$COMPILER -O3 -fopenmp $SRC \
    -lmkl_intel_lp64 -lmkl_sequential -lmkl_core \
    -lpthread -lm -ldl \
    -o "$BIN"

echo "==> Compilation successful. Running with NP=$NP..."

# ==========================================
# 4. Execution Loop
# ==========================================
for bh in $HEIGHT_SEQ; do
    for bw in $WIDTH_SEQ; do
        if [ "$bw" -gt 5 ]; then
            continue
        fi

        iran_value=$(( RANDOM % 10000000 + 1 ))
        
        echo -e "\n--------------------------------------------------"
        echo "Running simulation: Height = $bh | Width = $bw | iran = $iran_value | NP = $NP"
        echo "--------------------------------------------------"

        # Update height, width, and random seed in input file
        sed -i "s/^[[:space:]]*barrier_height.*/barrier_height = $bh/" "$INPUT_FILE"
        sed -i "s/^[[:space:]]*barrier_width.*/barrier_width = $bw/" "$INPUT_FILE"
        sed -i "s/^[[:space:]]*iran.*/iran = $iran_value/" "$INPUT_FILE"

        # Clear temporary directory before execution
        rm -rf "$TEMP_RUN_DIR"
        mkdir -p "$TEMP_RUN_DIR"

        # Execute simulation
        mpirun -n "$NP" ./"$BIN" "$INPUT_FILE" 2>&1 | tee "$RESULTS_DIR/log_bh${bh}_bw${bw}.out"

        # Move generated output files to temporary directory
        mv fidelity_history.txt "$TEMP_RUN_DIR/" 2>/dev/null || true
        mv *.dat *.csv qst_*.out *.out "$TEMP_RUN_DIR/" 2>/dev/null || true

        # Extract peak fidelity from the log file (3rd column of CSV)
        MAX_RUN_FIDELITY=$(awk -F',' 'NR>1 {if ($3 > max) max=$3} END {print max}' "$TEMP_RUN_DIR/fidelity_history.txt" 2>/dev/null)
        
        # Remove fidelity_history.txt from temp directory after reading
        rm -f "$TEMP_RUN_DIR/fidelity_history.txt"

        if [ -z "$MAX_RUN_FIDELITY" ]; then
            MAX_RUN_FIDELITY="0.0"
        fi
        echo "--> Maximum fidelity achieved: $MAX_RUN_FIDELITY"

        # Track and keep best run results
        IS_BETTER=$(awk -v new="$MAX_RUN_FIDELITY" -v old="$BEST_FIDELITY_OVERALL" 'BEGIN {print (new > old) ? 1 : 0}')

        
        if [ "$IS_BETTER" -eq 1 ]; then
            BEST_FIDELITY_OVERALL="$MAX_RUN_FIDELITY"

            # Clean and update target results folder with best run artifacts
            rm -rf "${RESULTS_DIR:?}"/*
            cp -r "$TEMP_RUN_DIR"/* "$RESULTS_DIR/"

            # Render graphviz visualization if dot is installed
            if command -v dot >/dev/null 2>&1 && [ -f lattice.dot ]; then
                dot -Tpng lattice.dot -o "$RESULTS_DIR/lattice.png"
                echo "==> Rendered lattice.png in $RESULTS_DIR/"
            fi

            echo "==> NEW OVERALL BEST RECORDED ($BEST_FIDELITY_OVERALL)!"
        else
            echo "--> Did not beat current best ($BEST_FIDELITY_OVERALL). Data discarded."
        fi

        rm -rf "$TEMP_RUN_DIR"
    done
done

# Clean up top-level fidelity_history.txt if generated outside temp folder
rm -f fidelity_history.txt

# Render graphviz visualization if dot is installed
if command -v dot >/dev/null 2>&1 && [ -f lattice.dot ]; then
    dot -Tpng lattice.dot -o "$RESULTS_DIR/lattice.png"
    echo "==> Rendered lattice.png in $RESULTS_DIR/"
fi

# ==========================================
# 5. Summary
# ==========================================
echo -e "\n================================================--"
echo "Execution Complete!"
echo "Overall Best Fidelity: $BEST_FIDELITY_OVERALL"
echo "Results saved in: $RESULTS_DIR/"
echo "================================================--"
