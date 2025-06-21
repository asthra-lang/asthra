use std::collections::HashMap;
use std::path::{Path, PathBuf};
use tempfile::TempDir;
use tokio::fs;

use crate::resolver::{ImportParser, ImportType, ImportError, ImportErrorType};
use crate::error::BuildError;
use crate::compiler::BuildContext;

#[tokio::test]
async fn test_comprehensive_import_validation() {
    let temp_project = create_test_project().await;
    
    // Test valid imports compile successfully
    write_file(&temp_project, "src/main.asthra", r#"
        package main;
        import "stdlib/string";
        import "github.com/example/testpkg";
        import "./utils";
        
        fn main() -> void {
            println("Hello, world!");
        }
    "#).await;
    
    // Create utils file for local import
    write_file(&temp_project, "src/utils.asthra", r#"
        package utils;
        
        pub fn helper() -> i32 {
            return 42;
        }
    "#).await;
    
    let result = build_project(&temp_project).await;
    assert!(result.is_ok(), "Valid imports should compile successfully");
}

#[tokio::test]
async fn test_invalid_imports_fail_with_specific_errors() {
    let temp_project = create_test_project().await;
    
    // Test invalid imports fail with specific errors
    write_file(&temp_project, "src/invalid.asthra", r#"
        package invalid;
        import "";
        import "internal/runtime";
        import "malformed  path";
        import "github.com/user";
        import "local_without_prefix";
        
        fn main() -> void {}
    "#).await;
    
    let result = build_project(&temp_project).await;
    let errors = result.unwrap_err();
    
    assert_contains_error(&errors, "Empty import path");
    assert_contains_error(&errors, "Cannot access internal package");
    assert_contains_error(&errors, "Invalid whitespace in import path");
    assert_contains_error(&errors, "GitHub imports require format");
    assert_contains_error(&errors, "Local imports must use");
}

#[tokio::test]
async fn test_stdlib_import_validation() {
    let parser = ImportParser::new();
    
    // Valid stdlib imports
    let valid_stdlib = vec![
        "stdlib/string",
        "stdlib/io",
        "stdlib/collections",
        "stdlib/collections/hashmap",
        "stdlib/math",
        "stdlib/os",
        "stdlib/net",
    ];
    
    for import_path in valid_stdlib {
        let result = parser.classify_import(import_path);
        match result {
            ImportType::Stdlib(path) => {
                assert_eq!(path, import_path.strip_prefix("stdlib/").unwrap());
            }
            _ => panic!("Expected stdlib import type for: {}", import_path),
        }
    }
    
    // Invalid stdlib imports
    let invalid_stdlib = vec![
        "std/string",           // Wrong prefix
        "stdlib",               // Missing module path
        "stdlib/",              // Empty module path
        "stdlib//string",       // Double slash
        "stdlib/non_existent",  // Non-existent module (would be caught at build time)
    ];
    
    for import_path in invalid_stdlib {
        let result = parser.classify_import(import_path);
        assert!(
            !matches!(result, ImportType::Stdlib(_)),
            "Should not classify as stdlib: {}",
            import_path
        );
    }
}

#[tokio::test]
async fn test_github_import_validation() {
    let parser = ImportParser::new();
    
    // Valid GitHub imports
    let valid_github = vec![
        "github.com/user/repo",
        "github.com/organization/project",
        "gitlab.com/user/package",
        "bitbucket.org/team/library",
        "github.com/user/repo/subpackage",
    ];
    
    for import_path in valid_github {
        let result = parser.classify_import(import_path);
        match result {
            ImportType::ThirdParty(path) => {
                assert_eq!(path, import_path);
            }
            _ => panic!("Expected third-party import type for: {}", import_path),
        }
    }
    
    // Invalid GitHub imports
    let invalid_github = vec![
        "github.com/user",      // Missing repository
        "github.com",           // Missing user and repo
        "github/user/repo",     // Missing .com
        "githubcom/user/repo",  // Invalid domain
        "github.com//repo",     // Empty user
        "github.com/user/",     // Empty repo
    ];
    
    for import_path in invalid_github {
        let result = parser.validate_github_format(import_path, 1);
        assert!(
            result.is_err(),
            "Should reject invalid GitHub path: {}",
            import_path
        );
    }
}

#[tokio::test]
async fn test_local_import_validation() {
    let parser = ImportParser::new();
    let temp_dir = TempDir::new().unwrap();
    let context = create_parse_context(&temp_dir);
    
    // Create some local files for testing
    fs::create_dir_all(temp_dir.path().join("utils")).await.unwrap();
    fs::write(temp_dir.path().join("utils/helper.asthra"), "package helper;").await.unwrap();
    fs::write(temp_dir.path().join("common.asthra"), "package common;").await.unwrap();
    
    // Valid local imports
    let valid_local = vec![
        "./utils",
        "./utils/helper",
        "./common",
        "../parent_directory",
    ];
    
    for import_path in valid_local {
        let result = parser.classify_import(import_path);
        match result {
            ImportType::Local(path) => {
                assert_eq!(path, import_path);
            }
            _ => panic!("Expected local import type for: {}", import_path),
        }
    }
    
    // Invalid local imports (missing prefix)
    let invalid_local = vec![
        "utils",              // Missing ./ prefix
        "common",             // Missing ./ prefix
        "parent_directory",   // Missing ../ prefix
        "/absolute/path",     // Absolute path not supported
    ];
    
    for import_path in invalid_local {
        let result = parser.classify_import(import_path);
        assert!(
            !matches!(result, ImportType::Local(_)),
            "Should not classify as local import: {}",
            import_path
        );
    }
}

#[tokio::test]
async fn test_internal_import_access_control() {
    let parser = ImportParser::new();
    
    // Test user code context (should reject internal imports)
    let user_context = ParseContext {
        package_type: PackageType::User,
        file_path: PathBuf::from("src/main.asthra"),
        line_number: 1,
    };
    
    let result = parser.validate_import_early("internal/runtime", &user_context);
    assert!(
        result.is_err(),
        "User code should not be allowed to import internal packages"
    );
    
    if let Err(BuildError::ImportAccessViolation { reason, .. }) = result {
        assert!(reason.contains("User code cannot import internal packages"));
    } else {
        panic!("Expected ImportAccessViolation error");
    }
    
    // Test stdlib context (should allow internal imports)
    let stdlib_context = ParseContext {
        package_type: PackageType::Stdlib,
        file_path: PathBuf::from("stdlib/src/collections.asthra"),
        line_number: 1,
    };
    
    let result = parser.validate_import_early("internal/runtime", &stdlib_context);
    assert!(
        result.is_ok(),
        "Stdlib code should be allowed to import internal packages"
    );
}

#[test]
fn test_cross_platform_error_consistency() {
    // Ensure error messages are identical across platforms
    let windows_error = validate_import_on_platform("", Platform::Windows);
    let linux_error = validate_import_on_platform("", Platform::Linux);
    let macos_error = validate_import_on_platform("", Platform::MacOS);
    
    assert_eq!(windows_error.error_type, linux_error.error_type);
    assert_eq!(linux_error.error_type, macos_error.error_type);
    
    // Messages should be identical except for file paths
    assert_eq!(
        normalize_paths(&windows_error.message),
        normalize_paths(&linux_error.message)
    );
    assert_eq!(
        normalize_paths(&linux_error.message),
        normalize_paths(&macos_error.message)
    );
}

#[tokio::test]
async fn test_error_message_determinism() {
    let parser = ImportParser::new();
    
    // Test that same errors produce consistent messages
    let context1 = ParseContext {
        package_type: PackageType::User,
        file_path: PathBuf::from("src/file1.asthra"),
        line_number: 5,
    };
    
    let context2 = ParseContext {
        package_type: PackageType::User,
        file_path: PathBuf::from("src/file2.asthra"),
        line_number: 10,
    };
    
    let error1 = parser.validate_import_early("", &context1).unwrap_err();
    let error2 = parser.validate_import_early("", &context2).unwrap_err();
    
    // Same error type should produce identical message structure
    assert_same_error_format(&error1, &error2);
    assert_different_line_numbers(&error1, &error2);
}

#[tokio::test]
async fn test_import_path_format_validation() {
    let parser = ImportParser::new();
    let context = create_test_context();
    
    // Test whitespace validation
    let whitespace_errors = vec![
        (" stdlib/string", "leading whitespace"),
        ("stdlib/string ", "trailing whitespace"),
        ("stdlib  string", "double space"),
        ("stdlib\tstring", "tab character"),
        ("stdlib\nstring", "newline character"),
    ];
    
    for (import_path, description) in whitespace_errors {
        let result = parser.validate_import_early(import_path, &context);
        assert!(
            result.is_err(),
            "Should reject import with {}: '{}'",
            description,
            import_path
        );
    }
    
    // Test character validation
    let invalid_chars = vec![
        ("stdlib/str*ing", "asterisk"),
        ("stdlib/str?ing", "question mark"),
        ("stdlib/str|ing", "pipe"),
        ("stdlib/str<ing", "less than"),
        ("stdlib/str>ing", "greater than"),
    ];
    
    for (import_path, description) in invalid_chars {
        let result = parser.validate_import_early(import_path, &context);
        assert!(
            result.is_err(),
            "Should reject import with {}: '{}'",
            description,
            import_path
        );
    }
}

#[tokio::test]
async fn test_dependency_resolution() {
    let temp_project = create_test_project().await;
    
    // Create a file with external dependency
    write_file(&temp_project, "src/main.asthra", r#"
        package main;
        import "github.com/example/json";
        import "stdlib/string";
        
        fn main() -> void {
            let data = json.parse("{}");
        }
    "#).await;
    
    let parser = ImportParser::new();
    let imports = parser.extract_imports_from_file(&temp_project.join("src/main.asthra")).await.unwrap();
    
    // Should identify external dependency
    let external_deps: Vec<_> = imports
        .iter()
        .filter_map(|import| match &import.import_type {
            ImportType::ThirdParty(path) => Some(path),
            _ => None,
        })
        .collect();
    
    assert_eq!(external_deps.len(), 1);
    assert_eq!(external_deps[0], "github.com/example/json");
    
    // Should identify stdlib dependency
    let stdlib_deps: Vec<_> = imports
        .iter()
        .filter_map(|import| match &import.import_type {
            ImportType::Stdlib(path) => Some(path),
            _ => None,
        })
        .collect();
    
    assert_eq!(stdlib_deps.len(), 1);
    assert_eq!(stdlib_deps[0], "string");
}

#[tokio::test]
async fn test_circular_dependency_detection() {
    let temp_project = create_test_project().await;
    
    // Create circular dependency
    write_file(&temp_project, "src/module_a.asthra", r#"
        package module_a;
        import "./module_b";
        
        pub fn func_a() -> i32 {
            return module_b.func_b();
        }
    "#).await;
    
    write_file(&temp_project, "src/module_b.asthra", r#"
        package module_b;
        import "./module_a";
        
        pub fn func_b() -> i32 {
            return module_a.func_a();
        }
    "#).await;
    
    let result = build_project(&temp_project).await;
    assert!(result.is_err(), "Circular dependencies should be detected");
    
    let errors = result.unwrap_err();
    assert_contains_error(&errors, "Circular dependency detected");
}

#[tokio::test]
async fn test_performance_regression() {
    let parser = ImportParser::new();
    let context = create_test_context();
    
    // Test validation performance with many imports
    let start_time = std::time::Instant::now();
    
    for i in 0..1000 {
        let import_path = format!("stdlib/module_{}", i);
        let _ = parser.validate_import_early(&import_path, &context);
    }
    
    let elapsed = start_time.elapsed();
    
    // Validation should be fast (< 10ms for 1000 imports)
    assert!(
        elapsed.as_millis() < 10,
        "Import validation performance regression: {}ms",
        elapsed.as_millis()
    );
}

// Helper functions for testing

async fn create_test_project() -> TempDir {
    let temp_dir = TempDir::new().unwrap();
    
    // Create basic project structure
    fs::create_dir_all(temp_dir.path().join("src")).await.unwrap();
    
    // Create project file
    write_file(&temp_dir, "Project.toml", r#"
        [package]
        name = "test_project"
        version = "0.1.0"
        
        [dependencies]
        "github.com/example/testpkg" = "1.0.0"
    "#).await;
    
    temp_dir
}

async fn write_file(base_dir: &TempDir, relative_path: &str, content: &str) {
    let file_path = base_dir.path().join(relative_path);
    if let Some(parent) = file_path.parent() {
        fs::create_dir_all(parent).await.unwrap();
    }
    fs::write(file_path, content).await.unwrap();
}

async fn build_project(project_dir: &TempDir) -> Result<(), BuildError> {
    let context = BuildContext::new(project_dir.path());
    context.validate_all_imports().await
}

fn assert_contains_error(errors: &BuildError, expected_message: &str) {
    let error_string = format!("{:?}", errors);
    assert!(
        error_string.contains(expected_message),
        "Expected error message '{}' not found in: {}",
        expected_message,
        error_string
    );
}

fn create_test_context() -> ParseContext {
    ParseContext {
        package_type: PackageType::User,
        file_path: PathBuf::from("src/test.asthra"),
        line_number: 1,
    }
}

fn create_parse_context(temp_dir: &TempDir) -> ParseContext {
    ParseContext {
        package_type: PackageType::User,
        file_path: temp_dir.path().join("src/test.asthra"),
        line_number: 1,
    }
}

#[derive(Debug, Clone)]
enum Platform {
    Windows,
    Linux,
    MacOS,
}

fn validate_import_on_platform(import_path: &str, platform: Platform) -> ImportError {
    let parser = ImportParser::new();
    let context = ParseContext {
        package_type: PackageType::User,
        file_path: match platform {
            Platform::Windows => PathBuf::from("C:\\project\\src\\test.asthra"),
            Platform::Linux => PathBuf::from("/home/user/project/src/test.asthra"),
            Platform::MacOS => PathBuf::from("/Users/user/project/src/test.asthra"),
        },
        line_number: 1,
    };
    
    parser.validate_import_early(import_path, &context).unwrap_err()
}

fn normalize_paths(message: &str) -> String {
    message
        .replace("C:\\", "/")
        .replace("\\", "/")
        .replace("/home/user/", "/user/")
        .replace("/Users/user/", "/user/")
}

fn assert_same_error_format(error1: &BuildError, error2: &BuildError) {
    // Extract error types (should be identical)
    let type1 = std::mem::discriminant(error1);
    let type2 = std::mem::discriminant(error2);
    assert_eq!(type1, type2, "Error types should be identical");
    
    // Extract error messages without file paths
    let msg1 = normalize_paths(&format!("{}", error1));
    let msg2 = normalize_paths(&format!("{}", error2));
    
    // Should have same structure (ignoring line numbers)
    let norm1 = msg1.replace("line 5", "line X").replace("column 8", "column Y");
    let norm2 = msg2.replace("line 10", "line X").replace("column 8", "column Y");
    
    assert_eq!(norm1, norm2, "Error message structure should be identical");
}

fn assert_different_line_numbers(error1: &BuildError, error2: &BuildError) {
    let msg1 = format!("{}", error1);
    let msg2 = format!("{}", error2);
    
    // Should contain different line numbers
    assert!(msg1.contains("line 5") && msg2.contains("line 10"));
}

#[derive(Debug, Clone)]
struct ParseContext {
    package_type: PackageType,
    file_path: PathBuf,
    line_number: usize,
}

#[derive(Debug, Clone)]
enum PackageType {
    User,
    Stdlib,
    Internal,
}

impl ImportParser {
    pub fn new() -> Self {
        Self {
            stdlib_modules: create_stdlib_module_registry(),
        }
    }
    
    pub fn classify_import(&self, import_path: &str) -> ImportType {
        if import_path.starts_with("stdlib/") {
            ImportType::Stdlib(import_path.strip_prefix("stdlib/").unwrap().to_string())
        } else if import_path.starts_with("internal/") {
            ImportType::Internal(import_path.strip_prefix("internal/").unwrap().to_string())
        } else if import_path.starts_with("./") || import_path.starts_with("../") {
            ImportType::Local(import_path.to_string())
        } else if import_path.contains(".com/") || import_path.contains(".org/") {
            ImportType::ThirdParty(import_path.to_string())
        } else {
            ImportType::Unknown(import_path.to_string())
        }
    }
    
    pub fn validate_import_early(&self, import_path: &str, context: &ParseContext) -> Result<ImportValidation, BuildError> {
        // Empty path validation
        if import_path.is_empty() {
            return Err(BuildError::InvalidImportFormat {
                import_path: import_path.to_string(),
                expected_format: "Non-empty import path".to_string(),
                line_number: context.line_number,
            });
        }
        
        // Whitespace validation
        if import_path.starts_with(' ') || import_path.ends_with(' ') || import_path.contains("  ") {
            return Err(BuildError::InvalidImportFormat {
                import_path: import_path.to_string(),
                expected_format: "No leading/trailing/double whitespace".to_string(),
                line_number: context.line_number,
            });
        }
        
        let import_type = self.classify_import(import_path);
        
        match import_type {
            ImportType::Internal(_) if matches!(context.package_type, PackageType::User) => {
                Err(BuildError::ImportAccessViolation {
                    import_path: import_path.to_string(),
                    reason: "User code cannot import internal packages".to_string(),
                    suggestion: "Use stdlib alternatives or third-party packages".to_string(),
                    line_number: context.line_number,
                })
            }
            ImportType::ThirdParty(path) => {
                self.validate_github_format(&path, context.line_number)?;
                Ok(ImportValidation {
                    import_type,
                    resolved_path: PathBuf::from(path),
                    is_available: true,
                })
            }
            _ => Ok(ImportValidation {
                import_type,
                resolved_path: PathBuf::from(import_path),
                is_available: true,
            })
        }
    }
    
    pub fn validate_github_format(&self, path: &str, line_number: usize) -> Result<(), BuildError> {
        let parts: Vec<&str> = path.split('/').collect();
        if parts.len() < 3 {
            return Err(BuildError::InvalidImportFormat {
                import_path: path.to_string(),
                expected_format: "github.com/user/repository".to_string(),
                line_number,
            });
        }
        
        if !parts[0].ends_with(".com") && !parts[0].ends_with(".org") {
            return Err(BuildError::InvalidImportFormat {
                import_path: path.to_string(),
                expected_format: "Valid domain like github.com, gitlab.com, bitbucket.org".to_string(),
                line_number,
            });
        }
        
        Ok(())
    }
    
    pub async fn extract_imports_from_file(&self, file_path: &Path) -> Result<Vec<ImportInfo>, BuildError> {
        let content = fs::read_to_string(file_path).await.map_err(|e| BuildError::FileNotFound {
            path: file_path.to_path_buf(),
            source: e,
        })?;
        
        let mut imports = Vec::new();
        for (line_num, line) in content.lines().enumerate() {
            if let Some(import_path) = extract_import_from_line(line) {
                let import_type = self.classify_import(&import_path);
                imports.push(ImportInfo {
                    import_path,
                    import_type,
                    line_number: line_num + 1,
                });
            }
        }
        
        Ok(imports)
    }
}

fn extract_import_from_line(line: &str) -> Option<String> {
    let trimmed = line.trim();
    if trimmed.starts_with("import") {
        // Simple regex-like extraction for testing
        if let Some(start) = trimmed.find('"') {
            if let Some(end) = trimmed[start + 1..].find('"') {
                return Some(trimmed[start + 1..start + 1 + end].to_string());
            }
        }
    }
    None
}

fn create_stdlib_module_registry() -> HashMap<String, bool> {
    let mut modules = HashMap::new();
    modules.insert("string".to_string(), true);
    modules.insert("io".to_string(), true);
    modules.insert("collections".to_string(), true);
    modules.insert("collections/hashmap".to_string(), true);
    modules.insert("math".to_string(), true);
    modules.insert("os".to_string(), true);
    modules.insert("net".to_string(), true);
    modules.insert("crypto".to_string(), true);
    modules
}

#[derive(Debug, Clone)]
struct ImportValidation {
    import_type: ImportType,
    resolved_path: PathBuf,
    is_available: bool,
}

#[derive(Debug, Clone)]
struct ImportInfo {
    import_path: String,
    import_type: ImportType,
    line_number: usize,
}

// Mock types for testing
#[derive(Debug, Clone)]
struct ImportParser {
    stdlib_modules: HashMap<String, bool>,
}

#[derive(Debug, Clone)]
enum ImportType {
    Stdlib(String),
    Internal(String),
    ThirdParty(String),
    Local(String),
    Unknown(String),
}

impl BuildContext {
    fn new(project_path: &Path) -> Self {
        Self {
            project_path: project_path.to_path_buf(),
            parser: ImportParser::new(),
        }
    }
    
    async fn validate_all_imports(&self) -> Result<(), BuildError> {
        // Mock implementation for testing
        let src_dir = self.project_path.join("src");
        if !src_dir.exists() {
            return Ok(());
        }
        
        let mut entries = fs::read_dir(&src_dir).await.map_err(|e| BuildError::FileNotFound {
            path: src_dir.clone(),
            source: e,
        })?;
        
        while let Some(entry) = entries.next_entry().await.map_err(|e| BuildError::FileNotFound {
            path: src_dir.clone(),
            source: e,
        })? {
            if entry.path().extension().map_or(false, |ext| ext == "asthra") {
                let imports = self.parser.extract_imports_from_file(&entry.path()).await?;
                for import_info in imports {
                    let context = ParseContext {
                        package_type: PackageType::User,
                        file_path: entry.path(),
                        line_number: import_info.line_number,
                    };
                    self.parser.validate_import_early(&import_info.import_path, &context)?;
                }
            }
        }
        
        Ok(())
    }
}

#[derive(Debug)]
struct BuildContext {
    project_path: PathBuf,
    parser: ImportParser,
} 
