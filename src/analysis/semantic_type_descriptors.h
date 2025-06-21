#ifndef SEMANTIC_TYPE_DESCRIPTORS_H
#define SEMANTIC_TYPE_DESCRIPTORS_H

#include "semantic_types.h"
#include <stdbool.h>
#include <stddef.h>

// Core type descriptor lifecycle and operations
TypeDescriptor *type_descriptor_create_primitive(int primitive_kind);
void type_descriptor_retain(TypeDescriptor *type);
void type_descriptor_release(TypeDescriptor *type);
bool type_descriptor_equals(const TypeDescriptor *type1, const TypeDescriptor *type2);
size_t type_descriptor_hash(const TypeDescriptor *type);

#endif // SEMANTIC_TYPE_DESCRIPTORS_H