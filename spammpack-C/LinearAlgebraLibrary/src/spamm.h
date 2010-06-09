/** @file */

#if ! defined(__SPAMM_H)

/** Define in case spamm.h has been included. */
#define __SPAMM_H 1

#include "config.h"
#include "spamm_ll.h"
#include "spamm_mm.h"
#include <stdio.h>

/** \mainpage SpAMM
 *
 * \brief Sparse Approximate Matrix Multiply (SpAMM) library.
 *
 * SpAMM is a library for spare approximate matrices.
 *
 * The public functions of this library are all declared in spamm.h. Useful
 * helper functions are documented in spamm_ll.h and spamm_mm.h.
 *
 * \section Introduction
 *
 * A spamm_t matrix is defined as an \f$ N \f$-tree on the 2-dimensional
 * matrix elements. The matrix elements at the lowest level are stored in
 * dense matrix blocks. The matrix is padded with zeros so that the tree depth
 * \f$ d \f$ is integer according to
 *
 * \f[
 * M = M_{\mathrm{block}} M_{\mathrm{child}}^d
 * \]
 * \[
 * N = N_{\mathrm{block}} N_{\mathrm{child}}^d
 * \f]
 *
 * where \f$ M \f$ (\f$ N \f$) is the number of rows (columns) of the matrix,
 * \f$ M_{\mathrm{block}} \f$ (\f$ N_{\mathrm{block}} \f$) is the number of
 * rows (columns) of the dense matrix blocks at the lowest tree level, \f$
 * M_{\mathrm{child}} \f$ (\f$ N_{\mathrm{child}} \f$) is the number of rows
 * (columns) of the children nodes per node in the matrix tree, and \f$ d \f$
 * is the depth of the tree. This means that the matrix tree is a quadtree for
 * \f$ M_{\mathrm{child}} = N_{\mathrm{child}} = 2 \f$.
 *
 * \version 2010-04-19
 *
 * \author Nicolas Bock <nicolasbock@gmail.com>
 * \author Matt Challacombe <matt.challacombe@gmail.com>.
 */

/** Definition of global floating point precision.
 *
 * This is either float or double, as defined by the configure script.
 */
typedef FLOATING_PRECISION float_t;

/** Define shortcut macro for logging.
 *
 * Typical use of this macro:
 *
 * <code>LOG("opening new file: %s", filename);</code>
 *
 * Compare to spamm_log().
 *
 * @param format Format string. See printf() for a detailed description of its
 *               syntax.
 */
#define LOG(format, ...) spamm_log(format, __FILE__, __LINE__, __VA_ARGS__)

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
  float_t threshold;

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
  float_t threshold;

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
   * quadtree that was there before.
   */
  struct spamm_ll_t *linear_quadtree;

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

/** Linear quadtree.
 */
struct spamm_linear_quadtree_t
{
  /** The linear quadtree index of this data block.
   */
  unsigned int index;

  /** The data.
   */
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
struct spamm_multiply_stream_element_t
{
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
spamm_log (const char *format, const char *filename, const unsigned int linenumber, ...);

int
spamm_dense_index (const unsigned int i, const unsigned int j,
    const unsigned int M, const unsigned int N);

void
spamm_new (const unsigned int M, const unsigned int N,
    const unsigned int M_block, const unsigned int N_block,
    const unsigned int M_child, const unsigned int N_child,
    const float_t threshold, struct spamm_t *A);

void
spamm_new_node (struct spamm_node_t **node);

void
spamm_delete_node (struct spamm_node_t *node);

void
spamm_delete (struct spamm_t *A);

void
spamm_dense_to_spamm (const unsigned int M, const unsigned int N,
    const unsigned int M_block, const unsigned int N_block,
    const unsigned int M_child, const unsigned int N_child,
    const float_t threshold, const float_t *A_dense,
    struct spamm_t *A);

void
spamm_spamm_to_dense (const struct spamm_t *A, float_t **A_dense);

float_t
spamm_get (const unsigned int i, const unsigned int j, const struct spamm_t *A);

int
spamm_set (const unsigned int i, const unsigned int j, const float_t Aij, struct spamm_t *A);

void
spamm_print_dense (const unsigned int M, const unsigned int N, const float_t *A_dense);

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
spamm_read_MM (const char *filename,
    const unsigned int M_block, const unsigned int N_block,
    const unsigned int M_child, const unsigned int N_child,
    const float_t threshold, struct spamm_t *A);

unsigned int
spamm_number_nonzero (const struct spamm_t *A);

void
spamm_tree_pack (const unsigned int linear_tier, const unsigned int chunksize,
    const enum spamm_linear_mask_t mask, struct spamm_t *A);

#endif
