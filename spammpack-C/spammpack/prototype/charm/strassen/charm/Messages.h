#ifndef __MESSAGES_H
#define __MESSAGES_H

#include "Messages.decl.h"
#include "Node.h"

class EmptyMsg : public CMessage_EmptyMsg
{
};

class DoubleMsg : public CMessage_DoubleMsg
{
  public :

    double x;
    DoubleMsg ();
    DoubleMsg (double x);
};

class MatrixMsg : public CMessage_MatrixMsg
{
  public:

    int N;
    int blocksize;
    CProxy_Node *root;
    MatrixMsg (int N, int blocksize, CProxy_Node *root);
};

class NodeMsg : public CMessage_NodeMsg
{
  public:

    int iLower;
    int iUpper;
    int jLower;
    int jUpper;
    int blocksize;
    CProxy_Node *child[4];
    NodeMsg (int iLower, int iUpper, int jLower, int jUpper, int blocksize, CProxy_Node *child[4]);
};

class DataMsg : public CMessage_DataMsg
{
  public:

    double *data;
};

#endif
