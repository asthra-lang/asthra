#ifndef FFI_ASSEMBLY_CONCURRENCY_H
#define FFI_ASSEMBLY_CONCURRENCY_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "ffi_assembly_generator.h"

#ifdef __cplusplus
extern "C" {
#endif

bool ffi_generate_task_creation_with_handle(FFIAssemblyGenerator *generator, const char *function_name, Register *arg_regs, size_t arg_count, Register handle_reg);
bool ffi_validate_concurrency_annotations(FFIAssemblyGenerator *generator, ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // FFI_ASSEMBLY_CONCURRENCY_H
