/** @file
 *
 * The implementation of the Multiply class.
 *
 * @author Nicolas Bock <nicolas.bock@freeon.org>
 * @author Matt Challacombe <matt.challacombe@freeon.org>
 */

#include "multiply.h"
#include "messages.h"
#include "logger.h"

/** The constructor.
 *
 * @param A Matrix A.
 * @param B Matrix B.
 * @param C Matrix C.
 * @param blocksize The SpAMM blocksize.
 * @param depth The depth of the matrix trees.
 * @param ANodes The Node objects of A.
 * @param BNodes The Node objects of B.
 * @param CNodes The Node objects of C.
 */
Multiply::Multiply (CProxy_Matrix A, CProxy_Matrix B, CProxy_Matrix C,
    int blocksize, int depth, CProxy_Node ANodes, CProxy_Node BNodes,
    CProxy_Node CNodes)
{
  INFO("Multiply constructor\n");

  this->A = A;
  this->B = B;
  this->C = C;

  int NTier = 1 << depth;

  this->convolution = CProxy_MultiplyElement::ckNew(blocksize, depth, depth,
      ANodes, BNodes, CNodes, NTier, NTier, NTier);

  DEBUG("Multiply constructor, created %dx%dx%d convolution\n", NTier, NTier,
      NTier);
}

/** Multiply two Matrix objects.
 *
 * A full convolution (as a space filling curve) is constructed.
 *
 * @param tolerance The SpAMM tolerance.
 * @param cb The callback.
 */
void Multiply::multiply (double tolerance, CkCallback &cb)
{
  INFO("tolerance = %e\n", tolerance);
  convolution.multiply(tolerance, CkCallbackResumeThread());

  INFO("storeBack\n");
  convolution.storeBack(CkCallbackResumeThread());

  cb.send();
}

#include "multiply.def.h"
