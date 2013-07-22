#ifndef SIMD_H
#define SIMD_H
#include "Includes.h"

/* Found in SSE.cpp */
/*----------------------------------------------------------------------------------------------*/

void  sse_conj(void *A, int32 cnt) __attribute__ ((noinline));											//!< Pointwise vector conjugate
void  sse_cmul(CPX *A, CPX *B, int32 cnt) __attribute__ ((noinline));									//!< Pointwise vector multiply
void  sse_cmuls(void *A, void *B, int32 cnt, int32 shift) __attribute__ ((noinline));					//!< Pointwise vector multiply with shift
void  sse_cmulsc(void *A, void *B, void *C, int32 cnt, int32 shift) __attribute__ ((noinline));			//!< Pointwise vector multiply with shift, dump results into C
int64 sse_cacc(void *A, void *B, int cnt) __attribute__ ((noinline));
void sse_mulc(void *A, void *B, void *C, int cnt) __attribute__ ((noinline));
/*----------------------------------------------------------------------------------------------*/
#endif 
