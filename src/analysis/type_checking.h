#ifndef TYPE_CHECKING_H
#define TYPE_CHECKING_H

#include "semantic_symbols.h"
#include "semantic_types.h"

// Type checking function declarations
bool semantic_check_type_compatibility(SemanticAnalyzer *analyzer, TypeDescriptor *type1,
                                       TypeDescriptor *type2);

bool semantic_can_cast(SemanticAnalyzer *analyzer, TypeDescriptor *from, TypeDescriptor *to);

bool semantic_validate_cast_compatibility(SemanticAnalyzer *analyzer, TypeDescriptor *from_type,
                                          TypeDescriptor *to_type, SourceLocation location);

#endif // TYPE_CHECKING_H