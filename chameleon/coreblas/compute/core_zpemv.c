/**
 *
 * @file core_zpemv.c
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2021 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 ***
 *
 * @brief Chameleon core_zpemv CPU kernel
 *
 * @version 1.0.0
 * @comment This file has been automatically generated
 *          from Plasma 2.5.0 for CHAMELEON 0.9.2
 * @author Dulceneia Becker
 * @author Mathieu Faverge
 * @author Emmanuel Agullo
 * @author Cedric Castagnede
 * @author Florent Pruvost
 * @date 2020-03-03
 * @precisions normal z -> c d s
 *
 */
#include "coreblas/cblas.h"
#include "coreblas/lapacke.h"
#include "coreblas.h"

/**
 *
 * @ingroup CORE_CHAMELEON_Complex64_t
 *
 *  CORE_zpemv  performs one of the matrix-vector operations
 *
 *     y = alpha*op( A )*x + beta*y
 *
 *  where  op( A ) is one of
 *
 *     op( A ) = A   or   op( A ) = A^T   or   op( A ) = A^H,
 *
 *  alpha and beta are scalars, x and y are vectors and A is a
 *  pentagonal matrix (see further details).
 *
 *  Arguments
 *  ==========
 *
 * @param[in] storev
 *
 *         @arg ChamColumnwise :  array A stored columwise
 *         @arg ChamRowwise    :  array A stored rowwise
 *
 * @param[in] trans
 *
 *         @arg ChamNoTrans   :  y := alpha*A*x    + beta*y.
 *         @arg ChamTrans     :  y := alpha*A^T*x + beta*y.
 *         @arg ChamConjTrans :  y := alpha*A^H*x + beta*y.
 *
 * @param[in] M
 *         Number of rows of the matrix A.
 *         M must be at least zero.
 *
 * @param[in] N
 *         Number of columns of the matrix A.
 *         N must be at least zero.
 *
 * @param[in] L
 *         Order of triangle within the matrix A (L specifies the shape
 *         of the matrix A; see further details).
 *
 * @param[in] ALPHA
 *         Scalar alpha.
 *
 * @param[in] A
 *         Array of size LDA-by-N.  On entry, the leading M by N part
 *         of the array A must contain the matrix of coefficients.
 *
 * @param[in] LDA
 *         Leading dimension of array A.
 *
 * @param[in] X
 *         On entry, the incremented array X must contain the vector x.
 *
 * @param[in] INCX
 *         Increment for the elements of X. INCX must not be zero.
 *
 * @param[in] BETA
 *         Scalar beta.
 *
 * @param[in,out] Y
 *         On entry, the incremented array Y must contain the vector y.
 *
 * @param[out] INCY
 *         Increment for the elements of Y. INCY must not be zero.
 *
 * @param[out] WORK
 *         Workspace array of size at least L.
 *
 *  Further Details
 *  ===============
 *
 *               |     N    |
 *            _   ___________   _
 *               |          |
 *     A:        |          |
 *          M-L  |          |
 *               |          |  M
 *            _  |.....     |
 *               \    :     |
 *            L    \  :     |
 *            _      \:_____|  _
 *
 *               |  L | N-L |
 *
 *******************************************************************************
 *
 * @retval CHAMELEON_SUCCESS successful exit
 * @retval <0 if -i, the i-th argument had an illegal value
 *
 */


int CORE_zpemv(cham_trans_t trans, cham_store_t storev,
               int M, int N, int L,
               CHAMELEON_Complex64_t ALPHA,
               const CHAMELEON_Complex64_t *A, int LDA,
               const CHAMELEON_Complex64_t *X, int INCX,
               CHAMELEON_Complex64_t BETA,
               CHAMELEON_Complex64_t *Y, int INCY,
               CHAMELEON_Complex64_t *WORK)
{

   /*
    *  y = alpha * op(A) * x + beta * y
    */

    int K;
    static CHAMELEON_Complex64_t zzero = 0.0;


    /* Check input arguments */
    if ( !isValidTrans( trans ) ) {
        coreblas_error(1, "Illegal value of trans");
        return -1;
    }
    if ((storev != ChamColumnwise) && (storev != ChamRowwise)) {
        coreblas_error(2, "Illegal value of storev");
        return -2;
    }
    if (!( ((storev == ChamColumnwise) && (trans != ChamNoTrans)) ||
           ((storev == ChamRowwise) && (trans == ChamNoTrans)) )) {
        coreblas_error(2, "Illegal values of trans/storev");
        return -2;
    }
    if (M < 0) {
        coreblas_error(3, "Illegal value of M");
        return -3;
    }
    if (N < 0) {
        coreblas_error(4, "Illegal value of N");
        return -4;
    }
    if (L > chameleon_min(M ,N)) {
        coreblas_error(5, "Illegal value of L");
        return -5;
    }
    if (LDA < chameleon_max(1,M)) {
        coreblas_error(8, "Illegal value of LDA");
        return -8;
    }
    if (INCX < 1) {
        coreblas_error(10, "Illegal value of INCX");
        return -10;
    }
    if (INCY < 1) {
        coreblas_error(13, "Illegal value of INCY");
        return -13;
    }

    /* Quick return */
    if ((M == 0) || (N == 0))
        return CHAMELEON_SUCCESS;
    if ((ALPHA == zzero) && (BETA == zzero))
        return CHAMELEON_SUCCESS;

    /* If L < 2, there is no triangular part */
    if (L == 1) L = 0;

    /* Columnwise */
    if (storev == ChamColumnwise) {
        /*
         *        ______________
         *        |      |     |    A1: A[ 0 ]
         *        |      |     |    A2: A[ M-L ]
         *        |  A1  |     |    A3: A[ (N-L) * LDA ]
         *        |      |     |
         *        |______| A3  |
         *        \      |     |
         *          \ A2 |     |
         *            \  |     |
         *              \|_____|
         *
         */


        /* Columnwise / NoTrans */
        if (trans == ChamNoTrans) {
            coreblas_error(1, "The case ChamNoTrans / ChamColumnwise is not yet implemented");
            return -1;
        }
        /* Columnwise / [Conj]Trans */
        else {

            /* L top rows of y */
            if (L > 0) {

                /* w = A_2' * x_2 */
                cblas_zcopy(
                            L, &X[INCX*(M-L)], INCX, WORK, 1);
                cblas_ztrmv(
                            CblasColMajor, (CBLAS_UPLO)ChamUpper,
                            (CBLAS_TRANSPOSE)trans,
                            (CBLAS_DIAG)ChamNonUnit,
                            L, &A[M-L], LDA, WORK, 1);

                if (M > L) {

                    /* y_1 = beta * y_1 [ + alpha * A_1 * x_1 ] */
                    cblas_zgemv(
                                CblasColMajor, (CBLAS_TRANSPOSE)trans,
                                M-L, L, CBLAS_SADDR(ALPHA), A, LDA,
                                X, INCX, CBLAS_SADDR(BETA), Y, INCY);

                    /* y_1 = y_1 + alpha * w */
                    cblas_zaxpy(L, CBLAS_SADDR(ALPHA), WORK, 1, Y, INCY);

                } else {

                    /* y_1 = y_1 + alpha * w */
                    if (BETA == zzero) {
                        cblas_zscal(L, CBLAS_SADDR(ALPHA), WORK, 1);
                        cblas_zcopy(L, WORK, 1, Y, INCY);
                    } else {
                        cblas_zscal(L, CBLAS_SADDR(BETA), Y, INCY);
                        cblas_zaxpy(L, CBLAS_SADDR(ALPHA), WORK, 1, Y, INCY);
                    }
                }
            }

            /* N-L bottom rows of Y */
            if (N > L) {
                K = N - L;
                cblas_zgemv(
                            CblasColMajor, (CBLAS_TRANSPOSE)trans,
                            M, K, CBLAS_SADDR(ALPHA), &A[LDA*L], LDA,
                            X, INCX, CBLAS_SADDR(BETA), &Y[INCY*L], INCY);
            }
        }
    }
    /* Rowwise */
    else {
        /*
         * --------------
         * |            | \           A1:  A[ 0 ]
         * |    A1      |   \         A2:  A[ (N-L) * LDA ]
         * |            | A2  \       A3:  A[ L ]
         * |--------------------\
         * |        A3          |
         * ----------------------
         *
         */

        /* Rowwise / NoTrans */
        if (trans == ChamNoTrans) {
            /* L top rows of A and y */
            if (L > 0) {

                /* w = A_2 * x_2 */
                cblas_zcopy(
                            L, &X[INCX*(N-L)], INCX, WORK, 1);
                cblas_ztrmv(
                            CblasColMajor, (CBLAS_UPLO)ChamLower,
                            (CBLAS_TRANSPOSE)ChamNoTrans,
                            (CBLAS_DIAG)ChamNonUnit,
                            L, &A[LDA*(N-L)], LDA, WORK, 1);

                if (N > L) {

                    /* y_1 = beta * y_1 [ + alpha * A_1 * x_1 ] */
                    cblas_zgemv(
                                CblasColMajor, (CBLAS_TRANSPOSE)ChamNoTrans,
                                L, N-L, CBLAS_SADDR(ALPHA), A, LDA,
                                X, INCX, CBLAS_SADDR(BETA), Y, INCY);

                    /* y_1 = y_1 + alpha * w */
                    cblas_zaxpy(L, CBLAS_SADDR(ALPHA), WORK, 1, Y, INCY);

                } else {

                    /* y_1 = y_1 + alpha * w */
                    if (BETA == zzero) {
                        cblas_zscal(L, CBLAS_SADDR(ALPHA), WORK, 1);
                        cblas_zcopy(L, WORK, 1, Y, INCY);
                    } else {
                        cblas_zscal(L, CBLAS_SADDR(BETA), Y, INCY);
                        cblas_zaxpy(L, CBLAS_SADDR(ALPHA), WORK, 1, Y, INCY);
                    }
                }
            }

            /* M-L bottom rows of Y */
            if (M > L) {
                cblas_zgemv(
                        CblasColMajor, (CBLAS_TRANSPOSE)ChamNoTrans,
                        M-L, N, CBLAS_SADDR(ALPHA), &A[L], LDA,
                        X, INCX, CBLAS_SADDR(BETA), &Y[INCY*L], INCY);
            }
        }
        /* Rowwise / [Conj]Trans */
        else {
            coreblas_error(1, "The case Cham[Conj]Trans / ChamRowwise is not yet implemented");
            return -1;
        }
    }

    return CHAMELEON_SUCCESS;
}
