INPUT_DIR="results"

OUTPUT_DIR="results_2d"



mkdir -p "$OUTPUT_DIR"



#######################################

# Step 1: move all .out files to parent

#######################################

for file in "$INPUT_DIR"/*.out; do

    [[ -e "$file" ]] || continue

    mv "$file" .

done



#######################################

# Step 2: sort into NxNyh folders

#######################################

for file in *.out; do

    [[ -e "$file" ]] || continue

    filename=$(basename "$file")



    if [[ $filename =~ Nx([0-9]+)_Ny([0-9]+).*barrier_height([0-9]+(\.[0-9]+)?)_ ]]; then

        Nx="${BASH_REMATCH[1]}"

        Ny="${BASH_REMATCH[2]}"

        barrier="${BASH_REMATCH[3]}"



        # Remove trailing .0 for clean folder names

        barrier=${barrier%.*}



        folder="$OUTPUT_DIR/Nx${Nx}Ny${Ny}h${barrier}"

        mkdir -p "$folder"



        mv "$file" "$folder/"

        echo "Moved $filename -> $folder/"

    else

        echo "Skipped $filename (pattern not matched)"

    fi

done

Here is the updated Bash script.

The regex pattern in Step 2 has been modified to capture barrier_width alongside barrier_height, and the target folder path now includes the width parameter (e.g., Nx5Ny5h4w2).

Bash
#!/bin/bash

INPUT_DIR="results"
OUTPUT_DIR="results_2d_heavy_hex"

mkdir -p "$OUTPUT_DIR"

#######################################
# Step 1: move all .out files to parent
#######################################
for file in "$INPUT_DIR"/*.out; do
    [[ -e "$file" ]] || continue
    mv "$file" .
done

#######################################
# Step 2: sort into NxNyhw folders
#######################################
for file in *.out; do
    [[ -e "$file" ]] || continue
    filename=$(basename "$file")

    # Match Nx, Ny, barrier_height, AND barrier_width
    if [[ $filename =~ Nx([0-9]+)_Ny([0-9]+).*barrier_height([0-9]+(\.[0-9]+)?)_barrier_width([0-9]+) ]]; then
        Nx="${BASH_REMATCH[1]}"
        Ny="${BASH_REMATCH[2]}"
        height="${BASH_REMATCH[3]}"
        width="${BASH_REMATCH[5]}"

        # Remove trailing decimal points (e.g. 4.00 -> 4) for clean folder names
        height=${height%.*}
        width=${width%.*}

        folder="$OUTPUT_DIR/Nx${Nx}Ny${Ny}h${height}w${width}"
        mkdir -p "$folder"

        mv "$file" "$folder/"
        echo "Moved $filename -> $folder/"
    else
        echo "Skipped $filename (pattern not matched)"
    fi
done