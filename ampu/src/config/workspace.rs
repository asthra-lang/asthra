use crate::config::{AsthraMod, WorkspaceConfig};
use crate::error::{BuildError, Result};
use std::path::{Path, PathBuf};
use tokio::fs;
use walkdir::WalkDir;

pub struct WorkspaceManager {
    root_path: PathBuf,
}

impl WorkspaceManager {
    pub fn new(root_path: PathBuf) -> Self {
        Self { root_path }
    }

    pub async fn discover_workspace(&self) -> Result<Option<WorkspaceConfig>> {
        let asthra_toml_path = self.root_path.join("asthra.toml");
        
        if !asthra_toml_path.exists() {
            return Ok(None);
        }

        let content = fs::read_to_string(&asthra_toml_path).await?;
        let asthra_mod: AsthraMod = toml::from_str(&content)?;
        
        Ok(asthra_mod.workspace)
    }

    pub async fn find_workspace_members(&self, workspace: &WorkspaceConfig) -> Result<Vec<PathBuf>> {
        let mut members = Vec::new();
        
        for member_pattern in &workspace.members {
            let member_paths = self.resolve_glob_pattern(member_pattern)?;
            for path in member_paths {
                if !workspace.exclude.iter().any(|exclude| path.to_string_lossy().contains(exclude)) {
                    if self.is_valid_package(&path).await? {
                        members.push(path);
                    }
                }
            }
        }
        
        Ok(members)
    }

    async fn is_valid_package(&self, path: &Path) -> Result<bool> {
        let asthra_toml = path.join("asthra.toml");
        Ok(asthra_toml.exists())
    }

    fn resolve_glob_pattern(&self, pattern: &str) -> Result<Vec<PathBuf>> {
        let mut paths = Vec::new();
        
        if pattern.contains('*') {
            // Handle glob patterns
            for entry in WalkDir::new(&self.root_path)
                .max_depth(if pattern.contains("**") { 10 } else { 2 })
                .into_iter()
                .filter_map(|e| e.ok())
            {
                let path = entry.path();
                if path.is_dir() && self.matches_pattern(path, pattern) {
                    paths.push(path.to_path_buf());
                }
            }
        } else {
            // Direct path
            let path = self.root_path.join(pattern);
            if path.exists() && path.is_dir() {
                paths.push(path);
            }
        }
        
        Ok(paths)
    }

    fn matches_pattern(&self, path: &Path, pattern: &str) -> bool {
        let relative_path = path.strip_prefix(&self.root_path).unwrap_or(path);
        let path_str = relative_path.to_string_lossy();
        
        // Simple glob matching - could be enhanced with a proper glob library
        if pattern.contains('*') {
            let pattern_parts: Vec<&str> = pattern.split('*').collect();
            if pattern_parts.len() == 2 {
                let prefix = pattern_parts[0];
                let suffix = pattern_parts[1];
                return path_str.starts_with(prefix) && path_str.ends_with(suffix);
            }
        }
        
        path_str == pattern
    }

    pub async fn validate_workspace(&self, workspace: &WorkspaceConfig) -> Result<()> {
        let members = self.find_workspace_members(workspace).await?;
        
        if members.is_empty() {
            return Err(BuildError::InvalidImportPath(
                "Workspace has no valid members".to_string(),
            ));
        }

        // Check for circular dependencies between workspace members
        // This is a simplified check - a full implementation would build a dependency graph
        for member_path in &members {
            let asthra_toml = member_path.join("asthra.toml");
            let content = fs::read_to_string(&asthra_toml).await?;
            let member_mod: AsthraMod = toml::from_str(&content)?;
            
            // Check if this member depends on other workspace members
            for (dep_name, _) in &member_mod.dependencies {
                if self.is_workspace_member(dep_name, &members).await? {
                    // Found workspace dependency - could check for cycles here
                    tracing::debug!("Workspace member {} depends on {}", member_mod.package.name, dep_name);
                }
            }
        }
        
        Ok(())
    }

    async fn is_workspace_member(&self, package_name: &str, members: &[PathBuf]) -> Result<bool> {
        for member_path in members {
            let asthra_toml = member_path.join("asthra.toml");
            if asthra_toml.exists() {
                let content = fs::read_to_string(&asthra_toml).await?;
                let member_mod: AsthraMod = toml::from_str(&content)?;
                if member_mod.package.name == package_name {
                    return Ok(true);
                }
            }
        }
        Ok(false)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use tempfile::TempDir;

    #[tokio::test]
    async fn test_workspace_discovery() {
        let temp_dir = TempDir::new().unwrap();
        let workspace_manager = WorkspaceManager::new(temp_dir.path().to_path_buf());
        
        // No asthra.toml file
        let result = workspace_manager.discover_workspace().await.unwrap();
        assert!(result.is_none());
    }

    #[test]
    fn test_glob_pattern_matching() {
        let temp_dir = TempDir::new().unwrap();
        let workspace_manager = WorkspaceManager::new(temp_dir.path().to_path_buf());
        
        assert!(workspace_manager.matches_pattern(Path::new("packages/core"), "packages/*"));
        assert!(workspace_manager.matches_pattern(Path::new("tools/cli"), "tools/*"));
        assert!(!workspace_manager.matches_pattern(Path::new("src/main"), "packages/*"));
    }
} 
