# 2D Quantum State Transfer (QST) Optimizer

This project aims to find an optimized set of couplings such that quantum state can pass through a barrier with a high fidelity rate. 
The code supports two lattice geometries:
 - Square grid lattice
 - Heavy-hex lattice
Although both geometries are supported, the heavy-hex lattice generally performs less effectively due to its lower connectivity.
The optimizer supports the transfer of both single-qubit quantum states and entangled quantum states, including Bell states.

## Input Parameters
The program is configured through an input file. The following parameters can be adjusted:

| Parameter | Description |
| :--- | :--- |
| **Nx, Ny** | Height and width of the lattice, respectively. |
| **Jmin, Jmax** | Minimum and maximum allowed coupling strengths, representing hardware constraints. |
| **barrier_height** | Energy height of the potential barrier. |
| **barrier_width** | Width of the barrier, measured in qubits and centered along the antidiagonal of the lattice. |
| **Jpen** | Penalty weight applied when coupling strengths fall outside the allowed range `[Jmin, Jmax]`. |
| **P** | Penalty exponent controlling how the penalty increases as coupling strengths exceed the allowed range. |
| **inversion** | Boolean flag that exploits the natural symmetry of the problem to optimize only half of the coupling strengths. |
| **time_evol** | Boolean flag that enables visualization of the quantum state's time evolution. This is useful for plotting results. |
| **n_t_slices** | Number of time intervals used to represent the time evolution. |
| **opt_time** | Parameter controlling the optimization time. `opt_time = 4` is recommended as it scales with lattice size. |
| **bell** | Boolean flag that determines whether the optimizer targets the transfer of a Bell state. |
| **heavy_hex** | Boolean flag that selects the heavy-hex lattice instead of the square grid lattice. |

## Running the Optimization
### 1. Configure the input file
Edit the input file to specify the lattice geometry, coupling constraints, barrier properties, and optimization settings.
### 2. (Optional) Tune the parameters for the optimization algorithm
To tune the parameters of the machine-learning optimization algorithm, run:
```
./tuning.sh
```
The tuning process produces results in the results_gridsearch/ directory, including:
* best_input.in — The optimized input configuration.
* best_run_summary.txt — A summary of the optimization results.
* fidelity_history.txt — The history of fidelity values throughout the optimization process.
move best_input.in out of the results_gridsearch/ directory and use it as your input file. You can ignore the other two text files.
### 3. Run the optimized code
Execute the program using:
```
./running.sh
```
You can also specify the input file and customize the number of runs and MPI instances per run using environment variables.
For example:
```
RUNS=3 SPLITS=6 ./running.sh best_input.in
```
The parameters are:
 - RUNS — Number of independent runs.
 - SPLITS — Number of MPI instances used per run.
 - Input file — Optional input configuration passed as a command-line argument. If you performed the tuning step, you can pass best_input.in here or any other file name.

By default, RUNS=2 and SPLITS=2.

Also ensure that RUNS × SPLITS does not exceed the number of CPU cores available on your machine.

By default, running.sh executes the optimized code over a range of barrier configurations:
 - Barrier height: From 0 to 10.
 - Barrier width: From 0 to Ny.

To modify these ranges, edit the HEIGHT_SEQ and WIDTH_SEQ variables in running.sh.

### 4. Output Management

Running the program generates output files, including intermediate results and debug files. These can accumulate quickly and may be difficult to navigate.

Two utility scripts are provided to help manage the output:

Clean up unnecessary files
```
./clean.sh
```
This removes the debug and other unnecessary files targeted by the cleanup script.

Organize the results
```
./sort.sh
```
This organizes the generated files into a more structured directory hierarchy, making it easier to locate and compare results.
