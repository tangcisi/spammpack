#include "spamm.h"
#include "spamm_types_private.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/** @private The user data object that is passed to to the hash table
 * iterator.
 */
struct spamm_check_user_data_t
{
  /** The result. */
  int result;

  /** The tier this hash table is on. */
  unsigned int tier;

  /** The spamm_hashed_t object the hash table was taken from. */
  const struct spamm_hashed_t *A;

  /** The test tolerance. */
  float tolerance;
};

/** Verify tree structure. This step verifies that the tree hierarchy is
 * correct. Although we don't explicitly store the tree as a quadtree pointer
 * list, the tree hierarchy is still present and is given simply by the fact
 * that a non-zero node should have a parent.
 *
 * @param index The linear index of this node.
 * @param value The matrix tree node. This can have either type
 * spamm_hashed_node_t or spamm_hashed_data_t depending on the tier.
 * @param user_data The user data of type spamm_check_user_data_t.
 */
void
spamm_check_tree_structure (unsigned int index, void *value, void *user_data)
{
  char binary_string_1[100];
  char binary_string_2[100];
  unsigned int tier;
  unsigned int next_index;
  struct spamm_hashtable_t *tier_hashtable;
  struct spamm_check_user_data_t *user = user_data;

  /* Go up the tiers and make sure that we have a node that covers the indices
   * of the current node.
   */
  for (next_index = index, tier = user->tier; ; )
  {
    if (tier > 0)
    {
      tier -= 1;
      next_index >>= 2;
    }

    else
    {
      break;
    }

    tier_hashtable = user->A->tier_hashtable[tier];
    if (spamm_hashtable_lookup(tier_hashtable, next_index) == NULL)
    {
      spamm_uint_to_bin_string(2*user->tier, index, binary_string_1);
      spamm_uint_to_bin_string(2*tier, next_index, binary_string_2);
      printf("missing node in tree: cannot find node for index %s ", binary_string_2);
      printf("at tier %u which is needed for index %s at tier %u\n", tier, binary_string_1, user->tier);
      user->result = SPAMM_ERROR;
      break;
    }
  }
}

/** Verify linear indices. This step computes the linear matrix index of each
 * tree node and verifies that the stored index matches the one calculated.
 *
 * @param index The linear index of this node.
 * @param value The matrix tree node. This can have either type
 * spamm_hashed_node_t or spamm_hashed_data_t depending on the tier.
 * @param user_data The user data of type spamm_check_user_data_t.
 */
void
spamm_check_linear_index (unsigned int index, void *value, void *user_data)
{
  //printf("[FIXME] (verifying linear indices)\n");
}

/** Verify the norm of a node.
 *
 * This function is called in the hash table iterator for each tier of the
 * matrix tree.
 *
 * @param index The linear index of this node.
 * @param value The matrix tree node. This can have either type
 * spamm_hashed_node_t or spamm_hashed_data_t depending on the tier.
 * @param user_data The user data of type spamm_check_user_data_t.
 */
void
spamm_check_norm (unsigned int index, void *value, void *user_data)
{
  short i_child, j_child;
  short i_blocked, j_blocked;
  short i_basic, j_basic;
  float norm2 = 0.0;

  float Aij;

  unsigned int child_index;
  struct spamm_hashed_node_t *child_node = NULL;
  struct spamm_hashed_data_t *child_data = NULL;

  struct spamm_hashed_node_t *node = NULL;
  struct spamm_hashed_data_t *data = NULL;
  struct spamm_check_user_data_t *user = user_data;

  unsigned int next_tier;
  struct spamm_hashtable_t *next_tier_hashtable = NULL;

  unsigned int norm_offset;

  /* Load correct value. */
  if (user->tier == user->A->kernel_tier)
  {
    data = value;

    /* Check norms on kernel blocks. */
    for (i_blocked = 0; i_blocked < SPAMM_N_KERNEL_BLOCKED; i_blocked++) {
      for (j_blocked = 0; j_blocked < SPAMM_N_KERNEL_BLOCKED; j_blocked++)
      {
        norm2 = 0.0;
        for (i_basic = 0; i_basic < SPAMM_N_BLOCK; i_basic++) {
          for (j_basic = 0; j_basic < SPAMM_N_BLOCK; j_basic++)
          {
            Aij = data->block_dense[spamm_index_kernel_block_hierarchical(i_blocked, j_blocked, i_basic, j_basic, data->layout)];
            norm2 += Aij*Aij;
          }
        }

        norm_offset = spamm_index_norm(i_blocked, j_blocked);
        if (fabs(norm2-data->norm2[norm_offset]) > user->tolerance*norm2 ||
            fabs(sqrt(norm2)-data->norm[norm_offset]) > user->tolerance*sqrt(norm2))
        {
          printf("tier %u, index %u, block (%u,%u): incorrect block norm value, found %e, should be %e, |diff| = %e, rel. diff = %e\n",
              data->tier, data->index_2D, i_blocked, j_blocked,
              data->norm[norm_offset],
              sqrt(norm2),
              fabs(data->norm[norm_offset]-sqrt(norm2)),
              (norm2 != 0.0 ? fabs(data->norm[norm_offset]-sqrt(norm2))/sqrt(norm2) : 0));
          user->result = SPAMM_ERROR;
        }
      }
    }

    /* Check norms on kernel tier block. */
    norm2 = 0.0;
    for (i_blocked = 0; i_blocked < SPAMM_N_KERNEL_BLOCKED; i_blocked++) {
      for (j_blocked = 0; j_blocked < SPAMM_N_KERNEL_BLOCKED; j_blocked++)
      {
        norm2 += data->norm2[spamm_index_norm(i_blocked, j_blocked)];
      }
    }

    if (fabs(norm2-data->node_norm2) > user->tolerance*norm2 ||
        fabs(sqrt(norm2)-data->node_norm) > user->tolerance*sqrt(norm2))
    {
      printf("tier %u, index %u: incorrect node norm value, found %e = sqrt(%e), should be %e, |diff| = %e, rel. diff = %e\n",
          data->tier, data->index_2D,
          data->node_norm,
          data->node_norm2,
          sqrt(norm2),
          fabs(data->node_norm-sqrt(norm2)),
          (norm2 != 0.0 ? fabs(data->node_norm-sqrt(norm2))/sqrt(norm2) : 0));
      user->result = SPAMM_ERROR;
    }
  }

  else
  {
    node = value;

    /* Get the tier hashtable for the next tier. */
    next_tier = user->tier+1;
    next_tier_hashtable = user->A->tier_hashtable[next_tier];

    norm2 = 0.0;

    if (next_tier == user->A->kernel_tier)
    {
      for (i_child = 0; i_child < 2; i_child++) {
        for (j_child = 0; j_child < 2; j_child++)
        {
          /* Construct index of child block. */
          child_index = (index << 2) | (i_child << 1) | j_child;

          /* Get child node. */
          child_data = spamm_hashtable_lookup(next_tier_hashtable, child_index);

          if (child_data != NULL)
          {
            norm2 += child_data->node_norm2;
          }
        }
      }
    }

    else
    {
      for (i_child = 0; i_child < 2; i_child++) {
        for (j_child = 0; j_child < 2; j_child++)
        {
          /* Construct index of child block. */
          child_index = (index << 2) | (i_child << 1) | j_child;

          /* Get child node. */
          child_node = spamm_hashtable_lookup(next_tier_hashtable, child_index);

          if (child_node != NULL)
          {
            norm2 += child_node->norm2;
          }
        }
      }
    }

    if (fabs(norm2-node->norm2) > user->tolerance*norm2 ||
        fabs(sqrt(norm2)-node->norm) > user->tolerance*sqrt(norm2))
    {
      printf("tier %u, index %u: incorrect norm value, found %e = sqrt(%e), should be %e, |diff| = %e, rel. diff = %e\n",
          node->tier, node->index_2D,
          node->norm,
          node->norm2,
          sqrt(norm2),
          fabs(node->norm-sqrt(norm2)),
          (norm2 != 0.0 ? fabs(node->norm-sqrt(norm2))/sqrt(norm2) : 0));
      user->result = SPAMM_ERROR;
    }
  }
}

void
spamm_check_data_consistency (unsigned int index, void *value, void *user_data)
{
  short i_blocked, j_blocked;
  short i_basic, j_basic;
  short i_dilated;
  float Aij;
  unsigned int data_offset;
  struct spamm_hashed_data_t *data = value;
  struct spamm_check_user_data_t *user = user_data;

  for (i_blocked = 0; i_blocked < SPAMM_N_KERNEL_BLOCKED && user->result == SPAMM_OK; i_blocked++) {
    for (j_blocked = 0; j_blocked < SPAMM_N_KERNEL_BLOCKED && user->result == SPAMM_OK; j_blocked++) {
      for (i_basic = 0; i_basic < SPAMM_N_BLOCK && user->result == SPAMM_OK; i_basic++) {
        for (j_basic = 0; j_basic < SPAMM_N_BLOCK; j_basic++)
        {
          data_offset = spamm_index_kernel_block_hierarchical(i_blocked, j_blocked, i_basic, j_basic, user->A->layout);
          Aij = data->block_dense[data_offset];
          if (Aij != data->block_dense_store[spamm_index_kernel_block_hierarchical(i_blocked, j_blocked, i_basic, j_basic, user->A->layout)])
          {
            printf("index %u: data block inconsistency between block_dense and block_dense_store, ", index);
            printf("i_blocked = %u, j_blocked = %u, i_basic = %u, j_basic = %u, Aij = %e, (Aij)store = %e, |diff| = %e\n",
                i_blocked, j_blocked, i_basic, j_basic,
                Aij,
                data->block_dense_store[spamm_index_kernel_block_hierarchical(i_blocked, j_blocked, i_basic, j_basic, user->A->layout)],
                fabs(Aij-data->block_dense_store[spamm_index_kernel_block_hierarchical(i_blocked, j_blocked, i_basic, j_basic, user->A->layout)]));
            user->result = SPAMM_ERROR;
            break;
          }
          for (i_dilated = 0; i_dilated < 4; i_dilated++)
          {
            if (Aij != data->block_dense_dilated[data_offset*4+i_dilated])
            {
              printf("index %u: data block inconsistency between block_dense and block_dense_dilated\n", index);
              user->result = SPAMM_ERROR;
              break;
            }
          }
          if (Aij != data->block_dense_transpose[spamm_index_kernel_block_transpose_hierarchical(i_blocked, j_blocked, i_basic, j_basic, user->A->layout)])
          {
            printf("index %u: data block inconsistency between block_dense and block_dense_transpose, ", index);
            printf("i_blocked = %u, j_blocked = %u, i_basic = %u, j_basic = %u, Aij = %e, (A^T)ij = %e, |diff| = %e\n",
                i_blocked, j_blocked, i_basic, j_basic,
                Aij,
                data->block_dense_transpose[spamm_index_kernel_block_transpose_hierarchical(i_blocked, j_blocked, i_basic, j_basic, user->A->layout)],
                fabs(Aij-data->block_dense_transpose[spamm_index_kernel_block_transpose_hierarchical(i_blocked, j_blocked, i_basic, j_basic, user->A->layout)]));
            user->result = SPAMM_ERROR;
            break;
          }
        }
      }
    }
  }
}

/** Check the internal consistency of a matrix.
 *
 * @param A The matrix to check
 * @param tolerance The absolute tolerance when comparing values.
 *
 * @return The following error codes are returned:
 *   - SPAMM_OK - The matrix is consistent.
 *   - SPAMM_ERROR - Something is not consistent.
 */
int
spamm_check (const struct spamm_matrix_t *A, const float tolerance)
{
  unsigned int depth;
  unsigned int N_padded;
  int dim;
  float x, x_N;
  struct spamm_check_user_data_t user_data;

  assert(A != NULL);

  user_data.result = SPAMM_OK;

  /* Calculate the padding and depth of matrix based on values stored in N. */
  x = 0;
  for (dim = 0; dim < A->number_dimensions; dim++)
  {
    x_N = (log(A->N[dim]) > log(SPAMM_N_BLOCK) ? log(A->N[dim]) - log(SPAMM_N_BLOCK) : 0)/log(2);
    if (x_N > x) { x = x_N; }
  }

  /* The ceil() function can lead to a depth that is one tier too large
   * because of numerical errors in the calculation of x. We need to check
   * whether the depth is appropriate.
   */
  depth = (unsigned int) ceil(x);

  /* Double check depth. */
  if (depth >= 1)
  {
    for (dim = 0; dim < A->number_dimensions; dim++)
    {
      if ((int) (SPAMM_N_BLOCK*ipow(2, depth-1)) < A->N[dim])
      {
        depth++;
        break;
      }
    }
    depth--;
  }

  /* Adjust tree to kernel depth. */
  if (depth < SPAMM_KERNEL_DEPTH) { depth = SPAMM_KERNEL_DEPTH; }

  if (A->depth != depth)
  {
    printf("depth incorrect, should be %u, but is %u\n", depth, A->depth);
    return SPAMM_ERROR;
  }

  N_padded = (int) (SPAMM_N_BLOCK*ipow(2, depth));

  if (A->N_padded != N_padded)
  {
    printf("padded matrix dimensions incorrect, should be %u, but is %u\n", N_padded, A->N_padded);
    return SPAMM_ERROR;
  }

  if (A->kernel_tier != depth-SPAMM_KERNEL_DEPTH)
  {
    SPAMM_WARN("kernel tier incorrect, should be %u, but is %u\n", depth-SPAMM_KERNEL_DEPTH, A->kernel_tier);
    return SPAMM_ERROR;
  }

  /* Check whether there are tier hashtables for every tier. */
  //if (A->tier_hashtable == NULL)
  //{
  //  printf("no tier hashtable\n");
  //  return SPAMM_ERROR;
  //}

  //for (tier = 0; tier <= A->kernel_tier; tier++)
  //{
  //  if ((hashtable = A->tier_hashtable[tier]) == NULL)
  //  {
  //    printf("missing tier hashtable for tier %u\n", tier);
  //    return SPAMM_ERROR;
  //  }
  //}

  /* Check each node. */
  //for (tier = A->kernel_tier+1; tier >= 1; tier--)
  //{
  //  /* Get tier hashtable. */
  //  reverse_tier = tier-1;
  //  hashtable = A->tier_hashtable[reverse_tier];

  //  user_data.tier = reverse_tier;
  //  user_data.A = A;
  //  user_data.tolerance = tolerance;

  //  if (reverse_tier == A->kernel_tier)
  //  {
  //    /* Verify tree structure. */
  //    spamm_hashtable_foreach(hashtable, spamm_check_tree_structure, &user_data);

  //    /* Verify transpose and dilated data. */
  //    spamm_hashtable_foreach(hashtable, spamm_check_data_consistency, &user_data);
  //  }

  //  /* Verify consistency of linear indices. */
  //  spamm_hashtable_foreach(hashtable, spamm_check_linear_index, &user_data);

  //  /* Verify norms. */
  //  spamm_hashtable_foreach(hashtable, spamm_check_norm, &user_data);
  //}

  return user_data.result;
}
