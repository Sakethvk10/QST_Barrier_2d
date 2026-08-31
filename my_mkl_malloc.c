#include "my_mkl_malloc.h"

#ifndef COMPLEX_H_   /* Include guard */
#include <complex.h>
#endif

#ifndef STDBOOL_H_
#include <stdbool.h>
#endif

#ifndef STDDEF_H_
#include <stddef.h>
#endif

#ifndef STDINT_H_
#include <stdint.h>
#endif

#ifndef STDLIB_H_
#include <stdlib.h>
#endif

#ifndef STDIO_H_
#include <stdio.h>
#endif

#undef ERROR
#define ERROR(x)        {printf("\n\nERROR!!! %s\n\n\n",(x)); exit(-1);}

        /* Typedefs */
// typedef uint32_t UI;
// typedef int64_t  LI;
// typedef uint64_t ULI;
// typedef int8_t   SI;
// typedef uint8_t  USI;

#define ALIGN 64

/////  1D arrays - not initialized  ///////////////////////////////////////////////////////////////////////////////////////////

/* ======================== subroutine for dynamically allocating 1d bool array ============================================= */
bool *malloc_1d_bool(size_t xlen)
{
    bool *array=NULL;

    if( (array = (bool*) mkl_malloc(xlen*sizeof(bool), ALIGN )) == NULL) ERROR("without enough memory for defining 1d bool array");

    return array;
}

/* ======================== subroutine for dynamically allocating 1d int array ============================================== */
UI *malloc_1d_UI(size_t xlen)
{
    UI *array=NULL;
    
    if( (array = (UI*) mkl_malloc(xlen*sizeof(UI), ALIGN )) == NULL) ERROR("without enough memory for defining 1d UI array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d int array ============================================== */
int *malloc_1d_int(size_t xlen)
{
    int *array=NULL;
    
    if( (array = (int*) mkl_malloc(xlen*sizeof(int), ALIGN )) == NULL) ERROR("without enough memory for defining 1d int array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d int array ============================================== */
USI *malloc_1d_USI(size_t xlen)
{
    USI *array=NULL;
    
    if( (array = (USI*) mkl_malloc(xlen*sizeof(USI), ALIGN )) == NULL) ERROR("without enough memory for defining 1d USI array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d int array ============================================== */
SI *malloc_1d_SI(size_t xlen)
{
    SI *array=NULL;
    
    if( (array = (SI*) mkl_malloc(xlen*sizeof(SI), ALIGN )) == NULL) ERROR("without enough memory for defining 1d SI array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d ULI array ============================================== */
ULI *malloc_1d_ULI(size_t xlen)
{
    ULI *array=NULL;
    
    if( (array = (ULI*) mkl_malloc(xlen*sizeof(ULI), ALIGN )) == NULL) ERROR("without enough memory for defining 1d ULI array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d LI array ============================================== */
LI *malloc_1d_LI(size_t xlen)
{
    LI *array=NULL;
    
    if( (array = (LI*) mkl_malloc(xlen*sizeof(LI), ALIGN )) == NULL) ERROR("without enough memory for defining 1d LI array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d double array ============================================== */
double *malloc_1d_double(size_t xlen)
{
    double *array=NULL;
    
    if( (array = (double *) mkl_malloc(xlen*sizeof(double), ALIGN )) == NULL) ERROR("without enough memory for defining double array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d double complex array ============================================== */
MKL_Complex16 *malloc_1d_dcomplex(size_t xlen)
{
    MKL_Complex16 *array=NULL;
    
    if( (array = (MKL_Complex16*) mkl_malloc(xlen*sizeof(MKL_Complex16), ALIGN )) == NULL) ERROR("without enough memory for defining double complex array");
    
    return array;
}

/////  1D arrays - initialized zero  ///////////////////////////////////////////////////////////////////////////////////////////

/* ======================== subroutine for dynamically allocating 1d bool array ============================================= */
bool *calloc_1d_bool(size_t xlen)
{
    bool *array=NULL;

    if( (array = (bool*) mkl_calloc(xlen, sizeof(bool), ALIGN) ) == NULL) ERROR("without enough memory for defining 1d bool array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d int array ============================================== */
UI *calloc_1d_UI(size_t xlen)
{
    UI *array=NULL;
    
    if( (array = (UI*) mkl_calloc(xlen, sizeof(UI), ALIGN) ) == NULL) ERROR("without enough memory for defining 1d UI array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d int array ============================================== */
int *calloc_1d_int(size_t xlen)
{
    int *array=NULL;
    
    if( (array = (int*) mkl_calloc(xlen, sizeof(int), ALIGN )) == NULL) ERROR("without enough memory for defining 1d int array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d int array ============================================== */
USI *calloc_1d_USI(size_t xlen)
{
    USI *array=NULL;
    
    if( (array = (USI*) mkl_calloc(xlen, sizeof(USI), ALIGN )) == NULL) ERROR("without enough memory for defining 1d USI array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d int array ============================================== */
SI *calloc_1d_SI(size_t xlen)
{
    SI *array=NULL;
    
    if( (array = (SI*) mkl_calloc(xlen, sizeof(SI), ALIGN )) == NULL) ERROR("without enough memory for defining 1d SI array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d ULI array ============================================== */
ULI *calloc_1d_ULI(size_t xlen)
{
    ULI *array=NULL;
    
    if( (array = (ULI*) mkl_calloc(xlen,sizeof(ULI), ALIGN )) == NULL) ERROR("without enough memory for defining 1d ULI array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d LI array ============================================== */
LI *calloc_1d_LI(size_t xlen)
{
    LI *array=NULL;
    
    if( (array = (LI*) mkl_calloc(xlen,sizeof(LI), ALIGN )) == NULL) ERROR("without enough memory for defining 1d LI array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d double array ============================================== */
double *calloc_1d_double(size_t xlen)
{
    double *array=NULL;
    
    if( (array = (double *) mkl_calloc(xlen,sizeof(double), ALIGN )) == NULL) ERROR("without enough memory for defining double array");
    
    return array;
}

/* ======================== subroutine for dynamically allocating 1d double complex array ============================================== */
MKL_Complex16 *calloc_1d_dcomplex(size_t xlen)
{
    MKL_Complex16 *array=NULL;
    
    if( (array = (MKL_Complex16*) mkl_calloc(xlen,sizeof(MKL_Complex16), ALIGN)) == NULL) ERROR("without enough memory for defining double complex array");
    
    return array;
}

//////////////////// Uninitialized 2d arrays ///////////////////////////////////////////////////////////////////////////////////////////////
/* ======================== subroutine for dynamically allocating a 2d bool array ========================== */
bool **malloc_2d_bool(size_t xlen, size_t ylen)
{
    size_t i;
    bool **array=NULL;  
    
    if( (array = (bool **) mkl_malloc( xlen*sizeof(bool *), ALIGN )) == NULL) ERROR("without enough memory for defining bool matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (bool *) mkl_malloc( ylen* sizeof(bool ), ALIGN)) == NULL)  ERROR("without enough memory for defining bool matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d  UI array ========================== */
UI **malloc_2d_UI(size_t xlen, size_t ylen)
{
    size_t i;
    UI **array=NULL;  
    
    if( (array = (UI **) mkl_malloc( xlen*sizeof(UI *), ALIGN )) == NULL) ERROR("without enough memory for defining UI matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (UI *) mkl_malloc( ylen* sizeof(UI ), ALIGN )) == NULL)  ERROR("without enough memory for defining UI matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d  int array ========================== */
int **malloc_2d_int(size_t xlen, size_t ylen)
{
    size_t i;
    int **array=NULL;  
    
    if( (array = (int **) mkl_malloc( xlen*sizeof(int *), ALIGN )) == NULL) ERROR("without enough memory for defining int matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (int *) mkl_malloc( ylen* sizeof(int ), ALIGN )) == NULL)  ERROR("without enough memory for defining int matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d  USI array ========================== */
USI **malloc_2d_USI(size_t xlen, size_t ylen)
{
    size_t i;
    USI **array=NULL;  
    
    if( (array = (USI **) mkl_malloc( xlen*sizeof(USI *), ALIGN)) == NULL) ERROR("without enough memory for defining USI matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (USI *) mkl_malloc( ylen*sizeof(USI ), ALIGN )) == NULL)  ERROR("without enough memory for defining USI matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d  SI array ========================== */
SI **malloc_2d_SI(size_t xlen, size_t ylen)
{
    size_t i;
    SI **array=NULL;  
    
    if( (array = (SI **) mkl_malloc( xlen*sizeof(SI *), ALIGN )) == NULL) ERROR("without enough memory for defining SI matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (SI *) mkl_malloc( ylen* sizeof(SI ), ALIGN )) == NULL)  ERROR("without enough memory for defining SI matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d  ULI array ========================== */
ULI **malloc_2d_ULI(size_t xlen, size_t ylen)
{
    size_t i;
    ULI **array=NULL;  
    
    if( (array = (ULI **) mkl_malloc( xlen*sizeof(ULI *), ALIGN )) == NULL) ERROR("without enough memory for defining ULI matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (ULI *) mkl_malloc( ylen* sizeof(ULI ), ALIGN )) == NULL)  ERROR("without enough memory for defining ULI matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d  LI array ========================== */
LI **malloc_2d_LI(size_t xlen, size_t ylen)
{
    size_t i;
    LI **array=NULL;  
    
    if( (array = (LI **) mkl_malloc( xlen*sizeof(LI *), ALIGN )) == NULL) ERROR("without enough memory for defining LI matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (LI *) mkl_malloc( ylen* sizeof(LI ), ALIGN )) == NULL)  ERROR("without enough memory for defining LI matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d  double array ========================== */
double **malloc_2d_double(size_t xlen, size_t ylen)
{
    size_t i;
    double **array=NULL;  
    
    if( (array = (double **) mkl_malloc( xlen*sizeof(double *), ALIGN ) ) == NULL) ERROR("without enough memory for defining double matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (double *) mkl_malloc( ylen* sizeof(double ), ALIGN )) == NULL)  ERROR("without enough memory for defining double matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d  double complex array ========================== */
MKL_Complex16 **malloc_2d_dcomplex(size_t xlen, size_t ylen)
{
    size_t i;
    MKL_Complex16 **array=NULL;  
    
    if( (array = (MKL_Complex16**) mkl_malloc( xlen*sizeof(MKL_Complex16*), ALIGN ) ) == NULL) ERROR("without enough memory for defining double complex matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (MKL_Complex16*) mkl_malloc( ylen* sizeof(MKL_Complex16), ALIGN )) == NULL)  ERROR("without enough memory for defining double complex matrix array[i]");
    }

    return array;
}


//////////////////// Zero'ed 2d arrays ///////////////////////////////////////////////////////////////////////////////////////////////
/* ======================== subroutine for dynamically allocating a 2d zeroed bool array ========================== */
bool **calloc_2d_bool(size_t xlen, size_t ylen)
{
    size_t i;
    bool **array=NULL;  
    
    if( (array = (bool **) mkl_calloc( xlen,sizeof(bool *), ALIGN ) ) == NULL) ERROR("without enough memory for defining bool matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (bool *) mkl_calloc( ylen, sizeof(bool ), ALIGN )) == NULL)  ERROR("without enough memory for defining bool matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d zeroed UI array ========================== */
UI **calloc_2d_UI(size_t xlen, size_t ylen)
{
    size_t i;
    UI **array=NULL;  
    
    if( (array = (UI **) mkl_calloc( xlen,sizeof(UI *), ALIGN ) ) == NULL) ERROR("without enough memory for defining UI matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (UI *) mkl_calloc( ylen, sizeof(UI ), ALIGN )) == NULL)  ERROR("without enough memory for defining UI matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d zeroed int array ========================== */
int **calloc_2d_int(size_t xlen, size_t ylen)
{
    size_t i;
    int **array=NULL;  
    
    if( (array = (int **) mkl_calloc( xlen,sizeof(int *), ALIGN ) ) == NULL) ERROR("without enough memory for defining int matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (int *) mkl_calloc( ylen, sizeof(int ), ALIGN )) == NULL)  ERROR("without enough memory for defining int matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d zeroed USI array ========================== */
USI **calloc_2d_USI(size_t xlen, size_t ylen)
{
    size_t i;
    USI **array=NULL;  
    
    if( (array = (USI **) mkl_calloc( xlen, sizeof(USI *), ALIGN ) ) == NULL) ERROR("without enough memory for defining USI matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (USI *) mkl_calloc( ylen, sizeof(USI ), ALIGN )) == NULL)  ERROR("without enough memory for defining USI matrix array[i]");
        
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d zeroed SI array ========================== */
SI **calloc_2d_SI(size_t xlen, size_t ylen)
{
    size_t i;
    SI **array=NULL;  
    
    if( (array = (SI **) mkl_calloc( xlen, sizeof(SI *), ALIGN ) ) == NULL) ERROR("without enough memory for defining SI matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (SI *) mkl_calloc( ylen, sizeof(SI ), ALIGN )) == NULL)  ERROR("without enough memory for defining SI matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d zeroed ULI array ========================== */
ULI **calloc_2d_ULI(size_t xlen, size_t ylen)
{
    size_t i;
    ULI **array=NULL;  
    
    if( (array = (ULI **) mkl_calloc( xlen, sizeof(ULI *), ALIGN ) ) == NULL) ERROR("without enough memory for defining ULI matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (ULI *) mkl_calloc( ylen, sizeof(ULI ), ALIGN )) == NULL)  ERROR("without enough memory for defining ULI matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d zeroed LI array ========================== */
LI **calloc_2d_LI(size_t xlen, size_t ylen)
{
    size_t i;
    LI **array=NULL;  
    
    if( (array = (LI **) mkl_calloc( xlen,sizeof(LI *), ALIGN) ) == NULL) ERROR("without enough memory for defining LI matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (LI *) mkl_calloc( ylen, sizeof(LI ), ALIGN)) == NULL)  ERROR("without enough memory for defining LI matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d zeroed double array ========================== */
double **calloc_2d_double(size_t xlen, size_t ylen)
{
    size_t i;
    double **array=NULL;  
    
    if( (array = (double **) mkl_calloc( xlen,sizeof(double *), ALIGN) ) == NULL) ERROR("without enough memory for defining double matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (double *) mkl_calloc( ylen, sizeof(double ), ALIGN)) == NULL)  ERROR("without enough memory for defining double matrix array[i]");
    }

    return array;
}

/* ======================== subroutine for dynamically allocating a 2d zeroed double complex array ========================== */
MKL_Complex16  **calloc_2d_dcomplex(size_t xlen, size_t ylen)
{
    size_t i;
    MKL_Complex16  **array=NULL;  
    
    if( (array = (MKL_Complex16  **) mkl_calloc( xlen, sizeof(MKL_Complex16  *), ALIGN) ) == NULL) ERROR("without enough memory for defining double complex matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (MKL_Complex16  *) mkl_calloc( ylen, sizeof(MKL_Complex16  ), ALIGN)) == NULL)  ERROR("without enough memory for defining double complex matrix array[i]");
    }

    return array;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* ======================== subroutine for dynamically allocating 3d double matrix ================================== */
bool ***malloc_3d_bool(size_t xlen, size_t ylen, size_t zlen)
{
    size_t i,j;
    bool ***array=NULL;  
    
    if( (array = (bool***) mkl_malloc( xlen * sizeof(bool**), ALIGN) ) == NULL) ERROR("without enough memory for defining matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (bool**) mkl_malloc( ylen * sizeof(bool*), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i]");
        for(j = 0; j < ylen; j++){
            if ( (array[i][j] = (bool*) mkl_malloc( zlen * sizeof(bool), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i][j]");
        }
    }

    return array;
}

/* ======================== subroutine for dynamically allocating 3d SI matrix ===================================== */
SI ***malloc_3d_SI(size_t xlen, size_t ylen, size_t zlen)
{
    size_t i,j;
    SI ***array=NULL;  
    
    if( (array = (SI***) mkl_malloc( xlen * sizeof(SI**), ALIGN) ) == NULL) ERROR("without enough memory for defining matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (SI**) mkl_malloc( ylen * sizeof(SI*), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i]");
        for(j = 0; j < ylen; j++){
            if ( (array[i][j] = (SI*) mkl_malloc( zlen * sizeof(SI), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i][j]");
        }
    }

    return array;
}

/* ======================== subroutine for dynamically allocating 3d USI matrix ===================================== */
USI ***malloc_3d_USI(size_t xlen, size_t ylen, size_t zlen)
{
    size_t i,j;
    USI ***array=NULL;  
    
    if( (array = (USI***) mkl_malloc( xlen * sizeof(USI**), ALIGN) ) == NULL) ERROR("without enough memory for defining matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (USI**) mkl_malloc( ylen * sizeof(USI*), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i]");
        for(j = 0; j < ylen; j++){
            if ( (array[i][j] = (USI*) mkl_malloc( zlen * sizeof(USI), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i][j]");
        }
    }

    return array;
}


/* ======================== subroutine for dynamically allocating 3d LI matrix ===================================== */
LI ***malloc_3d_LI(size_t xlen, size_t ylen, size_t zlen)
{
    size_t i,j;
    LI ***array=NULL;  
    
    if( (array = (LI***) mkl_malloc( xlen * sizeof(LI**), ALIGN) ) == NULL) ERROR("without enough memory for defining matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (LI**) mkl_malloc( ylen * sizeof(LI*), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i]");
        for(j = 0; j < ylen; j++){
            if ( (array[i][j] = (LI*) mkl_malloc( zlen * sizeof(LI), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i][j]");
        }
    }

    return array;
}

/* ======================== subroutine for dynamically allocating 3d ULI matrix ===================================== */
ULI ***malloc_3d_ULI(size_t xlen, size_t ylen, size_t zlen)
{
    size_t i,j;
    ULI ***array=NULL;  
    
    if( (array = (ULI***) mkl_malloc( xlen * sizeof(ULI**), ALIGN) ) == NULL) ERROR("without enough memory for defining matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (ULI**) mkl_malloc( ylen * sizeof(ULI*), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i]");
        for(j = 0; j < ylen; j++){
            if ( (array[i][j] = (ULI*) mkl_malloc( zlen * sizeof(ULI), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i][j]");
        }
    }

    return array;
}

/* ======================== subroutine for dynamically allocating 3d int matrix ================================== */
int ***malloc_3d_int(size_t xlen, size_t ylen, size_t zlen)
{
    size_t i,j;
    int ***array=NULL;  
    
    if( (array = (int***) mkl_malloc( xlen * sizeof(int**), ALIGN) ) == NULL) ERROR("without enough memory for defining matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (int**) mkl_malloc( ylen * sizeof(int*), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i]");
        for(j = 0; j < ylen; j++){
            if ( (array[i][j] = (int*) mkl_malloc( zlen * sizeof(int), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i][j]");
        }
    }

    return array;
}

/* ======================== subroutine for dynamically allocating 3d double complex matrix ========================== */
double  ***malloc_3d_double(size_t xlen, size_t ylen, size_t zlen)
{
    size_t i,j;
    double  ***array=NULL;  
    
    if( (array = (double ***) mkl_malloc( xlen * sizeof(double **), ALIGN) ) == NULL) ERROR("without enough memory for defining matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (double **) mkl_malloc( ylen * sizeof(double *), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i]");
        for(j = 0; j < ylen; j++){
            if ( (array[i][j] = (double *) mkl_malloc( zlen * sizeof(double ), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i][j]");
        }
    }

    return array;
}

/* ======================== subroutine for dynamically allocating 3d double complex matrix ========================== */
MKL_Complex16 ***malloc_3d_dcomplex(size_t xlen, size_t ylen, size_t zlen)
{
    size_t i,j;
    MKL_Complex16 ***array=NULL;  
    
    if( (array = (MKL_Complex16 ***) mkl_malloc( xlen * sizeof(MKL_Complex16 **), ALIGN) ) == NULL) ERROR("without enough memory for defining matrix array");
    for(i = 0; i < xlen; i++){
        if ( (array[i] = (MKL_Complex16 **) mkl_malloc( ylen * sizeof(MKL_Complex16 *), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i]");
        for(j = 0; j < ylen; j++){
            if ( (array[i][j] = (MKL_Complex16 *) mkl_malloc( zlen * sizeof(MKL_Complex16 ), ALIGN)) == NULL)  ERROR("without enough memory for defining matrix array[i][j]");
        }
    }

    return array;
}
