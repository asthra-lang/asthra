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
#include "../../../src/codegen/code_generator.h"
#include "../../../src/codegen/code_generator_core.h"

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

// Stub for code generator creation
CodeGenerator* code_generator_create(TargetArchitecture arch, CallingConvention calling_convention) {
    CodeGenerator* gen = calloc(1, sizeof(CodeGenerator));
    if (gen) {
        gen->target_arch = arch;
        gen->calling_conv = calling_convention;
    }
    return gen;
}

// Stub for code generator destruction
void code_generator_destroy(CodeGenerator* generator) {
    free(generator);
}

// Stub for code generation
bool code_generate_program(CodeGenerator* generator, ASTNode* program) {
    // Minimal stub - just return success
    return true;
}

// Stub for assembly emission - matches actual signature
bool code_generator_emit_assembly(CodeGenerator *generator, 
                                 char * restrict output_buffer, 
                                 size_t buffer_size) {
    // Minimal stub - write simple assembly to buffer
    if (output_buffer && buffer_size > 20) {
        const char* stub = "; Assembly stub\n";
        strncpy(output_buffer, stub, buffer_size - 1);
        output_buffer[buffer_size - 1] = '\0';
        return true;
    }
    return false;
}

// Stub for argument list creation
struct AsthraArgumentList *asthra_argument_list_create(size_t initial_capacity) {
    return calloc(1, sizeof(struct AsthraArgumentList));
}

// Stub for argument list destruction
void asthra_argument_list_destroy(struct AsthraArgumentList *list) {
    free(list);
}

// Stub for error handling context
void* asthra_error_context_create(void) {
    return calloc(1, sizeof(void*));
}

void asthra_error_context_destroy(void* ctx) {
    free(ctx);
}

// Stub for version string
const char* asthra_get_version_string(void) {
    return "1.0.0-test";
}

// Stub for compiler context creation
AsthraCompilerContext* asthra_compiler_context_create(const AsthraCompilerOptions* options) {
    AsthraCompilerContext* ctx = calloc(1, sizeof(AsthraCompilerContext));
    if (ctx && options) {
        ctx->options = *options;
    }
    return ctx;
}

// Stub for compiler context destruction
void asthra_compiler_context_destroy(AsthraCompilerContext* ctx) {
    free(ctx);
}

#endif // MINIMAL_TEST_BUILD