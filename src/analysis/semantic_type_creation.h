#ifndef SEMANTIC_TYPE_CREATION_H
#define SEMANTIC_TYPE_CREATION_H

#include "semantic_types.h"
#include "../parser/ast_types.h"
#include <stdbool.h>
#include <stddef.h>

// Type descriptor creation functions
TypeDescriptor *type_descriptor_create_struct(const char *name, size_t field_count);
TypeDescriptor *type_descriptor_create_pointer(TypeDescriptor *pointee_type);
TypeDescriptor *type_descriptor_create_slice(TypeDescriptor *element_type);
TypeDescriptor *type_descriptor_create_array(TypeDescriptor *element_type, size_t size);
TypeDescriptor *type_descriptor_create_result(TypeDescriptor *ok_type, TypeDescriptor *err_type);
TypeDescriptor *type_descriptor_create_option(TypeDescriptor *value_type);
TypeDescriptor *type_descriptor_create_function(void);
TypeDescriptor *type_descriptor_create_function_with_params(TypeDescriptor *return_type, size_t param_count);
TypeDescriptor *type_descriptor_create_generic_instance(TypeDescriptor *base_type, 
                                                       TypeDescriptor **type_args, 
                                                       size_t type_arg_count);
TypeDescriptor *type_descriptor_create_tuple(TypeDescriptor **element_types, size_t element_count);

// Struct type operations stubs
bool type_descriptor_add_struct_field(TypeDescriptor *struct_type, 
                                     const char *field_name, 
                                     TypeDescriptor *field_type,
                                     ASTNode *field_declaration);
SymbolEntry *type_descriptor_lookup_struct_field(TypeDescriptor *struct_type, const char *field_name);

#endif // SEMANTIC_TYPE_CREATION_H