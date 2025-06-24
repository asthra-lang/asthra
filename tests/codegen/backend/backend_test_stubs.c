/**
 * Minimal stubs for backend testing
 * Provides stub implementations to avoid linking issues
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../src/parser/ast.h"
#include "../../../src/parser/ast_types.h"
#include "../../../src/compiler.h"
// Code generator headers removed - using LLVM backend only

#ifdef MINIMAL_TEST_BUILD

// Stub for generate_c_code - matches actual signature
int generate_c_code(FILE *output, ASTNode *node) {
    // Minimal stub - just write a comment
    if (output) {
        fprintf(output, "// Generated C code stub\n");
        return 0;
    }
    return -1;
}

// These functions are now provided by the asthra_compiler library
// so we don't need stubs for them anymore

// Code generation functions are provided by the asthra_compiler library

// Argument list functions are provided by the asthra_compiler library

// Only provide stubs for functions that are genuinely missing from the library
// Most functions are now provided by asthra_compiler library

// Stub for error handling context (if not provided by library)
void* asthra_error_context_create(void) {
    return calloc(1, sizeof(void*));
}

void asthra_error_context_destroy(void* ctx) {
    free(ctx);
}

#endif // MINIMAL_TEST_BUILD