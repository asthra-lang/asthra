#!/bin/bash

# Build the FFI assembly generator test
make test-ffi-assembly-generator -B

# Run the test with the standalone flag
ASTHRA_TEST_STANDALONE=1 bin/test_ffi_assembly_generator 
