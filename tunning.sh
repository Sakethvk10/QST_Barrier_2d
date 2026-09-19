#!/bin/bash
set -e

# Load OneAPI environment if present
if [ -f /opt/intel/oneapi/setvars.sh ]; then
    source /opt/intel/oneapi/setvars.sh > /dev/null 2>&1 || true
fi

SRC="qst_barrier_2d.c my_mkl_malloc.c"
BIN="qst_barrier_2d"
INPUT_FILE="input.in"
RESULTS_DIR="results_gridsearch"

# Set parallel ranks
NP=16

# Define parameter ranges to explore
ALPHA_LIST="0.1 0.05 0.01"
BETA1_LIST=" 0.95 0.99 0.995"
BETA2_LIST="0.99 0.999 0.9999"
EPOCHS_LIST="100000"

mkdir -p "$RESULTS_DIR"

# Compile executable
COMPILER=$(command -v mpicc || echo "gcc")
echo "==> Compiling source files using $COMPILER..."
$COMPILER -O3 -march=native -ffast-math $SRC \
  -lmkl_intel_lp64 -lmkl_sequential -lmkl_core \
  -lpthread -lm -ldl \
  -o "$BIN"

echo "==> Compilation successful. Starting grid search with NP=$NP..."

# Initialize global tracking variables
BEST_FIDELITY_OVERALL=0.0
BEST_RUN_INFO=""
BEST_IRAN=""
iran_value=$(( RANDOM % 10000000 + 1 ))

# Temporary output folder for each run
TEMP_RUN_DIR="temp_run_data"

for a in $ALPHA_LIST; do
    for b1 in $BETA1_LIST; do
        for b2 in $BETA2_LIST; do
            for ep in $EPOCHS_LIST; do

                echo -e "\n--------------------------------------------------"
                echo "Running: alpha=$a | beta1=$b1 | beta2=$b2 | epochs=$ep | iran=$iran_value | NP=$NP"
                echo "--------------------------------------------------"

                # Update Adam hyperparameters in input.in
                sed -i "s/^[[:space:]]*alpha.*/alpha = $a/" "$INPUT_FILE"
                sed -i "s/^[[:space:]]*beta1.*/beta1 = $b1/" "$INPUT_FILE"
                sed -i "s/^[[:space:]]*beta2.*/beta2 = $b2/" "$INPUT_FILE"
                sed -i "s/^[[:space:]]*max_epochs.*/max_epochs = $ep/" "$INPUT_FILE"

                # Clear temporary directory before execution
                rm -rf "$TEMP_RUN_DIR"
                mkdir -p "$TEMP_RUN_DIR"

                # Run simulation
                mpirun -n "$NP" ./"$BIN" "$INPUT_FILE" >  /dev/null

                # Move generated logs to temporary execution directory
                mv fidelity_history.txt "$TEMP_RUN_DIR/" 2>/dev/null || true
                mv *.dat *.csv qst_*.out "$TEMP_RUN_DIR/" 2>/dev/null || true

                # Extract peak fidelity from the log file (3rd column of CSV)
                MAX_RUN_FIDELITY=$(awk -F',' 'NR>1 {if ($3 > max) max=$3} END {print max}' "$TEMP_RUN_DIR/fidelity_history.txt" 2>/dev/null)

                if [ -z "$MAX_RUN_FIDELITY" ]; then
                    MAX_RUN_FIDELITY="0.0"
                fi
                echo "--> Maximum fidelity found across rank files: $MAX_RUN_FIDELITY"

                # Check if current run beat global best
                IS_BETTER=$(awk -v new="$MAX_RUN_FIDELITY" -v old="$BEST_FIDELITY_OVERALL" 'BEGIN {print (new > old) ? 1 : 0}')

                if [ "$IS_BETTER" -eq 1 ]; then
                    BEST_FIDELITY_OVERALL="$MAX_RUN_FIDELITY"
                    BEST_RUN_INFO="alpha=$a | beta1=$b1 | beta2=$b2 | max_epochs=$ep"
                    BEST_IRAN="$iran_value"

                    # Save rank data files and best input.in configuration
                    rm -rf "${RESULTS_DIR:?}"/*
                    cp -r "$TEMP_RUN_DIR"/* "$RESULTS_DIR/"
                    cp "$INPUT_FILE" "$RESULTS_DIR/best_input.in"

                    # Write a standalone summary file for exact reproduction
                    cat <<EOF > "$RESULTS_DIR/best_run_summary.txt"
==================================================
              BEST RUN REPRODUCTION DATA          
==================================================
Best Fidelity Achieved: $BEST_FIDELITY_OVERALL
Random Seed (iran):     $BEST_IRAN
Hyperparameters:        $BEST_RUN_INFO
MPI Ranks (NP):         $NP
Saved Config File:      best_input.in
==================================================
EOF

                    echo "==> NEW OVERALL BEST RECORDED ($BEST_FIDELITY_OVERALL)!"
                    echo "    Saved reproduction parameters to '$RESULTS_DIR/best_run_summary.txt'."
                else
                    echo "--> Did not beat current best ($BEST_FIDELITY_OVERALL). Data discarded."
                fi

                rm -rf "$TEMP_RUN_DIR"

            done
        done
    done
done

echo -e "\n=================================================="
echo "Grid Search Complete!"
echo "Overall Best Fidelity: $BEST_FIDELITY_OVERALL"
echo "Best Hyperparameters:  $BEST_RUN_INFO"
echo "Best Random Seed:      iran = $BEST_IRAN"
echo "Full Results saved in: $RESULTS_DIR/"
echo "=================================================="
