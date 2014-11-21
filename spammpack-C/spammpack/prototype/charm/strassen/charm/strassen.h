#ifndef __STRASSEN_H
#define __STRASSEN_H

#include "strassen.decl.h"

class Main : public CBase_Main
{
  public:

    Main (CkArgMsg *msg);
    void run (int N, int blocksize, bool debug, bool verify);
};

#endif
