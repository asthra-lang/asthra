pub mod asthra_toml;
pub mod workspace;

use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::path::PathBuf;

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
    #[serde(default)]
    pub description: Option<String>,
    #[serde(default)]
    pub authors: Vec<String>,
    #[serde(default)]
    pub license: Option<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct BuildConfig {
    #[serde(default = "default_target")]
    pub target: Target,
    #[serde(default = "default_optimization")]
    pub optimization: OptimizationLevel,
    #[serde(default)]
    pub debug_info: bool,
    #[serde(default)]
    pub parallel_jobs: Option<usize>,
}

impl Default for BuildConfig {
    fn default() -> Self {
        Self {
            target: default_target(),
            optimization: default_optimization(),
            debug_info: false,
            parallel_jobs: None,
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct WorkspaceConfig {
    pub members: Vec<String>,
    #[serde(default)]
    pub exclude: Vec<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Target {
    #[serde(rename = "native")]
    Native,
    #[serde(rename = "x86_64")]
    X86_64,
    #[serde(rename = "arm64")]
    Arm64,
    #[serde(rename = "wasm32")]
    Wasm32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum OptimizationLevel {
    #[serde(rename = "none")]
    None,
    #[serde(rename = "basic")]
    Basic,
    #[serde(rename = "standard")]
    Standard,
    #[serde(rename = "aggressive")]
    Aggressive,
}

#[derive(Debug, Clone)]
pub struct Package {
    pub name: String,
    pub version: semver::Version,
    pub dependencies: HashMap<String, semver::VersionReq>,
    pub package_type: PackageType,
    pub source_files: Vec<PathBuf>,
    pub main_file: PathBuf,
    pub output_path: PathBuf,
    pub dependency_paths: Vec<PathBuf>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PackageType {
    UserCode,
    Stdlib,
    Internal,
    ThirdParty,
}

#[derive(Debug, Clone)]
pub struct PackageInfo {
    pub name: String,
    pub version: semver::Version,
    pub dependencies: HashMap<String, semver::VersionReq>,
    pub source_files: Vec<PathBuf>,
    pub main_file: PathBuf,
    pub output_path: PathBuf,
    pub checksum: String,
}

fn default_target() -> Target {
    Target::Native
}

fn default_optimization() -> OptimizationLevel {
    OptimizationLevel::Standard
} 
