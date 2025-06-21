pub mod asthra_compiler;

use crate::config::{OptimizationLevel, PackageInfo, Target};
use crate::error::{BuildError, Result};
use crate::resolver::{access_control::AccessController, ResolvedDependencies};
use crate::downloader::DependencyResolver;
use crate::storage::{BuildDirectories, BuildProfile, LibraryArtifact, LibraryType, LibraryCache, CleanType};
use std::path::PathBuf;
use std::collections::HashMap;
use std::time::SystemTime;
use tokio::sync::Semaphore;
use std::sync::Arc;
use tracing::{debug, info};
use std::ffi::OsString;

#[derive(Debug, Clone)]
pub struct BuildConfig {
    pub target: Target,
    pub optimization: OptimizationLevel,
    pub debug_info: bool,
    pub output_dir: PathBuf,
    pub parallel_jobs: usize,
    pub asthra_compiler_path: PathBuf,
    
    // Enhanced with library storage configuration
    pub build_directories: BuildDirectories,
    pub library_type: LibraryType,
    pub external_link_preferences: HashMap<String, LibraryType>,
    pub cache_max_size_mb: u64,
    pub cache_max_age_days: u64,
}

impl BuildConfig {
    pub fn new(project_root: PathBuf, profile: BuildProfile) -> Result<Self> {
        let build_directories = BuildDirectories::new(project_root.clone(), profile)?;
        
        Ok(Self {
            target: Target::Native,
            optimization: OptimizationLevel::Standard,
            debug_info: true,
            output_dir: build_directories.profile_dir.clone(),
            parallel_jobs: num_cpus::get(),
            asthra_compiler_path: PathBuf::from("asthra"), // Default compiler path
            build_directories,
            library_type: LibraryType::Static,
            external_link_preferences: HashMap::new(),
            cache_max_size_mb: 1024, // 1GB default
            cache_max_age_days: 30,  // 30 days default
        })
    }
}

#[derive(Debug)]
pub struct BuildResult {
    pub output_path: PathBuf,
    pub compiled_packages: Vec<CompilationResult>,
    pub build_time: std::time::Duration,
    pub cache_hits: usize,
    pub cache_misses: usize,
}

#[derive(Debug, Clone)]
pub struct CompilationResult {
    pub package: PackageInfo,
    pub output_path: PathBuf,
    pub warnings: Vec<String>,
    pub library_artifact: Option<LibraryArtifact>,
    pub compilation_time: std::time::Duration,
}

pub struct BuildOrchestrator {
    config: BuildConfig,
    resolver: DependencyResolver,
    access_controller: AccessController,
    compiler_interface: asthra_compiler::AsthraCCompilerInterface,
    library_cache: LibraryCache,
}

impl BuildOrchestrator {
    pub fn new(config: BuildConfig) -> Self {
        let compiler_interface = asthra_compiler::AsthraCCompilerInterface::new(
            config.asthra_compiler_path.clone()
        );
        
        Self {
            resolver: DependencyResolver::new(),
            access_controller: AccessController,
            compiler_interface,
            library_cache: LibraryCache::new(),
            config,
        }
    }

    pub async fn build_project(&mut self, project_root: &std::path::Path) -> Result<BuildResult> {
        let start_time = SystemTime::now();
        info!("Starting build orchestration for project at {}", project_root.display());
        
        // Ensure build directories exist
        self.config.build_directories.ensure_directories_exist().await?;
        
        // 1. Parse asthra.toml and resolve dependencies
        let asthra_mod = self.load_asthra_toml(project_root).await?;
        info!("Loaded project configuration: {}", asthra_mod.package.name);
        
        // Create a mutable resolver for dependency resolution
        let mut resolver = DependencyResolver::new();
        let resolved_deps = resolver.resolve_dependencies(&asthra_mod.dependencies).await?;
        info!("Resolved {} dependencies", resolved_deps.packages.len());
        
        // 2. Validate import access control
        self.validate_access_control(project_root, &resolved_deps).await?;
        info!("✅ Access control validation passed");
        
        // 3. Determine compilation order
        let build_order = self.determine_build_order(&resolved_deps)?;
        info!("Determined build order for {} packages", build_order.len());
        
        // 4. Check for incremental compilation opportunities
        let (packages_to_build, cache_hits) = self.filter_packages_for_incremental_build(&build_order).await?;
        info!("Building {} packages (incremental: {} cache hits)", 
              packages_to_build.len(), cache_hits);
        
        // 5. Compile packages in parallel where possible
        let mut results = Vec::new();
        let packages_count = packages_to_build.len();
        
        if self.config.parallel_jobs > 1 && packages_to_build.len() > 1 {
            // Parallel compilation where dependencies allow
            results = self.compile_packages_parallel(&packages_to_build).await?;
        } else {
            // Sequential compilation
            for package in packages_to_build {
                let result = self.compile_package(&package).await?;
                results.push(result);
            }
        }
        
        // 6. Update build cache
        self.update_build_cache(&results).await?;
        
        // 7. Link final executable
        let final_output = self.link_executable(&results).await?;
        
        let build_time = start_time.elapsed().unwrap_or_default();
        info!("Build completed in {:.2}s", build_time.as_secs_f64());
        
        Ok(BuildResult {
            output_path: final_output,
            compiled_packages: results,
            build_time,
            cache_hits,
            cache_misses: packages_count,
        })
    }

    async fn filter_packages_for_incremental_build(&mut self, packages: &[PackageInfo]) -> Result<(Vec<PackageInfo>, usize)> {
        let mut packages_to_build = Vec::new();
        let mut cache_hits = 0;
        
        for package in packages {
            if !self.library_cache.is_up_to_date(&package.name).await? {
                debug!("Package {} needs rebuild", package.name);
                packages_to_build.push(package.clone());
            } else {
                debug!("Package {} is up to date, skipping", package.name);
                cache_hits += 1;
            }
        }
        
        Ok((packages_to_build, cache_hits))
    }

    async fn update_build_cache(&mut self, results: &[CompilationResult]) -> Result<()> {
        for result in results {
            // Update cache with new build information
            debug!("Updating cache for package: {}", result.package.name);
            // TODO: Implement actual cache update with file hashes
        }
        
        // Cleanup old artifacts if needed
        if self.library_cache.last_cleanup.elapsed().unwrap_or_default().as_secs() > 86400 {
            self.library_cache.cleanup_old_artifacts(self.config.cache_max_age_days).await?;
        }
        
        Ok(())
    }

    // Clean build artifacts with advanced options
    pub async fn clean_build_artifacts(&self, clean_type: CleanType) -> Result<()> {
        match clean_type {
            CleanType::All => {
                if self.config.build_directories.target_dir.exists() {
                    tokio::fs::remove_dir_all(&self.config.build_directories.target_dir).await?;
                    info!("Cleaned all build artifacts");
                }
            }
            CleanType::Profile(profile) => {
                info!("Cleaning {} profile", profile);
                let profile_dir = self.config.build_directories.target_dir.join(&profile);
                if profile_dir.exists() {
                    tokio::fs::remove_dir_all(&profile_dir).await?;
                    info!("Cleaned {} profile", profile);
                }
            }
            CleanType::Dependencies => {
                if self.config.build_directories.deps_dir.exists() {
                    tokio::fs::remove_dir_all(&self.config.build_directories.deps_dir).await?;
                    info!("Cleaned dependency libraries");
                }
            }
            CleanType::Cache => {
                if self.config.build_directories.cache_dir.exists() {
                    tokio::fs::remove_dir_all(&self.config.build_directories.cache_dir).await?;
                    info!("Cleaned build cache");
                }
            }
        }
        Ok(())
    }

    async fn load_asthra_toml(&self, project_root: &std::path::Path) -> Result<crate::config::AsthraMod> {
        let asthra_toml_path = project_root.join("asthra.toml");
        crate::config::asthra_toml::AsthraTOMLParser::parse_file(&asthra_toml_path).await
    }

    async fn validate_access_control(
        &self,
        project_root: &std::path::Path,
        _resolved_deps: &ResolvedDependencies,
    ) -> Result<()> {
        let validator = crate::resolver::access_control::BuildValidator::new()?;
        validator.validate_project(project_root).await
    }

    fn determine_build_order(&self, dependencies: &ResolvedDependencies) -> Result<Vec<PackageInfo>> {
        // Convert resolved dependencies to PackageInfo for compilation
        let mut packages = Vec::new();
        
        for (name, node) in &dependencies.packages {
            // Discover source files for this package
            let source_files = self.discover_source_files_for_package(name)?;
            
            // Determine main file (entry point)
            let main_file = self.determine_main_file_for_package(name, &source_files)?;
            
            // Calculate package checksum for incremental builds
            let checksum = self.calculate_package_checksum(name, &source_files)?;
            
            let package_info = PackageInfo {
                name: name.clone(),
                version: node.version.clone(),
                dependencies: node.dependencies.clone(),
                source_files,
                main_file,
                output_path: self.config.output_dir.join(format!("{}.o", name.replace('/', "_"))),
                checksum,
            };
            packages.push(package_info);
        }
        
        // Implement topological sort based on dependencies
        self.topological_sort_packages(packages)
    }
    
    fn discover_source_files_for_package(&self, package_name: &str) -> Result<Vec<PathBuf>> {
        let mut source_files = Vec::new();
        
        // Determine package path based on package type
        let package_path = if package_name.starts_with("stdlib/") {
            // Standard library packages
            self.config.build_directories.target_dir
                .parent()
                .unwrap_or(&self.config.build_directories.target_dir)
                .join("stdlib")
                .join(package_name.strip_prefix("stdlib/").unwrap())
        } else if package_name.starts_with("internal/") {
            // Internal packages (should not be accessible to user code)
            return Err(BuildError::AccessViolation {
                message: format!("User code cannot access internal package: {}", package_name),
            });
        } else if package_name.contains("github.com/") || package_name.contains("gitlab.com/") {
            // Third-party packages from cache
            self.config.build_directories.cache_dir.join(package_name)
        } else {
            // Local packages
            self.config.build_directories.target_dir
                .parent()
                .unwrap_or(&self.config.build_directories.target_dir)
                .join("src")
        };
        
        // Recursively find all .asthra files
        if package_path.exists() {
            self.find_asthra_files_recursive(&package_path, &mut source_files)?;
        }
        
        Ok(source_files)
    }
    
    fn find_asthra_files_recursive(&self, dir: &PathBuf, files: &mut Vec<PathBuf>) -> Result<()> {
        if dir.is_dir() {
            for entry in std::fs::read_dir(dir)? {
                let entry = entry?;
                let path = entry.path();
                
                if path.is_dir() {
                    self.find_asthra_files_recursive(&path, files)?;
                } else if path.extension().map_or(false, |ext| ext == "asthra") {
                    files.push(path);
                }
            }
        }
        Ok(())
    }
    
    fn determine_main_file_for_package(&self, package_name: &str, source_files: &[PathBuf]) -> Result<PathBuf> {
        // Look for main.asthra or lib.asthra
        for file in source_files {
            if let Some(file_name) = file.file_name().and_then(|n| n.to_str()) {
                if file_name == "main.asthra" || file_name == "lib.asthra" {
                    return Ok(file.clone());
                }
            }
        }
        
        // If no main file found, use the first source file
        if let Some(first_file) = source_files.first() {
            Ok(first_file.clone())
        } else {
            Err(BuildError::PackageNotFound(format!(
                "No source files found for package: {}", package_name
            )))
        }
    }
    
    fn calculate_package_checksum(&self, _package_name: &str, source_files: &[PathBuf]) -> Result<String> {
        use std::collections::hash_map::DefaultHasher;
        use std::hash::{Hash, Hasher};
        
        let mut hasher = DefaultHasher::new();
        
        // Hash all source file contents and modification times
        for file in source_files {
            if let Ok(content) = std::fs::read_to_string(file) {
                content.hash(&mut hasher);
            }
            
            if let Ok(metadata) = std::fs::metadata(file) {
                if let Ok(modified) = metadata.modified() {
                    if let Ok(duration) = modified.duration_since(std::time::UNIX_EPOCH) {
                        duration.as_secs().hash(&mut hasher);
                    }
                }
            }
        }
        
        Ok(format!("{:x}", hasher.finish()))
    }
    
    fn topological_sort_packages(&self, packages: Vec<PackageInfo>) -> Result<Vec<PackageInfo>> {
        use std::collections::{HashMap, VecDeque};
        
        // Build dependency graph
        let mut graph: HashMap<String, Vec<String>> = HashMap::new();
        let mut in_degree: HashMap<String, usize> = HashMap::new();
        let package_map: HashMap<String, PackageInfo> = packages
            .into_iter()
            .map(|p| (p.name.clone(), p))
            .collect();
        
        // Initialize graph
        for package in package_map.values() {
            graph.insert(package.name.clone(), Vec::new());
            in_degree.insert(package.name.clone(), 0);
        }
        
        // Build edges
        for package in package_map.values() {
            for dep_name in package.dependencies.keys() {
                if package_map.contains_key(dep_name) {
                    graph.get_mut(dep_name).unwrap().push(package.name.clone());
                    *in_degree.get_mut(&package.name).unwrap() += 1;
                }
            }
        }
        
        // Kahn's algorithm for topological sort
        let mut queue: VecDeque<String> = VecDeque::new();
        let mut result = Vec::new();
        
        // Find all nodes with no incoming edges
        for (name, &degree) in &in_degree {
            if degree == 0 {
                queue.push_back(name.clone());
            }
        }
        
        while let Some(current) = queue.pop_front() {
            result.push(package_map[&current].clone());
            
            // Remove edges from current node
            if let Some(neighbors) = graph.get(&current) {
                for neighbor in neighbors {
                    let degree = in_degree.get_mut(neighbor).unwrap();
                    *degree -= 1;
                    if *degree == 0 {
                        queue.push_back(neighbor.clone());
                    }
                }
            }
        }
        
        // Check for circular dependencies
        if result.len() != package_map.len() {
            let remaining: Vec<String> = package_map
                .keys()
                .filter(|name| !result.iter().any(|p| &p.name == *name))
                .cloned()
                .collect();
            
            return Err(BuildError::CircularDependency {
                cycle: remaining,
            });
        }
        
        Ok(result)
    }

    async fn compile_package(&self, package: &PackageInfo) -> Result<CompilationResult> {
        debug!("Compiling package: {}", package.name);
        
        // Ensure output directory exists
        if let Some(parent) = package.output_path.parent() {
            tokio::fs::create_dir_all(parent).await?;
        }
        
        // Resolve import paths for this package
        let import_paths = self.resolve_import_paths_for_package(package)?;
        
        let invocation = asthra_compiler::CompilerInvocation {
            source_files: package.source_files.clone(),
            import_paths,
            output_path: package.output_path.clone(),
            target: self.config.target.clone(),
            optimization: self.config.optimization.clone(),
            additional_args: self.build_additional_compiler_args(package)?,
        };
        
        let start_time = std::time::Instant::now();
        let output = self.compiler_interface.compile(invocation).await?;
        let compilation_time = start_time.elapsed();
        
        if output.success {
            // Create library artifact if this is a library package
            let library_artifact = self.create_library_artifact(package, &output).await?;
            
            Ok(CompilationResult {
                package: package.clone(),
                output_path: package.output_path.clone(),
                warnings: self.parse_warnings(&output.stderr)?,
                library_artifact,
                compilation_time,
            })
        } else {
            Err(BuildError::CompilationFailed {
                package: package.name.clone(),
                stdout: output.stdout,
                stderr: output.stderr,
                exit_code: output.exit_code,
            })
        }
    }

    fn resolve_import_paths_for_package(&self, package: &PackageInfo) -> Result<Vec<PathBuf>> {
        let mut import_paths = Vec::new();
        
        // Always include standard library path
        let stdlib_path = self.config.build_directories.target_dir
            .parent()
            .unwrap_or(&self.config.build_directories.target_dir)
            .join("stdlib");
        if stdlib_path.exists() {
            import_paths.push(stdlib_path);
        }
        
        // Include dependency paths
        for dep_name in package.dependencies.keys() {
            if dep_name.starts_with("stdlib/") {
                // Standard library dependencies are already included above
                continue;
            } else if dep_name.starts_with("internal/") {
                // Internal dependencies should not be accessible
                return Err(BuildError::AccessViolation {
                    message: format!("Package {} cannot access internal dependency: {}", 
                                   package.name, dep_name),
                });
            } else if dep_name.contains("github.com/") || dep_name.contains("gitlab.com/") {
                // Third-party dependencies from cache
                let dep_path = self.config.build_directories.cache_dir.join(dep_name);
                if dep_path.exists() {
                    import_paths.push(dep_path);
                }
            } else {
                // Local dependencies
                let dep_path = self.config.build_directories.target_dir
                    .parent()
                    .unwrap_or(&self.config.build_directories.target_dir)
                    .join("src")
                    .join(dep_name);
                if dep_path.exists() {
                    import_paths.push(dep_path);
                }
            }
        }
        
        // Include the package's own source directory for local imports
        if let Some(first_source) = package.source_files.first() {
            if let Some(source_dir) = first_source.parent() {
                import_paths.push(source_dir.to_path_buf());
            }
        }
        
        Ok(import_paths)
    }
    
    fn build_additional_compiler_args(&self, package: &PackageInfo) -> Result<Vec<OsString>> {
        let mut args = Vec::new();
        
        // Add debug information if enabled
        if self.config.debug_info {
            args.push(OsString::from("--debug"));
        }
        
        // Add optimization level
        match self.config.optimization {
            crate::config::OptimizationLevel::None => args.push(OsString::from("--opt=0")),
            crate::config::OptimizationLevel::Basic => args.push(OsString::from("--opt=1")),
            crate::config::OptimizationLevel::Standard => args.push(OsString::from("--opt=1")),
            crate::config::OptimizationLevel::Aggressive => args.push(OsString::from("--opt=2")),
        }
        
        // Add target architecture
        match self.config.target {
            crate::config::Target::Native => {}, // Default, no flag needed
            crate::config::Target::X86_64 => args.push(OsString::from("--target=x86_64")),
            crate::config::Target::Arm64 => args.push(OsString::from("--target=arm64")),
            crate::config::Target::Wasm32 => args.push(OsString::from("--target=wasm32")),
        }
        
        // Add library type preference
        match self.config.library_type {
            crate::storage::LibraryType::Static => args.push(OsString::from("--library-type=static")),
            crate::storage::LibraryType::Dynamic => args.push(OsString::from("--library-type=dynamic")),
            crate::storage::LibraryType::Object => args.push(OsString::from("--library-type=object")),
        }
        
        // Add package-specific flags
        if package.name.starts_with("stdlib/") {
            args.push(OsString::from("--stdlib-mode"));
        }
        
        Ok(args)
    }
    
    async fn create_library_artifact(&self, package: &PackageInfo, _output: &asthra_compiler::CompilerOutput) -> Result<Option<crate::storage::LibraryArtifact>> {
        // Only create library artifacts for library packages
        if package.name == "main" || package.main_file.file_name().map_or(false, |n| n == "main.asthra") {
            return Ok(None);
        }
        
        let artifact = crate::storage::LibraryArtifact {
            name: package.name.clone(),
            version: package.version.clone(),
            library_type: self.config.library_type.clone(),
            file_path: package.output_path.clone(),
            dependencies: package.dependencies.keys().cloned().collect(),
        };
        
        // Store the artifact in the library cache
        let artifact_path = self.config.build_directories.deps_dir
            .join(artifact.generate_filename());
        
        // Copy the compiled output to the library cache
        if package.output_path.exists() {
            tokio::fs::copy(&package.output_path, &artifact_path).await?;
        }
        
        Ok(Some(artifact))
    }

    async fn compile_packages_parallel(&self, packages: &[PackageInfo]) -> Result<Vec<CompilationResult>> {
        let semaphore = Arc::new(Semaphore::new(self.config.parallel_jobs));
        let mut handles = Vec::new();
        
        for package in packages {
            let sem = semaphore.clone();
            let package = package.clone();
            let config = self.config.clone();
            let compiler_interface = self.compiler_interface.clone();
            
            let handle = tokio::spawn(async move {
                let _permit = sem.acquire().await.unwrap();
                
                // Use the same compilation logic as the sequential version
                Self::compile_package_static(&config, &compiler_interface, &package).await
            });
            
            handles.push(handle);
        }
        
        let mut results = Vec::new();
        for handle in handles {
            results.push(handle.await??);
        }
        
        Ok(results)
    }
    
    // Static version of compile_package for use in parallel compilation
    async fn compile_package_static(
        config: &BuildConfig,
        compiler_interface: &asthra_compiler::AsthraCCompilerInterface,
        package: &PackageInfo,
    ) -> Result<CompilationResult> {
        debug!("Compiling package: {}", package.name);
        
        // Ensure output directory exists
        if let Some(parent) = package.output_path.parent() {
            tokio::fs::create_dir_all(parent).await?;
        }
        
        // Resolve import paths for this package
        let import_paths = Self::resolve_import_paths_for_package_static(config, package)?;
        
        let invocation = asthra_compiler::CompilerInvocation {
            source_files: package.source_files.clone(),
            import_paths,
            output_path: package.output_path.clone(),
            target: config.target.clone(),
            optimization: config.optimization.clone(),
            additional_args: Self::build_additional_compiler_args_static(config, package)?,
        };
        
        let start_time = std::time::Instant::now();
        let output = compiler_interface.compile(invocation).await?;
        let compilation_time = start_time.elapsed();
        
        if output.success {
            // Create library artifact if this is a library package
            let library_artifact = Self::create_library_artifact_static(config, package, &output).await?;
            
            Ok(CompilationResult {
                package: package.clone(),
                output_path: package.output_path.clone(),
                warnings: Self::parse_warnings_static(&output.stderr)?,
                library_artifact,
                compilation_time,
            })
        } else {
            Err(BuildError::CompilationFailed {
                package: package.name.clone(),
                stdout: output.stdout,
                stderr: output.stderr,
                exit_code: output.exit_code,
            })
        }
    }
    
    // Static helper functions for parallel compilation
    fn resolve_import_paths_for_package_static(config: &BuildConfig, package: &PackageInfo) -> Result<Vec<PathBuf>> {
        let mut import_paths = Vec::new();
        
        // Always include standard library path
        let stdlib_path = config.build_directories.target_dir
            .parent()
            .unwrap_or(&config.build_directories.target_dir)
            .join("stdlib");
        if stdlib_path.exists() {
            import_paths.push(stdlib_path);
        }
        
        // Include dependency paths
        for dep_name in package.dependencies.keys() {
            if dep_name.starts_with("stdlib/") {
                continue;
            } else if dep_name.starts_with("internal/") {
                return Err(BuildError::AccessViolation {
                    message: format!("Package {} cannot access internal dependency: {}", 
                                   package.name, dep_name),
                });
            } else if dep_name.contains("github.com/") || dep_name.contains("gitlab.com/") {
                let dep_path = config.build_directories.cache_dir.join(dep_name);
                if dep_path.exists() {
                    import_paths.push(dep_path);
                }
            } else {
                let dep_path = config.build_directories.target_dir
                    .parent()
                    .unwrap_or(&config.build_directories.target_dir)
                    .join("src")
                    .join(dep_name);
                if dep_path.exists() {
                    import_paths.push(dep_path);
                }
            }
        }
        
        // Include the package's own source directory for local imports
        if let Some(first_source) = package.source_files.first() {
            if let Some(source_dir) = first_source.parent() {
                import_paths.push(source_dir.to_path_buf());
            }
        }
        
        Ok(import_paths)
    }
    
    fn build_additional_compiler_args_static(config: &BuildConfig, package: &PackageInfo) -> Result<Vec<OsString>> {
        let mut args = Vec::new();
        
        if config.debug_info {
            args.push(OsString::from("--debug"));
        }
        
        match config.optimization {
            crate::config::OptimizationLevel::None => args.push(OsString::from("--opt=0")),
            crate::config::OptimizationLevel::Basic => args.push(OsString::from("--opt=1")),
            crate::config::OptimizationLevel::Standard => args.push(OsString::from("--opt=1")),
            crate::config::OptimizationLevel::Aggressive => args.push(OsString::from("--opt=2")),
        }
        
        match config.target {
            crate::config::Target::Native => {},
            crate::config::Target::X86_64 => args.push(OsString::from("--target=x86_64")),
            crate::config::Target::Arm64 => args.push(OsString::from("--target=arm64")),
            crate::config::Target::Wasm32 => args.push(OsString::from("--target=wasm32")),
        }
        
        match config.library_type {
            crate::storage::LibraryType::Static => args.push(OsString::from("--library-type=static")),
            crate::storage::LibraryType::Dynamic => args.push(OsString::from("--library-type=dynamic")),
            crate::storage::LibraryType::Object => args.push(OsString::from("--library-type=object")),
        }
        
        if package.name.starts_with("stdlib/") {
            args.push(OsString::from("--stdlib-mode"));
        }
        
        Ok(args)
    }
    
    async fn create_library_artifact_static(config: &BuildConfig, package: &PackageInfo, _output: &asthra_compiler::CompilerOutput) -> Result<Option<crate::storage::LibraryArtifact>> {
        if package.name == "main" || package.main_file.file_name().map_or(false, |n| n == "main.asthra") {
            return Ok(None);
        }
        
        let artifact = crate::storage::LibraryArtifact {
            name: package.name.clone(),
            version: package.version.clone(),
            library_type: config.library_type.clone(),
            file_path: package.output_path.clone(),
            dependencies: package.dependencies.keys().cloned().collect(),
        };
        
        let artifact_path = config.build_directories.deps_dir
            .join(artifact.generate_filename());
        
        if package.output_path.exists() {
            tokio::fs::copy(&package.output_path, &artifact_path).await?;
        }
        
        Ok(Some(artifact))
    }
    
    fn parse_warnings_static(stderr: &str) -> Result<Vec<String>> {
        let warnings = stderr
            .lines()
            .filter(|line| line.contains("warning:"))
            .map(|line| line.to_string())
            .collect();
        
        Ok(warnings)
    }

    async fn link_executable(&self, results: &[CompilationResult]) -> Result<PathBuf> {
        // TODO: Implement linking step
        // For now, if we have results, return the first one's output path
        // In a real implementation, this would link all object files together
        if let Some(first_result) = results.first() {
            Ok(first_result.output_path.clone())
        } else {
            // No packages to link, create a default output path
            Ok(self.config.output_dir.join("main"))
        }
    }

    fn parse_warnings(&self, stderr: &str) -> Result<Vec<String>> {
        // Parse compiler warnings from stderr
        let warnings = stderr
            .lines()
            .filter(|line| line.contains("warning:"))
            .map(|line| line.to_string())
            .collect();
        
        Ok(warnings)
    }
} 
