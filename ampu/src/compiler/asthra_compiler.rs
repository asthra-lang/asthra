use crate::config::{OptimizationLevel, Target};
use crate::error::{BuildError, Result};
use std::ffi::OsString;
use std::path::PathBuf;
use tokio::process::Command;
use tracing::{debug, warn};

#[derive(Debug, Clone)]
pub struct CompilerInvocation {
    pub source_files: Vec<PathBuf>,
    pub import_paths: Vec<PathBuf>,
    pub output_path: PathBuf,
    pub target: Target,
    pub optimization: OptimizationLevel,
    pub additional_args: Vec<OsString>,
}

#[derive(Debug)]
pub struct CompilerOutput {
    pub success: bool,
    pub exit_code: Option<i32>,
    pub stdout: String,
    pub stderr: String,
}

#[derive(Clone)]
pub struct AsthraCCompilerInterface {
    compiler_path: PathBuf,
}

impl AsthraCCompilerInterface {
    pub fn new(compiler_path: PathBuf) -> Self {
        Self { compiler_path }
    }

    pub async fn compile(&self, invocation: CompilerInvocation) -> Result<CompilerOutput> {
        debug!("Invoking Asthra compiler: {}", self.compiler_path.display());
        
        let mut cmd = Command::new(&self.compiler_path);
        
        // Add source files
        for source_file in &invocation.source_files {
            cmd.arg(source_file);
        }
        
        // Add output specification
        cmd.arg("-o").arg(&invocation.output_path);
        
        // Add include paths for import resolution
        for include_path in &invocation.import_paths {
            cmd.arg("-I").arg(include_path);
        }
        
        // Add target specification
        match invocation.target {
            Target::Native => {}, // Default
            Target::X86_64 => {
                cmd.arg("--target").arg("x86_64");
            }
            Target::Arm64 => {
                cmd.arg("--target").arg("arm64");
            }
            Target::Wasm32 => {
                cmd.arg("--target").arg("wasm32");
            }
        }
        
        // Add optimization level
        let opt_flag = match invocation.optimization {
            OptimizationLevel::None => "-O0",
            OptimizationLevel::Basic => "-O1", 
            OptimizationLevel::Standard => "-O2",
            OptimizationLevel::Aggressive => "-O3",
        };
        cmd.arg(opt_flag);
        
        // Add any additional arguments
        cmd.args(&invocation.additional_args);
        
        debug!("Compiler command: {:?}", cmd);
        
        // Execute with timeout
        let output = tokio::time::timeout(
            std::time::Duration::from_secs(300), // 5 minute timeout
            cmd.output()
        ).await??;
        
        let stdout = String::from_utf8_lossy(&output.stdout).to_string();
        let stderr = String::from_utf8_lossy(&output.stderr).to_string();
        
        if !output.status.success() {
            warn!("Compiler failed with exit code: {:?}", output.status.code());
            debug!("Compiler stdout: {}", stdout);
            debug!("Compiler stderr: {}", stderr);
        }
        
        Ok(CompilerOutput {
            success: output.status.success(),
            exit_code: output.status.code(),
            stdout,
            stderr,
        })
    }

    pub async fn check_version(&self) -> Result<String> {
        let mut cmd = Command::new(&self.compiler_path);
        cmd.arg("--version");
        
        let output = tokio::time::timeout(
            std::time::Duration::from_secs(10),
            cmd.output()
        ).await??;
        
        if output.status.success() {
            Ok(String::from_utf8_lossy(&output.stdout).trim().to_string())
        } else {
            Err(BuildError::Compiler(
                "Failed to get compiler version".to_string()
            ))
        }
    }

    pub async fn validate_compiler(&self) -> Result<()> {
        // Check if compiler exists and is executable
        if !self.compiler_path.exists() {
            return Err(BuildError::Compiler(format!(
                "Asthra compiler not found at: {}",
                self.compiler_path.display()
            )));
        }

        // Try to get version to verify it's working
        match self.check_version().await {
            Ok(version) => {
                debug!("Found Asthra compiler version: {}", version);
                Ok(())
            }
            Err(e) => {
                warn!("Compiler validation failed: {}", e);
                Err(BuildError::Compiler(format!(
                    "Asthra compiler at {} is not working properly: {}",
                    self.compiler_path.display(),
                    e
                )))
            }
        }
    }
}

pub struct ImportPathResolver {
    stdlib_root: PathBuf,
    internal_root: PathBuf,
    cache_dir: PathBuf,
}

impl ImportPathResolver {
    pub fn new(stdlib_root: PathBuf, internal_root: PathBuf, cache_dir: PathBuf) -> Self {
        Self {
            stdlib_root,
            internal_root,
            cache_dir,
        }
    }

    pub fn resolve_import_to_path(&self, import: &crate::resolver::ImportType) -> Result<PathBuf> {
        use crate::resolver::ImportType;
        
        match import {
            ImportType::Stdlib(path) => {
                // "stdlib/string" -> "/path/to/asthra/stdlib/string"
                let relative_path = path.strip_prefix("stdlib/").unwrap_or(path);
                Ok(self.stdlib_root.join(relative_path))
            }
            ImportType::Internal(path) => {
                // "internal/runtime/memory" -> "/path/to/asthra/internal/runtime/memory"
                let relative_path = path.strip_prefix("internal/").unwrap_or(path);
                Ok(self.internal_root.join(relative_path))
            }
            ImportType::ThirdParty(path) => {
                // "github.com/user/repo" -> "~/.asthra/cache/github.com/user/repo"
                Ok(self.cache_dir.join(path))
            }
            ImportType::Local(path) => {
                // "./utils" -> resolve relative to current package
                Ok(PathBuf::from(path))
            }
        }
    }

    pub fn build_include_paths(&self, imports: &[crate::resolver::ImportType]) -> Result<Vec<PathBuf>> {
        let mut include_paths = Vec::new();
        
        for import in imports {
            let resolved_path = self.resolve_import_to_path(import)?;
            if !include_paths.contains(&resolved_path) {
                include_paths.push(resolved_path);
            }
        }
        
        // Always include stdlib
        include_paths.push(self.stdlib_root.clone());
        
        Ok(include_paths)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use tempfile::TempDir;

    #[tokio::test]
    async fn test_compiler_interface_creation() {
        let temp_dir = TempDir::new().unwrap();
        let fake_compiler = temp_dir.path().join("asthra");
        
        let interface = AsthraCCompilerInterface::new(fake_compiler.clone());
        assert_eq!(interface.compiler_path, fake_compiler);
    }

    #[test]
    fn test_import_path_resolution() {
        let temp_dir = TempDir::new().unwrap();
        let stdlib_root = temp_dir.path().join("stdlib");
        let internal_root = temp_dir.path().join("internal");
        let cache_dir = temp_dir.path().join("cache");
        
        let resolver = ImportPathResolver::new(stdlib_root.clone(), internal_root.clone(), cache_dir.clone());
        
        use crate::resolver::ImportType;
        
        let stdlib_import = ImportType::Stdlib("stdlib/string".to_string());
        let resolved = resolver.resolve_import_to_path(&stdlib_import).unwrap();
        assert_eq!(resolved, stdlib_root.join("string"));
        
        let internal_import = ImportType::Internal("internal/runtime".to_string());
        let resolved = resolver.resolve_import_to_path(&internal_import).unwrap();
        assert_eq!(resolved, internal_root.join("runtime"));
        
        let third_party_import = ImportType::ThirdParty("github.com/user/repo".to_string());
        let resolved = resolver.resolve_import_to_path(&third_party_import).unwrap();
        assert_eq!(resolved, cache_dir.join("github.com/user/repo"));
    }
} 
