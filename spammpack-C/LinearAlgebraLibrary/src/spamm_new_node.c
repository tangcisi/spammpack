#include "spamm.h"
#include <stdlib.h>

void
spamm_new_node (struct spamm_node_t **node)
{
  int i;

  *node = (struct spamm_node_t*) malloc(sizeof(struct spamm_node_t));

  (*node)->M_upper = 0;
  (*node)->M_lower = 0;
  (*node)->N_upper = 0;
  (*node)->N_lower = 0;

  (*node)->M_child = 0;
  (*node)->N_child = 0;

  (*node)->M_block = 0;
  (*node)->N_block = 0;

  (*node)->threshold = 0.0;

  (*node)->index = 0;
  (*node)->ordering = P;

  (*node)->child = NULL;
  (*node)->block_dense = NULL;
}
