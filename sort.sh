#!/bin/bash

INPUT_DIR="results"
OUTPUT_DIR="results_2d"

mkdir -p "$OUTPUT_DIR"

# Clean up top-level fidelity_history.txt if present
if [[ -f "$INPUT_DIR/fidelity_history.txt" ]]; then
    rm "$INPUT_DIR/fidelity_history.txt"
    echo "Removed fidelity_history.txt from $INPUT_DIR"
fi

# Loop through all .out files directly inside results/
for file in "$INPUT_DIR"/*.out; do
    [[ -f "$file" ]] || continue
    filename=$(basename "$file")

    # Extract Nx, Ny, barrier_height, and barrier_width directly from the filename
    if [[ $filename =~ Nx([0-9]+)_Ny([0-9]+).*barrier_height([0-9]+(\.[0-9]+)?).*barrier_width([0-9]+) ]]; then
        Nx="${BASH_REMATCH[1]}"
        Ny="${BASH_REMATCH[2]}"
        
        # Convert floating point height (e.g., 4.00) to integer format (e.g., 4)
        height_raw="${BASH_REMATCH[3]}"
        height=$(printf "%.0f" "$height_raw")
        
        width="${BASH_REMATCH[5]}"

        # Target directory formatted as: Nx<Nx>Ny<Ny>h<height>w<width>
        target_dir="$OUTPUT_DIR/Nx${Nx}Ny${Ny}h${height}w${width}"
        mkdir -p "$target_dir"

        mv "$file" "$target_dir/"
        echo "Moved $filename -> $target_dir/"
    fi
done
