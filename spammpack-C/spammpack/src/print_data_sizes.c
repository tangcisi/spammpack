#include "config.h"
#include "spamm_kernel.h"
#include "spamm_types_private.h"

#include <stdint.h>
#include <stdio.h>

int
main ()
{
  struct spamm_hashed_data_t data;
  struct spamm_multiply_stream_t stream_element;

  printf("# spammpack version: %s\n", SPAMM_VERSION);
  printf("#\n");
  printf("# sizeof(struct spamm_multiply_stream_t) = %lu\n", sizeof(struct spamm_multiply_stream_t));
  printf("# &stream_element at   %p\n", &stream_element);
  printf("# &stream_element.A at %p, offset = %lu\n", &stream_element.A, (intptr_t) &stream_element.A - (intptr_t) &stream_element);
  printf("# &stream_element.B at %p, offset = %lu\n", &stream_element.B, (intptr_t) &stream_element.B - (intptr_t) &stream_element);
  printf("# &stream_element.C at %p, offset = %lu\n", &stream_element.C, (intptr_t) &stream_element.C - (intptr_t) &stream_element);
  printf("# \n");
  printf("# sizeof(struct spamm_hashed_data_t) = %lu\n", sizeof(struct spamm_hashed_data_t));
  printf("# &data at                       %p\n", &data);
  printf("# &data.norm at                  %p, offset_norm                  = %6lu\n", &data.norm, (intptr_t) &data.norm - (intptr_t) &data);
  printf("# &data.block_dense at           %p, offset_block_dense           = %6lu\n", &data.block_dense, (intptr_t) &data.block_dense - (intptr_t) &data);
  printf("# &data.block_dense_store at     %p, offset_block_dense_store     = %6lu\n", &data.block_dense_store, (intptr_t) &data.block_dense_store - (intptr_t) &data);
  printf("# &data.block_dense_transpose at %p, offset_block_dense_transpose = %6lu\n", &data.block_dense_transpose, (intptr_t) &data.block_dense_transpose - (intptr_t) &data);
  printf("# &data.block_dense_dilated at   %p, offset_block_dense_dilated   = %6lu\n", &data.block_dense_dilated, (intptr_t) &data.block_dense_dilated - (intptr_t) &data);
  printf("\n");
  printf("class spammOffsets:\n");
  printf("  \"\"\"The byte offsets into the stream element data structure.\n");
  printf("  \"\"\"\n");
  printf("\n");
  printf("  sizeof_multiply_stream_t     = %6lu\n", sizeof(struct spamm_multiply_stream_t));
  printf("  offset_norm                  = %6lu # 0x%04lx\n", (intptr_t) &data.norm - (intptr_t) &data, (intptr_t) &data.norm - (intptr_t) &data);
  printf("  offset_block_dense           = %6lu # 0x%04lx\n", (intptr_t) &data.block_dense - (intptr_t) &data, (intptr_t) &data.block_dense - (intptr_t) &data);
  printf("  offset_block_dense_store     = %6lu # 0x%04lx\n", (intptr_t) &data.block_dense_store - (intptr_t) &data, (intptr_t) &data.block_dense_store - (intptr_t) &data);
  printf("  offset_block_dense_transpose = %6lu # 0x%04lx\n", (intptr_t) &data.block_dense_transpose - (intptr_t) &data, (intptr_t) &data.block_dense_transpose - (intptr_t) &data);
  printf("  offset_block_dense_dilated   = %6lu # 0x%04lx\n", (intptr_t) &data.block_dense_dilated - (intptr_t) &data, (intptr_t) &data.block_dense_dilated - (intptr_t) &data);

  return 0;
}
