#include "spamm.h"
#include <assert.h>
#include <stdlib.h>

/** \private Calculate
 *
 * \f$B_{\mathrm node} = \alpha A_{\mathrm node} + \beta B_{\mathrm node}\f$
 *
 * This is the recursive part. Use spamm_add() instead.
 *
 * @param alpha The factor \f$\alpha\f$.
 * @param A_node Matrix node \f$A\f$.
 * @param beta The factor \f$\beta\f$.
 * @param B_node Matrix node \f$B\f$.
 */
void
spamm_add_node (const floating_point_t alpha, const struct spamm_node_t *A_node, const floating_point_t beta, struct spamm_node_t **B_node)
{
  int i, j;
  char binary_string[100];
  struct spamm_node_t *new_node;
  struct spamm_ll_iterator_t *iterator_A, *iterator_B;
  struct spamm_ll_node_t *linear_node_A, *linear_node_B;
  struct spamm_linear_quadtree_t *linear_A, *linear_B;

  /* We distinguish between the following cases:
   *
   * (1) neither A nor B exist --> We are done.
   * (2) A exists but B doesn't --> We need to create a new node in B.
   */

  if (A_node == NULL && *B_node == NULL)
  {
    /* We are done here. */
    LOG2_DEBUG("A and B are NULL\n");
    return;
  }

  if (A_node != NULL && *B_node == NULL)
  {
    /* We need to add to B. */
    LOG2_DEBUG("A != NULL && B == NULL: creating new tree node in B\n");
    *B_node = spamm_new_node();

    (*B_node)->tier = A_node->tier;
    (*B_node)->tree_depth = A_node->tree_depth;

    (*B_node)->M_lower = A_node->M_lower;
    (*B_node)->M_upper = A_node->M_upper;
    (*B_node)->N_lower = A_node->N_lower;
    (*B_node)->N_upper = A_node->N_upper;

    (*B_node)->linear_tier = A_node->linear_tier;
  }

  /* Decide on how to recurse further. */
  if (A_node != NULL && A_node->child != NULL && (*B_node)->child == NULL)
  {
    /* We need to recurse further. A still has children nodes, but B doesn't.
     * We therefore create children nodes in B.
     */
    LOG2_DEBUG("A has children, B does not. Creating children in B to recurse.\n");
    for (i = 0; i < SPAMM_M_CHILD; ++i) {
      for (j = 0; j < SPAMM_N_CHILD; ++j)
      {
        (*B_node)->child[i][j] = spamm_new_node();
        new_node = (*B_node)->child[i][j];

        new_node->tier = (*B_node)->tier+1;
        new_node->tree_depth = (*B_node)->tree_depth;

        new_node->M_lower = (*B_node)->M_lower+i*((*B_node)->M_upper-(*B_node)->M_lower)/SPAMM_M_CHILD;
        new_node->M_upper = (*B_node)->M_lower+(i+1)*((*B_node)->M_upper-(*B_node)->M_lower)/SPAMM_M_CHILD;
        new_node->N_lower = (*B_node)->N_lower+j*((*B_node)->N_upper-(*B_node)->N_lower)/SPAMM_N_CHILD;
        new_node->N_upper = (*B_node)->N_lower+(j+1)*((*B_node)->N_upper-(*B_node)->N_lower)/SPAMM_N_CHILD;

        new_node->linear_tier = (*B_node)->linear_tier;
      }
    }
  }

  if (A_node != NULL && A_node->block_dense != NULL && (*B_node)->block_dense == NULL)
  {
    /* We can stop recursing. A has a dense block but B doesn't. We therefore
     * create an empty dense block in B. */
    LOG2_DEBUG("A and B have a dense block here.\n");
    (*B_node)->block_dense = (floating_point_t*) spamm_allocate(sizeof(floating_point_t)*SPAMM_M_BLOCK*SPAMM_N_BLOCK);
    for (i = 0; i < SPAMM_M_BLOCK; ++i) {
      for (j = 0; j < SPAMM_N_BLOCK; ++j)
      {
        (*B_node)->block_dense[spamm_dense_index(i, j, SPAMM_M_BLOCK, SPAMM_N_BLOCK)] = 0;
      }
    }
  }

  if (A_node != NULL && A_node->linear_quadtree != NULL && (*B_node)->linear_quadtree == NULL)
  {
    /* Create a new linear quadtree in B. */
    LOG2_DEBUG("A has linear quadtree, B does not.\n");
    (*B_node)->linear_quadtree = spamm_ll_new();
    (*B_node)->linear_quadtree_memory = spamm_mm_new(A_node->linear_quadtree_memory->chunksize);
  }

  if ((*B_node)->child != NULL)
  {
    LOG2_DEBUG("B has children, recursing further.\n");

    /* Recurse further down in A & B. */
    if (A_node != NULL && A_node->child != NULL)
    {
      for (i = 0; i < SPAMM_M_CHILD; ++i) {
        for (j = 0; j < SPAMM_N_CHILD; ++j)
        {
          spamm_add_node(alpha, A_node->child[i][j], beta, &((*B_node)->child[i][j]));
        }
      }
    }

    /* A doesn't exist, only multiply B with beta. */
    else
    {
      for (i = 0; i < SPAMM_M_CHILD; ++i) {
        for (j = 0; j < SPAMM_N_CHILD; ++j)
        {
          spamm_add_node(alpha, NULL, beta, &((*B_node)->child[i][j]));
        }
      }
    }
  }

  else if ((*B_node)->block_dense != NULL)
  {
    LOG2_DEBUG("B has a dense block.\n");

    /* Add dense blocks. */
    if (A_node != NULL && A_node->block_dense != NULL)
    {
      for (i = 0; i < SPAMM_M_BLOCK; ++i) {
        for (j = 0; j < SPAMM_N_BLOCK; ++j)
        {
          (*B_node)->block_dense[spamm_dense_index(i, j, SPAMM_M_BLOCK, SPAMM_N_BLOCK)] = alpha*A_node->block_dense[spamm_dense_index(i, j, SPAMM_M_BLOCK, SPAMM_N_BLOCK)]
            + beta*(*B_node)->block_dense[spamm_dense_index(i, j, SPAMM_M_BLOCK, SPAMM_N_BLOCK)];
        }
      }
    }

    /* A doesn't exist. Only multiply B with beta. */
    else
    {
      for (i = 0; i < SPAMM_M_BLOCK; ++i) {
        for (j = 0; j < SPAMM_N_BLOCK; ++j)
        {
          (*B_node)->block_dense[spamm_dense_index(i, j, SPAMM_M_BLOCK, SPAMM_N_BLOCK)] = beta*(*B_node)->block_dense[spamm_dense_index(i, j, SPAMM_M_BLOCK, SPAMM_N_BLOCK)];
        }
      }
    }
  }

  else if ((*B_node)->linear_quadtree != NULL)
  {
    LOG2_DEBUG("B has a linear quadtree.\n");

    /* Multiply B with beta. */
    iterator_B = spamm_ll_iterator_new((*B_node)->linear_quadtree);
    for (linear_node_B = spamm_ll_iterator_first(iterator_B); linear_node_B != NULL; linear_node_B = spamm_ll_iterator_next(iterator_B))
    {
      linear_B = linear_node_B->data;
      for (i = 0; i < SPAMM_M_BLOCK; ++i) {
        for (j = 0; j < SPAMM_N_BLOCK; ++j)
        {
          linear_B->block_dense[spamm_dense_index(i, j, SPAMM_M_BLOCK, SPAMM_N_BLOCK)] *= beta;
        }
      }
    }
    spamm_ll_iterator_delete(&iterator_B);

    /* Add to B to A. */
    if (A_node != NULL && A_node->linear_quadtree != NULL)
    {
      LOG2_DEBUG("adding existing linear quadtree of A to B.\n");

      iterator_A = spamm_ll_iterator_new(A_node->linear_quadtree);
      for (linear_node_A = spamm_ll_iterator_first(iterator_A); linear_node_A != NULL; linear_node_A = spamm_ll_iterator_next(iterator_A))
      {
        linear_A = linear_node_A->data;

        spamm_int_to_binary(linear_A->index, A_node->tree_depth*2, binary_string);
        LOG_DEBUG("found block in A: %s\n", binary_string);

        /* Search B to find whether we have a matching block that we can add.
         * If not, we need to create a new one.
         */
        linear_B = NULL;
        iterator_B = spamm_ll_iterator_new((*B_node)->linear_quadtree);
        for (linear_node_B = spamm_ll_iterator_first(iterator_B); linear_node_B != NULL; linear_node_B = spamm_ll_iterator_next(iterator_B))
        {
          linear_B = linear_node_B->data;
          if (linear_A->index == linear_B->index)
          {
            /* Found matching block in B. */
            spamm_int_to_binary(linear_A->index, A_node->tree_depth*2, binary_string);
            LOG_DEBUG("found matching block in B: index = %s\n", binary_string);
            break;
          }
        }
        spamm_ll_iterator_delete(&iterator_B);

        if (linear_B == NULL || (linear_B != NULL && linear_A->index != linear_B->index))
        {
          /* We didn't find a matching block in B. Create a new one. */
          LOG2_DEBUG("could not find matching block in B. Creating new one.\n");
          linear_B = spamm_new_linear_quadtree_node(SPAMM_M_BLOCK, SPAMM_N_BLOCK, (*B_node)->linear_quadtree_memory);
          linear_B->index = linear_A->index;
          spamm_ll_append(linear_B, (*B_node)->linear_quadtree);

          for (i = 0; i < SPAMM_M_BLOCK; ++i) {
            for (j = 0; j < SPAMM_N_BLOCK; ++j)
            {
              linear_B->block_dense[spamm_dense_index(i, j, SPAMM_M_BLOCK, SPAMM_N_BLOCK)] = 0.0;
            }
          }
        }

        /* Add blocks of A and B. */
        LOG2_DEBUG("adding A and B blocks\n");
        for (i = 0; i < SPAMM_M_BLOCK; ++i) {
          for (j = 0; j < SPAMM_N_BLOCK; ++j)
          {
            linear_B->block_dense[spamm_dense_index(i, j, SPAMM_M_BLOCK, SPAMM_N_BLOCK)] += alpha*linear_A->block_dense[spamm_dense_index(i, j, SPAMM_M_BLOCK, SPAMM_N_BLOCK)];
          }
        }
      }
      spamm_ll_iterator_delete(&iterator_A);
    }
  }
}

/** Calculate
 *
 * \f$B = \alpha A + \beta B\f$
 *
 * @param alpha The factor \f$\alpha\f$.
 * @param A Matrix \f$A\f$.
 * @param beta The factor \f$\beta\f$.
 * @param B Matrix \f$B\f$.
 */
void
spamm_add (const floating_point_t alpha, const struct spamm_t *A, const floating_point_t beta, struct spamm_t *B)
{
  assert(A != NULL);
  assert(B != NULL);

  if (A->M != B->M)
  {
    LOG_FATAL("matrix size mismatch, A->M = %i, B->M = %i\n", A->M, B->M);
    exit(1);
  }

  if (A->N != B->N)
  {
    LOG_FATAL("matrix size mismatch, A->N = %i, B->N = %i\n", A->N, B->N);
    exit(1);
  }

  if (A->linear_tier != B->linear_tier)
  {
    LOG_FATAL("linear_tier mismatch, A->linear_tier = %u, B->linear_tier = %u\n", A->linear_tier, B->linear_tier);
    exit(1);
  }

  LOG2_DEBUG("starting to add...\n");
  spamm_add_node(alpha, A->root, beta, &(B->root));
  LOG2_DEBUG("done.\n");
}
