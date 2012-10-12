#include "spamm.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_TOLERANCE 5e-7

//#define PRINT_MATRIX

int
main (int argc, char **argv)
{
  int result;

  const unsigned int N[] = { 513, 513 };
  const unsigned int linear_tier = 3;
  const unsigned int contiguous_tier = 4;

  double alpha = 1.2;
  double beta = 0.8;

  float *A_dense;
  float *B_dense;

  float max_diff;

  struct spamm_matrix_t *A;
  struct spamm_matrix_t *B;

  unsigned int i[2];

  A_dense = calloc(N[0]*N[1], sizeof(double));
  B_dense = calloc(N[0]*N[1], sizeof(double));

  for (i[0] = 0; i[0] < N[0]; i[0]++) {
    for (i[1] = 0; i[1] < N[1]; i[1]++)
    {
      A_dense[spamm_index_row_major(i[0], i[1], N[0], N[1])] = rand()/(double) RAND_MAX;
      B_dense[spamm_index_row_major(i[0], i[1], N[0], N[1])] = rand()/(double) RAND_MAX;
    }
  }

#ifdef PRINT_MATRIX
  printf("A_dense =\n");
  spamm_print_dense(N, N, row_major, A_dense);

  printf("B_dense =\n");
  spamm_print_dense(N, N, row_major, B_dense);
#endif

  /* Convert to SpAMM matrix. */
  A = spamm_convert_dense_to_spamm(2, N, linear_tier, contiguous_tier, row_major, A_dense, row_major);
  B = spamm_convert_dense_to_spamm(2, N, linear_tier, contiguous_tier, row_major, B_dense, row_major);

  /* Add by hand for verification. */
  for (i[0] = 0; i[0] < N[0]; i[0]++) {
    for (i[1] = 0; i[1] < N[1]; i[1]++)
    {
      A_dense[spamm_index_row_major(i[0], i[1], N[0], N[1])] =
        alpha*A_dense[spamm_index_row_major(i[0], i[1], N[0], N[1])]
        + beta*B_dense[spamm_index_row_major(i[0], i[1], N[0], N[1])];
    }
  }

#ifdef PRINT_MATRIX
  printf("A_dense =\n");
  spamm_print_dense(N, N, row_major, A_dense);
#endif

  spamm_add(alpha, A, beta, B);

#ifdef PRINT_MATRIX
  printf("A =\n");
  spamm_print(A);
#endif

  /* Check tree consistency. */
  result |= spamm_check(A, TEST_TOLERANCE);

  /* Compare result. */
  max_diff = 0.0;
  for (i[0] = 0; i[0] < N[0]; i[0]++) {
    for (i[1] = 0; i[1] < N[1]; i[1]++)
    {
      if (fabs(A_dense[spamm_index_row_major(i[0], i[1], N[0], N[1])]-spamm_get(i, A)) > max_diff)
      {
        max_diff = fabs(A_dense[spamm_index_row_major(i[0], i[1], N[0], N[1])]-spamm_get(i, A));
      }
    }
  }

  if (max_diff > TEST_TOLERANCE)
  {
    result |= SPAMM_ERROR;
    printf("[add_spamm] max diff (test tolerance was %1.2e) = %e\n", TEST_TOLERANCE, max_diff);
  }

  free(A_dense);
  free(B_dense);

  spamm_delete(&A);
  spamm_delete(&B);

  return result;
}
