use crate::error::Result;
use crate::resolver::ResolvedDependencies;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::path::{Path, PathBuf};
use semver::Version;
use chrono::{DateTime, Utc};
use tokio::fs;

#[derive(Debug, Serialize, Deserialize)]
pub struct LockFile {
    pub version: String,
    pub packages: HashMap<String, LockedPackage>,
    pub generated_at: DateTime<Utc>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct LockedPackage {
    pub version: Version,
    pub checksum: String,
    pub dependencies: HashMap<String, Version>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CachedPackage {
    pub import_path: String,
    pub version: Version,
    pub checksum: String,
    pub cached_at: DateTime<Utc>,
}

impl LockFile {
    pub fn generate(resolved: &ResolvedDependencies) -> Self {
        let mut packages = HashMap::new();
        
        for (name, node) in &resolved.packages {
            let locked = LockedPackage {
                version: node.version.clone(),
                checksum: String::new(), // TODO: Get checksum from package info
                dependencies: node.dependencies.iter()
                    .filter_map(|(dep_name, _)| {
                        resolved.packages.get(dep_name).map(|dep_node| {
                            (dep_name.clone(), dep_node.version.clone())
                        })
                    })
                    .collect(),
            };
            packages.insert(name.clone(), locked);
        }
        
        Self {
            version: "1".to_string(),
            packages,
            generated_at: Utc::now(),
        }
    }

    pub async fn save(&self, path: &Path) -> Result<()> {
        let content = toml::to_string_pretty(self)?;
        fs::write(path, content).await?;
        Ok(())
    }

    pub async fn load(path: &Path) -> Result<Self> {
        let content = fs::read_to_string(path).await?;
        let lock_file = toml::from_str(&content)?;
        Ok(lock_file)
    }

    pub fn is_up_to_date(&self, resolved: &ResolvedDependencies) -> bool {
        // Check if all packages in resolved dependencies match the lock file
        for (name, node) in &resolved.packages {
            if let Some(locked) = self.packages.get(name) {
                if locked.version != node.version {
                    return false;
                }
            } else {
                return false;
            }
        }
        
        // Check if lock file has extra packages
        if self.packages.len() != resolved.packages.len() {
            return false;
        }
        
        true
    }
}

pub struct PackageCache {
    cache_dir: PathBuf,
}

impl PackageCache {
    pub fn new(cache_dir: PathBuf) -> Self {
        Self { cache_dir }
    }

    pub async fn get_package_path(&self, import_path: &str) -> PathBuf {
        self.cache_dir.join(import_path)
    }

    pub async fn is_package_cached(&self, import_path: &str, version: &Version) -> bool {
        let package_path = self.get_package_path(import_path).await;
        let version_file = package_path.join(".version");
        
        if !package_path.exists() || !version_file.exists() {
            return false;
        }
        
        if let Ok(cached_version_str) = fs::read_to_string(&version_file).await {
            if let Ok(cached_version) = Version::parse(cached_version_str.trim()) {
                return cached_version == *version;
            }
        }
        
        false
    }

    pub async fn cache_package_version(&self, import_path: &str, version: &Version) -> Result<()> {
        let package_path = self.get_package_path(import_path).await;
        let version_file = package_path.join(".version");
        
        fs::create_dir_all(&package_path).await?;
        fs::write(&version_file, version.to_string()).await?;
        
        Ok(())
    }

    pub async fn clear_cache(&self) -> Result<()> {
        if self.cache_dir.exists() {
            fs::remove_dir_all(&self.cache_dir).await?;
        }
        Ok(())
    }

    pub async fn get_cache_size(&self) -> Result<u64> {
        if !self.cache_dir.exists() {
            return Ok(0);
        }
        
        let mut total_size = 0;
        let mut entries = fs::read_dir(&self.cache_dir).await?;
        
        while let Some(entry) = entries.next_entry().await? {
            if entry.file_type().await?.is_dir() {
                total_size += self.get_directory_size(&entry.path()).await?;
            } else {
                total_size += entry.metadata().await?.len();
            }
        }
        
        Ok(total_size)
    }

    async fn get_directory_size(&self, dir: &Path) -> Result<u64> {
        let mut total_size = 0;
        let mut entries = fs::read_dir(dir).await?;
        
        while let Some(entry) = entries.next_entry().await? {
            let metadata = entry.metadata().await?;
            if metadata.is_dir() {
                total_size += Box::pin(self.get_directory_size(&entry.path())).await?;
            } else {
                total_size += metadata.len();
            }
        }
        
        Ok(total_size)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use tempfile::TempDir;
    use crate::resolver::DependencyNode;

    #[tokio::test]
    async fn test_lock_file_generation() {
        let mut resolved = ResolvedDependencies {
            packages: HashMap::new(),
        };
        
        let node = DependencyNode {
            import_path: "github.com/test/package".to_string(),
            version: Version::parse("1.0.0").unwrap(),
            dependencies: HashMap::new(),
        };
        
        resolved.packages.insert("github.com/test/package".to_string(), node);
        
        let lock_file = LockFile::generate(&resolved);
        assert_eq!(lock_file.packages.len(), 1);
        assert!(lock_file.packages.contains_key("github.com/test/package"));
    }

    #[tokio::test]
    async fn test_package_cache() {
        let temp_dir = TempDir::new().unwrap();
        let cache = PackageCache::new(temp_dir.path().to_path_buf());
        
        let import_path = "github.com/test/package";
        let version = Version::parse("1.0.0").unwrap();
        
        // Initially not cached
        assert!(!cache.is_package_cached(import_path, &version).await);
        
        // Cache the package
        cache.cache_package_version(import_path, &version).await.unwrap();
        
        // Now it should be cached
        assert!(cache.is_package_cached(import_path, &version).await);
        
        // Different version should not be cached
        let different_version = Version::parse("2.0.0").unwrap();
        assert!(!cache.is_package_cached(import_path, &different_version).await);
    }
} 
