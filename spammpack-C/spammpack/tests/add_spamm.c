#include "spamm.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_TOLERANCE 1e-10

#define PRINT_MATRIX

int
main (int argc, char **argv)
{
  int result = 1;

  unsigned int N = 20;

  double alpha = 1.2;
  double beta = 0.8;

  float *A_dense;
  float *B_dense;

  float max_diff;

  struct spamm_hashed_t *A;
  struct spamm_hashed_t *B;

  unsigned int i, j;

  A_dense = calloc(N*N, sizeof(double));
  B_dense = calloc(N*N, sizeof(double));

  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++)
    {
      A_dense[spamm_index_row_major(i, j, N, N)] = rand()/(double) RAND_MAX;
      B_dense[spamm_index_row_major(i, j, N, N)] = rand()/(double) RAND_MAX;
    }
  }

#ifdef PRINT_MATRIX
  printf("A_dense =\n");
  spamm_print_dense(N, N, row_major, A_dense);

  printf("B_dense =\n");
  spamm_print_dense(N, N, row_major, B_dense);
#endif

  /* Convert to SpAMM matrix. */
  A = spamm_convert_dense_to_spamm(N, N, row_major, A_dense, row_major);
  B = spamm_convert_dense_to_spamm(N, N, row_major, B_dense, row_major);

  /* Add by hand for verification. */
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++)
    {
      A_dense[spamm_index_row_major(i, j, N, N)] =
        alpha*A_dense[spamm_index_row_major(i, j, N, N)]
        + beta*B_dense[spamm_index_row_major(i, j, N, N)];
    }
  }

#ifdef PRINT_MATRIX
  printf("A_dense =\n");
  spamm_print_dense(N, N, row_major, A_dense);
#endif

  spamm_hashed_add(alpha, A, beta, B);

#ifdef PRINT_MATRIX
  printf("A =\n");
  spamm_print(A);
#endif

  /* Compare result. */
  max_diff = 0.0;
  for (i = 0; i < N; i++) {
    for (j = 0; j < N; j++)
    {
      if (fabs(A_dense[spamm_index_row_major(i, j, N, N)]-spamm_get(i, j, A)) > max_diff)
      {
        max_diff = fabs(A_dense[spamm_index_row_major(i, j, N, N)]-spamm_get(i, j, A));
      }
    }
  }
  printf("[add_spamm] max diff = %e\n", max_diff);

  if (max_diff < TEST_TOLERANCE)
  {
    result = 0;
  }

  free(A_dense);
  free(B_dense);

  spamm_hashed_delete(&A);
  spamm_hashed_delete(&B);

  return result;
}
