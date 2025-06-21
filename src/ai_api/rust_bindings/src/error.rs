//! Error handling for Asthra Semantic API
//!
//! This module defines error types and result types for the Rust bindings.

use std::fmt;
use std::error::Error;

/// Result type for Asthra Semantic API operations
pub type Result<T> = std::result::Result<T, AsthraSemanticsError>;

/// Error types for Asthra Semantic API operations
#[derive(Debug, Clone, PartialEq)]
pub enum AsthraSemanticsError {
    /// Invalid file path provided
    InvalidPath,
    /// Failed to initialize the semantic API
    InitializationFailed,
    /// Invalid input provided to API function
    InvalidInput,
    /// Symbol not found in the symbol table
    SymbolNotFound,
    /// Type inference failed for the given expression
    TypeInferenceFailed,
    /// Semantic analysis failed
    AnalysisFailed,
    /// Memory allocation failed
    OutOfMemory,
    /// Thread synchronization error
    ThreadError,
    /// FFI conversion error
    ConversionError,
    /// Generic API error with message
    ApiError(String),
}

impl fmt::Display for AsthraSemanticsError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            AsthraSemanticsError::InvalidPath => {
                write!(f, "Invalid file path provided")
            }
            AsthraSemanticsError::InitializationFailed => {
                write!(f, "Failed to initialize Asthra semantic API")
            }
            AsthraSemanticsError::InvalidInput => {
                write!(f, "Invalid input provided to API function")
            }
            AsthraSemanticsError::SymbolNotFound => {
                write!(f, "Symbol not found in the symbol table")
            }
            AsthraSemanticsError::TypeInferenceFailed => {
                write!(f, "Type inference failed for the given expression")
            }
            AsthraSemanticsError::AnalysisFailed => {
                write!(f, "Semantic analysis failed")
            }
            AsthraSemanticsError::OutOfMemory => {
                write!(f, "Memory allocation failed")
            }
            AsthraSemanticsError::ThreadError => {
                write!(f, "Thread synchronization error")
            }
            AsthraSemanticsError::ConversionError => {
                write!(f, "FFI data conversion error")
            }
            AsthraSemanticsError::ApiError(msg) => {
                write!(f, "API error: {}", msg)
            }
        }
    }
}

impl Error for AsthraSemanticsError {
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        None
    }
}

impl From<std::ffi::NulError> for AsthraSemanticsError {
    fn from(_: std::ffi::NulError) -> Self {
        AsthraSemanticsError::ConversionError
    }
}

impl From<std::str::Utf8Error> for AsthraSemanticsError {
    fn from(_: std::str::Utf8Error) -> Self {
        AsthraSemanticsError::ConversionError
    }
} 
