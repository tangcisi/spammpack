#include "spamm.h"
#include <assert.h>
#include <stdlib.h>

/** Delete memory managed memory.
 *
 * @param memory The managed memory.
 */
void
spamm_mm_delete (struct spamm_mm_t **memory)
{
  assert(*memory != NULL);

  spamm_ll_delete(spamm_mm_delete_chunk, &(*memory)->chunks);
  free(*memory);
  *memory = NULL;
}
