#include "config.h"
#include "spamm.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>

/** Multiply a chunk with a scalar.
 *
 * @param alpha The factor.
 * @param chunk The chunk.
 *
 * @return The new squared norm.
 */
float
spamm_chunk_multiply_scalar (const float alpha,
    spamm_chunk_t *chunk)
{
  unsigned int i;
  unsigned int N_contiguous;
  unsigned int number_dimensions;

  float *A;
  float *norm;
  float *norm2;

  if(chunk == NULL) { return 0.0; }

  number_dimensions = *spamm_chunk_get_number_dimensions(chunk);
  N_contiguous = spamm_chunk_get_N_contiguous(chunk);
  A = spamm_chunk_get_matrix(chunk);

  for(i = 0; i < ipow(N_contiguous, number_dimensions); i++)
  {
    A[i] *= alpha;
  }

  norm = spamm_chunk_get_norm(chunk);
  norm2 = spamm_chunk_get_norm2(chunk);
  for(i = 0; i < 0; i++)
  {
    norm[i] *= alpha;
    norm2[i] *= alpha*alpha;
  }

  return norm2[0];
}

float
spamm_chunk_multiply (const float tolerance,
    const float alpha,
    spamm_chunk_t *chunk_A,
    spamm_chunk_t *chunk_B,
    spamm_chunk_t *chunk_C,
    const unsigned int tier,
    const unsigned int chunk_tier,
    const unsigned int linear_index_A,
    const unsigned int linear_index_B,
    const unsigned int linear_index_C,
    struct spamm_timer_t *timer,
    sgemm_func sgemm,
    const enum spamm_kernel_t kernel)
{
  unsigned int i, j, k;

  unsigned int number_dimensions_A;
  unsigned int number_dimensions_B;
  unsigned int number_dimensions_C;

  unsigned int new_linear_index_A;
  unsigned int new_linear_index_B;
  unsigned int new_linear_index_C;

  float *norm_A;
  float *norm_B;

  float *norm_C;
  float *norm2_C;

  float alpha_sgemm = alpha;
  float beta = 1.0;

  int N_contiguous;

  float *matrix_A;
  float *matrix_B;
  float *matrix_C;

  short use_linear_tree;

  unsigned int number_dimensions;

  if(chunk_A == NULL || chunk_B == NULL) { return 0.0; }

  use_linear_tree = *spamm_chunk_get_use_linear_tree(chunk_A);
  number_dimensions = *spamm_chunk_get_number_dimensions(chunk_A);

  if(use_linear_tree)
  {
    return spamm_linear_multiply(tolerance, alpha, chunk_A, chunk_B, beta, chunk_C, timer);
  }

  else
  {
    norm_A = spamm_chunk_get_norm(chunk_A);
    norm_B = spamm_chunk_get_norm(chunk_B);

    norm_C = spamm_chunk_get_norm(chunk_C);
    norm2_C = spamm_chunk_get_norm2(chunk_C);

    if(norm_A[0]*norm_B[0] > tolerance)
    {
      matrix_A = spamm_chunk_get_matrix(chunk_A);
      matrix_B = spamm_chunk_get_matrix(chunk_B);
      matrix_C = spamm_chunk_get_matrix(chunk_C);

      N_contiguous = spamm_chunk_get_N_contiguous(chunk_A);

      if(sgemm)
      {
        sgemm("N", "N", &N_contiguous, &N_contiguous, &N_contiguous,
            &alpha_sgemm, matrix_A, &N_contiguous, matrix_B, &N_contiguous,
            &beta, matrix_C, &N_contiguous);
      }

      else
      {
        /* Braindead multiply in nested loops. */
        for(i = 0; i < N_contiguous; i++) {
          for(j = 0; j < N_contiguous; j++) {
            for(k = 0; k < N_contiguous; k++)
            {
              matrix_C[spamm_index_column_major(i, j, N_contiguous, N_contiguous)] += alpha
                *matrix_A[spamm_index_column_major(i, k, N_contiguous, N_contiguous)]
                *matrix_B[spamm_index_column_major(k, j, N_contiguous, N_contiguous)];
            }
          }
        }
      }

      /* Update norm on C. */
      norm2_C[0] = 0;
      for(i = 0; i < ipow(N_contiguous, number_dimensions); i++)
      {
        norm2_C[0] += ipow(matrix_C[i], 2);
      }
      norm_C[0] = sqrt(norm2_C[0]);
    }

    return norm_C[0];
  }
}

void
spamm_chunk_add (const float alpha,
    spamm_chunk_t **A,
    const float beta,
    spamm_chunk_t *B)
{
  float *A_matrix;
  float *B_matrix;

  unsigned int i;
  unsigned int N_contiguous;
  unsigned int number_dimensions;

  A_matrix = spamm_chunk_get_matrix(*A);
  B_matrix = spamm_chunk_get_matrix(B);

  N_contiguous = spamm_chunk_get_N_contiguous(B);
  number_dimensions = *spamm_chunk_get_number_dimensions(B);

  for(i = 0; i < ipow(N_contiguous, number_dimensions); i++)
  {
    A_matrix[i] = alpha*A_matrix[i]+beta*B_matrix[i];
  }
}

/** Pad memory address to some alignment.
 *
 * @param address Address to pad.
 * @param alignment The byte boundary to align to.
 *
 * @return The aligned address.
 */
size_t
spamm_chunk_pad (const size_t address,
    const size_t alignment)
{
  size_t new_address = address & (-alignment);

  if(new_address != address)
  {
    new_address += alignment;
  }

  return new_address;
}

/** Get the number_dimensions.
 *
 * @param chunk The chunk.
 *
 * @return The number_dimensions.
 */
unsigned int *
spamm_chunk_get_number_dimensions (spamm_chunk_t *chunk)
{
  return (unsigned int*) chunk;
}

/** Get N_block.
 *
 * @param chunk The chunk.
 *
 * @return number_tiers
 */
unsigned int *
spamm_chunk_get_number_tiers (spamm_chunk_t *chunk)
{
  return (unsigned int*) ((intptr_t) chunk + sizeof(unsigned int));
}

/** Get use_linear_tree.
 *
 * @param chunk The chunk.
 *
 * @return use_linear_tree.
 */
unsigned int *
spamm_chunk_get_use_linear_tree (spamm_chunk_t *chunk)
{
  return (unsigned int*) ((intptr_t) chunk + 2*sizeof(unsigned int));
}

/** Get the address of the N array.
 *
 * @param chunk The chunk.
 *
 * @return The N array.
 */
unsigned int *
spamm_chunk_get_N (spamm_chunk_t *chunk)
{
  void **chunk_pointer = (void*) ((intptr_t) chunk + 4*sizeof(unsigned int));
  return (unsigned int*) ((intptr_t) chunk + (intptr_t) chunk_pointer[0]);
}

/** Get the address of the N_lower array.
 *
 * @param chunk The chunk.
 *
 * @return The N_lower array.
 */
unsigned int *
spamm_chunk_get_N_lower (spamm_chunk_t *chunk)
{
  void **chunk_pointer = (void*) ((intptr_t) chunk + 4*sizeof(unsigned int));
  return (unsigned int*) ((intptr_t) chunk + (intptr_t) chunk_pointer[1]);
}

/** Get the address of the N_upper array.
 *
 * @param chunk The chunk.
 *
 * @return The N_upper array.
 */
unsigned int *
spamm_chunk_get_N_upper (spamm_chunk_t *chunk)
{
  void **chunk_pointer = (void*) ((intptr_t) chunk + 4*sizeof(unsigned int));
  return (unsigned int*) ((intptr_t) chunk + (intptr_t) chunk_pointer[2]);
}

/** Get the size of the contiguous matrix block.
 *
 * @param chunk The chunk.
 *
 * @return The size of the contiguous matrix.
 */
unsigned int
spamm_chunk_get_N_contiguous (spamm_chunk_t *chunk)
{
  unsigned int *N_lower;
  unsigned int *N_upper;

  N_lower = spamm_chunk_get_N_lower(chunk);
  N_upper = spamm_chunk_get_N_upper(chunk);

  return N_upper[0]-N_lower[0];
}

/** Get the address of matrix block.
 *
 * @param chunk The chunk.
 *
 * @return The address of the matrix.
 */
float *
spamm_chunk_get_matrix (spamm_chunk_t *chunk)
{
  void **chunk_pointer = (void*) ((intptr_t) chunk + 4*sizeof(unsigned int));
  return (float*) ((intptr_t) chunk + (intptr_t) chunk_pointer[3]);
}

/** Get the address of dilated matrix block.
 *
 * @param chunk The chunk.
 *
 * @return The address of the dilated matrix.
 */
float *
spamm_chunk_get_matrix_dilated (spamm_chunk_t *chunk)
{
  void **chunk_pointer = (void*) ((intptr_t) chunk + 4*sizeof(unsigned int));
  return (float*) ((intptr_t) chunk + (intptr_t) chunk_pointer[4]);
}

/** Get the total number of norm entries across all tiers stored in the SpAMM
 * chunk.
 *
 * @param chunk The chunk.
 *
 * @return The total number of entries.
 */
unsigned int
spamm_chunk_get_number_norm_entries (spamm_chunk_t *chunk)
{
  unsigned int result = 0;
  unsigned int number_dimensions;
  unsigned int tier;

  number_dimensions = *spamm_chunk_get_number_dimensions(chunk);
  for(tier = 0; tier < *spamm_chunk_get_number_tiers(chunk); tier++)
  {
    result += ipow(ipow(2, number_dimensions), tier);
  }

  return result;
}

/** Get a pointer to the norm arrays.
 *
 * @param chunk The chunk.
 *
 * @return The pointer to the norm arrays.
 */
float *
spamm_chunk_get_norm (spamm_chunk_t *chunk)
{
  void **chunk_pointer = (void*) ((intptr_t) chunk + 4*sizeof(unsigned int));
  return (float*) ((intptr_t) chunk + (intptr_t) chunk_pointer[5]);
}

/** Calculate the starting address of the norm arrays inside a SpAMM chunk.
 * The norm arrays start at tier == chunk_tier, with one entry, and
 * generally have pow(pow(2, number_dimensions), tier-chunk_tier)
 * entries.
 *
 * @param tier The tier.
 * @param chunk The chunk.
 *
 * @return A pointer to the start of the norm chunk at this tier.
 */
float *
spamm_chunk_get_tier_norm (const unsigned int tier,
    spamm_chunk_t *chunk)
{
  float *norm;
  unsigned int number_dimensions;
  unsigned int offset = 0;

  unsigned int i;

  norm = spamm_chunk_get_norm(chunk);
  number_dimensions = *spamm_chunk_get_number_dimensions(chunk);

  for(i = 0; i < tier; i++)
  {
    offset += ipow(ipow(2, number_dimensions), i);
  }

  return &norm[offset];
}

/** Get a pointer to the square of the norm arrays.
 *
 * @param chunk The chunk.
 *
 * @return The pointer to the norm2 arrays.
 */
float *
spamm_chunk_get_norm2 (spamm_chunk_t *chunk)
{
  void **chunk_pointer = (void*) ((intptr_t) chunk + 4*sizeof(unsigned int));
  return (float*) ((intptr_t) chunk + (intptr_t) chunk_pointer[6]);
}

/** Calculate the starting address of the norm2 arrays inside a SpAMM chunk.
 * The norm arrays start at tier == chunk_tier, with one entry, and
 * generally have pow(pow(2, number_dimensions), tier-chunk_tier)
 * entries.
 *
 * @param tier The tier.
 * @param chunk The chunk.
 *
 * @return A pointer to the start of the norm chunk at this tier.
 */
float *
spamm_chunk_get_tier_norm2 (const unsigned int tier,
    spamm_chunk_t *chunk)
{
  float *norm2;
  unsigned int number_dimensions;
  unsigned int offset = 0;

  unsigned int i;

  norm2 = spamm_chunk_get_norm2(chunk);
  number_dimensions = *spamm_chunk_get_number_dimensions(chunk);

  for(i = 0; i < tier; i++)
  {
    offset += ipow(ipow(2, number_dimensions), i);
  }

  return &norm2[offset];
}

/** Calculate a linear offset into a SpAMM chunk matrix. The matrix data is
 * stored in a matrix of width N_contiguous, of submatrices of width N_block.
 * Storage order is column-major (for reasons of compatibilty with Fortran).
 *
 * @param number_dimensions The number of dimensions.
 * @param N_block The block size.
 * @param N_lower The bounding box upper index array.
 * @param i The index array.
 *
 * @return The linear offset into the matrix data.
 */
unsigned int
spamm_chunk_matrix_index (const unsigned int number_dimensions,
    const unsigned int N_block,
    const unsigned int *const N_lower,
    const unsigned int *const i)
{
  unsigned int offset;
  unsigned int block_offset = 0;
  unsigned int *i_temp;

  int dim;

  i_temp = calloc(number_dimensions, sizeof(unsigned int));
  for(dim = 0; dim < number_dimensions; dim++)
  {
    i_temp[dim] = (i[dim]-N_lower[dim])/N_block;
  }
  offset = ipow(N_block, number_dimensions)*spamm_index_linear(number_dimensions, i_temp);
  free(i_temp);

  for(dim = number_dimensions-1; dim >= 1; dim--)
  {
    block_offset = N_block*(block_offset+(i[dim]-N_lower[dim])%N_block);
  }
  block_offset += (i[0]-N_lower[0])%N_block;
  offset += block_offset;

  return offset;
}

/** Get the size of a SpAMM data chunk.
 *
 * See the documentation for spamm_new_chunk() for a detailed description of
 * the contents of a SpAMM data chunk.
 *
 * @param number_dimensions [in] The number of dimensions.
 * @param use_linear_tree [in] Whether to use the linear tree code or not.
 * @param number_tiers [out] The number of tiers stored in this chunk. If
 * use_linear_tree then the chunk matrix size is >= SPAMM_N_KERNEL, and the
 * number of tiers is down to SPAMM_N_BLOCK.
 * @param N [in] The array of the original, unpadded matrix size.
 * @param N_lower [in] The array of the bounding box.
 * @param N_upper [in] The array of the bounding box.
 * @param N_lower_pointer [out] Pointer to N_lower[].
 * @param N_upper_pointer [out] Pointer to N_upper[].
 * @param A_pointer [out] Pointer to A.
 * @param A_dilated_pointer [out] Pointer to A_dilated.
 * @param norm_pointer [out] Pointer to norm[].
 * @param norm2_pointer [out] Pointer to norm2[].
 *
 * @return The size in bytes of the chunk.
 */
size_t
spamm_chunk_get_size (const unsigned int number_dimensions,
    const short use_linear_tree,
    unsigned int *number_tiers,
    const unsigned int *const N,
    const unsigned int *const N_lower,
    const unsigned int *const N_upper,
    unsigned int **N_pointer,
    unsigned int **N_lower_pointer,
    unsigned int **N_upper_pointer,
    float **A_pointer,
    float **A_dilated_pointer,
    float **norm_pointer,
    float **norm2_pointer)
{
  unsigned int tier;
  unsigned int N_contiguous;
  int dim;

  double tier_temp;

  size_t size = 0;

  /* Calculate sizes needed. */
  for(dim = 0, N_contiguous = 0; dim < number_dimensions; dim++)
  {
    if(N_contiguous == 0)
    {
      N_contiguous = N_upper[dim]-N_lower[dim];
    }

    if(N_upper[dim]-N_lower[dim] != N_contiguous)
    {
      SPAMM_FATAL("only square shaped regions are supported (N_upper[%u]-N_lower[%u] = %u, N_contiguous = %u)\n",
          dim, dim, N_upper[dim]-N_lower[dim], N_contiguous);
    }
  }

  /* Calculate the number of tiers stored here. */
  if(use_linear_tree)
  {
    if(N_contiguous < SPAMM_N_KERNEL)
    {
      SPAMM_FATAL("logic error, N_contiguous (%u) has to be at least %u\n", N_contiguous, SPAMM_N_KERNEL);
    }
    tier_temp = log(N_contiguous/SPAMM_N_KERNEL)/log(2)+1;

    if(tier_temp-round(tier_temp) < 1e-10)
    {
      *number_tiers = (unsigned int) round(tier_temp);
    }

    else
    {
      *number_tiers = (unsigned int) ceil(tier_temp);
    }

    /* Add more tiers for norms at SPAMM_N_BLOCK. */
    *number_tiers += SPAMM_KERNEL_DEPTH;

    if(SPAMM_N_KERNEL*ipow(2, *number_tiers-1-SPAMM_KERNEL_DEPTH) != N_contiguous)
    {
      SPAMM_FATAL("logic error, number_tiers = %u, N_contiguous = %u, %u*2^%u = %u\n",
          *number_tiers, N_contiguous, SPAMM_N_KERNEL, *number_tiers-1,
          SPAMM_N_KERNEL*ipow(2, *number_tiers-1));
    }
  }

  else
  {
    *number_tiers = 1;
  }

  /* Pointers to fields. */
  size += sizeof(unsigned int);  /* number_dimensions */
  size += sizeof(unsigned int);  /* number_tiers */
  size += sizeof(unsigned int);  /* use_linear_tree */
  size += sizeof(unsigned int);  /* Padding. */

  size += sizeof(unsigned int*); /* N_pointer */
  size += sizeof(unsigned int*); /* N_lower_pointer */
  size += sizeof(unsigned int*); /* N_upper_pointer */
  size += sizeof(float*);        /* A_pointer */
  size += sizeof(float*);        /* A_dilated_pointer */
  size += sizeof(float*);        /* norm_pointer */
  size += sizeof(float*);        /* norm2_pointer */

  /* Fields. */
  *N_pointer       = (unsigned int*) size; size += number_dimensions*sizeof(unsigned int); /* N[number_dimensions] */
  *N_lower_pointer = (unsigned int*) size; size += number_dimensions*sizeof(unsigned int); /* N_lower[number_dimensions] */
  *N_upper_pointer = (unsigned int*) size; size += number_dimensions*sizeof(unsigned int); /* N_upper[number_dimensions] */

  /* Pad. */
  size = spamm_chunk_pad(size, SPAMM_ALIGNMENT);

  /* Matrix data. */
  *A_pointer = (float*) size; size += ipow(N_contiguous, number_dimensions)*sizeof(float); /* A[ipow(N_contiguous, number_dimensions)] */

  /* Pad. */
  size = spamm_chunk_pad(size, SPAMM_ALIGNMENT);

  /* Dilated matrix data. */
  *A_dilated_pointer = (float*) size; size += 4*ipow(N_contiguous, number_dimensions)*sizeof(float); /* A_dilated[4*N_contiguous, number_dimensions)] */

  /* Norm. */
  *norm_pointer = (float*) size;

  /* Add up all tiers. */
  for(tier = 0; tier < *number_tiers; tier++)
  {
    size += ipow(ipow(2, number_dimensions), tier)*sizeof(float);
  }

  /* Squared norm. */
  *norm2_pointer = (float*) size;

  /* Add up all tiers. */
  for(tier = 0; tier < *number_tiers; tier++)
  {
    size += ipow(ipow(2, number_dimensions), tier)*sizeof(float);
  }

  return size;
}
