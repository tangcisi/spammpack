#include <stdlib.h>
#include <xmmintrin.h>

#define A_OFFSET_11 (0*4+0)*64 // 0
#define A_OFFSET_12 (0*4+1)*64 // 64
#define A_OFFSET_13 (0*4+2)*64 // 128
#define A_OFFSET_14 (0*4+3)*64 // 192
#define A_OFFSET_21 (1*4+0)*64 // 256
#define A_OFFSET_22 (1*4+1)*64 // 320
#define A_OFFSET_23 (1*4+2)*64 // 384
#define A_OFFSET_24 (1*4+3)*64 // 448
#define A_OFFSET_31 (2*4+0)*64 // 512
#define A_OFFSET_32 (2*4+1)*64 // 576
#define A_OFFSET_33 (2*4+2)*64 // 640
#define A_OFFSET_34 (2*4+3)*64 // 704
#define A_OFFSET_41 (3*4+0)*64 // 768
#define A_OFFSET_42 (3*4+1)*64 // 832
#define A_OFFSET_43 (3*4+2)*64 // 896
#define A_OFFSET_44 (3*4+3)*64 // 960

#define B_OFFSET_11 (0*4+0)*16 // 0
#define B_OFFSET_12 (0*4+1)*16 // 16
#define B_OFFSET_13 (0*4+2)*16 // 32
#define B_OFFSET_14 (0*4+3)*16 // 48
#define B_OFFSET_21 (1*4+0)*16 // 64
#define B_OFFSET_22 (1*4+1)*16 // 80
#define B_OFFSET_23 (1*4+2)*16 // 96
#define B_OFFSET_24 (1*4+3)*16 // 112
#define B_OFFSET_31 (2*4+0)*16 // 128
#define B_OFFSET_32 (2*4+1)*16 // 144
#define B_OFFSET_33 (2*4+2)*16 // 160
#define B_OFFSET_34 (2*4+3)*16 // 176
#define B_OFFSET_41 (3*4+0)*16 // 192
#define B_OFFSET_42 (3*4+1)*16 // 208
#define B_OFFSET_43 (3*4+2)*16 // 224
#define B_OFFSET_44 (3*4+3)*16 // 240

#define C_OFFSET_11 (0*4+0)*16 // 0
#define C_OFFSET_12 (0*4+1)*16 // 16
#define C_OFFSET_13 (0*4+2)*16 // 32
#define C_OFFSET_14 (0*4+3)*16 // 48
#define C_OFFSET_21 (1*4+0)*16 // 64
#define C_OFFSET_22 (1*4+1)*16 // 80
#define C_OFFSET_23 (1*4+2)*16 // 96
#define C_OFFSET_24 (1*4+3)*16 // 112
#define C_OFFSET_31 (2*4+0)*16 // 128
#define C_OFFSET_32 (2*4+1)*16 // 144
#define C_OFFSET_33 (2*4+2)*16 // 160
#define C_OFFSET_34 (2*4+3)*16 // 176
#define C_OFFSET_41 (3*4+0)*16 // 192
#define C_OFFSET_42 (3*4+1)*16 // 208
#define C_OFFSET_43 (3*4+2)*16 // 224
#define C_OFFSET_44 (3*4+3)*16 // 240

struct multiply_stream_t
{
  float *A_block;
  float *B_block;
  float *C_block;
  float  norm[32];
};

void
stream_kernel_20_SSE_intrinsics (const unsigned int number_stream_elements,
    float alpha,
    float tolerance,
    struct multiply_stream_t *multiply_stream)
{
  short int i;
  unsigned int stream_index;
  unsigned int max_stream_index;

  float *restrict A;
  float *restrict B;
  float *restrict C;

  float *restrict norm;

  __m128 alpha_row;

  __m128 A_element;
  __m128 B_row;
  __m128 C_row[4];

  /* Divide number of stream elements by 64 to simulate stride of 64. */
  max_stream_index = number_stream_elements/64;

  alpha_row = _mm_set1_ps(alpha);

  for (stream_index = 0; stream_index < max_stream_index; stream_index++)
  {
    /* Load pointers to matrix data blocks. */
    A = multiply_stream[stream_index].A_block;
    B = multiply_stream[stream_index].B_block;
    C = multiply_stream[stream_index].C_block;
    norm = multiply_stream[stream_index].norm;

    /* Reset C(1,1) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[0]*norm[16] >= tolerance &&
        norm[1]*norm[20] >= tolerance &&
        norm[2]*norm[24] >= tolerance &&
        norm[3]*norm[28] >= tolerance)
    {
      /* A(1,1)*B(1,1) = C(1,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(1,2)*B(2,1) = C(1,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(1,3)*B(3,1) = C(1,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(1,4)*B(4,1) = C(1,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(1,1) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_11]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_11], C_row[i]);
    }

    /* Reset C(1,2) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[0]*norm[17] >= tolerance &&
        norm[1]*norm[21] >= tolerance &&
        norm[2]*norm[25] >= tolerance &&
        norm[3]*norm[29] >= tolerance)
    {
      /* A(1,1)*B(1,2) = C(1,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(1,2)*B(2,2) = C(1,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(1,3)*B(3,2) = C(1,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(1,4)*B(4,2) = C(1,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(1,2) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_12]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_12], C_row[i]);
    }

    /* Reset C(1,3) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[0]*norm[18] >= tolerance &&
        norm[1]*norm[22] >= tolerance &&
        norm[2]*norm[26] >= tolerance &&
        norm[3]*norm[30] >= tolerance)
    {
      /* A(1,1)*B(1,3) = C(1,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(1,2)*B(2,3) = C(1,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(1,3)*B(3,3) = C(1,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(1,4)*B(4,3) = C(1,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(1,3) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_13]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_13], C_row[i]);
    }

    /* Reset C(1,4) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[0]*norm[19] >= tolerance &&
        norm[1]*norm[23] >= tolerance &&
        norm[2]*norm[27] >= tolerance &&
        norm[3]*norm[31] >= tolerance)
    {
      /* A(1,1)*B(1,4) = C(1,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_11]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(1,2)*B(2,4) = C(1,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_12]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(1,3)*B(3,4) = C(1,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_13]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(1,4)*B(4,4) = C(1,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_14]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(1,4) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_14]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_14], C_row[i]);
    }

    /* Reset C(2,1) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[4]*norm[16] >= tolerance &&
        norm[5]*norm[20] >= tolerance &&
        norm[6]*norm[24] >= tolerance &&
        norm[7]*norm[28] >= tolerance)
    {
      /* A(2,1)*B(1,1) = C(2,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(2,2)*B(2,1) = C(2,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(2,3)*B(3,1) = C(2,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(2,4)*B(4,1) = C(2,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(2,1) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_21]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_21], C_row[i]);
    }

    /* Reset C(2,2) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[4]*norm[17] >= tolerance &&
        norm[5]*norm[21] >= tolerance &&
        norm[6]*norm[25] >= tolerance &&
        norm[7]*norm[29] >= tolerance)
    {
      /* A(2,1)*B(1,2) = C(2,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(2,2)*B(2,2) = C(2,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(2,3)*B(3,2) = C(2,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(2,4)*B(4,2) = C(2,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(2,2) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_22]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_22], C_row[i]);
    }

    /* Reset C(2,3) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[4]*norm[18] >= tolerance &&
        norm[5]*norm[22] >= tolerance &&
        norm[6]*norm[26] >= tolerance &&
        norm[7]*norm[30] >= tolerance)
    {
      /* A(2,1)*B(1,3) = C(2,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(2,2)*B(2,3) = C(2,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(2,3)*B(3,3) = C(2,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(2,4)*B(4,3) = C(2,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(2,3) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_23]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_23], C_row[i]);
    }

    /* Reset C(2,4) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[4]*norm[19] >= tolerance &&
        norm[5]*norm[23] >= tolerance &&
        norm[6]*norm[27] >= tolerance &&
        norm[7]*norm[31] >= tolerance)
    {
      /* A(2,1)*B(1,4) = C(2,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_21]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(2,2)*B(2,4) = C(2,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_22]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(2,3)*B(3,4) = C(2,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_23]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(2,4)*B(4,4) = C(2,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_24]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(2,4) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_24]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_24], C_row[i]);
    }

    /* Reset C(3,1) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[8]*norm[16] >= tolerance &&
        norm[9]*norm[20] >= tolerance &&
        norm[10]*norm[24] >= tolerance &&
        norm[11]*norm[28] >= tolerance)
    {
      /* A(3,1)*B(1,1) = C(3,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(3,2)*B(2,1) = C(3,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(3,3)*B(3,1) = C(3,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(3,4)*B(4,1) = C(3,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(3,1) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_31]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_31], C_row[i]);
    }

    /* Reset C(3,2) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[8]*norm[17] >= tolerance &&
        norm[9]*norm[21] >= tolerance &&
        norm[10]*norm[25] >= tolerance &&
        norm[11]*norm[29] >= tolerance)
    {
      /* A(3,1)*B(1,2) = C(3,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(3,2)*B(2,2) = C(3,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(3,3)*B(3,2) = C(3,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(3,4)*B(4,2) = C(3,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(3,2) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_32]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_32], C_row[i]);
    }

    /* Reset C(3,3) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[8]*norm[18] >= tolerance &&
        norm[9]*norm[22] >= tolerance &&
        norm[10]*norm[26] >= tolerance &&
        norm[11]*norm[30] >= tolerance)
    {
      /* A(3,1)*B(1,3) = C(3,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(3,2)*B(2,3) = C(3,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(3,3)*B(3,3) = C(3,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(3,4)*B(4,3) = C(3,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(3,3) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_33]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_33], C_row[i]);
    }

    /* Reset C(3,4) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[8]*norm[19] >= tolerance &&
        norm[9]*norm[23] >= tolerance &&
        norm[10]*norm[27] >= tolerance &&
        norm[11]*norm[31] >= tolerance)
    {
      /* A(3,1)*B(1,4) = C(3,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_31]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(3,2)*B(2,4) = C(3,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_32]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(3,3)*B(3,4) = C(3,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_33]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(3,4)*B(4,4) = C(3,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_34]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(3,4) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_34]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_34], C_row[i]);
    }

    /* Reset C(4,1) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[12]*norm[16] >= tolerance &&
        norm[13]*norm[20] >= tolerance &&
        norm[14]*norm[24] >= tolerance &&
        norm[15]*norm[28] >= tolerance)
    {
      /* A(4,1)*B(1,1) = C(4,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_11]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(4,2)*B(2,1) = C(4,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_21]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(4,3)*B(3,1) = C(4,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_31]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(4,4)*B(4,1) = C(4,1). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_41]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(4,1) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_41]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_41], C_row[i]);
    }

    /* Reset C(4,2) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[12]*norm[17] >= tolerance &&
        norm[13]*norm[21] >= tolerance &&
        norm[14]*norm[25] >= tolerance &&
        norm[15]*norm[29] >= tolerance)
    {
      /* A(4,1)*B(1,2) = C(4,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_12]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(4,2)*B(2,2) = C(4,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_22]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(4,3)*B(3,2) = C(4,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_32]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(4,4)*B(4,2) = C(4,2). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_42]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(4,2) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_42]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_42], C_row[i]);
    }

    /* Reset C(4,3) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[12]*norm[18] >= tolerance &&
        norm[13]*norm[22] >= tolerance &&
        norm[14]*norm[26] >= tolerance &&
        norm[15]*norm[30] >= tolerance)
    {
      /* A(4,1)*B(1,3) = C(4,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_13]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(4,2)*B(2,3) = C(4,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_23]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(4,3)*B(3,3) = C(4,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_33]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(4,4)*B(4,3) = C(4,3). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_43]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(4,3) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_43]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_43], C_row[i]);
    }

    /* Reset C(4,4) matrix accumulators */
    C_row[0] = _mm_setzero_ps();
    C_row[1] = _mm_setzero_ps();
    C_row[2] = _mm_setzero_ps();
    C_row[3] = _mm_setzero_ps();

    if (norm[12]*norm[19] >= tolerance &&
        norm[13]*norm[23] >= tolerance &&
        norm[14]*norm[27] >= tolerance &&
        norm[15]*norm[31] >= tolerance)
    {
      /* A(4,1)*B(1,4) = C(4,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_41]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_14]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(4,2)*B(2,4) = C(4,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_42]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_24]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(4,3)*B(3,4) = C(4,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_43]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_34]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }

      /* A(4,4)*B(4,4) = C(4,4). */
      for (i = 0; i < 4; i++)
      {
        A_element = _mm_load_ps(&A[(i*4+0)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[0*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+1)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[1*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+2)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[2*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);

        A_element = _mm_load_ps(&A[(i*4+3)*4+A_OFFSET_44]);
        B_row = _mm_load_ps(&B[3*4+B_OFFSET_44]);
        C_row[i] = _mm_add_ps(_mm_mul_ps(A_element, B_row), C_row[i]);
      }
    }

    /* Store C(4,4) block. */
    for (i = 0; i < 4; i++)
    {
      C_row[i] = _mm_mul_ps(alpha_row, C_row[i]);
      C_row[i] = _mm_add_ps(_mm_load_ps(&C[i*4+C_OFFSET_44]), C_row[i]);
      _mm_store_ps(&C[i*4+C_OFFSET_44], C_row[i]);
    }
  }
}
