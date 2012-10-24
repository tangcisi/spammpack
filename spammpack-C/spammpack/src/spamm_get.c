#include "spamm.h"
#include "spamm_types_private.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/** Get an element from a matrix.
 *
 * @param i The row index.
 * @param j The column index.
 * @param A The matrix.
 *
 * @return The matrix element \f$A(i,j)\f$.
 */
float
spamm_hashed_get (const unsigned int i, const unsigned int j, const struct spamm_hashed_t *A)
{
  unsigned int index, i_tier, j_tier;
  struct spamm_hashtable_t *node_hashtable;
  struct spamm_hashed_data_t *data;

  i_tier = (i-A->M_lower)/SPAMM_N_KERNEL;
  j_tier = (j-A->N_lower)/SPAMM_N_KERNEL;

  /* Construct linear index of the node on this tier. */
  index = spamm_index_2D(i_tier, j_tier);

  /* Get hash table at this tier. */
  node_hashtable = A->tier_hashtable[A->kernel_tier-A->tier];

  if ((data = spamm_hashtable_lookup(node_hashtable, index)) != NULL)
  {
    return data->block_dense[spamm_index_kernel_block((i-A->M_lower)%SPAMM_N_KERNEL, (j-A->N_lower)%SPAMM_N_KERNEL, A->layout)];
  }

  else { return 0; }
}

/** Get an element from a recursive matrix.
 *
 * If the matrix is NULL, this function returns 0.
 *
 * @param i An array of row indices.
 * @param A The matrix.
 *
 * @return The matrix element Aij.
 */
float
spamm_recursive_get (const unsigned int *const i,
    const struct spamm_recursive_node_t *node)
{
  unsigned int number_rows;
  unsigned int number_columns;

  if (node == NULL) { return 0; }

  number_rows = node->N_upper[0]-node->N_lower[0];

  if (node->number_dimensions == 2 && node->tier == node->contiguous_tier && node->use_linear_tree)
  {
    return spamm_hashed_get(i[0], i[1], node->tree.hashed_tree);
  }

  else if (node->tier == node->contiguous_tier)
  {
    /* Get the matrix element. */
    if (node->tree.data == NULL) { return 0.0; }
    else
    {
      switch (node->number_dimensions)
      {
        case 1:
          return node->tree.data[i[0]-node->N_lower[0]];
          break;

        case 2:
          return node->tree.data[spamm_index_column_major(i[0]-node->N_lower[0], i[1]-node->N_lower[1], number_rows, number_rows)];
          break;

        default:
          SPAMM_FATAL("not implemented\n");
      }
    }
  }

  else
  {
    switch (node->number_dimensions)
    {
      case 1:
        if (i[0] < node->N_lower[0]+(number_rows)/2)
        {
          return spamm_recursive_get(i, node->tree.child[0]);
        }

        else
        {
          return spamm_recursive_get(i, node->tree.child[1]);
        }

      case 2:
        number_columns = node->N_upper[1]-node->N_lower[1];
        if (i[0] < node->N_lower[0]+(number_rows)/2 &&
            i[1] < node->N_lower[1]+(number_columns)/2)
        {
          return spamm_recursive_get(i, node->tree.child[0]);
        }

        else if (i[0] <  node->N_lower[0]+(number_rows)/2 &&
            i[1] >= node->N_lower[1]+(number_columns)/2)
        {
          return spamm_recursive_get(i, node->tree.child[1]);
        }

        else if (i[0] >= node->N_lower[0]+(number_rows)/2 &&
            i[1] <  node->N_lower[1]+(number_columns)/2)
        {
          return spamm_recursive_get(i, node->tree.child[2]);
        }

        else
        {
          return spamm_recursive_get(i, node->tree.child[3]);
        }
        break;

      default:
        SPAMM_FATAL("not implemented\n");
    }
  }

  SPAMM_FATAL("I should not be here\n");
  return 0;
}

/** Get an element from a matrix.
 *
 * @param i The row/column index.
 * @param A The matrix.
 *
 * @return The matrix element.
 */
float
spamm_get (const unsigned int *const i, const struct spamm_matrix_t *A)
{
  int dim;

  assert(A != NULL);

  for (dim = 0; dim < A->number_dimensions; dim++)
  {
    if (i[dim] >= A->N[dim])
    {
      SPAMM_FATAL("i[%u] out of bounds (i[%u] = %i and N[%u] = %i)\n", dim, dim, i, dim, A->N[dim]);
    }
  }

  if (A->number_dimensions == 2 && A->contiguous_tier == 0 && A->use_linear_tree)
  {
    /* In case we only have a linear tree. */
    return spamm_hashed_get(i[0], i[1], A->tree.hashed_tree);
  }

  else
  {
    return spamm_recursive_get(i, A->tree.recursive_tree);
  }
}

/** Get the number of rows of a matrix.
 *
 * @param A The matrix.
 *
 * @return The number of rows.
 */
unsigned int
spamm_get_number_of_rows (const struct spamm_hashed_t *const A)
{
  return A->M_upper-A->M_lower;
}

/** Get the number of columns of a matrix.
 *
 * @param A The matrix.
 *
 * @return The number of rows.
 */
unsigned int
spamm_get_number_of_columns (const struct spamm_hashed_t *const A)
{
  return A->N_upper-A->N_lower;
}

/** Get the Frobenius norm of the matrix.
 *
 * @param A The matrix.
 *
 * @return The Frobenius norm.
 */
float
spamm_hashed_get_norm (const struct spamm_hashed_t *const A)
{
  struct spamm_hashtable_t *tier_hashtable;
  struct spamm_hashed_node_t *root;

  assert(A != NULL);

  if ((tier_hashtable = A->tier_hashtable[0]) == NULL)
  {
    return 0;
  }

  if ((root = spamm_hashtable_lookup(tier_hashtable, 0)) == NULL)
  {
    return 0;
  }

  return root->norm;
}

/** Get the Frobenius norm of the matrix.
 *
 * @param A The matrix.
 *
 * @return The Frobenius norm.
 */
float
spamm_get_norm (const struct spamm_matrix_t *const A)
{
  assert(A != NULL);

  if (A->tree.recursive_tree != NULL)
  {
    return A->tree.recursive_tree->norm;
  }

  else if (A->tree.hashed_tree != NULL)
  {
    return spamm_hashed_get_norm(A->tree.hashed_tree);
  }

  else { return 0; }
}
