

/** A recursive implementation of sgemm(). This function is not feature complete,
 * it hardly does anything.
 */
void spamm_sgemm (char * transA, char * transB,
    int *M, int *N, int *K,
    float *alpha, float *A, int *LDA, float *B, int *LDB,
    float *beta, float *C, int *LDC)
{
  int i, j, k;

  for (i = 0; i < *M; i++) {
    for (j = 0; j < *N; j++) {
      for (k = 0; k < *K; k++)
      {
        C[spamm_index_column_major(i, j, *M, *N)] = (*beta)*C[spamm_index_column_major(i, j, *M, *N)]
          +(*alpha)*A[spamm_index_column_major(i, k, *M, *K)]*B[spamm_index_column_major(k, j, *K, *N)];
      }
    }
  }
}