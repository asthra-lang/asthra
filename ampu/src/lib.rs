pub mod cli;
pub mod config;
pub mod compiler;
pub mod downloader;
pub mod error;
pub mod formatter;
pub mod resolver;
pub mod storage;

// Re-export commonly used types
pub use error::{BuildError, Result};
pub use config::{AsthraMod, BuildConfig, PackageConfig, Target, OptimizationLevel};
pub use resolver::{ImportType, ImportGraph, ResolvedDependencies};

/// Version information for the ampu build tool
pub const VERSION: &str = env!("CARGO_PKG_VERSION");
pub const NAME: &str = env!("CARGO_PKG_NAME");

/// Main entry point for the ampu build tool library
pub async fn run() -> Result<()> {
    cli::run_cli().await
}

/// Initialize a new Asthra project in the given directory
pub async fn init_project(
    name: &str,
    path: &std::path::Path,
    is_lib: bool,
) -> Result<()> {
    use tokio::fs;

    // Create project directory if it doesn't exist
    if !path.exists() {
        fs::create_dir_all(path).await?;
    }

    // Create asthra.toml
    let asthra_toml = create_asthra_toml(name, is_lib);
    let asthra_toml_path = path.join("asthra.toml");
    fs::write(asthra_toml_path, asthra_toml).await?;

    // Create src directory
    let src_dir = path.join("src");
    fs::create_dir_all(&src_dir).await?;

    // Create main file
    let main_file_name = if is_lib { "lib.asthra" } else { "main.asthra" };
    let main_file_path = src_dir.join(main_file_name);
    let main_content = create_main_file_content(is_lib);
    fs::write(main_file_path, main_content).await?;

    // Create .gitignore
    let gitignore_path = path.join(".gitignore");
    let gitignore_content = create_gitignore_content();
    fs::write(gitignore_path, gitignore_content).await?;

    // Create README.md
    let readme_path = path.join("README.md");
    let readme_content = create_readme_content(name);
    fs::write(readme_path, readme_content).await?;

    Ok(())
}

fn create_asthra_toml(name: &str, _is_lib: bool) -> String {
    format!(
        r#"[package]
name = "{}"
version = "0.1.0"
asthra_version = "1.0"
description = "A new Asthra project"
authors = ["Your Name <your.email@example.com>"]

[dependencies]
# Add your dependencies here
# "github.com/asthra-lang/json" = "1.0.0"

[dev-dependencies]
# Add your development dependencies here
# "github.com/asthra-lang/test" = "1.0.0"

[build]
target = "native"
optimization = "standard"
"#,
        name
    )
}

fn create_main_file_content(is_lib: bool) -> String {
    if is_lib {
        r#"// This is a library module
// Export your public functions and types here

pub fn hello() -> string {
    return "Hello from library!";
}
"#.to_string()
    } else {
        r#"// This is the main entry point for your application
import "stdlib/io"

fn main() {
    io.println("Hello, Asthra!");
}
"#.to_string()
    }
}

fn create_gitignore_content() -> String {
    r#"# Build artifacts
/target/
*.o
*.so
*.dylib
*.dll

# IDE files
.vscode/
.idea/
*.swp
*.swo

# OS files
.DS_Store
Thumbs.db

# Asthra specific
asthra.lock
"#.to_string()
}

fn create_readme_content(name: &str) -> String {
    format!(
        r#"# {}

A new Asthra project.

## Building

```bash
ampu build
```

## Running

```bash
ampu run
```

## Testing

```bash
ampu test
```

## Adding Dependencies

```bash
ampu add github.com/asthra-lang/json
```
"#,
        name
    )
}

/// Validate that the current directory contains a valid Asthra project
pub async fn validate_project(path: &std::path::Path) -> Result<()> {
    let asthra_toml_path = path.join("asthra.toml");
    
    if !asthra_toml_path.exists() {
        return Err(BuildError::InvalidImportPath(
            "No asthra.toml found. This doesn't appear to be an Asthra project.".to_string()
        ));
    }

    // Try to parse the asthra.toml to ensure it's valid
    let _asthra_mod = config::asthra_toml::AsthraTOMLParser::parse_file(&asthra_toml_path).await?;
    
    Ok(())
}

/// Get information about the current project
pub async fn get_project_info(path: &std::path::Path) -> Result<config::AsthraMod> {
    let asthra_toml_path = path.join("asthra.toml");
    config::asthra_toml::AsthraTOMLParser::parse_file(&asthra_toml_path).await
}

#[cfg(test)]
mod tests {
    use super::*;
    use tempfile::TempDir;

    #[tokio::test]
    async fn test_init_project() {
        let temp_dir = TempDir::new().unwrap();
        let project_path = temp_dir.path().join("test_project");
        
        init_project("test_project", &project_path, false).await.unwrap();
        
        // Check that files were created
        assert!(project_path.join("asthra.toml").exists());
        assert!(project_path.join("src").exists());
        assert!(project_path.join("src/main.asthra").exists());
        assert!(project_path.join(".gitignore").exists());
        assert!(project_path.join("README.md").exists());
    }

    #[tokio::test]
    async fn test_init_library_project() {
        let temp_dir = TempDir::new().unwrap();
        let project_path = temp_dir.path().join("test_lib");
        
        init_project("test_lib", &project_path, true).await.unwrap();
        
        // Check that lib.asthra was created instead of main.asthra
        assert!(project_path.join("src/lib.asthra").exists());
        assert!(!project_path.join("src/main.asthra").exists());
    }

    #[tokio::test]
    async fn test_validate_project() {
        let temp_dir = TempDir::new().unwrap();
        let project_path = temp_dir.path().join("valid_project");
        
        // Initialize a project first
        init_project("valid_project", &project_path, false).await.unwrap();
        
        // Validation should pass
        validate_project(&project_path).await.unwrap();
        
        // Validation should fail for non-project directory
        let empty_dir = temp_dir.path().join("empty");
        tokio::fs::create_dir(&empty_dir).await.unwrap();
        assert!(validate_project(&empty_dir).await.is_err());
    }
} 
