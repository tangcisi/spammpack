/** @file */

#if ! defined(__SPAMM_H)

/** Define in case spamm.h has been included. */
#define __SPAMM_H 1

#include "config.h"
#include "spamm_ll.h"
#include "spamm_mm.h"
#include <stdio.h>

/** Definition of global floating point precision.
 *
 * This is either float or double, as defined by the configure script.
 */
typedef FLOATING_PRECISION floating_point_t;

/** The severity levels for the logger.
 */
enum spamm_log_severity_t
{
  /** A fatal message that should always be displayed. */
  fatal,

  /** A message that might be printed if the global loglevel is set to at
   * least info. */
  info,

  /** A message that only gets printed if the global loglevel is set to debug.
   */
  debug
};

/** Define shortcut macro for logging.
 *
 * Typical use of this macro:
 *
 * <code>LOG_FATAL("opening new file: %s\n", filename);</code>
 *
 * Compare to spamm_log().
 *
 * @param format Format string. See printf() for a detailed description of its
 *               syntax.
 */
#define LOG_FATAL(format, ...) spamm_log(fatal, format, __FILE__, __LINE__, __VA_ARGS__)

/** Define shortcut macro for logging.
 *
 * Typical use of this macro:
 *
 * <code>LOG2_FATAL("a message without arguments\n");</code>
 *
 * Compare to spamm_log().
 *
 * @param format Format string. See printf() for a detailed description of its
 *               syntax.
 */
#define LOG2_FATAL(format) spamm_log(fatal, format, __FILE__, __LINE__)

/** Define shortcut macro for logging.
 *
 * Typical use of this macro:
 *
 * <code>LOG_INFO("opening new file: %s\n", filename);</code>
 *
 * Compare to spamm_log().
 *
 * @param format Format string. See printf() for a detailed description of its
 *               syntax.
 */
#define LOG_INFO(format, ...) spamm_log(info, format, __FILE__, __LINE__, __VA_ARGS__)

/** Define shortcut macro for logging.
 *
 * Typical use of this macro:
 *
 * <code>LOG2_INFO("a message without arguments\n");</code>
 *
 * Compare to spamm_log().
 *
 * @param format Format string. See printf() for a detailed description of its
 *               syntax.
 */
#define LOG2_INFO(format) spamm_log(info, format, __FILE__, __LINE__)

/** Define shortcut macro for logging.
 *
 * Typical use of this macro:
 *
 * <code>LOG_DEBUG("opening new file: %s\n", filename);</code>
 *
 * Compare to spamm_log().
 *
 * @param format Format string. See printf() for a detailed description of its
 *               syntax.
 */
#define LOG_DEBUG(format, ...) spamm_log(debug, format, __FILE__, __LINE__, __VA_ARGS__)

/** Define shortcut macro for logging.
 *
 * Typical use of this macro:
 *
 * <code>LOG2_DEBUG("a message without arguments\n");</code>
 *
 * Compare to spamm_log().
 *
 * @param format Format string. See printf() for a detailed description of its
 *               syntax.
 */
#define LOG2_DEBUG(format) spamm_log(debug, format, __FILE__, __LINE__)

/* Definition of return codes. */

/** Return code: Everything went fine. */
#define SPAMM_RESULT_OK 0

/** Return code: Something went wrong. */
#define SPAMM_RESULT_FAILED -1

/** Return code: Something went wrong. */
#define SPAMM_RESULT_BELOW_THRESHOLD 1

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
  floating_point_t threshold;

  /** The depth of the tree. */
  unsigned int tree_depth;

  /** Contiguous linear quadtree storage.
   *
   * Nodes in tier >= linear_tier are stored in linear quadtree format and
   * allocated in contiguous chunks. This helps with the bandwidth/latency
   * tradeoff during parallel data distribution. Legal value range:
   * linear_tier > 0.
   */
  unsigned int linear_tier;

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

/** Multiply algorithm.
 */
enum spamm_multiply_algorithm_t
{
  /** Only tree. */
  tree,

  /** Generate cached multiply stream. */
  cache
};

/** The mask to apply to the linear matrix block index.
 */
enum spamm_linear_mask_t
{
  /** Apply i-Mask, i.e. remove the j component. */
  i_mask,

  /** Apply j-Mask, i.e. remove the i component. */
  j_mask
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

  /** The depth of the tree. */
  unsigned int tree_depth;

  /** Contiguous linear quadtree storage.
   *
   * Nodes in tier >= linear_tier are stored in linear quadtree format and
   * allocated in contiguous chunks. This helps with the bandwidth/latency
   * tradeoff during parallel data distribution. Legal value range:
   * linear_tier > 0.
   */
  unsigned int linear_tier;

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
  floating_point_t threshold;

  /** The linear index of this block along the curve. */
  unsigned int index;

  /** Link to previous spamm_node_t in linear quadtree in i sorting.
   *
   * In tier >= linear_tier the tree nodes are also linked in a linear
   * quadtree. The linear tree is sorted on i and on j. This is the i part of
   * the links.
   */
  struct spamm_node_t *previous_i;

  /** Link to next spamm_node_t in linear quadtree in i sorting.
   *
   * In tier >= linear_tier the tree nodes are also linked in a linear
   * quadtree. The linear tree is sorted on i and on j. This is the i part of
   * the links.
   */
  struct spamm_node_t *next_i;

  /** Link to previous spamm_node_t in linear quadtree in j sorting.
   *
   * In tier >= linear_tier the tree nodes are also linked in a linear
   * quadtree. The linear tree is sorted on i and on j. This is the j part of
   * the links.
   */
  struct spamm_node_t *previous_j;

  /** Link to next spamm_node_t in linear quadtree in j sorting.
   *
   * In tier >= linear_tier the tree nodes are also linked in a linear
   * quadtree. The linear tree is sorted on i and on j. This is the j part of
   * the links.
   */
  struct spamm_node_t *next_j;

  /** The name of the block ordering pattern. */
  enum spamm_block_ordering_t ordering;

  /** The linear quadtree.
   *
   * A packed tree stores a linear quadtree at the linear_tier, replacing the
   * quadtree that was there before. The elements of the linked list at
   * linear_quadtree are of type spamm_linear_quadtree_t.
   */
  struct spamm_ll_t *linear_quadtree;

  /** The linear quadtree memory chunksize.
   *
   * This is set here to provide a default value for the creation of linear
   * quadtrees. Possibly should be moved someplace else.
   */
  unsigned int linear_quadtree_default_chunksize;

  /** The memory that holds the linear quadtree.
   */
  struct spamm_mm_t *linear_quadtree_memory;

#if defined(HAVE_CUDA)
  /** Device pointers. */
  void *device_pointer;
#endif

  /** At the non-block level, pointers to the children nodes.
   *
   * The pointers can be accessed in 2 ways:
   *
   * - As a 2-D array using spamm_dense_index()
   * - As a 1-D array which is sorted on the index.
   */
  struct spamm_node_t **child;

  /** At the block level, the dense matrix data. */
  floating_point_t *block_dense;
};

/** Linear quadtree.
 */
struct spamm_linear_quadtree_t
{
  /** The linear quadtree index of this data block.
   */
  unsigned int index;

  /** The number of rows of the dense matrix block. */
  unsigned int M;

  /** The number of columns of the dense data block. */
  unsigned int N;

  /** The data.
   */
  floating_point_t *block_dense;
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
  floating_point_t average_sparsity;
};

/** The multiply stream.
 *
 * The multiplication is mapped onto a linear stream of products, which can
 * then be evaluated by spamm_multiply_stream().
 */
struct spamm_multiply_stream_element_t
{
  /** Value of alpha. */
  floating_point_t alpha;

  /** Value of beta. */
  floating_point_t beta;

  /** Index of matrix A. */
  unsigned int A_index;

  /** Index of matrix B. */
  unsigned int B_index;

  /** Index of matrix C. */
  unsigned int C_index;

  /** Number of rows of dense matrix block of A. */
  unsigned int M_A;

  /** Number of columns of dense matrix block of A. */
  unsigned int N_A;

  /** Number of rows of dense matrix block of B. */
  unsigned int M_B;

  /** Number of columns of dense matrix block of B. */
  unsigned int N_B;

  /** Number of rows of dense matrix block of C. */
  unsigned int M_C;

  /** Number of columns of dense matrix block of C. */
  unsigned int N_C;

  /** Is block A loaded into the GPU? */
  short unsigned int block_A_loaded_in_GPU;

  /** Is block B loaded into the GPU? */
  short unsigned int block_B_loaded_in_GPU;

  /** Is block C loaded into the GPU? */
  short unsigned int block_C_loaded_in_GPU;

  /** Pointer to dense matrix block of A. */
  floating_point_t *A_block_dense;

  /** Pointer to dense matrix block of B. */
  floating_point_t *B_block_dense;

  /** Pointer to dense matrix block of C. */
  floating_point_t *C_block_dense;
};

/* Function declarations. */

void
spamm_add (const floating_point_t alpha, const struct spamm_t *A, const floating_point_t beta, struct spamm_t *B);

void
spamm_add_node (const floating_point_t alpha, const struct spamm_node_t *A_node, const floating_point_t beta, struct spamm_node_t **B_node);

int
spamm_compare_int (const void *integer1, const void *integer2);

int
spamm_compare_multiply_stream_element (const void *element1, const void *element2);

void
spamm_delete (struct spamm_t *A);

void
spamm_delete_node (struct spamm_node_t **node);

int
spamm_dense_index (const unsigned int i, const unsigned int j,
    const unsigned int M, const unsigned int N);

void
spamm_dense_to_spamm (const unsigned int M, const unsigned int N,
    const unsigned int M_block, const unsigned int N_block,
    const unsigned int M_child, const unsigned int N_child,
    const floating_point_t threshold, const floating_point_t *A_dense,
    struct spamm_t *A);

floating_point_t
spamm_get (const unsigned int i, const unsigned int j, const struct spamm_t *A);

enum spamm_log_severity_t
spamm_get_loglevel ();

void
spamm_int_to_binary (const unsigned int integer, const int width, char *binary_string);

void
spamm_linear_to_coordinates (const unsigned int index, unsigned int *i,
    unsigned int *j, const unsigned int M, const unsigned int N,
    const unsigned int M_block, const unsigned int N_block);

void
spamm_log (const enum spamm_log_severity_t severity, const char *format,
    const char *filename, const unsigned int linenumber, ...);

unsigned int
spamm_mask (const unsigned int index, const unsigned int width,
    const enum spamm_linear_mask_t mask);

void
spamm_multiply (const enum spamm_multiply_algorithm_t algorithm,
    const floating_point_t alpha, const struct spamm_t *A,
    const struct spamm_t *B, const floating_point_t beta, struct spamm_t *C);

void
spamm_multiply_scalar (const floating_point_t alpha, struct spamm_t *A);

void
spamm_new (const unsigned int M, const unsigned int N,
    const unsigned int M_block, const unsigned int N_block,
    const unsigned int M_child, const unsigned int N_child,
    const floating_point_t threshold, struct spamm_t *A);

struct spamm_linear_quadtree_t*
spamm_new_linear_quadtree_node (const unsigned int M, const unsigned int N,
    struct spamm_mm_t *memory);

struct spamm_node_t *
spamm_new_node ();

unsigned int
spamm_number_nonzero (const struct spamm_t *A);

void
spamm_print_dense (const unsigned int M, const unsigned int N, const floating_point_t *A_dense);

void
spamm_print_multiply_stream (const struct spamm_ll_t *stream);

void
spamm_print_node (const struct spamm_node_t *node);

void
spamm_print_spamm (const struct spamm_t *A);

void
spamm_print_tree (const struct spamm_t *A);

void
spamm_read_MM (const char *filename,
    const unsigned int M_block, const unsigned int N_block,
    const unsigned int M_child, const unsigned int N_child,
    const floating_point_t threshold, struct spamm_t *A);

int
spamm_set (const unsigned int i, const unsigned int j, const floating_point_t Aij, struct spamm_t *A);

void
spamm_set_loglevel (const enum spamm_log_severity_t loglevel);

void
spamm_sgemm_trivial (const char opA, const char opB,
    const unsigned int M, const unsigned int N, const unsigned int K,
    const floating_point_t alpha,
    const floating_point_t *A_block_dense,
    const unsigned int lda,
    const floating_point_t *B_block_dense,
    const unsigned int ldb,
    const floating_point_t beta,
    floating_point_t *C_block_dense,
    const unsigned int ldc);

void
spamm_spamm_to_dense (const struct spamm_t *A, floating_point_t **A_dense);

void
spamm_swap_block_dense (const unsigned int M, const unsigned int N, floating_point_t *A, floating_point_t *B);

void
spamm_swap_floating_point_t (floating_point_t *a, floating_point_t *b);

void
spamm_swap_floating_point_t_pointer (floating_point_t **a, floating_point_t **b);

void
spamm_swap_linear_quadtree (void *data1, void *data2);

void
spamm_swap_multiply_stream (void *data1, void *data2);

void
spamm_swap_short_unsigned_int (short unsigned int *a, short unsigned int *b);

void
spamm_swap_unsigned_int (unsigned int *a, unsigned int *b);

void
spamm_tree_pack (const unsigned int linear_tier, const unsigned int chunksize,
    const enum spamm_linear_mask_t mask, struct spamm_t *A);

void
spamm_tree_stats (struct spamm_tree_stats_t *stats, const struct spamm_t *A);

#endif
