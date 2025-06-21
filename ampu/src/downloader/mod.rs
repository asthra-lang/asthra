pub mod git_downloader;
pub mod cache;

use crate::config::PackageInfo;
use crate::error::{BuildError, Result};
use crate::resolver::{DependencyNode, ResolvedDependencies};
use std::collections::HashMap;
use std::path::PathBuf;
use semver::{Version, VersionReq};
use tracing::{debug, info};

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
    pub fn from_import_path(import_path: &str) -> Result<Self> {
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

pub struct PackageDownloader {
    cache_dir: PathBuf,
    client: reqwest::Client,
}

impl PackageDownloader {
    pub fn new(cache_dir: PathBuf) -> Self {
        Self {
            cache_dir,
            client: reqwest::Client::new(),
        }
    }

    pub async fn download_package(
        &self,
        import_path: &str,
        version_req: &VersionReq,
    ) -> Result<PackageInfo> {
        debug!("Downloading package: {} with version requirement: {}", import_path, version_req);
        
        let source = PackageSource::from_import_path(import_path)?;
        let cache_path = self.get_cache_path(import_path);

        // Check if already cached
        if cache_path.exists() {
            if let Ok(cached) = self.load_cached_package(&cache_path).await {
                if version_req.matches(&cached.version) {
                    debug!("Using cached package: {} v{}", cached.name, cached.version);
                    return Ok(cached);
                }
            }
        }

        info!("Downloading {} from {}", import_path, source.url);

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

    fn get_cache_path(&self, import_path: &str) -> PathBuf {
        self.cache_dir.join(import_path)
    }

    async fn load_cached_package(&self, cache_path: &PathBuf) -> Result<PackageInfo> {
        let asthra_toml_path = cache_path.join("asthra.toml");
        if !asthra_toml_path.exists() {
            return Err(BuildError::PackageNotFound(
                "No asthra.toml found in cached package".to_string()
            ));
        }

        let asthra_mod = crate::config::asthra_toml::AsthraTOMLParser::parse_file(&asthra_toml_path).await?;
        let version = Version::parse(&asthra_mod.package.version)?;
        
        // Convert dependencies to VersionReq
        let mut dependencies = HashMap::new();
        for (name, version_str) in asthra_mod.dependencies {
            let version_req = VersionReq::parse(&version_str)?;
            dependencies.insert(name, version_req);
        }

        Ok(PackageInfo {
            name: asthra_mod.package.name,
            version,
            dependencies,
            source_files: self.discover_source_files(cache_path).await?,
            main_file: cache_path.join("src").join("main.asthra"), // TODO: determine actual main file
            output_path: cache_path.join("target").join("lib.o"),
            checksum: self.calculate_checksum(cache_path).await?,
        })
    }

    async fn clone_or_update_repo(
        &self,
        source: &PackageSource,
        cache_path: &PathBuf,
    ) -> Result<()> {
        use git2::Repository;
        
        if cache_path.exists() {
            debug!("Updating existing repository at {}", cache_path.display());
            // Update existing repo
            let repo = Repository::open(cache_path)?;
            let mut remote = repo.find_remote("origin")?;
            remote.fetch(&["refs/heads/*:refs/remotes/origin/*"], None, None)?;
        } else {
            debug!("Cloning repository to {}", cache_path.display());
            // Clone new repo
            tokio::fs::create_dir_all(cache_path.parent().unwrap()).await?;
            Repository::clone(&source.url, cache_path)?;
        }
        Ok(())
    }

    async fn get_available_versions(&self, repo_path: &PathBuf) -> Result<Vec<Version>> {
        use git2::Repository;
        
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

    fn find_best_version(&self, available_versions: &[Version], version_req: &VersionReq) -> Result<Version> {
        for version in available_versions.iter().rev() {
            if version_req.matches(version) {
                return Ok(version.clone());
            }
        }
        
        Err(BuildError::PackageNotFound(format!(
            "No version matching {} found in available versions: {:?}",
            version_req, available_versions
        )))
    }

    async fn checkout_version(&self, repo_path: &PathBuf, version: &Version) -> Result<()> {
        use git2::{Repository, ObjectType};
        
        let repo = Repository::open(repo_path)?;
        let tag_name = format!("v{}", version);
        
        // Try to find the tag
        if let Ok(tag_ref) = repo.find_reference(&format!("refs/tags/{}", tag_name)) {
            let tag_oid = tag_ref.target().unwrap();
            let tag_object = repo.find_object(tag_oid, Some(ObjectType::Any))?;
            
            // Checkout the tag
            repo.checkout_tree(&tag_object, None)?;
            repo.set_head_detached(tag_oid)?;
        } else {
            debug!("Tag {} not found, staying on current branch", tag_name);
        }
        
        Ok(())
    }

    async fn parse_package_info(&self, cache_path: &PathBuf, version: Version) -> Result<PackageInfo> {
        let asthra_toml_path = cache_path.join("asthra.toml");
        let asthra_mod = crate::config::asthra_toml::AsthraTOMLParser::parse_file(&asthra_toml_path).await?;
        
        // Convert dependencies to VersionReq
        let mut dependencies = HashMap::new();
        for (name, version_str) in asthra_mod.dependencies {
            let version_req = VersionReq::parse(&version_str)?;
            dependencies.insert(name, version_req);
        }

        Ok(PackageInfo {
            name: asthra_mod.package.name,
            version,
            dependencies,
            source_files: self.discover_source_files(cache_path).await?,
            main_file: cache_path.join("src").join("main.asthra"),
            output_path: cache_path.join("target").join("lib.o"),
            checksum: self.calculate_checksum(cache_path).await?,
        })
    }

    async fn discover_source_files(&self, package_path: &PathBuf) -> Result<Vec<PathBuf>> {
        let mut source_files = Vec::new();
        let src_dir = package_path.join("src");
        
        if src_dir.exists() {
            for entry in walkdir::WalkDir::new(&src_dir) {
                let entry = entry?;
                let path = entry.path();
                
                if path.is_file() && path.extension().map_or(false, |ext| ext == "asthra") {
                    source_files.push(path.to_path_buf());
                }
            }
        }
        
        Ok(source_files)
    }

    async fn calculate_checksum(&self, package_path: &PathBuf) -> Result<String> {
        use sha2::{Sha256, Digest};
        
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
            let content = tokio::fs::read(&file_path).await?;
            hasher.update(&content);
        }
        
        Ok(format!("{:x}", hasher.finalize()))
    }
}

// Implement DependencyResolver here since it's closely related to downloading
pub struct DependencyResolver {
    downloader: PackageDownloader,
    resolved_cache: HashMap<String, DependencyNode>,
}

impl DependencyResolver {
    pub fn new() -> Self {
        let cache_dir = dirs::home_dir()
            .unwrap_or_else(|| PathBuf::from("."))
            .join(".asthra")
            .join("cache");
        
        Self {
            downloader: PackageDownloader::new(cache_dir),
            resolved_cache: HashMap::new(),
        }
    }

    pub async fn resolve_dependencies(
        &mut self,
        root_dependencies: &HashMap<String, String>,
    ) -> Result<ResolvedDependencies> {
        let mut resolved = HashMap::new();
        let mut to_resolve = std::collections::VecDeque::new();
        
        // Start with root dependencies
        for (import_path, version_str) in root_dependencies {
            let version_req = VersionReq::parse(version_str)?;
            to_resolve.push_back((import_path.clone(), version_req, None));
        }
        
        while let Some((import_path, version_req, dependent)) = to_resolve.pop_front() {
            if resolved.contains_key(&import_path) {
                // Check version compatibility
                let existing: &DependencyNode = &resolved[&import_path];
                if !version_req.matches(&existing.version) {
                    return Err(BuildError::VersionConflict {
                        package: import_path.clone(),
                        required: version_req.to_string(),
                        existing: existing.version.to_string(),
                        dependent: dependent.clone(),
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
} 
