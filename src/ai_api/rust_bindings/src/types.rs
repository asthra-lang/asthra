//! Rust type definitions for Asthra Semantic API
//!
//! This module contains safe, idiomatic Rust types that wrap the C API data structures.

use std::fmt;

/// Information about a struct field
#[derive(Debug, Clone, PartialEq)]
pub struct FieldInfo {
    /// Field name
    pub name: String,
    /// Field type name
    pub type_name: String,
    /// Whether the field is public
    pub is_public: bool,
    /// Whether the field is mutable
    pub is_mutable: bool,
}

impl fmt::Display for FieldInfo {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let visibility = if self.is_public { "pub " } else { "priv " };
        let mutability = if self.is_mutable { "mut " } else { "" };
        write!(f, "{}{}{}: {}", visibility, mutability, self.name, self.type_name)
    }
}

/// Comprehensive information about a struct type
#[derive(Debug, Clone, PartialEq)]
pub struct StructInfo {
    /// Struct name
    pub name: String,
    /// List of struct fields
    pub fields: Vec<FieldInfo>,
    /// List of method names
    pub methods: Vec<String>,
    /// Whether the struct is generic
    pub is_generic: bool,
    /// Generic type parameter names
    pub type_parameters: Vec<String>,
}

impl fmt::Display for StructInfo {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "struct {}", self.name)?;
        
        if self.is_generic && !self.type_parameters.is_empty() {
            write!(f, "<{}>", self.type_parameters.join(", "))?;
        }
        
        writeln!(f, " {{")?;
        for field in &self.fields {
            writeln!(f, "    {},", field)?;
        }
        writeln!(f, "}}")?;
        
        if !self.methods.is_empty() {
            writeln!(f, "Methods: {}", self.methods.join(", "))?;
        }
        
        Ok(())
    }
}

/// Code location information
#[derive(Debug, Clone, PartialEq)]
pub struct CodeLocation {
    /// File path
    pub file_path: String,
    /// Line number (1-based)
    pub line: usize,
    /// Column number (1-based)
    pub column: usize,
    /// Context code around the location
    pub context_code: String,
}

impl fmt::Display for CodeLocation {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}:{}:{}", self.file_path, self.line, self.column)
    }
}

/// Symbol usage information
#[derive(Debug, Clone, PartialEq)]
pub struct SymbolUsage {
    /// Symbol name
    pub symbol_name: String,
    /// Usage type (declaration, reference, assignment, etc.)
    pub usage_type: String,
    /// Location of the usage
    pub location: CodeLocation,
    /// Scope containing the usage
    pub scope_name: String,
}

impl fmt::Display for SymbolUsage {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} {} at {} in scope {}", 
               self.usage_type, self.symbol_name, self.location, self.scope_name)
    }
}

/// Symbol location information
#[derive(Debug, Clone, PartialEq)]
pub struct SymbolLocation {
    /// File path
    pub file_path: String,
    /// Line number (1-based)
    pub line: usize,
    /// Column number (1-based)
    pub column: usize,
    /// Symbol type (struct, function, enum, etc.)
    pub symbol_type: String,
}

impl fmt::Display for SymbolLocation {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} {} at {}:{}:{}", 
               self.symbol_type, self.file_path, self.line, self.column, self.symbol_type)
    }
}

/// Type information
#[derive(Debug, Clone, PartialEq)]
pub struct TypeInfo {
    /// Type name
    pub name: String,
    /// Type category (primitive, struct, enum, etc.)
    pub category: String,
    /// Whether the type is mutable
    pub is_mutable: bool,
    /// Whether the type is generic
    pub is_generic: bool,
    /// Generic parameter names
    pub generic_params: Vec<String>,
    /// Type size in bytes (if known)
    pub size_bytes: Option<usize>,
    /// Type alignment (if known)
    pub alignment: Option<usize>,
    /// Whether the type is FFI compatible
    pub is_ffi_compatible: bool,
    /// Whether the type can be copied (vs moved)
    pub is_copy_type: bool,
}

impl fmt::Display for TypeInfo {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} {}", self.category, self.name)?;
        
        if self.is_generic && !self.generic_params.is_empty() {
            write!(f, "<{}>", self.generic_params.join(", "))?;
        }
        
        let mut attributes = Vec::new();
        if self.is_mutable { attributes.push("mutable"); }
        if self.is_ffi_compatible { attributes.push("ffi-compatible"); }
        if self.is_copy_type { attributes.push("copy"); }
        
        if !attributes.is_empty() {
            write!(f, " [{}]", attributes.join(", "))?;
        }
        
        if let Some(size) = self.size_bytes {
            write!(f, " (size: {} bytes)", size)?;
        }
        
        Ok(())
    }
} 
