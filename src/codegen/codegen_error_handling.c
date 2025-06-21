/**
 * Asthra Programming Language Compiler
 * Code Generator - Error Handling
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Functions for handling and reporting code generation errors
 */

#include "code_generator.h"
#include <stdio.h>
#include <stdarg.h>

void code_generator_report_error(CodeGenerator *generator, CodeGenErrorCode code, 
                                const char *format, ...) {
    if (!generator || !format) return;
    
    fprintf(stderr, "Code generation error (%d): ", code);
    
    va_list args;
    va_start(args, format);
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
    vfprintf(stderr, format, args);
#ifdef __clang__
    #pragma clang diagnostic pop
#endif
    va_end(args);
    
    fprintf(stderr, "\n");
}

const char *code_generator_error_name(CodeGenErrorCode code) {
    switch (code) {
        case CODEGEN_ERROR_NONE: return "NONE";
        case CODEGEN_ERROR_OUT_OF_MEMORY: return "OUT_OF_MEMORY";
        case CODEGEN_ERROR_INVALID_INSTRUCTION: return "INVALID_INSTRUCTION";
        case CODEGEN_ERROR_REGISTER_ALLOCATION_FAILED: return "REGISTER_ALLOCATION_FAILED";
        case CODEGEN_ERROR_LABEL_NOT_FOUND: return "LABEL_NOT_FOUND";
        case CODEGEN_ERROR_UNSUPPORTED_OPERATION: return "UNSUPPORTED_OPERATION";
        case CODEGEN_ERROR_ABI_VIOLATION: return "ABI_VIOLATION";
        case CODEGEN_ERROR_STACK_OVERFLOW: return "STACK_OVERFLOW";
        default: return "UNKNOWN";
    }
}