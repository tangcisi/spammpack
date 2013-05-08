/** @file */

#ifndef __SPAMM_SORT_H
#define __SPAMM_SORT_H

#include "spamm_types.h"

void
spamm_sort_masked (const unsigned int length,
    unsigned int *list,
    const unsigned int mask);

void
spamm_sort_norm (const unsigned int length,
    unsigned int *list,
    spamm_norm_t *norm);

#endif