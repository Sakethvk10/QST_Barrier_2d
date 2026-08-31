#ifndef QST_BARRIER_2D_H_
#define QST_BARRIER_2D_H_

#include <mkl.h>
#include "my_mkl_malloc.h"
#define MKL_H_


#ifndef INTTYPES_H_
#include <inttypes.h>
#define INTTYPES_H_
#endif  // INTTYPES_H_

/* Typedefs */
typedef uint32_t UI;
typedef int64_t  LI;
typedef uint64_t ULI;
typedef int8_t   SI;
typedef uint8_t  USI;

#ifndef TIME_H_
#include <sys/time.h>
#define TIME_H_
#endif  // TIME_H_

#ifndef MATH_H_
    #include <math.h>
    #define MATH_H_
#endif   // MATH_H_

#ifndef STDINT_H_
    #include <stdint.h>
    #define STDINT_H_
#endif   // STDINT_H_

#ifndef STDIO_H_
    #include <stdio.h>
    #define STDIO_H_
#endif  // STDIO_H_

#ifndef STDLIB_H_
    #include <stdlib.h>
    #define STDLIB_H_
#endif  // STDLIB_H_

#ifndef STRING_H_
    #include <string.h>
    #define STRING_H_
#endif  // STRING_H_

#ifndef MPI_H_
    #include <mpi.h>
    #define MPI_H_
#endif  // MPI_H_

#ifndef STDBOOL_H_
    #include <stdbool.h>
    #define STDBOOL_H_
#endif  // STDBOOL_H_

#ifndef COMPLEX_H_
    #include <complex.h>
    #define COMPLEX_H_
#endif  // COMPLEX_H_

#ifndef OMP_H_
    #include <omp.h>
    #define OMP_H_
#endif  // OMP_H_

#endif // MKL_H_
 
/* Structures */
/* Hamiltonian parameters */
typedef struct {
    double Jmin;
    double Jmax;
    double barrier_height;
    int barrier_width;
    int    size_hilb;
    double *J_val;
} H_Parameters;
    
/* Lattice graph structures */
typedef struct {
    int index;
    int n_couplings;
    int *coupling_indices;
} Qubit;

typedef struct {
    int site1;
    int site2;
    double strength;
} Coupling;

typedef enum {
    SQUARE_LATTICE,
    HEAVY_HEX_LATTICE
} LatticeType;

/* Lattice parameters */
typedef struct {
    int    Nx;
    int    Ny;
    int    size;
    int    n_edges;
    Qubit  *sites;
    Coupling *edges;
    LatticeType lattice_type;

    /* Optional square-lattice helper arrays for coordinate and neighbor lookup. */
    int *xcoord;
    int *ycoord;
    int *xplus;
    int *xminus;
    int *yplus;
    int *yminus;
} L_Parameters;

typedef struct {
    double alpha;
    double beta1;
    double beta2;
    int max_epochs;
} Adam_Parameters;

typedef struct {
    double *m;
    double *v;
    int t;
} Adam_State;

/* Monte Carlo parameters */
typedef struct {
    int iran;
    double Jpen;
    double P;
} MC_Parameters;

/* Time parameters */
typedef struct {
    int n_t_slices;
    double dt;
} T_Parameters;

/* Function prototypes */
void read_input(H_Parameters *ham_params, L_Parameters *lat_params, MC_Parameters *mc_params, Adam_Parameters *adam_params, T_Parameters *t_params, bool *inversion, int *realization, bool *time_evol, int *opt_time, int *Bell, bool *use_heavy_hex);
void open_output_file(H_Parameters ham_params, L_Parameters lat_params, MC_Parameters mc_params, bool inversion, int realization, int opt_time, bool use_heavy_hex, FILE **fpout);
void out_header(FILE *fpout,H_Parameters ham_params, L_Parameters lat_params, MC_Parameters mc_params);
void diagonalize_symmetric(double *H, double *eigenv, double *eigenvec, ULI size_hilb, bool get_eigenvec);
void build_hamiltonian(double *H, double *J_val, H_Parameters ham_params, L_Parameters lat_params, bool inversion, int N_couplings);
void print_matrix( char* desc, int m, int n, double * a, int lda );
void check_hermiticity(char* desc, double *H, ULI size);
void build_grid_lattice(L_Parameters *l);
void build_heavy_hex_lattice(L_Parameters *l, int L);
void export_to_dot(const L_Parameters *l, const char *path);
bool is_barrier_site(int i, L_Parameters lat_params, H_Parameters ham_params);
void add_edge(L_Parameters *l, int site1, int site2, int *edge_count);
void init_qubit(Qubit *q, int index, int degree);
ULI inv_op(ULI site, ULI nsites);
double get_ave_coupling(double *J);
double global_update_couplings(double *J, H_Parameters ham_params, MC_Parameters mc_params, int N_couplings);
double local_update_couplings(double *J, H_Parameters ham_params, MC_Parameters mc_params, int N_couplings);
void freeing_arrays(L_Parameters *, H_Parameters *h);
double compute_opt_time(int opt_time, double J_ave, double t_opt);
void init_quantum_states(int Bell, ULI size_hilb, double *psi_0, MKL_Complex16 *psi_target);
void initialize_random_couplings(double *J_val, int N_couplings);
void write_time_evolution_results(FILE *fpout, double *prob_ini_fin, double *prob_sites, double *eigenvec, double *eigenv, double *psi_0, MKL_Complex16 *psi_target, double eval_time, int n_t_slices, ULI size_hilb);
void safe_free_all(Adam_State *adam_state, double **grad, double **H, double **eigenv, double **eigenvec, double **psi_0, MKL_Complex16 **psi_target, MKL_Complex16 **U, double **prob_sites, double **prob_ini_fin, double **J_temp, double **J_best, L_Parameters *l, H_Parameters *h
);void parse_parameter(FILE *fpin, const char *param_name, const char *format, void *dest);
static inline void mkl_safe_free_ptr(void **ptr);
static inline void safe_free_ptr(void **ptr);

/* prototypes of the dynamics */
void comp_dynamics(double *prob_ini_fin, double *prob_sites, double *eigenvec, double *eigenv, double *psi_0, MKL_Complex16 *psi_target, double time, ULI size_hilb);
void compute_gradients(double *grad, double *J_val, int N_couplings, double delta_J,H_Parameters ham_params, L_Parameters lat_params, bool inversion, double *H, double *eigenv, double *eigenvec, double *psi_0, MKL_Complex16 *psi_target, double eval_time, ULI size_hilb);
void adam_step(double *J_val, const double *grad, Adam_State *state, Adam_Parameters *adam, int N_couplings);