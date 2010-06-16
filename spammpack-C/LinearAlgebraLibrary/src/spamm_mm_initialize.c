#include "spamm.h"
#include <stdlib.h>

/** Initialize a new dynamic memory region.
 *
 * @param chunksize The size of contiguous chunks.
 *
 * @return A pointer to a linked list that contains the allocated memory. A
 *         return value of NULL indicates that allocation failed.
 */
struct spamm_ll_t *
spamm_mm_initialize (const unsigned int chunksize)
{
  struct spamm_ll_t *result = NULL;

#ifdef SPAMM_MM_DEBUG
  LOG("allocating memory with chunks of %u bytes\n", chunksize);
#endif
  result = (struct spamm_ll_t*) malloc(sizeof(struct spamm_ll_t));
  if (result != NULL)
  {
    spamm_ll_initialize(result);

    /* Allocate memory for data chunk bookkeeping structure. */
    if (spamm_mm_grow(chunksize, result) == NULL)
    {
      spamm_log("failed to grow memory\n", __FILE__, __LINE__);
      return NULL;
    }
  }

  /* Debug. */
#ifdef SPAMM_MM_DEBUG
  spamm_mm_print(result);
#endif

  return result;
}
