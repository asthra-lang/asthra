#ifndef ASTHRA_TYPE_INFO_BUILTINS_H
#define ASTHRA_TYPE_INFO_BUILTINS_H

#include "type_info_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BUILT-IN TYPE CONSTANTS
// =============================================================================

// Built-in primitive types (initialized at startup)
extern TypeInfo *TYPE_INFO_I8;
extern TypeInfo *TYPE_INFO_I16;
extern TypeInfo *TYPE_INFO_I32;
extern TypeInfo *TYPE_INFO_I64;
extern TypeInfo *TYPE_INFO_I128;
extern TypeInfo *TYPE_INFO_U8;
extern TypeInfo *TYPE_INFO_U16;
extern TypeInfo *TYPE_INFO_U32;
extern TypeInfo *TYPE_INFO_U64;
extern TypeInfo *TYPE_INFO_U128;
extern TypeInfo *TYPE_INFO_F32;
extern TypeInfo *TYPE_INFO_F64;
extern TypeInfo *TYPE_INFO_BOOL;
extern TypeInfo *TYPE_INFO_CHAR;
extern TypeInfo *TYPE_INFO_STRING;
extern TypeInfo *TYPE_INFO_VOID;
extern TypeInfo *TYPE_INFO_USIZE;
extern TypeInfo *TYPE_INFO_ISIZE;

/**
 * Initialize built-in types
 */
bool type_info_init_builtins(void);

/**
 * Cleanup built-in types
 */
void type_info_cleanup_builtins(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TYPE_INFO_BUILTINS_H