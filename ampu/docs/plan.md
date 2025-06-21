# Asthra Build Tool Implementation Plan

## Overview

The Asthra build tool (`ampu`) will be implemented as a **separate Rust binary** (similar to how Cargo works for Rust) that supports the hierarchical import system with access control. This approach avoids the bootstrapping problem since the Asthra compiler is written in C, and leverages Rust's excellent ecosystem for build tools.

The build tool will orchestrate compilation by calling the existing `asthra` compiler binary while handling dependency management, import resolution, and access control enforcement.

## Architecture Decision

**Implementation Language**: Rust
- **Rationale**: Proven pattern (Cargo), excellent ecosystem, memory safety, strong async/networking support
- **Distribution**: Single binary that can be installed alongside the Asthra compiler
- **Integration**: Calls the existing C-based `asthra` compiler as a subprocess

## Core Requirements

1. **Import Resolution**: Resolve `stdlib/*`, `internal/*`, and third-party imports
2. **Access Control**: Enforce import restrictions (user code cannot import internal packages)
3. **Dependency Management**: Download and manage third-party packages using Git
4. **Build Pipeline**: Compile packages in correct dependency order by orchestrating `asthra` compiler calls
5. **Module System**: Support for `asthra.toml` files (similar to `Cargo.toml`)
6. **Cross-Platform**: Work on Windows, macOS, and Linux

## Library Storage Strategy

### Overview

Based on Asthra's design decision to use **static libraries (.a/.lib) as the primary format** with selective dynamic linking for external C dependencies, `ampu` needs a comprehensive strategy for managing temporary library files during the build process.

### Storage Hierarchy

```
project_root/
├── asthra.toml                  # Project configuration
├── src/                         # Source files
├── target/                      # Build output directory (similar to Cargo)
│   ├── debug/                   # Debug build artifacts
│   │   ├── deps/                # Dependency libraries
│   │   │   ├── libstdlib_string.a
│   │   │   ├── libgithub_com_user_package.a
│   │   │   └── external/        # External C libraries
│   │   │       ├── liboqs.so    # Dynamic external deps
│   │   │       └── libssl.a     # Static external deps
│   │   ├── build/               # Intermediate build files
│   │   │   ├── package_name/    # Per-package build artifacts
│   │   │   │   ├── *.o          # Object files
│   │   │   │   └── *.d          # Dependency files
│   │   │   └── link_order.txt   # Dependency link order
│   │   ├── examples/            # Built examples
│   │   └── main                 # Final executable
│   ├── release/                 # Release build artifacts (same structure)
│   └── cache/                   # Build cache and metadata
│       ├── fingerprints/        # File change detection
│       ├── dep-info/           # Dependency information
│       └── library-metadata.json
└── .asthra/                     # Global cache (user-wide)
    ├── cache/                   # Downloaded packages
    │   └── github.com/user/pkg/ # Third-party packages
    ├── registry/                # Package registry cache
    └── config.toml              # Global configuration
```

### Library File Naming Convention

```rust
#[derive(Debug, Clone)]
pub struct LibraryArtifact {
    pub name: String,           // "stdlib_string", "github_com_user_package"
    pub version: Version,       // For cache invalidation
    pub library_type: LibraryType,
    pub file_path: PathBuf,     // Absolute path to .a/.so/.lib/.dll
    pub dependencies: Vec<String>, // Link dependencies
}

#[derive(Debug, Clone)]
pub enum LibraryType {
    Static,                     // .a (Unix) / .lib (Windows)
    Dynamic,                    // .so (Linux) / .dylib (macOS) / .dll (Windows)
    Object,                     // .o (Unix) / .obj (Windows) - intermediate
}

impl LibraryArtifact {
    pub fn generate_filename(&self) -> String {
        let sanitized_name = self.name.replace("/", "_").replace("-", "_");
        match self.library_type {
            LibraryType::Static => {
                #[cfg(windows)]
                return format!("{}.lib", sanitized_name);
                #[cfg(not(windows))]
                return format!("lib{}.a", sanitized_name);
            }
            LibraryType::Dynamic => {
                #[cfg(target_os = "linux")]
                return format!("lib{}.so", sanitized_name);
                #[cfg(target_os = "macos")]
                return format!("lib{}.dylib", sanitized_name);
                #[cfg(windows)]
                return format!("{}.dll", sanitized_name);
            }
            LibraryType::Object => {
                #[cfg(windows)]
                return format!("{}.obj", sanitized_name);
                #[cfg(not(windows))]
                return format!("{}.o", sanitized_name);
            }
        }
    }
}
```

### Build Directory Management

```rust
#[derive(Debug, Clone)]
pub struct BuildDirectories {
    pub project_root: PathBuf,
    pub target_dir: PathBuf,        // project_root/target
    pub profile_dir: PathBuf,       // target/debug or target/release
    pub deps_dir: PathBuf,          // profile/deps
    pub build_dir: PathBuf,         // profile/build
    pub external_dir: PathBuf,      // profile/deps/external
    pub cache_dir: PathBuf,         // target/cache
    pub global_cache: PathBuf,      // ~/.asthra/cache
}

impl BuildDirectories {
    pub fn new(project_root: PathBuf, profile: BuildProfile) -> Result<Self, BuildError> {
        let target_dir = project_root.join("target");
        let profile_name = match profile {
            BuildProfile::Debug => "debug",
            BuildProfile::Release => "release",
            BuildProfile::Test => "test",
        };
        let profile_dir = target_dir.join(profile_name);
        
        Ok(Self {
            project_root: project_root.clone(),
            target_dir,
            profile_dir: profile_dir.clone(),
            deps_dir: profile_dir.join("deps"),
            build_dir: profile_dir.join("build"),
            external_dir: profile_dir.join("deps").join("external"),
            cache_dir: target_dir.join("cache"),
            global_cache: Self::get_global_cache_dir()?,
        })
    }

    pub async fn ensure_directories_exist(&self) -> Result<(), BuildError> {
        for dir in [&self.target_dir, &self.profile_dir, &self.deps_dir, 
                   &self.build_dir, &self.external_dir, &self.cache_dir] {
            tokio::fs::create_dir_all(dir).await?;
        }
        Ok(())
    }

    pub fn get_library_path(&self, artifact: &LibraryArtifact) -> PathBuf {
        match artifact.library_type {
            LibraryType::Static | LibraryType::Dynamic => {
                if artifact.name.starts_with("external_") {
                    self.external_dir.join(artifact.generate_filename())
                } else {
                    self.deps_dir.join(artifact.generate_filename())
                }
            }
            LibraryType::Object => {
                self.build_dir.join(&artifact.name).join(artifact.generate_filename())
            }
        }
    }
}
```

### Library Cache Management

```rust
#[derive(Debug, Serialize, Deserialize)]
pub struct LibraryCache {
    pub libraries: HashMap<String, CachedLibrary>,
    pub build_fingerprints: HashMap<String, String>,
    pub last_cleanup: SystemTime,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct CachedLibrary {
    pub artifact: LibraryArtifact,
    pub source_hash: String,        // Hash of source files
    pub dependency_hash: String,    // Hash of dependencies
    pub build_time: SystemTime,
    pub access_time: SystemTime,    // For LRU cleanup
}

impl LibraryCache {
    pub async fn is_up_to_date(&self, package: &PackageInfo) -> Result<bool, BuildError> {
        if let Some(cached) = self.libraries.get(&package.name) {
            let current_hash = self.calculate_source_hash(package).await?;
            let current_dep_hash = self.calculate_dependency_hash(package).await?;
            
            Ok(cached.source_hash == current_hash && 
               cached.dependency_hash == current_dep_hash)
        } else {
            Ok(false)
        }
    }

    pub async fn cleanup_old_artifacts(&mut self, max_age_days: u64) -> Result<(), BuildError> {
        let cutoff = SystemTime::now() - Duration::from_secs(max_age_days * 24 * 3600);
        let mut to_remove = Vec::new();
        
        for (name, cached) in &self.libraries {
            if cached.access_time < cutoff {
                // Remove file
                if cached.artifact.file_path.exists() {
                    tokio::fs::remove_file(&cached.artifact.file_path).await?;
                }
                to_remove.push(name.clone());
            }
        }
        
        for name in to_remove {
            self.libraries.remove(&name);
        }
        
        self.last_cleanup = SystemTime::now();
        Ok(())
    }
}
```

### Integration with Build Pipeline

The library storage strategy integrates with the existing build pipeline by updating the `BuildConfig` structure:

```rust
#[derive(Debug, Clone)]
pub struct BuildConfig {
    pub target: Target,
    pub optimization: OptimizationLevel,
    pub debug_info: bool,
    pub output_dir: PathBuf,
    pub parallel_jobs: usize,
    pub asthra_compiler_path: PathBuf,
    
    // New library storage configuration
    pub build_directories: BuildDirectories,
    pub library_type: LibraryType,          // Default library type (static)
    pub external_link_preferences: HashMap<String, LibraryType>, // Per-dependency overrides
    pub cache_max_size_mb: u64,             // Cache size limit
    pub cache_max_age_days: u64,            // Cache cleanup threshold
}
```

### Cross-Platform Considerations

```rust
impl BuildDirectories {
    #[cfg(windows)]
    fn get_global_cache_dir() -> Result<PathBuf, BuildError> {
        let appdata = std::env::var("APPDATA")
            .map_err(|_| BuildError::MissingEnvironmentVariable("APPDATA".to_string()))?;
        Ok(PathBuf::from(appdata).join("asthra"))
    }

    #[cfg(unix)]
    fn get_global_cache_dir() -> Result<PathBuf, BuildError> {
        let home = std::env::var("HOME")
            .map_err(|_| BuildError::MissingEnvironmentVariable("HOME".to_string()))?;
        Ok(PathBuf::from(home).join(".asthra"))
    }
}
```

### Cleanup and Maintenance

```rust
impl BuildOrchestrator {
    pub async fn clean_build_artifacts(&self, clean_type: CleanType) -> Result<(), BuildError> {
        match clean_type {
            CleanType::All => {
                // Remove entire target directory
                if self.config.build_directories.target_dir.exists() {
                    tokio::fs::remove_dir_all(&self.config.build_directories.target_dir).await?;
                }
            }
            CleanType::Profile(profile) => {
                // Remove specific profile directory
                let profile_dir = self.config.build_directories.target_dir.join(profile.to_string());
                if profile_dir.exists() {
                    tokio::fs::remove_dir_all(&profile_dir).await?;
                }
            }
            CleanType::Dependencies => {
                // Remove only dependency libraries
                if self.config.build_directories.deps_dir.exists() {
                    tokio::fs::remove_dir_all(&self.config.build_directories.deps_dir).await?;
                }
            }
            CleanType::Cache => {
                // Clean build cache but preserve global package cache
                if self.config.build_directories.cache_dir.exists() {
                    tokio::fs::remove_dir_all(&self.config.build_directories.cache_dir).await?;
                }
            }
        }
        Ok(())
    }
}

#[derive(Debug, Clone)]
pub enum CleanType {
    All,                    // Clean everything
    Profile(String),        // Clean specific profile (debug/release)
    Dependencies,           // Clean only dependencies
    Cache,                  // Clean build cache
}
```

This storage strategy ensures:
- **Efficient incremental builds** through proper caching
- **Cross-platform compatibility** with platform-specific file extensions
- **Clear separation** between Asthra libraries and external C libraries
- **Easy cleanup and maintenance** of build artifacts
- **Scalable storage** that works for both small and large projects

## Implementation Phases

### Phase 1: Rust Project Setup and CLI Interface

#### 1.1 Project Structure and Dependencies

**Setup Instructions:**
```bash
# Create new Rust project
cargo new ampu --bin
cd ampu

# Add dependencies to Cargo.toml
[dependencies]
clap = { version = "4.0", features = ["derive"] }
tokio = { version = "1.0", features = ["full"] }
serde = { version = "1.0", features = ["derive"] }
toml = "0.8"
reqwest = { version = "0.11", features = ["json"] }
git2 = "0.18"
semver = "1.0"
anyhow = "1.0"
thiserror = "1.0"
tracing = "0.1"
tracing-subscriber = "0.3"
walkdir = "2.0"
```

**Project Structure:**
```
ampu/
├── Cargo.toml
├── src/
│   ├── main.rs           # CLI entry point
│   ├── lib.rs            # Library exports
│   ├── cli/              # CLI command handlers
│   │   ├── mod.rs
│   │   ├── build.rs
│   │   ├── run.rs
│   │   ├── test.rs
│   │   └── mod_commands.rs
│   ├── resolver/         # Import resolution
│   │   ├── mod.rs
│   │   ├── import_parser.rs
│   │   └── access_control.rs
│   ├── compiler/         # Compilation orchestration
│   │   ├── mod.rs
│   │   └── asthra_compiler.rs
│   ├── downloader/       # Package downloading
│   │   ├── mod.rs
│   │   ├── git_downloader.rs
│   │   └── cache.rs
│   ├── config/           # Configuration management
│   │   ├── mod.rs
│   │   ├── asthra_toml.rs
│   │   └── workspace.rs
│   └── error.rs          # Error types
├── tests/                # Integration tests
└── README.md
```

**AI Prompt for Rust Implementation:**
```
Create a Rust-based CLI tool called "ampu" for the Asthra build system with the following requirements:

1. CLI structure using clap with derive macros:
   - `ampu build` - Build current package
   - `ampu run` - Build and run current package  
   - `ampu test` - Run tests
   - `ampu init` - Initialize new module
   - `ampu add <package>` - Add dependency
   - `ampu clean` - Clean build artifacts (with options for selective cleaning)

2. Core data structures using serde for serialization:
   ```rust
   #[derive(Debug, Clone, Serialize, Deserialize)]
   pub struct Package {
       pub name: String,
       pub version: semver::Version,
       pub dependencies: HashMap<String, semver::VersionReq>,
       pub package_type: PackageType,
   }

   #[derive(Debug, Clone, Serialize, Deserialize)]
   pub struct AsthraMod {
       pub package: PackageInfo,
       pub dependencies: HashMap<String, semver::VersionReq>,
       pub dev_dependencies: HashMap<String, semver::VersionReq>,
       pub build: BuildConfig,
   }

   #[derive(Debug)]
   pub struct ImportGraph {
       nodes: HashMap<String, PackageNode>,
       edges: Vec<DependencyEdge>,
   }
   ```

3. Error handling using thiserror and anyhow:
   ```rust
   #[derive(thiserror::Error, Debug)]
   pub enum BuildError {
       #[error("Import access violation: {message}")]
       AccessViolation { message: String },
       #[error("Dependency resolution failed: {0}")]
       DependencyResolution(#[from] semver::Error),
       #[error("Compiler error: {0}")]
       Compiler(String),
   }
   ```

4. Async support for network operations using tokio

Please implement the basic CLI structure, core data types, and error handling for the "ampu" build tool.
```

#### 1.2 Module System (asthra.toml)

**AI Prompt for Rust Implementation:**
```
Implement the Asthra module system using Rust and serde for TOML parsing:

1. `asthra.toml` file format (same as before but with Rust parsing):
   ```toml
   [package]
   name = "github.com/user/myproject"
   version = "1.0.0"
   asthra_version = "1.0"

   [dependencies]
   "github.com/asthra-lang/json" = "1.2.0"
   "github.com/asthra-lang/http" = "2.1.0"

   [dev-dependencies]
   "github.com/asthra-lang/test" = "1.0.0"

   [build]
   target = "native"
   optimization = "release"
   ```

2. Rust structures with serde:
   ```rust
   #[derive(Debug, Clone, Serialize, Deserialize)]
   pub struct AsthraMod {
       pub package: PackageConfig,
       #[serde(default)]
       pub dependencies: HashMap<String, String>,
       #[serde(default, rename = "dev-dependencies")]
       pub dev_dependencies: HashMap<String, String>,
       #[serde(default)]
       pub build: BuildConfig,
       #[serde(default)]
       pub workspace: Option<WorkspaceConfig>,
   }

   #[derive(Debug, Clone, Serialize, Deserialize)]
   pub struct PackageConfig {
       pub name: String,
       pub version: String,
       pub asthra_version: String,
   }
   ```

3. Module resolution using semver crate:
   ```rust
   pub struct ModuleResolver {
       cache_dir: PathBuf,
   }

   impl ModuleResolver {
       pub async fn resolve_dependencies(
           &self,
           root_mod: &AsthraMod,
       ) -> Result<ResolvedDependencies, BuildError> {
           // Implementation using semver for version resolution
       }
   }
   ```

4. Lock file generation (asthra.lock) using serde_json

Implement the module parsing, validation, and basic resolution logic using Rust.
```

### Phase 2: Import Resolution and Access Control

#### 2.1 Import Parser and Classifier

**AI Prompt for Rust Implementation:**
```
Create an import resolution system in Rust that can parse and classify Asthra imports:

1. Import types to handle using Rust enums:
   ```rust
   #[derive(Debug, Clone, PartialEq, Eq)]
   pub enum ImportType {
       Stdlib(String),      // "stdlib/string", "stdlib/asthra/core"
       Internal(String),    // "internal/runtime/memory_allocator"
       ThirdParty(String),  // "github.com/user/package"
       Local(String),       // "./utils", "../common"
   }

   #[derive(Debug, Clone, PartialEq, Eq)]
   pub enum PackageType {
       UserCode,
       Stdlib,
       Internal,
       ThirdParty,
   }
   ```

2. Import parser using regex and string parsing:
   ```rust
   use regex::Regex;
   use std::path::Path;

   pub struct ImportParser {
       stdlib_pattern: Regex,
       internal_pattern: Regex,
       third_party_pattern: Regex,
   }

   impl ImportParser {
       pub fn new() -> Result<Self, BuildError> {
           Ok(Self {
               stdlib_pattern: Regex::new(r"^stdlib/")?,
               internal_pattern: Regex::new(r"^internal/")?,
               third_party_pattern: Regex::new(r"^[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}/")?,
           })
       }

       pub fn parse_imports_from_file(&self, file_path: &Path) -> Result<Vec<ImportType>, BuildError> {
           // Read file and extract import statements
           // Parse each import and classify it
       }

       pub fn classify_import(&self, import_path: &str) -> ImportType {
           if self.stdlib_pattern.is_match(import_path) {
               ImportType::Stdlib(import_path.to_string())
           } else if self.internal_pattern.is_match(import_path) {
               ImportType::Internal(import_path.to_string())
           } else if self.third_party_pattern.is_match(import_path) {
               ImportType::ThirdParty(import_path.to_string())
           } else {
               ImportType::Local(import_path.to_string())
           }
       }
   }
   ```

3. Import graph construction using petgraph:
   ```rust
   use petgraph::{Graph, Directed};
   use petgraph::graph::NodeIndex;

   pub struct ImportGraph {
       graph: Graph<String, (), Directed>,
       package_indices: HashMap<String, NodeIndex>,
   }

   impl ImportGraph {
       pub fn build_from_imports(&mut self, imports: Vec<ImportType>) -> Result<(), BuildError> {
           // Build dependency graph
           // Detect circular dependencies using petgraph algorithms
       }

       pub fn topological_sort(&self) -> Result<Vec<String>, BuildError> {
           // Return packages in build order
       }
   }
   ```

4. Package type detection:
   ```rust
   pub fn detect_package_type(package_path: &Path) -> PackageType {
       let path_str = package_path.to_string_lossy();
       
       if path_str.contains("/stdlib/") {
           PackageType::Stdlib
       } else if path_str.contains("/internal/") {
           PackageType::Internal
       } else if path_str.contains("/.asthra/cache/") {
           PackageType::ThirdParty
       } else {
           PackageType::UserCode
       }
   }
   ```

Add regex dependency to Cargo.toml and implement the import parsing system.
```

#### 2.2 Access Control Enforcement

**AI Prompt for Rust Implementation:**
```
Implement the access control system in Rust that enforces import restrictions:

1. Access control rules using Rust pattern matching:
   ```rust
   #[derive(thiserror::Error, Debug)]
   pub enum AccessViolation {
       #[error("User code cannot import internal package: {package}")]
       UserToInternal { package: String },
       #[error("Third-party code cannot import internal package: {package}")]
       ThirdPartyToInternal { package: String },
   }

   pub struct AccessController;

   impl AccessController {
       pub fn validate_import(
           importing_package: PackageType,
           imported_package: &ImportType,
       ) -> Result<(), AccessViolation> {
           use PackageType::*;
           use ImportType::*;

           match (importing_package, imported_package) {
               // ✅ Allowed combinations
               (UserCode, Stdlib(_)) => Ok(()),
               (UserCode, ThirdParty(_)) => Ok(()),
               (UserCode, Local(_)) => Ok(()),
               (Stdlib, Stdlib(_)) => Ok(()),
               (Stdlib, Internal(_)) => Ok(()),
               (Internal, Stdlib(_)) => Ok(()),
               (Internal, Internal(_)) => Ok(()),
               (ThirdParty, Stdlib(_)) => Ok(()),
               (ThirdParty, ThirdParty(_)) => Ok(()),
               (ThirdParty, Local(_)) => Ok(()),

               // ❌ Forbidden combinations
               (UserCode, Internal(package)) => {
                   Err(AccessViolation::UserToInternal { 
                       package: package.clone() 
                   })
               }
               (ThirdParty, Internal(package)) => {
                   Err(AccessViolation::ThirdPartyToInternal { 
                       package: package.clone() 
                   })
               }
           }
       }

       pub fn validate_all_imports(
           &self,
           package_imports: &[(PackageType, Vec<ImportType>)],
       ) -> Result<(), Vec<AccessViolation>> {
           let mut violations = Vec::new();

           for (package_type, imports) in package_imports {
               for import in imports {
                   if let Err(violation) = Self::validate_import(*package_type, import) {
                       violations.push(violation);
                   }
               }
           }

           if violations.is_empty() {
               Ok(())
           } else {
               Err(violations)
           }
       }
   }
   ```

2. Integration with build pipeline:
   ```rust
   pub struct BuildValidator {
       access_controller: AccessController,
       import_parser: ImportParser,
   }

   impl BuildValidator {
       pub async fn validate_project(&self, project_root: &Path) -> Result<(), BuildError> {
           // Walk all .asthra files
           // Parse imports from each file
           // Validate access control
           // Return aggregated errors with file locations
       }
   }
   ```

3. Error reporting with file locations:
   ```rust
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
   ```

Implement the access control validation system with comprehensive error handling and file location tracking.
```

### Phase 3: Package Management and Downloading

#### 3.1 Third-Party Package Downloader

**AI Prompt for Rust Implementation:**
```
Create a package downloader for third-party Asthra packages using Rust async/await:

1. Support for multiple Git hosting providers using git2:
   ```rust
   use git2::{Repository, RepositoryInitOptions};
   use reqwest::Client;
   use semver::Version;

   #[derive(Debug, Clone)]
   pub struct PackageSource {
       pub url: String,
       pub provider: GitProvider,
   }

   #[derive(Debug, Clone)]
   pub enum GitProvider {
       GitHub,
       GitLab,
       Bitbucket,
       Custom(String),
   }

   impl PackageSource {
       pub fn from_import_path(import_path: &str) -> Result<Self, BuildError> {
           // Parse "github.com/user/repo" -> Git URL
           let parts: Vec<&str> = import_path.split('/').collect();
           if parts.len() < 3 {
               return Err(BuildError::InvalidImportPath(import_path.to_string()));
           }

           let provider = match parts[0] {
               "github.com" => GitProvider::GitHub,
               "gitlab.com" => GitProvider::GitLab,
               "bitbucket.org" => GitProvider::Bitbucket,
               domain => GitProvider::Custom(domain.to_string()),
           };

           let url = format!("https://{}/{}/{}.git", parts[0], parts[1], parts[2]);
           Ok(PackageSource { url, provider })
       }
   }
   ```

2. Async package downloader with caching:
   ```rust
   use tokio::fs;
   use std::path::PathBuf;

   pub struct PackageDownloader {
       cache_dir: PathBuf,
       client: Client,
   }

   impl PackageDownloader {
       pub fn new(cache_dir: PathBuf) -> Self {
           Self {
               cache_dir,
               client: Client::new(),
           }
       }

       pub async fn download_package(
           &self,
           import_path: &str,
           version_req: &semver::VersionReq,
       ) -> Result<PackageInfo, BuildError> {
           let source = PackageSource::from_import_path(import_path)?;
           let cache_path = self.get_cache_path(import_path);

           // Check if already cached
           if cache_path.exists() {
               if let Ok(cached) = self.load_cached_package(&cache_path).await {
                   if version_req.matches(&cached.version) {
                       return Ok(cached);
                   }
               }
           }

           // Download from Git
           self.clone_or_update_repo(&source, &cache_path).await?;
           
           // Find matching version
           let available_versions = self.get_available_versions(&cache_path).await?;
           let best_version = self.find_best_version(&available_versions, version_req)?;
           
           // Checkout specific version
           self.checkout_version(&cache_path, &best_version).await?;
           
           // Parse package info
           let package_info = self.parse_package_info(&cache_path, best_version).await?;
           
           Ok(package_info)
       }

       async fn clone_or_update_repo(
           &self,
           source: &PackageSource,
           cache_path: &Path,
       ) -> Result<(), BuildError> {
           if cache_path.exists() {
               // Update existing repo
               let repo = Repository::open(cache_path)?;
               let mut remote = repo.find_remote("origin")?;
               remote.fetch(&["refs/heads/*:refs/remotes/origin/*"], None, None)?;
           } else {
               // Clone new repo
               fs::create_dir_all(cache_path.parent().unwrap()).await?;
               Repository::clone(&source.url, cache_path)?;
           }
           Ok(())
       }

       async fn get_available_versions(&self, repo_path: &Path) -> Result<Vec<Version>, BuildError> {
           let repo = Repository::open(repo_path)?;
           let tags = repo.tag_names(None)?;
           
           let mut versions = Vec::new();
           for tag in tags.iter().flatten() {
               if let Ok(version) = Version::parse(tag.trim_start_matches('v')) {
                   versions.push(version);
               }
           }
           
           versions.sort();
           Ok(versions)
       }
   }
   ```

3. Package caching with integrity checks:
   ```rust
   use sha2::{Sha256, Digest};
   use serde::{Serialize, Deserialize};

   #[derive(Debug, Clone, Serialize, Deserialize)]
   pub struct CachedPackage {
       pub import_path: String,
       pub version: Version,
       pub checksum: String,
       pub cached_at: chrono::DateTime<chrono::Utc>,
   }

   impl PackageDownloader {
       async fn verify_package_integrity(&self, package_path: &Path) -> Result<String, BuildError> {
           let mut hasher = Sha256::new();
           let mut entries = Vec::new();
           
           // Collect all .asthra files for hashing
           for entry in walkdir::WalkDir::new(package_path) {
               let entry = entry?;
               if entry.path().extension().map_or(false, |ext| ext == "asthra") {
                   entries.push(entry.path().to_path_buf());
               }
           }
           
           entries.sort();
           
           for file_path in entries {
               let content = fs::read(&file_path).await?;
               hasher.update(&content);
           }
           
           Ok(format!("{:x}", hasher.finalize()))
       }
   }
   ```

Add git2, sha2, chrono, and walkdir dependencies to Cargo.toml and implement the async package downloading system.
```

#### 3.2 Dependency Resolution Algorithm

**AI Prompt for Rust Implementation:**
```
Implement a dependency resolution algorithm using Rust and semver:

1. Minimal Version Selection (MVS) algorithm:
   ```rust
   use semver::{Version, VersionReq};
   use std::collections::{HashMap, HashSet, VecDeque};

   #[derive(Debug, Clone)]
   pub struct DependencyNode {
       pub import_path: String,
       pub version: Version,
       pub dependencies: HashMap<String, VersionReq>,
   }

   pub struct DependencyResolver {
       downloader: PackageDownloader,
       resolved_cache: HashMap<String, DependencyNode>,
   }

   impl DependencyResolver {
       pub async fn resolve_dependencies(
           &mut self,
           root_dependencies: &HashMap<String, VersionReq>,
       ) -> Result<ResolvedDependencies, BuildError> {
           let mut resolved = HashMap::new();
           let mut to_resolve = VecDeque::new();
           
           // Start with root dependencies
           for (import_path, version_req) in root_dependencies {
               to_resolve.push_back((import_path.clone(), version_req.clone(), None));
           }
           
           while let Some((import_path, version_req, dependent)) = to_resolve.pop_front() {
               if resolved.contains_key(&import_path) {
                   // Check version compatibility
                   let existing = &resolved[&import_path];
                   if !version_req.matches(&existing.version) {
                       return Err(BuildError::VersionConflict {
                           package: import_path,
                           required: version_req,
                           existing: existing.version.clone(),
                           dependent,
                       });
                   }
                   continue;
               }
               
               // Download and resolve package
               let package_info = self.downloader.download_package(&import_path, &version_req).await?;
               let node = DependencyNode {
                   import_path: import_path.clone(),
                   version: package_info.version.clone(),
                   dependencies: package_info.dependencies.clone(),
               };
               
               // Add transitive dependencies to resolution queue
               for (dep_path, dep_req) in &node.dependencies {
                   to_resolve.push_back((
                       dep_path.clone(),
                       dep_req.clone(),
                       Some(import_path.clone()),
                   ));
               }
               
               resolved.insert(import_path, node);
           }
           
           Ok(ResolvedDependencies { packages: resolved })
       }

       pub fn detect_cycles(&self, dependencies: &ResolvedDependencies) -> Result<(), BuildError> {
           let mut visited = HashSet::new();
           let mut rec_stack = HashSet::new();
           
           for package_name in dependencies.packages.keys() {
               if !visited.contains(package_name) {
                   self.dfs_cycle_check(
                       package_name,
                       &dependencies.packages,
                       &mut visited,
                       &mut rec_stack,
                   )?;
               }
           }
           
           Ok(())
       }

       fn dfs_cycle_check(
           &self,
           package: &str,
           packages: &HashMap<String, DependencyNode>,
           visited: &mut HashSet<String>,
           rec_stack: &mut HashSet<String>,
       ) -> Result<(), BuildError> {
           visited.insert(package.to_string());
           rec_stack.insert(package.to_string());
           
           if let Some(node) = packages.get(package) {
               for dep in node.dependencies.keys() {
                   if !visited.contains(dep) {
                       self.dfs_cycle_check(dep, packages, visited, rec_stack)?;
                   } else if rec_stack.contains(dep) {
                       return Err(BuildError::CircularDependency {
                           cycle: vec![package.to_string(), dep.to_string()],
                       });
                   }
               }
           }
           
           rec_stack.remove(package);
           Ok(())
       }
   }
   ```

2. Lock file generation:
   ```rust
   #[derive(Debug, Serialize, Deserialize)]
   pub struct LockFile {
       pub version: String,
       pub packages: HashMap<String, LockedPackage>,
       pub generated_at: chrono::DateTime<chrono::Utc>,
   }

   #[derive(Debug, Serialize, Deserialize)]
   pub struct LockedPackage {
       pub version: Version,
       pub checksum: String,
       pub dependencies: HashMap<String, Version>,
   }

   impl LockFile {
       pub fn generate(resolved: &ResolvedDependencies) -> Self {
           let mut packages = HashMap::new();
           
           for (name, node) in &resolved.packages {
               let locked = LockedPackage {
                   version: node.version.clone(),
                   checksum: node.checksum.clone(),
                   dependencies: node.dependencies.iter()
                       .map(|(dep_name, _)| {
                           let dep_version = resolved.packages[dep_name].version.clone();
                           (dep_name.clone(), dep_version)
                       })
                       .collect(),
               };
               packages.insert(name.clone(), locked);
           }
           
           Self {
               version: "1".to_string(),
               packages,
               generated_at: chrono::Utc::now(),
           }
       }

       pub async fn save(&self, path: &Path) -> Result<(), BuildError> {
           let content = toml::to_string_pretty(self)?;
           fs::write(path, content).await?;
           Ok(())
       }

       pub async fn load(path: &Path) -> Result<Self, BuildError> {
           let content = fs::read_to_string(path).await?;
           let lock_file = toml::from_str(&content)?;
           Ok(lock_file)
       }
   }
   ```

Implement the dependency resolution algorithm with proper conflict handling and lock file generation.
```

### Phase 4: Build Pipeline and Compilation

#### 4.1 Build Orchestration

**AI Prompt for Rust Implementation:**
```
Create the build orchestration system in Rust that coordinates compilation by calling the existing C-based `asthra` compiler:

1. Build pipeline stages using async Rust:
   ```rust
   use tokio::process::Command;
   use std::process::Stdio;

   #[derive(Debug, Clone)]
   pub struct BuildConfig {
       pub target: Target,
       pub optimization: OptimizationLevel,
       pub debug_info: bool,
       pub output_dir: PathBuf,
       pub parallel_jobs: usize,
       pub asthra_compiler_path: PathBuf,
   }

   #[derive(Debug, Clone)]
   pub enum Target {
       Native,
       X86_64,
       Arm64,
       Wasm32,
   }

   pub struct BuildOrchestrator {
       config: BuildConfig,
       resolver: DependencyResolver,
       access_controller: AccessController,
   }

   impl BuildOrchestrator {
       pub async fn build_project(&self, project_root: &Path) -> Result<BuildResult, BuildError> {
           // 1. Parse asthra.toml and resolve dependencies
           let asthra_mod = self.load_asthra_toml(project_root).await?;
           let resolved_deps = self.resolver.resolve_dependencies(&asthra_mod.dependencies).await?;
           
           // 2. Validate import access control
           self.validate_access_control(project_root, &resolved_deps).await?;
           
           // 3. Determine compilation order
           let build_order = self.determine_build_order(&resolved_deps)?;
           
           // 4. Compile packages in dependency order
           let mut results = Vec::new();
           for package in build_order {
               let result = self.compile_package(&package).await?;
               results.push(result);
           }
           
           // 5. Link final executable
           let final_output = self.link_executable(&results).await?;
           
           Ok(BuildResult {
               output_path: final_output,
               compiled_packages: results,
           })
       }

       async fn compile_package(&self, package: &PackageInfo) -> Result<CompilationResult, BuildError> {
           let mut cmd = Command::new(&self.config.asthra_compiler_path);
           
           // Add compiler arguments
           cmd.arg(&package.main_file)
               .arg("-o")
               .arg(&package.output_path);
           
           // Add optimization flags
           match self.config.optimization {
               OptimizationLevel::None => cmd.arg("-O0"),
               OptimizationLevel::Basic => cmd.arg("-O1"),
               OptimizationLevel::Standard => cmd.arg("-O2"),
               OptimizationLevel::Aggressive => cmd.arg("-O3"),
           };
           
           // Add debug info if requested
           if self.config.debug_info {
               cmd.arg("-g");
           }
           
           // Add include paths for dependencies
           for dep_path in &package.dependency_paths {
               cmd.arg("-I").arg(dep_path);
           }
           
           // Execute compiler
           let output = cmd
               .stdout(Stdio::piped())
               .stderr(Stdio::piped())
               .output()
               .await?;
           
           if output.status.success() {
               Ok(CompilationResult {
                   package: package.clone(),
                   output_path: package.output_path.clone(),
                   warnings: self.parse_warnings(&output.stderr)?,
               })
           } else {
               Err(BuildError::CompilationFailed {
                   package: package.name.clone(),
                   stdout: String::from_utf8_lossy(&output.stdout).to_string(),
                   stderr: String::from_utf8_lossy(&output.stderr).to_string(),
                   exit_code: output.status.code(),
               })
           }
       }
   }
   ```

2. Parallel compilation with dependency ordering:
   ```rust
   use tokio::sync::Semaphore;
   use std::sync::Arc;

   impl BuildOrchestrator {
       pub async fn build_parallel(&self, packages: Vec<PackageInfo>) -> Result<Vec<CompilationResult>, BuildError> {
           let semaphore = Arc::new(Semaphore::new(self.config.parallel_jobs));
           let mut handles = Vec::new();
           
           for package in packages {
               let sem = semaphore.clone();
               let orchestrator = self.clone();
               
               let handle = tokio::spawn(async move {
                   let _permit = sem.acquire().await.unwrap();
                   orchestrator.compile_package(&package).await
               });
               
               handles.push(handle);
           }
           
           let mut results = Vec::new();
           for handle in handles {
               results.push(handle.await??);
           }
           
           Ok(results)
       }

       fn determine_build_order(&self, dependencies: &ResolvedDependencies) -> Result<Vec<PackageInfo>, BuildError> {
           // Use petgraph for topological sort
           let mut graph = petgraph::Graph::new();
           let mut node_indices = HashMap::new();
           
           // Add nodes
           for (name, _) in &dependencies.packages {
               let index = graph.add_node(name.clone());
               node_indices.insert(name.clone(), index);
           }
           
           // Add edges
           for (name, node) in &dependencies.packages {
               let from_index = node_indices[name];
               for dep_name in node.dependencies.keys() {
                   if let Some(&to_index) = node_indices.get(dep_name) {
                       graph.add_edge(to_index, from_index, ());
                   }
               }
           }
           
           // Topological sort
           match petgraph::algo::toposort(&graph, None) {
               Ok(sorted) => {
                   let build_order = sorted
                       .into_iter()
                       .map(|index| graph[index].clone())
                       .collect();
                   Ok(build_order)
               }
               Err(_) => Err(BuildError::CircularDependency {
                   cycle: vec!["detected".to_string()], // TODO: extract actual cycle
               }),
           }
       }
   }
   ```

3. Incremental compilation support:
   ```rust
   use std::time::SystemTime;
   use serde::{Serialize, Deserialize};

   #[derive(Debug, Serialize, Deserialize)]
   pub struct BuildCache {
       pub file_hashes: HashMap<PathBuf, String>,
       pub last_build: SystemTime,
       pub compiled_artifacts: HashMap<String, PathBuf>,
   }

   impl BuildOrchestrator {
       async fn needs_rebuild(&self, package: &PackageInfo, cache: &BuildCache) -> Result<bool, BuildError> {
           // Check if source files have changed
           for source_file in &package.source_files {
               let current_hash = self.calculate_file_hash(source_file).await?;
               if let Some(cached_hash) = cache.file_hashes.get(source_file) {
                   if current_hash != *cached_hash {
                       return Ok(true);
                   }
               } else {
                   return Ok(true); // New file
               }
           }
           
           // Check if dependencies have changed
           for dep in &package.dependencies {
               if self.dependency_changed(dep, cache).await? {
                   return Ok(true);
               }
           }
           
           Ok(false)
       }

       async fn update_cache(&self, package: &PackageInfo, cache: &mut BuildCache) -> Result<(), BuildError> {
           for source_file in &package.source_files {
               let hash = self.calculate_file_hash(source_file).await?;
               cache.file_hashes.insert(source_file.clone(), hash);
           }
           cache.last_build = SystemTime::now();
           Ok(())
       }
   }
   ```

Add petgraph dependency to Cargo.toml and implement the build orchestration system with proper subprocess management.
```

#### 4.2 Compiler Integration

**AI Prompt for Rust Implementation:**
```
Create the integration layer between the Rust build tool and the C-based Asthra compiler:

1. Compiler invocation with proper argument handling:
   ```rust
   use tokio::process::Command;
   use std::ffi::OsString;

   #[derive(Debug, Clone)]
   pub struct CompilerInvocation {
       pub source_files: Vec<PathBuf>,
       pub import_paths: Vec<PathBuf>,
       pub output_path: PathBuf,
       pub target: Target,
       pub optimization: OptimizationLevel,
       pub additional_args: Vec<OsString>,
   }

   pub struct AsthraCCompilerInterface {
       compiler_path: PathBuf,
   }

   impl AsthraCCompilerInterface {
       pub fn new(compiler_path: PathBuf) -> Self {
           Self { compiler_path }
       }

       pub async fn compile(&self, invocation: CompilerInvocation) -> Result<CompilerOutput, BuildError> {
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
               Target::X86_64 => cmd.arg("--target").arg("x86_64"),
               Target::Arm64 => cmd.arg("--target").arg("arm64"),
               Target::Wasm32 => cmd.arg("--target").arg("wasm32"),
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
           
           // Execute with timeout
           let output = tokio::time::timeout(
               std::time::Duration::from_secs(300), // 5 minute timeout
               cmd.output()
           ).await??;
           
           Ok(CompilerOutput {
               success: output.status.success(),
               exit_code: output.status.code(),
               stdout: String::from_utf8_lossy(&output.stdout).to_string(),
               stderr: String::from_utf8_lossy(&output.stderr).to_string(),
           })
       }
   }
   ```

2. Import path resolution for the C compiler:
   ```rust
   pub struct ImportPathResolver {
       stdlib_root: PathBuf,
       internal_root: PathBuf,
       cache_dir: PathBuf,
   }

   impl ImportPathResolver {
       pub fn resolve_import_to_path(&self, import: &ImportType) -> Result<PathBuf, BuildError> {
           match import {
               ImportType::Stdlib(path) => {
                   // "stdlib/string" -> "/path/to/asthra/stdlib/string"
                   let relative_path = path.strip_prefix("stdlib/").unwrap();
                   Ok(self.stdlib_root.join(relative_path))
               }
               ImportType::Internal(path) => {
                   // "internal/runtime/memory" -> "/path/to/asthra/internal/runtime/memory"
                   let relative_path = path.strip_prefix("internal/").unwrap();
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

       pub fn build_include_paths(&self, imports: &[ImportType]) -> Result<Vec<PathBuf>, BuildError> {
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
   ```

3. Error parsing and reporting:
   ```rust
   use regex::Regex;

   #[derive(Debug, Clone)]
   pub struct CompilerError {
       pub file: PathBuf,
       pub line: usize,
       pub column: usize,
       pub message: String,
       pub error_type: ErrorType,
   }

   #[derive(Debug, Clone)]
   pub enum ErrorType {
       SyntaxError,
       TypeError,
       ImportError,
       Other(String),
   }

   pub struct ErrorParser {
       error_regex: Regex,
   }

   impl ErrorParser {
       pub fn new() -> Result<Self, BuildError> {
           // Parse errors like: "file.asthra:10:5: error: message"
           let error_regex = Regex::new(r"([^:]+):(\d+):(\d+):\s*(error|warning):\s*(.+)")?;
           Ok(Self { error_regex })
       }

       pub fn parse_compiler_output(&self, stderr: &str) -> Vec<CompilerError> {
           let mut errors = Vec::new();
           
           for line in stderr.lines() {
               if let Some(captures) = self.error_regex.captures(line) {
                   let file = PathBuf::from(&captures[1]);
                   let line_num = captures[2].parse().unwrap_or(0);
                   let column = captures[3].parse().unwrap_or(0);
                   let error_type_str = &captures[4];
                   let message = captures[5].to_string();
                   
                   let error_type = match error_type_str {
                       "error" => self.classify_error(&message),
                       "warning" => ErrorType::Other("warning".to_string()),
                       _ => ErrorType::Other(error_type_str.to_string()),
                   };
                   
                   errors.push(CompilerError {
                       file,
                       line: line_num,
                       column,
                       message,
                       error_type,
                   });
               }
           }
           
           errors
       }

       fn classify_error(&self, message: &str) -> ErrorType {
           if message.contains("syntax") || message.contains("parse") {
               ErrorType::SyntaxError
           } else if message.contains("type") {
               ErrorType::TypeError
           } else if message.contains("import") || message.contains("cannot find") {
               ErrorType::ImportError
           } else {
               ErrorType::Other("compile_error".to_string())
           }
       }
   }
   ```

Implement the compiler integration with proper subprocess management, timeout handling, and error parsing.
```

### Phase 5: Advanced Features

#### 5.1 IDE Integration and Language Server

**AI Prompt:**
```
Create IDE integration features for the Asthra build tool:

1. Language Server Protocol (LSP) support:
   - Implement LSP server for Asthra
   - Provide import resolution for autocomplete
   - Real-time access control validation
   - Go-to-definition across packages

2. IDE features to support:
   - Import autocomplete with access control hints
   - Error highlighting for import violations
   - Package information on hover
   - Dependency graph visualization

3. Build tool integration:
   ```asthra
   // LSP server can query build tool for package info
   fn get_package_info(import_path: &str) -> Option<PackageInfo>;
   fn validate_import_access(from: &str, to: &str) -> Result<(), String>;
   fn resolve_import_path(import: &str) -> Option<PathBuf>;
   ```

4. Real-time features:
   - Watch file changes for incremental updates
   - Background dependency resolution
   - Cache invalidation on file changes
   - Fast import validation

5. Editor-specific integrations:
   - VS Code extension
   - Vim/Neovim plugin
   - IntelliJ plugin architecture
   - Emacs integration

Example LSP capabilities:
- Import path completion with access control filtering
- Error diagnostics for invalid imports
- Quick fixes for import issues
- Package documentation on hover

Implement the LSP server with focus on import system features.
```

#### 5.2 Testing and Quality Assurance

**AI Prompt:**
```
Implement testing infrastructure for the Asthra build tool:

1. Test framework integration:
   - `asthra test` command
   - Test discovery and execution
   - Test result reporting
   - Coverage analysis

2. Build tool testing:
   - Unit tests for all components
   - Integration tests with real packages
   - End-to-end build scenarios
   - Performance benchmarks

3. Test scenarios to cover:
   - Import resolution edge cases
   - Access control enforcement
   - Dependency resolution conflicts
   - Network failure handling
   - Concurrent build scenarios

4. Quality assurance tools:
   - Static analysis integration
   - Linting for Asthra code
   - Security vulnerability scanning
   - Performance profiling

5. Continuous integration:
   - GitHub Actions workflows
   - Multi-platform testing
   - Regression test suite
   - Performance monitoring

Example test structure:
```asthra
#[tokio::test]
async fn test_import_access_control() {
    let build_tool = BuildTool::new();
    let result = build_tool.validate_imports("user_code", vec!["internal/runtime"]);
    assert!(result.is_err());
    assert!(result.unwrap_err().to_string().contains("Cannot import internal package"));
}
```

Implement comprehensive testing infrastructure for the build tool.
```

### Phase 6: Documentation and Tooling

#### 6.1 Documentation Generation

**AI Prompt:**
```
Create documentation generation tools for Asthra packages:

1. Documentation extraction:
   - Parse doc comments from .asthra files
   - Extract package, function, and type documentation
   - Support for markdown in doc comments
   - Cross-reference resolution

2. Documentation formats:
   - HTML documentation (like rustdoc)
   - Markdown output for GitHub/GitLab
   - JSON API for tooling integration
   - PDF generation for offline use

3. Package documentation features:
   - Import examples with access control notes
   - API reference with type signatures
   - Usage examples and tutorials
   - Dependency information

4. Documentation hosting:
   - Integration with package repositories
   - Automatic documentation updates
   - Search functionality
   - Version-specific documentation

5. `asthra doc` command:
   ```bash
   asthra doc                    # Generate docs for current package
   asthra doc --open            # Generate and open in browser
   asthra doc --format=markdown # Output as markdown
   asthra doc --serve           # Start local doc server
   ```

Example documentation structure:
```
docs/
├── index.html
├── stdlib/
│   ├── string/
│   └── asthra/
├── internal/          # Only visible to stdlib docs
└── third-party/
```

Implement the documentation generation system with proper formatting and cross-references.
```

#### 6.2 Development Tools and Utilities

**AI Prompt:**
```
Create additional development tools for the Asthra ecosystem:

1. Package management utilities:
   - `ampu list` - List installed packages
   - `ampu clean` - Clean build cache
   - `ampu clean --all` - Clean all build artifacts including libraries
   - `ampu clean --profile debug` - Clean specific build profile
   - `ampu clean --deps` - Clean only dependency libraries
   - `ampu clean --cache` - Clean build cache but preserve libraries
   - `ampu update` - Update dependencies
   - `ampu vendor` - Vendor dependencies locally

2. Analysis and debugging tools:
   - `ampu deps` - Show dependency tree
   - `ampu graph` - Visualize dependency graph
   - `ampu why` - Explain why package is included
   - `ampu check` - Validate without building

3. Migration and upgrade tools:
   - Automatic import path updates
   - Version migration assistance
   - Breaking change detection
   - Compatibility checking

4. Performance and profiling tools:
   - Build time analysis
   - Dependency resolution profiling
   - Cache hit rate monitoring
   - Parallel compilation metrics

5. Integration tools:
   - CI/CD pipeline templates
   - Docker container support
   - Package publishing tools
   - Release automation

Example commands:
```bash
ampu deps --tree                    # Show dependency tree
ampu graph --format=dot > deps.dot  # Generate GraphViz
ampu why github.com/user/package    # Explain inclusion
ampu check --access-control         # Validate imports only
```

Implement the development tools with focus on usability and integration.
```

## Implementation Timeline

### Week 1-2: Rust Foundation
- Set up Cargo project with dependencies (clap, tokio, serde, etc.)
- Implement CLI structure and basic commands
- Create core data structures (Package, AsthraMod, ImportGraph)
- Set up error handling with thiserror and anyhow

### Week 3-4: Module System and Import Resolution
- Implement asthra.toml parsing with serde
- Create import parsing and classification system
- Build access control enforcement with pattern matching
- Add regex-based import path parsing

### Week 5-6: Package Management
- Implement Git-based package downloader with git2
- Create async dependency resolution algorithm
- Add package caching with integrity checks
- Implement lock file generation

### Week 7-8: Build Pipeline
- Create build orchestration that calls C compiler
- Implement parallel compilation with tokio
- Add incremental compilation support
- Create compiler integration layer

### Week 9-10: Advanced Features and Polish
- Add comprehensive error parsing and reporting
- Implement progress reporting and logging
- Create integration tests
- Performance optimization

### Week 11-12: Documentation and Distribution
- Write comprehensive documentation
- Create installation scripts
- Set up CI/CD for releases
- Package for distribution (brew, apt, etc.)

## Success Criteria

1. **Functional Requirements**:
   - ✅ Correctly resolve stdlib, internal, and third-party imports
   - ✅ Enforce access control (user code cannot import internal packages)
   - ✅ Download and cache third-party packages from Git repositories
   - ✅ Build packages in correct dependency order by calling C compiler
   - ✅ Generate reproducible builds with lock files

2. **Performance Requirements**:
   - ✅ Fast incremental compilation with file change detection
   - ✅ Efficient dependency resolution using semver
   - ✅ Parallel compilation where dependencies allow
   - ✅ Reasonable memory usage for large projects

3. **Integration Requirements**:
   - ✅ Seamless integration with existing C-based Asthra compiler
   - ✅ Proper subprocess management and error handling
   - ✅ Cross-platform support (Windows, macOS, Linux)
   - ✅ Easy installation alongside Asthra compiler

4. **Usability Requirements**:
   - ✅ Clear error messages for import violations with file locations
   - ✅ Intuitive CLI similar to Cargo
   - ✅ Comprehensive documentation and examples
   - ✅ Easy migration path for existing projects

5. **Reliability Requirements**:
   - ✅ Robust error handling for network failures
   - ✅ Graceful handling of compiler subprocess failures
   - ✅ Reproducible builds across environments
   - ✅ Comprehensive test coverage

## Getting Started

To begin implementation:

1. **Set up Rust project**:
   ```bash
   cargo new ampu --bin
   cd ampu
   # Add dependencies to Cargo.toml as specified in Phase 1.1
   ```

2. **Start with Phase 1.1** (CLI structure) using the provided Rust prompts
3. **Implement core data structures** using serde for serialization
4. **Add module system support** (asthra.toml parsing)
5. **Build import resolution** with regex and access control
6. **Test integration** with existing C compiler early and often

## Integration with Existing Asthra Project

The build tool will integrate with your existing project structure:

```
asthra/                          # Your existing project
├── src/                         # C compiler source
├── runtime/                     # Runtime library
├── stdlib/                      # Standard library packages
├── internal/                    # Internal packages
├── tools/
│   └── ampu/                    # New Rust build tool
│       ├── Cargo.toml
│       ├── src/
│       └── target/
└── bin/
    ├── asthra                   # C compiler binary
    └── ampu                     # Rust build tool binary
```

**Distribution Strategy**:
- Bundle `ampu` with Asthra releases
- Users get both binaries when installing Asthra
- `ampu` calls `asthra` compiler as needed
- Maintain separate versioning but coordinate releases

Each AI prompt is designed to be self-contained and can be used with any AI coding assistant to implement the specific component. The prompts include detailed Rust code examples, proper async patterns, and integration with the existing C compiler. 
