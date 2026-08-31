#ifndef MY_MKL_MALLOC_H_   /* Include guard */
#define MY_MKL_MALLOC_H_

#ifndef MKL_H_ /* Include guard */
#include <mkl.h>
#endif // MKL_H_

#ifndef COMPLEX_H_
#include <complex.h>
#endif  // COMPLEX_H_

#ifndef STDBOOL_H_
#include <stdbool.h>
#endif  // STDBOOL_H_

#ifndef STDDEF_H_
#include <stddef.h>
#endif  // STDDEF_H_

#ifndef STDINT_H_
#include <stdint.h>
#endif   // STDINT_H_

#ifndef STDLIB_H_
#include <stdlib.h>
#endif  // STDLIB_H_

#ifndef STDIO_H_
#include <stdio.h>
#endif  // STDIO_H_

    /* Typedefs */
typedef uint32_t UI;
typedef int64_t  LI;
typedef uint64_t ULI;
typedef int8_t   SI;
typedef uint8_t  USI;

bool *malloc_1d_bool(size_t xlen);       /* An example function declaration - ALL THE NECESSARY HEADERS*/
UI *malloc_1d_UI(size_t xlen);
int *malloc_1d_int(size_t xlen);
USI *malloc_1d_USI(size_t xlen);
SI *malloc_1d_SI(size_t xlen);
ULI *malloc_1d_ULI(size_t xlen);
LI *malloc_1d_LI(size_t xlen);
double *malloc_1d_double(size_t xlen);
MKL_Complex16 *malloc_1d_dcomplex(size_t xlen);
bool *calloc_1d_bool(size_t xlen);
UI *calloc_1d_UI(size_t xlen);
int *calloc_1d_int(size_t xlen);
USI *calloc_1d_USI(size_t xlen);
SI *calloc_1d_SI(size_t xlen);
ULI *calloc_1d_ULI(size_t xlen);
LI *calloc_1d_LI(size_t xlen);
double *calloc_1d_double(size_t xlen);
MKL_Complex16 *calloc_1d_dcomplex(size_t xlen);
bool **malloc_2d_bool(size_t xlen, size_t ylen);
UI **malloc_2d_UI(size_t xlen, size_t ylen);
int **malloc_2d_int(size_t xlen, size_t ylen);
USI **malloc_2d_USI(size_t xlen, size_t ylen);
SI **malloc_2d_SI(size_t xlen, size_t ylen);
ULI **malloc_2d_ULI(size_t xlen, size_t ylen);
LI **malloc_2d_LI(size_t xlen, size_t ylen);
double **malloc_2d_double(size_t xlen, size_t ylen);
MKL_Complex16 **malloc_2d_dcomplex(size_t xlen, size_t ylen);
bool **calloc_2d_bool(size_t xlen, size_t ylen);
UI **calloc_2d_UI(size_t xlen, size_t ylen);
int **calloc_2d_int(size_t xlen, size_t ylen);
USI **calloc_2d_USI(size_t xlen, size_t ylen);
SI **calloc_2d_SI(size_t xlen, size_t ylen);
ULI **calloc_2d_ULI(size_t xlen, size_t ylen);
LI **calloc_2d_LI(size_t xlen, size_t ylen);
double **calloc_2d_double(size_t xlen, size_t ylen);
MKL_Complex16 **calloc_2d_dcomplex(size_t xlen, size_t ylen);
bool ***malloc_3d_bool(size_t xlen, size_t ylen, size_t zlen);
SI ***malloc_3d_SI(size_t xlen, size_t ylen, size_t zlen);
USI ***malloc_3d_USI(size_t xlen, size_t ylen, size_t zlen);
LI ***malloc_3d_LI(size_t xlen, size_t ylen, size_t zlen);
ULI ***malloc_3d_ULI(size_t xlen, size_t ylen, size_t zlen);
int ***malloc_3d_int(size_t xlen, size_t ylen, size_t zlen);
double  ***malloc_3d_double(size_t xlen, size_t ylen, size_t zlen);
MKL_Complex16 ***malloc_3d_dcomplex(size_t xlen, size_t ylen, size_t zlen);

#endif // MY_MKL_MALLOC_H_
