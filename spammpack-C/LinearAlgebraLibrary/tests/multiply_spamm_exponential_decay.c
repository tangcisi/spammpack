#include <spamm.h>
#include <math.h>
#include <stdlib.h>

int
main ()
{
  struct spamm_t A;
  struct spamm_t B;
  struct spamm_t C;

  floating_point_t *A_dense;
  floating_point_t *B_dense;
  floating_point_t *C_dense;

  int i, j, k;

  floating_point_t tolerance = 1e-3;

  floating_point_t alpha = 1.2;
  floating_point_t beta = 0.5;

  unsigned int N = 500;

  double decayconstant = 3;

  double max_diff;

  unsigned int number_products;

#ifdef TEST_DEBUG
  spamm_set_loglevel(debug);
#endif

#ifdef TEST_DEBUG
  printf("alpha = %f, beta = %f, tolerance = %f\n", alpha, beta, tolerance);
  printf("C = (%f)*A*B + (%f)*C\n", alpha, beta);
#endif

  /* Allocate memory. */
  spamm_new(N, N, &A);
  spamm_new(N, N, &B);
  spamm_new(N, N, &C);

  A_dense = (floating_point_t*) malloc(sizeof(floating_point_t)*N*N);
  B_dense = (floating_point_t*) malloc(sizeof(floating_point_t)*N*N);
  C_dense = (floating_point_t*) malloc(sizeof(floating_point_t)*N*N);

  /* Fill matrices with random data. */
  for (i = 0; i < N; ++i)
  {
    A_dense[spamm_dense_index(i, i, N, N)] = rand()/(double) RAND_MAX;
    for (j = 0; j < N; ++j)
    {
      if (j != i)
      {
        A_dense[spamm_dense_index(i, j, N, N)] = rand()/(double) RAND_MAX * exp(-fabs(i-j)/decayconstant)*A_dense[spamm_dense_index(i, i, N, N)];
      }
    }
  }

  for (i = 0; i < N; ++i)
  {
    B_dense[spamm_dense_index(i, i, N, N)] = rand()/(double) RAND_MAX;
    for (j = 0; j < N; ++j)
    {
      if (j != i)
      {
        B_dense[spamm_dense_index(i, j, N, N)] = rand()/(double) RAND_MAX * exp(-fabs(i-j)/decayconstant)*B_dense[spamm_dense_index(i, i, N, N)];
      }
    }
  }

  for (i = 0; i < N; ++i)
  {
    C_dense[spamm_dense_index(i, i, N, N)] = rand()/(double) RAND_MAX;
    for (j = 0; j < N; ++j)
    {
      if (j != i)
      {
        C_dense[spamm_dense_index(i, j, N, N)] = rand()/(double) RAND_MAX * exp(-fabs(i-j)/decayconstant)*C_dense[spamm_dense_index(i, i, N, N)];
      }
    }
  }

#ifdef TEST_DEBUG
  printf("A:\n");
  spamm_print_dense(N, N, A_dense);
  printf("B:\n");
  spamm_print_dense(N, N, B_dense);
  printf("C:\n");
  spamm_print_dense(N, N, C_dense);
#endif

  /* Convert matrices. */
  spamm_dense_to_spamm(N, N, A_dense, &A);
  spamm_dense_to_spamm(N, N, B_dense, &B);
  spamm_dense_to_spamm(N, N, C_dense, &C);

#ifdef TEST_DEBUG
  printf("A (SpAMM):\n");
  spamm_print_spamm(&A);
  printf("B (SpAMM):\n");
  spamm_print_spamm(&B);
  printf("C (SpAMM):\n");
  spamm_print_spamm(&C);
#endif

  /* Multiply. */
  for (i = 0; i < N; ++i) {
    for (j = 0; j < N; ++j) {
      C_dense[spamm_dense_index(i, j, N, N)] *= beta;
      for (k = 0; k < N; ++k)
      {
        C_dense[spamm_dense_index(i, j, N, N)] += alpha*A_dense[spamm_dense_index(i, k, N, N)]*B_dense[spamm_dense_index(k, j, N, N)];
      }
    }
  }

#ifdef TEST_DEBUG
  printf("C:\n");
  spamm_print_dense(N, N, C_dense);
#endif

  number_products = spamm_multiply(tree, tolerance, alpha, &A, &B, beta, &C);

#ifdef TEST_DEBUG
  printf("C (SpAMM):\n");
  spamm_print_spamm(&C);
#endif

  /* Compare. */
  max_diff = 0;
  for (i = 0; i < N; ++i) {
    for (j = 0; j < N; ++j)
    {
      if (fabs(C_dense[spamm_dense_index(i, j, N, N)]-spamm_get(i, j, &C)) > max_diff)
      {
        max_diff = fabs(C_dense[spamm_dense_index(i, j, N, N)]-spamm_get(i, j, &C));
      }
    }
  }

  if (max_diff > tolerance)
  {
    LOG_FATAL("mismatch: %ux%u matrix, padded to %ux%u, calculated %u out of %u products (%1.2f%%), max_diff = %e, tolerance = %e\n",
        N, N, A.N_padded, A.N_padded, number_products,
        (double) number_products/(double) ((A.N_padded/SPAMM_N_BLOCK)*(B.N_padded/SPAMM_N_BLOCK)*(C.N_padded/SPAMM_N_BLOCK))*100,
        (A.N_padded/SPAMM_N_BLOCK)*(B.N_padded/SPAMM_N_BLOCK)*(C.N_padded/SPAMM_N_BLOCK), max_diff, tolerance);
    exit(1);
  }

  /* Free memory. */
  free(A_dense);
  free(B_dense);
  free(C_dense);
  spamm_delete(&A);
  spamm_delete(&B);
  spamm_delete(&C);

  return 0;
}
