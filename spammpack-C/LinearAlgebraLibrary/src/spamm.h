/** @file */

#include "config.h"
#include <stdio.h>

/** \mainpage SpAMM
 *
 * \brief Sparse Approximate Matrix Multiply (SpAMM) library.
 *
 * SpAMM is a library for spare approximate matrices.
 *
 * \bug In spamm_multiply(): A pre-existing C matrix will not work right now.
 *
 * \author Nicolas Bock <nicolasbock@gmail.com>
 * \author Matt Challacombe <matt.challacombe@gmail.com>.
 */

/** Definition of global floating point precision. */
typedef FLOATING_PRECISION float_t;

/** The basic matrix data type.
 */
struct spamm_t
{
  /** Number of rows of matrix. */
  unsigned int M;

  /** Number of columns of matrix. */
  unsigned int N;

  /** Padded number of rows. */
  unsigned int M_padded;

  /** Padded number of columns. */
  unsigned int N_padded;

  /** Number of rows of the dense data block at the leaf level. */
  unsigned int M_block;

  /** Number of columns of the dense data block at the leaf level. */
  unsigned int N_block;

  /** Number of rows of subdivisions on each child node. */
  unsigned int M_child;

  /** Number of columns of subdivisions on each child node. */
  unsigned int N_child;

  /** The matrix element threshold.
   *
   * Elements below this threshold are not stored.
   */
  float_t threshold;

  /** The depth of the tree. */
  unsigned int tree_depth;

  /** Contiguous linear quadtree storage.
   *
   * The last linear_tiers tiers are stored in linear quadtree format and
   * allocated in contiguous chunks. This helps with the bankdwidth/latency
   * tradeoff during parallel data distribution. Legal value range:
   * linear_tiers >= 0.
   */
  unsigned int linear_tiers;

  /** The number of non-zero blocks. */
  unsigned int number_nonzero_blocks;

  /** The root node. */
  struct spamm_node_t *root;
};

/** Block ordering types.
 *
 * The block ordering type is used for block orderings such as Hilbert or
 * Peano curve ordering that are generated by rotation of a basic pattern.
 */
enum spamm_block_ordering_t
{
  /** No ordering. This is the default for new nodes. */
  none,

  /** P type ordering, which is the basic ordering pattern without rotation. */
  P,

  /** Q type ordering. */
  Q,

  /** R type ordering. */
  R,

  /** S type ordering. */
  S
};

/** A node in the tree.
 *
 * This structure describes a node in the tree.
 */
struct spamm_node_t
{
  /** The tree tier.
   *
   * The root is tier 0, the last tier is equal to the tree_depth.
   */
  unsigned int tier;

  /** Contiguous linear quadtree storage.
   *
   * The last linear_tiers tiers are stored in linear quadtree format and
   * allocated in contiguous chunks. This helps with the bankdwidth/latency
   * tradeoff during parallel data distribution. Legal value range:
   * linear_tiers >= 0.
   */
  unsigned int linear_tiers;

  /** The rows of the padded matrix covered in this node.
   *
   * The indices are meant as [M_lower, M_upper[, i.e. the upper limit is not
   * included in the interval.
   */
  unsigned int M_lower;

  /** The rows of the padded matrix covered in this node.
   *
   * The indices are meant as [M_lower, M_upper[, i.e. the upper limit is not
   * included in the interval.
   */
  unsigned int M_upper;

  /** The columns of the padded matrix covered in this node.
   *
   * The indices are meant as [N_lower, N_upper[, i.e. the upper limit is not
   * included in the interval.
   */
  unsigned int N_lower;

  /** The columns of the padded matrix covered in this node.
   *
   * The indices are meant as [N_lower, N_upper[, i.e. the upper limit is not
   * included in the interval.
   */
  unsigned int N_upper;

  /** The number of rows stored in the data blocks at the leaf level. */
  unsigned int M_block;

  /** The number of columns stored in the data blocks at the leaf level. */
  unsigned int N_block;

  /** The number of rows of subdivisions on each child node. */
  unsigned int M_child;

  /** The number of rows of subdivisions on each child node. */
  unsigned int N_child;

  /** The matrix element threshold.
   *
   * Elements below this threshold are not stored.
   */
  float_t threshold;

  /** The linear index of this block along the curve. */
  unsigned int index;

  /** The name of the block ordering pattern. */
  enum spamm_block_ordering_t ordering;

  /** Is this block loaded into the GPU? */
  short unsigned int block_loaded_in_GPU;

#if defined(HAVE_CUDA)
  /** Device pointers. */
  void *device_pointer;
#endif

  /** At the non-block level, pointers to the children nodes. */
  struct spamm_node_t **child;

  /** At the block level, the dense matrix data. */
  float_t *block_dense;
};

/** Tree statistics.
 *
 * This structure is the result of a call to spamm_tree_stats().
 */
struct spamm_tree_stats_t
{
  /** The number of nodes. */
  unsigned int number_nodes;

  /** The number of dense blocks. */
  unsigned int number_dense_blocks;

  /** The memory consumption of the tree. */
  unsigned int memory_tree;

  /** The memory consumption of the dense blocks. */
  unsigned int memory_dense_blocks;

  /** The average sparsity of the dense blocks. */
  float_t average_sparsity;
};

/** The multiply stream.
 *
 * The multiplication is mapped onto a linear stream of products, which can
 * then be evaluated by spamm_multiply_stream().
 */
struct spamm_multiply_stream_t
{
  /** Number of elements. */
  unsigned int number_elements;

  /** Links to the first node in list. */
  struct spamm_multiply_stream_node_t *first;

  /** Links to the last node in list. */
  struct spamm_multiply_stream_node_t *last;
};

/** A node in the multiply stream.
 */
struct spamm_multiply_stream_node_t
{
  /** Link to the previous element. */
  struct spamm_multiply_stream_node_t *previous;

  /** Link to the next element. */
  struct spamm_multiply_stream_node_t *next;

  /** Value of alpha. */
  float_t alpha;

  /** Value of beta. */
  float_t beta;

  /** Index of matrix A. */
  unsigned int A_index;

  /** Index of matrix B. */
  unsigned int B_index;

  /** Index of matrix C. */
  unsigned int C_index;

  /** Pointer to node corresponding to index of matrix A. */
  struct spamm_node_t *A_node;

  /** Pointer to node corresponding to index of matrix B. */
  struct spamm_node_t *B_node;

  /** Pointer to node corresponding to index of matrix C. */
  struct spamm_node_t *C_node;
};

void
spamm_log (const char *format, const char *filename, const int linenumber, ...);

int
spamm_dense_index (const int i, const int j, const int M, const int N);

void
spamm_new (const int M, const int N, const int M_block, const int N_block,
    const int M_child, const int N_child, const float_t threshold,
    struct spamm_t *A);

void
spamm_new_node (struct spamm_node_t **node);

void
spamm_delete (struct spamm_t *A);

void
spamm_dense_to_spamm (const int M, const int N, const int M_block,
    const int N_block, const int M_child, const int N_child,
    const float_t threshold, const float_t *A_dense, struct spamm_t *A);

void
spamm_spamm_to_dense (const struct spamm_t *A, float_t **A_dense);

float_t
spamm_get (const unsigned int i, const unsigned int j, const struct spamm_t *A);

int
spamm_set (const unsigned int i, const unsigned int j, const float_t Aij, struct spamm_t *A);

void
spamm_print_dense (const int M, const int N, const float_t *A_dense);

void
spamm_print_spamm (const struct spamm_t *A);

void
spamm_print_node (const struct spamm_node_t *node);

void
spamm_print_tree (const struct spamm_t *A);

void
spamm_add_node (const float_t alpha, const struct spamm_node_t *A_node, const float_t beta, struct spamm_node_t **B_node);

void
spamm_add (const float_t alpha, const struct spamm_t *A, const float_t beta, struct spamm_t *B);

void
spamm_multiply (const float_t alpha, const struct spamm_t *A, const struct spamm_t *B, const float_t beta, struct spamm_t *C);

void
spamm_tree_stats (struct spamm_tree_stats_t *stats, const struct spamm_t *A);

void
spamm_read_MM (const char *filename, const int M_block, const int N_block,
    const int M_child, const int N_child, const float_t threshold,
    struct spamm_t *A);

void
spamm_ll_new (struct spamm_multiply_stream_t *list);

void
spamm_ll_delete (struct spamm_multiply_stream_t *list);

void
spamm_ll_append (const float_t alpha, const float_t beta,
    const unsigned int A_index, struct spamm_node_t *A_node,
    const unsigned int B_index, struct spamm_node_t *B_node,
    const unsigned int C_index, struct spamm_node_t *C_node,
    struct spamm_multiply_stream_t *list);

struct spamm_multiply_stream_node_t *
spamm_ll_get (const unsigned int i, const struct spamm_multiply_stream_t *list);

void
spamm_ll_swap (struct spamm_multiply_stream_node_t **node1,
    struct spamm_multiply_stream_node_t **node2,
    struct spamm_multiply_stream_t *list);

void
spamm_ll_sort (struct spamm_multiply_stream_t *list);

void
spamm_ll_print_node_debug (const char *name, const struct spamm_multiply_stream_node_t *node);

void
spamm_ll_print_node (const struct spamm_multiply_stream_node_t *node);

void
spamm_ll_print (const struct spamm_multiply_stream_t *list);

void
spamm_ll_print_matlab (const struct spamm_multiply_stream_t *list);

unsigned int
spamm_number_nonzero (const struct spamm_t *A);
