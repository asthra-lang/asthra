#!/bin/bash
# Script to remove old code generation infrastructure
# Run from the asthra-llvm directory

echo "Old Code Generation Infrastructure Cleanup Script"
echo "================================================"
echo ""
echo "This script will remove test files and infrastructure for the old"
echo "assembly-based code generator that has already been removed from"
echo "production code."
echo ""
echo "Press Ctrl+C to cancel, or Enter to continue..."
read

# Phase 1: Remove obsolete codegen test files
echo "Phase 1: Removing obsolete codegen test files..."
rm -vf tests/codegen/integration/test_code_generator.c
rm -vf tests/codegen/integration/test_simple_arithmetic.c
rm -vf tests/codegen/integration/test_multiline_string_codegen.c
rm -vf tests/codegen/integration/test_phase5_simple.c

# Phase 2: Remove FFI assembly tests
echo ""
echo "Phase 2: Removing FFI assembly test files..."
rm -vf tests/ffi/test_ffi_simple.c
rm -vf tests/ffi/test_ffi_complete.c
rm -vf tests/ffi/test_ffi_assembly_*.c
rm -vf tests/ffi/test_option_ffi.c  # Also uses old API

# Phase 3: Remove codegen test stub headers
echo ""
echo "Phase 3: Removing codegen test stub headers..."
find tests/codegen -name "codegen_test_stubs.h" -exec rm -v {} \;

# Phase 4: Remove symbol export tests that use old API
echo ""
echo "Phase 4: Removing symbol export tests..."
rm -vf tests/codegen/symbol_export/test_symbol_export.c
rm -vf tests/codegen/symbol_export/test_*_export.c

# Phase 5: Remove obsolete examples
echo ""
echo "Phase 5: Removing obsolete examples..."
rm -vf examples/test_codegen.c
rm -vf examples/test_optimizer.c

# Phase 6: Remove cross-platform tests that define old constants
echo ""
echo "Phase 6: Removing cross-platform common header..."
rm -vf tests/codegen/cross_platform/cross_platform_common.h

# Phase 7: Clean up legacy constant definitions
echo ""
echo "Phase 7: Updating backend wrapper to remove legacy constants..."
if [ -f tests/codegen/codegen_backend_wrapper.h ]; then
    # Remove the legacy constant definitions
    sed -i.bak '/#define TARGET_ARCH_X86_64/d' tests/codegen/codegen_backend_wrapper.h
    sed -i.bak '/#define CALLING_CONV_SYSTEM_V_AMD64/d' tests/codegen/codegen_backend_wrapper.h
    rm -f tests/codegen/codegen_backend_wrapper.h.bak
    echo "Updated codegen_backend_wrapper.h"
fi

# Phase 8: Remove immutable test files that use old constants
echo ""
echo "Phase 8: Cleaning immutable test stubs..."
rm -vf tests/immutable_by_default/legacy_stubs.c
rm -vf tests/immutable_by_default/immutable_creators.c

# Summary
echo ""
echo "Cleanup Summary:"
echo "================"
echo "Removed test files that were testing non-existent code generator"
echo "Removed FFI assembly generator tests"
echo "Removed obsolete examples"
echo "Cleaned up legacy constant definitions"
echo ""
echo "Next steps:"
echo "1. Update CMakeLists.txt files to remove references to deleted tests"
echo "2. Run 'git status' to review changes"
echo "3. Build and run remaining tests to ensure nothing is broken"
echo "4. Commit changes with message: 'chore: remove obsolete code generation test infrastructure'"