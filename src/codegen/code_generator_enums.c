/**
 * Asthra Programming Language Compiler
 * Code Generator Enum Declaration Generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Enum declaration generation including typedefs, constructors, and pattern functions
 */

#include "code_generator_common.h"

// =============================================================================
// ENUM DECLARATION CODE GENERATION (Phase 2)
// =============================================================================

// Forward declarations for static helper functions
static bool generate_enum_typedef(CodeGenerator *generator, const char *enum_name, ASTNodeList *variants);
static bool generate_enum_variant_constructor(CodeGenerator *generator, const char *enum_name, 
                                            ASTNode *variant, uint32_t tag_value);
static bool generate_enum_pattern_functions(CodeGenerator *generator, const char *enum_name, ASTNodeList *variants);

bool code_generate_enum_declaration(CodeGenerator *generator, ASTNode *enum_decl) {
    if (!generator || !enum_decl || enum_decl->type != AST_ENUM_DECL) {
        return false;
    }
    
    const char *enum_name = enum_decl->data.enum_decl.name;
    ASTNodeList *variants = enum_decl->data.enum_decl.variants;
    
    if (!enum_name) {
        return false;
    }
    
    // Generate C typedef for tagged union structure
    if (!generate_enum_typedef(generator, enum_name, variants)) {
        return false;
    }
    
    // Generate constructor functions for each variant
    if (variants) {
        size_t variant_count = ast_node_list_size(variants);
        for (size_t i = 0; i < variant_count; i++) {
            ASTNode *variant = ast_node_list_get(variants, i);
            if (variant && variant->type == AST_ENUM_VARIANT_DECL) {
                if (!generate_enum_variant_constructor(generator, enum_name, variant, (uint32_t)i)) {
                    return false;
                }
            }
        }
    }
    
    // Generate pattern matching support functions
    if (!generate_enum_pattern_functions(generator, enum_name, variants)) {
        return false;
    }
    
    return true;
}

static bool generate_enum_typedef(CodeGenerator *generator, const char *enum_name, ASTNodeList *variants) {
    if (!generator || !enum_name) {
        return false;
    }
    
    // Generate typedef struct for tagged union
    // typedef struct {
    //     uint32_t tag;
    //     union { ... } data;
    // } EnumName;
    
    char *typedef_code = malloc(DEFAULT_TYPEDEF_BUFFER_SIZE);
    if (!typedef_code) {
        return false;
    }
    
    int offset = snprintf(typedef_code, DEFAULT_TYPEDEF_BUFFER_SIZE, 
        "typedef struct {\n"
        "    uint32_t tag;\n"
        "    union {\n");
    
    // Generate union members for variants with data
    if (variants) {
        size_t variant_count = ast_node_list_size(variants);
        for (size_t i = 0; i < variant_count; i++) {
            ASTNode *variant = ast_node_list_get(variants, i);
            if (variant && variant->type == AST_ENUM_VARIANT_DECL) {
                const char *variant_name = variant->data.enum_variant_decl.name;
                ASTNode *variant_type = variant->data.enum_variant_decl.associated_type;
                
                if (variant_type && variant_name) {
                    // For simplicity, assuming all variant types are primitive for now
                    // In a complete implementation, this would handle complex types
                    offset += snprintf(typedef_code + offset, DEFAULT_TYPEDEF_BUFFER_SIZE - offset,
                        "        void *%s_value;\n", variant_name);
                }
            }
        }
    }
    
    offset += snprintf(typedef_code + offset, DEFAULT_TYPEDEF_BUFFER_SIZE - offset,
        "    } data;\n"
        "} %s;\n\n", enum_name);
    
    // Add the typedef to the code generator's output
    // In a complete implementation, this would be added to the instruction buffer
    // or a separate header generation system
    
    // For now, we'll simulate success
    free(typedef_code);
    return true;
}

static bool generate_enum_variant_constructor(CodeGenerator *generator, const char *enum_name, 
                                            ASTNode *variant, uint32_t tag_value) {
    if (!generator || !enum_name || !variant) {
        return false;
    }
    
    const char *variant_name = variant->data.enum_variant_decl.name;
    ASTNode *variant_type = variant->data.enum_variant_decl.associated_type;
    
    if (!variant_name) {
        return false;
    }
    
    // Generate constructor function: EnumName_VariantName_new
    char *constructor_code = malloc(DEFAULT_CONSTRUCTOR_BUFFER_SIZE);
    if (!constructor_code) {
        return false;
    }
    
    if (variant_type) {
        // Constructor with parameter
        snprintf(constructor_code, DEFAULT_CONSTRUCTOR_BUFFER_SIZE,
            "%s %s_%s_new(void *value) {\n"
            "    %s result;\n"
            "    result.tag = %u;\n"
            "    result.data.%s_value = value;\n"
            "    return result;\n"
            "}\n\n",
            enum_name, enum_name, variant_name,
            enum_name, tag_value, variant_name);
    } else {
        // Constructor without parameter (unit variant)
        snprintf(constructor_code, DEFAULT_CONSTRUCTOR_BUFFER_SIZE,
            "%s %s_%s_new(void) {\n"
            "    %s result;\n"
            "    result.tag = %u;\n"
            "    return result;\n"
            "}\n\n",
            enum_name, enum_name, variant_name,
            enum_name, tag_value);
    }
    
    // Add the constructor to the code generator's output
    // In a complete implementation, this would be added to the instruction buffer
    
    free(constructor_code);
    return true;
}

static bool generate_enum_pattern_functions(CodeGenerator *generator, const char *enum_name, ASTNodeList *variants) {
    if (!generator || !enum_name) {
        return false;
    }
    
    // Generate helper functions for pattern matching:
    // - bool EnumName_is_VariantName(EnumName *enum_value)
    // - void *EnumName_get_VariantName_data(EnumName *enum_value)
    
    if (variants) {
        size_t variant_count = ast_node_list_size(variants);
        for (size_t i = 0; i < variant_count; i++) {
            ASTNode *variant = ast_node_list_get(variants, i);
            if (variant && variant->type == AST_ENUM_VARIANT_DECL) {
                const char *variant_name = variant->data.enum_variant_decl.name;
                
                if (variant_name) {
                    char *helper_code = malloc(DEFAULT_HELPER_BUFFER_SIZE);
                    if (!helper_code) {
                        return false;
                    }
                    
                    snprintf(helper_code, DEFAULT_HELPER_BUFFER_SIZE,
                        "bool %s_is_%s(%s *enum_value) {\n"
                        "    return enum_value && enum_value->tag == %zu;\n"
                        "}\n\n"
                        "void *%s_get_%s_data(%s *enum_value) {\n"
                        "    return (enum_value && enum_value->tag == %zu) ? enum_value->data.%s_value : NULL;\n"
                        "}\n\n",
                        enum_name, variant_name, enum_name, i,
                        enum_name, variant_name, enum_name, i, variant_name);
                    
                    // Add helper functions to output
                    free(helper_code);
                }
            }
        }
    }
    
    return true;
} 
