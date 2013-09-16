/** @file
 *
 * The implementation of the Matrix class.
 *
 * @author Nicolas Bock <nicolas.bock@freeon.org>
 * @author Matt Challacombe <matt.challacombe@freeon.org>
 */

#include "matrix.h"
#include "messages.h"
#include "logger.h"
#include "utilities.h"
#include "index.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/** The constructor.
 *
 * @param initialPE The PE to place the Node chares.
 * @param alignPEs Align PEs in the diagonal matrix case.
 * @param blocksize The SpAMM blocksize.
 * @param nameLength The strlen of the name.
 * @param name The matrix name.
 * @param length The length of the filename string.
 * @param filename The filename.
 */
Matrix::Matrix (int initialPE, bool alignPEs, int blocksize, int nameLength,
    char *name, int filenamelength, char *filename)
{
  name = NULL;
  nodes = NULL;
  PEMap = NULL;
  PEMap_norm = NULL;

  FILE *fd;

  if((fd = fopen(filename, "r")) == NULL)
  {
    ABORT("error opening density file \"%s\"\n", filename);
  }

  INFO("reading density matrix from \"%s\"\n", filename);

  int N = -1;
  char linebuffer[2000];
  int i, j;
  double Aij;
  int result;
  while(fgets(linebuffer, 2000, fd) == linebuffer)
  {
    if((result = sscanf(linebuffer, "%d %d %le\n", &i, &j, &Aij)) == 3)
    {
      DEBUG("read %d %d %e\n", i, j, Aij);
      if(i > N) { N = i; }
      if(j > N) { N = j; }
    }

    else
    {
      break;
    }
  }

  if(result == EOF)
  {
    if(ferror(fd) != 0)
    {
      ABORT("error reading file: %s\n", strerror(errno));
    }
  }

  if(result == 0)
  {
    while(linebuffer[strlen(linebuffer)-1] == '\n')
    {
      linebuffer[strlen(linebuffer)-1] = '\0';
    }

    ABORT("syntax error: \"%s\"\n", linebuffer);
  }

  if(N < 1)
  {
    ABORT("could not read coordinate file\n");
  }

  INFO("reading %dx%d matrix\n", N, N);

  rewind(fd);

  double *ADense = new double[N*N];

  while(fgets(linebuffer, 2000, fd) == linebuffer)
  {
    sscanf(linebuffer, "%d %d %le\n", &i, &j, &Aij);
    ADense[BLOCK_INDEX(i, j, 0, 0, N)] = Aij;
  }

  fclose(fd);
}

/** The constructor.
 *
 * @param initialPE The PE to place the Node chares.
 * @param alignPEs Align PEs in the diagonal matrix case.
 * @param N The matrix size.
 * @param blocksize The SpAMM blocksize.
 * @param nameLength The strlen of the name.
 * @param name The matrix name.
 */
Matrix::Matrix (int initialPE, bool alignPEs, int N, int blocksize,
    int nameLength, char *name)
{
  this->name = strdup(name);
  this->blocksize = blocksize;

  this->N = N;

  /* Calculate tree depth. */
  depth = -1;
  for(int i = N/blocksize; i > 0; i >>= 1)
  {
    depth++;
  }
  if(blocksize*(1 << depth) < N) depth++;
  NPadded = blocksize*(1 << depth);

  nodes = new CProxy_Node[depth+1];
  for(int tier = 0; tier <= depth; tier++)
  {
    int NTier = 1 << tier;

    unsigned long bytes = NTier*NTier*(sizeof(Node)
        +blocksize*blocksize*sizeof(double));
    INFO("name = %s, N = %d, blocksize = %d, tier = %d, depth = %d, "
        "NPadded = %d, NTier = %d, creating %d Nodes using %d bytes (%s)\n",
        this->name, N, blocksize, tier, depth, NPadded,
        NTier, NTier*NTier, bytes, humanReadableSize(bytes).c_str());

    nodes[tier] = CProxy_Node::ckNew();
    for(int i = 0; i < NTier; i++) {
      for(int j = 0; j < NTier; j++)
      {
        if(alignPEs && i == j)
        {
          initialPE = i%CkNumPes();
        }
        nodes[tier](i, j).insert(N, depth, blocksize, tier, initialPE);
      }
    }
    nodes[tier].doneInserting();

    if(tier == depth)
    {
      PEMap = new int[NTier*NTier];
      PEMap_norm = new double[NTier*NTier];
    }
  }
}

/** The destructor.
 */
Matrix::~Matrix (void)
{
  delete[] PEMap;
  delete[] PEMap_norm;
}

/** Initialize the Matrix.
 */
void Matrix::initialize (void)
{
}

/** Get some basic information on the matrix.
 *
 * @return The matrix information.
 */
MatrixInfoMsg * Matrix::info (void)
{
  return new MatrixInfoMsg (N, blocksize, depth, NPadded);
}

/** Convert a Matrix to a dense matrix.
 *
 * @return The dense matrix.
 */
DenseMatrixMsg * Matrix::toDense (void)
{
  DenseMatrixMsg *A = new (N*N) DenseMatrixMsg();

  int NTier = 1 << depth;

  for(int i = 0; i < NTier; i++) {
    for(int j = 0; j < NTier; j++)
    {
      DenseMatrixMsg *block = nodes[depth](i, j).getBlock();

      for(int l = i*blocksize; l < (i+1)*blocksize && l < N; l++) {
        for(int m = j*blocksize; m < (j+1)*blocksize && m < N; m++)
        {
          A->A[BLOCK_INDEX(l, m, 0, 0, N)] = block->A[BLOCK_INDEX(l, m,
              i*blocksize, j*blocksize, blocksize)];
        }
      }

      delete block;
    }
  }

  return A;
}

/** Get the Node array on a particular tier.
 *
 * @param tier The tier.
 *
 * @return The Node array on that tier.
 */
MatrixNodeMsg * Matrix::getNodes (int tier)
{
  assert(tier >= 0 && tier <= depth);
  return new MatrixNodeMsg(nodes[tier]);
}

/** Print the PEs all @link Node Nodes @endlink are on.
 *
 * @param cb The callback to signal once all @link Node Nodes @endlink have
 * printed.
 */
void Matrix::updatePEMap (CkCallback &cb)
{
  this->cb = cb;
  CkCallback done(CkReductionTarget(Matrix, donePEMap), thisProxy);
  nodes[depth].PEMap(done);
}

/** The reduction target for Matrix::updatePEMap.
 *
 * @param msg The reduction message.
 */
void Matrix::donePEMap (CkReductionMsg *msg)
{
  int NTier = 1 << depth;

  CkReduction::setElement *current = (CkReduction::setElement*) msg->getData();
  while(current != NULL)
  {
    DEBUG("dataSize %d sizeof() %d\n", current->dataSize, sizeof(struct PEMap_Node_t));
    assert(current->dataSize == sizeof(struct PEMap_Node_t));
    struct PEMap_Node_t *result = (struct PEMap_Node_t*) &current->data;
    DEBUG("data = { %d, %d, %d }\n", result->index[0], result->index[1], result->PE);
    PEMap[BLOCK_INDEX(result->index[0], result->index[1], 0, 0, NTier)] = result->PE;
    PEMap_norm[BLOCK_INDEX(result->index[0], result->index[1], 0, 0, NTier)] = result->norm;
    current = current->next();
  }

  cb.send();
}

/** Set a matrix using a dense array.
 *
 * @param N The matrix size.
 * @param A The dense matrix.
 * @param cb The callback to signal once done.
 */
void Matrix::set (int N, double *A, CkCallback &cb)
{
  assert(this->N == N);

  DEBUG("setting matrix\n");

  /* Set the A matrix. */
  double *block = new double[blocksize*blocksize];

  for(int i = 0; i < NPadded/blocksize; i++) {
    for(int j = 0; j < NPadded/blocksize; j++)
    {
      memset(block, 0, sizeof(double)*blocksize*blocksize);

      for(int l = i*blocksize; l < (i+1)*blocksize && l < N; l++) {
        for(int m = j*blocksize; m < (j+1)*blocksize && m < N; m++)
        {
          block[BLOCK_INDEX(l, m, i*blocksize, j*blocksize, blocksize)] =
            A[BLOCK_INDEX(l, m, 0, 0, N)];
        }
      }

      nodes[depth](i, j).set(blocksize, block, CkCallbackResumeThread());
    }
  }
  delete[] block;

  /* Update norms. */
  thisProxy.setNorm(CkCallbackResumeThread());

  cb.send();
}

/** Update the norms based on the norm information of the leaf @link Node
 * nodes @endlink.
 *
 * @param cb The callback to send back to.
 */
void Matrix::setNorm (CkCallback &cb)
{
  /* Update the norms on the upper tiers. */
  for(int tier = depth-1; tier >= 0; tier--)
  {
    nodes[tier].setNorm(nodes[tier+1], CkCallbackResumeThread());
  }
  cb.send();
}

/** Return the PEMap. Call udpatePEMap() first.
 *
 * @return a PEMapMsg with the PEMap.
 */
PEMapMsg * Matrix::getPEMap (void)
{
  int NTier = 1 << depth;
  PEMapMsg *msg = new (NTier*NTier, NTier*NTier) PEMapMsg();
  memcpy(msg->PEMap, PEMap, NTier*NTier*sizeof(int));
  memcpy(msg->PEMap_norm, PEMap_norm, NTier*NTier*sizeof(double));
  return msg;
}

/** Update the trace on this matrix.
 *
 * @param cb The callback to send to when done.
 */
void Matrix::updateTrace (CkCallback &cb)
{
  this->cb = cb;
  nodes[depth].trace(CkCallback(CkReductionTarget(Matrix, doneTrace), thisProxy));
}

/** The reduction target for the trace operation.
 *
 * @param trace The reduction result.
 */
void Matrix::doneTrace (double trace)
{
  this->trace = trace;
  cb.send();
}

/** Get the trace of a matrix.
 *
 * @return The trace.
 */
DoubleMsg * Matrix::getTrace (void)
{
  return new DoubleMsg(trace);
}

/** Add another matrix to this one.
 *
 * @f[ A \leftarrow \alpha A + \beta B @f]
 *
 * where A is this Matrix.
 *
 * @param alpha The factor @f$ \alpha @f$.
 * @param beta The factor @f$ \beta @f$.
 * @param B The matrix B.
 * @param cb The callback to call when done.
 */
void Matrix::add (double alpha, double beta, CProxy_Matrix B, CkCallback &cb)
{
  MatrixNodeMsg *BNodes = B.getNodes(depth);
  nodes[depth].add(alpha, beta, BNodes->nodes, CkCallbackResumeThread());
  cb.send();
}

#include "matrix.def.h"
