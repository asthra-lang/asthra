use crate::config::AsthraMod;
use crate::error::{BuildError, Result};
use std::path::Path;
use tokio::fs;

pub struct AsthraTOMLParser;

impl AsthraTOMLParser {
    pub async fn parse_file(path: &Path) -> Result<AsthraMod> {
        let content = fs::read_to_string(path).await?;
        Self::parse_content(&content)
    }

    pub fn parse_content(content: &str) -> Result<AsthraMod> {
        let asthra_mod: AsthraMod = toml::from_str(content)?;
        Self::validate(&asthra_mod)?;
        Ok(asthra_mod)
    }

    fn validate(asthra_mod: &AsthraMod) -> Result<()> {
        // Validate package name format
        if asthra_mod.package.name.is_empty() {
            return Err(BuildError::InvalidImportPath(
                "Package name cannot be empty".to_string(),
            ));
        }

        // Validate version format
        if let Err(_) = semver::Version::parse(&asthra_mod.package.version) {
            return Err(BuildError::InvalidImportPath(format!(
                "Invalid version format: {}",
                asthra_mod.package.version
            )));
        }

        // Validate asthra version compatibility
        if let Err(_) = semver::VersionReq::parse(&asthra_mod.package.asthra_version) {
            return Err(BuildError::InvalidImportPath(format!(
                "Invalid asthra version requirement: {}",
                asthra_mod.package.asthra_version
            )));
        }

        // Validate dependency version requirements
        for (dep_name, version_req) in &asthra_mod.dependencies {
            if let Err(_) = semver::VersionReq::parse(version_req) {
                return Err(BuildError::InvalidImportPath(format!(
                    "Invalid version requirement for dependency '{}': {}",
                    dep_name, version_req
                )));
            }
        }

        // Validate dev-dependency version requirements
        for (dep_name, version_req) in &asthra_mod.dev_dependencies {
            if let Err(_) = semver::VersionReq::parse(version_req) {
                return Err(BuildError::InvalidImportPath(format!(
                    "Invalid version requirement for dev-dependency '{}': {}",
                    dep_name, version_req
                )));
            }
        }

        Ok(())
    }

    pub async fn save_file(asthra_mod: &AsthraMod, path: &Path) -> Result<()> {
        let content = toml::to_string_pretty(asthra_mod)?;
        fs::write(path, content).await?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::config::{BuildConfig, PackageConfig};

    #[test]
    fn test_parse_valid_toml() {
        let content = r#"
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
optimization = "standard"
"#;

        let result = AsthraTOMLParser::parse_content(content);
        assert!(result.is_ok());
        
        let asthra_mod = result.unwrap();
        assert_eq!(asthra_mod.package.name, "github.com/user/myproject");
        assert_eq!(asthra_mod.package.version, "1.0.0");
        assert_eq!(asthra_mod.dependencies.len(), 2);
        assert_eq!(asthra_mod.dev_dependencies.len(), 1);
    }

    #[test]
    fn test_parse_invalid_version() {
        let content = r#"
[package]
name = "test"
version = "invalid-version"
asthra_version = "1.0"
"#;

        let result = AsthraTOMLParser::parse_content(content);
        assert!(result.is_err());
    }
} 
