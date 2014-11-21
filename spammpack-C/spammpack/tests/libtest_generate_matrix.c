/** @file */

#define CONCAT2(a, b) a ## _ ## b
#define SPAMM_FUNCTION(name, type) CONCAT2(name, type)

#define MATRIX_TYPE float
#include "libtest_generate_matrix_sources.c"
#undef MATRIX_TYPE

#define MATRIX_TYPE double
#include "libtest_generate_matrix_sources.c"
#undef MATRIX_TYPE
