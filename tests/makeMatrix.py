#!/usr/bin/env python

import random

N = 2048

for i in range(N):
  for j in range(N):
    print "%d %d % 1.14e" % (i+1, j+1, random.uniform(0.1, 1.0))
