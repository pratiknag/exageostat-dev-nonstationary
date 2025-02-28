/**
 * @copyright (c) 2017-2022 King Abdullah University of Science and Technology (KAUST).
 *                     All rights reserved.
 **/

/**
 * @file timing.h
 *
 * This file contains the macros used for timing experiments.
 *
 * HiCMA is a software package provided by King Abdullah University of Science and Technology (KAUST)
 *
 * @version 1.0.0
 * @author Kadir Akbudak
 * @date 2018-11-08
 **/

/*
 * @copyright (c) 2009-2014 The University of Tennessee and The University
 *                          of Tennessee Research Foundation.
 *                          All rights reserved.
 * @copyright (c) 2012-2016 Inria. All rights reserved.
 * @copyright (c) 2012-2014 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria, Univ. Bordeaux. All rights reserved.
 */

#ifndef TIMING_H
#define TIMING_H

#include <hicma.h>
#include "hicma_common.h"

#define _TYPE  double
#define _PREC  double
#define _LAMCH LAPACKE_dlamch_work
/* See Lawn 41 page 120 */
#define _FMULS 0 //FMULS_GEMM(M, N, K) //FIXME
#define _FADDS 0 //FADDS_GEMM(M, N, K) //FIXME
#define _NAME  "HiCMA_zgemm_Tile"




typedef double hicma_time_t;

#include <stdio.h>
void print_array(int m, int n, int ld, double* arr, FILE* fp);
void fwrite_array(int m, int n, int ld, double* arr, char* file);
int RunTest(int *iparam, double *dparam, hicma_time_t *t_, char* rankfile);
void* hicma_getaddr_null(const HICMA_desc_t *A, int m, int n);

enum iparam_timing {
    IPARAM_THRDNBR,        /* Number of cores                            */
    IPARAM_THRDNBR_SUBGRP, /* Number of cores in a subgroup (NUMA node)  */
    IPARAM_SCHEDULER,      /* What scheduler do we choose (dyn, stat)    */
    IPARAM_M,              /* Number of rows of the matrix               */
    IPARAM_N,              /* Number of columns of the matrix            */
    IPARAM_K,              /* RHS or K                                   */
    IPARAM_LDA,            /* Leading dimension of A                     */
    IPARAM_LDB,            /* Leading dimension of B                     */
    IPARAM_LDC,            /* Leading dimension of C                     */
    IPARAM_IB,             /* Inner-blocking size                        */
    IPARAM_NB,             /* Number of columns in a tile                */
    IPARAM_MB,             /* Number of rows in a tile                   */
    IPARAM_NITER,          /* Number of iteration of each test           */
    IPARAM_WARMUP,         /* Run one test to load dynamic libraries     */
    IPARAM_BIGMAT,         /* Allocating one big mat or plenty of small  */
    IPARAM_CHECK,          /* Checking activated or not                  */
    IPARAM_VERBOSE,        /* How much noise do we want?                 */
    IPARAM_AUTOTUNING,     /* Disable/enable autotuning                  */
    IPARAM_INPUTFMT,       /* Input format (Use only for getmi/gecfi)    */
    IPARAM_OUTPUTFMT,      /* Output format (Use only for getmi/gecfi)   */
    IPARAM_TRACE,          /* Generate trace on the first non warmup run */
    IPARAM_DAG,            /* Do we require to output the DOT file?      */
    IPARAM_ASYNC,          /* Asynchronous calls                         */
    IPARAM_MX,             /* */
    IPARAM_NX,             /* */
    IPARAM_RHBLK,          /* Householder reduction parameter for QR/LQ  */
    IPARAM_INPLACE,        /* InPlace/OutOfPlace translation mode        */
    IPARAM_MODE,           /* Eigenvalue generation mode                 */

    IPARAM_INVERSE,
    IPARAM_NCUDAS,
    IPARAM_NMPI,
    IPARAM_P,              /* Parameter for 2D cyclic distribution       */
    IPARAM_Q,              /* Parameter for 2D cyclic distribution       */

    IPARAM_PROGRESS,       /* Use a progress indicator during computations */
    IPARAM_GEMM3M,         /* Use GEMM3M for complex matrix vector products */
    /* Added for StarPU version */
    IPARAM_PROFILE,
    IPARAM_PRINT_WARNINGS,
    IPARAM_PEAK,
    IPARAM_PARALLEL_TASKS,
    IPARAM_NO_CPU,
    IPARAM_BOUND,
    IPARAM_BOUNDDEPS,
    IPARAM_BOUNDDEPSPRIO,
    IPARAM_RK,
    IPARAM_HICMA_MAXRANK,
    IPARAM_HICMA_STARSH_PROB,
    IPARAM_HICMA_STARSH_MAXRANK,
    IPARAM_HICMA_PRINTMAT,
    IPARAM_HICMA_PRINTINDEX,
    IPARAM_HICMA_PRINTINDEXEND,
    IPARAM_HICMA_ALWAYS_FIXED_RANK,
    IPARAM_HICMA_REORDER_INNER_PRODUCTS,
    IPARAM_ORDER, // 0: no ordering, 1: Morton ordering
    IPARAM_RBFKERNEL, // RBF kernel_type
    IPARAM_NUMOBJ, // how many objects (e.g. total number of viruses)
    IPARAM_NUMSUBOBJ, // how many subobjects (e.g. number of subviruses within one batch)
    IPARAM_HICMA_NTRIAN,  /* @noha Please add a short description */
    IPARAM_HICMA_NIPP,  /* @noha Please add a short description */
    IPARAM_HICMA_PERCENT1,  
    IPARAM_HICMA_PERCENT2,  
    IPARAM_HICMA_ISPERCENT,
    IPARAM_HICMA_SOLVE,
    IPARAM_SOLVE,  /* Cholesky solver*/
    IPARAM_CSOLVE,  /* Check Cholesky solver*/

    /* End */
    IPARAM_SIZEOF
};

enum dparam_timing {
  IPARAM_TIME,
  IPARAM_ANORM,
  IPARAM_BNORM,
  IPARAM_XNORM,
  IPARAM_RNORM,

   //Infinity Norm for checking the solution

  IPARAM_IANORM,
  IPARAM_IBNORM,
  IPARAM_IXNORM,
  IPARAM_IRNORM,
  IPARAM_IRES,

  IPARAM_AinvNORM,
  IPARAM_ESTIMATED_PEAK,
  IPARAM_RES,
  /* Begin section for hydra integration tool */
  IPARAM_THRESHOLD_CHECK, /* Maximum value accepted for: |Ax-b||/N/eps/(||A||||x||+||b||) */
  IPARAM_HICMA_STARSH_DECAY,
  IPARAM_HICMA_STARSH_WAVE_K,
  IPARAM_HICMA_ACCURACY_THRESHOLD,
  /* End section for hydra integration tool  */
  IPARAM_DNBPARAM
};

#define PASTE_CODE_IPARAM_LOCALS(iparam)           \
    int64_t thrdnbr = iparam[IPARAM_THRDNBR];              \
    double  t;                                     \
    int64_t M     = iparam[IPARAM_M];              \
    int64_t N     = iparam[IPARAM_N];              \
    int64_t K     = iparam[IPARAM_K];              \
    int64_t NRHS  = K;                             \
    int64_t LDA   = hicma_max(M, iparam[IPARAM_LDA]);    \
    int64_t LDB   = hicma_max(N, iparam[IPARAM_LDB]);    \
    int64_t LDC   = hicma_max(K, iparam[IPARAM_LDC]);    \
    int64_t IB    = iparam[IPARAM_IB];             \
    int64_t MB    = iparam[IPARAM_MB];             \
    int64_t NB    = iparam[IPARAM_NB];             \
    int64_t P     = iparam[IPARAM_P];              \
    int64_t Q     = iparam[IPARAM_Q];              \
    int64_t MT    = (M%MB==0) ? (M/MB) : (M/MB+1); \
    int64_t NT    = (N%NB==0) ? (N/NB) : (N/NB+1); \
    int bigmat     = iparam[IPARAM_BIGMAT];         \
    int check     = iparam[IPARAM_CHECK];          \
    int check_solve     = iparam[IPARAM_CSOLVE];          \
    int loud      = iparam[IPARAM_VERBOSE];        \
    int nip      = iparam[IPARAM_HICMA_NIPP];     \
    int ntrian      = iparam[IPARAM_HICMA_NTRIAN];    \
    int solve	    =iparam[IPARAM_HICMA_SOLVE];   \
    (void)M;(void)N;(void)K;(void)NRHS;            \
    (void)LDA;(void)LDB;(void)LDC;                 \
    (void)IB;(void)MB;(void)NB;(void)P;(void)Q;    \
    (void)MT;(void)NT;(void)check;(void)loud;(void)bigmat;

/* Paste code to allocate a matrix in desc if cond_init is true */
#define PASTE_CODE_ALLOCATE_MATRIX_TILE(_desc_, _cond_, _type_, _type2_, _lda_, _m_, _n_) \
    HICMA_desc_t *_desc_ = NULL;                                        \
    int status ## _desc_ ; \
    if( _cond_ ) {                                                      \
       if (!bigmat){ \
           status ## _desc_ = HICMA_Desc_Create_User(&(_desc_), NULL, _type2_, MB, NB, MB*NB, _lda_, _n_, 0, 0, _m_, _n_, \
                          P, Q, hicma_getaddr_null, NULL, NULL);\
       }\
        else {\
           status ## _desc_ = HICMA_Desc_Create(&(_desc_), NULL, _type2_, MB, NB, MB*NB, _lda_, _n_, 0, 0, _m_, _n_, \
                    P, Q);\
        }\
        if (status ## _desc_ != HICMA_SUCCESS) return (status ## _desc_);          \
    }

#define PASTE_CODE_FREE_MATRIX(_desc_)                                  \
    HICMA_Desc_Destroy( &_desc_ );

#define PASTE_TILE_TO_LAPACK(_desc_, _name_, _cond_, _type_, _lda_, _n_) \
    _type_ *_name_ = NULL;                                               \
    if ( _cond_ ) {                                                      \
        _name_ = (_type_*)malloc( (_lda_) * (_n_) * sizeof(_type_));     \
        if ( ! _name_ ) {                                                \
            fprintf(stderr, "Out of Memory for %s\n", #_name_);          \
            return -1;                                                   \
        }                                                                \
        HICMA_Tile_to_Lapack(_desc_, (void*)_name_, _lda_);              \
    }

#define PASTE_CODE_ALLOCATE_MATRIX(_name_, _cond_, _type_, _lda_, _n_)  \
    _type_ *_name_ = NULL;                                              \
    if( _cond_ ) {                                                      \
        _name_ = (_type_*)malloc( (_lda_) * (_n_) * sizeof(_type_) );   \
        if ( ! _name_ ) {                                               \
            fprintf(stderr, "Out of Memory for %s\n", #_name_);         \
            return -1;                                                  \
        }                                                               \
    }

#define PASTE_CODE_ALLOCATE_COPY(_name_, _cond_, _type_, _orig_, _lda_, _n_) \
    _type_ *_name_ = NULL;                                                   \
    if( _cond_ ) {                                                           \
        _name_ = (_type_*)malloc( (_lda_) * (_n_) * sizeof(_type_) );        \
        if ( ! _name_ ) {                                                    \
            fprintf(stderr, "Out of Memory for %s\n", #_name_);              \
            return -1;                                                       \
        }                                                                    \
        memcpy(_name_, _orig_, (_lda_) * (_n_) * sizeof(_type_) );           \
    }

/*********************
 *
 * Macro for trace generation
 *
 */
#define START_TRACING()                        \
    HICMA_RUNTIME_start_stats();                     \
    if(iparam[IPARAM_TRACE] == 2) {            \
    	HICMA_RUNTIME_start_profiling();             \
    }                                          \
    if(iparam[IPARAM_BOUND]) {                 \
        HICMA_Enable(HICMA_BOUND);             \
    }

#define STOP_TRACING()                         \
    HICMA_RUNTIME_stop_stats();                      \
    if(iparam[IPARAM_TRACE] == 2) {            \
    	HICMA_RUNTIME_stop_profiling();              \
    }                                          \
    if(iparam[IPARAM_BOUND]) {                 \
        HICMA_Disable(HICMA_BOUND);            \
    }

/*********************
 *
 * Macro for DAG generation
 *
 */
#if 0
#define START_DAG()                   \
    if ( iparam[IPARAM_DAG] == 2 )    \
        HICMA_Enable(HICMA_DAG);

#define STOP_DAG()                    \
    if ( iparam[IPARAM_DAG] == 2 )    \
        HICMA_Disable(HICMA_DAG);
#else
#define START_DAG()  do {} while(0);
#define STOP_DAG()   do {} while(0);
#endif

/*********************
 *
 * Synchro for distributed computations
 *
 */
#if defined(HICMA_USE_MPI)
#define START_DISTRIBUTED()  HICMA_Distributed_start();
#define STOP_DISTRIBUTED()   HICMA_Distributed_stop();
#else
#define START_DISTRIBUTED()  do {} while(0);
#define STOP_DISTRIBUTED()   do {} while(0);
#endif

/*********************
 *
 * General Macros for timing
 *
 */
#define START_TIMING()                \
  t = -HICMA_RUNTIME_get_time();            \
  START_DAG();                        \
  START_TRACING();                    \
  START_DISTRIBUTED();

#define STOP_TIMING()                 \
  t += HICMA_RUNTIME_get_time();            \
  if (iparam[IPARAM_PROFILE] == 2) {  \
    HICMA_RUNTIME_kernelprofile_display();  \
    HICMA_RUNTIME_schedprofile_display();   \
  }                                   \
  *t_ = t;                            \
  STOP_DISTRIBUTED();                 \
  STOP_TRACING();                     \
  STOP_DAG();

#endif /* TIMING_H */
