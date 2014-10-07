---
layout: default
title: Spammpack
---

Spammpack
---------

Spammpack is an implementation of the Sparse Approximate Matrix Multiplication
(SpAMM) algorithm introduced in [[1]](/spammpack/references.html).  It
provides a matrix data type, and an approximate matrix product, which exhibits
linear scaling computational complexity for matrices with decay. The product
error and the performance of the multiply can be tuned by choosing an
appropriate tolerance. The library can be compiled for serial executation or
parallel execution on shared memory systems with an OpenMP capable compiler.
Currently under heavy development, the latest version can be obtained through git:

    $ git clone https://github.com/FreeON/spammpack.git

Alternatively we offer a snapshot of the latest version in a zip or tar archive
through the links on the right hand side. See [Building](/spammpack/building.html)
for more details on how to build the library.

Authors
-------

The principal authors of spammpack are:

  - Matt Challacombe @mattchallacombe
  - Nicolas Bock @nicolasbock