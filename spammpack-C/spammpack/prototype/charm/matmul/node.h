#ifndef __NODE_H
#define __NODE_H

#include "node.decl.h"
#include "types.h"
#include <map>
#include <list>

class Node : public CBase_Node
{
  private:

    int N;
    int depth;
    int blocksize;
    int tier;

    int iLower, iUpper;
    int jLower, jUpper;

    std::map<int, bool> callbackSet;
    std::map<int, CkCallback> cb;

    std::map<int, std::list<int> > childWorking;

    bool childNull[4];
    CProxy_Node child[4];

    double *block;

  public:

    Node (int N, int depth, int blocksize, int tier,
        int iLower, int iUpper, int jLower, int jUpper);
    NodeInfoMsg * info ();
    NodeBlockMsg * getBlock ();
    DoubleMsg * get (int i, int j);
    void initialize (int initType, int index, CkCallback &cb);
    void initializeDone (IntMsg *index);
    void multiply (int index, CProxy_Node A, CProxy_Node B, CkCallback &cb);
    void multiplyDone (IntMsg *index);
    void printLeafPes (int index, CkCallback &cb);
};

#endif
