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

echo "==> Compilation successful."

# ==========================================
# 4. Execution Loop
# ==========================================
for bh in $HEIGHT_SEQ; do
    for bw in $WIDTH_SEQ; do
        if [ "$bw" -gt 5 ]; then
            continue
        fi

        echo -e "\n--------------------------------------------------"
        echo "Running simulation: Height = $bh | Width = $bw | Ranks = $NP"
        echo "--------------------------------------------------"

        # Update height, width, and random seed in input file
        iran_value=$(( RANDOM % 10000000 + 1 ))
        sed -i "s/^[[:space:]]*barrier_height.*/barrier_height = $bh/" "$INPUT_FILE"
        sed -i "s/^[[:space:]]*barrier_width.*/barrier_width = $bw/" "$INPUT_FILE"
        sed -i "s/^[[:space:]]*iran.*/iran = $iran_value/" "$INPUT_FILE"

        # Execute simulation
        mpirun -n "$NP" ./"$BIN" "$INPUT_FILE" 2>&1 | tee "$RESULTS_DIR/log_bh${bh}_bw${bw}.out"

        # Move generated output files to results folder
        mv *.out "$RESULTS_DIR/" 2>/dev/null || true
    done
done

# ==========================================
# 5. Optional Graphviz Generation
# ==========================================
if command -v dot >/dev/null 2>&1 && [ -f lattice.dot ]; then
    dot -Tpng lattice.dot -o "$RESULTS_DIR/lattice.png"
    echo "==> Rendered lattice.png in $RESULTS_DIR/"
fi

echo -e "\n==> All simulations complete! Results saved to '$RESULTS_DIR/'."