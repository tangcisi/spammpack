#include "spamm_kernel.h"
#include "spamm_types_private.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

/** Get the name of the ith kernel.
 *
 * @param i The index of the kernel
 *
 * @return A pointer to a string that contains the name of the kernel.
 */
const char *
spamm_kernel_get_name (const unsigned int i)
{
  /* Available stream kernels (their names). */
  const char* spamm_stream_kernel_name [SPAMM_NUMBER_KERNELS] = {
    "kernel_external_sgemm",
    "kernel_stream_NULL",
    "kernel_standard_SSE",
    "kernel_standard_SSE_sparse",
    "kernel_standard_no_checks_SSE",
    "kernel_standard_SSE4_1",
    "kernel_standard_no_checks_SSE4_1",
    "kernel_Z_curve_SSE",
    "kernel_Z_curve_no_checks_SSE",
    "kernel_Z_curve_SSE4_1",
    "kernel_Z_curve_no_checks_SSE4_1",
    "kernel_hierarchical_SSE",
    "kernel_hierarchical_no_checks_SSE",
    "kernel_hierarchical_SSE4_1",
    "kernel_hierarchical_no_checks_SSE4_1",
    "kernel_hierarchical_Z_curve_SSE",
    "kernel_hierarchical_Z_curve_no_checks_SSE",
    "kernel_hierarchical_Z_curve_SSE4_1",
    "kernel_hierarchical_Z_curve_no_checks_SSE4_1"
  };

  if (i >= SPAMM_NUMBER_KERNELS)
  {
    printf("illegal kernel index\n");
    exit(1);
  }

  return spamm_stream_kernel_name[i];
}

/** Return the kernel from a name.
 *
 * @param name The name of the kernel.
 *
 * @return The kernel.
 */
enum spamm_kernel_t
spamm_kernel_get_kernel (const char* name)
{
  enum spamm_kernel_t kernel;

  if (strcasecmp(name, "kernel_external_sgemm") == 0)
  {
    kernel = kernel_external_sgemm;
  }

  else if (strcasecmp(name, "kernel_stream_NULL") == 0)
  {
    kernel = kernel_stream_NULL;
  }

  else if (strcasecmp(name, "kernel_standard_SSE") == 0)
  {
    kernel = kernel_standard_SSE;
  }

  else if (strcasecmp(name, "kernel_standard_SSE_sparse") == 0)
  {
    kernel = kernel_standard_SSE_sparse;
  }

  else if (strcasecmp(name, "kernel_standard_no_checks_SSE") == 0)
  {
    kernel = kernel_standard_no_checks_SSE;
  }

  else if (strcasecmp(name, "kernel_standard_SSE4_1") == 0)
  {
    kernel = kernel_standard_SSE4_1;
  }

  else if (strcasecmp(name, "kernel_standard_no_checks_SSE4_1") == 0)
  {
    kernel = kernel_standard_no_checks_SSE4_1;
  }

  else if (strcasecmp(name, "kernel_Z_curve_SSE") == 0)
  {
    kernel = kernel_Z_curve_SSE;
  }

  else if (strcasecmp(name, "kernel_Z_curve_no_checks_SSE") == 0)
  {
    kernel = kernel_Z_curve_no_checks_SSE;
  }

  else if (strcasecmp(name, "kernel_Z_curve_SSE4_1") == 0)
  {
    kernel = kernel_Z_curve_SSE4_1;
  }

  else if (strcasecmp(name, "kernel_Z_curve_no_checks_SSE4_1") == 0)
  {
    kernel = kernel_Z_curve_no_checks_SSE4_1;
  }

  else if (strcasecmp(name, "kernel_hierarchical_SSE") == 0)
  {
    kernel = kernel_hierarchical_SSE;
  }

  else if (strcasecmp(name, "kernel_hierarchical_no_checks_SSE") == 0)
  {
    kernel = kernel_hierarchical_no_checks_SSE;
  }

  else if (strcasecmp(name, "kernel_hierarchical_SSE4_1") == 0)
  {
    kernel = kernel_hierarchical_SSE4_1;
  }

  else if (strcasecmp(name, "kernel_hierarchical_no_checks_SSE4_1") == 0)
  {
    kernel = kernel_hierarchical_no_checks_SSE4_1;
  }

  else if (strcasecmp(name, "kernel_hierarchical_Z_curve_SSE") == 0)
  {
    kernel = kernel_hierarchical_Z_curve_SSE;
  }

  else if (strcasecmp(name, "kernel_hierarchical_Z_curve_no_checks_SSE") == 0)
  {
    kernel = kernel_hierarchical_Z_curve_no_checks_SSE;
  }

  else if (strcasecmp(name, "kernel_hierarchical_Z_curve_SSE4_1") == 0)
  {
    kernel = kernel_hierarchical_Z_curve_SSE4_1;
  }

  else if (strcasecmp(name, "kernel_hierarchical_Z_curve_no_checks_SSE4_1") == 0)
  {
    kernel = kernel_hierarchical_Z_curve_no_checks_SSE4_1;
  }

  else
  {
    printf("[get kernel] unknown kernel: %s\n", name);
    exit(1);
  }

  return kernel;
}

/** Return a kernel layout which the chosen kernel is compatible with.
 * Ignoring this "suggestion" is pretty much guaranteed to fail.
 *
 * @param kernel The kernel to use.
 *
 * @return The suggested layout for the basic matrix blocks at the kernel
 * tier.
 */
enum spamm_layout_t
spamm_kernel_suggest_layout (const enum spamm_kernel_t kernel)
{
  switch (kernel)
  {
    case kernel_external_sgemm:
    case kernel_stream_NULL:
      return dense_column_major;
      break;

    case kernel_standard_SSE:
    case kernel_standard_SSE_sparse:
    case kernel_standard_no_checks_SSE:
    case kernel_standard_SSE4_1:
    case kernel_standard_no_checks_SSE4_1:
    case kernel_hierarchical_SSE:
    case kernel_hierarchical_no_checks_SSE:
    case kernel_hierarchical_SSE4_1:
    case kernel_hierarchical_no_checks_SSE4_1:
      return row_major;
      break;

    case kernel_Z_curve_SSE:
    case kernel_Z_curve_no_checks_SSE:
    case kernel_Z_curve_SSE4_1:
    case kernel_Z_curve_no_checks_SSE4_1:
    case kernel_hierarchical_Z_curve_SSE:
    case kernel_hierarchical_Z_curve_no_checks_SSE:
    case kernel_hierarchical_Z_curve_SSE4_1:
    case kernel_hierarchical_Z_curve_no_checks_SSE4_1:
      return Z_curve;
      break;

    default:
      printf("[suggest layout] unknown kernel: %i\n", kernel);
      exit(1);
      break;
  }
}

/** Get a layout from a name.
 *
 * @param name The name of the layout.
 *
 * @return The layout.
 */
enum spamm_layout_t
spamm_kernel_get_layout (const char *name)
{
  enum spamm_layout_t layout;

  if (strcasecmp(name, "row_major") == 0)
  {
    layout = row_major;
  }

  else if (strcasecmp(name, "column_major") == 0)
  {
    layout = column_major;
  }

  else if (strcasecmp(name, "Z_curve") == 0)
  {
    layout = Z_curve;
  }

  else if (strcasecmp(name, "dense_column_major") == 0)
  {
    layout = dense_column_major;
  }

  else
  {
    printf("[spamm kernel get layout] unknown layout name: %s\n", name);
    exit(1);
  }

  return layout;
}
