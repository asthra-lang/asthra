use std::collections::HashMap;
use std::path::PathBuf;
use std::time::{Duration, SystemTime};
use serde::{Serialize, Deserialize};
use semver::Version;
use crate::error::{BuildError, Result};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LibraryArtifact {
    pub name: String,           // "stdlib_string", "github_com_user_package"
    pub version: Version,       // For cache invalidation
    pub library_type: LibraryType,
    pub file_path: PathBuf,     // Absolute path to .a/.so/.lib/.dll
    pub dependencies: Vec<String>, // Link dependencies
}

#[derive(Debug, Clone, Serialize, Deserialize)]
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

#[derive(Debug, Clone)]
pub enum BuildProfile {
    Debug,
    Release,
    Test,
}

impl std::fmt::Display for BuildProfile {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            BuildProfile::Debug => write!(f, "debug"),
            BuildProfile::Release => write!(f, "release"),
            BuildProfile::Test => write!(f, "test"),
        }
    }
}

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
    pub fn new(project_root: PathBuf, profile: BuildProfile) -> Result<Self> {
        let target_dir = project_root.join("target");
        let profile_name = profile.to_string();
        let profile_dir = target_dir.join(profile_name);
        
        Ok(Self {
            project_root: project_root.clone(),
            target_dir: target_dir.clone(),
            profile_dir: profile_dir.clone(),
            deps_dir: profile_dir.join("deps"),
            build_dir: profile_dir.join("build"),
            external_dir: profile_dir.join("deps").join("external"),
            cache_dir: target_dir.join("cache"),
            global_cache: Self::get_global_cache_dir()?,
        })
    }

    pub async fn ensure_directories_exist(&self) -> Result<()> {
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

    #[cfg(windows)]
    fn get_global_cache_dir() -> Result<PathBuf> {
        let appdata = std::env::var("APPDATA")
            .map_err(|_| BuildError::MissingEnvironmentVariable("APPDATA".to_string()))?;
        Ok(PathBuf::from(appdata).join("asthra"))
    }

    #[cfg(unix)]
    fn get_global_cache_dir() -> Result<PathBuf> {
        let home = std::env::var("HOME")
            .map_err(|_| BuildError::MissingEnvironmentVariable("HOME".to_string()))?;
        Ok(PathBuf::from(home).join(".asthra"))
    }
}

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
    pub fn new() -> Self {
        Self {
            libraries: HashMap::new(),
            build_fingerprints: HashMap::new(),
            last_cleanup: SystemTime::now(),
        }
    }

    pub async fn is_up_to_date(&self, package_name: &str) -> Result<bool> {
        if let Some(cached) = self.libraries.get(package_name) {
            let current_hash = self.calculate_source_hash(package_name).await?;
            let current_dep_hash = self.calculate_dependency_hash(package_name).await?;
            
            Ok(cached.source_hash == current_hash && 
               cached.dependency_hash == current_dep_hash)
        } else {
            Ok(false)
        }
    }

    pub async fn cleanup_old_artifacts(&mut self, max_age_days: u64) -> Result<()> {
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

    async fn calculate_source_hash(&self, _package_name: &str) -> Result<String> {
        // TODO: Implement source file hashing
        Ok("placeholder_hash".to_string())
    }

    async fn calculate_dependency_hash(&self, _package_name: &str) -> Result<String> {
        // TODO: Implement dependency hashing
        Ok("placeholder_dep_hash".to_string())
    }
}

#[derive(Debug, Clone)]
pub enum CleanType {
    All,                    // Clean everything
    Profile(String),        // Clean specific profile (debug/release)
    Dependencies,           // Clean only dependencies
    Cache,                  // Clean build cache
} 
