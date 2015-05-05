/** @file
 *
 * The implementation of the chunk functions.
 *
 * @author Nicolas Bock <nicolas.bock@freeon.org>
 * @author Matt Challacombe <matt.challacombe@freeon.org>
 */

#include "config.h"

#include "chunk_block.h"

#include <assert.h>
#if defined(BLOCK_SLEEP) || defined(CHUNK_BLOCK_NO_WORK)
#include <time.h>
#endif
#include <emmintrin.h>
#include <stdio.h>

/** Calculate a row-major offset. */
#define ROW_MAJOR(i, j, N) ((i)*(N)+(j))

/** Calculate a column-major offset. */
#define COLUMN_MAJOR(i, j, N) ((i)+(j)*(N))

/** A basic matrix product. The matrix elements are assumed to be in row-major
 * order.
 *
 * @param A The pointer to matrix A.
 * @param B The pointer to matrix B.
 * @param C The pointer to matrix C.
 * @param N The size of the matrices.
 */
void
chunk_block_multiply_general (const double *const restrict A,
    const double *const restrict B,
    double *const restrict C,
    const int N)
{
  assert(A != NULL);
  assert(B != NULL);
  assert(C != NULL);

#ifdef CHUNK_BLOCK_TRANSPOSE
  if(N >= CHUNK_BLOCK_TRANSPOSE)
  {
    double *const restrict B_transpose = calloc(N*N, sizeof(double));
    for(int i = 0; i < N; i++)
    {
      for(int j = 0; j < N; j++)
      {
        B_transpose[COLUMN_MAJOR(i, j, N)] = B[COLUMN_MAJOR(j, i, N)];
      }
    }

    for(int i = 0; i < N; i++)
    {
      for(int j = 0; j < N; j++)
      {
        for(int k = 0; k < N; k++)
        {
          C[COLUMN_MAJOR(i, j, N)] += A[COLUMN_MAJOR(i, k, N)] * B_transpose[COLUMN_MAJOR(j, k, N)];
        }
      }
    }

    free(B_transpose);
  }

  else
  {
#endif
    for(int i = 0; i < N; i++)
    {
      for(int j = 0; j < N; j++)
      {
        for(int k = 0; k < N; k++)
        {
          C[COLUMN_MAJOR(i, j, N)] += A[COLUMN_MAJOR(i, k, N)] * B[COLUMN_MAJOR(k, j, N)];
        }
      }
    }
#ifdef CHUNK_BLOCK_TRANSPOSE
  }
#endif
}

/* Interface for the assembly function. */
void
chunk_block_multiply_4x4 (const double *const restrict A,
    const double *const restrict B,
    double *const restrict C);

void
chunk_block_multiply_4x4_2 (const double *const restrict A,
    const double *const restrict B,
    double *const restrict C)
{
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      for(int k = 0; k < 4; k++)
      {
        C[COLUMN_MAJOR(i, j, 4)] += A[COLUMN_MAJOR(i, k, 4)] * B[COLUMN_MAJOR(k, j, 4)];
      }
    }
  }
}

void
chunk_block_multiply_4x4_3 (const double *const restrict A,
    const double *const restrict B,
    double *const restrict C)
{
  for(int i = 0; i < 16; i += 4)
  {
    for(int j = 0; j < 4; j++)
    {
      C[i+j] += A[j+0]*B[i+0]
        + A[j+ 4]*B[i+1]
        + A[j+ 8]*B[i+2]
        + A[j+12]*B[i+3];
    }
  }
}

/** A basic matrix product. The matrix elements are assumed to be in row-major
 * order.
 *
 * @param A The pointer to matrix A.
 * @param B The pointer to matrix B.
 * @param C The pointer to matrix C.
 * @param N The size of the matrices.
 */
void
chunk_block_multiply (const double *const restrict A,
    const double *const restrict B,
    double *const restrict C,
    const int N)
{
#ifdef CHUNK_BLOCK_NO_WORK
#warning skipping block product
  struct timespec requested_sleep;
  requested_sleep.tv_sec = 0;
  requested_sleep.tv_nsec = 0.00001e9;

  nanosleep(&requested_sleep, NULL);
#else
  if(N == 4)
  {
    //chunk_block_multiply_4x4(A, B, C);
    chunk_block_multiply_4x4_2(A, B, C);
    //chunk_block_multiply_4x4_3(A, B, C);
  }

  else
  {
    chunk_block_multiply_general(A, B, C, N);
  }

#ifdef BLOCK_SLEEP
  /* Sleep to add some extra slowness. */
  struct timespec requested_sleep;
  requested_sleep.tv_sec = 0;
  requested_sleep.tv_nsec = BLOCK_SLEEP;

  nanosleep(&requested_sleep, NULL);
#endif

#ifdef EXTRA_WORK
  {
    int j = 0;
    for(int i = 0; i < EXTRA_WORK; i++)
    {
      j = j+1;
      if(j < 0) break;
      if(j > EXTRA_WORK) j = 0;
    }
    if(j < 0) printf("...\n");
  }
#endif
#endif
}
