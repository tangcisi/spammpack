/** @file */

#include "spamm.h"
#include "spamm_types_private.h"

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/** Add two SpAMM chunks. @f$ A \leftarrow \alpha A + \beta B @f$.
 *
 * @param alpha The factor @f$ \alpha @f$.
 * @param A Chunk A.
 * @param beta The factor @f$ \beta @f$.
 * @param B Chunk B.
 * @param flop The flop count.
 */
void
spamm_chunk_add (const float alpha,
    spamm_chunk_t **A,
    const float beta,
    spamm_chunk_t *B,
    double *const flop)
{
  unsigned int *number_dimensions;
  unsigned int *number_tiers;

  spamm_norm_t *norm_A;
  spamm_norm_t *norm_B;
  spamm_norm_t *norm2_A;
  spamm_norm_t *norm2_B;

  float *A_matrix;
  float *B_matrix;

  float *A_matrix_dilated;

  unsigned int N_contiguous;

  unsigned int i;
  unsigned int i_norm;

  unsigned int tier;

  number_dimensions = spamm_chunk_get_number_dimensions(B);
  number_tiers = spamm_chunk_get_number_tiers(B);

  norm_A = spamm_chunk_get_norm(*A);
  norm_B = spamm_chunk_get_norm(B);
  norm2_A = spamm_chunk_get_norm2(*A);
  norm2_B = spamm_chunk_get_norm2(B);

  /* Update norms. */
  for(tier = 0, i_norm = 0; tier < *number_tiers; tier++)
  {
    for(i = 0; i < ipow(ipow(2, *number_dimensions), tier); i++)
    {
      norm2_A[i_norm] = alpha*alpha*norm2_A[i_norm]+beta*beta*norm2_B[i_norm]+2*alpha*beta*norm_A[i_norm]*norm_B[i_norm];
      norm_A[i_norm] = sqrt(norm2_A[i_norm]);
      i_norm++;
    }
  }

  A_matrix = spamm_chunk_get_matrix(*A);
  B_matrix = spamm_chunk_get_matrix(B);

  A_matrix_dilated = spamm_chunk_get_matrix_dilated(*A);

  N_contiguous = spamm_chunk_get_N_contiguous(B);

  for(i = 0; i < ipow(N_contiguous, *number_dimensions); i++)
  {
    A_matrix[i] = alpha*A_matrix[i]+beta*B_matrix[i];

    A_matrix_dilated[4*i+0] = A_matrix[i];
    A_matrix_dilated[4*i+1] = A_matrix[i];
    A_matrix_dilated[4*i+2] = A_matrix[i];
    A_matrix_dilated[4*i+3] = A_matrix[i];
  }

  /* Update flop count. */
  *flop += ipow(N_contiguous, *number_dimensions);
}

/** Add two spamm matrices. @f$ A \leftarrow \alpha A + \beta B @f$.
 *
 * @param alpha The factor @f$ \alpha @f$.
 * @param A The matrix A.
 * @param beta The factor @f$ \beta @f$.
 * @param B The matrix B.
 * @param number_dimensions The number of dimensions.
 * @param tier The tier.
 * @param chunk_tier The chunk tier.
 * @param use_linear_tree Are we using a linear tree?
 * @param flop The flop count.
 */
void
spamm_recursive_add (const float alpha,
    struct spamm_recursive_node_t *A,
    const float beta,
    const struct spamm_recursive_node_t *const B,
    const unsigned int number_dimensions,
    const unsigned int tier,
    const unsigned int chunk_tier,
    const short use_linear_tree,
    double *const flop)
{
  unsigned int i;

  /* We need access to a lock on A, which is why A can not be NULL. We need to
   * allocate a new node if we need to a tier above. */
  assert(A != NULL);

  if(tier == chunk_tier)
  {
    if(A->tree.chunk == NULL)
    {
      if(B == NULL || B->tree.chunk == NULL)
      {
        return;
      }

#ifdef _OPENMP
      omp_set_lock(&A->lock);
#endif

      spamm_chunk_copy(&A->tree.chunk, beta, B->tree.chunk, use_linear_tree);

#ifdef _OPENMP
      omp_unset_lock(&A->lock);
#endif
    }

    else if(B == NULL || B->tree.chunk == NULL)
    {
#ifdef _OPENMP
      omp_set_lock(&A->lock);
#endif

      A->norm2 = spamm_chunk_multiply_scalar(alpha, A->tree.chunk, flop);
      A->norm = sqrt(A->norm2);

#ifdef _OPENMP
      omp_unset_lock(&A->lock);
#endif
    }

    else
    {
#ifdef _OPENMP
      omp_set_lock(&A->lock);
#endif

      spamm_chunk_add(alpha, &A->tree.chunk, beta, B->tree.chunk, flop);

      A->norm2 = alpha*alpha*A->norm2+beta*beta*B->norm2+2*alpha*beta*A->norm*B->norm;
      A->norm = sqrt(A->norm2);

#ifdef _OPENMP
      omp_unset_lock(&A->lock);
#endif
    }
  }

  else /* if(tier == chunk_tier) */
  {
    if(A->tree.child == NULL && (B != NULL && B->tree.child != NULL))
    {
      /* Copy B node to A. */
      spamm_recursive_copy(A, beta, B, number_dimensions, tier,
          chunk_tier, use_linear_tree);
    }

    else if(A->tree.child != NULL && B == NULL)
    {
      /* Multiply A by alpha. */
      spamm_recursive_multiply_scalar(alpha, A, number_dimensions, tier,
          chunk_tier, use_linear_tree, flop);
    }

    else
    {
      if(B == NULL || B->tree.child == NULL)
      {
        return;
      }

#ifdef _OPENMP
      omp_set_lock(&A->lock);
#endif

      if(A->tree.child == NULL)
      {
        A->tree.child = calloc(ipow(2, number_dimensions), sizeof(struct spamm_recursive_node_t*));
      }

#ifdef _OPENMP
      omp_unset_lock(&A->lock);
#endif

      for(i = 0; i < ipow(2, number_dimensions); i++)
      {
#ifdef _OPENMP
        omp_set_lock(&A->lock);
#endif

        if(A->tree.child[i] == NULL)
        {
          A->tree.child[i] = spamm_recursive_new_node();
        }

#ifdef _OPENMP
        omp_unset_lock(&A->lock);
#endif

#pragma omp task untied
        spamm_recursive_add(alpha, A->tree.child[i], beta,
            (const struct spamm_recursive_node_t*const) B->tree.child[i],
            number_dimensions, tier+1, chunk_tier, use_linear_tree, flop);
      }
#pragma omp taskwait

      A->norm2 = alpha*alpha*A->norm2+beta*beta*B->norm2+2*alpha*beta*A->norm*B->norm;
      A->norm = sqrt(A->norm2);

#ifdef _OPENMP
      omp_unset_lock(&A->lock);
#endif
    }
  }
}

/** Add two spamm matrices. @f$ A \leftarrow \alpha A + \beta B @f$.
 *
 * @param alpha The factor @f$ \alpha @f$.
 * @param A The matrix A.
 * @param beta The factor @f$ \beta @f$.
 * @param B The matrix B.
 * @param flop The flop count.
 */
void
spamm_add (const float alpha,
    struct spamm_matrix_t *const A,
    const float beta,
    const struct spamm_matrix_t *const B,
    double *const flop)
{
  struct spamm_recursive_node_t *B_pointer;

  assert(A != NULL);

  B_pointer = NULL;
  if(B != NULL)
  {
    B_pointer = B->recursive_tree;
  }

  if(A->recursive_tree == NULL)
  {
    A->recursive_tree = spamm_recursive_new_node();
  }

#pragma omp parallel
  {
#pragma omp single
    {
#pragma omp task untied
      spamm_recursive_add(alpha, A->recursive_tree, beta,
          (const struct spamm_recursive_node_t*const) B_pointer,
          A->number_dimensions, 0, A->chunk_tier, A->use_linear_tree, flop);
    }
  }
}
