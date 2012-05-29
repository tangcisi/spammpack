#!/bin/bash

echo > scaling_test.output

echo -n " 1 thread... "; ./spamm_multiply_01 >> scaling_test.output && echo "ok"
echo -n " 2 thread... "; ./spamm_multiply_02 >> scaling_test.output && echo "ok"
echo -n " 4 thread... "; ./spamm_multiply_04 >> scaling_test.output && echo "ok"
echo -n " 8 thread... "; ./spamm_multiply_08 >> scaling_test.output && echo "ok"
echo -n "12 thread... "; ./spamm_multiply_12 >> scaling_test.output && echo "ok"
echo -n "16 thread... "; ./spamm_multiply_16 >> scaling_test.output && echo "ok"
echo -n "20 thread... "; ./spamm_multiply_20 >> scaling_test.output && echo "ok"
echo -n "24 thread... "; ./spamm_multiply_24 >> scaling_test.output && echo "ok"
echo -n "28 thread... "; ./spamm_multiply_28 >> scaling_test.output && echo "ok"
echo -n "32 thread... "; ./spamm_multiply_32 >> scaling_test.output && echo "ok"
echo -n "36 thread... "; ./spamm_multiply_36 >> scaling_test.output && echo "ok"
echo -n "40 thread... "; ./spamm_multiply_40 >> scaling_test.output && echo "ok"
echo -n "44 thread... "; ./spamm_multiply_44 >> scaling_test.output && echo "ok"
echo -n "48 thread... "; ./spamm_multiply_48 >> scaling_test.output && echo "ok"

grep SpAMM_SCALING scaling_test.output | awk '{print $2, $3}' > scaling_test.dat

# Test result.
./parseTestResults.py --verbose --output scaling_test.output --reference scaling_test.reference
