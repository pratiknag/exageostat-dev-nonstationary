/**
 *
 * @file pzgeqrfrh.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2021 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon zgeqrfrh parallel algorithm
 *
 * @version 1.0.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 0.9.2
 * @author Jakub Kurzak
 * @author Hatem Ltaief
 * @author Dulceneia Becker
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @author Florent Pruvost
 * @author Samuel Thibault
 * @date 2020-03-03
 * @precisions normal z -> s d c
 *
 */
#include "control/common.h"

#define A(m,n)  A,  (m),  (n)
#define T(m,n)  T,  (m),  (n)
#define T2(m,n) T,  (m), ((n)+A->nt)
#define D(m,n)  D,  (m),  (n)

/**
 *  Parallel tile QR factorization (reduction Householder) - dynamic scheduling
 */
void chameleon_pzgeqrfrh( int genD, int BS, CHAM_desc_t *A, CHAM_desc_t *T, CHAM_desc_t *D,
                          RUNTIME_sequence_t *sequence, RUNTIME_request_t *request )
{
    CHAM_context_t *chamctxt;
    RUNTIME_option_t options;
    size_t ws_worker = 0;
    size_t ws_host = 0;
    int k, m, n;
    int K, M, RD;
    int tempkmin, tempkn, tempMm, tempnn, tempmm, tempMRDm;
    int ib, node;

    chamctxt = chameleon_context_self();
    if (sequence->status != CHAMELEON_SUCCESS) {
        return;
    }
    RUNTIME_options_init(&options, chamctxt, sequence, request);

    ib = CHAMELEON_IB;

    if ( D == NULL ) {
        D    = A;
        genD = 0;
    }

    /*
     * zgeqrt  = A->nb * (ib+1)
     * zunmqr  = A->nb * ib
     * ztpqrt  = A->nb * (ib+1)
     * ztpmqrt = A->nb * ib
     */
    ws_worker = A->nb * (ib+1);

    /* Allocation of temporary (scratch) working space */
#if defined(CHAMELEON_USE_CUDA)
    /* Worker space
     *
     * zunmqr  =     A->nb * ib
     * ztpmqrt = 3 * A->nb * ib
     */
    ws_worker = chameleon_max( ws_worker, ib * A->nb * 3 );
#endif

    ws_worker *= sizeof(CHAMELEON_Complex64_t);
    ws_host   *= sizeof(CHAMELEON_Complex64_t);

    RUNTIME_options_ws_alloc( &options, ws_worker, ws_host );

    K = chameleon_min(A->mt, A->nt);
    for (k = 0; k < K; k++) {
        RUNTIME_iteration_push(chamctxt, k);

        tempkn = k == A->nt-1 ? A->n-k*A->nb : A->nb;
        for (M = k; M < A->mt; M += BS) {
            tempMm = M == A->mt-1 ? A->m-M*A->mb : A->mb;
            tempkmin = chameleon_min(tempMm, tempkn);

            INSERT_TASK_zgeqrt(
                &options,
                tempMm, tempkn, ib, T->nb,
                A(M, k),
                T(M, k));
            if ( genD ) {
                int tempDMm = M == D->mt-1 ? D->m-M*D->mb : D->mb;
                int tempDkn = k == D->nt-1 ? D->n-k*D->nb : D->nb;

                INSERT_TASK_zlacpy(
                    &options,
                    ChamLower, tempDMm, tempDkn, A->nb,
                    A(M, k),
                    D(M, k) );
#if defined(CHAMELEON_USE_CUDA)
                INSERT_TASK_zlaset(
                    &options,
                    ChamUpper, tempDMm, tempDkn,
                    0., 1.,
                    D(M, k) );
#endif
            }
            for (n = k+1; n < A->nt; n++) {
                tempnn = n == A->nt-1 ? A->n-n*A->nb : A->nb;
                INSERT_TASK_zunmqr(
                    &options,
                    ChamLeft, ChamConjTrans,
                    tempMm, tempnn, tempkmin, ib, T->nb,
                    D(M, k),
                    T(M, k),
                    A(M, n));
            }
            RUNTIME_data_flush( sequence, D(M, k) );
            RUNTIME_data_flush( sequence, T(M, k) );

            for (m = M+1; m < chameleon_min(M+BS, A->mt); m++) {
                tempmm = m == A->mt-1 ? A->m-m*A->mb : A->mb;

                RUNTIME_data_migrate( sequence, A(M, k),
                                      A->get_rankof( A, m, k ) );

                /* TS kernel */
                INSERT_TASK_ztpqrt(
                    &options,
                    tempmm, tempkn, 0, ib, T->nb,
                    A(M, k),
                    A(m, k),
                    T(m, k));

                for (n = k+1; n < A->nt; n++) {
                    tempnn = n == A->nt-1 ? A->n-n*A->nb : A->nb;

                    RUNTIME_data_migrate( sequence, A(M, n),
                                          A->get_rankof( A, m, n ) );

                    INSERT_TASK_ztpmqrt(
                        &options,
                        ChamLeft, ChamConjTrans,
                        tempmm, tempnn, A->nb, 0, ib, T->nb,
                        A(m, k),
                        T(m, k),
                        A(M, n),
                        A(m, n));
                }
                RUNTIME_data_flush( sequence, A(m, k) );
                RUNTIME_data_flush( sequence, T(m, k) );
            }
        }
        for (RD = BS; RD < A->mt-k; RD *= 2) {
            for (M = k; M+RD < A->mt; M += 2*RD) {
                tempMRDm = M+RD == A->mt-1 ? A->m-(M+RD)*A->mb : A->mb;

                node = A->get_rankof( A, M+RD, k );
                RUNTIME_data_migrate( sequence, A(M, k),    node );
                RUNTIME_data_migrate( sequence, A(M+RD, k), node );

                /* TT kernel */
                INSERT_TASK_ztpqrt(
                    &options,
                    tempMRDm, tempkn, chameleon_min( tempMRDm, tempkn ), ib, T->nb,
                    A (M   , k),
                    A (M+RD, k),
                    T2(M+RD, k));

                for (n = k+1; n < A->nt; n++) {
                    tempnn = n == A->nt-1 ? A->n-n*A->nb : A->nb;

                    node = A->get_rankof( A, M+RD, n );
                    RUNTIME_data_migrate( sequence, A(M, n),    node );
                    RUNTIME_data_migrate( sequence, A(M+RD, n), node );

                    INSERT_TASK_ztpmqrt(
                        &options,
                        ChamLeft, ChamConjTrans,
                        tempMRDm, tempnn, A->nb, tempMRDm, ib, T->nb,
                        A (M+RD, k),
                        T2(M+RD, k),
                        A (M,    n),
                        A (M+RD, n));
                }
                RUNTIME_data_flush( sequence, A (M+RD, k) );
                RUNTIME_data_flush( sequence, T2(M+RD, k) );
            }
        }

        /* Restore the original location of the tiles */
        for (n = k; n < A->nt; n++) {
            RUNTIME_data_migrate( sequence, A(k, n),
                                  A->get_rankof( A, k, n ) );
        }

        RUNTIME_iteration_pop(chamctxt);
    }

    RUNTIME_options_ws_free(&options);
    RUNTIME_options_finalize(&options, chamctxt);
    (void)D;
}
