use std::path::PathBuf;
use thiserror::Error;

#[derive(Error, Debug)]
pub enum BuildError {
    #[error("Import access violation: {message}")]
    AccessViolation { message: String },
    
    #[error("Dependency resolution failed: {0}")]
    DependencyResolution(String),
    
    #[error("Compiler error: {0}")]
    Compiler(String),
    
    #[error("Parser error: {0}")]
    Parser(String),
    
    #[error("Compilation failed for package '{package}': {stderr}")]
    CompilationFailed {
        package: String,
        stdout: String,
        stderr: String,
        exit_code: Option<i32>,
    },
    
    #[error("Invalid import path: {0}")]
    InvalidImportPath(String),
    
    #[error("Version conflict for package '{package}': required {required}, existing {existing}")]
    VersionConflict {
        package: String,
        required: String,
        existing: String,
        dependent: Option<String>,
    },
    
    #[error("Circular dependency detected: {cycle:?}")]
    CircularDependency { cycle: Vec<String> },
    
    #[error("Package not found: {0}")]
    PackageNotFound(String),
    
    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),
    
    #[error("IO error: {0}")]
    IoString(String),
    
    #[error("Git error: {0}")]
    Git(#[from] git2::Error),
    
    #[error("TOML parsing error: {0}")]
    TomlParsing(#[from] toml::de::Error),
    
    #[error("TOML serialization error: {0}")]
    TomlSerialization(#[from] toml::ser::Error),
    
    #[error("JSON error: {0}")]
    Json(#[from] serde_json::Error),
    
    #[error("HTTP request error: {0}")]
    Http(#[from] reqwest::Error),
    
    #[error("Regex error: {0}")]
    Regex(#[from] regex::Error),
    
    #[error("Task join error: {0}")]
    TaskJoin(#[from] tokio::task::JoinError),
    
    #[error("Timeout error: {0}")]
    Timeout(#[from] tokio::time::error::Elapsed),
    
    #[error("Walkdir error: {0}")]
    Walkdir(#[from] walkdir::Error),
    
    #[error("Semver error: {0}")]
    Semver(#[from] semver::Error),
    
    #[error("Missing environment variable: {0}")]
    MissingEnvironmentVariable(String),
    
    #[error("Invalid configuration: {0}")]
    InvalidConfiguration(String),
    
    #[error("Network error: {0}")]
    Network(String),
    
    #[error("Cache error: {0}")]
    Cache(String),
}

/// Fast check specific errors
#[derive(Error, Debug)]
pub enum AmpuError {
    #[error("Build error: {0}")]
    Build(#[from] BuildError),
    
    #[error("Fast check engine creation failed")]
    EngineCreationFailed,
    
    #[error("Fast check configuration creation failed")]
    ConfigCreationFailed,
    
    #[error("Check failed for: {0}")]
    CheckFailed(String),
    
    #[error("Invalid path: {0}")]
    InvalidPath(String),
    
    #[error("Invalid input: {0}")]
    InvalidInput(String),
    
    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),
    
    #[error("JSON error: {0}")]
    Json(#[from] serde_json::Error),
}

#[derive(Error, Debug)]
pub enum AccessViolation {
    #[error("User code cannot import internal package: {package}")]
    UserToInternal { package: String },
    
    #[error("Third-party code cannot import internal package: {package}")]
    ThirdPartyToInternal { package: String },
}

#[derive(Debug)]
pub struct ImportError {
    pub file: PathBuf,
    pub line: usize,
    pub column: usize,
    pub violation: AccessViolation,
}

impl std::fmt::Display for ImportError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{}:{}:{}: {}",
            self.file.display(),
            self.line,
            self.column,
            self.violation
        )
    }
}

pub type Result<T> = std::result::Result<T, BuildError>; 
