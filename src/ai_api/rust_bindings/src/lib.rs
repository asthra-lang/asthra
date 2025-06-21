//! # Asthra Semantic API - Rust Bindings
//!
//! This crate provides safe, idiomatic Rust bindings for Asthra's semantic analysis engine.

use std::ffi::{CStr, CString};
use std::path::Path;
use std::ptr;

pub mod ffi;
pub mod types;
pub mod error;

pub use types::*;
pub use error::{AsthraSemanticsError, Result};

/// Main interface to Asthra's semantic analysis engine
pub struct AsthraSemanticsAPI {
    inner: *mut ffi::AsthraSemanticsAPI,
}

impl AsthraSemanticsAPI {
    /// Create a new semantic API instance for the given source file
    pub fn new(source_file: &Path) -> Result<Self> {
        let source_file_cstr = CString::new(source_file.to_string_lossy().as_ref())
            .map_err(|_| AsthraSemanticsError::InvalidPath)?;
        
        let inner = unsafe { ffi::asthra_ai_create_api(source_file_cstr.as_ptr()) };
        if inner.is_null() {
            return Err(AsthraSemanticsError::InitializationFailed);
        }
        
        Ok(AsthraSemanticsAPI { inner })
    }
    
    /// Initialize the API with source code content
    pub fn initialize_from_source(&self, source_code: &str) -> Result<()> {
        let source_code_cstr = CString::new(source_code)
            .map_err(|_| AsthraSemanticsError::InvalidInput)?;
        
        let success = unsafe { 
            ffi::asthra_ai_initialize_from_source(self.inner, source_code_cstr.as_ptr()) 
        };
        
        if success {
            Ok(())
        } else {
            Err(AsthraSemanticsError::AnalysisFailed)
        }
    }
    
    /// Get comprehensive information about a struct type
    pub fn get_struct_info(&self, struct_name: &str) -> Result<Option<StructInfo>> {
        let struct_name_cstr = CString::new(struct_name)
            .map_err(|_| AsthraSemanticsError::InvalidInput)?;
        
        let info_ptr = unsafe { 
            ffi::asthra_ai_get_struct_info(self.inner, struct_name_cstr.as_ptr()) 
        };
        
        if info_ptr.is_null() {
            return Ok(None);
        }
        
        // Convert C struct to Rust struct
        let info = unsafe { convert_struct_info(info_ptr) };
        
        // Free the C struct
        unsafe { ffi::asthra_ai_free_struct_info(info_ptr) };
        
        Ok(Some(info))
    }
    
    /// Infer the type of an Asthra expression
    pub fn infer_expression_type(&self, expression: &str) -> Result<String> {
        let expression_cstr = CString::new(expression)
            .map_err(|_| AsthraSemanticsError::InvalidInput)?;
        
        let type_ptr = unsafe { 
            ffi::asthra_ai_infer_expression_type(self.inner, expression_cstr.as_ptr()) 
        };
        
        if type_ptr.is_null() {
            return Err(AsthraSemanticsError::TypeInferenceFailed);
        }
        
        let type_name = unsafe { 
            CStr::from_ptr(type_ptr).to_string_lossy().into_owned() 
        };
        
        // Free the C string
        unsafe { libc::free(type_ptr as *mut libc::c_void) };
        
        Ok(type_name)
    }
    
    /// Check if two types are compatible
    pub fn check_type_compatibility(&self, expected: &str, actual: &str) -> Result<bool> {
        let expected_cstr = CString::new(expected)
            .map_err(|_| AsthraSemanticsError::InvalidInput)?;
        let actual_cstr = CString::new(actual)
            .map_err(|_| AsthraSemanticsError::InvalidInput)?;
        
        let compatible = unsafe { 
            ffi::asthra_ai_check_type_compatibility(
                self.inner, 
                expected_cstr.as_ptr(), 
                actual_cstr.as_ptr()
            ) 
        };
        
        Ok(compatible)
    }
}

impl Drop for AsthraSemanticsAPI {
    fn drop(&mut self) {
        unsafe { ffi::asthra_ai_destroy_api(self.inner) };
    }
}

// Thread safety: The C API uses mutexes for thread safety
unsafe impl Send for AsthraSemanticsAPI {}
unsafe impl Sync for AsthraSemanticsAPI {}

// Helper functions for converting C data structures to Rust
unsafe fn convert_struct_info(info_ptr: *mut ffi::AIStructInfo) -> StructInfo {
    let info = &*info_ptr;
    
    let name = CStr::from_ptr(info.struct_name).to_string_lossy().into_owned();
    
    // Convert fields
    let fields = if info.field_count > 0 && !info.fields.is_null() {
        let field_slice = std::slice::from_raw_parts(info.fields, info.field_count);
        field_slice.iter().map(|field| {
            FieldInfo {
                name: CStr::from_ptr(field.name).to_string_lossy().into_owned(),
                type_name: CStr::from_ptr(field.type_name).to_string_lossy().into_owned(),
                is_public: field.is_public,
                is_mutable: field.is_mutable,
            }
        }).collect()
    } else {
        Vec::new()
    };
    
    // Convert methods
    let methods = if info.method_count > 0 && !info.method_names.is_null() {
        let method_slice = std::slice::from_raw_parts(info.method_names, info.method_count);
        method_slice.iter().map(|method_ptr| {
            CStr::from_ptr(*method_ptr).to_string_lossy().into_owned()
        }).collect()
    } else {
        Vec::new()
    };
    
    StructInfo {
        name,
        fields,
        methods,
        is_generic: info.is_generic,
        type_parameters: Vec::new(), // Simplified for now
    }
} 
