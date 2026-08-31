// This code aims in building a single-particle QST problem in 2d depending on a barrier
// 1) Inversion symmetry is now an option in the input file
// 2) Previous opt_times are still valid, and now added a new one: opt_time = 4
//    which sets the time to be t_opt = (pi/2)/J_opt, where J_opt is the maximum coupling
//    in the chain (which is the minimum coupling outside the barrier region)
// 3) The code is parallelized with MPI, where each node tackles a single realization
//    (the seed for the random number generator is changed accordingly)
// 4) The code is also parallelized with OpenMP within each node (for the Hamiltonian construction)
// 5) The code uses MKL for linear algebra routines
// 6) The code uses MKL's VSL for random number generation
// 7) We now add an option to select the initial state, single excitation localized at site 0 or a Bell state in the first two sites

#include "qst_barrier_2d_copy.h"
#include <sys/utsname.h>


/* MACROS */
#define ERROR(x)        {printf("\n\nERROR!!! %s\n\n\n",(x)); exit(-1);}
#define PI (4*atan(1.0))

#define SAFE_MKL_FREE(ptr) mkl_safe_free_ptr((void**)&(ptr))
#define SAFE_FREE(ptr) safe_free_ptr((void**)&(ptr))

int N_couplings = 0;   // Global variable

/* Global variables - streams of random numbers */
VSLStreamStatePtr stream_d = NULL;

// Global variables for Lapack routines
const MKL_Complex16 z_one = {1.0, 0.0};
const MKL_Complex16 z_zero = {0.0, 0.0};
const double d_one = 1.0;
const double d_zero = 0.0;
const int    i_one = 1;
const int    i_zero = 0;
const double epsilon = 1e-12;

int main(int argc, char *argv[]) {
    // 1. Declare local state variables
    L_Parameters lat_params = {0};
    H_Parameters ham_params = {0};
    MC_Parameters mc_params = {0};
    Adam_Parameters adam_params = {0};
    T_Parameters t_params = {0};

    bool inversion = false;
    bool time_evol = false;
    bool use_heavy_hex = false;
    int realization = 0;
    int opt_time = 0;
    int Bell = 0;
    FILE *fpout = NULL;

    // 2. Read configuration file FIRST
    read_input(&ham_params, &lat_params, &mc_params, &adam_params, &t_params, 
               &inversion, &realization, &time_evol, &opt_time, &Bell, &use_heavy_hex);

    // 3. Build lattice topology to determine system sizes
    if (use_heavy_hex) {
        build_heavy_hex_lattice(&lat_params, lat_params.Nx);
    } else {
        build_grid_lattice(&lat_params);
    }

    ULI size_hilb = (ULI)lat_params.size;
    
    if (inversion && !use_heavy_hex) {
        N_couplings = (lat_params.n_edges + 1) / 2;
    } else {
        N_couplings = lat_params.n_edges;
    }

    if (N_couplings <= 0) {
        ERROR("Invalid N_couplings calculated from lattice layout!");
    }

    double J_ave = (ham_params.Jmax + ham_params.Jmin) / 2.0;
    double t_opt = (PI / 2.0) / ham_params.Jmax; 
    double eval_time = compute_opt_time(opt_time, J_ave, t_opt);

    open_output_file(ham_params, lat_params, mc_params, inversion, realization, opt_time, use_heavy_hex, &fpout);
    out_header(fpout, ham_params, lat_params, mc_params);

    // 4. Memory Allocations
    double *H = malloc_1d_double(size_hilb * size_hilb);
    double *eigenv = malloc_1d_double(size_hilb);
    double *eigenvec = malloc_1d_double(size_hilb * size_hilb);
    double *psi_0 = malloc_1d_double(size_hilb);
    MKL_Complex16 *psi_target = malloc_1d_dcomplex(size_hilb);

    double *J_val = malloc_1d_double(N_couplings);
    double *J_best = malloc_1d_double(N_couplings);
    double *grad = malloc_1d_double(N_couplings);

    // Keep reference in ham_params
    ham_params.J_val = J_val;

    // 5. Initialize random stream & generate initial couplings
    int seed = mc_params.iran + realization * 1000;
    int status = vslNewStream(&stream_d, VSL_BRNG_MT19937, seed);
    if (status != VSL_STATUS_OK) {
        fprintf(stderr, "Error: Failed to initialize MKL random stream (Status code: %d).\n", status);
        exit(EXIT_FAILURE);
    }

    vdRngUniform(VSL_RNG_METHOD_UNIFORM_STD, stream_d, N_couplings, J_val, 0.0, 1.0);
    init_quantum_states(Bell, size_hilb, psi_0, psi_target);

    double *prob_sites = malloc_1d_double(size_hilb * t_params.n_t_slices);
    
    Adam_State adam_state;
    adam_state.m = calloc_1d_double(N_couplings);
    adam_state.v = calloc_1d_double(N_couplings);
    adam_state.t = 0;

    double best_loss = 1e9;
    double delta_J = 1e-5;

    // 6. Optimization Loop
    for (int step = 0; step < adam_params.max_epochs; step++) {
        build_hamiltonian(H, J_val, ham_params, lat_params, inversion, N_couplings);
        diagonalize_symmetric(H, eigenv, eigenvec, size_hilb, true);
        
        double prob_ini_fin[2];
        comp_dynamics(prob_ini_fin, NULL, eigenvec, eigenv, psi_0, psi_target, eval_time, size_hilb);
        
        double current_fidelity = prob_ini_fin[1];
        double current_loss = (1.0 - current_fidelity) * (double)size_hilb;

        if (current_loss < best_loss) {
            best_loss = current_loss;
            cblas_dcopy(N_couplings, J_val, 1, J_best, 1);
        }

        // --- TERMINAL PROGRESS BAR ---
        int bar_width = 30;
        float progress = (float)(step + 1) / adam_params.max_epochs;
        int filled = (int)(progress * bar_width);

        // \r returns the cursor to the start of the line without printing a newline
        printf("\rProgress: [");
        for (int i = 0; i < bar_width; i++) {
            if (i < filled) printf("=");
            else if (i == filled) printf(">");
            else printf(" ");
        }
        printf("] %3d%% | Loss: %8.4f | Fidelity: %.6f | Best: %8.4f", 
            (int)(progress * 100), current_loss, current_fidelity, best_loss);
        
        // Force terminal to display the buffer immediately
        fflush(stdout);

        compute_gradients(grad, J_val, N_couplings, delta_J, ham_params, lat_params, inversion, H, eigenv, eigenvec, psi_0, psi_target, eval_time, size_hilb);

        adam_step(J_val, grad, &adam_state, &adam_params, N_couplings);
    }

    // Print a newline at the end so the next terminal prompt starts on a fresh line
    printf("\n");

    // 7. Final Evaluation & Output Writing
    build_hamiltonian(H, J_best, ham_params, lat_params, inversion, N_couplings);
    diagonalize_symmetric(H, eigenv, eigenvec, size_hilb, true);

    double final_probs[2];
    comp_dynamics(final_probs, prob_sites, eigenvec, eigenv, psi_0, psi_target, eval_time, size_hilb);

    if (time_evol) write_time_evolution_results(fpout, final_probs, prob_sites, eigenvec, eigenv, psi_0, psi_target, eval_time, t_params.n_t_slices, size_hilb);

    fclose(fpout);

    // Prevent double-free: detach shared alias before safe_free_all
    ham_params.J_val = NULL;

    // 8. Safe Cleanup
    safe_free_all(&adam_state, &grad, &H, &eigenv, &eigenvec, &psi_0, &psi_target, NULL, &prob_sites, NULL, &J_val, &J_best, &lat_params, &ham_params);
    
    vslDeleteStream(&stream_d);

    return 0;
}

/* ===================== Read input file parameters ========================== */
void read_input(H_Parameters *ham_params, L_Parameters *lat_params, MC_Parameters *mc_params, Adam_Parameters *adam_params, T_Parameters *t_params, bool *inversion, int *realization, bool *time_evol, int *opt_time, int *Bell, bool *use_heavy_hex){
    FILE *fpin = fopen("input.in", "r");
    if (!fpin) ERROR("Can't open/find input file: input.in!\n");

    char string_inversion[50];
    char comp_time_evol[50];
    int hex_val = 0;

    // 1. Lattice parameters
    parse_parameter(fpin, "Nx", "%d", &(lat_params->Nx));
    parse_parameter(fpin, "Ny", "%d", &(lat_params->Ny));

   // 2. Hamiltonian parameters
    parse_parameter(fpin, "Jmin", "%lf", &(ham_params->Jmin));
    parse_parameter(fpin, "Jmax", "%lf", &(ham_params->Jmax));
    parse_parameter(fpin, "barrier_height", "%lf", &(ham_params->barrier_height));
    parse_parameter(fpin, "barrier_width", "%d", &(ham_params->barrier_width));

    if (ham_params->barrier_width >= lat_params->Nx) {
        ERROR("Barrier width must be smaller than the lattice size (Nx).");
    }

    // 3. Random Seed
    parse_parameter(fpin, "iran", "%d", &(mc_params->iran));
    parse_parameter(fpin, "realization", "%d", realization);

    // 4. Adam Optimization Parameters
    parse_parameter(fpin, "alpha", "%lf", &(adam_params->alpha));
    parse_parameter(fpin, "beta1", "%lf", &(adam_params->beta1));
    parse_parameter(fpin, "beta2", "%lf", &(adam_params->beta2));
    parse_parameter(fpin, "max_epochs", "%d", &(adam_params->max_epochs));

   // 5. Penalty / Boundary terms
    parse_parameter(fpin, "Jpen", "%lf", &(mc_params->Jpen));
    parse_parameter(fpin, "P", "%lf", &(mc_params->P));

    parse_parameter(fpin, "inversion", "%s", string_inversion);
    *inversion = (strcmp(string_inversion, "true") == 0);

    parse_parameter(fpin, "time_evol", "%s", comp_time_evol);
    if(strcmp(comp_time_evol, "true") == 0) *time_evol = true;
    else *time_evol = false;

    // 7. Time & Optimization settings
    parse_parameter(fpin, "n_t_slices", "%d", &(t_params->n_t_slices));
    parse_parameter(fpin, "opt_time", "%d", opt_time);

    // 8. Initial state and topology
    parse_parameter(fpin, "Bell", "%d", Bell);
    parse_parameter(fpin, "Heavy_hex", "%d", &hex_val);
    *use_heavy_hex = (hex_val == 1);

    fclose(fpin); fpin=NULL;
    printf("finished reading\n");
    return;
}

void parse_parameter(FILE *fpin, const char *param_name, const char *format, void *dest) {
    char line[200];
    char search_str[100];
    sprintf(search_str, "%s =", param_name);
    size_t len = strlen(search_str);

    fseek(fpin, 0, SEEK_SET);

    while (fgets(line, sizeof(line), fpin)) {
        if (strncmp(line, search_str, len) == 0) {
            char parse_format[100];
            sprintf(parse_format, "%s = %s", param_name, format);
            if (sscanf(line, parse_format, dest) == 1) {
                return;
            }
        }
    }
    fprintf(stderr, "Input read error: '%s' parameter not found or incorrectly formatted.\n", param_name);
    exit(1);
}

double compute_opt_time(int opt_time, double J_ave, double t_opt) {
    switch (opt_time) {
        case 1:  return (PI / 2.0) * fabs(J_ave);
        case 2:  return (PI / 2.0);
        case 3:  return (PI / 2.0) / fabs(J_ave);
        case 4:  return t_opt;
        default: return t_opt;
    }
}

void init_quantum_states(int Bell, ULI size_hilb, double *psi_0, MKL_Complex16 *psi_target) {
    memset(psi_0, 0, size_hilb * sizeof(double));
    memset(psi_target, 0, size_hilb * sizeof(MKL_Complex16));

    if (Bell) {
        psi_0[0] = 1.0 / sqrt(2.0);
        psi_0[1] = -1.0 / sqrt(2.0);
        psi_target[size_hilb - 2].real = 1.0 / sqrt(2.0);
        psi_target[size_hilb - 1].real = -1.0 / sqrt(2.0);
    } else {
        psi_0[0] = 1.0;
        psi_target[size_hilb - 1].real = 1.0;
    }
}

void open_output_file(H_Parameters ham_params, L_Parameters lat_params, MC_Parameters mc_params, bool inversion, int realization, int opt_time, bool use_heavy_hex, FILE **fpout){
    char outputfile[300];
    H_Parameters *h = &ham_params;
    L_Parameters *l = &lat_params;
    MC_Parameters *mc = &mc_params;

    sprintf(outputfile, "2d_Nx%d_Ny%dJpen%.3fP%.1f_Jmin%.2fJmax%.2f_barrier_height%.2f_barrier_width%d_inversion%d_r%d_opt_time%d%s.out", 
        l->Nx, l->Ny, mc->Jpen, mc->P, h->Jmin, h->Jmax, h->barrier_height, h->barrier_width, (int)inversion, realization, opt_time, use_heavy_hex ? "_HeavyHex" : "");
    
    if((*fpout = fopen(outputfile,"w"))==NULL) ERROR("It was not possible to generate output file");
    
    return;
}

void out_header(FILE *fpout,H_Parameters ham_params, L_Parameters lat_params, MC_Parameters mc_params)
{
    fprintf(fpout, "Nx = %d\tNy = %d\n\n", lat_params.Nx, lat_params.Ny);
    fprintf(fpout," iran = %d\n\n",mc_params.iran);
    return;
}

void diagonalize_symmetric(double *H, double *eigenv, double *eigenvec, ULI size_hilb, bool get_eigenvec)
{
    int n = (int) size_hilb;
    double wkopt;
    double *work = NULL;
    int info, lwork;

    cblas_dcopy(size_hilb*size_hilb, H, i_one, eigenvec, i_one);

    if (get_eigenvec) {
        lwork = -1;
        dsyev( "v", "u", &n, eigenvec, &n, eigenv, &wkopt, &lwork, &info );
        lwork = (int)wkopt;
        work = (double *)mkl_malloc(lwork * sizeof(double), 64);
        dsyev( "v", "u", &n, eigenvec, &n, eigenv, work, &lwork, &info );
    } else {
        lwork = -1;
        dsyev( "n", "u", &n, eigenvec, &n, eigenv, &wkopt, &lwork, &info );
        lwork = (int)wkopt;
        work = (double *)mkl_malloc(lwork * sizeof(double), 64);
        dsyev( "n", "u", &n, eigenvec, &n, eigenv, work, &lwork, &info );
    }

    if(info!=0) ERROR("Diagonalization failed to converge");
    SAFE_MKL_FREE(work);

    return;
}

void write_time_evolution_results(FILE *fpout, double *prob_ini_fin, double *prob_sites, double *eigenvec, double *eigenv, double *psi_0, MKL_Complex16 *psi_target, double eval_time, int n_t_slices, ULI size_hilb) {
    double total_time = fabs(eval_time);
    double dt = (n_t_slices > 1) ? (total_time / (n_t_slices - 1)) : epsilon;

    fprintf(fpout, "n_t_slices = %d\n\n", n_t_slices);
    fprintf(fpout, "time");
    for (ULI s = 0; s < size_hilb; s++) {
        fprintf(fpout, "\tsite_%llu", (unsigned long long)s);
    }
    fprintf(fpout, "\n");

    for (int step = 0; step < n_t_slices; step++) {
        double current_t = step * dt;
        comp_dynamics(prob_ini_fin, prob_sites, eigenvec, eigenv, psi_0, psi_target, current_t, size_hilb);

        fprintf(fpout, "%.6f\t", current_t);
        for (ULI s = 0; s < size_hilb; s++) {
            fprintf(fpout, "\t%.16f", prob_sites[s]);
        }
        fprintf(fpout, "\n");
    }
}

static inline void mkl_safe_free_ptr(void **ptr) {
    if (ptr && *ptr) {
        mkl_free(*ptr);
        *ptr = NULL; // Prevents double-free on repeated references
    }
}

static inline void safe_free_ptr(void **ptr) {
    if (ptr && *ptr) {
        mkl_free(*ptr);
        *ptr = NULL; // Prevents double-free on repeated references
    }
}

void safe_free_all(Adam_State *adam_state, double **grad, double **H, double **eigenv, double **eigenvec, double **psi_0, MKL_Complex16 **psi_target, MKL_Complex16 **U, double **prob_sites, double **prob_ini_fin, double **J_temp, double **J_best, L_Parameters *l, H_Parameters *h) {
    if (adam_state) {
        if (adam_state->m) { mkl_free(adam_state->m); adam_state->m = NULL; }
        if (adam_state->v) { mkl_free(adam_state->v); adam_state->v = NULL; }
    }
    
    // Safely verify double pointers before dereferencing and freeing
    if (grad && *grad)         { mkl_free(*grad); *grad = NULL; }
    if (H && *H)               { mkl_free(*H); *H = NULL; }
    if (eigenv && *eigenv)     { mkl_free(*eigenv); *eigenv = NULL; }
    if (eigenvec && *eigenvec) { mkl_free(*eigenvec); *eigenvec = NULL; }
    if (psi_0 && *psi_0)       { mkl_free(*psi_0); *psi_0 = NULL; }
    if (psi_target && *psi_target) { mkl_free(*psi_target); *psi_target = NULL; }
    if (U && *U)               { mkl_free(*U); *U = NULL; }
    if (prob_sites && *prob_sites) { mkl_free(*prob_sites); *prob_sites = NULL; }
    if (prob_ini_fin && *prob_ini_fin) { mkl_free(*prob_ini_fin); *prob_ini_fin = NULL; }
    if (J_temp && *J_temp)     { mkl_free(*J_temp); *J_temp = NULL; }
    if (J_best && *J_best)     { mkl_free(*J_best); *J_best = NULL; }

    if (h && h->J_val) {
        mkl_free(h->J_val);
        h->J_val = NULL;
    }

    if (l) {
        if (l->sites) {
            for (int i = 0; i < l->size; i++) {
                if (l->sites[i].coupling_indices) {
                    mkl_free(l->sites[i].coupling_indices);
                    l->sites[i].coupling_indices = NULL;
                }
            }
            mkl_free(l->sites);
            l->sites = NULL;
        }

        if (l->edges) {
            mkl_free(l->edges);
            l->edges = NULL;
        }

        if (l->lattice_type == SQUARE_LATTICE) {
            if (l->xplus)  { mkl_free(l->xplus); l->xplus = NULL; }
            if (l->xminus) { mkl_free(l->xminus); l->xminus = NULL; }
            if (l->yplus)  { mkl_free(l->yplus); l->yplus = NULL; }
            if (l->yminus) { mkl_free(l->yminus); l->yminus = NULL; }
        }
        
        if (l->xcoord) { mkl_free(l->xcoord); l->xcoord = NULL; }
        if (l->ycoord) { mkl_free(l->ycoord); l->ycoord = NULL; }
    }
}

void build_hamiltonian(double *H, double *J_val, H_Parameters ham_params, L_Parameters lat_params, bool inversion, int N_couplings)
{
    int N = lat_params.size;

    for(int i=0;i<N*N;i++) H[i]=0.0;

    for(int e=0;e<lat_params.n_edges;e++)
    {
        int i = lat_params.edges[e].site1;
        int j = lat_params.edges[e].site2;

        int coupling_index = inversion ? (e % N_couplings) : e;
        double J = J_val[coupling_index];

        H[i + N*j] = J;
        H[j + N*i] = J;
    }

    for(int i=0;i<N;i++)
    {
        if(is_barrier_site(i, lat_params, ham_params))
        {
            H[i + N*i] = ham_params.barrier_height;
        }
    }
}

bool is_barrier_site(int i, L_Parameters lat, H_Parameters ham)
{
    int x = lat.xcoord[i];

    switch(lat.lattice_type)
    {
    case SQUARE_LATTICE:
    case HEAVY_HEX_LATTICE:
    {
        int cut = lat.Nx/2;
        return abs(x-cut) < ham.barrier_width/2.0;
    }
    default:
        return false;
    }
}

void add_edge(L_Parameters *l, int site1, int site2, int *edge_count)
{
    int e = *edge_count;

    l->edges[e].site1 = site1;
    l->edges[e].site2 = site2;

    l->sites[site1].coupling_indices[l->sites[site1].n_couplings++] = e;
    l->sites[site2].coupling_indices[l->sites[site2].n_couplings++] = e;

    (*edge_count)++;
}

void init_qubit(Qubit *q, int index, int degree)
{
    q->index = index;
    q->n_couplings = 0;

    q->coupling_indices = malloc_1d_int(degree);

    if (!q->coupling_indices)
        ERROR("Memory allocation failed for coupling indices");

    for (int i = 0; i < degree; i++)
        q->coupling_indices[i] = -1;
}

void build_grid_lattice(L_Parameters *l)
{
    l->lattice_type = SQUARE_LATTICE;
    int lx = l->Nx;
    int ly = l->Ny;
    int n = lx * ly;

    int max_edges = (lx-1)*ly + lx*(ly-1);

    l->size = n;

    l->xcoord = malloc_1d_int(n);
    l->ycoord = malloc_1d_int(n);
    l->xplus  = malloc_1d_int(n);
    l->xminus = malloc_1d_int(n);
    l->yplus  = malloc_1d_int(n);
    l->yminus = malloc_1d_int(n);

    l->sites = (Qubit*) mkl_malloc(n * sizeof(Qubit), 64);
    l->edges = (Coupling*) mkl_malloc(max_edges * sizeof(Coupling), 64);

    if (!l->sites || !l->edges || !l->xcoord || !l->ycoord || !l->xplus || !l->xminus || !l->yplus || !l->yminus) 
    {
        ERROR("Lattice allocation failed");
    }

    for (int i = 0; i < n; i++)
    {
        int x = i % lx;
        int y = i / lx;

        l->xcoord[i] = x;
        l->ycoord[i] = y;

        int degree = 0;
        if (x > 0) degree++;
        if (x < lx - 1) degree++;
        if (y > 0) degree++;
        if (y < ly - 1) degree++;

        init_qubit(&l->sites[i], i, degree);

        l->xplus[i] = l->xminus[i] = -1;
        l->yplus[i] = l->yminus[i] = -1;
    }

    int edge_count = 0;

    for (int i = 0; i < n; i++)
    {
        int x = l->xcoord[i];
        int y = l->ycoord[i];

        if (x+1 < lx)
        {
            add_edge(l, i, i+1, &edge_count);
            l->xplus[i] = i+1;
            l->xminus[i+1] = i;
        }

        if (y+1 < ly)
        {
            add_edge(l, i, i+lx, &edge_count);
            l->yplus[i] = i+lx;
            l->yminus[i+lx] = i;
        }
    }

    l->n_edges = edge_count;
}

void build_heavy_hex_lattice(L_Parameters *l, int L)
{
    l->lattice_type = HEAVY_HEX_LATTICE;

    int rows = L+1;
    int cols = L+1;

    int *grid = mkl_malloc(rows*cols * sizeof(int), 64);

    for (int i=0;i<rows*cols;i++) grid[i] = -1;

    int n = 0;

    for(int r=0;r<rows;r++)
    {
        for(int c=0;c<cols;c++)
        {
            bool valid = false;

            if(r%2==0)
                valid = true;
            else
            {
                int ra = r-1;
                valid = ((ra%4==0 && c%4==0) || (ra%4==2 && c%4==2));
            }
            if(valid) grid[r*cols+c] = n++;
        }
    }

    l->size = n;

    l->xcoord = malloc_1d_int(n);
    l->ycoord = malloc_1d_int(n);

    l->sites = (Qubit*) mkl_malloc(n * sizeof(Qubit), 64);

    int max_edges = n*3;
    l->edges = (Coupling*) mkl_malloc(max_edges * sizeof(Coupling), 64);

    if (!l->sites || !l->edges || !l->xcoord || !l->ycoord) {
        ERROR("Memory allocation failed inside build_heavy_hex_lattice");
    }

    for(int i=0;i<n;i++) init_qubit(&l->sites[i], i, 3);

    int edge_count = 0;

    for(int r=0;r<rows;r++)
    {
        for(int c=0;c<cols;c++)
        {
            int u = grid[r*cols+c];
            if(u == -1)continue;

            if(c+1<cols)
            {
                int v = grid[r*cols+c+1];
                if(v!=-1) add_edge(l,u,v,&edge_count);
            }

            if(r+1<rows)
            {
                int v = grid[(r+1)*cols+c];
                if(v!=-1) add_edge(l,u,v,&edge_count);
            }
        }
    }

    l->n_edges = edge_count;

    for(int r=0;r<rows;r++)
    {
        for(int c=0;c<cols;c++)
        {
            int i = grid[r*cols+c];

            if(i!=-1)
            {
                l->xcoord[i] = c;
                l->ycoord[i] = r;
            }
        }
    }

    SAFE_MKL_FREE(grid);
}

void export_to_dot(const L_Parameters *l, const char *path)
{
    int n = l->size;
    FILE *out = fopen(path, "w");
    if (!out) {
        fprintf(stderr, "ERROR: Could not open %s for writing\n", path);
        return;
    }

    fprintf(out, "graph G {\n");
    for (int i = 0; i < n; i++) {
        fprintf(out, "  n%d;\n", i);
    }

    for (int e = 0; e < l->n_edges; e++) {
        fprintf(out, "  n%d -- n%d;\n", l->edges[e].site1, l->edges[e].site2);
    }

    fprintf(out, "}\n");
    fclose(out);
}

double get_ave_coupling(double *J)
{
    double ave = 0.0;
    for (int i = 0; i < N_couplings; i++) {
        ave += J[i];
    }
    ave = ave/(2.0*N_couplings);
    return ave;
}

void compute_gradients(double *grad, double *J_val, int N_couplings, double delta_J, H_Parameters ham_params, L_Parameters lat_params, bool inversion, double *H, double *eigenv, double *eigenvec, double *psi_0, MKL_Complex16 *psi_target, double eval_time, ULI size_hilb) {
    double *J_temp = malloc_1d_double(N_couplings);
    cblas_dcopy(N_couplings, J_val, 1, J_temp, 1);

    for (int k = 0; k < N_couplings; k++) {
        double original_J = J_temp[k];

        J_temp[k] = original_J + delta_J;
        ham_params.J_val = J_temp;
        build_hamiltonian(H, J_temp, ham_params, lat_params, inversion, N_couplings);
        diagonalize_symmetric(H, eigenv, eigenvec, size_hilb, true);
        double prob_plus[2];
        comp_dynamics(prob_plus, NULL, eigenvec, eigenv, psi_0, psi_target, eval_time, size_hilb);
        double E_plus = (1.0 - prob_plus[1]) * size_hilb;

        J_temp[k] = original_J - delta_J;
        build_hamiltonian(H, J_temp, ham_params, lat_params, inversion, N_couplings);
        diagonalize_symmetric(H, eigenv, eigenvec, size_hilb, true);
        double prob_minus[2];
        comp_dynamics(prob_minus, NULL, eigenvec, eigenv, psi_0, psi_target, eval_time, size_hilb);
        double E_minus = (1.0 - prob_minus[1]) * size_hilb;

        grad[k] = (E_plus - E_minus) / (2.0 * delta_J);
        J_temp[k] = original_J;
    }

    SAFE_MKL_FREE(J_temp);
    ham_params.J_val = J_val;
}

void adam_step(double *J_val, const double *grad, Adam_State *state, Adam_Parameters *adam, int N_couplings) {
    state->t++;
    double beta1_t = pow(adam->beta1, state->t);
    double beta2_t = pow(adam->beta2, state->t);
    double eps = 1e-8;

    for (int i = 0; i < N_couplings; i++) {
        state->m[i] = adam->beta1 * state->m[i] + (1.0 - adam->beta1) * grad[i];
        state->v[i] = adam->beta2 * state->v[i] + (1.0 - adam->beta2) * (grad[i] * grad[i]);

        double m_hat = state->m[i] / (1.0 - beta1_t);
        double v_hat = state->v[i] / (1.0 - beta2_t);

        J_val[i] -= adam->alpha * m_hat / (sqrt(v_hat) + eps);
    }
}

void comp_dynamics(double *prob_ini_fin, double *prob_sites, double *eigenvec, double *eigenv, double *psi_0, MKL_Complex16 *psi_target, double time, ULI size_hilb)
{
    int N = (int) size_hilb;
    const int STACK_LIMIT = 64;

    double *c_k = NULL;
    double *psi_t_real = NULL;
    double *psi_t_imag = NULL;

    double c_k_stack[STACK_LIMIT];
    double psi_t_real_stack[STACK_LIMIT];
    double psi_t_imag_stack[STACK_LIMIT];

    if (N <= STACK_LIMIT) {
        c_k = c_k_stack;
        psi_t_real = psi_t_real_stack;
        psi_t_imag = psi_t_imag_stack;
    } else {
        c_k = (double*) malloc(N * sizeof(double));
        psi_t_real = (double*) malloc(N * sizeof(double));
        psi_t_imag = (double*) malloc(N * sizeof(double));

        if (!c_k || !psi_t_real || !psi_t_imag) {
            fprintf(stderr, "Allocation failed in comp_dynamics for N=%d\n", N);
            exit(EXIT_FAILURE);
        }
    }

    for (int k = 0; k < N; k++) {
        double sum = 0.0;
        for (int j = 0; j < N; j++) {
            if (psi_0[j] != 0.0) {
                sum += eigenvec[j + k * N] * psi_0[j];
            }
        }
        c_k[k] = sum;
    }

    for (int i = 0; i < N; i++) {
        double real_i = 0.0;
        double imag_i = 0.0;
        for (int k = 0; k < N; k++) {
            double phase = -eigenv[k] * time;
            double coeff = eigenvec[i + k * N] * c_k[k];
            real_i += coeff * cos(phase);
            imag_i += coeff * sin(phase);
        }
        psi_t_real[i] = real_i;
        psi_t_imag[i] = imag_i;

        if (prob_sites) {
            prob_sites[i] = real_i * real_i + imag_i * imag_i;
        }
    }
    double overlap_real = 0.0;
    double overlap_imag = 0.0;

    for (int i = 0; i < N; i++) {
        double a_r = psi_t_real[i];
        double a_i = psi_t_imag[i];
        double b_r = psi_target[i].real;
        double b_i = psi_target[i].imag;

        overlap_real += (a_r * b_r + a_i * b_i);
        overlap_imag += (a_r * b_i - a_i * b_r);
    }

    double fidelity = overlap_real * overlap_real + overlap_imag * overlap_imag;

    if (prob_ini_fin) {
        double initial_prob = 0.0;
        for (int i = 0; i < N; i++) {
            initial_prob += psi_0[i] * psi_0[i];
        }
        prob_ini_fin[0] = initial_prob;
        prob_ini_fin[1] = fidelity;
    }

    if (N > STACK_LIMIT) {
        SAFE_FREE(c_k);
        SAFE_FREE(psi_t_real);
        SAFE_FREE(psi_t_imag);
    }
}

ULI inv_op(ULI site, ULI nsites)
{
    return (ULI) (nsites - 1 - site);
}