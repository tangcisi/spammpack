/** @file */

#ifndef __SPAMM_KERNEL_H
#define __SPAMM_KERNEL_H

#include "spamm_types.h"

/** Available stream kernels. */
#define SPAMM_NUMBER_KERNELS 4

/** The different stream kernels.
 */
enum spamm_kernel_t
{
  /** The external sgemm version of the stream kernel. */
  kernel_external_sgemm,

  /** The external sgemm version of the stream kernel with NULL as sgemm_(). */
  kernel_stream_NULL,

  /** The standard stream kernel (SSE). */
  kernel_standard_SSE,

  /** The standard stream kernel (SSE4.1). */
  kernel_standard_SSE4_1
};

const char *
spamm_kernel_get_name (const unsigned int i);

enum spamm_kernel_t
spamm_kernel_get_kernel (const char* name);

enum spamm_layout_t
spamm_kernel_suggest_layout (const enum spamm_kernel_t kernel);

enum spamm_layout_t
spamm_kernel_get_layout (const char *name);

void
spamm_stream_kernel_SSE (const unsigned int number_stream_elements,
    float alpha,
    float tolerance,
    struct spamm_multiply_stream_t *multiply_stream);

void
spamm_stream_kernel_SSE4_1 (const unsigned int number_stream_elements,
    float alpha,
    float tolerance,
    struct spamm_multiply_stream_t *multiply_stream);

void
spamm_stream_external_sgemm (const unsigned int number_stream_elements,
    float alpha,
    float tolerance,
    struct spamm_multiply_stream_t *multiply_stream,
    const short call_external_sgemm);

#endif
