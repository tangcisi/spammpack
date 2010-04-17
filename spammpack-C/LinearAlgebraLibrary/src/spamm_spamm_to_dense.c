#include "spamm.h"
#include <assert.h>
#include <stdlib.h>

void
spamm_spamm_to_dense (const struct spamm_t *A, double **A_dense)
{
  assert(A != NULL);
  assert(A_dense != NULL);

  int i, j;

  *A_dense = (double*) malloc(sizeof(double)*A->M*A->N);

  for (i = 0; i < A->M; ++i) {
    for (j = 0; j < A->N; ++j)
    {
      (*A_dense)[spamm_dense_index(i, j, A->N)] = spamm_get(i, j, A);
    }
  }
}
