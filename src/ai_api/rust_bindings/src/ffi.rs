//! FFI declarations for Asthra Semantic API
//!
//! This module contains the raw C function declarations and data structures
//! used to interface with Asthra's semantic analysis engine.

use libc::{c_char, c_int, size_t, c_void};

/// Opaque pointer to the C AsthraSemanticsAPI structure
#[repr(C)]
pub struct AsthraSemanticsAPI {
    _private: [u8; 0],
}

/// Opaque pointer to the C ASTNode structure
#[repr(C)]
pub struct ASTNode {
    pub node_type: ASTNodeType,
    pub location: SourceLocation,
    _private: [u8; 0],
}

/// AST node types (subset of the full enum)
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub enum ASTNodeType {
    AST_STRUCT_DECL = 1,
    AST_FUNCTION_DECL = 2,
    AST_ENUM_DECL = 3,
    // Add other types as needed
}

/// Source location information
#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub struct SourceLocation {
    pub line: c_int,
    pub column: c_int,
    pub offset: size_t,
}

/// AI-friendly struct information
#[repr(C)]
pub struct AIStructInfo {
    pub struct_name: *mut c_char,
    pub fields: *mut AIFieldInfo,
    pub field_count: size_t,
    pub method_names: *mut *mut c_char,
    pub method_count: size_t,
    pub is_generic: bool,
    pub type_parameters: *mut *mut c_char,
    pub type_param_count: size_t,
}

/// Field information
#[repr(C)]
pub struct AIFieldInfo {
    pub name: *mut c_char,
    pub type_name: *mut c_char,
    pub is_public: bool,
    pub is_mutable: bool,
}

/// Symbol usage information
#[repr(C)]
pub struct AISymbolUsage {
    pub symbol_name: *mut c_char,
    pub usage_type: *mut c_char,
    pub location: AICodeLocation,
    pub scope_name: *mut c_char,
}

/// Code location information
#[repr(C)]
pub struct AICodeLocation {
    pub file_path: *mut c_char,
    pub line: size_t,
    pub column: size_t,
    pub context_code: *mut c_char,
}

extern "C" {
    // Core API lifecycle
    pub fn asthra_ai_create_api(source_file: *const c_char) -> *mut AsthraSemanticsAPI;
    pub fn asthra_ai_destroy_api(api: *mut AsthraSemanticsAPI);
    pub fn asthra_ai_initialize_from_source(api: *mut AsthraSemanticsAPI, source_code: *const c_char) -> bool;
    
    // Symbol information queries
    pub fn asthra_ai_get_struct_info(api: *mut AsthraSemanticsAPI, struct_name: *const c_char) -> *mut AIStructInfo;
    pub fn asthra_ai_get_available_methods(api: *mut AsthraSemanticsAPI, type_name: *const c_char, count: *mut size_t) -> *mut *mut c_char;
    
    // Type system queries
    pub fn asthra_ai_infer_expression_type(api: *mut AsthraSemanticsAPI, expression: *const c_char) -> *mut c_char;
    pub fn asthra_ai_check_type_compatibility(api: *mut AsthraSemanticsAPI, expected: *const c_char, actual: *const c_char) -> bool;
    
    // AST navigation
    pub fn asthra_ai_find_declaration(api: *mut AsthraSemanticsAPI, symbol_name: *const c_char) -> *mut ASTNode;
    pub fn asthra_ai_find_symbol_usages(api: *mut AsthraSemanticsAPI, symbol_name: *const c_char, count: *mut size_t) -> *mut *mut AISymbolUsage;
    
    // Memory management
    pub fn asthra_ai_free_struct_info(info: *mut AIStructInfo);
    pub fn asthra_ai_free_string_array(array: *mut *mut c_char, count: size_t);
    pub fn asthra_ai_free_symbol_usage_array(usages: *mut *mut AISymbolUsage, count: size_t);
} 
